/**
 * @file canopen.c
 * @brief CANopen stack (lely-core N7S) bridged to HAL CAN, node or master role.
 *
 * Node role (configurations 1 and 3, set S1): node-ID 0x10, producer
 * heartbeat 100 ms, SYNC-driven TPDO 0x190 / RPDO 0x210. Object 0x2100 (RPDO)
 * carries the data towards EtherCAT, object 0x2000 (TPDO) the data from
 * EtherCAT. Each SYNC is reported once through canopen_sync_take().
 *
 * Master role (configurations 2 and 4, sets S2 and S7): node-ID 0x7F,
 * heartbeat 0x77F 100 ms, nodes 1..N. The SYNC is transmitted on request of
 * the cycle generator (canopen_master_sync_send); the synchronous TPDOs
 * 0x200+n (set-points 0x2300:n) follow it. The TPDOs of the nodes 0x180+n
 * are received asynchronously (0x2200:n) and queued for the conversion. The
 * states of the nodes come from their heartbeats (consumer 0x1016).
 *
 * Non-blocking: all work happens in canopen_step() and the calls of the
 * Software Core.
 */

#include "canopen.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "od/gateway_od.h"
#include "od/gateway_master_od.h"

#include "hwa_interface.h"

#include <lely/util/mempool.h>
#include <lely/util/time.h>
#include <lely/can/net.h>
#include <lely/co/dev.h>
#include <lely/co/nmt.h>
#include <lely/co/rpdo.h>
#include <lely/co/sync.h>

#include <string.h>

#define CANOPEN_POOL_SIZE (48U * 1024U)
#define CANOPEN_RX_MAX_PER_STEP 16U
/* Outside the 11/29-bit identifier range: "no frame returned". */
#define CANOPEN_RX_NO_FRAME 0xFFFFFFFFUL
#define CANOPEN_FB_QUEUE    16U

typedef struct
{
    bool     seen;       /* heartbeat or boot-up received since the reset */
    bool     lost;       /* heartbeat timeout, until its next heartbeat */
    bool     bootSeen;   /* boot-up while the nodes were started */
    uint8_t  st;         /* last NMT state reported by the node */
    uint32_t fbCount;    /* TPDOs received */
    uint64_t fbLast;
    uint32_t hbRx;       /* frames 0x700+n on the bus (any NMT state) */
    uint32_t pdoRx;      /* frames 0x180+n on the bus (any NMT state) */
} canopen_node_t;

typedef struct
{
    uint8_t node;
    uint8_t data[8];
} canopen_fb_t;

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
    /* master role */
    canopen_role_t role;
    uint8_t nodeCount;
    canopen_node_t nodes[CANOPEN_NODES_MAX];
    bool started;               /* nodes commanded to Operational */
    uint64_t startUs;
    canopen_fb_t fb[CANOPEN_FB_QUEUE];
    uint32_t fbHead;
    uint32_t fbTail;
    uint32_t fbOverflows;
    uint32_t nmtCommands;
    /* diagnostics: frames handed to the stack */
    uint32_t rxFrames;
    uint16_t rxLastId;
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

    if ((ctx != NULL) && (ctx->role == CANOPEN_ROLE_MASTER))
    {
        /* Master: the supervision reacts (thesis Table 5.5: NMT stop of the
         * nodes); lely's own reaction (reset of the node) is not used. */
        if ((reason == CO_NMT_EC_TIMEOUT) && (id >= 1U) && (id <= ctx->nodeCount))
        {
            ctx->nodes[id - 1U].lost = (state == CO_NMT_EC_OCCURRED);
        }
        return;
    }
    if ((ctx != NULL) && (reason == CO_NMT_EC_TIMEOUT))
    {
        ctx->masterHbLost = (state == CO_NMT_EC_OCCURRED);
        ctx->masterHbEvents++;
        ctx->localStateChange = true; /* pre-op caused by the timeout */
    }
    co_nmt_on_hb(nmt, id, state, reason);
}

/* NMT state of a node, from its heartbeat or boot-up message (master). */
static void canopen_st_ind(co_nmt_t *nmt, co_unsigned8_t id, co_unsigned8_t st, void *data)
{
    canopen_ctx_t *ctx = (canopen_ctx_t *)data;

    if ((ctx != NULL) && (ctx->role == CANOPEN_ROLE_MASTER)
            && (id >= 1U) && (id <= ctx->nodeCount))
    {
        canopen_node_t *n = &ctx->nodes[id - 1U];

        n->seen = true;
        n->st = (uint8_t)(st & (co_unsigned8_t)~CO_NMT_ST_TOGGLE);
        if ((n->st == CO_NMT_ST_BOOTUP) && ctx->started)
        {
            n->bootSeen = true;     /* the node has reset itself */
        }
    }
    co_nmt_on_st(nmt, id, st);
}

