/**
 * @file    bsp_can.c
 * @brief   CAN底层驱动（STM32F103C8T6 / CAN1）
 * @author  肆叁伍12
 * @date    2025/12/27
 *
 * @note
 * 1) 本驱动采用“中断接收 + 队列缓存”的方式：
 *    - 在 CAN RX0 中断里快速读 FIFO0，组包后覆盖写入队列（只保留最新一帧）。
 *    - 应用层使用 can_read_message_block() 在任务上下文读取消息并做协议解析。
 * 2) 这样做的原因：
 *    - 中断里不做复杂解析，降低中断占用时间；
 *    - 事件组的清除/组合逻辑更适合在任务上下文处理。
 */

/* 头文件引用 */
#include "bsp_can.h"
#include "stm32f1xx_hal_gpio_ex.h"

/* 全局句柄 */
CAN_HandleTypeDef hcan1;

/* ============================== 内部资源 ============================== */
/**
 * @brief CAN 接收队列（ISR -> Task）
 * @note
 * - 队列长度设为 1，配合 xQueueOverwriteFromISR()：始终保留“最新一帧”。
 * - 本项目只关心“当前模式”（加速/减速/停车/正常），保留最新帧最合适。
 */
static QueueHandle_t CAN_RX_QUEUE = NULL;

/* ============================== 内部函数声明 ============================== */
static void bsp_can_gpio_init(void);
static void bsp_can_mode_init(void);
static void bsp_can_filter_init(void);
static void bsp_can_nvic_init(void);
static void bsp_can_rx_queue_init(void);

/* ============================== 内部函数定义 ============================== */
/**
 * @brief 初始化 CAN1 相关 GPIO 与 AFIO 重映射
 * @note
 * - TX：复用推挽输出
 * - RX：输入（上拉）
 */
static void bsp_can_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIOB 与 AFIO 时钟 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();

  /* CAN1 重映射到 PB8/PB9（CASE 2） */
  __HAL_AFIO_REMAP_CAN1_2();

  /* TX：PB9，复用推挽 */
  GPIO_InitStruct.Pin = BSP_CAN1_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BSP_CAN1_GPIOx, &GPIO_InitStruct);

  /* RX：PB8，输入（通常建议上拉，避免总线空闲时抖动） */
  GPIO_InitStruct.Pin = BSP_CAN1_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BSP_CAN1_GPIOx, &GPIO_InitStruct);
}

/**
 * @brief 初始化 CAN1 工作模式与波特率参数
 * @note
 * - 波特率由：APB1 时钟 / Prescaler / (1 + BS1 + BS2) 决定
 * - 如果你的系统时钟/分频与此处不同，需要相应调整 Prescaler/BS1/BS2
 */
static void bsp_can_mode_init(void)
{
  __HAL_RCC_CAN1_CLK_ENABLE();

  hcan1.Instance = CAN1;

  /* 基本模式配置 */
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;

  /* 位时序配置（请按你的CAN总线波特率需求调整） */
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.Prescaler = 6;

  /* 初始化 CAN 外设 */
  (void)HAL_CAN_Init(&hcan1);
}

/**
 * @brief 配置 CAN 过滤器
 * @note
 * - 目前配置为“全接收”（不过滤ID），将所有报文导入 FIFO0
 * - 后续如果你明确了控制帧 ID，可在此处改成按 ID 过滤，降低中断频率
 */
static void bsp_can_filter_init(void)
{
  CAN_FilterTypeDef sFilterConfig = {0};

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

  /* 全接收：ID 与 MASK 全部填 0 */
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;

  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14; /* 单 CAN 实例下该字段无意义，填默认即可 */

  (void)HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}

/**
 * @brief 配置 CAN1 RX0 中断优先级
 * @note
 * - 本项目在 FreeRTOS 下运行，若 ISR 内部调用 FromISR API，
 *   中断优先级必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY(=5)。
 * - 这里设置为 6，与 USART 中断风格保持一致。
 */
static void bsp_can_nvic_init(void)
{
  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

/**
 * @brief 初始化 CAN 接收队列
 */
static void bsp_can_rx_queue_init(void)
{
  if (CAN_RX_QUEUE == NULL)
  {
    CAN_RX_QUEUE = xQueueCreate(1, sizeof(can_message_t));
  }
}

/* ============================== 对外接口实现 ============================== */
void can_init(void)
{
  /* 先初始化队列，避免 CAN 中断先来导致写队列失败 */
  bsp_can_rx_queue_init();

  bsp_can_gpio_init();
  bsp_can_mode_init();
  bsp_can_filter_init();
  bsp_can_nvic_init();

  /* 启动 CAN 并开启 FIFO0 接收中断 */
  (void)HAL_CAN_Start(&hcan1);
  (void)HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

bool can_send_message(const can_message_t *msg)
{
  if (msg == NULL || msg->len > 8)
    return false;

  CAN_TxHeaderTypeDef TxHeader = {0};
  uint32_t TxMailbox = 0;

  /* 标准帧/扩展帧配置 */
  if (msg->extended_id)
  {
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.ExtId = msg->id;
  }
  else
  {
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.StdId = msg->id;
  }

  /* 数据帧/远程帧 */
  TxHeader.RTR = msg->remote_frame ? CAN_RTR_REMOTE : CAN_RTR_DATA;
  TxHeader.DLC = msg->len;
  TxHeader.TransmitGlobalTime = DISABLE;

  return (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, (uint8_t *)msg->data,
                              &TxMailbox) == HAL_OK);
}

bool can_read_message_block(can_message_t *msg, TickType_t timeout)
{
  if (msg == NULL || CAN_RX_QUEUE == NULL)
    return false;

  return (xQueueReceive(CAN_RX_QUEUE, msg, timeout) == pdTRUE);
}

bool can_read_message(can_message_t *msg) { return can_read_message_block(msg, 0); }

/* ============================== 中断与回调实现 ============================== */
/**
 * @brief CAN1 FIFO0 接收中断入口
 * @note 向量名为 USB_LP_CAN1_RX0_IRQHandler（见 startup_stm32f103xb.s）
 */
void USB_LP_CAN1_RX0_IRQHandler(void) { HAL_CAN_IRQHandler(&hcan1); }

/**
 * @brief CAN FIFO0 收到新报文回调（HAL 弱定义函数，用户可重写）
 * @note
 * - 在此处“只做搬运”：从 FIFO 读出 -> 组装 can_message_t -> 覆盖写入队列。
 * - 不在中断里做复杂业务解析，避免影响系统实时性。
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (hcan == NULL || hcan->Instance != CAN1 || CAN_RX_QUEUE == NULL)
    return;

  CAN_RxHeaderTypeDef RxHeader = {0};
  uint8_t rx_data[8] = {0};

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rx_data) != HAL_OK)
    return;

  can_message_t msg = {0};

  msg.extended_id = (RxHeader.IDE == CAN_ID_EXT);
  msg.remote_frame = (RxHeader.RTR == CAN_RTR_REMOTE);
  msg.id = msg.extended_id ? RxHeader.ExtId : RxHeader.StdId;
  msg.len = (RxHeader.DLC <= 8) ? (uint8_t)RxHeader.DLC : 8;

  for (uint8_t i = 0; i < msg.len; i++)
    msg.data[i] = rx_data[i];

  /* 覆盖写入：只保留最新一帧 */
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueOverwriteFromISR(CAN_RX_QUEUE, &msg, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
