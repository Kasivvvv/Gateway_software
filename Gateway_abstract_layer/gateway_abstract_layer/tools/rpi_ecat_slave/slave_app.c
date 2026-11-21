#include "slave_app.h"

#include "ecat_slv.h"
#include "utypes.h"
#include "slave_sii.h"

_Objects Obj;                    /* object dictionary storage (utypes.h) */

static soft_esc_t s_esc;
static uint32_t s_outputUpdates;

soft_esc_t *slave_app_esc(void)
{
    return &s_esc;
}

/* SOES callbacks */
void cb_get_inputs(void)
{
    Obj.Buttons.Button1 = (uint8_t)(Obj.LEDs.LED0 + 1U);
}

void cb_set_outputs(void)
{
    s_outputUpdates++;
}

void slave_app_init(uint32_t watchdogCalls)
{
    static esc_cfg_t config =
    {
        .user_arg = NULL,
        .use_interrupt = 0,
        .watchdog_cnt = 0,
        .set_defaults_hook = NULL,
        .pre_state_change_hook = NULL,
        .post_state_change_hook = NULL,
        .application_hook = NULL,
        .safeoutput_override = NULL,
        .pre_object_download_hook = NULL,
        .post_object_download_hook = NULL,
        .rxpdo_override = NULL,
        .txpdo_override = NULL,
        .esc_hw_interrupt_enable = NULL,
        .esc_hw_interrupt_disable = NULL,
        .esc_hw_eep_handler = NULL,
        .esc_check_dc_handler = NULL,
    };

    soft_esc_init(&s_esc, slave_sii, (uint16_t)slave_sii_len);
    config.watchdog_cnt = (int)watchdogCalls;
    ecat_slv_init(&config);
}

void slave_app_poll(void)
{
    ecat_slv();
}

uint8_t slave_app_al_status(void)
{
    uint8_t st;

    soft_esc_pdi_read(&s_esc, 0x0130U, &st, 1U);
    return st;
}

uint32_t slave_app_outputs_updates(void)
{
    return s_outputUpdates;
}
