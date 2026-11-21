/* Host stand-in for hwa_interface.h: only what conversion.c uses. */
#ifndef HWA_INTERFACE_H
#define HWA_INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
typedef enum { HAL_MARKER_T1 = 0, HAL_MARKER_T2, HAL_MARKER_T3, HAL_MARKER_T4 } hwa_marker_id_t;
void hwa_marker_pulse(hwa_marker_id_t id);
#endif
