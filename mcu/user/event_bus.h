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
typedef enum
{

  EVT_NONE = 0, /* 无事件 */

  /* 1~7位：底层驱动事件 */
  EVT_USART_RX = (1 << 1), /* usart接收事件 */
  EVT_CAN_RX = (1 << 2),   /* can接收事件 */

} MAIN_EVT_T;

/* 函数声明 */
void event_bus_init(void);
EventGroupHandle_t event_bus_getHandle(void);

#endif