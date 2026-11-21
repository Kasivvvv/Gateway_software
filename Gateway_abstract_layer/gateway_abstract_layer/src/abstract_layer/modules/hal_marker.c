#include "hal_interface.h"

#include "definitions.h"
#include "peripheral/pio/plib_pio.h"

/*
 * Pin map from MCC (plib_pio.h):
 *   MARK_T1 PB3, MARK_T2 PB2, MARK_T3 PA0, MARK_T4 PC30,
 *   MARK_LOOP PC13, MARK_STATE PA5, MARK_BUFFER PA25 (used as BUSOFF).
 *
 * Status LED0 = PA23, active low (SAM V71 Xplained Ultra user guide). The pin
 * is configured by MCC (function LED_AL, initial level high = off); it is
 * driven through the generic plib_pio API so the code does not depend on the
 * BSP macro names, which changed between Harmony BSP versions.
 */
#define HAL_LED0_PIN  PIO_PIN_PA23

static bool s_markersEnabled = true;

void hal_marker_init(void)
{
    s_markersEnabled = true;

    MARK_T1_Clear();
    MARK_T2_Clear();
    MARK_T3_Clear();
    MARK_T4_Clear();
    MARK_LOOP_Clear();
    MARK_STATE_Clear();
    MARK_BUFFER_Clear();
}

void hal_marker_enable(bool enabled)
{
    s_markersEnabled = enabled;
    if (!enabled)
    {
        MARK_T1_Clear();
        MARK_T2_Clear();
        MARK_T3_Clear();
        MARK_T4_Clear();
        MARK_LOOP_Clear();
        MARK_STATE_Clear();
        MARK_BUFFER_Clear();
    }
}

bool hal_marker_is_enabled(void)
{
    return s_markersEnabled;
}

void hal_marker_set(hal_marker_id_t id)
{
    if (!s_markersEnabled)
    {
        return;
    }

    switch (id)
    {
        case HAL_MARKER_T1:     MARK_T1_Set();     break;
        case HAL_MARKER_T2:     MARK_T2_Set();     break;
        case HAL_MARKER_T3:     MARK_T3_Set();     break;
        case HAL_MARKER_T4:     MARK_T4_Set();     break;
        case HAL_MARKER_LOOP:   MARK_LOOP_Set();   break;
        case HAL_MARKER_STATE:  MARK_STATE_Set();  break;
        case HAL_MARKER_BUSOFF: MARK_BUFFER_Set(); break;
        default:                                   break;
    }
}

void hal_marker_clear(hal_marker_id_t id)
{
    if (!s_markersEnabled)
    {
        return;
    }

    switch (id)
    {
        case HAL_MARKER_T1:     MARK_T1_Clear();     break;
        case HAL_MARKER_T2:     MARK_T2_Clear();     break;
        case HAL_MARKER_T3:     MARK_T3_Clear();     break;
        case HAL_MARKER_T4:     MARK_T4_Clear();     break;
        case HAL_MARKER_LOOP:   MARK_LOOP_Clear();   break;
        case HAL_MARKER_STATE:  MARK_STATE_Clear();  break;
        case HAL_MARKER_BUSOFF: MARK_BUFFER_Clear(); break;
        default:                                     break;
    }
}

void hal_marker_toggle(hal_marker_id_t id)
{
    if (!s_markersEnabled)
    {
        return;
    }

    switch (id)
    {
        case HAL_MARKER_T1:     MARK_T1_Toggle();     break;
        case HAL_MARKER_T2:     MARK_T2_Toggle();     break;
        case HAL_MARKER_T3:     MARK_T3_Toggle();     break;
        case HAL_MARKER_T4:     MARK_T4_Toggle();     break;
        case HAL_MARKER_LOOP:   MARK_LOOP_Toggle();   break;
        case HAL_MARKER_STATE:  MARK_STATE_Toggle();  break;
        case HAL_MARKER_BUSOFF: MARK_BUFFER_Toggle(); break;
        default:                                      break;
    }
}

void hal_marker_pulse(hal_marker_id_t id, uint32_t minCycles)
{
    uint32_t t0;

    if (!s_markersEnabled)
    {
        return;
    }

    t0 = DWT->CYCCNT;
    hal_marker_set(id);
    while ((DWT->CYCCNT - t0) < minCycles)
    {
    }
    hal_marker_clear(id);
}

void hal_led_set(bool on)
{
    /* Active low. */
    PIO_PinWrite(HAL_LED0_PIN, !on);
}

void hal_led_toggle(void)
{
    PIO_PinToggle(HAL_LED0_PIN);
}
