#ifndef ETH_PROBE_H
#define ETH_PROBE_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Stage 4a raw-Ethernet probe (replaced by the SOEM master in Stage 4b).
 * Sends an EtherCAT NOP frame (EtherType 0x88A4) every ETH_PROBE_TX_PERIOD
 * ticks while the link is up and counts received frames.
 */

#define ETH_PROBE_TX_PERIOD_TICKS (5U) /* 1 ms @ Tloop 200 us */

void eth_probe_init(void);
void eth_probe_step(void);

uint32_t eth_probe_tx_seq(void);
uint32_t eth_probe_rx_frames(void);
uint32_t eth_probe_rx_ecat_frames(void);

#endif /* ETH_PROBE_H */
