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
#include <stdio.h>

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
static void bsp_can_gpio_clock_enable(GPIO_TypeDef *GPIOx);
static bool bsp_can_calc_bit_timing(uint32_t pclk1_hz, uint32_t baudrate_bps);

/* ============================== 内部函数定义 ============================== */
/**
 * @brief GPIO 时钟使能（根据 GPIO 组自动选择 RCC）
 * @note 这里不直接依赖 bsp_gpio.c 的 static 函数，避免跨模块耦合。
 */
static void bsp_can_gpio_clock_enable(GPIO_TypeDef *GPIOx)
{
  switch ((uint32_t)GPIOx)
  {
  case (uint32_t)GPIOA:
    __HAL_RCC_GPIOA_CLK_ENABLE();
    break;
  case (uint32_t)GPIOB:
    __HAL_RCC_GPIOB_CLK_ENABLE();
    break;
  case (uint32_t)GPIOC:
    __HAL_RCC_GPIOC_CLK_ENABLE();
    break;
  case (uint32_t)GPIOD:
    __HAL_RCC_GPIOD_CLK_ENABLE();
    break;
  case (uint32_t)GPIOE:
    __HAL_RCC_GPIOE_CLK_ENABLE();
    break;
  default:
    break;
  }
}

/**
 * @brief 根据当前 PCLK1 与目标波特率搜索一组可用位时序
 * @param pclk1_hz      APB1 外设时钟（Hz）
 * @param baudrate_bps  目标波特率（bps）
 * @return true 找到可用参数并写入 hcan1.Init；false 未找到（保持原参数不变）
 *
 * @note
 * - STM32 CAN 位时序约束：
 *   - Prescaler：1~1024
 *   - BS1：1~16TQ
 *   - BS2：1~8TQ
 *   - 总 TQ = 1(SYNC) + BS1 + BS2，通常 8~25 之间较常见
 * - 这里使用“整除搜索”，确保波特率完全匹配，避免与其他节点长期漂移。
 * - 以采样点 87.5% 为目标（常见经验值），在可行解中选“采样点最接近且 TQ 更大”的组合。
 */
static bool bsp_can_calc_bit_timing(uint32_t pclk1_hz, uint32_t baudrate_bps)
{
  if (pclk1_hz == 0 || baudrate_bps == 0)
    return false;

  static const uint32_t bs1_map[16] = {
      CAN_BS1_1TQ,  CAN_BS1_2TQ,  CAN_BS1_3TQ,  CAN_BS1_4TQ,
      CAN_BS1_5TQ,  CAN_BS1_6TQ,  CAN_BS1_7TQ,  CAN_BS1_8TQ,
      CAN_BS1_9TQ,  CAN_BS1_10TQ, CAN_BS1_11TQ, CAN_BS1_12TQ,
      CAN_BS1_13TQ, CAN_BS1_14TQ, CAN_BS1_15TQ, CAN_BS1_16TQ,
  };
  static const uint32_t bs2_map[8] = {
      CAN_BS2_1TQ, CAN_BS2_2TQ, CAN_BS2_3TQ, CAN_BS2_4TQ,
      CAN_BS2_5TQ, CAN_BS2_6TQ, CAN_BS2_7TQ, CAN_BS2_8TQ,
  };

  /* 目标采样点：87.5%（用千分比表示，避免浮点） */
  const uint32_t target_sp_x1000 = 875U;

  bool found = false;
  uint32_t best_err = 0xFFFFFFFFU;
  uint8_t best_tq = 0;
  uint8_t best_bs1 = 0;
  uint8_t best_bs2 = 0;
  uint32_t best_prescaler = 0;

  /* 搜索常见 TQ 范围：8~25 */
  for (uint8_t tq = 8; tq <= 25; tq++)
  {
    for (uint8_t bs2 = 1; bs2 <= 8; bs2++)
    {
      int bs1 = (int)tq - 1 - (int)bs2;
      if (bs1 < 1 || bs1 > 16)
        continue;

      uint32_t denom = baudrate_bps * (uint32_t)tq;
      if (denom == 0)
        continue;

      /* 仅接受“完全整除”的组合 */
      if ((pclk1_hz % denom) != 0)
        continue;

      uint32_t prescaler = pclk1_hz / denom;
      if (prescaler < 1 || prescaler > 1024)
        continue;

      /* 采样点 = (1 + BS1) / TQ */
      uint32_t sp_x1000 = (1000U * (uint32_t)(1 + bs1)) / (uint32_t)tq;
      uint32_t err = (sp_x1000 > target_sp_x1000)
                         ? (sp_x1000 - target_sp_x1000)
                         : (target_sp_x1000 - sp_x1000);

      /* 先比误差，再比 TQ（TQ 大通常更稳健） */
      if (!found || err < best_err || (err == best_err && tq > best_tq))
      {
        found = true;
        best_err = err;
        best_tq = tq;
        best_bs1 = (uint8_t)bs1;
        best_bs2 = bs2;
        best_prescaler = prescaler;
      }
    }
  }

  if (!found)
    return false;

  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = bs1_map[best_bs1 - 1];
  hcan1.Init.TimeSeg2 = bs2_map[best_bs2 - 1];
  hcan1.Init.Prescaler = best_prescaler;
  return true;
}

