#ifndef ECAT_MASTER_H
#define ECAT_MASTER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * EtherCAT master on SOEM v1.4.0 (configurations 3 and 4).
 *
 * Superloop state machine, one ecat_master_step() per Tloop:
 *   WAIT_LINK -> PROBE -> CONFIG -> OP
 *   PROBE : non-blocking BRD every cycle, counts slaves by WKC;
 *   CONFIG: blocking SOEM configuration (bounded time), Configuration state
 *           only - the only place where the superloop may block;
 *   OP    : cyclic process data, send in one cycle, receive (timeout 0) in
 *           the next one, never blocking.
 */

#define ECAT_MAX_SLAVES       (8U)   /* slave entries 1..N (0 = master) */
#define ECAT_IOMAP_SIZE       (1024U)
#define ECAT_CYCLE_TICKS      (5U)   /* EtherCAT cycle: 5 x Tloop = 1 ms */

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
    int32_t  slaves;        /* slaves configured (CONFIG/OP) */
    int32_t  probeWkc;      /* last BRD WKC, -1 = frame did not return */
    uint32_t probeTx;       /* BRD probes sent */
    uint32_t probeRx;       /* BRD probes that came back */
    uint32_t configs;       /* configuration attempts */
    uint32_t lastConfigUs;  /* duration of the last (blocking) configuration */
    uint32_t iomapBytes;
    int32_t  expectedWkc;
    int32_t  lastWkc;
    uint32_t cycles;        /* process data cycles sent */
    uint32_t wkcOk;
    uint32_t wkcMiss;       /* frame missing or WKC below expected */
    /* Stage 5 test pattern (replaced by the conversion in Stage 6):
     * outputs[0] = counter, the test slave answers inputs[0] = outputs[0] + 1 */
    uint8_t  outValue;      /* last counter written to outputs[0] */
    uint8_t  inValue;       /* last inputs[0] received */
    uint8_t  echoLag;       /* outValue - (inValue - 1): cycles until echo */
} ecat_master_stats_t;

void ecat_master_init(void);
void ecat_master_step(void);

void ecat_master_stats_get(ecat_master_stats_t *stats);
const char *ecat_master_state_name(ecat_state_t state);

#endif /* ECAT_MASTER_H */
