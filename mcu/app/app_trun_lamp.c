/**
 * @file		app_trun_lamp.c
 * @brief		用于定义操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
 **/

#define __APP_TRUN_LAMP_C

/* 头文件引用 */
#include "app_trun_lamp.h"
#include "__port_type__.h"
#include "bsp_gpio.h"
#include "event_bus.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_gpio.h"
#include "task.h"

RESULT_Init app_trunL_init()
{
  RESULT_Init ret = ERR_Init_Start;
  /* 初始化左转灯 */
  bsp_gpio_Init(LEFT_GPIOx, LEFT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_LOW);
  /* 初始化右转灯 */
  bsp_gpio_Init(RIGHT_GPIOx, RIGHT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_LOW);
  /* 初始化关闭所有灯 */
  HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
  ret = ERR_Init_Finished;
  return ret;
}

RESULT_RUN app_trunL_open_left()
{
  HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_SET);
  return ERR_RUN_Finished;
}

RESULT_RUN app_trunL_open_right()
{
  HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_SET);
  return ERR_RUN_Finished;
}

RESULT_RUN app_trunL_close_left()
{
  HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_RESET);
  return ERR_RUN_Finished;
}

RESULT_RUN app_trunL_close_right()
{
  HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
  return ERR_RUN_Finished;
}

void app_trunL_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();
  u8 state = 0; /* 简易状态机 */
  u8 blink_on = 0; /* 0=灭,1=亮 */

  while (1)
  {
    /**
     * @note
     * 本任务既要“响应事件”又要“周期性闪烁”，因此不能用 portMAX_DELAY 永久阻塞。
     * 做法：用闪烁周期作为等待超时：
     * - 收到事件：立即切换状态（并把灯置为亮）
     * - 超时返回：表示本周期到了，执行一次翻转，实现闪烁
     */
    const TickType_t blink_period = pdMS_TO_TICKS(500);
    EventBits_t bits = xEventGroupWaitBits(
        evt, EVT_TURN_LEFT | EVT_TURN_RIGHT | EVT_TURN_BACK, pdTRUE, pdFALSE,
        blink_period);

    if (bits & EVT_TURN_LEFT)
    {
      state = 1; /* 左转状态 */
      blink_on = 1;
    }
    else if (bits & EVT_TURN_RIGHT)
    {
      state = 2; /* 右转状态 */
      blink_on = 1;
    }
    else if (bits & EVT_TURN_BACK)
    {
      state = 0; /* 回正状态/常态 */
      blink_on = 0;
    }

    switch (state)
    {
    case 1: // 左转状态
      if (bits == 0) /* 超时：到达闪烁周期，翻转一次 */
        blink_on = (u8)!blink_on;
      HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN,
                        blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
      break;

    case 2: // 右转状态
      if (bits == 0)
        blink_on = (u8)!blink_on;
      HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN,
                        blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_RESET);
      break;

    case 0: // 回正状态/常态
    default:
      app_trunL_close_left();
      app_trunL_close_right();
      break;
    }
  }
}
