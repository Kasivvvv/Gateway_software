#include "hal_interface.h"
#include "hal_nocache.h"

#include <string.h>

#include "definitions.h"

/*
 * Raw Ethernet on GMAC (RMII) + Micrel KSZ8061RNBVA PHY, no TCP/IP stack.
 *
 * References:
 *   DS60001527 (online), GMAC chapter: buffer descriptors ("Receive Buffer
 *     Descriptor Entry", "Transmit Buffer Descriptor Entry"), NCR/NCFGR/DCFGR,
 *     PHY maintenance (MAN, NSR.IDLE), priority queues.
 *   Atmel-42408 SAM V71 Xplained Ultra User Guide, 4.3.7 Ethernet:
 *     Tab. 4-26 pins (PC10 RESET, PA19 INTERRUPT, PA29 SIGDET),
 *     Tab. 4-27 straps (PHYAD = 001, RMII, auto-negotiation enabled).
 *
 * Design (bare-metal superloop, non-blocking after init):
 *   - one frame per DMA buffer (1536 B), RX ring 16, TX ring 4;
 *   - descriptors in the non-cacheable window (hal_nocache.h, MCC MPU
 *     region 5), right after the MCAN Message RAM;
 *   - data buffers in cached SRAM with explicit D-cache clean/invalidate;
 *   - frames are copied in/out from the superloop only; the GMAC interrupt
 *     (RCOMP) only produces the t1 marker and an arrival timestamp;
 *   - PHY link supervision through a non-blocking MDIO state machine.
 */

/* ---- Configuration -------------------------------------------------------- */

#define HAL_ETH_PHY_ADDR           (1U)
#define HAL_ETH_RX_DESC_NUM        (16U)
#define HAL_ETH_TX_DESC_NUM        (4U)
/* Multiple of 64 B (DCFGR.DRBS unit) and of the 32 B cache line. */
#define HAL_ETH_BUF_SIZE           (1536U)
#define HAL_ETH_PRIO_QUEUE_NUM     (5U)


#define HAL_ETH_CACHE_LINE         (32U)
#define HAL_ETH_CYCLES_PER_US      (300U)
#define HAL_ETH_PHY_RESET_US       (10000U)
#define HAL_ETH_PHY_STARTUP_US     (10000U)
#define HAL_ETH_MDIO_TIMEOUT_US    (1000U)
/* PHY status poll period: 50 x Tloop(200 us) = 10 ms. */
#define HAL_ETH_LINK_POLL_TICKS    (50U)
/* ~2 us @ 300 MHz, visible on a logic analyser (same as hal_can). */
#define HAL_ETH_MARK_MIN_CYCLES    (600U)

/* GMAC buffer descriptor bits (DS60001527, GMAC buffer descriptor tables). */
#define HAL_ETH_RXD_OWN            (1UL << 0)   /* word0: 1 = frame stored, SW owns */
#define HAL_ETH_RXD_WRAP           (1UL << 1)   /* word0: last descriptor */
#define HAL_ETH_RXD_ADDR_MSK       (0xFFFFFFFCUL)
#define HAL_ETH_RXD_LEN_MSK        (0x1FFFUL)   /* word1: frame length */
#define HAL_ETH_RXD_SOF            (1UL << 14)
#define HAL_ETH_RXD_EOF            (1UL << 15)

#define HAL_ETH_TXD_LEN_MSK        (0x3FFFUL)
#define HAL_ETH_TXD_LAST           (1UL << 15)
#define HAL_ETH_TXD_WRAP           (1UL << 30)
#define HAL_ETH_TXD_USED           (1UL << 31)  /* 1 = SW owns / sent */

/* Clause 22 PHY registers (IEEE 802.3, KSZ8061 datasheet). */
#define HAL_ETH_PHY_BMSR           (1U)
#define HAL_ETH_PHY_ID1            (2U)
#define HAL_ETH_PHY_ID2            (3U)
#define HAL_ETH_PHY_ANAR           (4U)
#define HAL_ETH_PHY_ANLPAR         (5U)
#define HAL_ETH_BMSR_LINK          (1U << 2)
#define HAL_ETH_BMSR_ANEG_DONE     (1U << 5)
#define HAL_ETH_AN_10HD            (1U << 5)
#define HAL_ETH_AN_10FD            (1U << 6)
#define HAL_ETH_AN_100HD           (1U << 7)
#define HAL_ETH_AN_100FD           (1U << 8)

