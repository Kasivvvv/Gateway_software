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
/* Consecutive missed cycles in OP before falling back to PROBE. */
#define ECAT_OP_LOST_CYCLES       (1000U)
/* Hard limit of one blocking configuration (agreed rule: SOEM may block only
 * in the Configuration state, with a time limit). */
#define ECAT_CONFIG_LIMIT_US      (2000000U)
/* Pause after a failed configuration before probing again (cycles). */
#define ECAT_RETRY_CYCLES         (1000U)

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
static bool     s_nicReady;
static uint32_t s_ticks;
static int      s_probeIdx;
static uint32_t s_probeHits;
static bool     s_pdPending;
static uint32_t s_missRun;
static uint32_t s_retryCycles;

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
    (void)snprintf(line, sizeof(line), "ECAT: %s (%d) after %lu us, retry in %u ms\r\n",
            what, value, (unsigned long)s_st.lastConfigUs,
            (unsigned)ECAT_RETRY_CYCLES);
    ecat_log(line);
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

    /* One process data exchange with valid outputs, then request OP. */
    (void)ecx_send_processdata(&s_ctx);
    (void)ecx_receive_processdata(&s_ctx, EC_TIMEOUTRET);
    s_slaves[0].state = EC_STATE_OPERATIONAL;
    (void)ecx_writestate(&s_ctx, 0);

    osal_deadline_set(0U);
    s_st.lastConfigUs = (uint32_t)(hwa_time_now_us() - t0);
    (void)snprintf(line, sizeof(line),
            "ECAT: %d slave(s), IOmap %d B, expected WKC %ld, config %lu us\r\n",
            n, iomap, (long)s_st.expectedWkc, (unsigned long)s_st.lastConfigUs);
    ecat_log(line);
    ecat_report_slaves();

    s_pdPending = false;
    s_missRun = 0U;
    ecat_set_state(ECAT_ST_OP);
}

/* ---- OP: cyclic process data, non-blocking --------------------------------- */

static void ecat_cycle(void)
{
    if (s_pdPending)
    {
        int wkc = ecx_receive_processdata(&s_ctx, 0);

        s_st.lastWkc = wkc;
        if (wkc >= s_st.expectedWkc)
        {
            s_st.wkcOk++;
            s_missRun = 0U;
        }
        else
        {
            s_st.wkcMiss++;
            s_missRun++;
        }
    }

    if (s_missRun >= ECAT_OP_LOST_CYCLES)
    {
        ecat_log("ECAT: process data lost, back to probing\r\n");
        s_pdPending = false;
        s_probeHits = 0U;
        ecat_set_state(ECAT_ST_PROBE);
        return;
    }

    (void)ecx_send_processdata(&s_ctx);
    s_pdPending = true;
    s_st.cycles++;
}

/* ---- Public ----------------------------------------------------------------- */

void ecat_master_init(void)
{
    (void)memset(&s_st, 0, sizeof(s_st));
    s_st.state = ECAT_ST_WAIT_LINK;
    s_st.probeWkc = -1;
    s_nicReady = false;
    s_ticks = 0U;
    s_probeIdx = -1;
    s_probeHits = 0U;
    s_pdPending = false;
    s_missRun = 0U;
    s_retryCycles = 0U;
}

void ecat_master_step(void)
{
    hwa_eth_link_t link;

    hwa_eth_link_get(&link);

    if (!link.up)
    {
        if (s_st.state != ECAT_ST_WAIT_LINK)
        {
            s_probeIdx = -1;
            s_pdPending = false;
            s_probeHits = 0U;
            ecat_set_state(ECAT_ST_WAIT_LINK);
        }
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

    s_ticks++;
    if (s_ticks < ECAT_CYCLE_TICKS)
    {
        return;
    }
    s_ticks = 0U;

    switch (s_st.state)
    {
    case ECAT_ST_WAIT_LINK:
        ecat_set_state(ECAT_ST_PROBE);
        ecat_probe_send();
        break;

    case ECAT_ST_PROBE:
        ecat_probe_collect();
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

    case ECAT_ST_OP:
        ecat_cycle();
        break;

    case ECAT_ST_CONFIG:
    case ECAT_ST_ERROR:
    default:
        break;
    }
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
