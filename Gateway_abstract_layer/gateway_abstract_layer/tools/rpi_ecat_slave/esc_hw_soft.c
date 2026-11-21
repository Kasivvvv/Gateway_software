/*
 * SOES hardware layer (ESC_read/ESC_write/ESC_init/ESC_reset) on top of the
 * software ESC (src/abstract_layer/modules/esc/soft_esc.c), PDI side.
 * Replaces soes/hal/<board>/esc_hw.c. Mirrors the LAN9252 HAL behaviour of
 * refreshing ESCvar.ALevent after every access.
 */
#include <string.h>

#include "esc.h"
#include "slave_app.h"

static void esc_refresh_alevent(void)
{
    uint16_t alevent;

    soft_esc_pdi_read(slave_app_esc(), ESCREG_ALEVENT, &alevent, sizeof(alevent));
    ESCvar.ALevent = etohs(alevent);
}

void ESC_read(uint16_t address, void *buf, uint16_t len)
{
    soft_esc_pdi_read(slave_app_esc(), address, buf, len);
    esc_refresh_alevent();
}

void ESC_write(uint16_t address, void *buf, uint16_t len)
{
    soft_esc_pdi_write(slave_app_esc(), address, buf, len);
    esc_refresh_alevent();
}

void ESC_reset(void)
{
}

void ESC_init(const esc_cfg_t *config)
{
    (void)config; /* soft-ESC is initialised by slave_app_init() */
}
