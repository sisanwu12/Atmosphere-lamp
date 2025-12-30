/**
 * @file    app_can.c
 * @brief   CAN 协议解析与事件触发模块
 * @author  肆叁伍12
 * @date    2025/12/27
 *
 * @note
 * 1) 为什么要在任务里做“事件位清除/设置”？
 *    - CAN 接收发生在中断上下文；FreeRTOS EventGroup 没有 ClearBitsFromISR，
 *      如果直接在中断里 SetBits，会导致旧事件位可能残留并与新事件位叠加。
 *    - 因此本模块在任务上下文统一维护“点阵灯模式事件”，确保：
 *      - 同一时刻 EVT_UP/EVT_DOWN/EVT_STOP 只会有一个有效（或全部清空为 NORMAL）。
 *
 * 2) 协议来源：
 *    - doc/datasheet/can总线通信帧格式.png
 *    - 其中 Byte2 表示“点阵灯的模式”：
 *      0x00 加速，0x01 减速，0x02 停车，0x03 正常
 */

#define __APP_CAN_C

/* 头文件引用 */
#include "app_can.h"
#include "FreeRTOS.h"
#include "bsp_can.h"
#include "event_bus.h"
#include "task.h"
#include <stdio.h>

/* ============================== 可选调试配置 ============================== */
/**
 * @brief 是否打印 CAN 联调日志
 * @note
 * - 你已经实现了 printf 重定向，因此这里可以直接 printf 到串口；
 * - 如果你担心影响实时性，可以将其改为 0。
 */
#ifndef APP_CAN_DEBUG_PRINT
#define APP_CAN_DEBUG_PRINT 1
#endif

/**
 * @brief CAN 接收等待超时（ms）
 * @note
 * - 原实现使用 portMAX_DELAY 一直等报文；
 * - 这里改成“有限等待”，这样即使一直收不到数据，也能周期性打印错误码，方便定位问题。
 */
#ifndef APP_CAN_RX_WAIT_MS
#define APP_CAN_RX_WAIT_MS 500
#endif

/**
 * @brief 是否开启“自发自收”联调（建议仅在 BSP_CAN_MODE 为 LOOPBACK/静默回环时使用）
 * @note
 * - 你之前提到“没有 CAN 收发器”，在这种情况下无法接入真实 CANH/CANL 总线；
 * - 开启本选项后，本机将周期性发送一帧“加速模式”的测试报文，用于验证：
 *   CAN 初始化 -> 发送 -> 回环接收 -> 解析 -> 事件触发 -> 点阵灯响应
 */
#ifndef APP_CAN_SELF_TEST_TX
#define APP_CAN_SELF_TEST_TX 0
#endif

#ifndef APP_CAN_SELF_TEST_ID
#define APP_CAN_SELF_TEST_ID 0x123U
#endif

#ifndef APP_CAN_SELF_TEST_PERIOD_MS
#define APP_CAN_SELF_TEST_PERIOD_MS 1000
#endif

/**
 * @brief 将协议中的“点阵灯模式”映射为事件位
 * @param mode 协议 Byte2 的值
 * @return 对应的事件位；若为 NORMAL 或未知值，则返回 0
 */
static inline EventBits_t app_can_mode_to_evt(uint8_t mode)
{
  switch (mode)
  {
  case BSP_CAN_DOT_MODE_UP:
    return EVT_UP;
  case BSP_CAN_DOT_MODE_DOWN:
    return EVT_DOWN;
  case BSP_CAN_DOT_MODE_STOP:
    return EVT_STOP;
  case BSP_CAN_DOT_MODE_NORMAL:
  default:
    return 0;
  }
}

RESULT_Init app_can_init(void)
{
  /* 初始化 CAN 硬件与接收队列 */
  can_init();

  return ERR_Init_Finished;
}

