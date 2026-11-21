/**
 * @file canopen.c
 * @brief Stage-3 CANopen slave (lely-core N7S) bridged to HAL CAN.
 *
 * Node-ID 0x10, producer heartbeat 100 ms, SYNC-driven TPDO 0x190 /
 * RPDO 0x210 (set S1). Object 0x2100 (RPDO) carries the data towards
 * EtherCAT, object 0x2000 (TPDO) the data from EtherCAT. Each SYNC is
 * reported once through canopen_sync_take(). Non-blocking: all work happens
 * in canopen_step().
 */

#include "canopen.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "od/gateway_od.h"

#include "hwa_interface.h"

#include <lely/util/mempool.h>
#include <lely/util/time.h>
#include <lely/can/net.h>
#include <lely/co/dev.h>
#include <lely/co/nmt.h>

#include <string.h>

#define CANOPEN_POOL_SIZE (48U * 1024U)
#define CANOPEN_RX_MAX_PER_STEP 16U
/* Outside the 11/29-bit identifier range: "no frame returned". */
#define CANOPEN_RX_NO_FRAME 0xFFFFFFFFUL

typedef struct
{
    uint8_t memory[CANOPEN_POOL_SIZE];
    struct mempool pool;
    alloc_t *alloc;
    can_net_t *net;
    co_dev_t *dev;
    co_nmt_t *nmt;
    uint64_t netTimeNs;
    uint32_t syncCount;
    uint32_t tpdoCount;
    uint32_t rpdoCount;
    uint64_t rpdoLast;
    uint64_t rpdoSeen;
    uint64_t syncRpdo;      /* 0x2100 value at the last SYNC */
    bool syncPending;
    bool syncRpdoValid;
    bool ready;
    /* supervision (thesis Table 5.5, CAN node role) */
    bool masterHbLost;          /* consumer heartbeat of the master timed out */
    uint32_t masterHbEvents;
    bool wasOperational;
    uint8_t lastSt;             /* NMT state in the previous step */
    bool localStateChange;      /* NMT state changed by the gateway itself */
    bool leftOpByMaster;        /* latched: master left Operational (stop/pre-op) */
    uint32_t enteredOpCount;    /* edges into Operational (NMT start = resume) */
} canopen_ctx_t;

static canopen_ctx_t s_co;

static int canopen_send(const struct can_msg *msg, uint_least8_t bus_id,
        void *data)
{
    uint8_t payload[8];
    uint8_t len;
    (void)bus_id;
    (void)data;

    if (msg == NULL)
    {
        return -1;
    }
    if ((msg->flags & CAN_FLAG_IDE) != 0U)
    {
        /* Stage 3 uses 11-bit identifiers only. */
        return -1;
    }

    len = msg->len;
    if (len > 8U)
    {
        len = 8U;
    }
    (void)memcpy(payload, msg->data, len);
    hwa_can_tx((uint32_t)msg->id, 0U, payload, len);
    return 0;
}

/* Heartbeat consumer event (object 0x1016: master 0x7F). Recorded for the
 * Error handler, then passed to lely's default handling, which switches the
 * node to Pre-operational on a timeout (error behaviour 0x1029 = 0): the safe
 * reaction of the CAN node role, visible in the heartbeat. */
static void canopen_hb_ind(co_nmt_t *nmt, co_unsigned8_t id, co_nmt_ec_state_t state,
        co_nmt_ec_reason_t reason, void *data)
{
    canopen_ctx_t *ctx = (canopen_ctx_t *)data;

    if ((ctx != NULL) && (reason == CO_NMT_EC_TIMEOUT))
    {
        ctx->masterHbLost = (state == CO_NMT_EC_OCCURRED);
        ctx->masterHbEvents++;
        ctx->localStateChange = true; /* pre-op caused by the timeout */
    }
    co_nmt_on_hb(nmt, id, state, reason);
}

