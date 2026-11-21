#ifndef SUPERVISION_H
#define SUPERVISION_H

#include <stdint.h>
#include <stdbool.h>

#include "state_machine.h"
#include "error_handler.h"

/*
 * Supervision part of the Software Core (thesis 7.2.2): collects the status
 * of the modules, evaluates the Error handler and the State Machine once per
 * iteration, executes the commands (working state / communication stop /
 * communication reset / hardware reset) and moves the data between the
 * stacks and the Conversion module in the Working State.
 *
 * Role configurations (gw_config.h, fixed until the configuration manager,
 * stage 9):
 *   3  EtherCAT master + CANopen node 0x10, EtherCAT cycle started by SYNC;
 *   4  EtherCAT master + CANopen master 0x7F, EtherCAT cycle and SYNC from
 *      the cycle generator (fixed phase, R5); the nodes must be present
 *      (heartbeat) before T4; T7 only by link loss or service command, T8 only
 *      by the service command (no external master).
 *
 * Transitions T1..T11 of thesis Table 7.2 plus T12 Safe -> Error (critical
 * error while in the Safe State).
 *
 * Safe reaction on entering the Error or the Safe State (thesis Table 5.5):
 *   conversion stopped and buffers flushed;
 *   EtherCAT master: safe (zero) outputs in the next frame, slaves -> SAFE-OP;
 *   CANopen node: own NMT state -> Pre-operational (seen in its heartbeat);
 *   CANopen master: NMT stop of all nodes, own state Pre-operational;
 *   then MARK_STATE is set (end of the time to the safe state, HIL-06).
 *
 * Service commands on the console: 'S' safe-state request (T7),
 * 'R' resume (T8).
 */

/* Configuration -> T5: failed EtherCAT configuration attempts with slaves
 * present. Without slaves the gateway waits in the Configuration state. */
#define SUPV_CONFIG_MAX_FAILURES (3U)

/* Bus-off recovery (T10 of the CANopen side): the controller stays off the
 * bus for a delay that doubles with each consecutive re-initialisation
 * (100 ms, 200 ms, ... up to 2 s) and is back to the minimum after a stable
 * Working State. A persistent bus fault (e.g. CANH-CANL short) is retried at
 * this rate instead of a reset loop; the gateway waits in Configuration. */
#define SUPV_CAN_RECOVER_MIN_US (100000ULL)
#define SUPV_CAN_RECOVER_MAX_US (2000000ULL)

typedef struct
{
    gw_state_t     state;
    uint8_t        lastTransition;
    uint32_t       transitions;
    gw_error_t     error;
    gw_err_class_t errorClass;
    uint32_t       reinitCount;
    bool           safeLink;     /* safe cause: Ethernet link down */
    bool           safeNmt;      /* safe cause: master left Operational */
    bool           safeService;  /* safe cause: service command */
    bool           masterHbLost;
    bool           nodeFault;    /* CANopen master: failure of a node */
} supv_status_t;

/** cfgOk: result of gw_config_init(), part of the self-test (T2/T3). */
void supv_init(bool cfgOk);

/** First call of the iteration: cycle generator of configuration 4 (SYNC). */
void supv_step_cycle(void);

/** Before ecat_master_step(): supervision, then CAN -> EtherCAT. */
void supv_step_input(void);

/** After ecat_master_step(): EtherCAT -> CAN. */
void supv_step_output(void);

void supv_status_get(supv_status_t *status);

#endif /* SUPERVISION_H */
