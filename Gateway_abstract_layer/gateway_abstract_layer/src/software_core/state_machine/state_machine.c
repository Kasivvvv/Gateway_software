#include "state_machine.h"

#include <stddef.h>

/*
 * Transitions (thesis Table 7.2):
 *   T1  INIT          -> Startup        low-level initialisation completed
 *   T2  Startup       -> Configuration  self-test passed
 *   T3  Startup       -> Error          configuration CRC or self-test failed
 *   T4  Configuration -> Working        both stacks initialised
 *   T5  Configuration -> Error          stack initialisation failed (also:
 *                                       a running stack failed while waiting
 *                                       in Configuration: bus-off, configured
 *                                       EtherCAT network lost)
 *   T6  Working       -> Error          critical error
 *   T7  Working       -> Safe           safe-state request / link loss / service
 *   T8  Safe          -> Working        cause removed and resume command
 *   T9  Error         -> Working        error reset, no re-initialisation
 *   T10 Error         -> Startup        re-initialisation of the communication
 *   T11 Error         -> INIT           irrecoverable: hardware reset
 *   T12 Safe          -> Error          critical error in the Safe State
 *                                       (extension of Table 7.2: without it a
 *                                       bus-off in Safe is a deadlock, since
 *                                       the resume command of T8 cannot arrive)
 * An error (T6, T12) takes precedence over a safe-state request (T7) and a
 * resume command (T8).
 */

static gw_state_t s_state = GW_ST_INIT;

void sm_init(void)
{
    s_state = GW_ST_INIT;
}

gw_state_t sm_state(void)
{
    return s_state;
}

static uint8_t sm_next(const sm_inputs_t *in, gw_state_t *next)
{
    switch (s_state)
    {
    case GW_ST_INIT:
        if (in->lowLevelInitDone)
        {
            *next = GW_ST_STARTUP;
            return 1U;
        }
        break;

    case GW_ST_STARTUP:
        if (in->selfTestDone)
        {
            *next = in->selfTestOk ? GW_ST_CONFIGURATION : GW_ST_ERROR;
            return in->selfTestOk ? 2U : 3U;
        }
        break;

    case GW_ST_CONFIGURATION:
        if (in->stacksFailed)
        {
            *next = GW_ST_ERROR;
            return 5U;
        }
        if (in->stacksReady)
        {
            *next = GW_ST_WORKING;
            return 4U;
        }
        break;

    case GW_ST_WORKING:
        if (in->errorClass != GW_ERR_CLASS_NONE)
        {
            *next = GW_ST_ERROR;
            return 6U;
        }
        if (in->safeCause)
        {
            *next = GW_ST_SAFE;
            return 7U;
        }
        break;

    case GW_ST_SAFE:
        if (in->errorClass != GW_ERR_CLASS_NONE)
        {
            *next = GW_ST_ERROR;
            return 12U;
        }
        if (!in->safeCause && in->resume)
        {
            *next = GW_ST_WORKING;
            return 8U;
        }
        break;

    case GW_ST_ERROR:
        if (in->errorClass == GW_ERR_CLASS_FATAL)
        {
            *next = GW_ST_INIT;
            return 11U;
        }
        if (in->errorClass == GW_ERR_CLASS_REINIT)
        {
            *next = GW_ST_STARTUP;
            return 10U;
        }
        if (in->errorReset)
        {
            *next = GW_ST_WORKING;
            return 9U;
        }
        break;

    default:
        *next = GW_ST_INIT;
        return 11U;
    }
    return 0U;
}

void sm_step(const sm_inputs_t *in, sm_output_t *out)
{
    gw_state_t next = s_state;
    uint8_t t;

    if ((in == NULL) || (out == NULL))
    {
        return;
    }

    t = sm_next(in, &next);
    out->previous = s_state;
    out->transition = t;
    s_state = next;
    out->state = s_state;
    out->convert = (s_state == GW_ST_WORKING);
    out->commStop = (t != 0U) && ((s_state == GW_ST_ERROR) || (s_state == GW_ST_SAFE));
    out->commReset = (t == 10U);
    out->hwReset = (t == 11U);
}

const char *sm_state_name(gw_state_t st)
{
    switch (st)
    {
    case GW_ST_INIT:          return "INIT";
    case GW_ST_STARTUP:       return "STARTUP";
    case GW_ST_CONFIGURATION: return "CONFIGURATION";
    case GW_ST_WORKING:       return "WORKING";
    case GW_ST_ERROR:         return "ERROR";
    case GW_ST_SAFE:          return "SAFE";
    default:                  return "?";
    }
}
