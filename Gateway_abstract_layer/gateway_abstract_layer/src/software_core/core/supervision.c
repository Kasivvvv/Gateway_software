#include "supervision.h"

#include <stdio.h>
#include <string.h>

#include "canopen.h"
#include "conversion.h"
#include "ecat_master.h"
#include "hwa_interface.h"

#define SUPV_PDO_BYTES     (8U)
#define SUPV_SOURCE_RPDO   (0x210U)
#define SUPV_SOURCE_ECAT   (0x0001U)

/*
 * Buffer capacity from Eq. (5.12): the CAN->EC buffer is handed over once per
 * EtherCAT cycle, Delta_max = T_EC + Tloop = 2 ms + 0.2 ms, C_min = 47 us
 * -> K >= 47. The same capacity is used for EC->CAN.
 */
#define SUPV_CONV_CAPACITY (48U)

static sm_inputs_t   s_si;
static eh_inputs_t   s_ei;
static sm_output_t   s_so;
static eh_output_t   s_eo;
static supv_status_t s_status;

static uint32_t s_configFailuresAtEnter;
static bool     s_canRecoverPending;
static uint64_t s_canRecoverAtUs;
static bool     s_resumeLatched;
static bool     s_serviceSafe;
static uint32_t s_lastOpEntries;
static uint32_t s_lastCanOverflows;

/* ---- Helpers ---------------------------------------------------------------- */

static void supv_log_transition(const sm_output_t *so, const eh_output_t *eo, uint64_t nowUs)
{
    char line[112];

    (void)snprintf(line, sizeof(line), "SM: T%u %s -> %s err=%s t=%lu us\r\n",
            (unsigned)so->transition, sm_state_name(so->previous),
            sm_state_name(so->state), eh_error_name(eo->code),
            (unsigned long)nowUs);
    (void)hwa_console_write_str(line);
}

/* Delay before the CAN controller re-joins the bus after the n-th
 * consecutive re-initialisation (n >= 1). */
static uint64_t supv_can_recover_delay(uint32_t n)
{
    uint64_t d = SUPV_CAN_RECOVER_MIN_US;

    while ((n > 1U) && (d < SUPV_CAN_RECOVER_MAX_US))
    {
        d *= 2U;
        n--;
    }
    return (d < SUPV_CAN_RECOVER_MAX_US) ? d : SUPV_CAN_RECOVER_MAX_US;
}

/* Self-test of Startup (thesis Table 7.1): DMA memory not cached, PHY found.
 * The configuration CRC check comes with the configuration manager. */
static bool supv_self_test(void)
{
    return hwa_mem_dma_nocache_ok()
            && ((hwa_eth_phy_id() & 0xFFFFFFF0UL) == 0x00221570UL);
}

/* Safe reaction of configuration 3 (thesis Table 5.5). */
static void supv_safe_reaction(void)
{
    conversion_flush();
    ecat_master_outputs_safe();
    ecat_master_request_al_state(ECAT_AL_SAFE_OP);
    canopen_enter_preop();
    hwa_marker_set(HAL_MARKER_STATE);
}

static void supv_on_transition(uint64_t nowUs)
{
    supv_log_transition(&s_so, &s_eo, nowUs);
    s_status.lastTransition = s_so.transition;
    s_status.transitions++;

    if (s_so.previous == GW_ST_CONFIGURATION)
    {
        ecat_master_set_config_enabled(false);
    }
    if (s_so.previous == GW_ST_STARTUP)
    {
        s_si.selfTestDone = false;
    }

    if (s_so.commStop)
    {
        supv_safe_reaction();
    }

    switch (s_so.state)
    {
    case GW_ST_STARTUP:
        hwa_marker_clear(HAL_MARKER_STATE);
        if (s_so.commReset)
        {
            /* T10: re-initialise only the communication that failed */
            if (s_eo.reinitCan)
            {
                char line[64];
                uint64_t delay = supv_can_recover_delay(s_eo.reinitCount);

                /* the controller stays off the bus (bus-off: CCCR.INIT set
                 * by the hardware) until the recovery delay has elapsed */
                s_canRecoverPending = true;
                s_canRecoverAtUs = nowUs + delay;
                (void)snprintf(line, sizeof(line),
                        "SM: T10 re-initialise CANopen in %lu ms\r\n",
                        (unsigned long)(delay / 1000U));
                (void)hwa_console_write_str(line);
            }
            if (s_eo.reinitEc)
            {
                ecat_master_reset();
                (void)hwa_console_write_str("SM: T10 re-initialise EtherCAT\r\n");
            }
            conversion_flush();
        }
        s_si.selfTestOk = supv_self_test();
        s_si.selfTestDone = true;
        s_ei.selfTestFailed = !s_si.selfTestOk;
        break;

    case GW_ST_CONFIGURATION:
        s_configFailuresAtEnter = ecat_master_config_failures();
        ecat_master_set_config_enabled(true);
        break;

    case GW_ST_WORKING:
        /* working state command: conversion permitted, slaves to OP - not
         * while a safe-state request is pending (T7 follows in the next
         * iteration; the slaves stay in SAFE-OP) */
        conversion_flush();
        conversion_fault_clear();
        if (!s_si.safeCause)
        {
            ecat_master_request_al_state(ECAT_AL_OP);
        }
        hwa_marker_clear(HAL_MARKER_STATE);
        break;

    case GW_ST_SAFE:
        s_resumeLatched = false;
        break;

    case GW_ST_INIT:
        if (s_so.hwReset)
        {
            (void)hwa_console_write_str("SM: T11 hardware reset\r\n");
            hwa_system_reset();
        }
        break;

    case GW_ST_ERROR:
    default:
        break;
    }
}

