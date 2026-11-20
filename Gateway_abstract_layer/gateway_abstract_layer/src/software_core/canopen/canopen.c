/**
 * @file canopen.c
 * @brief Stage-3 CANopen slave (lely-core N7S) bridged to HAL CAN.
 *
 * Node-ID 0x10, producer heartbeat 100 ms, SYNC-driven TPDO 0x190 /
 * RPDO 0x210 (set S1). Non-blocking: all work happens in canopen_step().
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
    uint64_t tpdoSeq;
    uint64_t rpdoLast;
    uint64_t rpdoSeen;
    bool ready;
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

    /* PDOs run only in Operational; heartbeat/SDO already work in Pre-op. */
    if (st != CO_NMT_ST_START)
    {
        return;
    }

    /* TPDO (type 1) was already emitted with the current OD value. */
    ctx->tpdoCount++;
    ctx->tpdoSeq = co_dev_get_val_u64(ctx->dev, 0x2000U, 0x00U);

    /* Prepare next SYNC cycle's sequence counter. */
    ctx->tpdoSeq++;
    (void)co_dev_set_val_u64(ctx->dev, 0x2000U, 0x00U, ctx->tpdoSeq);

    /* RPDO values are actuated on SYNC before this callback. */
    {
        uint64_t rx = co_dev_get_val_u64(ctx->dev, 0x2100U, 0x00U);
        if (rx != ctx->rpdoSeen)
        {
            ctx->rpdoSeen = rx;
            ctx->rpdoLast = rx;
            ctx->rpdoCount++;
        }
    }
}

static void canopen_feed_rx(void)
{
    uint32_t n;

    for (n = 0U; n < CANOPEN_RX_MAX_PER_STEP; n++)
    {
        uint32_t id = 0U;
        uint32_t subId = 0U;
        uint8_t data[8];
        uint8_t len = 0U;
        struct can_msg msg;

        if (hwa_get_number_of_massages() == 0U)
        {
            break;
        }

        hwa_can_rx(&id, &subId, data, &len);
        (void)subId;
        if (len == 0U)
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

static void canopen_advance_time(void)
{
    struct timespec ts;
    uint32_t tloopUs = hwa_time_tloop_us();

    s_co.netTimeNs += (uint64_t)tloopUs * 1000ULL;
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
    if (!s_co.ready)
    {
        return;
    }

    canopen_feed_rx();
    canopen_advance_time();
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

uint64_t canopen_tpdo_seq(void)
{
    return s_co.tpdoSeq;
}
