#include "hwa_interface.h"
#include "hal_interface.h"

#include <string.h>

void hwa_init(void)
{
    hal_marker_init();
    hal_console_init();
    /* Before hal_time_init(): blocks ~20 ms (PHY reset), no Tloop overruns. */
    hal_eth_init();
    hal_time_init();
    hal_can_init();
}

void hwa_deinit(void)
{
    hal_eth_deinit();
    hal_can_deinit();
    hal_time_deinit();
    hal_console_deinit();
}

bool hwa_mem_dma_nocache_ok(void)
{
    return hal_nocache_region_ok();
}

void hwa_time_start(void)
{
    hal_time_start();
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

uint64_t hwa_time_now_us(void)
{
    return hal_time_now_us();
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

void hwa_led_set(bool on)
{
    hal_led_set(on);
}

void hwa_led_toggle(void)
{
    hal_led_toggle();
}

void hwa_marker_pulse(hwa_marker_id_t id)
{
    /* ~2 us @ 300 MHz, same width as the t1/t4 markers in the drivers */
    hal_marker_pulse(id, 600U);
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

void hwa_console_poll(void)
{
    hal_console_poll();
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

void hwa_can_diag_get(hwa_can_diag_t *diag)
{
    hal_can_diag_get(diag);
}

void hwa_eth_poll(void)
{
    hal_eth_poll();
}

bool hwa_eth_tx(const uint8_t *frame, uint16_t len)
{
    return hal_eth_tx(frame, len) == HAL_ETH_OK;
}

uint16_t hwa_eth_rx(uint8_t *frame, uint16_t maxLen)
{
    uint16_t len = 0U;

    if (hal_eth_rx(frame, maxLen, &len) != HAL_ETH_OK)
    {
        return 0U;
    }
    return len;
}

void hwa_eth_link_get(hwa_eth_link_t *link)
{
    hal_eth_link_get(link);
}

uint32_t hwa_eth_phy_id(void)
{
    return hal_eth_phy_id();
}

bool hwa_eth_sigdet(void)
{
    return hal_eth_sigdet();
}

uint32_t hwa_eth_rx_count(void)
{
    return hal_eth_rx_count();
}

uint32_t hwa_eth_tx_count(void)
{
    return hal_eth_tx_count();
}

uint32_t hwa_eth_rx_drop_count(void)
{
    return hal_eth_rx_drop_count();
}

uint32_t hwa_eth_tx_drop_count(void)
{
    return hal_eth_tx_drop_count();
}

uint32_t hwa_eth_rx_isr_count(void)
{
    return hal_eth_rx_isr_count();
}

uint32_t hwa_eth_link_change_count(void)
{
    return hal_eth_link_change_count();
}

void hwa_can_recover(void)
{
    hal_can_recover();
}

void hwa_system_reset(void)
{
    hal_console_flush(100000U);
    hal_system_reset();
}