#define HAL_ETH_MAN_OP_WRITE       (1U)
#define HAL_ETH_MAN_OP_READ        (2U)

#ifndef ETH_RESET_Set
#error "MCC: configure PC10 as GPIO output, latch High, custom name ETH_RESET"
#endif
#ifndef ETH_SIGDET_Get
#error "MCC: configure PA29 as GPIO input, custom name ETH_SIGDET"
#endif

/* ---- Descriptors (non-cacheable) and buffers (cached) --------------------- */

typedef struct
{
    volatile uint32_t addr;
    volatile uint32_t status;
} hal_eth_desc_t;

typedef struct
{
    hal_eth_desc_t rx[HAL_ETH_RX_DESC_NUM];
    hal_eth_desc_t tx[HAL_ETH_TX_DESC_NUM];
    /* Unused priority queues 1..5 must point at valid, inactive descriptors. */
    hal_eth_desc_t rxDummy;
    hal_eth_desc_t txDummy;
} hal_eth_desc_area_t;

_Static_assert(sizeof(hal_eth_desc_area_t) <= HAL_NOCACHE_GMAC_MAX,
        "GMAC descriptors exceed their part of the non-cacheable window");

static hal_eth_desc_area_t __attribute__((aligned(8), space(data),
        address(HAL_NOCACHE_GMAC_ADDR))) s_desc;

static uint8_t __attribute__((aligned(HAL_ETH_CACHE_LINE)))
    s_rxBuf[HAL_ETH_RX_DESC_NUM][HAL_ETH_BUF_SIZE];
static uint8_t __attribute__((aligned(HAL_ETH_CACHE_LINE)))
    s_txBuf[HAL_ETH_TX_DESC_NUM][HAL_ETH_BUF_SIZE];

/* ---- State ----------------------------------------------------------------- */

typedef enum
{
    HAL_ETH_MDIO_IDLE = 0,
    HAL_ETH_MDIO_WAIT_BMSR,
    HAL_ETH_MDIO_WAIT_ANLPAR
} hal_eth_mdio_state_t;

static uint32_t s_rxIdx;
static uint32_t s_txHead;
static uint32_t s_txTail;
static uint32_t s_txPending;

static hal_eth_link_t s_link;
static uint32_t s_phyId;
static uint16_t s_phyAnar;
static bool s_phyOk;
static hal_eth_mdio_state_t s_mdioState;
static uint32_t s_linkPollTicks;
static uint32_t s_linkChanges;

static uint32_t s_rxFrames;
static uint32_t s_txFrames;
static uint32_t s_rxDrop;
static uint32_t s_txDrop;
static volatile uint32_t s_rxIsrCount;
static volatile uint32_t s_rxIsrStamp;

/* ---- Helpers --------------------------------------------------------------- */

static void hal_eth_delay_us(uint32_t us)
{
    uint32_t t0 = DWT->CYCCNT;
    uint32_t cycles = us * HAL_ETH_CYCLES_PER_US;

    while ((DWT->CYCCNT - t0) < cycles)
    {
    }
}

static int32_t hal_eth_cache_len(uint32_t len)
{
    return (int32_t)((len + (HAL_ETH_CACHE_LINE - 1U))
            & ~(HAL_ETH_CACHE_LINE - 1U));
}

static uint32_t hal_eth_next(uint32_t idx, uint32_t num)
{
    idx++;
    return (idx >= num) ? 0U : idx;
}

/* ---- MDIO (PHY maintenance, NCFGR.CLK = MCK/64 = 2.34 MHz) ---------------- */

static bool hal_eth_mdio_idle(void)
{
    return (GMAC_REGS->GMAC_NSR & GMAC_NSR_IDLE_Msk) != 0U;
}

