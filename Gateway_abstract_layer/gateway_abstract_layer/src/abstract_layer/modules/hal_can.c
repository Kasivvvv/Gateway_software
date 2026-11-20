#include "hal_interface.h"

#include <string.h>

#include "definitions.h"
#include "peripheral/mcan/plib_mcan1.h"

/*
 * MCAN1 Classic CAN @ 1 Mbit/s (MCC).
 * Datasheet online DS60001527:
 *   48.4.2  CAN clock (PCK5 = 40 MHz)
 *   48.5.4  Rx FIFO
 *   48.5.5  Dedicated Tx buffers — lowest ID first among pending
 */

#define HAL_CAN_RX_QUEUE_DEPTH     (64U)
#define HAL_CAN_TX_DEDICATED_NUM   (32U)
#define HAL_CAN_STD_ID_SHIFT       (18U)

static uint8_t CACHE_ALIGN __attribute__((coherent))
    s_mcan1MsgRam[MCAN1_MESSAGE_RAM_CONFIG_SIZE];

static volatile uint32_t s_txBusyMask = 0U;
static volatile uint32_t s_txDropCount = 0U;
static volatile bool     s_busOff = false;

static hal_can_frame_t s_rxQueue[HAL_CAN_RX_QUEUE_DEPTH];
static volatile uint32_t s_rxHead = 0U;
static volatile uint32_t s_rxTail = 0U;
static volatile uint32_t s_rxCount = 0U;
static volatile uint32_t s_rxOverflow = 0U;
static volatile uint32_t s_rxHighWater = 0U;

static uint32_t hal_can_std_id_to_mcan(uint32_t stdId)
{
    return (stdId & 0x7FFU) << HAL_CAN_STD_ID_SHIFT;
}

static uint32_t hal_can_mcan_to_std_id(uint32_t mcanId)
{
    return (mcanId >> HAL_CAN_STD_ID_SHIFT) & 0x7FFU;
}

static uint8_t hal_can_dlc_encode(uint8_t len)
{
    if (len > 8U)
    {
        return 8U;
    }
    return len;
}

static uint8_t hal_can_dlc_decode(uint8_t dlc)
{
    if (dlc > 8U)
    {
        return 8U;
    }
    return dlc;
}

static void hal_can_rx_push_isr(const MCAN_RX_BUFFER *rxBuf)
{
    uint32_t nextHead;
    uint32_t count;

    if (rxBuf->xtd != 0U)
    {
        return;
    }

    if (s_rxCount >= HAL_CAN_RX_QUEUE_DEPTH)
    {
        s_rxOverflow++;
        return;
    }

    s_rxQueue[s_rxHead].id = hal_can_mcan_to_std_id(rxBuf->id);
    s_rxQueue[s_rxHead].sub_id = 0U;
    s_rxQueue[s_rxHead].len = hal_can_dlc_decode((uint8_t)rxBuf->dlc);
    s_rxQueue[s_rxHead].timestamp = (uint16_t)rxBuf->rxts;
    (void)memcpy(s_rxQueue[s_rxHead].data, rxBuf->data, s_rxQueue[s_rxHead].len);

    nextHead = s_rxHead + 1U;
    if (nextHead >= HAL_CAN_RX_QUEUE_DEPTH)
    {
        nextHead = 0U;
    }
    s_rxHead = nextHead;
    s_rxCount++;

    count = s_rxCount;
    if (count > s_rxHighWater)
    {
        s_rxHighWater = count;
    }
}

static void hal_can_rx_fifo_callback(uint8_t numberOfMessage, uintptr_t context)
{
    MCAN_RX_BUFFER rxBuf;
    uint8_t i;

    (void)context;

    hal_marker_set(HAL_MARKER_T1);

    for (i = 0U; i < numberOfMessage; i++)
    {
        if (MCAN1_MessageReceiveFifo(MCAN_RX_FIFO_0, 1U, &rxBuf))
        {
            hal_can_rx_push_isr(&rxBuf);
        }
    }

    hal_marker_clear(HAL_MARKER_T1);
}

static void hal_can_tx_buffer_callback(uint8_t bufferNumber, uintptr_t context)
{
    (void)context;

    if (bufferNumber < HAL_CAN_TX_DEDICATED_NUM)
    {
        s_txBusyMask &= ~(1UL << bufferNumber);
    }
}

static void hal_can_status_callback(uint32_t interruptStatus, uintptr_t context)
{
    (void)context;

    /* BO (Bus_Off Status Interrupt) — MCAN_IR.BO */
    if ((interruptStatus & MCAN_IR_BO_Msk) != 0U)
    {
        s_busOff = true;
        hal_marker_set(HAL_MARKER_BUSOFF);
    }
}

