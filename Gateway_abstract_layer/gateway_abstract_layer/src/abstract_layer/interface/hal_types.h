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

#endif /* HAL_TYPES_H */
