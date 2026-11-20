#include "core.h"

#include "bsp/bsp.h"
#include "hwa_interface.h"

/* Stage 1: Tloop-driven superloop with MARK_LOOP and console stats. */

#define CORE_STATS_PERIOD_TICKS   (5000U)  /* 5000 * 200 us = 1 s */
#define CORE_LED_PERIOD_TICKS     (2500U)  /* 2500 * 200 us = 500 ms */

typedef struct
{
    uint32_t iterations;
    uint32_t longestCycles;
    uint32_t lastCycles;
    uint32_t overrunsAtReport;
} core_stats_t;

static core_stats_t s_stats;
static uint32_t s_ticksSinceStats;
static uint32_t s_ticksSinceLed;

static void core_u32_to_dec(uint32_t value, char *out, size_t outLen)
{
    char tmp[11];
    size_t n = 0U;
    size_t i;

    if ((out == NULL) || (outLen == 0U))
    {
        return;
    }

    if (value == 0U)
    {
        if (outLen >= 2U)
        {
            out[0] = '0';
            out[1] = '\0';
        }
        return;
    }

    while ((value > 0U) && (n < sizeof(tmp)))
    {
        tmp[n++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    if (n >= outLen)
    {
        n = outLen - 1U;
    }

    for (i = 0U; i < n; i++)
    {
        out[i] = tmp[n - 1U - i];
    }
    out[n] = '\0';
}

static void core_report_stats(void)
{
    char buf[96];
    char num[12];
    size_t pos = 0U;

    /* Format without printf to keep the image small and non-blocking. */
    #define CORE_APPEND_LIT(lit) \
        do { \
            size_t litLen = sizeof(lit) - 1U; \
            size_t k; \
            for (k = 0U; (k < litLen) && (pos + 1U < sizeof(buf)); k++) \
            { \
                buf[pos++] = (lit)[k]; \
            } \
        } while (0)

    #define CORE_APPEND_U32(v) \
        do { \
            size_t k; \
            core_u32_to_dec((v), num, sizeof(num)); \
            for (k = 0U; (num[k] != '\0') && (pos + 1U < sizeof(buf)); k++) \
            { \
                buf[pos++] = num[k]; \
            } \
        } while (0)

    CORE_APPEND_LIT("Tloop stats: it=");
    CORE_APPEND_U32(s_stats.iterations);
    CORE_APPEND_LIT(" last_cy=");
    CORE_APPEND_U32(s_stats.lastCycles);
    CORE_APPEND_LIT(" max_cy=");
    CORE_APPEND_U32(s_stats.longestCycles);
    CORE_APPEND_LIT(" ov=");
    CORE_APPEND_U32(hwa_time_overrun_count());
    CORE_APPEND_LIT("\r\n");
    buf[pos] = '\0';

    (void)hwa_console_write_str(buf);

    #undef CORE_APPEND_LIT
    #undef CORE_APPEND_U32
}

void core_init(void)
{
    hwa_init();

    s_stats.iterations = 0U;
    s_stats.longestCycles = 0U;
    s_stats.lastCycles = 0U;
    s_stats.overrunsAtReport = 0U;
    s_ticksSinceStats = 0U;
    s_ticksSinceLed = 0U;

    LED0_Off();

    (void)hwa_console_write_str("Gateway core Stage 1: Tloop=");
    {
        char num[12];
        core_u32_to_dec(hwa_time_tloop_us(), num, sizeof(num));
        (void)hwa_console_write_str(num);
    }
    (void)hwa_console_write_str(" us\r\n");
}

void core_step(void)
{
    hwa_cycles_t t0;
    hwa_cycles_t elapsed;

    if (!hwa_time_tick_take())
    {
        return;
    }

    hwa_marker_set(HAL_MARKER_LOOP);
    t0 = hwa_time_cycles();

    /* Stage 1: no conversion / stacks yet — measure empty iteration cost. */

    elapsed = hwa_time_cycles() - t0;
    hwa_marker_clear(HAL_MARKER_LOOP);

    s_stats.iterations++;
    s_stats.lastCycles = elapsed;
    if (elapsed > s_stats.longestCycles)
    {
        s_stats.longestCycles = elapsed;
    }

    s_ticksSinceLed++;
    if (s_ticksSinceLed >= CORE_LED_PERIOD_TICKS)
    {
        s_ticksSinceLed = 0U;
        LED0_Toggle();
    }

    s_ticksSinceStats++;
    if (s_ticksSinceStats >= CORE_STATS_PERIOD_TICKS)
    {
        s_ticksSinceStats = 0U;
        core_report_stats();
    }
}