static void canopen_sync_ind(co_nmt_t *nmt, co_unsigned8_t cnt, void *data)
{
    canopen_ctx_t *ctx = (canopen_ctx_t *)data;
    co_unsigned8_t st;
    (void)cnt;

    if ((ctx == NULL) || (nmt == NULL))
    {
        return;
    }

    ctx->syncCount++;
    st = co_nmt_get_st(nmt) & (co_unsigned8_t)~CO_NMT_ST_TOGGLE;

    /* Every SYNC starts a gateway cycle (configuration 3: EtherCAT cycle). */
    ctx->syncPending = true;
    ctx->syncRpdoValid = (st == CO_NMT_ST_START);

    /* PDOs run only in Operational; heartbeat/SDO already work in Pre-op. */
    if (st != CO_NMT_ST_START)
    {
        return;
    }

    /* TPDO (type 1) was already emitted with the current 0x2000 value. */
    ctx->tpdoCount++;

    /* RPDO values are actuated on SYNC before this callback. */
    ctx->syncRpdo = co_dev_get_val_u64(ctx->dev, 0x2100U, 0x00U);
    if (ctx->syncRpdo != ctx->rpdoSeen)
    {
        ctx->rpdoSeen = ctx->syncRpdo;
        ctx->rpdoLast = ctx->syncRpdo;
        ctx->rpdoCount++;
    }
}

static void canopen_feed_rx(void)
{
    uint32_t n;

    for (n = 0U; n < CANOPEN_RX_MAX_PER_STEP; n++)
    {
        uint32_t id = CANOPEN_RX_NO_FRAME;
        uint32_t subId = 0U;
        uint8_t data[8];
        uint8_t len = 0U;
        struct can_msg msg;

        if (hwa_get_number_of_massages() == 0U)
        {
            break;
        }

        /* len == 0 is a valid frame (SYNC, NMT error-control RTR), so an
         * empty queue is detected by hwa_can_rx() leaving id untouched. */
        hwa_can_rx(&id, &subId, data, &len);
        (void)subId;
        if (id == CANOPEN_RX_NO_FRAME)
        {
            break;
        }

        msg = (struct can_msg)CAN_MSG_INIT;
        msg.id = id;
        msg.flags = 0U;
        msg.len = (len > 8U) ? 8U : len;
        (void)memcpy(msg.data, data, msg.len);
        (void)can_net_recv(s_co.net, &msg, 0U);
    }
}

/* Network time follows the real clock, so a blocking EtherCAT configuration
 * (several Tloop periods) does not make lely timers (heartbeat) drift. */
static void canopen_advance_time(void)
{
    struct timespec ts;

    s_co.netTimeNs = hwa_time_now_us() * 1000ULL;
    ts.tv_sec = (time_t)(s_co.netTimeNs / 1000000000ULL);
    ts.tv_nsec = (long)(s_co.netTimeNs % 1000000000ULL);
    (void)can_net_set_time(s_co.net, &ts);
}

bool canopen_init(void)
{
    struct timespec ts = { 0, 0 };

    (void)memset(&s_co, 0, sizeof(s_co));

    s_co.alloc = mempool_init(&s_co.pool, s_co.memory, sizeof(s_co.memory));
    if (s_co.alloc == NULL)
    {
        return false;
    }

    s_co.net = can_net_create(s_co.alloc, 0U);
    if (s_co.net == NULL)
    {
        return false;
    }
    can_net_set_send_func(s_co.net, &canopen_send, NULL);
    (void)can_net_set_time(s_co.net, &ts);

    s_co.dev = gateway_od_init();
    if (s_co.dev == NULL)
    {
        return false;
    }
    if (co_dev_get_id(s_co.dev) != CANOPEN_NODE_ID)
    {
        (void)co_dev_set_id(s_co.dev, CANOPEN_NODE_ID);
    }

    s_co.nmt = co_nmt_create(s_co.net, s_co.dev);
    if (s_co.nmt == NULL)
    {
        return false;
    }

    co_nmt_set_sync_ind(s_co.nmt, &canopen_sync_ind, &s_co);
    co_nmt_set_hb_ind(s_co.nmt, &canopen_hb_ind, &s_co);

    /* Boot-up → pre-operational (sends 0x710 boot-up, starts heartbeat). */
    if (co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_RESET_NODE) == -1)
    {
        return false;
    }

    s_co.ready = true;
    return true;
}