static void hal_eth_mdio_start(uint32_t op, uint32_t reg, uint16_t data)
{
    GMAC_REGS->GMAC_MAN = GMAC_MAN_CLTTO_Msk
            | GMAC_MAN_OP(op)
            | GMAC_MAN_PHYA(HAL_ETH_PHY_ADDR)
            | GMAC_MAN_REGA(reg)
            | GMAC_MAN_WTN(2U)
            | GMAC_MAN_DATA(data);
}

static uint16_t hal_eth_mdio_result(void)
{
    return (uint16_t)(GMAC_REGS->GMAC_MAN & GMAC_MAN_DATA_Msk);
}

static bool hal_eth_mdio_wait_idle(void)
{
    uint32_t t0 = DWT->CYCCNT;
    uint32_t limit = HAL_ETH_MDIO_TIMEOUT_US * HAL_ETH_CYCLES_PER_US;

    while (!hal_eth_mdio_idle())
    {
        if ((DWT->CYCCNT - t0) > limit)
        {
            return false;
        }
    }
    return true;
}

/* Blocking read: init phase only. */
static bool hal_eth_mdio_read_blocking(uint32_t reg, uint16_t *value)
{
    if (!hal_eth_mdio_wait_idle())
    {
        return false;
    }
    hal_eth_mdio_start(HAL_ETH_MAN_OP_READ, reg, 0U);
    if (!hal_eth_mdio_wait_idle())
    {
        return false;
    }
    *value = hal_eth_mdio_result();
    return true;
}

/* ---- MAC / DMA ------------------------------------------------------------- */

static void hal_eth_rings_init(void)
{
    uint32_t i;

    for (i = 0U; i < HAL_ETH_RX_DESC_NUM; i++)
    {
        uint32_t addr = (uint32_t)&s_rxBuf[i][0] & HAL_ETH_RXD_ADDR_MSK;
        if (i == (HAL_ETH_RX_DESC_NUM - 1U))
        {
            addr |= HAL_ETH_RXD_WRAP;
        }
        s_desc.rx[i].status = 0U;
        s_desc.rx[i].addr = addr;
    }

    for (i = 0U; i < HAL_ETH_TX_DESC_NUM; i++)
    {
        uint32_t status = HAL_ETH_TXD_USED;
        if (i == (HAL_ETH_TX_DESC_NUM - 1U))
        {
            status |= HAL_ETH_TXD_WRAP;
        }
        s_desc.tx[i].addr = (uint32_t)&s_txBuf[i][0];
        s_desc.tx[i].status = status;
    }

    s_desc.rxDummy.addr = HAL_ETH_RXD_OWN | HAL_ETH_RXD_WRAP;
    s_desc.rxDummy.status = 0U;
    s_desc.txDummy.addr = 0U;
    s_desc.txDummy.status = HAL_ETH_TXD_USED | HAL_ETH_TXD_WRAP;

    /* RX buffers were zeroed through the cache at start-up: write back and
     * drop those lines so an eviction can never overwrite DMA data. */
    SCB_CleanInvalidateDCache_by_Addr((void *)&s_rxBuf[0][0],
            (int32_t)sizeof(s_rxBuf));
    __DSB();

    GMAC_REGS->GMAC_RBQB = (uint32_t)&s_desc.rx[0];
    GMAC_REGS->GMAC_TBQB = (uint32_t)&s_desc.tx[0];
    for (i = 0U; i < HAL_ETH_PRIO_QUEUE_NUM; i++)
    {
        GMAC_REGS->GMAC_RBQBAPQ[i] = (uint32_t)&s_desc.rxDummy;
        GMAC_REGS->GMAC_TBQBAPQ[i] = (uint32_t)&s_desc.txDummy;
    }

    s_rxIdx = 0U;
    s_txHead = 0U;
    s_txTail = 0U;
    s_txPending = 0U;
}

