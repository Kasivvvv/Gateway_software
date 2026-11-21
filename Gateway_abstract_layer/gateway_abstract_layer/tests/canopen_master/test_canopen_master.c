/*
 * Host test: CANopen master role (configuration 4) with the real lely-core
 * build of the firmware (same config.h, static master OD) and canopen.c; the
 * CAN driver and the time base are stubbed. Checks the frames on the bus:
 * NMT reset/start/stop, SYNC followed by the RPDOs of the nodes, reception of
 * the TPDOs of the nodes, heartbeat supervision of the nodes.
 */
#include <stdio.h>
#include <string.h>

#include "canopen.h"
#include "hwa_interface.h"

static int s_fail;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); s_fail++; } } while (0)

/* ---- stubs ---------------------------------------------------------------- */
typedef struct { uint32_t id; uint8_t len; uint8_t d[8]; } frame_t;
static frame_t s_tx[256]; static int s_ntx;
static frame_t s_rx[64]; static int s_rxHead, s_rxTail;
static uint64_t s_nowUs;

void hwa_can_tx(uint32_t id, uint32_t sub_id, uint8_t *data, uint8_t len)
{
    (void)sub_id;
    if (s_ntx < 256) { s_tx[s_ntx].id = id; s_tx[s_ntx].len = len; memcpy(s_tx[s_ntx].d, data, len); s_ntx++; }
}
void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len)
{
    if (s_rxTail == s_rxHead) return;
    *id = s_rx[s_rxTail].id; *sub_id = 0; *len = s_rx[s_rxTail].len;
    memcpy(data, s_rx[s_rxTail].d, s_rx[s_rxTail].len);
    s_rxTail = (s_rxTail + 1) % 64;
}
uint32_t hwa_get_number_of_massages(void) { return (uint32_t)((s_rxHead - s_rxTail + 64) % 64); }
uint64_t hwa_time_now_us(void) { return s_nowUs; }

static void rx(uint32_t id, uint8_t len, const uint8_t *d)
{
    s_rx[s_rxHead].id = id; s_rx[s_rxHead].len = len;
    if (len) memcpy(s_rx[s_rxHead].d, d, len);
    s_rxHead = (s_rxHead + 1) % 64;
}
static void hb(uint8_t node, uint8_t st) { rx(0x700U + node, 1, &st); }
static int find(uint32_t id, int from) { for (int i = from; i < s_ntx; i++) if (s_tx[i].id == id) return i; return -1; }
static void run_ms(uint32_t ms, bool hb1, bool hb2, uint8_t st)
{
    for (uint32_t t = 0; t < ms * 5U; t++)
    {
        s_nowUs += 200U;
        if ((s_nowUs % 100000U) == 0U) { if (hb1) hb(1, st); if (hb2) hb(2, st); }
        canopen_step();
    }
}

