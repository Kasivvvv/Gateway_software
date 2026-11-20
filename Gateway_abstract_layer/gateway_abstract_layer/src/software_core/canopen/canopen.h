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

/** TX sequence counter currently mapped into TPDO (object 0x2000). */
uint64_t canopen_tpdo_seq(void);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_H */
