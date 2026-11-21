/* Host stand-in for src/software_core/interface/hwa_interface.h (only what
 * the EtherCAT master and its SOEM port use). */
#ifndef HWA_INTERFACE_H
#define HWA_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct { bool up; bool fullDuplex; uint16_t speedMbps; } hwa_eth_link_t;
#define HWA_ETH_FRAME_MAX 1514U

bool     hwa_eth_tx(const uint8_t *frame, uint16_t len);
uint16_t hwa_eth_rx(uint8_t *frame, uint16_t maxLen);
void     hwa_eth_link_get(hwa_eth_link_t *link);
uint64_t hwa_time_now_us(void);
size_t   hwa_console_write_str(const char *str);

#endif
