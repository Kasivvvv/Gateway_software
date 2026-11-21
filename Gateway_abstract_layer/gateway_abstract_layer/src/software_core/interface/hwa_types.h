#ifndef HWA_TYPES_H
#define HWA_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hal_types.h"

typedef hal_marker_id_t   hwa_marker_id_t;
typedef hal_cycles_t      hwa_cycles_t;
typedef hal_tick_t        hwa_tick_t;
typedef hal_can_frame_t   hwa_can_frame_t;
typedef hal_can_status_t  hwa_can_status_t;
typedef hal_eth_link_t    hwa_eth_link_t;
typedef hal_can_diag_t    hwa_can_diag_t;

#define HWA_ETH_FRAME_MAX HAL_ETH_FRAME_MAX

#endif /* HWA_TYPES_H */