int main(void)
{
    uint8_t node, data[8];
    int i;

    /* init: master boot-up, NMT reset communication of all nodes */
    CHECK(canopen_init(CANOPEN_ROLE_MASTER, 2U));
    CHECK(canopen_is_master());
    canopen_step();
    i = find(0x000U, 0);
    CHECK(i >= 0 && s_tx[i].len == 2 && s_tx[i].d[0] == 0x82 && s_tx[i].d[1] == 0x00);
    CHECK(find(0x77FU, 0) >= 0);                       /* boot-up of the master */
    CHECK(canopen_nmt_state() == 0x7F);                /* stays Pre-operational */
    CHECK(!canopen_master_nodes_alive());

    /* nodes announce themselves */
    { uint8_t b = 0; rx(0x701, 1, &b); rx(0x702, 1, &b); }
    run_ms(150, true, true, 0x7F);
    CHECK(canopen_master_nodes_alive() && !canopen_master_node_fault());

    /* heartbeat of the master every 100 ms */
    { int n = 0; s_ntx = 0; run_ms(1000, true, true, 0x7F); for (i = 0; i < s_ntx; i++) n += (s_tx[i].id == 0x77FU); CHECK(n >= 9 && n <= 11); printf("master heartbeats in 1 s: %d\n", n); }

    /* start: own Operational + NMT start all */
    s_ntx = 0;
    canopen_master_start();
    i = find(0x000U, 0);
    CHECK(i >= 0 && s_tx[i].d[0] == 0x01 && s_tx[i].d[1] == 0x00);
    CHECK(canopen_operational());

    /* SYNC then the synchronous RPDOs 0x201/0x202 with the set-points */
    { uint8_t sp1[8] = {1,2,3,4,5,6,7,8}, sp2[8] = {9,9,9,9,9,9,9,9};
      canopen_master_setpoint_set(1, sp1); canopen_master_setpoint_set(2, sp2); }
    s_ntx = 0;
    canopen_master_sync_send();
    CHECK(s_ntx >= 3 && s_tx[0].id == 0x080U && s_tx[0].len == 0);
    i = find(0x201U, 0); CHECK(i > 0 && s_tx[i].len == 8 && s_tx[i].d[0] == 1 && s_tx[i].d[7] == 8);
    i = find(0x202U, 0); CHECK(i > 0 && s_tx[i].d[0] == 9);
    CHECK(find(0x203U, 0) < 0);                        /* node 3 not configured */

    /* TPDO of node 2 received asynchronously */
    { uint8_t f[8] = {0xAA,0xBB,0,0,0,0,0,0x11}; rx(0x182, 8, f); }
    canopen_step();
    CHECK(canopen_master_feedback_take(&node, data) && node == 2 && data[0] == 0xAA && data[7] == 0x11);
    CHECK(!canopen_master_feedback_take(&node, data));

    /* nodes Operational in their heartbeat: no fault after the grace time */
    run_ms(500, true, true, 0x05);
    CHECK(!canopen_master_node_fault());

    /* node 2 stays silent: heartbeat timeout (consumer 250 ms) */
    run_ms(400, true, false, 0x05);
    CHECK(canopen_master_node_fault() && !canopen_master_nodes_alive());
    s_ntx = 0;
    canopen_master_stop();                             /* safe reaction */
    i = find(0x000U, 0);
    CHECK(i >= 0 && s_tx[i].d[0] == 0x02 && s_tx[i].d[1] == 0x00);
    CHECK(find(0x000U, i + 1) < 0);                    /* no reset comm after the stop */
    CHECK(canopen_nmt_state() == 0x04);                /* own state Stopped */
    s_ntx = 0; run_ms(200, true, false, 0x05);
    i = find(0x77FU, 0);
    CHECK(i >= 0 && s_tx[i].len == 1 && s_tx[i].d[0] == 0x04);   /* own heartbeat */
    s_ntx = 0; canopen_master_sync_send();
    CHECK(s_ntx == 1 && s_tx[0].id == 0x080U);          /* no set-points when stopped */

    /* node 2 back (stopped: 0x04) -> alive, no fault while not started */
    run_ms(300, true, true, 0x04);
    CHECK(canopen_master_nodes_alive() && !canopen_master_node_fault());

    /* started again but node 1 stays Pre-operational -> fault after grace */
    canopen_master_start();
    run_ms(200, true, true, 0x7F);
    CHECK(!canopen_master_node_fault());               /* still within the grace */
    run_ms(200, true, true, 0x7F);
    CHECK(canopen_master_node_fault());

    /* boot-up of a node while started -> fault */
    canopen_master_stop(); canopen_master_start();
    run_ms(100, true, true, 0x05);
    { uint8_t b = 0; rx(0x701, 1, &b); } canopen_step();
    CHECK(canopen_master_node_fault());

    /* reset communication (T10): nodes must announce themselves again */
    canopen_reset_comm();
    canopen_step();
    CHECK(!canopen_master_nodes_alive());

    /* node role (configuration 3): no TPDO 0x190 on SYNC outside Operational */
    CHECK(canopen_init(CANOPEN_ROLE_NODE, 0U));
    canopen_step();
    s_ntx = 0; rx(0x080, 0, NULL); canopen_step();
    CHECK(canopen_nmt_state() == 0x7F && find(0x190U, 0) < 0);
    { uint8_t c[2] = {0x01, 0x10}; rx(0x000, 2, c); } canopen_step();
    s_ntx = 0; rx(0x080, 0, NULL); canopen_step();
    CHECK(canopen_operational() && find(0x190U, 0) >= 0);
    { uint8_t c[2] = {0x80, 0x10}; rx(0x000, 2, c); } canopen_step();
    s_ntx = 0; rx(0x080, 0, NULL); canopen_step();
    CHECK(canopen_nmt_state() == 0x7F && find(0x190U, 0) < 0);
    { bool v = true; uint8_t d[8]; CHECK(canopen_sync_take(d, &v) && !v); }

    printf("RESULT: %s\n", s_fail ? "FAIL" : "PASS");
    return s_fail ? 1 : 0;
}
