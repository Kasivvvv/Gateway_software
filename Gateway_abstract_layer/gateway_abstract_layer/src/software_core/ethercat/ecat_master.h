#ifndef ECAT_MASTER_H
#define ECAT_MASTER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * EtherCAT master on SOEM v1.4.0 (configurations 3 and 4).
 *
 * Superloop module, one ecat_master_step() per Tloop:
 *   WAIT_LINK -> PROBE -> CONFIG -> OP
 *   PROBE : non-blocking BRD every cycle, counts slaves by WKC;
 *   CONFIG: blocking SOEM configuration with a hard time limit. Probing and
 *           configuration run only while the Software Core enables them,
 *           i.e. in the Configuration state of the gateway (the only state in
 *           which the superloop may block);
 *   OP    : cyclic process data; the frame is sent at the cycle start and its
 *           return is polled once per Tloop (never blocking). Every cycle
 *           also carries one auxiliary datagram that reads the AL status of
 *           the slaves or, when it differs from the target state, writes
 *           their AL control (with error acknowledge if needed).
 * A configured master keeps its configuration over a link loss: when the
 * link returns it resumes the cycles; re-configuration needs the
 * Configuration state again (ecat_master_reset(), transition T10).
 *
 * Cycle start: own timer (ECAT_CYCLE_TICKS, configuration 4) or the external
 * trigger ecat_master_trigger() on each SYNC (configuration 3, thesis 7.2.4).
 * In external mode the master falls back to its own timer when no trigger
 * arrived for ECAT_SYNC_LOSS_PERIODS SYNC periods (averaged, at least 10 ms
 * against the jitter of a PC-based SYNC producer), so the slaves
 * keep receiving frames (and the safe outputs / SAFE-OP request of the safe
 * reaction reach them) also when the CANopen side is silent.
 *
 * Working counter: checked against the target AL state; in SAFE-OP only the
 * inputs are exchanged, so the expected WKC is that of the inputs.
 */

#define ECAT_MAX_SLAVES       (8U)   /* slave entries 1..N (0 = master) */
#define ECAT_IOMAP_SIZE       (1024U)
#define ECAT_CYCLE_TICKS      (5U)   /* timer mode: 5 x Tloop = 1 ms */
#define ECAT_WKC_FAULT_CYCLES (3U)   /* consecutive lost cycles = WKC fault */
#define ECAT_OP_GRACE_CYCLES  (250U) /* cycles allowed for SAFE-OP -> OP */
#define ECAT_LINK_GRACE_CYCLES (1000U) /* cycles allowed after a link-up */
#define ECAT_SYNC_LOSS_PERIODS (4U)  /* missing SYNC periods -> own timer */
#define ECAT_SYNC_LOSS_MIN_TICKS (50U)   /* bounds of the SYNC-loss timeout, */
#define ECAT_SYNC_LOSS_MAX_TICKS (500U)  /* Tloop units: 10 ms .. 100 ms     */

/* AL states (ETG.1000.6), for ecat_master_request_al_state() */
#define ECAT_AL_SAFE_OP       (0x04U)
#define ECAT_AL_OP            (0x08U)

typedef enum
{
    ECAT_CYCLE_TIMER = 0,
    ECAT_CYCLE_EXTERNAL
} ecat_cycle_mode_t;

typedef enum
{
    ECAT_ST_WAIT_LINK = 0,
    ECAT_ST_PROBE,
    ECAT_ST_CONFIG,
    ECAT_ST_OP,
    ECAT_ST_ERROR
} ecat_state_t;

typedef struct
{
    ecat_state_t state;
    int32_t  slaves;        /* slaves configured */
    int32_t  probeWkc;      /* last BRD WKC, -1 = frame did not return */
    uint32_t probeTx;
    uint32_t probeRx;
    uint32_t configs;       /* configuration attempts */
    uint32_t configFailures;/* attempts with slaves present that failed */
    uint32_t lastConfigUs;  /* duration of the last (blocking) configuration */
    uint32_t iomapBytes;
    int32_t  expectedWkc;   /* LRW working counter with all slaves in OP */
    int32_t  targetWkc;     /* working counter expected in the target AL state */
    int32_t  lastWkc;
    uint32_t cycles;        /* process data cycles sent */
    uint32_t wkcOk;
    uint32_t wkcMiss;       /* frame missing or WKC below expected */
    uint32_t missRun;       /* consecutive misses */
    uint32_t inputsUpdates; /* cycles with valid inputs */
    uint8_t  out0;          /* first byte of the last outputs written */
    uint8_t  in0;           /* first byte of the last inputs received */
    uint16_t alStatus;      /* AL status read from the slaves (OR of all) */
    int32_t  alWkc;         /* slaves that answered the AL status read */
    uint16_t alTarget;      /* requested AL state */
    bool     syncLost;      /* external mode: cycles on the own timer */
    uint32_t timerCycles;   /* cycles started by the fallback timer */
} ecat_master_stats_t;

void ecat_master_init(ecat_cycle_mode_t mode);
void ecat_master_step(void);

/** Allow probing and the blocking configuration (Configuration state only). */
void ecat_master_set_config_enabled(bool enabled);

/** Forget the configuration (T10): the next Configuration state redoes it. */
void ecat_master_reset(void);

/** Start a process data cycle in the next ecat_master_step() (external mode). */
void ecat_master_trigger(void);

/** Outputs of the next cycle (bytes beyond the mapped size are ignored). */
void ecat_master_outputs_write(const uint8_t *data, uint32_t len);

/** Outputs of the slave from byte offset on (one CAN node per 8 bytes in
 *  configuration 4). */
void ecat_master_outputs_write_at(uint32_t offset, const uint8_t *data, uint32_t len);

/** Safe output values (all zero) from the next cycle on. */
void ecat_master_outputs_safe(void);

/** Inputs of the last completed cycle; true once per new valid input set. */
bool ecat_master_inputs_take(uint8_t *data, uint32_t maxLen, uint32_t *len);

/** Target AL state of all slaves (ECAT_AL_OP / ECAT_AL_SAFE_OP). A
 *  (re-)configuration brings the slaves to SAFE-OP and on to this target.
 *  Requesting SAFE-OP cancels the grace period of a preceding OP request. */
void ecat_master_request_al_state(uint16_t state);

/** Configured and cycling (link may be down: see ecat_master_link_up()). */
bool ecat_master_operational(void);
bool ecat_master_configured(void);
bool ecat_master_link_up(void);

/** Working counter fault: ECAT_WKC_FAULT_CYCLES consecutive cycles below the
 *  WKC of the target AL state, link up, after the grace periods of a
 *  configuration / link-up and of an OP request. */
bool ecat_master_pd_fault(void);

/** Failed configuration attempts (slaves present) since init. */
uint32_t ecat_master_config_failures(void);

void ecat_master_stats_get(ecat_master_stats_t *stats);
const char *ecat_master_state_name(ecat_state_t state);

#endif /* ECAT_MASTER_H */
