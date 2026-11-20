#include "core.h"

#include "bsp/bsp.h"
#include "hwa_interface.h"

/* Stage 2: Tloop + CAN echo on MCAN1. */

#define CORE_STATS_PERIOD_TICKS   (5000U)  /* 1 s */
#define CORE_LED_PERIOD_TICKS     (2500U)  /* 500 ms */
#define CORE_MARK_LOOP_MIN_CYCLES (300U)   /* ~1 us @ 300 MHz */
#define CORE_CAN_ECHO_MAX_PER_TICK (8U)

typedef struct
{
    uint32_t iterations;
    uint32_t longestCycles;
    uint32_t lastCycles;
    uint32_t iterationsAtReport;
    uint32_t canEchoed;
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
    char buf[160];
    char num[12];
    size_t pos = 0U;

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

    CORE_APPEND_LIT("S2: dit=");
    CORE_APPEND_U32(s_stats.iterations - s_stats.iterationsAtReport);
    CORE_APPEND_LIT(" echo=");
    CORE_APPEND_U32(s_stats.canEchoed);
    CORE_APPEND_LIT(" rxq=");
    CORE_APPEND_U32(hwa_get_number_of_massages());
    CORE_APPEND_LIT(" hw=");
    CORE_APPEND_U32(hwa_can_rx_high_water());
    CORE_APPEND_LIT(" ovf=");
    CORE_APPEND_U32(hwa_can_rx_overflow_count());
    CORE_APPEND_LIT(" txdrop=");
    CORE_APPEND_U32(hwa_can_tx_drop_count());
    CORE_APPEND_LIT(" busoff=");
    CORE_APPEND_U32(hwa_can_is_bus_off() ? 1U : 0U);
    CORE_APPEND_LIT(" max_cy=");
    CORE_APPEND_U32(s_stats.longestCycles);
    CORE_APPEND_LIT("\r\n");
    s_stats.iterationsAtReport = s_stats.iterations;
    buf[pos] = '\0';

    (void)hwa_console_write_str(buf);

    #undef CORE_APPEND_LIT
    #undef CORE_APPEND_U32
}

static void core_can_echo(void)
{
    uint32_t n;

    for (n = 0U; n < CORE_CAN_ECHO_MAX_PER_TICK; n++)
    {
        uint32_t id = 0U;
        uint32_t subId = 0U;
        uint8_t data[8];
        uint8_t len = 0U;

        if (hwa_get_number_of_massages() == 0U)
        {
            break;
        }

        hwa_can_rx(&id, &subId, data, &len);
        if (len == 0U)
        {
            break;
        }

        /* Echo with same ID; sub_id unused on the wire in Stage 2. */
        hwa_can_tx(id, subId, data, len);
        s_stats.canEchoed++;
    }
}

void core_init(void)
{
    hwa_init();

    s_stats.iterations = 0U;
    s_stats.longestCycles = 0U;
    s_stats.lastCycles = 0U;
    s_stats.iterationsAtReport = 0U;
    s_stats.canEchoed = 0U;
    s_ticksSinceStats = 0U;
    s_ticksSinceLed = 0U;

    LED0_Off();

    (void)hwa_console_write_str("Gateway core Stage 2: CAN echo, Tloop=");
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

    core_can_echo();

    elapsed = hwa_time_cycles() - t0;

    while ((hwa_time_cycles() - t0) < CORE_MARK_LOOP_MIN_CYCLES)
    {
    }

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