void app_can_dispose_Task(void)
{
  EventGroupHandle_t evt = event_bus_getHandle();

  /* 用于控制“无报文时”的错误打印频率 */
  TickType_t last_err_print_tick = 0;
  TickType_t last_self_tx_tick = 0;

  while (1)
  {
#if APP_CAN_SELF_TEST_TX
    /**
     * 自测发送：
     * - 放在 while 顶部，保证即使收不到外部报文也能持续跑通链路；
     * - 使用 DLC=3，并把 Byte2 填为“加速模式”(0x00)。
     */
    TickType_t now_tick = xTaskGetTickCount();
    if ((now_tick - last_self_tx_tick) >=
        pdMS_TO_TICKS(APP_CAN_SELF_TEST_PERIOD_MS))
    {
      can_message_t tx = {0};
      tx.id = APP_CAN_SELF_TEST_ID;
      tx.extended_id = false;
      tx.remote_frame = false;
      tx.len = 3;
      tx.data[BSP_CAN_CMD_BYTE_DOT_MODE] = BSP_CAN_DOT_MODE_UP;

      bool ok = can_send_message(&tx);
#if APP_CAN_DEBUG_PRINT
      printf("[CAN] self-tx id=0x%lX %s\r\n", (unsigned long)tx.id,
             ok ? "OK" : "FAIL");
#endif
      last_self_tx_tick = now_tick;
    }
#endif

    can_message_t msg = {0};

    /* 有限阻塞等待 CAN 新报文（底层只保留最新一帧，适配“模式类”数据） */
    if (!can_read_message_block(&msg, pdMS_TO_TICKS(APP_CAN_RX_WAIT_MS)))
    {
#if APP_CAN_DEBUG_PRINT
      /**
       * 一直收不到数据时，最常见的原因：
       * 1) 你没有外接 CAN 收发器（STM32 的 TX/RX 只是“控制器侧”，物理差分需要收发器转换到 CANH/CANL）
       * 2) 总线上没有其他节点提供 ACK（单节点发送会一直报 ACK 错误）
       * 3) 波特率不一致（例如对端 500k，你这里却跑 125k）
       * 4) 引脚映射不对（PA11/PA12 vs PB8/PB9）
       * 5) 总线未正确接线/未加 120Ω 终端
       *
       * 因此这里周期性打印 HAL_CAN_GetError()，帮助你快速定位。
       */
      TickType_t now = xTaskGetTickCount();
      if ((now - last_err_print_tick) >= pdMS_TO_TICKS(1000))
      {
        uint32_t err = HAL_CAN_GetError(&hcan1);
        HAL_CAN_StateTypeDef st = HAL_CAN_GetState(&hcan1);

        if (err != HAL_CAN_ERROR_NONE)
        {
          printf("[CAN] no-rx, state=%d, err=0x%08lX\r\n", (int)st,
                 (unsigned long)err);
          if (err & HAL_CAN_ERROR_ACK)
          {
            printf("[CAN] 提示：检测到 ACK 错误，通常表示“总线上没有其他节点/没有收发器/未接终端/波特率不匹配”。\r\n");
          }
        }
        else
        {
          /* err=0 但仍收不到，优先检查：引脚映射、滤波器、对端是否在发 */
          printf("[CAN] no-rx, state=%d, err=0x%08lX\r\n", (int)st,
                 (unsigned long)err);
        }

        last_err_print_tick = now;
      }
#endif
      continue;
    }

    /* 只处理数据帧；远程帧直接忽略 */
    if (msg.remote_frame)
      continue;

    /**
     * 协议（doc/datasheet/can总线通信帧格式.png）约定：
     * - Byte2 为“点阵灯模式”（0x00 加速，0x01 减速，0x02 停车，0x03 正常）
     *
     * 但在联调阶段，你可能会用 CAN 工具只发送 1~2 个字节。
     * 为了让你更容易验证链路，这里做一个“兼容读取”：
     * - DLC >= 3：严格按协议读 Byte2
     * - DLC < 3：尝试使用“最后一个字节”作为 mode（若值在 0x00~0x03 范围内）
     */
    uint8_t mode = 0xFF;
    uint8_t mode_index = 0xFF;
    if (msg.len > BSP_CAN_CMD_BYTE_DOT_MODE)
    {
      mode = msg.data[BSP_CAN_CMD_BYTE_DOT_MODE];
      mode_index = BSP_CAN_CMD_BYTE_DOT_MODE;
    }
    else if (msg.len >= 1)
    {
      uint8_t idx = msg.len - 1;
      uint8_t v = msg.data[idx];
      if (v <= BSP_CAN_DOT_MODE_NORMAL)
      {
        mode = v;
        mode_index = idx;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }

    /* 将收到的模式映射为事件位（NORMAL/未知 -> 0） */
    EventBits_t want_bits = app_can_mode_to_evt(mode);
    const EventBits_t mode_mask = EVT_UP | EVT_DOWN | EVT_STOP;

    /**
     * 事件维护策略（关键点）：
     * - 本项目中有些任务在 xEventGroupWaitBits(..., clearOnExit=pdTRUE) 下会“消费并清除事件位”；
     * - 如果我们只在“mode变化”时 setBits，那么当任务清掉事件位后，即使对端持续发送同一模式，
     *   也可能因为“mode未变”而不再 setBits，导致“看起来没有现象”。
     *
     * 因此这里改为：
     * - 以事件组当前状态为准：只要当前状态与 want 不一致，就执行一次清空/设置；
     * - 这样既能避免无意义的重复 set（减少事件风暴），也能在事件被消费后自动恢复。
     */
    EventBits_t now_bits = xEventGroupGetBits(evt) & mode_mask;
    if (now_bits != want_bits)
    {
      xEventGroupClearBits(evt, mode_mask);
      if (want_bits)
        xEventGroupSetBits(evt, want_bits);
    }

    /* 可选：给调试/统计用的“收到CAN帧”事件 */
    xEventGroupSetBits(evt, EVT_CAN_RX);

#if APP_CAN_DEBUG_PRINT
    printf("[CAN] rx id=0x%lX, dlc=%u, mode=0x%02X (byte%u)\r\n",
           (unsigned long)msg.id, (unsigned)msg.len, (unsigned)mode,
           (unsigned)mode_index);
#endif
  }
}
