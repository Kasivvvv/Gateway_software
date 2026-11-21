#ifndef CANOPEN_H
#define CANOPEN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** CANopen node-ID in the node role, set S1 (HB 0x710, TPDO 0x190, RPDO 0x210). */
#define CANOPEN_NODE_ID 0x10u
/** CANopen node-ID in the master role (heartbeat 0x77F). */
#define CANOPEN_MASTER_ID 0x7Fu
/** Nodes supported in the master role (set S7). */
#define CANOPEN_NODES_MAX 7u
/** After NMT start: time within which every node must report Operational in
 *  its heartbeat (above the consumer time of 250 ms). */
#define CANOPEN_NODE_START_GRACE_US (300000ULL)

typedef enum
{
    CANOPEN_ROLE_NODE = 0,      /* configurations 1 and 3 */
    CANOPEN_ROLE_MASTER         /* configurations 2 and 4 */
} canopen_role_t;

typedef struct
{
    bool     seen;
    bool     lost;
    uint8_t  st;
    uint32_t fbCount;
    uint64_t fbLast;            /* last TPDO of the node taken by the core */
    uint64_t setpoint;          /* value of its next RPDO */
    uint32_t hbRx;              /* its heartbeat frames seen on the bus */
    uint32_t pdoRx;             /* its TPDO frames seen (any NMT state) */
} canopen_node_info_t;

/**
 * @brief Initialize CANopen stack (lely-core) and enter NMT reset → pre-op.
 *
 * Must be called after hwa_init() so MCAN is ready. Non-blocking thereafter.
 * nodeCount is used in the master role only (nodes 1..nodeCount).
 * @return true on success.
 */
bool canopen_init(canopen_role_t role, uint8_t nodeCount);

/**
 * @brief One non-blocking CANopen step for the superloop Working state.
 *
 * Feeds RX frames from HAL into the stack, advances network time by one
 * Tloop, and processes timers (heartbeat, etc.).
 */
void canopen_step(void);

/** Current NMT state (CO_NMT_ST_* bits without toggle). */
uint8_t canopen_nmt_state(void);

/** Number of SYNC frames received since init. */
uint32_t canopen_sync_count(void);

/** Number of TPDO frames emitted since init. */
uint32_t canopen_tpdo_count(void);

/** Number of RPDO frames received since init. */
uint32_t canopen_rpdo_count(void);

/** Last RPDO payload as little-endian u64 (object 0x2100). */
uint64_t canopen_rpdo_last(void);

/** Value currently mapped into the TPDO (object 0x2000, data from EtherCAT). */
uint64_t canopen_tpdo_data(void);

/**
 * One SYNC received since the last call (consumed by this call).
 * rpdo receives the RPDO data actuated at this SYNC (object 0x2100, 8 bytes,
 * little endian); rpdoValid is false outside NMT Operational (PDOs inactive).
 */
bool canopen_sync_take(uint8_t rpdo[8], bool *rpdoValid);

/** Data for the next synchronous TPDO 0x190 (object 0x2000, 8 bytes). */
void canopen_tpdo_data_set(const uint8_t data[8]);

/* ---- Supervision (thesis 7.3, Table 5.5, CAN node role) ---------------- */

bool canopen_operational(void);

/** Heartbeat of the CANopen master (0x77F, consumer 0x1016) timed out. */
bool canopen_master_hb_lost(void);

/** Latched safe-state request of the master: the node left Operational on a
 *  command of the master (NMT stop / enter pre-operational) or was stopped
 *  from any state; cleared when it becomes Operational again. */
bool canopen_left_op_by_master(void);

/** Number of entries into Operational (NMT start received). */
uint32_t canopen_entered_op_count(void);

/** Safe reaction of the node: own NMT state -> Pre-operational. */
void canopen_enter_preop(void);

/** Re-initialisation of the communication (T10): NMT reset communication
 *  (master: of the nodes as well). */
void canopen_reset_comm(void);

/* ---- Master role (thesis 7.2.4, Table 5.5) ------------------------------ */

bool canopen_is_master(void);

/** Transmit SYNC 0x080 now (cycle generator) and the synchronous TPDOs. */
void canopen_master_sync_send(void);

/** Working state command: own NMT Operational, NMT start of all nodes. */
void canopen_master_start(void);

/** Safe reaction: NMT stop of all nodes (first), own NMT state Stopped. */
void canopen_master_stop(void);

/** Set-point of node 1..N for its next RPDO (0x200+node). */
void canopen_master_setpoint_set(uint8_t node, const uint8_t data[8]);

/** Next TPDO received from a node (0x180+node), in order of reception. */
bool canopen_master_feedback_take(uint8_t *node, uint8_t data[8]);

/** Every node has announced itself and none has a heartbeat timeout. */
bool canopen_master_nodes_alive(void);

/** Failure of a node: heartbeat timeout; or, once started, a boot-up or a
 *  state other than Operational after CANOPEN_NODE_START_GRACE_US. */
bool canopen_master_node_fault(void);

void canopen_master_node_get(uint8_t node, canopen_node_info_t *info);
uint8_t canopen_master_node_count(void);
uint32_t canopen_master_fb_overflows(void);
uint32_t canopen_master_nmt_commands(void);

/* ---- Diagnostics -------------------------------------------------------- */

/** Frames handed to the stack since init. */
uint32_t canopen_rx_frames(void);
/** Identifier of the last frame handed to the stack. */
uint16_t canopen_rx_last_id(void);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_H */
