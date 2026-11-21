#include "eth_probe.h"

#include <string.h>

#include "hwa_interface.h"

#define ETH_PROBE_RX_MAX_PER_STEP (8U)
#define ETH_PROBE_ETHERTYPE_ECAT  (0x88A4U)

/*
 * Frame layout (ETG.1000.4): Ethernet header, EtherCAT header
 * (11-bit length + type 1), one NOP datagram with 4 data bytes.
 */
#define ETH_PROBE_ETH_HDR_LEN     (14U)
#define ETH_PROBE_ECAT_HDR_LEN    (2U)
#define ETH_PROBE_DG_HDR_LEN      (10U)
#define ETH_PROBE_DG_DATA_LEN     (4U)
#define ETH_PROBE_DG_WKC_LEN      (2U)
#define ETH_PROBE_DG_LEN          (ETH_PROBE_DG_HDR_LEN + ETH_PROBE_DG_DATA_LEN \
                                   + ETH_PROBE_DG_WKC_LEN)
#define ETH_PROBE_FRAME_LEN       (ETH_PROBE_ETH_HDR_LEN + ETH_PROBE_ECAT_HDR_LEN \
                                   + ETH_PROBE_DG_LEN)

typedef struct
{
    uint32_t ticks;
    uint32_t txSeq;
    uint32_t rxFrames;
    uint32_t rxEcatFrames;
    uint8_t  txFrame[ETH_PROBE_FRAME_LEN];
    uint8_t  rxFrame[HWA_ETH_FRAME_MAX];
} eth_probe_ctx_t;

static eth_probe_ctx_t s_probe;

static void eth_probe_build_frame(void)
{
    static const uint8_t header[ETH_PROBE_ETH_HDR_LEN] =
    {
        0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, /* broadcast, as SOEM */
        0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x10U, /* locally administered */
        0x88U, 0xA4U
    };
    uint8_t *p = s_probe.txFrame;
    uint16_t ecatHdr = (uint16_t)(ETH_PROBE_DG_LEN | (1U << 12));

    (void)memset(s_probe.txFrame, 0, sizeof(s_probe.txFrame));
    (void)memcpy(p, header, sizeof(header));
    p += ETH_PROBE_ETH_HDR_LEN;

    p[0] = (uint8_t)(ecatHdr & 0xFFU);
    p[1] = (uint8_t)(ecatHdr >> 8);
    p += ETH_PROBE_ECAT_HDR_LEN;

    /* cmd NOP (0), idx 0, address 0, len 4, irq 0; data/WKC set per frame. */
    p[0] = 0x00U;
    p[1] = 0x00U;
    p[6] = (uint8_t)(ETH_PROBE_DG_DATA_LEN & 0xFFU);
    p[7] = 0x00U;
}

static void eth_probe_send(void)
{
    uint8_t *data = &s_probe.txFrame[ETH_PROBE_ETH_HDR_LEN
            + ETH_PROBE_ECAT_HDR_LEN + ETH_PROBE_DG_HDR_LEN];
    uint32_t seq = s_probe.txSeq;

    data[0] = (uint8_t)(seq & 0xFFU);
    data[1] = (uint8_t)((seq >> 8) & 0xFFU);
    data[2] = (uint8_t)((seq >> 16) & 0xFFU);
    data[3] = (uint8_t)((seq >> 24) & 0xFFU);
    s_probe.txFrame[ETH_PROBE_ETH_HDR_LEN + ETH_PROBE_ECAT_HDR_LEN + 1U] =
            (uint8_t)(seq & 0xFFU); /* datagram index */

    if (hwa_eth_tx(s_probe.txFrame, (uint16_t)sizeof(s_probe.txFrame)))
    {
        s_probe.txSeq++;
    }
}

static void eth_probe_receive(void)
{
    uint32_t n;

    for (n = 0U; n < ETH_PROBE_RX_MAX_PER_STEP; n++)
    {
        uint16_t len = hwa_eth_rx(s_probe.rxFrame, (uint16_t)sizeof(s_probe.rxFrame));
        uint16_t type;

        if (len == 0U)
        {
            break;
        }
        s_probe.rxFrames++;

        if (len >= ETH_PROBE_ETH_HDR_LEN)
        {
            type = (uint16_t)(((uint16_t)s_probe.rxFrame[12] << 8)
                    | s_probe.rxFrame[13]);
            if (type == ETH_PROBE_ETHERTYPE_ECAT)
            {
                s_probe.rxEcatFrames++;
            }
        }
    }
}

void eth_probe_init(void)
{
    (void)memset(&s_probe, 0, sizeof(s_probe));
    eth_probe_build_frame();
}

void eth_probe_step(void)
{
    hwa_eth_link_t link;

    eth_probe_receive();

    hwa_eth_link_get(&link);
    s_probe.ticks++;
    if (s_probe.ticks >= ETH_PROBE_TX_PERIOD_TICKS)
    {
        s_probe.ticks = 0U;
        if (link.up)
        {
            eth_probe_send();
        }
    }
}

uint32_t eth_probe_tx_seq(void)
{
    return s_probe.txSeq;
}

uint32_t eth_probe_rx_frames(void)
{
    return s_probe.rxFrames;
}

uint32_t eth_probe_rx_ecat_frames(void)
{
    return s_probe.rxEcatFrames;
}
