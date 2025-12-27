/**
 * @file    app_can.h
 * @brief   CAN 协议解析与事件触发模块
 * @author  肆叁伍12
 * @date    2025/12/27
 *
 * @note
 * 本模块基于 bsp_can（硬件驱动）读取 CAN 报文，并按照
 * doc/datasheet/can总线通信帧格式.png 的约定解析出：
 * - 加速 / 减速 / 停车
 * 然后在 FreeRTOS 事件组中产生对应事件位：
 * - EVT_UP / EVT_DOWN / EVT_STOP
 */

#ifndef __APP_CAN_H
#define __APP_CAN_H

/* 头文件引用 */
#include "ERR.h"

/* 函数声明 */
RESULT_Init app_can_init(void);
void app_can_dispose_Task(void);

#endif
