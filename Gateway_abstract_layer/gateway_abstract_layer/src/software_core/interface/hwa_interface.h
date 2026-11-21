#ifndef HWA_INTERFACE_H
#define HWA_INTERFACE_H

#include "hwa_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void hwa_init(void);
void hwa_deinit(void);

/** Start-up check: DMA buffers of CAN/Ethernet are in non-cacheable memory. */
bool       hwa_mem_dma_nocache_ok(void);

/* Time ------------------------------------------------------------------ */
/** Start Tloop; call once at the end of start-up. */
void       hwa_time_start(void);
bool       hwa_time_tick_take(void);
hwa_tick_t hwa_time_tick_count(void);
uint32_t   hwa_time_overrun_count(void);
hwa_cycles_t hwa_time_cycles(void);
uint32_t   hwa_time_cpu_hz(void);
uint32_t   hwa_time_tloop_us(void);
/** Monotonic microseconds since start-up (64-bit). */
uint64_t   hwa_time_now_us(void);

/* Markers --------------------------------------------------------------- */
void hwa_marker_set(hwa_marker_id_t id);
void hwa_marker_clear(hwa_marker_id_t id);
void hwa_marker_toggle(hwa_marker_id_t id);
void hwa_marker_enable(bool enabled);

/* Status LED ------------------------------------------------------------ */
void hwa_led_set(bool on);
void hwa_led_toggle(void);

/* Console --------------------------------------------------------------- */
size_t hwa_console_write(const uint8_t *data, size_t len);
size_t hwa_console_write_str(const char *str);
size_t hwa_console_read(uint8_t *data, size_t len);
/** Call once per Tloop: drains the console TX queue (non-blocking). */
void   hwa_console_poll(void);

/* CAN ------------------------------------------------------------------- */
/** Transmit classic frame. sub_id kept for CANopen OD path; unused on wire. */
void hwa_can_tx(uint32_t id, uint32_t sub_id, uint8_t *data, uint8_t len);

/** Receive one queued frame; sub_id set to 0 for raw Stage-2 frames. */
void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len);

/** Number of frames waiting in the RX software queue. */
uint32_t hwa_get_number_of_massages(void);

uint32_t hwa_can_rx_overflow_count(void);
uint32_t hwa_can_rx_high_water(void);
uint32_t hwa_can_tx_drop_count(void);
bool     hwa_can_is_bus_off(void);
void     hwa_can_diag_get(hwa_can_diag_t *diag);

/* ETHERCAT -------------------------------------------------------------- */
/* Raw Ethernet frames (GMAC); the EtherCAT stack runs on top of these. */

/** Call once per Tloop: PHY link supervision and TX housekeeping. */
void     hwa_eth_poll(void);

/** Queue one frame (14..HWA_ETH_FRAME_MAX bytes); false if dropped. */
bool     hwa_eth_tx(const uint8_t *frame, uint16_t len);

/** Copy one received frame; returns its length, 0 if none is waiting. */
uint16_t hwa_eth_rx(uint8_t *frame, uint16_t maxLen);

void     hwa_eth_link_get(hwa_eth_link_t *link);
uint32_t hwa_eth_phy_id(void);
bool     hwa_eth_sigdet(void);
uint32_t hwa_eth_rx_count(void);
uint32_t hwa_eth_tx_count(void);
uint32_t hwa_eth_rx_drop_count(void);
uint32_t hwa_eth_tx_drop_count(void);
uint32_t hwa_eth_rx_isr_count(void);
uint32_t hwa_eth_link_change_count(void);

/* USB ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* HWA_INTERFACE_H */
