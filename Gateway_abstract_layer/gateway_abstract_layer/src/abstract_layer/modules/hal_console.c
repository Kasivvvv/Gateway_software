#include "hal_interface.h"

#include <string.h>

#include "peripheral/usart/plib_usart1.h"

void hal_console_init(void)
{
    /* USART1_Initialize() already called from SYS_Initialize(). */
}

void hal_console_deinit(void)
{
}

size_t hal_console_write(const uint8_t *data, size_t len)
{
    if ((data == NULL) || (len == 0U))
    {
        return 0U;
    }

    return USART1_Write((uint8_t *)data, len);
}

size_t hal_console_write_str(const char *str)
{
    if (str == NULL)
    {
        return 0U;
    }

    return hal_console_write((const uint8_t *)str, strlen(str));
}

size_t hal_console_read(uint8_t *data, size_t len)
{
    if ((data == NULL) || (len == 0U))
    {
        return 0U;
    }

    return USART1_Read(data, len);
}

size_t hal_console_tx_free(void)
{
    return USART1_WriteFreeBufferCountGet();
}
