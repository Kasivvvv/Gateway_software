#include "hal_interface.h"

#include "definitions.h"
#include "peripheral/tc/plib_tc0.h"

/*
 * Tloop source: TC0 channel 1 (channel 0 is SYS_TIME).
 * Datasheet (online DS60001527): TC waveform UP_RC, interrupt on RC compare
 * (TC_SR.CPCS). Clock: MCK with NODIVCLK → 150 MHz (same as generated PLIB).
 *
 * MCC was configured with period unit "nanosecond" (RC=29 → 200 ns).
 * Software forces 200 µs: RC = f_clk * T_us / 1e6 - 1 = 29999.
 */

#define HAL_TLOOP_US              (200U)
#define HAL_CPU_HZ                (300000000UL)

static volatile bool s_tickPending = false;
static volatile uint32_t s_tickCount = 0U;
static volatile uint32_t s_overrunCount = 0U;

static void hal_time_tloop_isr(TC_TIMER_STATUS status, uintptr_t context)
{
    (void)status;
    (void)context;

    s_tickCount++;
    if (s_tickPending)
    {
        s_overrunCount++;
    }
    else
    {
        s_tickPending = true;
    }
}

static void hal_time_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void hal_time_init(void)
{
    uint32_t freqHz;
    uint32_t periodCounts;

    s_tickPending = false;
    s_tickCount = 0U;
    s_overrunCount = 0U;

    hal_time_dwt_init();

    freqHz = TC0_CH1_TimerFrequencyGet();
    periodCounts = ((freqHz / 1000000UL) * (uint32_t)HAL_TLOOP_US) - 1UL;
    TC0_CH1_TimerPeriodSet((uint16_t)periodCounts);

    TC0_CH1_TimerCallbackRegister(hal_time_tloop_isr, (uintptr_t)NULL);
    TC0_CH1_TimerStart();
}

void hal_time_deinit(void)
{
    TC0_CH1_TimerStop();
    TC0_CH1_TimerCallbackRegister(NULL, (uintptr_t)NULL);
}

bool hal_time_tick_take(void)
{
    bool taken = false;
    bool irqState = NVIC_INT_Disable();

    if (s_tickPending)
    {
        s_tickPending = false;
        taken = true;
    }

    NVIC_INT_Restore(irqState);
    return taken;
}

hal_tick_t hal_time_tick_count(void)
{
    return s_tickCount;
}

uint32_t hal_time_overrun_count(void)
{
    return s_overrunCount;
}

hal_cycles_t hal_time_cycles(void)
{
    return DWT->CYCCNT;
}

uint32_t hal_time_cpu_hz(void)
{
    return HAL_CPU_HZ;
}

uint32_t hal_time_tloop_us(void)
{
    return HAL_TLOOP_US;
}
