#include "hwa_interface.h"
#include "hal_interface.h"

#include <string.h>

void hwa_init(void)
{
    hal_marker_init();
    hal_console_init();
    hal_time_init();
    hal_can_init();
}

void hwa_deinit(void)
{
    hal_can_deinit();
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
    hal_can_frame_t frame;

    if ((data == NULL) && (len > 0U))
    {
        return;
    }

    (void)memset(&frame, 0, sizeof(frame));
    frame.id = id;
    frame.sub_id = sub_id;
    frame.len = len;
    if ((data != NULL) && (len > 0U))
    {
        (void)memcpy(frame.data, data, (len > 8U) ? 8U : len);
        if (len > 8U)
        {
            frame.len = 8U;
        }
    }

    (void)hal_can_tx(&frame);
}

void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len)
{
    hal_can_frame_t frame;

    if ((id == NULL) || (sub_id == NULL) || (data == NULL) || (len == NULL))
    {
        return;
    }

    if (hal_can_rx(&frame) != HAL_CAN_OK)
    {
        *len = 0U;
        return;
    }

    *id = frame.id;
    *sub_id = frame.sub_id;
    *len = frame.len;
    if (frame.len > 0U)
    {
        (void)memcpy(data, frame.data, frame.len);
    }
}

uint32_t hwa_get_number_of_massages(void)
{
    return hal_can_rx_count();
}

uint32_t hwa_can_rx_overflow_count(void)
{
    return hal_can_rx_overflow_count();
}

uint32_t hwa_can_rx_high_water(void)
{
    return hal_can_rx_high_water();
}

uint32_t hwa_can_tx_drop_count(void)
{
    return hal_can_tx_drop_count();
}

bool hwa_can_is_bus_off(void)
{
    return hal_can_is_bus_off();
}
