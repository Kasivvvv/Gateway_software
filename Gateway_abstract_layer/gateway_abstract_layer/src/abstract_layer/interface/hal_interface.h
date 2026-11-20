#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include "hal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Time (Tloop + cycle timestamps) ------------------------------------ */

void hal_time_init(void);
void hal_time_deinit(void);

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

/* ---- HIL markers -------------------------------------------------------- */

void hal_marker_init(void);
void hal_marker_set(hal_marker_id_t id);
void hal_marker_clear(hal_marker_id_t id);
void hal_marker_toggle(hal_marker_id_t id);

/** When disabled (CAL-01), set/clear/toggle become no-ops. */
void hal_marker_enable(bool enabled);
bool hal_marker_is_enabled(void);

/* ---- Console (USART1 / EDBG VCOM, non-blocking) ------------------------- */

void hal_console_init(void);
void hal_console_deinit(void);

/** Queue bytes for TX; returns number accepted into the ring buffer. */
size_t hal_console_write(const uint8_t *data, size_t len);

/** Queue a NUL-terminated string. */
size_t hal_console_write_str(const char *str);

size_t hal_console_read(uint8_t *data, size_t len);
size_t hal_console_tx_free(void);

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

#ifdef __cplusplus
}
#endif

#endif /* HAL_INTERFACE_H */