static void hal_eth_mac_config(void)
{
    uint32_t i;

    GMAC_REGS->GMAC_NCR = 0U;
    GMAC_REGS->GMAC_IDR = 0xFFFFFFFFUL;
    for (i = 0U; i < HAL_ETH_PRIO_QUEUE_NUM; i++)
    {
        GMAC_REGS->GMAC_IDRPQ[i] = 0xFFFFFFFFUL;
    }
    GMAC_REGS->GMAC_NCR = GMAC_NCR_CLRSTAT_Msk;
    GMAC_REGS->GMAC_RSR = GMAC_RSR_BNA_Msk | GMAC_RSR_REC_Msk
            | GMAC_RSR_RXOVR_Msk | GMAC_RSR_HNO_Msk;
    GMAC_REGS->GMAC_TSR = GMAC_TSR_UBR_Msk | GMAC_TSR_COL_Msk
            | GMAC_TSR_RLE_Msk | GMAC_TSR_TXGO_Msk | GMAC_TSR_TFC_Msk
            | GMAC_TSR_TXCOMP_Msk | GMAC_TSR_HRESP_Msk;
    (void)GMAC_REGS->GMAC_ISR;
    for (i = 0U; i < HAL_ETH_PRIO_QUEUE_NUM; i++)
    {
        (void)GMAC_REGS->GMAC_ISRPQ[i];
    }

    /*
     * 100 Mbit/s full duplex until the PHY reports the negotiated mode,
     * MDC = MCK/64, strip FCS, accept 1536 B frames, copy all frames
     * (an EtherCAT master works promiscuously, like SOEM on Linux).
     */
    GMAC_REGS->GMAC_NCFGR = GMAC_NCFGR_SPD_Msk | GMAC_NCFGR_FD_Msk
            | GMAC_NCFGR_CLK(4U) | GMAC_NCFGR_RFCS_Msk
            | GMAC_NCFGR_MAXFS_Msk | GMAC_NCFGR_CAF_Msk;

    /* UR.RMII = 0 selects RMII on SAM V71 (same as the Harmony driver). */
    GMAC_REGS->GMAC_UR = GMAC_UR_RMII(0U);

    GMAC_REGS->GMAC_DCFGR = GMAC_DCFGR_DRBS(HAL_ETH_BUF_SIZE / 64U)
            | GMAC_DCFGR_RXBMS(3U) | GMAC_DCFGR_TXPBMS_Msk
            | GMAC_DCFGR_FBLDO_INCR4 | GMAC_DCFGR_DDRP_Msk;
    for (i = 0U; i < HAL_ETH_PRIO_QUEUE_NUM; i++)
    {
        GMAC_REGS->GMAC_RBSRPQ[i] = 1U;
    }

    /* Management port on; TX/RX enabled later. */
    GMAC_REGS->GMAC_NCR = GMAC_NCR_MPE_Msk;
}

static void hal_eth_mac_enable(bool enable)
{
    if (enable)
    {
        GMAC_REGS->GMAC_NCR |= (GMAC_NCR_TXEN_Msk | GMAC_NCR_RXEN_Msk);
    }
    else
    {
        GMAC_REGS->GMAC_NCR &= ~(GMAC_NCR_TXEN_Msk | GMAC_NCR_RXEN_Msk);
    }
}

/* Apply the negotiated mode; rings restart from their base (frames dropped). */
static void hal_eth_mac_apply_link(bool fullDuplex, bool speed100)
{
    uint32_t ncfgr;

    hal_eth_mac_enable(false);

    ncfgr = GMAC_REGS->GMAC_NCFGR & ~(GMAC_NCFGR_SPD_Msk | GMAC_NCFGR_FD_Msk);
    if (speed100)
    {
        ncfgr |= GMAC_NCFGR_SPD_Msk;
    }
    if (fullDuplex)
    {
        ncfgr |= GMAC_NCFGR_FD_Msk;
    }
    GMAC_REGS->GMAC_NCFGR = ncfgr;

    hal_eth_rings_init();
    hal_eth_mac_enable(true);
}

static void hal_eth_tx_reclaim(void)
{
    while ((s_txPending > 0U)
            && ((s_desc.tx[s_txTail].status & HAL_ETH_TXD_USED) != 0U))
    {
        s_txTail = hal_eth_next(s_txTail, HAL_ETH_TX_DESC_NUM);
        s_txPending--;
    }
}

