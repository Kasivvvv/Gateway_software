#include "ecat_master.h"

#include <stdio.h>
#include <string.h>

#include "ethercat.h"
#include "oshw.h"
#include "osal_port.h"

#include "hwa_interface.h"

/* Consecutive probes that must see slaves before configuring. */
#define ECAT_PROBE_CONFIRM        (3U)
/* Bounded waits inside the Configuration state. */
#define ECAT_STATE_TIMEOUT_US     (100000)
/* Hard limit of one blocking configuration (agreed rule: SOEM may block only
 * in the Configuration state, with a time limit). */
#define ECAT_CONFIG_LIMIT_US      (2000000U)
/* Pause after a failed configuration before probing again (cycles). */
#define ECAT_RETRY_CYCLES         (1000U)
#define ECAT_AL_STATE_MASK        (0x0FU)

/* ---- SOEM context (EC_VER2: no global default context in the library) ----- */

static ecx_portt        s_port;
static ec_slavet        s_slaves[ECAT_MAX_SLAVES + 1U];
static int              s_slaveCount;
static ec_groupt        s_groups[1];
static uint8            s_esibuf[EC_MAXEEPBUF];
static uint32           s_esimap[EC_MAXEEPBITMAP];
static ec_eringt        s_elist;
static ec_idxstackT     s_idxstack;
static boolean          s_ecatError;
static int64            s_dcTime;
static ec_SMcommtypet   s_smCommType[EC_MAX_MAPT];
static ec_PDOassignt    s_pdoAssign[EC_MAX_MAPT];
static ec_PDOdesct      s_pdoDesc[EC_MAX_MAPT];
static ec_eepromSMt     s_eepSM;
static ec_eepromFMMUt   s_eepFMMU;

static ecx_contextt s_ctx =
{
    &s_port,                       /* port */
    &s_slaves[0],                  /* slavelist */
    &s_slaveCount,                 /* slavecount */
    (int)(ECAT_MAX_SLAVES + 1U),   /* maxslave */
    &s_groups[0],                  /* grouplist */
    1,                             /* maxgroup */
    &s_esibuf[0],                  /* esibuf */
    &s_esimap[0],                  /* esimap */
    0,                             /* esislave */
    &s_elist,                      /* elist */
    &s_idxstack,                   /* idxstack */
    &s_ecatError,                  /* ecaterror */
    0,                             /* DCtO */
    0,                             /* DCl */
    &s_dcTime,                     /* DCtime */
    &s_smCommType[0],              /* SMcommtype */
    &s_pdoAssign[0],               /* PDOassign */
    &s_pdoDesc[0],                 /* PDOdesc */
    &s_eepSM,                      /* eepSM */
    &s_eepFMMU,                    /* eepFMMU */
    NULL,                          /* FOEhook */
    NULL,                          /* EOEhook */
    0                              /* manualstatechange */
};

static uint8 s_iomap[ECAT_IOMAP_SIZE];

/* ---- State ------------------------------------------------------------------ */

static ecat_master_stats_t s_st;
static ecat_cycle_mode_t s_mode;
static bool     s_nicReady;
static bool     s_configEnabled;
static bool     s_configured;
static bool     s_linkUp;
static uint32_t s_ticks;
static bool     s_trigger;
static int      s_probeIdx;
static uint32_t s_probeHits;
static uint32_t s_retryCycles;
static bool     s_pdPending;
static bool     s_inputsFresh;
static int      s_auxIdx;
static bool     s_auxIsRead;
static bool     s_auxWriteTurn;
static uint32_t s_graceCycles;     /* after a configuration or a link-up */
static uint32_t s_opGraceCycles;   /* after an OP request (SAFE-OP -> OP) */
static uint32_t s_noTriggerTicks;   /* Tloop ticks since the last trigger */
static uint32_t s_syncPeriodTicks;  /* measured SYNC period, 0 = unknown */

static void ecat_log(const char *text)
{
    (void)hwa_console_write_str(text);
}

