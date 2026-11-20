#include "hwa_interface.h"
#include "hal_interface.h"

void hwa_init(void)
{
    hal_marker_init();
    hal_console_init();
    hal_time_init();
}

void hwa_deinit(void)
{
    hal_time_deinit();
    hal_console_deinit();
}

bool hwa_time_tick_take(void)
{
    return hal_time_tick_take();
}

hwa_tick_t hwa_time_tick_count(void)
{
    return hal_time_tick_count();
}

uint32_t hwa_time_overrun_count(void)
{
    return hal_time_overrun_count();
}

hwa_cycles_t hwa_time_cycles(void)
{
    return hal_time_cycles();
}

uint32_t hwa_time_cpu_hz(void)
{
    return hal_time_cpu_hz();
}

uint32_t hwa_time_tloop_us(void)
{
    return hal_time_tloop_us();
}

void hwa_marker_set(hwa_marker_id_t id)
{
    hal_marker_set(id);
}

void hwa_marker_clear(hwa_marker_id_t id)
{
    hal_marker_clear(id);
}

void hwa_marker_toggle(hwa_marker_id_t id)
{
    hal_marker_toggle(id);
}

void hwa_marker_enable(bool enabled)
{
    hal_marker_enable(enabled);
}

size_t hwa_console_write(const uint8_t *data, size_t len)
{
    return hal_console_write(data, len);
}

size_t hwa_console_write_str(const char *str)
{
    return hal_console_write_str(str);
}

size_t hwa_console_read(uint8_t *data, size_t len)
{
    return hal_console_read(data, len);
}

void hwa_can_tx(uint32_t id, uint32_t sub_id, uint8_t *data, uint8_t len)
{
    (void)id;
    (void)sub_id;
    (void)data;
    (void)len;
}

void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len)
{
    (void)id;
    (void)sub_id;
    (void)data;
    (void)len;
}

void hwa_get_number_of_massages(void)
{
}
