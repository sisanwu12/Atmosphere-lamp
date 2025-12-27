/**
 * @file event_bus.h
 * @author 肆叁伍12
 * @brief
 * 管理本项目的事件组文件
 * @version 0.1
 * @date 2025-12-09
 *
 */

#ifndef __EVENT_BUS_H
#define __EVENT_BUS_H

/* 头文件引用 */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

/* 事件组定义 */
// clang-format off
typedef enum
{
  EVT_NONE       = 0,          /* 无事件 */

  /* 1~7：  底层驱动事件 */
  EVT_USART_RX   = (1 << 1),   /* usart接收事件 */
  EVT_CAN_RX     = (1 << 2),   /* can接收事件 */

  /* 8~15:  控制逻辑事件 */
  EVT_TURN_BACK  = (1 << 8),   /* 回正事件 */
  EVT_TURN_LEFT  = (1 << 9),   /* 左转事件 */
  EVT_TURN_RIGHT = (1 << 10),  /* 右转事件 */
  EVT_UP         = (1 << 11),  /* 加速事件 */
  EVT_DOWN       = (1 << 12),  /* 减速事件 */
  EVT_STOP       = (1 << 13),  /* 停车事件 */

  /* 16~23: 用户交互事件 */
  EVT_USER_COM   = (1 << 16), /* 用户靠近 */

} MAIN_EVT_T;
// clang-format on

/* 函数声明 */
void event_bus_init(void);
EventGroupHandle_t event_bus_getHandle(void);

#endif
