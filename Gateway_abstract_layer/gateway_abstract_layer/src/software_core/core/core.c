#include "core.h"

#include "canopen.h"
#include "eth_probe.h"
#include "hwa_interface.h"

/* Stage 4a: Tloop + CANopen node (set S1) + raw Ethernet probe. */

#define CORE_STATS_PERIOD_TICKS   (5000U)  /* 1 s */
#define CORE_LED_PERIOD_TICKS     (2500U)  /* 500 ms */
#define CORE_MARK_LOOP_MIN_CYCLES (300U)   /* ~1 us @ 300 MHz */

typedef struct
{
    uint32_t iterations;
    uint32_t longestCycles;
    uint32_t lastCycles;
    uint32_t iterationsAtReport;
} core_stats_t;

static core_stats_t s_stats;
static uint32_t s_ticksSinceStats;
static uint32_t s_ticksSinceLed;
static uint32_t s_lastLinkChanges;

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

static void core_u64_to_dec(uint64_t value, char *out, size_t outLen)
{
    char tmp[21];
    size_t n = 0U;
    size_t i;

    if ((out == NULL) || (outLen == 0U))
    {
        return;
    }

    if (value == 0ULL)
    {
        if (outLen >= 2U)
        {
            out[0] = '0';
            out[1] = '\0';
        }
        return;
    }

    while ((value > 0ULL) && (n < sizeof(tmp)))
    {
        tmp[n++] = (char)('0' + (uint32_t)(value % 10ULL));
        value /= 10ULL;
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

static void core_write_hex32(uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    char out[11];
    uint32_t i;

    out[0] = '0';
    out[1] = 'x';
    for (i = 0U; i < 8U; i++)
    {
        out[2U + i] = hex[(value >> (28U - (4U * i))) & 0x0FU];
    }
    out[10] = '\0';
    (void)hwa_console_write_str(out);
}

static void core_write_u32(uint32_t value)
{
    char num[12];

    core_u32_to_dec(value, num, sizeof(num));
    (void)hwa_console_write_str(num);
}

/* Print link transitions as they happen (detected by the HAL each 10 ms). */
static void core_report_link(void)
{
    uint32_t changes = hwa_eth_link_change_count();
    hwa_eth_link_t link;

    if (changes == s_lastLinkChanges)
    {
        return;
    }
    s_lastLinkChanges = changes;

    hwa_eth_link_get(&link);
    if (link.up)
    {
        (void)hwa_console_write_str("ETH: link up ");
        core_write_u32(link.speedMbps);
        (void)hwa_console_write_str(link.fullDuplex ? " Mbit/s full duplex\r\n"
                                                    : " Mbit/s half duplex\r\n");
    }
    else
    {
        (void)hwa_console_write_str("ETH: link down\r\n");
    }
}

static void core_report_stats(void)
{
    char buf[200];
    char num[24];
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

    #define CORE_APPEND_U64(v) \
        do { \
            size_t k; \
            core_u64_to_dec((v), num, sizeof(num)); \
            for (k = 0U; (num[k] != '\0') && (pos + 1U < sizeof(buf)); k++) \
            { \
                buf[pos++] = num[k]; \
            } \
        } while (0)

    CORE_APPEND_LIT("S3: dit=");
    CORE_APPEND_U32(s_stats.iterations - s_stats.iterationsAtReport);
    CORE_APPEND_LIT(" nmt=0x");
    {
        static const char hex[] = "0123456789ABCDEF";
        uint8_t st = canopen_nmt_state();
        if (pos + 2U < sizeof(buf))
        {
            buf[pos++] = hex[(st >> 4) & 0x0FU];
            buf[pos++] = hex[st & 0x0FU];
        }
    }
    CORE_APPEND_LIT(" sync=");
    CORE_APPEND_U32(canopen_sync_count());
    CORE_APPEND_LIT(" tpdo=");
    CORE_APPEND_U32(canopen_tpdo_count());
    CORE_APPEND_LIT(" seq=");
    CORE_APPEND_U64(canopen_tpdo_seq());
    CORE_APPEND_LIT(" rpdo=");
    CORE_APPEND_U32(canopen_rpdo_count());
    CORE_APPEND_LIT(" rx=");
    CORE_APPEND_U64(canopen_rpdo_last());
    CORE_APPEND_LIT(" ovf=");
    CORE_APPEND_U32(hwa_can_rx_overflow_count());
    CORE_APPEND_LIT(" txdrop=");
    CORE_APPEND_U32(hwa_can_tx_drop_count());
    CORE_APPEND_LIT(" busoff=");
    CORE_APPEND_U32(hwa_can_is_bus_off() ? 1U : 0U);
    CORE_APPEND_LIT(" max_cy=");
    CORE_APPEND_U32(s_stats.longestCycles);
    {
        hwa_can_diag_t diag;
        hwa_can_diag_get(&diag);
        CORE_APPEND_LIT(" tec=");
        CORE_APPEND_U32(diag.tec);
        CORE_APPEND_LIT(" rec=");
        CORE_APPEND_U32(diag.rec);
        CORE_APPEND_LIT(" lec=");
        CORE_APPEND_U32(diag.lec);
    }
    CORE_APPEND_LIT("\r\n");
    buf[pos] = '\0';
    (void)hwa_console_write_str(buf);

    pos = 0U;
    {
        hwa_eth_link_t link;
        hwa_eth_link_get(&link);
        CORE_APPEND_LIT("S4: link=");
        if (link.up)
        {
            CORE_APPEND_U32(link.speedMbps);
            if (link.fullDuplex)
            {
                CORE_APPEND_LIT("FD");
            }
            else
            {
                CORE_APPEND_LIT("HD");
            }
        }
        else
        {
            CORE_APPEND_LIT("down");
        }
    }
    CORE_APPEND_LIT(" sigdet=");
    CORE_APPEND_U32(hwa_eth_sigdet() ? 1U : 0U);
    CORE_APPEND_LIT(" ethtx=");
    CORE_APPEND_U32(hwa_eth_tx_count());
    CORE_APPEND_LIT(" seq=");
    CORE_APPEND_U32(eth_probe_tx_seq());
    CORE_APPEND_LIT(" ethrx=");
    CORE_APPEND_U32(eth_probe_rx_frames());
    CORE_APPEND_LIT(" ecrx=");
    CORE_APPEND_U32(eth_probe_rx_ecat_frames());
    CORE_APPEND_LIT(" rxisr=");
    CORE_APPEND_U32(hwa_eth_rx_isr_count());
    CORE_APPEND_LIT(" rxdrop=");
    CORE_APPEND_U32(hwa_eth_rx_drop_count());
    CORE_APPEND_LIT(" txdrop=");
    CORE_APPEND_U32(hwa_eth_tx_drop_count());
    CORE_APPEND_LIT(" ovr=");
    CORE_APPEND_U32(hwa_time_overrun_count());
    CORE_APPEND_LIT("\r\n");
    buf[pos] = '\0';
    (void)hwa_console_write_str(buf);

    s_stats.iterationsAtReport = s_stats.iterations;

    #undef CORE_APPEND_LIT
    #undef CORE_APPEND_U32
    #undef CORE_APPEND_U64
}

void core_init(void)
{
    hwa_init();

    s_stats.iterations = 0U;
    s_stats.longestCycles = 0U;
    s_stats.lastCycles = 0U;
    s_stats.iterationsAtReport = 0U;
    s_ticksSinceStats = 0U;
    s_ticksSinceLed = 0U;

    s_lastLinkChanges = 0U;

    hwa_led_set(false);

    if (hwa_mem_dma_nocache_ok())
    {
        (void)hwa_console_write_str("MEM: DMA window 0x2045F800-0x2045FFFF non-cacheable OK\r\n");
    }
    else
    {
        (void)hwa_console_write_str("MEM: ERROR DMA window is cacheable - check MCC MPU region 5\r\n");
    }

    eth_probe_init();
    (void)hwa_console_write_str("ETH: PHY addr=1 id=");
    core_write_hex32(hwa_eth_phy_id());
    if ((hwa_eth_phy_id() & 0xFFFFFFF0UL) == 0x00221570UL)
    {
        (void)hwa_console_write_str(" (KSZ8061)\r\n");
    }
    else
    {
        (void)hwa_console_write_str(" (unexpected - check MDIO/reset)\r\n");
    }

    if (!canopen_init())
    {
        (void)hwa_console_write_str("Gateway core Stage 4a: CANopen init FAILED\r\n");
    }
    else
    {
        (void)hwa_console_write_str("Gateway core Stage 4a: CANopen node=0x10 HB=100ms + raw ETH, Tloop=");
        core_write_u32(hwa_time_tloop_us());
        (void)hwa_console_write_str(" us\r\n");
    }

    hwa_time_start();
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

    hwa_eth_poll();
    canopen_step();
    eth_probe_step();

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
        hwa_led_toggle();
    }

    core_report_link();
    hwa_console_poll();

    s_ticksSinceStats++;
    if (s_ticksSinceStats >= CORE_STATS_PERIOD_TICKS)
    {
        s_ticksSinceStats = 0U;
        core_report_stats();
    }
}
