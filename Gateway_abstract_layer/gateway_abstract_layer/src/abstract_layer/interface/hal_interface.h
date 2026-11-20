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

#ifdef __cplusplus
}
#endif

#endif /* HAL_INTERFACE_H */
