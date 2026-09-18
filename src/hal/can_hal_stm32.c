/* STM32 implementation of can_hal.h.
   Excluded from the host test build - see test/CMakeLists.txt.

   Wire-up notes (Nucleo-F446RE):
     PA11 = CAN1_RX, PA12 = CAN1_TX -> transceiver -> twisted pair, 120R each end.
     500 kbit/s with APB1 at 45 MHz: prescaler 5, BS1 = 13TQ, BS2 = 2TQ, SJW = 1TQ.
*/

#include "can_hal.h"

#ifdef STM32_TARGET
#include "stm32f4xx_hal.h"

static CAN_HandleTypeDef hcan1;

bool can_hal_init(uint32_t bitrate_bps)
{
    (void)bitrate_bps;   /* TODO: derive timing from bitrate */

    hcan1.Instance                  = CAN1;
    hcan1.Init.Prescaler            = 5;
    hcan1.Init.Mode                 = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1             = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2             = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode    = DISABLE;
    hcan1.Init.AutoBusOff           = ENABLE;
    hcan1.Init.AutoWakeUp           = DISABLE;
    hcan1.Init.AutoRetransmission   = ENABLE;
    hcan1.Init.ReceiveFifoLocked    = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&hcan1) != HAL_OK) return false;
    return HAL_CAN_Start(&hcan1) == HAL_OK;
}

bool can_hal_send(const can_frame_t *frame)
{
    CAN_TxHeaderTypeDef tx;
    uint32_t mailbox;

    if (frame == 0) return false;

    tx.ExtId = frame->id;
    tx.IDE   = CAN_ID_EXT;
    tx.RTR   = CAN_RTR_DATA;
    tx.DLC   = frame->dlc;
    tx.TransmitGlobalTime = DISABLE;

    return HAL_CAN_AddTxMessage(&hcan1, &tx,
                                (uint8_t *)frame->data, &mailbox) == HAL_OK;
}

bool can_hal_recv(can_frame_t *frame)
{
    CAN_RxHeaderTypeDef rx;

    if (frame == 0) return false;
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0u) return false;
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx, frame->data) != HAL_OK) {
        return false;
    }

    frame->id  = rx.ExtId;
    frame->dlc = (uint8_t)rx.DLC;
    return true;
}

uint32_t can_hal_millis(void) { return HAL_GetTick(); }

#endif /* STM32_TARGET */
