#ifndef HWA_INTERFACE_H
#define HWA_INTERFACE_H

#include "hwa_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void hwa_init(void);
void hwa_deinit(void);

/* Time ------------------------------------------------------------------ */
bool       hwa_time_tick_take(void);
hwa_tick_t hwa_time_tick_count(void);
uint32_t   hwa_time_overrun_count(void);
hwa_cycles_t hwa_time_cycles(void);
uint32_t   hwa_time_cpu_hz(void);
uint32_t   hwa_time_tloop_us(void);

/* Markers --------------------------------------------------------------- */
void hwa_marker_set(hwa_marker_id_t id);
void hwa_marker_clear(hwa_marker_id_t id);
void hwa_marker_toggle(hwa_marker_id_t id);
void hwa_marker_enable(bool enabled);

/* Console --------------------------------------------------------------- */
size_t hwa_console_write(const uint8_t *data, size_t len);
size_t hwa_console_write_str(const char *str);
size_t hwa_console_read(uint8_t *data, size_t len);

/* CAN ------------------------------------------------------------------- */
void hwa_can_tx(uint32_t id, uint32_t sub_id, uint8_t *data, uint8_t len);
void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len);
void hwa_get_number_of_massages(void);

/* ETHERCAT -------------------------------------------------------------- */

/* USB ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* HWA_INTERFACE_H */