/* TPDO of node n received by the master (asynchronous RPDO n). */
static void canopen_rpdo_ind(co_rpdo_t *pdo, co_unsigned32_t ac, const void *ptr,
        size_t n, void *data)
{
    canopen_ctx_t *ctx = (canopen_ctx_t *)data;
    co_unsigned16_t num;
    uint32_t next;
    canopen_fb_t *e;

    if ((ctx == NULL) || (pdo == NULL) || (ac != 0U) || (ptr == NULL))
    {
        return;
    }
    num = co_rpdo_get_num(pdo);
    if ((num < 1U) || (num > ctx->nodeCount))
    {
        return;
    }
    ctx->nodes[num - 1U].fbCount++;
    ctx->rpdoCount++;
    next = (ctx->fbHead + 1U) % CANOPEN_FB_QUEUE;
    if (next == ctx->fbTail)
    {
        ctx->fbOverflows++;
        return;
    }
    e = &ctx->fb[ctx->fbHead];
    e->node = (uint8_t)num;
    (void)memset(e->data, 0, sizeof(e->data));
    (void)memcpy(e->data, ptr, (n > 8U) ? 8U : n);
    ctx->fbHead = next;
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
    if (ctx->role == CANOPEN_ROLE_MASTER)
    {
        /* SYNC sent by the gateway; the synchronous TPDOs (set-points of
         * the nodes) were transmitted before this callback when Operational */
        if (canopen_operational())
        {
            ctx->tpdoCount += ctx->nodeCount;
        }
        return;
    }
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

/*
 * SYNC handling only in NMT Operational. Without dynamic memory
 * (LELY_NO_MALLOC) lely keeps the PDO objects for the lifetime of the NMT
 * service and only starts/stops them, but co_nmt_on_sync() -> co_tpdo_sync()
 * does not check whether a TPDO is stopped: synchronous TPDOs would still be
 * transmitted on SYNC in Pre-operational and Stopped (CiA 301 allows PDOs
 * only in Operational, and the safe reaction relies on it). The SYNC
 * indication of the stack is therefore replaced: outside Operational only the
 * bookkeeping of the gateway runs.
 */
static void canopen_sync_srv_ind(co_sync_t *sync, co_unsigned8_t cnt, void *data)
{
    canopen_ctx_t *ctx = (canopen_ctx_t *)data;
    (void)sync;

    if ((ctx == NULL) || (ctx->nmt == NULL))
    {
        return;
    }
    if (canopen_operational())
    {
        co_nmt_on_sync(ctx->nmt, cnt);      /* PDOs, then canopen_sync_ind */
    }
    else
    {
        canopen_sync_ind(ctx->nmt, cnt, ctx);
    }
}

/* Bus view independent of the NMT state (PDOs are inactive outside
 * Operational, so lely alone would not show that the TPDOs arrive). */
static void canopen_count_rx(uint32_t id)
{
    s_co.rxFrames++;
    s_co.rxLastId = (uint16_t)(id & 0x7FFU);
    if (s_co.role == CANOPEN_ROLE_MASTER)
    {
        uint32_t base = id & 0x780U;
        uint32_t node = id & 0x07FU;

        if ((node >= 1U) && (node <= s_co.nodeCount))
        {
            if (base == 0x700U)
            {
                s_co.nodes[node - 1U].hbRx++;
            }
            else if (base == 0x180U)
            {
                s_co.nodes[node - 1U].pdoRx++;
            }
            else
            {
                /* other services of the node: not counted */
            }
        }
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

        canopen_count_rx(id);
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

/* Master OD for nodeCount nodes: PDOs and heartbeat consumers of the other
 * nodes are disabled before the services are created. */
static void canopen_master_od_setup(co_dev_t *dev, uint8_t nodeCount)
{
    uint8_t n;

    for (n = (uint8_t)(nodeCount + 1U); n <= CANOPEN_NODES_MAX; n++)
    {
        co_unsigned16_t i = (co_unsigned16_t)(n - 1U);

        (void)co_dev_set_val_u32(dev, (co_unsigned16_t)(0x1400U + i), 0x01U,
                0x80000000UL | (0x180UL + n));
        (void)co_dev_set_val_u32(dev, (co_unsigned16_t)(0x1800U + i), 0x01U,
                0x80000000UL | (0x200UL + n));
        (void)co_dev_set_val_u32(dev, 0x1016U, n, 0U);
    }
}

bool canopen_init(canopen_role_t role, uint8_t nodeCount)
{
    struct timespec ts = { 0, 0 };

    (void)memset(&s_co, 0, sizeof(s_co));
    s_co.role = role;
    s_co.nodeCount = (nodeCount > CANOPEN_NODES_MAX) ? CANOPEN_NODES_MAX : nodeCount;

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

    if (role == CANOPEN_ROLE_MASTER)
    {
        s_co.dev = gateway_master_od_init();
        if (s_co.dev == NULL)
        {
            return false;
        }
        (void)co_dev_set_id(s_co.dev, CANOPEN_MASTER_ID);
        canopen_master_od_setup(s_co.dev, s_co.nodeCount);
    }
    else
    {
        s_co.dev = gateway_od_init();
        if (s_co.dev == NULL)
        {
            return false;
        }
        if (co_dev_get_id(s_co.dev) != CANOPEN_NODE_ID)
        {
            (void)co_dev_set_id(s_co.dev, CANOPEN_NODE_ID);
        }
    }

    s_co.nmt = co_nmt_create(s_co.net, s_co.dev);
    if (s_co.nmt == NULL)
    {
        return false;
    }

    co_nmt_set_sync_ind(s_co.nmt, &canopen_sync_ind, &s_co);
    co_nmt_set_hb_ind(s_co.nmt, &canopen_hb_ind, &s_co);
    if (co_nmt_get_sync(s_co.nmt) != NULL)
    {
        /* the SYNC service exists for the lifetime of the NMT service */
        co_sync_set_ind(co_nmt_get_sync(s_co.nmt), &canopen_sync_srv_ind, &s_co);
    }
    if (role == CANOPEN_ROLE_MASTER)
    {
        co_unsigned16_t n;

        co_nmt_set_st_ind(s_co.nmt, &canopen_st_ind, &s_co);
        /* without dynamic memory the PDO services exist for the lifetime
         * of the NMT service; they only start and stop with the NMT state */
        for (n = 1U; n <= s_co.nodeCount; n++)
        {
            co_rpdo_t *pdo = co_nmt_get_rpdo(s_co.nmt, n);

            if (pdo == NULL)
            {
                return false;
            }
            co_rpdo_set_ind(pdo, &canopen_rpdo_ind, &s_co);
        }
    }

    /* Boot-up -> pre-operational (boot-up message, heartbeat started). As
     * the master (0x1F80 = 0x05) lely also sends NMT reset communication to
     * all nodes and stays Pre-operational until canopen_master_start(). */
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
    if (s_co.role == CANOPEN_ROLE_MASTER)
    {
        return;
    }

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

    if (!s_co.ready || !s_co.syncPending || (s_co.role != CANOPEN_ROLE_NODE))
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
    return (s_co.role == CANOPEN_ROLE_NODE) && s_co.leftOpByMaster;
}

uint32_t canopen_entered_op_count(void)
{
    return (s_co.role == CANOPEN_ROLE_NODE) ? s_co.enteredOpCount : 0U;
}

void canopen_enter_preop(void)
{
    if ((s_co.role == CANOPEN_ROLE_NODE) && canopen_operational())
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
    /* master: the nodes are reset as well and have to announce themselves
     * again (boot-up) before the Working State */
    (void)memset(s_co.nodes, 0, sizeof(s_co.nodes));
    s_co.started = false;
    s_co.fbHead = 0U;
    s_co.fbTail = 0U;
    /* boot-up message, communication objects and heartbeat consumers reset */
    (void)co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_RESET_COMM);
}

/* ---- Master role ------------------------------------------------------------ */

bool canopen_is_master(void)
{
    return s_co.ready && (s_co.role == CANOPEN_ROLE_MASTER);
}

void canopen_master_sync_send(void)
{
    struct can_msg msg = CAN_MSG_INIT;

    if (!canopen_is_master())
    {
        return;
    }
    msg.id = 0x080U;
    msg.len = 0U;
    (void)can_net_send(s_co.net, &msg);
    /* synchronous PDOs of this window: the set-points follow the SYNC
     * (Operational only, see canopen_sync_srv_ind) */
    canopen_sync_srv_ind(NULL, 0U, &s_co);
}

void canopen_master_start(void)
{
    uint8_t n;

    if (!canopen_is_master())
    {
        return;
    }
    if (!canopen_operational())
    {
        (void)co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_START);   /* own PDOs on */
    }
    (void)co_nmt_cs_req(s_co.nmt, CO_NMT_CS_START, 0U);   /* all nodes */
    s_co.nmtCommands++;
    for (n = 0U; n < s_co.nodeCount; n++)
    {
        s_co.nodes[n].bootSeen = false;
    }
    s_co.started = true;
    s_co.startUs = hwa_time_now_us();
}

void canopen_master_stop(void)
{
    if (!canopen_is_master())
    {
        return;
    }
    (void)co_nmt_cs_req(s_co.nmt, CO_NMT_CS_STOP, 0U);    /* all nodes */
    s_co.nmtCommands++;
    /* Own NMT state Stopped (PDOs off, heartbeat 0x04). Not Pre-operational:
     * lely runs the master start-up on every entry into Pre-operational,
     * which with LELY_NO_CO_NMT_BOOT commands NMT reset communication of
     * all nodes right after the stop (co_nmt_startup_master). The NMT
     * master commands themselves stay available in Stopped. */
    if (canopen_nmt_state() != CO_NMT_ST_STOP)
    {
        (void)co_nmt_cs_ind(s_co.nmt, CO_NMT_CS_STOP);
    }
    s_co.started = false;
}

void canopen_master_setpoint_set(uint8_t node, const uint8_t data[8])
{
    uint64_t v = 0U;
    uint32_t i;

    if (!canopen_is_master() || (data == NULL) || (node < 1U) || (node > s_co.nodeCount))
    {
        return;
    }
    for (i = 0U; i < 8U; i++)
    {
        v |= (uint64_t)data[i] << (8U * i);
    }
    (void)co_dev_set_val_u64(s_co.dev, 0x2300U, node, v);
}

bool canopen_master_feedback_take(uint8_t *node, uint8_t data[8])
{
    canopen_fb_t *e;

    if (!canopen_is_master() || (s_co.fbTail == s_co.fbHead))
    {
        return false;
    }
    e = &s_co.fb[s_co.fbTail];
    if (node != NULL)
    {
        *node = e->node;
    }
    if (data != NULL)
    {
        (void)memcpy(data, e->data, 8U);
    }
    s_co.nodes[e->node - 1U].fbLast = 0U;
    {
        uint32_t i;
        for (i = 0U; i < 8U; i++)
        {
            s_co.nodes[e->node - 1U].fbLast |= (uint64_t)e->data[i] << (8U * i);
        }
    }
    s_co.fbTail = (s_co.fbTail + 1U) % CANOPEN_FB_QUEUE;
    return true;
}

bool canopen_master_nodes_alive(void)
{
    uint8_t n;

    if (!canopen_is_master())
    {
        return false;
    }
    for (n = 0U; n < s_co.nodeCount; n++)
    {
        if (!s_co.nodes[n].seen || s_co.nodes[n].lost)
        {
            return false;
        }
    }
    return true;
}

bool canopen_master_node_fault(void)
{
    uint8_t n;
    bool graceOver;

    if (!canopen_is_master())
    {
        return false;
    }
    graceOver = s_co.started
            && ((hwa_time_now_us() - s_co.startUs) >= CANOPEN_NODE_START_GRACE_US);
    for (n = 0U; n < s_co.nodeCount; n++)
    {
        const canopen_node_t *nd = &s_co.nodes[n];

        if (nd->lost)
        {
            return true;            /* heartbeat timeout (Table 5.5) */
        }
        if (s_co.started && nd->bootSeen)
        {
            return true;            /* the node has reset itself */
        }
        if (graceOver && (nd->st != CO_NMT_ST_START))
        {
            return true;            /* not Operational although started */
        }
    }
    return false;
}

void canopen_master_node_get(uint8_t node, canopen_node_info_t *info)
{
    if ((info == NULL) || (node < 1U) || (node > CANOPEN_NODES_MAX))
    {
        return;
    }
    info->seen = s_co.nodes[node - 1U].seen;
    info->lost = s_co.nodes[node - 1U].lost;
    info->st = s_co.nodes[node - 1U].st;
    info->fbCount = s_co.nodes[node - 1U].fbCount;
    info->fbLast = s_co.nodes[node - 1U].fbLast;
    info->hbRx = s_co.nodes[node - 1U].hbRx;
    info->pdoRx = s_co.nodes[node - 1U].pdoRx;
    info->setpoint = (s_co.ready && (s_co.role == CANOPEN_ROLE_MASTER))
            ? co_dev_get_val_u64(s_co.dev, 0x2300U, node) : 0U;
}

uint8_t canopen_master_node_count(void)
{
    return s_co.nodeCount;
}

uint32_t canopen_master_fb_overflows(void)
{
    return s_co.fbOverflows;
}

uint32_t canopen_master_nmt_commands(void)
{
    return s_co.nmtCommands;
}

uint32_t canopen_rx_frames(void)
{
    return s_co.rxFrames;
}

uint16_t canopen_rx_last_id(void)
{
    return s_co.rxLastId;
}
