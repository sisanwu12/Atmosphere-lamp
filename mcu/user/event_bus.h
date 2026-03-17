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
#include "ERR.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

/* 事件组定义 */
// clang-format off
typedef enum
{
  SIG_NONE            = 0,          /* 无事件 */

  /* 0~7: 通知型事件 */
  SIG_LAMP_UPDATE    = (1 << 0),    /* 灯光状态已更新 */
  SIG_DISPLAY_UPDATE = (1 << 1),    /* 显示状态已更新 */
  SIG_CAN_RX         = (1 << 2),    /* 收到有效 CAN 报文 */
  SIG_RESERVED_USER  = (1 << 3),    /* 预留给用户交互来源 */

} system_signal_t;
// clang-format on

/* 函数声明 */
RESULT_Init event_bus_init(void);
EventGroupHandle_t event_bus_getHandle(void);

#endif
