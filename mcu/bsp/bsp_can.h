#ifndef BSP_CAN_H
#define BSP_CAN_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// CAN硬件配置宏
#define CAN_TX_PIN GPIO_PIN_9
#define CAN_TX_PORT GPIOB
#define CAN_TX_AF GPIO_AF9_CAN1
#define CAN_RX_PIN GPIO_PIN_8
#define CAN_RX_PORT GPIOB
#define CAN_RX_AF GPIO_AF9_CAN1

// CAN消息结构体（统一字段名）
typedef struct
{
  uint32_t id;
  uint8_t data[8];
  uint8_t len;
  bool extended_id; // 扩展帧标识
  bool remoteFrame; // 远程帧标识
} can_message_t;

// CAN状态结构体（用于串口上报）
typedef struct
{
  uint8_t can_state;     // CAN工作状态
  uint8_t can_error;     // CAN错误码
  uint32_t can_rx_count; // 接收计数
  uint32_t can_tx_count; // 发送计数
} CANStatus;

// 外部声明
extern CAN_HandleTypeDef hcan1;
extern void *can_rb;

// 函数声明
void can_init(void);
void can_blockbuffer_init(void);
bool can_send_message(const can_message_t *msg);
bool can_read_message(can_message_t *msg);
void process_powertrain_message(can_message_t *msg);
void process_chassis_safety_message(can_message_t *msg);
void process_body_message(can_message_t *msg);
void process_infotainment_message(can_message_t *msg);

#endif