/**
 * @brief 初始化 CAN1 相关 GPIO 与 AFIO 重映射
 * @note
 * - TX：复用推挽输出
 * - RX：输入（上拉）
 */
static void bsp_can_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO 与 AFIO 时钟 */
  bsp_can_gpio_clock_enable(BSP_CAN1_GPIOx);
  __HAL_RCC_AFIO_CLK_ENABLE();

  /* CAN1 引脚映射选择（CASE 1/2/3） */
#if (BSP_CAN1_REMAP_CASE == 1)
  __HAL_AFIO_REMAP_CAN1_1();
#elif (BSP_CAN1_REMAP_CASE == 2)
  __HAL_AFIO_REMAP_CAN1_2();
#elif (BSP_CAN1_REMAP_CASE == 3)
  __HAL_AFIO_REMAP_CAN1_3();
#endif

  /* TX：复用推挽 */
  GPIO_InitStruct.Pin = BSP_CAN1_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BSP_CAN1_GPIOx, &GPIO_InitStruct);

  /* RX：输入（通常建议上拉，避免总线空闲时抖动） */
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

  /* 工作模式（正常/回环/静默回环等） */
  hcan1.Init.Mode = BSP_CAN_MODE;

  /* 位时序：根据 PCLK1 与目标波特率自动搜索一组可行参数 */
  uint32_t pclk1_hz = HAL_RCC_GetPCLK1Freq();
  bool ok = bsp_can_calc_bit_timing(pclk1_hz, BSP_CAN_BAUDRATE);
  if (!ok)
  {
    /**
     * 找不到可整除参数时的兜底：
     * - 这里保留一组“比较常用”的默认值，便于你快速定位问题；
     * - 但强烈建议：通过调整系统时钟/目标波特率，保证能找到“整除解”。
     */
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.Prescaler = 6;
  }

  /* 初始化 CAN 外设 */
  (void)HAL_CAN_Init(&hcan1);

#if BSP_CAN_DEBUG_PRINT
  /* 仅用于联调：打印当前配置，帮助确认“引脚/模式/波特率”是否与你的硬件一致 */
  {
    /* BTR 的 TS1/TS2 字段存的是 “(实际TQ数 - 1)” */
    uint32_t ts1 = ((hcan1.Init.TimeSeg1 & CAN_BTR_TS1) >> CAN_BTR_TS1_Pos) + 1U;
    uint32_t ts2 = ((hcan1.Init.TimeSeg2 & CAN_BTR_TS2) >> CAN_BTR_TS2_Pos) + 1U;
    uint32_t tq = 1U + ts1 + ts2;
    uint32_t br = 0;
    if (tq != 0 && hcan1.Init.Prescaler != 0)
      br = pclk1_hz / (hcan1.Init.Prescaler * tq);
    printf("[CAN] PCLK1=%lu Hz, target=%lu bps, actual=%lu bps, TQ=%lu, PSC=%lu, mode=%lu, remap=%d\r\n",
           (unsigned long)pclk1_hz, (unsigned long)BSP_CAN_BAUDRATE,
           (unsigned long)br, (unsigned long)tq,
           (unsigned long)hcan1.Init.Prescaler, (unsigned long)BSP_CAN_MODE,
           (int)BSP_CAN1_REMAP_CASE);
  }
#endif
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
  /**
   * 开启通知：
   * - RX FIFO0：用于接收
   * - ERROR 系列：用于定位“无收发器/无ACK/波特率不匹配/总线未接”等常见问题
   *
   * @note
   * - 错误回调发生在中断上下文，切勿在回调里 printf（可能导致阻塞/重入）。
   * - 本工程在 app_can 任务里会周期性读取 HAL_CAN_GetError() 进行打印。
   */
  (void)HAL_CAN_ActivateNotification(
      &hcan1,
      CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE |
          CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR);
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
