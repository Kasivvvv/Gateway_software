#include "hal_interface.h"

#include <string.h>

#include "peripheral/usart/plib_usart1.h"

/*
 * TX path: software queue -> USART1 plib ring buffer (128 B in MCC) -> wire.
 * The plib ring alone is too small for one stats report, and USART1_Write()
 * silently drops what does not fit. hal_console_write() only queues;
 * hal_console_poll() (once per Tloop) moves as much as the plib ring accepts.
 * Non-blocking; 115200 Bd drains ~11.5 B/ms. Main context only.
 */
#define HAL_CONSOLE_TXQ_SIZE  (2048U) /* power of two */
#define HAL_CONSOLE_TXQ_MASK  (HAL_CONSOLE_TXQ_SIZE - 1U)

_Static_assert((HAL_CONSOLE_TXQ_SIZE & HAL_CONSOLE_TXQ_MASK) == 0U,
        "console queue size must be a power of two");

static uint8_t  s_txq[HAL_CONSOLE_TXQ_SIZE];
static uint32_t s_txqHead;    /* next write position (free-running) */
static uint32_t s_txqTail;    /* next byte to hand to the plib */
static uint32_t s_txqDropped;

void hal_console_init(void)
{
    /* USART1_Initialize() already called from SYS_Initialize(). */
    s_txqHead = 0U;
    s_txqTail = 0U;
    s_txqDropped = 0U;
}

void hal_console_deinit(void)
{
}

void hal_console_poll(void)
{
    uint32_t pass;

    /* At most two passes: the queued data may wrap around the array end. */
    for (pass = 0U; pass < 2U; pass++)
    {
        uint32_t queued = s_txqHead - s_txqTail;
        uint32_t idx = s_txqTail & HAL_CONSOLE_TXQ_MASK;
        uint32_t chunk = HAL_CONSOLE_TXQ_SIZE - idx;
        size_t room = USART1_WriteFreeBufferCountGet();
        size_t written;

        if ((queued == 0U) || (room == 0U))
        {
            break;
        }
        if (chunk > queued)
        {
            chunk = queued;
        }
        if (chunk > room)
        {
            chunk = (uint32_t)room;
        }

        written = USART1_Write(&s_txq[idx], chunk);
        s_txqTail += (uint32_t)written;
        if (written < chunk)
        {
            break;
        }
    }
}

size_t hal_console_write(const uint8_t *data, size_t len)
{
    uint32_t space;
    uint32_t n;
    uint32_t i;

    if ((data == NULL) || (len == 0U))
    {
        return 0U;
    }

    space = HAL_CONSOLE_TXQ_SIZE - (s_txqHead - s_txqTail);
    n = (len < space) ? (uint32_t)len : space;
    for (i = 0U; i < n; i++)
    {
        s_txq[(s_txqHead + i) & HAL_CONSOLE_TXQ_MASK] = data[i];
    }
    s_txqHead += n;
    s_txqDropped += (uint32_t)len - n;

    hal_console_poll();
    return n;
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
    return HAL_CONSOLE_TXQ_SIZE - (s_txqHead - s_txqTail);
}

uint32_t hal_console_tx_dropped(void)
{
    return s_txqDropped;
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

    hal_led_set(true);
    for (;;)
    {
    }
}
