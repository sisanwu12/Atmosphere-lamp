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

  /* 记录上一次模式，避免同一模式的报文反复触发造成无意义的事件抖动 */
  uint8_t last_mode = 0xFF;

  while (1)
  {
    can_message_t msg = {0};

    /* 阻塞等待 CAN 新报文（底层只保留最新一帧，适配“模式类”数据） */
    if (!can_read_message_block(&msg, portMAX_DELAY))
      continue;

    /* 只处理数据帧；远程帧直接忽略 */
    if (msg.remote_frame)
      continue;

    /* 协议要求 Byte2 有意义，因此 DLC 至少要 >= 3 */
    if (msg.len <= BSP_CAN_CMD_BYTE_DOT_MODE)
      continue;

    uint8_t mode = msg.data[BSP_CAN_CMD_BYTE_DOT_MODE];

    /* 同一模式重复上报时，不重复触发（减少事件风暴） */
    if (mode == last_mode)
      continue;
    last_mode = mode;

    /* 维护模式事件：先清空，再设置（保证互斥） */
    xEventGroupClearBits(evt, EVT_UP | EVT_DOWN | EVT_STOP);

    EventBits_t set_bits = app_can_mode_to_evt(mode);
    if (set_bits)
    {
      xEventGroupSetBits(evt, set_bits);
    }

    /* 可选：如果你希望其他任务知道“CAN来过一帧”，可以放开下面这一行 */
    /* xEventGroupSetBits(evt, EVT_CAN_RX); */
  }
}

