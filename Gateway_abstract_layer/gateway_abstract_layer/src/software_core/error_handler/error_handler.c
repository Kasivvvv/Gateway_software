#include "error_handler.h"

#include <stddef.h>

static gw_error_t     s_code;
static gw_err_class_t s_class;
static bool           s_seenErrorState;
static uint32_t       s_reinitCount;
static uint32_t       s_initFailCount;
static uint64_t       s_workingSinceUs;
static bool           s_working;
static uint8_t        s_reinitMask;

#define EH_STACK_CAN  (0x01U)
#define EH_STACK_EC   (0x02U)

static gw_err_class_t eh_class_of(gw_error_t code)
{
    switch (code)
    {
    case GW_ERR_CAN_HB_TIMEOUT:
    case GW_ERR_CAN_RX_OVERFLOW:
    case GW_ERR_CONV_OVERFLOW:
        return GW_ERR_CLASS_RECOVERABLE;

    case GW_ERR_CAN_BUSOFF:
    case GW_ERR_EC_WKC:
    case GW_ERR_EC_INIT:
    case GW_ERR_CAN_INIT:
        return GW_ERR_CLASS_REINIT;

    case GW_ERR_CONFIG_CRC:
    case GW_ERR_SELFTEST:
        return GW_ERR_CLASS_FATAL;

    case GW_ERR_NONE:
    default:
        return GW_ERR_CLASS_NONE;
    }
}

static uint8_t eh_stack_of(gw_error_t code)
{
    switch (code)
    {
    case GW_ERR_CAN_BUSOFF:
    case GW_ERR_CAN_INIT:
        return EH_STACK_CAN;

    case GW_ERR_EC_WKC:
    case GW_ERR_EC_INIT:
        return EH_STACK_EC;

    default:
        return 0U;
    }
}

/* A failed (re-)initialisation of a stack: the fault may be inside the
 * gateway, so a hardware reset can help. Bus-off and the loss of EtherCAT
 * slaves are faults of the network: a reset of the gateway cannot remove
 * them and would only disturb the other network. */
static bool eh_is_init_failure(gw_error_t code)
{
    return (code == GW_ERR_EC_INIT) || (code == GW_ERR_CAN_INIT);
}

/* Latch an error; a higher class replaces the latched one. REINIT errors are
 * counted once per error episode; failed initialisations escalate to FATAL
 * after EH_MAX_REINIT. */
static void eh_latch(gw_error_t code)
{
    gw_err_class_t cls = eh_class_of(code);

    if (cls == GW_ERR_CLASS_REINIT)
    {
        s_reinitMask |= eh_stack_of(code);
        if (s_class < GW_ERR_CLASS_REINIT)
        {
            s_reinitCount++;
            if (eh_is_init_failure(code))
            {
                s_initFailCount++;
                if (s_initFailCount > EH_MAX_REINIT)
                {
                    cls = GW_ERR_CLASS_FATAL;
                }
            }
        }
        else
        {
            cls = s_class;
        }
    }
    if (cls > s_class)
    {
        s_code = code;
        s_class = cls;
    }
}

/* Highest-priority indication that applies to the current state. */
static gw_error_t eh_detect(gw_state_t state, const eh_inputs_t *in)
{
    switch (state)
    {
    case GW_ST_STARTUP:
        if (in->configCrcFailed)
        {
            return GW_ERR_CONFIG_CRC;
        }
        if (in->selfTestFailed)
        {
            return GW_ERR_SELFTEST;
        }
        break;

    case GW_ST_CONFIGURATION:
        if (in->canInitFailed)
        {
            return GW_ERR_CAN_INIT;
        }
        if (in->ecInitFailed)
        {
            return GW_ERR_EC_INIT;
        }
        /* a running stack failed while waiting (e.g. for the other stack):
         * re-initialise it, no escalation */
        if (in->canBusOff)
        {
            return GW_ERR_CAN_BUSOFF;
        }
        if (in->ecWkcFault)
        {
            return GW_ERR_EC_WKC;
        }
        break;

    case GW_ST_WORKING:
        if (in->canBusOff)
        {
            return GW_ERR_CAN_BUSOFF;
        }
        if (in->ecWkcFault)
        {
            return GW_ERR_EC_WKC;
        }
        if (in->canHbLost)
        {
            return GW_ERR_CAN_HB_TIMEOUT;
        }
        if (in->canRxOverflow)
        {
            return GW_ERR_CAN_RX_OVERFLOW;
        }
        if (in->convOverflow)
        {
            return GW_ERR_CONV_OVERFLOW;
        }
        break;

    case GW_ST_SAFE:
        /* critical errors only (T12); the conversion is stopped, so buffer
         * overflows do not matter here */
        if (in->canBusOff)
        {
            return GW_ERR_CAN_BUSOFF;
        }
        if (in->ecWkcFault)
        {
            return GW_ERR_EC_WKC;
        }
        if (in->canHbLost)
        {
            return GW_ERR_CAN_HB_TIMEOUT;
        }
        break;

    default:
        break;
    }
    return GW_ERR_NONE;
}

