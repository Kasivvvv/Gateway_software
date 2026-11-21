#ifndef CYCLE_GEN_H
#define CYCLE_GEN_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Cycle generator of the gateway as the master of both networks
 * (configuration 4, thesis 7.2.4 and R5). Both cycles are derived from the
 * absolute count of Tloop ticks of one timer, so their phase is fixed by
 * construction:
 *
 *   EtherCAT cycle  at ticks  k * nEc
 *   SYNC            at ticks  k * nSync + phase
 *
 * A cycle whose tick was merged into a late iteration (overrun) is started
 * once in the next iteration; later cycles keep their absolute phase.
 * Pure logic.
 */

typedef struct
{
    uint32_t nEc;       /* T_EC in ticks */
    uint32_t nSync;     /* T_SYNC in ticks */
    uint32_t phase;     /* SYNC offset in ticks, < nSync */
    uint32_t lastEc;    /* index of the last started cycle */
    uint32_t lastSync;
    bool     started;
} cycle_gen_t;

typedef struct
{
    bool ecDue;
    bool syncDue;
} cycle_gen_out_t;

/** False if the periods are zero or the phase is not below the SYNC period. */
bool cycle_gen_init(cycle_gen_t *cg, uint32_t nEc, uint32_t nSync, uint32_t phase);

/** Evaluate at the absolute tick count of the current iteration. */
cycle_gen_out_t cycle_gen_step(cycle_gen_t *cg, uint32_t tick);

#endif /* CYCLE_GEN_H */
