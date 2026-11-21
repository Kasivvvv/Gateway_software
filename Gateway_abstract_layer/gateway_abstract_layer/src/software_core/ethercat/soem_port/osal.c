/*
 * SOEM OSAL for the bare-metal gateway.
 *
 * Time base: hwa_time_now_us() (64-bit, DWT-based). osal_usleep() busy-waits;
 * SOEM calls it only from blocking configuration paths, which the gateway runs
 * in the Configuration state only. No threads (EC_MAX_MAPT = 1).
 */
#include <osal.h>

#include "osal_port.h"
#include "hwa_interface.h"

#define OSAL_USEC_PER_SEC (1000000ULL)

static uint64_t s_deadlineUs; /* 0 = none */
static bool s_deadlineHit;

void osal_deadline_set(uint64_t deadline_us)
{
    s_deadlineUs = deadline_us;
    s_deadlineHit = false;
}

bool osal_deadline_hit(void)
{
    return s_deadlineHit;
}

static bool osal_deadline_passed(uint64_t now)
{
    if ((s_deadlineUs != 0U) && (now >= s_deadlineUs))
    {
        s_deadlineHit = true;
        return true;
    }
    return false;
}

static ec_timet osal_from_us(uint64_t us)
{
    ec_timet t;

    t.sec = (uint32)(us / OSAL_USEC_PER_SEC);
    t.usec = (uint32)(us % OSAL_USEC_PER_SEC);
    return t;
}

static uint64_t osal_to_us(const ec_timet *t)
{
    return ((uint64_t)t->sec * OSAL_USEC_PER_SEC) + t->usec;
}

ec_timet osal_current_time(void)
{
    return osal_from_us(hwa_time_now_us());
}

void osal_time_diff(ec_timet *start, ec_timet *end, ec_timet *diff)
{
    uint64_t s = osal_to_us(start);
    uint64_t e = osal_to_us(end);

    *diff = osal_from_us((e > s) ? (e - s) : 0U);
}

void osal_timer_start(osal_timert *self, uint32 timeout_us)
{
    self->stop_time = osal_from_us(hwa_time_now_us() + timeout_us);
}

boolean osal_timer_is_expired(osal_timert *self)
{
    uint64_t now = hwa_time_now_us();

    if (osal_deadline_passed(now))
    {
        return TRUE;
    }
    return (now >= osal_to_us(&self->stop_time)) ? TRUE : FALSE;
}

int osal_usleep(uint32 usec)
{
    uint64_t stop = hwa_time_now_us() + usec;
    uint64_t now;

    do
    {
        now = hwa_time_now_us();
    } while ((now < stop) && !osal_deadline_passed(now));
    return 0;
}

int osal_thread_create(void *thandle, int stacksize, void *func, void *param)
{
    (void)thandle;
    (void)stacksize;
    (void)func;
    (void)param;
    return 0; /* no threads on bare metal */
}

int osal_thread_create_rt(void *thandle, int stacksize, void *func, void *param)
{
    return osal_thread_create(thandle, stacksize, func, param);
}