static void ecat_set_state(ecat_state_t st)
{
    char line[48];

    if (s_st.state != st)
    {
        s_st.state = st;
        (void)snprintf(line, sizeof(line), "ECAT: state %s\r\n",
                ecat_master_state_name(st));
        ecat_log(line);
    }
}

/* Release every frame buffer in use (link loss, reset). */
static void ecat_update_target_wkc(void)
{
    s_st.targetWkc = (s_st.alTarget == EC_STATE_OPERATIONAL)
            ? s_st.expectedWkc
            : (int32_t)s_groups[0].inputsWKC;
}

static void ecat_release_buffers(void)
{
    int i;

    for (i = 0; i < EC_MAXBUF; i++)
    {
        s_port.rxbufstat[i] = EC_BUF_EMPTY;
    }
    s_idxstack.pushed = 0U;
    s_idxstack.pulled = 0U;
    s_probeIdx = -1;
    s_auxIdx = -1;
    s_pdPending = false;
    s_inputsFresh = false;
}

/* ---- PROBE: non-blocking BRD of the ESC type register ----------------------- */

static void ecat_probe_collect(void)
{
    int wkc;

    if (s_probeIdx < 0)
    {
        return;
    }

    /* timeout 0: one look at the RX ring, never waits */
    wkc = ecx_waitinframe(&s_port, s_probeIdx, 0);
    ecx_setbufstat(&s_port, s_probeIdx, EC_BUF_EMPTY);
    s_probeIdx = -1;

    s_st.probeWkc = (wkc > EC_NOFRAME) ? wkc : -1;
    if (wkc > EC_NOFRAME)
    {
        s_st.probeRx++;
    }

    if ((wkc > 0) && (wkc <= (int)ECAT_MAX_SLAVES))
    {
        s_probeHits++;
    }
    else
    {
        s_probeHits = 0U;
    }
}

static void ecat_probe_send(void)
{
    uint16 w = 0U;
    int idx = ecx_getindex(&s_port);

    (void)ecx_setupdatagram(&s_port, &(s_port.txbuf[idx]), EC_CMD_BRD,
            (uint8)idx, 0x0000U, ECT_REG_TYPE, sizeof(w), &w);
    if (ecx_outframe_red(&s_port, idx) < 0)
    {
        ecx_setbufstat(&s_port, idx, EC_BUF_EMPTY);
        return;
    }
    s_probeIdx = idx;
    s_st.probeTx++;
}

/* ---- CONFIG: blocking SOEM configuration (Configuration state only) --------- */

static void ecat_report_slaves(void)
{
    char line[96];
    int i;

    for (i = 1; i <= s_slaveCount; i++)
    {
        (void)snprintf(line, sizeof(line),
                "ECAT: slave %d '%s' out=%luB in=%luB state=0x%02X\r\n",
                i, s_slaves[i].name, (unsigned long)s_slaves[i].Obytes,
                (unsigned long)s_slaves[i].Ibytes, s_slaves[i].state);
        ecat_log(line);
    }
}

static void ecat_config_failed(const char *what, int value, uint64_t t0)
{
    char line[96];

    osal_deadline_set(0U);
    s_st.lastConfigUs = (uint32_t)(hwa_time_now_us() - t0);
    (void)snprintf(line, sizeof(line), "ECAT: %s (%d) after %lu us, retry in %u cycles\r\n",
            what, value, (unsigned long)s_st.lastConfigUs,
            (unsigned)ECAT_RETRY_CYCLES);
    ecat_log(line);
    s_st.configFailures++;
    s_probeHits = 0U;
    s_retryCycles = ECAT_RETRY_CYCLES;
    ecat_set_state(ECAT_ST_PROBE);
}

