#include "cycle_gen.h"

#include <stddef.h>

bool cycle_gen_init(cycle_gen_t *cg, uint32_t nEc, uint32_t nSync, uint32_t phase)
{
    if ((cg == NULL) || (nEc == 0U) || (nSync == 0U) || (phase >= nSync))
    {
        return false;
    }
    cg->nEc = nEc;
    cg->nSync = nSync;
    cg->phase = phase;
    cg->lastEc = 0U;
    cg->lastSync = 0U;
    cg->started = false;
    return true;
}

cycle_gen_out_t cycle_gen_step(cycle_gen_t *cg, uint32_t tick)
{
    cycle_gen_out_t out = { false, false };
    uint32_t ec;
    uint32_t sync;

    if (cg == NULL)
    {
        return out;
    }
    /* index of the cycle whose start tick is the latest one <= tick */
    ec = tick / cg->nEc;
    sync = (tick >= cg->phase) ? (((tick - cg->phase) / cg->nSync) + 1U) : 0U;

    if (!cg->started)
    {
        /* first call: start only cycles that begin exactly now */
        cg->started = true;
        out.ecDue = ((tick % cg->nEc) == 0U);
        out.syncDue = (sync != 0U) && (((tick - cg->phase) % cg->nSync) == 0U);
    }
    else
    {
        out.ecDue = (ec != cg->lastEc);
        out.syncDue = (sync != cg->lastSync);
    }
    cg->lastEc = ec;
    cg->lastSync = sync;
    return out;
}