/*
 * TSTART is ignored while the transmitter is active. If the GMAC read our
 * descriptor as "used" just before we queued it, it halts with frames
 * pending; restart it here.
 */
static void hal_eth_tx_kick(void)
{
    if ((s_txPending > 0U)
            && ((GMAC_REGS->GMAC_TSR & GMAC_TSR_TXGO_Msk) == 0U))
    {
        GMAC_REGS->GMAC_NCR |= GMAC_NCR_TSTART_Msk;
    }
}

static void hal_eth_resolve_link(uint16_t lpa)
{
    uint16_t common = (uint16_t)(lpa & s_phyAnar);
    bool fd = true;
    bool s100 = true;

    if ((common & HAL_ETH_AN_100FD) != 0U)
    {
        fd = true;
        s100 = true;
    }
    else if ((common & HAL_ETH_AN_100HD) != 0U)
    {
        fd = false;
        s100 = true;
    }
    else if ((common & HAL_ETH_AN_10FD) != 0U)
    {
        fd = true;
        s100 = false;
    }
    else
    {
        fd = false;
        s100 = false;
    }

    hal_eth_mac_apply_link(fd, s100);
    s_link.fullDuplex = fd;
    s_link.speedMbps = s100 ? 100U : 10U;
    s_link.up = true;
    s_linkChanges++;
}

static void hal_eth_link_poll(void)
{
    switch (s_mdioState)
    {
    case HAL_ETH_MDIO_IDLE:
        s_linkPollTicks++;
        if ((s_linkPollTicks >= HAL_ETH_LINK_POLL_TICKS) && hal_eth_mdio_idle())
        {
            s_linkPollTicks = 0U;
            hal_eth_mdio_start(HAL_ETH_MAN_OP_READ, HAL_ETH_PHY_BMSR, 0U);
            s_mdioState = HAL_ETH_MDIO_WAIT_BMSR;
        }
        break;

    case HAL_ETH_MDIO_WAIT_BMSR:
        if (hal_eth_mdio_idle())
        {
            uint16_t bmsr = hal_eth_mdio_result();
            bool up = ((bmsr & HAL_ETH_BMSR_LINK) != 0U)
                    && ((bmsr & HAL_ETH_BMSR_ANEG_DONE) != 0U);

            s_mdioState = HAL_ETH_MDIO_IDLE;
            if (up && !s_link.up)
            {
                hal_eth_mdio_start(HAL_ETH_MAN_OP_READ, HAL_ETH_PHY_ANLPAR, 0U);
                s_mdioState = HAL_ETH_MDIO_WAIT_ANLPAR;
            }
            else if (!up && s_link.up)
            {
                s_link.up = false;
                s_linkChanges++;
            }
            else
            {
                /* no change */
            }
        }
        break;

    case HAL_ETH_MDIO_WAIT_ANLPAR:
        if (hal_eth_mdio_idle())
        {
            hal_eth_resolve_link(hal_eth_mdio_result());
            s_mdioState = HAL_ETH_MDIO_IDLE;
        }
        break;

    default:
        s_mdioState = HAL_ETH_MDIO_IDLE;
        break;
    }
}

static void hal_eth_hw_stats(void)
{
    /* Statistics registers clear on read (GMAC statistics block). */
    s_rxDrop += GMAC_REGS->GMAC_RRE;   /* no free RX buffer */
    s_rxDrop += GMAC_REGS->GMAC_ROE;   /* receive overrun */
    GMAC_REGS->GMAC_RSR = GMAC_RSR_BNA_Msk | GMAC_RSR_REC_Msk
            | GMAC_RSR_RXOVR_Msk | GMAC_RSR_HNO_Msk;
}

/* ---- Interrupt: t1 marker only --------------------------------------------- */

void GMAC_InterruptHandler(void)
{
    uint32_t isr = GMAC_REGS->GMAC_ISR; /* clear on read */

    if ((isr & GMAC_ISR_RCOMP_Msk) != 0U)
    {
        /* t1: frame stored in RX memory (thesis Tab. 7.3). */
        s_rxIsrStamp = DWT->CYCCNT;
        s_rxIsrCount++;
        hal_marker_pulse(HAL_MARKER_T1, HAL_ETH_MARK_MIN_CYCLES);
    }
}