void canopen_step(void)
{
    bool op;
    uint8_t st;

    if (!s_co.ready)
    {
        return;
    }

    canopen_feed_rx();
    canopen_advance_time();

    /* Safe-state request of the master (T7): leaving Operational, or NMT
     * stop from any state (also from Pre-operational). Entering Operational
     * is the resume command (T8). */
    st = canopen_nmt_state();
    op = (st == CO_NMT_ST_START);
    if (!s_co.localStateChange
            && ((s_co.wasOperational && !op)
                || ((st == CO_NMT_ST_STOP) && (s_co.lastSt != CO_NMT_ST_STOP))))
    {
        s_co.leftOpByMaster = true;
    }
    if (!s_co.wasOperational && op)
    {
        s_co.enteredOpCount++;
        s_co.leftOpByMaster = false;
    }
    s_co.localStateChange = false;
    s_co.wasOperational = op;
    s_co.lastSt = st;
}

uint8_t canopen_nmt_state(void)
{
    if (!s_co.ready || (s_co.nmt == NULL))
    {
        return CO_NMT_ST_BOOTUP;
    }
    return (uint8_t)(co_nmt_get_st(s_co.nmt) & (co_unsigned8_t)~CO_NMT_ST_TOGGLE);
}

uint32_t canopen_sync_count(void)
{
    return s_co.syncCount;
}

uint32_t canopen_tpdo_count(void)
{
    return s_co.tpdoCount;
}

uint32_t canopen_rpdo_count(void)
{
    return s_co.rpdoCount;
}

uint64_t canopen_rpdo_last(void)
{
    return s_co.rpdoLast;
}

uint64_t canopen_tpdo_data(void)
{
    if (!s_co.ready)
    {
        return 0U;
    }
    return co_dev_get_val_u64(s_co.dev, 0x2000U, 0x00U);
}

bool canopen_sync_take(uint8_t rpdo[8], bool *rpdoValid)
{
    uint32_t i;

    if (!s_co.ready || !s_co.syncPending)
    {
        return false;
    }
    s_co.syncPending = false;
    if (rpdo != NULL)
    {
        for (i = 0U; i < 8U; i++)
        {
            rpdo[i] = (uint8_t)(s_co.syncRpdo >> (8U * i));
        }
    }
    if (rpdoValid != NULL)
    {
        *rpdoValid = s_co.syncRpdoValid;
    }
    return true;
}

void canopen_tpdo_data_set(const uint8_t data[8])
{
    uint64_t v = 0U;
    uint32_t i;

    if (!s_co.ready || (data == NULL))
    {
        return;
    }
    for (i = 0U; i < 8U; i++)
    {
        v |= (uint64_t)data[i] << (8U * i);
    }
    (void)co_dev_set_val_u64(s_co.dev, 0x2000U, 0x00U, v);
}

bool canopen_operational(void)
{
    return s_co.ready && (canopen_nmt_state() == CO_NMT_ST_START);
}

bool canopen_master_hb_lost(void)
{
    return s_co.masterHbLost;
}

bool canopen_left_op_by_master(void)
{
    return s_co.leftOpByMaster;
}

uint32_t canopen_entered_op_count(void)
{
    return s_co.enteredOpCount;
}

void canopen_enter_preop(void)
{
    if (canopen_operational())
    {
        s_co.localStateChange = true;
        (void)co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_ENTER_PREOP);
    }
}

void canopen_reset_comm(void)
{
    if (!s_co.ready)
    {
        return;
    }
    s_co.localStateChange = true;
    s_co.leftOpByMaster = false;
    s_co.masterHbLost = false;
    s_co.syncPending = false;
    /* boot-up message, communication objects and heartbeat consumers reset */
    (void)co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_RESET_COMM);
}
