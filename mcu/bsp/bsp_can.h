/**
 * @file    bsp_can.h
 * @brief   CAN底层驱动（STM32F103C8T6 / CAN1）
 * @author  肆叁伍12
 * @date    2025/12/27
 *
 * @note
 * - 本文件只做“CAN硬件初始化 + 接收缓存 + 发送接口”。
 * - 与业务相关的“协议解析/事件触发”建议放在 app 层（例如 app_can.c）。
 */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

/* 头文件引用 */
#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ============================== 硬件引脚配置 ============================== */
/**
 * @brief CAN1 引脚选择
 * @note
 * - STM32F103 的 CAN1 默认映射：PA11(RX) / PA12(TX)
 * - 本工程使用 PB8(RX) / PB9(TX)，需要开启 CAN1 重映射 CASE2
 */
#define BSP_CAN1_RX_PIN  GPIO_PIN_8
#define BSP_CAN1_TX_PIN  GPIO_PIN_9
#define BSP_CAN1_GPIOx   GPIOB

/* ============================== 协议字段定义 ============================== */
/**
 * @brief “can总线通信帧格式.png”中定义的协议字段位置
 * @note 本项目目前只关心 Byte2（点阵灯模式：加速/减速/停车/正常）
 */
#define BSP_CAN_CMD_BYTE_DOT_MODE 2

typedef enum
{
  BSP_CAN_DOT_MODE_UP     = 0x00, /* 加速 */
  BSP_CAN_DOT_MODE_DOWN   = 0x01, /* 减速 */
  BSP_CAN_DOT_MODE_STOP   = 0x02, /* 停车 */
  BSP_CAN_DOT_MODE_NORMAL = 0x03, /* 正常/无动作 */
} BSP_CAN_DOT_MODE_T;

/* ============================== 数据结构定义 ============================== */
/**
 * @brief CAN 消息结构体（用于应用层读取）
 * @note
 * - id：标准帧为 11bit；扩展帧为 29bit
 * - len：0~8
 */
typedef struct
{
  uint32_t id;         /* CAN ID（标准/扩展统一放在这里） */
  uint8_t data[8];     /* 数据区 */
  uint8_t len;         /* DLC */
  bool extended_id;    /* true：扩展帧；false：标准帧 */
  bool remote_frame;   /* true：远程帧；false：数据帧 */
} can_message_t;

/* ============================== 外部变量声明 ============================== */
extern CAN_HandleTypeDef hcan1;

/* ============================== 对外接口声明 ============================== */
/**
 * @brief CAN1 初始化（GPIO/重映射/波特率/滤波/中断/接收队列）
 * @note 需在 vTaskStartScheduler() 之前调用，建议在 event_bus_init() 之后调用。
 */
void can_init(void);

/**
 * @brief 发送 CAN 报文（标准帧/扩展帧均支持）
 * @param msg 待发送报文
 * @return true 发送请求已进入邮箱；false 参数错误或邮箱不可用
 */
bool can_send_message(const can_message_t *msg);

/**
 * @brief 读取一帧 CAN 报文（非阻塞）
 * @param msg 输出报文
 * @return true 读取到新报文；false 当前无新报文或队列未初始化
 *
 * @note 等价于 can_read_message_block(msg, 0)。
 */
bool can_read_message(can_message_t *msg);

/**
 * @brief 读取一帧 CAN 报文（阻塞读取，超时返回）
 * @param msg     输出报文
 * @param timeout 等待超时（FreeRTOS tick）
 * @return true 读取到新报文；false 超时或队列未初始化
 *
 * @note 该接口从内部队列取“最新一帧”。如果中断期间来了多帧，旧帧会被覆盖。
 */
bool can_read_message_block(can_message_t *msg, TickType_t timeout);

#endif
