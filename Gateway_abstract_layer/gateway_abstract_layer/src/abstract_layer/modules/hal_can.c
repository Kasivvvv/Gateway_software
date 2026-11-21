#include "hal_interface.h"
#include "hal_nocache.h"

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
/* ~2 µs @ 300 MHz — visible at a few MS/s on a logic analyser. */
#define HAL_CAN_MARK_MIN_CYCLES    (600U)

/*
 * MCAN Message RAM must not be D-cached: the MCAN DMA reads/writes it and the
 * MCC plib accesses it without cache maintenance. XC32 ignores
 * __attribute__((coherent)) on Cortex-M, so the buffer is pinned into the
 * non-cacheable window (hal_nocache.h, MCC MPU region 5).
 * DS60001527 ch. 48: Message RAM base = CCFG_CAN1.CAN1DMABA (upper 16 bits).
 */
_Static_assert(MCAN1_MESSAGE_RAM_CONFIG_SIZE <= HAL_NOCACHE_MCAN_MAX,
        "MCAN1 Message RAM exceeds its part of the non-cacheable window");

static uint8_t __attribute__((aligned(32), space(data),
        address(HAL_NOCACHE_MCAN_ADDR)))
    s_mcan1MsgRam[MCAN1_MESSAGE_RAM_CONFIG_SIZE];

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

    {
        uint32_t t0 = DWT->CYCCNT;
        while ((DWT->CYCCNT - t0) < HAL_CAN_MARK_MIN_CYCLES)
        {
        }
    }
    hal_marker_clear(HAL_MARKER_T1);
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
    MCAN1_CallbackRegister(hal_can_status_callback, (uintptr_t)NULL);

    hal_marker_clear(HAL_MARKER_BUSOFF);
}

void hal_can_deinit(void)
{
    MCAN1_RxFifoCallbackRegister(MCAN_RX_FIFO_0, NULL, (uintptr_t)NULL);
    MCAN1_CallbackRegister(NULL, (uintptr_t)NULL);
}

hal_can_status_t hal_can_tx(const hal_can_frame_t *frame)
{
    MCAN_TX_BUFFER txBuf;
    uint8_t slot;
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

    /*
     * Free slot = no pending request in MCAN_TXBRP (DS60001527 ch. 48, Tx Buffer Request
     * Pending). TXBRP is set by the TXBAR write and cleared by hardware after
     * transmission, so no software busy mask is needed. A mask cleared from the
     * TX-complete callback leaked slots: the MCC plib writes MCAN_TXBTIE with
     * '=' instead of '|=', disabling the interrupt of earlier pending buffers.
     * hal_can_tx() is called only from the superloop (single producer).
     */
    for (slot = 0U; slot < HAL_CAN_TX_DEDICATED_NUM; slot++)
    {
        if (!MCAN1_TxBufferIsBusy(slot))
        {
            break;
        }
    }

    if (slot >= HAL_CAN_TX_DEDICATED_NUM)
    {
        s_txDropCount++;
        return HAL_CAN_ERR_BUSY;
    }

    /* t4: hand-over to MCAN TX buffer (Table 7.3). */
    {
        uint32_t tMark = DWT->CYCCNT;
        hal_marker_set(HAL_MARKER_T4);
        if (!MCAN1_MessageTransmit(slot, &txBuf))
        {
            s_txDropCount++;
            hal_marker_clear(HAL_MARKER_T4);
            return HAL_CAN_ERR_BUSY;
        }
        while ((DWT->CYCCNT - tMark) < HAL_CAN_MARK_MIN_CYCLES)
        {
        }
        hal_marker_clear(HAL_MARKER_T4);
    }

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

void hal_can_diag_get(hal_can_diag_t *diag)
{
    uint32_t ecr;
    uint32_t psr;

    if (diag == NULL)
    {
        return;
    }

    ecr = MCAN1_REGS->MCAN_ECR;
    psr = MCAN1_REGS->MCAN_PSR; /* LEC is set to 7 by this read */

    diag->tec = (uint8_t)((ecr & MCAN_ECR_TEC_Msk) >> MCAN_ECR_TEC_Pos);
    diag->rec = (uint8_t)((ecr & MCAN_ECR_REC_Msk) >> MCAN_ECR_REC_Pos);
    diag->lec = (uint8_t)((psr & MCAN_PSR_LEC_Msk) >> MCAN_PSR_LEC_Pos);
    diag->errorPassive = (psr & MCAN_PSR_EP_Msk) != 0U;
    diag->busOff = (psr & MCAN_PSR_BO_Msk) != 0U;
}

/*
 * Bus-off recovery (re-initialisation of the communication, transition T10).
 * After bus-off MCAN sets CCCR.INIT; clearing it starts the recovery
 * sequence of 128 x 11 recessive bits (DS60001527 ch. 48, Bus-Off recovery).
 * Pending transmissions are cancelled so no stale frame is sent afterwards.
 */
void hal_can_recover(void)
{
    MCAN1_REGS->MCAN_TXBCR = 0xFFFFFFFFUL;
    if ((MCAN1_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) != 0U)
    {
        MCAN1_REGS->MCAN_CCCR &= ~MCAN_CCCR_INIT_Msk;
    }
    s_busOff = false;
    hal_marker_clear(HAL_MARKER_BUSOFF);
}
