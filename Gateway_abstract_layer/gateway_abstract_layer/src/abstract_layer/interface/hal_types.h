#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** HIL GPIO markers (chapter 8). Names match MCC pin labels where possible. */
typedef enum
{
    HAL_MARKER_T1 = 0,
    HAL_MARKER_T2,
    HAL_MARKER_T3,
    HAL_MARKER_T4,
    HAL_MARKER_LOOP,
    HAL_MARKER_STATE,
    /** Mapped to MCC pin MARK_BUFFER (bus-off / buffer events). */
    HAL_MARKER_BUSOFF,
    HAL_MARKER_COUNT
} hal_marker_id_t;

/** Monotonic timestamp in CPU cycles (DWT CYCCNT @ CPU clock). */
typedef uint32_t hal_cycles_t;

/** Free-running Tloop tick count since start. */
typedef uint32_t hal_tick_t;

/** Classic CAN frame (11-bit ID path used in Stage 2). */
typedef struct
{
    uint32_t id;
    uint32_t sub_id;
    uint8_t  data[8];
    uint8_t  len;
    uint16_t timestamp;
} hal_can_frame_t;

typedef enum
{
    HAL_CAN_OK = 0,
    HAL_CAN_ERR_BUSY,
    HAL_CAN_ERR_FULL,
    HAL_CAN_ERR_EMPTY,
    HAL_CAN_ERR_PARAM,
    HAL_CAN_ERR_BUSOFF
} hal_can_status_t;

#endif /* HAL_TYPES_H */