void hal_can_init(void)
{
    s_txBusyMask = 0U;
    s_txDropCount = 0U;
    s_busOff = false;
    s_rxHead = 0U;
    s_rxTail = 0U;
    s_rxCount = 0U;
    s_rxOverflow = 0U;
    s_rxHighWater = 0U;

    /* MCAN1_Initialize() already ran in SYS_Initialize(); configure Message RAM. */
    MCAN1_MessageRAMConfigSet(s_mcan1MsgRam);

    MCAN1_RxFifoCallbackRegister(MCAN_RX_FIFO_0, hal_can_rx_fifo_callback, (uintptr_t)NULL);
    MCAN1_TxBuffersCallbackRegister(hal_can_tx_buffer_callback, (uintptr_t)NULL);
    MCAN1_CallbackRegister(hal_can_status_callback, (uintptr_t)NULL);

    hal_marker_clear(HAL_MARKER_BUSOFF);
}

void hal_can_deinit(void)
{
    MCAN1_RxFifoCallbackRegister(MCAN_RX_FIFO_0, NULL, (uintptr_t)NULL);
    MCAN1_TxBuffersCallbackRegister(NULL, (uintptr_t)NULL);
    MCAN1_CallbackRegister(NULL, (uintptr_t)NULL);
}

hal_can_status_t hal_can_tx(const hal_can_frame_t *frame)
{
    MCAN_TX_BUFFER txBuf;
    uint8_t slot;
    bool irqState;
    uint8_t len;

    if ((frame == NULL) || (frame->len > 8U))
    {
        return HAL_CAN_ERR_PARAM;
    }

    if (s_busOff)
    {
        return HAL_CAN_ERR_BUSOFF;
    }

    len = frame->len;
    (void)memset(&txBuf, 0, sizeof(txBuf));
    txBuf.id = hal_can_std_id_to_mcan(frame->id);
    txBuf.rtr = 0U;
    txBuf.xtd = 0U;
    txBuf.esi = 0U;
    txBuf.dlc = hal_can_dlc_encode(len);
    txBuf.brs = 0U;
    txBuf.fdf = 0U;
    txBuf.efc = 0U;
    txBuf.mm = 0U;
    if (len > 0U)
    {
        (void)memcpy(txBuf.data, frame->data, len);
    }

    irqState = NVIC_INT_Disable();
    for (slot = 0U; slot < HAL_CAN_TX_DEDICATED_NUM; slot++)
    {
        if (((s_txBusyMask >> slot) & 1UL) == 0UL)
        {
            if (!MCAN1_TxBufferIsBusy(slot))
            {
                s_txBusyMask |= (1UL << slot);
                break;
            }
        }
    }
    NVIC_INT_Restore(irqState);

    if (slot >= HAL_CAN_TX_DEDICATED_NUM)
    {
        s_txDropCount++;
        return HAL_CAN_ERR_BUSY;
    }

    /* t4: hand-over to MCAN TX buffer (Table 7.3). */
    hal_marker_set(HAL_MARKER_T4);
    if (!MCAN1_MessageTransmit(slot, &txBuf))
    {
        irqState = NVIC_INT_Disable();
        s_txBusyMask &= ~(1UL << slot);
        NVIC_INT_Restore(irqState);
        s_txDropCount++;
        hal_marker_clear(HAL_MARKER_T4);
        return HAL_CAN_ERR_BUSY;
    }
    hal_marker_clear(HAL_MARKER_T4);

    (void)frame->sub_id; /* reserved for CANopen OD path */
    return HAL_CAN_OK;
}

hal_can_status_t hal_can_rx(hal_can_frame_t *frame)
{
    bool irqState;

    if (frame == NULL)
    {
        return HAL_CAN_ERR_PARAM;
    }

    irqState = NVIC_INT_Disable();
    if (s_rxCount == 0U)
    {
        NVIC_INT_Restore(irqState);
        return HAL_CAN_ERR_EMPTY;
    }

    *frame = s_rxQueue[s_rxTail];
    s_rxTail++;
    if (s_rxTail >= HAL_CAN_RX_QUEUE_DEPTH)
    {
        s_rxTail = 0U;
    }
    s_rxCount--;
    NVIC_INT_Restore(irqState);

    return HAL_CAN_OK;
}

uint32_t hal_can_rx_count(void)
{
    return s_rxCount;
}

uint32_t hal_can_rx_overflow_count(void)
{
    return s_rxOverflow;
}

uint32_t hal_can_rx_high_water(void)
{
    return s_rxHighWater;
}

uint32_t hal_can_tx_drop_count(void)
{
    return s_txDropCount;
}

bool hal_can_is_bus_off(void)
{
    return s_busOff;
}
