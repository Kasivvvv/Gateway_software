#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#include "state_machine.h"

/*
 * Error handler (thesis 7.2.8): receives the error indications of both
 * stacks and of the gateway, converts them into one common error code with a
 * class (which selects the exit of the Error State), and issues the error
 * reset when the cause has been removed. It only classifies; the decision on
 * the state stays with the State Machine. Pure logic.
 *
 * Classes (hierarchy of recovery, thesis 7.3):
 *   RECOVERABLE (T9)  master heartbeat timeout, CAN receive buffer overflow,
 *                     conversion buffer overflow. A heartbeat timeout puts the
 *                     node in Pre-operational (safe reaction); it is removed
 *                     when the heartbeat is back and the master has started
 *                     the node again (NMT start), so the Working State is
 *                     never entered with inactive PDOs.
 *   REINIT      (T10) bus-off, EtherCAT working counter fault, failed stack
 *                     initialisation
 *   FATAL       (T11) configuration CRC, self-test; and failed stack
 *                     initialisations (EC_INIT, CAN_INIT) that repeat
 *                     EH_MAX_REINIT times without a stable Working State.
 *                     Network faults (bus-off, EtherCAT working counter) never
 *                     escalate: a hardware reset cannot remove them and would
 *                     disturb the other network; they are retried by T10 (with
 *                     a growing bus-off recovery delay, see supervision.h).
 *
 * Detection: all errors in the Working State (T6); in the Safe State only the
 * critical ones - bus-off, EtherCAT working counter fault, heartbeat timeout
 * of the master (T12); in the Configuration State the failed initialisations
 * and the failure of a running stack - bus-off, loss of the configured
 * EtherCAT network (T5).
 *
 * Escalation: while the machine is in the Error State, an error of a higher
 * class (bus-off, EtherCAT working counter fault) replaces the latched one,
 * e.g. a heartbeat timeout followed by bus-off leads to T10.
 *
 * T10 re-initialises only the communication that failed: reinitCan for
 * bus-off / CAN initialisation, reinitEc for EtherCAT errors.
 */

#define EH_MAX_REINIT     (3U)
#define EH_STABLE_US      (5000000ULL)  /* Working this long resets escalation */

typedef enum
{
    GW_ERR_NONE = 0,
    GW_ERR_CONFIG_CRC,
    GW_ERR_SELFTEST,
    GW_ERR_EC_INIT,
    GW_ERR_CAN_INIT,
    GW_ERR_CAN_BUSOFF,
    GW_ERR_EC_WKC,
    GW_ERR_CAN_HB_TIMEOUT,
    GW_ERR_CAN_RX_OVERFLOW,
    GW_ERR_CONV_OVERFLOW,
    GW_ERR_COUNT
} gw_error_t;

typedef struct
{
    bool configCrcFailed;   /* Startup */
    bool selfTestFailed;    /* Startup */
    bool ecInitFailed;      /* Configuration */
    bool canInitFailed;     /* Configuration */
    bool canBusOff;         /* level; false while its recovery is scheduled */
    bool canHbLost;         /* level: heartbeat of the CANopen master missing */
    bool canOperational;    /* level: CANopen node in NMT Operational */
    bool canRxOverflow;     /* event since the previous step */
    bool ecWkcFault;        /* level */
    bool convOverflow;      /* event since the previous step */
} eh_inputs_t;

typedef struct
{
    gw_error_t     code;        /* active (latched) error */
    gw_err_class_t cls;
    bool           errorReset;  /* cause removed (RECOVERABLE only) */
    uint32_t       reinitCount; /* consecutive REINIT errors (any cause) */
    bool           reinitCan;   /* T10: re-initialise the CANopen side */
    bool           reinitEc;    /* T10: re-initialise the EtherCAT side */
} eh_output_t;

void eh_init(void);

/** Evaluate with the state of the machine before its step in this iteration. */
void eh_step(gw_state_t state, const eh_inputs_t *in, uint64_t nowUs, eh_output_t *out);

const char *eh_error_name(gw_error_t code);

#endif /* ERROR_HANDLER_H */