void eh_init(void)
{
    s_code = GW_ERR_NONE;
    s_class = GW_ERR_CLASS_NONE;
    s_seenErrorState = false;
    s_reinitCount = 0U;
    s_initFailCount = 0U;
    s_workingSinceUs = 0U;
    s_working = false;
    s_reinitMask = 0U;
}

void eh_step(gw_state_t state, const eh_inputs_t *in, uint64_t nowUs, eh_output_t *out)
{
    bool reset = false;

    if ((in == NULL) || (out == NULL))
    {
        return;
    }

    /* Escalation is forgotten after a stable period in the Working State. */
    if (state == GW_ST_WORKING)
    {
        if (!s_working)
        {
            s_working = true;
            s_workingSinceUs = nowUs;
        }
        else if ((nowUs - s_workingSinceUs) >= EH_STABLE_US)
        {
            s_reinitCount = 0U;
            s_initFailCount = 0U;
        }
    }
    else
    {
        s_working = false;
    }

    /* The machine has left the Error State: the error has been handled. */
    if ((s_code != GW_ERR_NONE) && s_seenErrorState && (state != GW_ST_ERROR))
    {
        s_code = GW_ERR_NONE;
        s_class = GW_ERR_CLASS_NONE;
        s_seenErrorState = false;
        s_reinitMask = 0U;
    }

    if (s_code == GW_ERR_NONE)
    {
        gw_error_t code = eh_detect(state, in);

        if (code != GW_ERR_NONE)
        {
            eh_latch(code);
        }
    }

    if (state == GW_ST_ERROR)
    {
        s_seenErrorState = true;
        /* escalation inside the Error State */
        if (in->canBusOff)
        {
            eh_latch(GW_ERR_CAN_BUSOFF);
        }
        if (in->ecWkcFault)
        {
            eh_latch(GW_ERR_EC_WKC);
        }
        if (s_class == GW_ERR_CLASS_RECOVERABLE)
        {
            /* Overflows are removed by the communication stop (buffers
             * flushed); a heartbeat timeout when the heartbeat is back and
             * the master has started the node again. */
            reset = (s_code != GW_ERR_CAN_HB_TIMEOUT)
                    || (!in->canHbLost && in->canOperational);
        }
    }

    out->code = s_code;
    out->cls = s_class;
    out->errorReset = reset;
    out->reinitCount = s_reinitCount;
    out->reinitCan = (s_reinitMask & EH_STACK_CAN) != 0U;
    out->reinitEc = (s_reinitMask & EH_STACK_EC) != 0U;
}

const char *eh_error_name(gw_error_t code)
{
    switch (code)
    {
    case GW_ERR_NONE:            return "NONE";
    case GW_ERR_CONFIG_CRC:      return "CONFIG_CRC";
    case GW_ERR_SELFTEST:        return "SELFTEST";
    case GW_ERR_EC_INIT:         return "EC_INIT";
    case GW_ERR_CAN_INIT:        return "CAN_INIT";
    case GW_ERR_CAN_BUSOFF:      return "CAN_BUSOFF";
    case GW_ERR_EC_WKC:          return "EC_WKC";
    case GW_ERR_CAN_HB_TIMEOUT:  return "CAN_HB_TIMEOUT";
    case GW_ERR_CAN_RX_OVERFLOW: return "CAN_RX_OVERFLOW";
    case GW_ERR_CONV_OVERFLOW:   return "CONV_OVERFLOW";
    default:                     return "?";
    }
}