/* ---- Public API ------------------------------------------------------------ */

void hal_eth_init(void)
{
    uint16_t id1 = 0U;
    uint16_t id2 = 0U;

    (void)memset(&s_link, 0, sizeof(s_link));
    s_phyId = 0U;
    s_phyAnar = 0U;
    s_phyOk = false;
    s_mdioState = HAL_ETH_MDIO_IDLE;
    s_linkPollTicks = HAL_ETH_LINK_POLL_TICKS;
    s_linkChanges = 0U;
    s_rxFrames = 0U;
    s_txFrames = 0U;
    s_rxDrop = 0U;
    s_txDrop = 0U;
    s_rxIsrCount = 0U;
    s_rxIsrStamp = 0U;

    /* DWT cycle counter for the init delays (hal_time_init() runs later). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* GMAC peripheral clock (PMC_PCER1, ID_GMAC = 39); kept here so it does
     * not depend on the MCC clock page after the Harmony driver is removed. */
    PMC_REGS->PMC_PCER1 = (1UL << (ID_GMAC - 32U));

    /* PHY hardware reset (active low), straps are latched on release. */
    ETH_RESET_Clear();
    hal_eth_delay_us(HAL_ETH_PHY_RESET_US);
    ETH_RESET_Set();
    hal_eth_delay_us(HAL_ETH_PHY_STARTUP_US);

    hal_eth_mac_config();
    hal_eth_rings_init();

    if (hal_eth_mdio_read_blocking(HAL_ETH_PHY_ID1, &id1)
            && hal_eth_mdio_read_blocking(HAL_ETH_PHY_ID2, &id2)
            && hal_eth_mdio_read_blocking(HAL_ETH_PHY_ANAR, &s_phyAnar))
    {
        s_phyId = ((uint32_t)id1 << 16) | id2;
        s_phyOk = (id1 != 0xFFFFU) && (id1 != 0x0000U);
    }

    GMAC_REGS->GMAC_IER = GMAC_IER_RCOMP_Msk;
    NVIC_SetPriority(GMAC_IRQn, 7U);
    NVIC_EnableIRQ(GMAC_IRQn);

    hal_eth_mac_enable(true);
}

void hal_eth_deinit(void)
{
    GMAC_REGS->GMAC_IDR = 0xFFFFFFFFUL;
    NVIC_DisableIRQ(GMAC_IRQn);
    GMAC_REGS->GMAC_NCR = 0U;
    s_link.up = false;
}

void hal_eth_poll(void)
{
    hal_eth_tx_reclaim();
    hal_eth_tx_kick();

    if (s_phyOk)
    {
        hal_eth_link_poll();
        if (s_linkPollTicks == 0U)
        {
            hal_eth_hw_stats();
        }
    }
}

hal_eth_status_t hal_eth_tx(const uint8_t *frame, uint16_t len)
{
    uint32_t idx;
    uint32_t status;

    if ((frame == NULL) || (len < 14U) || (len > HAL_ETH_FRAME_MAX))
    {
        return HAL_ETH_ERR_PARAM;
    }
    if (!s_link.up)
    {
        s_txDrop++;
        return HAL_ETH_ERR_LINK;
    }

    hal_eth_tx_reclaim();
    if (s_txPending >= HAL_ETH_TX_DESC_NUM)
    {
        s_txDrop++;
        return HAL_ETH_ERR_BUSY;
    }

    idx = s_txHead;
    (void)memcpy(&s_txBuf[idx][0], frame, len);
    SCB_CleanDCache_by_Addr((void *)&s_txBuf[idx][0], hal_eth_cache_len(len));

    /* Short frames are padded to 60 B and the FCS appended by the GMAC. */
    status = ((uint32_t)len & HAL_ETH_TXD_LEN_MSK) | HAL_ETH_TXD_LAST;
    if (idx == (HAL_ETH_TX_DESC_NUM - 1U))
    {
        status |= HAL_ETH_TXD_WRAP;
    }

    {
        /* t4: hand-over of the frame to the GMAC (thesis Tab. 7.3). */
        uint32_t tMark = DWT->CYCCNT;
        hal_marker_set(HAL_MARKER_T4);

        __DMB();
        s_desc.tx[idx].status = status; /* USED = 0: owned by GMAC */
        __DSB();
        GMAC_REGS->GMAC_NCR |= GMAC_NCR_TSTART_Msk;

        while ((DWT->CYCCNT - tMark) < HAL_ETH_MARK_MIN_CYCLES)
        {
        }
        hal_marker_clear(HAL_MARKER_T4);
    }

    s_txHead = hal_eth_next(idx, HAL_ETH_TX_DESC_NUM);
    s_txPending++;
    s_txFrames++;
    return HAL_ETH_OK;
}