static void supv_console_commands(void)
{
    uint8_t c;

    while (hwa_console_read(&c, 1U) == 1U)
    {
        if ((c == 'S') || (c == 's'))
        {
            s_serviceSafe = true;
            (void)hwa_console_write_str("SVC: safe-state request\r\n");
        }
        else if ((c == 'R') || (c == 'r'))
        {
            s_serviceSafe = false;
            s_resumeLatched = true;
            (void)hwa_console_write_str("SVC: resume\r\n");
        }
        else if (c == '?')
        {
            (void)hwa_console_write_str("SVC: S = safe-state request (T7), R = resume (T8)\r\n");
        }
        else
        {
            /* ignore */
        }
    }
}

/* ---- Data path (Working State only, strategy A) ---------------------------------- */

static void supv_can_to_ec(bool sync, const uint8_t rpdo[SUPV_PDO_BYTES], bool rpdoValid,
        uint64_t nowUs)
{
    conv_msg_t msg;

    if (rpdoValid)
    {
        (void)memset(&msg, 0, sizeof(msg));
        (void)memcpy(msg.data, rpdo, SUPV_PDO_BYTES);
        msg.len = (uint8_t)SUPV_PDO_BYTES;
        msg.source = (uint16_t)SUPV_SOURCE_RPDO;
        (void)conversion_push(CONV_DIR_CAN_TO_EC, &msg, nowUs);   /* t2 */
    }
    if (sync)
    {
        while (conversion_pop(CONV_DIR_CAN_TO_EC, &msg, nowUs))   /* t3 */
        {
            ecat_master_outputs_write(msg.data, msg.len);
        }
    }
}

/* ---- Public ----------------------------------------------------------------- */

void supv_init(void)
{
    conv_config_t conv;

    (void)memset(&conv, 0, sizeof(conv));
    conv.strategy = CONV_STRATEGY_A;
    conv.dir[CONV_DIR_CAN_TO_EC].capacity = SUPV_CONV_CAPACITY;
    conv.dir[CONV_DIR_EC_TO_CAN].capacity = SUPV_CONV_CAPACITY;
    (void)conversion_init(&conv);

    (void)memset(&s_si, 0, sizeof(s_si));
    (void)memset(&s_ei, 0, sizeof(s_ei));
    (void)memset(&s_so, 0, sizeof(s_so));
    (void)memset(&s_eo, 0, sizeof(s_eo));
    (void)memset(&s_status, 0, sizeof(s_status));
    s_resumeLatched = false;
    s_serviceSafe = false;
    s_canRecoverPending = false;
    s_lastOpEntries = canopen_entered_op_count();
    s_lastCanOverflows = hwa_can_rx_overflow_count();

    sm_init();
    eh_init();
    ecat_master_init(ECAT_CYCLE_EXTERNAL);
    /* the configuration brings the slaves to SAFE-OP; OP comes with the
     * working state command (T4/T8/T9) */
    ecat_master_request_al_state(ECAT_AL_SAFE_OP);

    /* T1 is taken in the first iteration: low-level initialisation is done */
    s_si.lowLevelInitDone = true;
}