static void ecat_configure(void)
{
    char line[96];
    uint64_t t0 = hwa_time_now_us();
    int n;
    int iomap;

    s_st.configs++;
    s_st.slaves = 0;
    osal_deadline_set(t0 + ECAT_CONFIG_LIMIT_US);

    /* The probe guarantees 1..ECAT_MAX_SLAVES slaves: SOEM's own check uses
     * EC_MAXSLAVE (200), larger than s_slaves[]. */
    n = ecx_config_init(&s_ctx, FALSE);
    if ((n <= 0) || (n > (int)ECAT_MAX_SLAVES) || osal_deadline_hit())
    {
        ecat_config_failed(osal_deadline_hit() ? "config_init time limit"
                                               : "config_init failed", n, t0);
        return;
    }

    iomap = ecx_config_map_group(&s_ctx, s_iomap, 0);
    if ((iomap < 0) || ((uint32_t)iomap > ECAT_IOMAP_SIZE) || osal_deadline_hit())
    {
        ecat_config_failed(osal_deadline_hit() ? "config_map time limit"
                                               : "IOmap too large", iomap, t0);
        return;
    }

    (void)ecx_configdc(&s_ctx);
    (void)ecx_statecheck(&s_ctx, 0, EC_STATE_SAFE_OP, ECAT_STATE_TIMEOUT_US);
    if (osal_deadline_hit())
    {
        ecat_config_failed("SAFE-OP time limit", 0, t0);
        return;
    }

    s_st.slaves = n;
    s_st.iomapBytes = (uint32_t)iomap;
    s_st.expectedWkc = (int32_t)((s_groups[0].outputsWKC * 2) + s_groups[0].inputsWKC);
    ecat_update_target_wkc();

    /* One process data exchange with valid (zero) outputs, then request the
     * AL state the gateway wants: OP only on the working state command, so a
     * re-configuration during a safe reaction leaves the slaves in SAFE-OP. */
    (void)memset(s_iomap, 0, sizeof(s_iomap));
    (void)ecx_send_processdata(&s_ctx);
    (void)ecx_receive_processdata(&s_ctx, EC_TIMEOUTRET);
    if (s_st.alTarget == EC_STATE_OPERATIONAL)
    {
        s_slaves[0].state = EC_STATE_OPERATIONAL;
        (void)ecx_writestate(&s_ctx, 0);
    }

    osal_deadline_set(0U);
    s_st.lastConfigUs = (uint32_t)(hwa_time_now_us() - t0);
    (void)snprintf(line, sizeof(line),
            "ECAT: %d slave(s), IOmap %d B, expected WKC %ld, config %lu us\r\n",
            n, iomap, (long)s_st.expectedWkc, (unsigned long)s_st.lastConfigUs);
    ecat_log(line);
    ecat_report_slaves();

    ecat_release_buffers();
    s_configured = true;
    s_st.missRun = 0U;
    s_graceCycles = ECAT_OP_GRACE_CYCLES;
    s_trigger = false;
    ecat_set_state(ECAT_ST_OP);
}

/* ---- OP: auxiliary AL status / control datagram ----------------------------- */

