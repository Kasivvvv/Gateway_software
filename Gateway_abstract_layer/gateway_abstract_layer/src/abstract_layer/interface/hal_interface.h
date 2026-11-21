#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include "hal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Time (Tloop + cycle timestamps) ------------------------------------ */

void hal_time_init(void);
void hal_time_deinit(void);
/** Start the Tloop timer (call at the end of start-up). */
void hal_time_start(void);

/** True once per Tloop period; clears the pending flag (non-blocking). */
bool hal_time_tick_take(void);

/** Ticks observed since init (includes overruns still pending). */
hal_tick_t hal_time_tick_count(void);

/** Number of Tloop periods that fired while a previous tick was still pending. */
uint32_t hal_time_overrun_count(void);

hal_cycles_t hal_time_cycles(void);

/** CPU cycle clock frequency in Hz (DWT reference). */
uint32_t hal_time_cpu_hz(void);

/** Nominal Tloop period in microseconds. */
uint32_t hal_time_tloop_us(void);

/** Monotonic time since hal_time_init() in microseconds (64-bit, no wrap). */
uint64_t hal_time_now_us(void);

/* ---- HIL markers -------------------------------------------------------- */

void hal_marker_init(void);
void hal_marker_set(hal_marker_id_t id);
void hal_marker_clear(hal_marker_id_t id);
void hal_marker_toggle(hal_marker_id_t id);

/**
 * Pulse a marker for at least minCycles (DWT). Used so a logic analyser
 * at a few MS/s can reliably see t1/t4 edges.
 */
void hal_marker_pulse(hal_marker_id_t id, uint32_t minCycles);

/** When disabled (CAL-01), set/clear/toggle become no-ops. */
void hal_marker_enable(bool enabled);
bool hal_marker_is_enabled(void);

/* ---- Status LED (LED0) -------------------------------------------------- */

void hal_led_set(bool on);
void hal_led_toggle(void);

/* ---- Console (USART1 / EDBG VCOM, non-blocking) ------------------------- */

void hal_console_init(void);
void hal_console_deinit(void);

/** Queue bytes for TX; returns number accepted into the ring buffer. */
size_t hal_console_write(const uint8_t *data, size_t len);

/** Queue a NUL-terminated string. */
size_t hal_console_write_str(const char *str);

size_t hal_console_read(uint8_t *data, size_t len);
size_t hal_console_tx_free(void);

/** Once per Tloop: move queued TX bytes into the USART ring (non-blocking). */
void     hal_console_poll(void);
/** Bytes dropped because the TX queue was full. */
uint32_t hal_console_tx_dropped(void);

/* ---- CAN (MCAN1 / PLIB, non-blocking) ----------------------------------- */

void hal_can_init(void);
void hal_can_deinit(void);

/** Queue one classic frame for TX via dedicated buffers (R4). */
hal_can_status_t hal_can_tx(const hal_can_frame_t *frame);

/** Pop one received frame from the software RX queue. */
hal_can_status_t hal_can_rx(hal_can_frame_t *frame);

uint32_t hal_can_rx_count(void);
uint32_t hal_can_rx_overflow_count(void);
uint32_t hal_can_rx_high_water(void);
uint32_t hal_can_tx_drop_count(void);
bool     hal_can_is_bus_off(void);
/** Error counters and last error code (reading clears the LEC). */
void     hal_can_diag_get(hal_can_diag_t *diag);

/* ---- Memory ------------------------------------------------------------- */

/** True if the MPU makes the DMA window (hal_nocache.h) Normal non-cacheable. */
bool hal_nocache_region_ok(void);

/* ---- Ethernet (GMAC + KSZ8061 PHY, raw frames, non-blocking) ----------- */

/** Clock, PHY reset, MAC and DMA rings. Blocks ~25 ms (init phase only). */
void hal_eth_init(void);
void hal_eth_deinit(void);

/** Once per Tloop: PHY link state machine (MDIO), TX reclaim/restart. */
void hal_eth_poll(void);

/** Copy one frame into a free TX buffer and start DMA. */
hal_eth_status_t hal_eth_tx(const uint8_t *frame, uint16_t len);

/** Copy the oldest received frame out; HAL_ETH_ERR_EMPTY if none. */
hal_eth_status_t hal_eth_rx(uint8_t *frame, uint16_t maxLen, uint16_t *len);

void     hal_eth_link_get(hal_eth_link_t *link);
uint32_t hal_eth_phy_id(void);
bool     hal_eth_sigdet(void);
uint32_t hal_eth_rx_count(void);
uint32_t hal_eth_tx_count(void);
uint32_t hal_eth_rx_drop_count(void);
uint32_t hal_eth_tx_drop_count(void);
uint32_t hal_eth_rx_isr_count(void);
uint32_t hal_eth_link_change_count(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_INTERFACE_H */