void supv_step_input(void)
{
    uint64_t now = hwa_time_now_us();
    gw_state_t st = sm_state();
    uint8_t rpdo[SUPV_PDO_BYTES];
    bool rpdoValid = false;
    bool sync;
    uint32_t n;
    bool ecFault;

    supv_console_commands();

    /* --- delayed re-initialisation of the CANopen side (T10) ----------- */
    if (s_canRecoverPending && (now >= s_canRecoverAtUs))
    {
        s_canRecoverPending = false;
        hwa_can_recover();
        canopen_reset_comm();   /* boot-up message, node in Pre-operational */
        (void)hwa_console_write_str("CAN: controller back on the bus\r\n");
    }

    /* --- status of the modules ---------------------------------------- */
    /* A bus-off whose recovery is already scheduled (controller kept off the
     * bus for the backoff delay) is being handled: not a new error. */
    s_ei.canBusOff = hwa_can_is_bus_off() && !s_canRecoverPending;

    /* Configuration: the stacks are ready when the EtherCAT master is
     * configured with running process data and the CAN controller is on the
     * bus. A stack that fails while waiting here - bus-off, or a configured
     * EtherCAT network that stops answering (slave lost or restarted) - fails
     * the configuration (T5 -> T10 re-initialises that stack); repeated failed
     * EtherCAT configurations with slaves present give EC_INIT. */
    ecFault = ecat_master_pd_fault();
    s_si.stacksReady = ecat_master_configured() && !ecFault && !s_canRecoverPending;
    s_ei.ecInitFailed = (st == GW_ST_CONFIGURATION) && !ecat_master_configured()
            && ((ecat_master_config_failures() - s_configFailuresAtEnter)
                >= SUPV_CONFIG_MAX_FAILURES);
    s_ei.ecWkcFault = ecFault;
    s_si.stacksFailed = (st == GW_ST_CONFIGURATION)
            && (s_ei.ecInitFailed || ecFault || s_ei.canBusOff);
    s_ei.canHbLost = canopen_master_hb_lost();
    s_ei.canOperational = canopen_operational();
    n = hwa_can_rx_overflow_count();
    s_ei.canRxOverflow = (n != s_lastCanOverflows);
    s_lastCanOverflows = n;
    s_ei.convOverflow = conversion_fault();
    conversion_fault_clear();

    s_status.safeLink = !ecat_master_link_up();
    s_status.safeNmt = canopen_left_op_by_master();
    s_status.safeService = s_serviceSafe;
    s_si.safeCause = s_status.safeLink || s_status.safeNmt || s_status.safeService;

    n = canopen_entered_op_count();
    if (n != s_lastOpEntries)
    {
        s_lastOpEntries = n;
        s_resumeLatched = true; /* NMT start from the master */
    }
    s_si.resume = s_resumeLatched;

    /* --- Error handler and State Machine (T_dec <= Tloop) -------------- */
    eh_step(st, &s_ei, now, &s_eo);
    s_si.errorClass = s_eo.cls;
    s_si.errorReset = s_eo.errorReset;
    sm_step(&s_si, &s_so);
    if (s_so.transition != 0U)
    {
        supv_on_transition(now);
    }

    s_status.state = s_so.state;
    s_status.error = s_eo.code;
    s_status.errorClass = s_eo.cls;
    s_status.reinitCount = s_eo.reinitCount;
    s_status.masterHbLost = s_ei.canHbLost;

    /* --- SYNC: start of the EtherCAT cycle (configuration 3) ----------- */
    sync = canopen_sync_take(rpdo, &rpdoValid);
    if (s_so.convert)
    {
        supv_can_to_ec(sync, rpdo, rpdoValid && sync, now);
    }
    if (sync)
    {
        /* the cycles go on outside Working too: safe outputs reach the slaves */
        ecat_master_trigger();
    }
}

void supv_step_output(void)
{
    uint8_t in[SUPV_PDO_BYTES];
    uint32_t len = 0U;
    conv_msg_t msg;
    uint64_t now;

    if (!ecat_master_inputs_take(in, SUPV_PDO_BYTES, &len) || !s_so.convert)
    {
        return;
    }
    now = hwa_time_now_us();
    (void)memset(&msg, 0, sizeof(msg));
    (void)memcpy(msg.data, in, len);
    msg.len = (uint8_t)len;
    msg.source = (uint16_t)SUPV_SOURCE_ECAT;
    (void)conversion_push(CONV_DIR_EC_TO_CAN, &msg, now);          /* t2 */
    while (conversion_pop(CONV_DIR_EC_TO_CAN, &msg, now))         /* t3 */
    {
        canopen_tpdo_data_set(msg.data); /* TPDO at the next SYNC: t4 */
    }
}

void supv_status_get(supv_status_t *status)
{
    if (status != NULL)
    {
        *status = s_status;
    }
}
