#include "core.h"

#include "bsp/bsp.h"
#include "system/time/sys_time.h"

/* Stage 0 smoke test: visible life sign on LED0. */
#define CORE_LED0_BLINK_MS    (500U)

static uint32_t s_ledToggleCount = 0U;

void core_init(void)
{
    s_ledToggleCount = SYS_TIME_CounterGet();
    LED0_Off();
}

void core_step(void)
{
    uint32_t nowCount = SYS_TIME_CounterGet();

    if ((nowCount - s_ledToggleCount) >= SYS_TIME_MSToCount(CORE_LED0_BLINK_MS))
    {
        s_ledToggleCount = nowCount;
        LED0_Toggle();
    }
}
