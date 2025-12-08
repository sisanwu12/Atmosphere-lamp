#include "bsp_can.h"
#include "bsp_usart.h"
#include <stdio.h>

void *can_rb = NULL;
CAN_HandleTypeDef hcan1;

// GPIO配置
void can_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();

  // TX引脚
  GPIO_InitStruct.Pin = CAN_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CAN_TX_PORT, &GPIO_InitStruct);

  // RX引脚
  GPIO_InitStruct.Pin = CAN_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CAN_RX_PORT, &GPIO_InitStruct);
}

// CAN模式配置
void can_mode_config(void)
{
  __HAL_RCC_CAN1_CLK_ENABLE();

  hcan1.Instance = CAN1;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;

  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.Prescaler = 6;

  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    printf("CAN Init failed\n");
  }
}

// CAN过滤器配置
static void can_filter_config(void)
{
  CAN_FilterTypeDef sFilterConfig;

  // 过滤器0：动力域 (0x0000-0x00FF)
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  // 过滤器1：底盘与安全域 (0x0100-0x01FF)
  sFilterConfig.FilterBank = 1;
  sFilterConfig.FilterIdHigh = 0x0100 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  // 过滤器2：车身域 (0x0200-0x02FF)
  sFilterConfig.FilterBank = 2;
  sFilterConfig.FilterIdHigh = 0x0200 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  // 过滤器3：信息娱乐域 (0x0300-0x03FF)
  sFilterConfig.FilterBank = 3;
  sFilterConfig.FilterIdHigh = 0x0300 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  printf("CAN过滤器配置完成：按域范围过滤\r\n");
}

// CAN初始化
void can_init(void)
{
  can_gpio_config();
  can_mode_config();
  can_filter_config();

  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

// 中断服务函数
void CAN1_RX0_IRQHandler(void) { HAL_CAN_IRQHandler(&hcan1); }
