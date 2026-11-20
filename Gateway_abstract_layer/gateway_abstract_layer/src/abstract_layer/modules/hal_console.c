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

/*
 * Fatal assert reporter.
 *
 * XC32 (musl) maps a failed assert() to __assert_fail(); the library default
 * stops silently, which looks like "the board does nothing". This override
 * prints "ASSERT <file>:<line> <expr>" on the EDBG console by polling USART1
 * (DS60001527 ch. 46 USART, US_CSR.TXRDY / US_THR) with interrupts disabled,
 * so it also works from an ISR, then halts with LED0 on.
 */
#include "definitions.h"

static void hal_console_poll_str(const char *str)
{
    if (str == NULL)
    {
        return;
    }
    while (*str != '\0')
    {
        while ((USART1_REGS->US_CSR & US_CSR_USART_TXRDY_Msk) == 0U)
        {
        }
        USART1_REGS->US_THR = (uint32_t)(uint8_t)*str;
        str++;
    }
}

static void hal_console_poll_u32(uint32_t value)
{
    char tmp[11];
    uint32_t n = 0U;
    char out[2] = { '\0', '\0' };

    do
    {
        tmp[n++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while ((value > 0U) && (n < sizeof(tmp)));

    while (n > 0U)
    {
        out[0] = tmp[--n];
        hal_console_poll_str(out);
    }
}

void __assert_fail(const char *expr, const char *file, int line,
        const char *func)
{
    (void)func;
    __disable_irq();

    hal_console_poll_str("\r\nASSERT ");
    hal_console_poll_str(file);
    hal_console_poll_str(":");
    hal_console_poll_u32((uint32_t)line);
    hal_console_poll_str(" ");
    hal_console_poll_str(expr);
    hal_console_poll_str("\r\n");

    LED0_On();
    for (;;)
    {
    }
}