static uint16_t rd16le(const uint8 *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static void ecat_aux_collect(void)
{
    int wkc;

    if (s_auxIdx < 0)
    {
        return;
    }
    wkc = ecx_waitinframe(&s_port, s_auxIdx, 0);
    if (s_auxIsRead)
    {
        if (wkc > EC_NOFRAME)
        {
            s_st.alStatus = rd16le(&s_port.rxbuf[s_auxIdx][EC_HEADERSIZE]);
            s_st.alWkc = wkc;
        }
        else
        {
            s_st.alWkc = 0;
        }
    }
    ecx_setbufstat(&s_port, s_auxIdx, EC_BUF_EMPTY);
    s_auxIdx = -1;
}

static void ecat_aux_send(void)
{
    uint16 value = 0U;
    uint8 cmd = EC_CMD_BRD;
    uint16 ado = ECT_REG_ALSTAT;
    uint16_t st = s_st.alStatus;
    int idx;

    /* Every other cycle, if the slaves are not in the target state, write the
     * AL control: acknowledge an error first, then request the state. */
    s_auxWriteTurn = !s_auxWriteTurn;
    if (s_auxWriteTurn && (s_st.alWkc > 0)
            && (((st & ECAT_AL_STATE_MASK) != s_st.alTarget)
                || ((st & EC_STATE_ERROR) != 0U)))
    {
        value = ((st & EC_STATE_ERROR) != 0U)
                ? (uint16)((st & ECAT_AL_STATE_MASK) | EC_STATE_ACK)
                : (uint16)s_st.alTarget;
        cmd = EC_CMD_BWR;
        ado = ECT_REG_ALCTL;
    }

    idx = ecx_getindex(&s_port);
    (void)ecx_setupdatagram(&s_port, &(s_port.txbuf[idx]), cmd, (uint8)idx,
            0x0000U, ado, sizeof(value), &value);
    if (ecx_outframe_red(&s_port, idx) < 0)
    {
        ecx_setbufstat(&s_port, idx, EC_BUF_EMPTY);
        return;
    }
    s_auxIdx = idx;
    s_auxIsRead = (cmd == EC_CMD_BRD);
}

/* ---- OP: cyclic process data, non-blocking --------------------------------- */

/* All datagrams of the current process data cycle are back (not yet completed). */
static bool ecat_pd_all_received(void)
{
    uint8 i;

    if (s_idxstack.pushed == 0U)
    {
        return false;
    }
    for (i = s_idxstack.pulled; i < s_idxstack.pushed; i++)
    {
        if (s_port.rxbufstat[s_idxstack.idx[i]] != EC_BUF_RCVD)
        {
            return false;
        }
    }
    return true;
}

/*
 * Complete the pending process data cycle. Without 'force' only when every
 * frame is already back (polled once per Tloop, so the inputs are available
 * one iteration after they arrive); with 'force' at the start of the next
 * cycle, which counts a frame that is still missing as a lost cycle.
 */
static void ecat_pd_collect(bool force)
{
    int wkc;

    if (!s_pdPending)
    {
        return;
    }
    nicdrv_rx_poll(&s_port);
    if (!force && !ecat_pd_all_received())
    {
        return;
    }

    wkc = ecx_receive_processdata(&s_ctx, 0);
    s_pdPending = false;
    s_st.lastWkc = wkc;
    if (wkc >= s_st.targetWkc)
    {
        s_st.wkcOk++;
        s_st.missRun = 0U;
        s_graceCycles = 0U; /* slaves are in the target state */
        s_opGraceCycles = 0U;
        if ((s_slaves[0].Ibytes > 0U) && (s_slaves[0].inputs != NULL))
        {
            s_inputsFresh = true;
            s_st.inputsUpdates++;
            s_st.in0 = s_slaves[0].inputs[0];
        }
    }
    else
    {
        s_st.wkcMiss++;
        s_st.missRun++;
    }
}

static void ecat_cycle(void)
{
    ecat_pd_collect(true);
    ecat_aux_collect();
    if (s_graceCycles > 0U)
    {
        s_graceCycles--;
    }
    if (s_opGraceCycles > 0U)
    {
        s_opGraceCycles--;
    }

    (void)ecx_send_processdata(&s_ctx);
    s_pdPending = true;
    s_st.cycles++;
    ecat_aux_send();
}

/* ---- Public ----------------------------------------------------------------- */

void ecat_master_init(ecat_cycle_mode_t mode)
{
    (void)memset(&s_st, 0, sizeof(s_st));
    s_st.state = ECAT_ST_WAIT_LINK;
    s_st.probeWkc = -1;
    s_st.alTarget = EC_STATE_OPERATIONAL;
    s_mode = mode;
    s_nicReady = false;
    s_configEnabled = false;
    s_configured = false;
    s_linkUp = false;
    s_ticks = 0U;
    s_trigger = false;
    s_probeHits = 0U;
    s_retryCycles = 0U;
    s_auxWriteTurn = false;
    s_graceCycles = 0U;
    s_opGraceCycles = 0U;
    s_noTriggerTicks = 0U;
    s_syncPeriodTicks = 0U;
    ecat_release_buffers();
}

void ecat_master_set_config_enabled(bool enabled)
{
    s_configEnabled = enabled;
}

void ecat_master_reset(void)
{
    s_configured = false;
    s_st.slaves = 0;
    s_st.missRun = 0U;
    s_probeHits = 0U;
    s_retryCycles = 0U;
    ecat_release_buffers();
    ecat_set_state(ECAT_ST_WAIT_LINK);
}

void ecat_master_trigger(void)
{
    s_trigger = true;
}

void ecat_master_step(void)
{
    hwa_eth_link_t link;
    bool cycleStart;

    hwa_eth_link_get(&link);
    s_linkUp = link.up;

    if (!link.up)
    {
        if (s_st.state != ECAT_ST_WAIT_LINK)
        {
            ecat_release_buffers();
            s_probeHits = 0U;
            ecat_set_state(ECAT_ST_WAIT_LINK);
        }
        s_trigger = false;
        return;
    }

    if (!s_nicReady)
    {
        s_nicReady = (ecx_init(&s_ctx, OSHW_IFNAME) > 0);
        if (!s_nicReady)
        {
            ecat_set_state(ECAT_ST_ERROR);
            return;
        }
    }

    /* Cycle start: external trigger (SYNC, configuration 3) in OP, own timer
     * otherwise (always for probing, and when the SYNC is lost). */
    s_ticks++;
    if (s_trigger)
    {
        uint32_t sample = s_noTriggerTicks + 1U;

        /* averaged period (1/8 weight) against single short or long gaps */
        s_syncPeriodTicks = (s_syncPeriodTicks == 0U) ? sample
                : (((s_syncPeriodTicks * 7U) + sample) / 8U);
        s_noTriggerTicks = 0U;
    }
    else if (s_noTriggerTicks < ECAT_SYNC_LOSS_MAX_TICKS)
    {
        s_noTriggerTicks++;
    }
    else
    {
        /* saturated */
    }
    {
        uint32_t limit = (s_syncPeriodTicks == 0U) ? ECAT_SYNC_LOSS_MAX_TICKS
                : (s_syncPeriodTicks * ECAT_SYNC_LOSS_PERIODS);

        if (limit < ECAT_SYNC_LOSS_MIN_TICKS)
        {
            limit = ECAT_SYNC_LOSS_MIN_TICKS;
        }
        if (limit > ECAT_SYNC_LOSS_MAX_TICKS)
        {
            limit = ECAT_SYNC_LOSS_MAX_TICKS;
        }
        s_st.syncLost = (s_mode == ECAT_CYCLE_EXTERNAL) && (s_noTriggerTicks >= limit);
    }
    if ((s_st.state == ECAT_ST_OP) && (s_mode == ECAT_CYCLE_EXTERNAL) && !s_st.syncLost)
    {
        cycleStart = s_trigger;
    }
    else
    {
        cycleStart = (s_ticks >= ECAT_CYCLE_TICKS);
        if (cycleStart && (s_st.state == ECAT_ST_OP) && s_st.syncLost)
        {
            s_st.timerCycles++;
        }
    }
    s_trigger = false;
    if (cycleStart)
    {
        s_ticks = 0U;
    }

    if (s_st.state == ECAT_ST_OP)
    {
        ecat_pd_collect(false);
        if (cycleStart)
        {
            ecat_cycle();
        }
        return;
    }

    if (!cycleStart)
    {
        return;
    }

    switch (s_st.state)
    {
    case ECAT_ST_WAIT_LINK:
        if (s_configured)
        {
            /* link back: resume the cycles with the kept configuration; the
             * partner may need a while before it answers again */
            s_st.missRun = 0U;
            s_graceCycles = ECAT_LINK_GRACE_CYCLES;
            ecat_set_state(ECAT_ST_OP);
        }
        else if (s_configEnabled)
        {
            ecat_set_state(ECAT_ST_PROBE);
            ecat_probe_send();
        }
        else
        {
            /* wait for the Configuration state */
        }
        break;

    case ECAT_ST_PROBE:
        ecat_probe_collect();
        if (!s_configEnabled)
        {
            ecat_set_state(ECAT_ST_WAIT_LINK);
            break;
        }
        if (s_retryCycles > 0U)
        {
            s_retryCycles--;
            s_probeHits = 0U;
        }
        if (s_probeHits >= ECAT_PROBE_CONFIRM)
        {
            s_probeHits = 0U;
            ecat_set_state(ECAT_ST_CONFIG);
            ecat_configure();
        }
        else
        {
            ecat_probe_send();
        }
        break;

    case ECAT_ST_CONFIG:
    case ECAT_ST_ERROR:
    default:
        break;
    }
}

void ecat_master_outputs_write(const uint8_t *data, uint32_t len)
{
    ecat_master_outputs_write_at(0U, data, len);
}

void ecat_master_outputs_write_at(uint32_t offset, const uint8_t *data, uint32_t len)
{
    uint32_t n;

    if (!s_configured || (data == NULL) || (s_slaves[0].outputs == NULL)
            || (offset >= s_slaves[0].Obytes))
    {
        return;
    }
    n = s_slaves[0].Obytes - offset;
    n = (len < n) ? len : n;
    (void)memcpy(&s_slaves[0].outputs[offset], data, n);
    if ((offset == 0U) && (n > 0U))
    {
        s_st.out0 = data[0];
    }
}

void ecat_master_outputs_safe(void)
{
    if (s_configured && (s_slaves[0].outputs != NULL))
    {
        (void)memset(s_slaves[0].outputs, 0, s_slaves[0].Obytes);
        s_st.out0 = 0U;
    }
}

bool ecat_master_inputs_take(uint8_t *data, uint32_t maxLen, uint32_t *len)
{
    uint32_t n;

    if (!s_inputsFresh || (data == NULL) || (s_slaves[0].inputs == NULL))
    {
        return false;
    }
    n = (maxLen < s_slaves[0].Ibytes) ? maxLen : s_slaves[0].Ibytes;
    (void)memcpy(data, s_slaves[0].inputs, n);
    if (len != NULL)
    {
        *len = n;
    }
    s_inputsFresh = false;
    return true;
}

void ecat_master_request_al_state(uint16_t state)
{
    if (state != s_st.alTarget)
    {
        s_st.alTarget = state;
        ecat_update_target_wkc();
        if (state == EC_STATE_OPERATIONAL)
        {
            s_opGraceCycles = ECAT_OP_GRACE_CYCLES;
            s_st.missRun = 0U;
        }
        else
        {
            /* back to SAFE-OP: the inputs WKC is expected at once, a lost
             * slave must not hide behind the grace of an OP request */
            s_opGraceCycles = 0U;
        }
    }
}

bool ecat_master_operational(void)
{
    return s_st.state == ECAT_ST_OP;
}

bool ecat_master_configured(void)
{
    return s_configured;
}

bool ecat_master_link_up(void)
{
    return s_linkUp;
}

bool ecat_master_pd_fault(void)
{
    return s_linkUp && s_configured && (s_graceCycles == 0U)
            && (s_opGraceCycles == 0U) && (s_st.missRun >= ECAT_WKC_FAULT_CYCLES);
}

void ecat_master_stats_get(ecat_master_stats_t *stats)
{
    if (stats != NULL)
    {
        *stats = s_st;
    }
}

const char *ecat_master_state_name(ecat_state_t state)
{
    switch (state)
    {
    case ECAT_ST_WAIT_LINK: return "WAIT_LINK";
    case ECAT_ST_PROBE:     return "PROBE";
    case ECAT_ST_CONFIG:    return "CONFIG";
    case ECAT_ST_OP:        return "OP";
    case ECAT_ST_ERROR:     return "ERROR";
    default:                return "?";
    }
}

uint32_t ecat_master_config_failures(void)
{
    return s_st.configFailures;
}