hal_eth_status_t hal_eth_rx(uint8_t *frame, uint16_t maxLen, uint16_t *len)
{
    uint32_t guard;

    if ((frame == NULL) || (len == NULL))
    {
        return HAL_ETH_ERR_PARAM;
    }
    *len = 0U;

    for (guard = 0U; guard < HAL_ETH_RX_DESC_NUM; guard++)
    {
        hal_eth_desc_t *d = &s_desc.rx[s_rxIdx];
        uint32_t word0 = d->addr;
        uint32_t status;
        uint32_t frameLen;
        hal_eth_status_t result = HAL_ETH_OK;

        if ((word0 & HAL_ETH_RXD_OWN) == 0U)
        {
            return HAL_ETH_ERR_EMPTY;
        }

        status = d->status;
        frameLen = status & HAL_ETH_RXD_LEN_MSK;

        if (((status & (HAL_ETH_RXD_SOF | HAL_ETH_RXD_EOF))
                    != (HAL_ETH_RXD_SOF | HAL_ETH_RXD_EOF))
                || (frameLen == 0U) || (frameLen > HAL_ETH_BUF_SIZE))
        {
            /* Not a single-buffer frame: drop and look at the next one. */
            s_rxDrop++;
            result = HAL_ETH_ERR_SIZE;
        }
        else if (frameLen > maxLen)
        {
            s_rxDrop++;
            result = HAL_ETH_ERR_SIZE;
        }
        else
        {
            SCB_InvalidateDCache_by_Addr((void *)&s_rxBuf[s_rxIdx][0],
                    hal_eth_cache_len(frameLen));
            (void)memcpy(frame, &s_rxBuf[s_rxIdx][0], frameLen);
            *len = (uint16_t)frameLen;
            s_rxFrames++;
        }

        /* Give the buffer back to the GMAC (clear ownership, keep WRAP). */
        __DMB();
        d->addr = word0 & ~HAL_ETH_RXD_OWN;
        s_rxIdx = hal_eth_next(s_rxIdx, HAL_ETH_RX_DESC_NUM);

        if (result == HAL_ETH_OK)
        {
            return HAL_ETH_OK;
        }
        if (frameLen > maxLen)
        {
            return HAL_ETH_ERR_SIZE;
        }
    }

    return HAL_ETH_ERR_EMPTY;
}

void hal_eth_link_get(hal_eth_link_t *link)
{
    if (link != NULL)
    {
        *link = s_link;
    }
}

uint32_t hal_eth_phy_id(void)
{
    return s_phyId;
}

bool hal_eth_sigdet(void)
{
    return ETH_SIGDET_Get() != 0U;
}

uint32_t hal_eth_rx_count(void)
{
    return s_rxFrames;
}

uint32_t hal_eth_tx_count(void)
{
    return s_txFrames;
}

uint32_t hal_eth_rx_drop_count(void)
{
    return s_rxDrop;
}

uint32_t hal_eth_tx_drop_count(void)
{
    return s_txDrop;
}

uint32_t hal_eth_rx_isr_count(void)
{
    return s_rxIsrCount;
}

uint32_t hal_eth_link_change_count(void)
{
    return s_linkChanges;
}
