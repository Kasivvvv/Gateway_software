#ifndef CANOPEN_H
#define CANOPEN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** CANopen node-ID used in Stage 3 / set S1 (HB 0x710, TPDO 0x190, RPDO 0x210). */
#define CANOPEN_NODE_ID 0x10u

/**
 * @brief Initialize CANopen stack (lely-core) and enter NMT reset → pre-op.
 *
 * Must be called after hwa_init() so MCAN is ready. Non-blocking thereafter.
 * @return true on success.
 */
bool canopen_init(void);

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

/** Re-initialisation of the communication (T10): NMT reset communication. */
void canopen_reset_comm(void);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_H */
