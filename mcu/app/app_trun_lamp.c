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
#include "app_state.h"
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
  app_steer_state_t state = APP_STEER_CENTER;
  u8 blink_on = 0; /* 0=灭,1=亮 */
  const TickType_t blink_period = pdMS_TO_TICKS(500);

  app_trunL_close_left();
  app_trunL_close_right();

  while (1)
  {
    TickType_t wait_ticks =
        (state == APP_STEER_CENTER) ? portMAX_DELAY : blink_period;
    EventBits_t bits = xEventGroupWaitBits(evt, SIG_LAMP_UPDATE, pdTRUE,
                                           pdFALSE, wait_ticks);

    if (bits & SIG_LAMP_UPDATE)
    {
      app_state_snapshot_t snapshot;
      app_state_get_snapshot(&snapshot);
      state = snapshot.steer;
      blink_on = (state == APP_STEER_CENTER) ? 0 : 1;
    }

    switch (state)
    {
    case APP_STEER_LEFT:
      if (bits == 0) /* 超时：到达闪烁周期，翻转一次 */
        blink_on = (u8)!blink_on;
      HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN,
                        blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
      break;

    case APP_STEER_RIGHT:
      if (bits == 0)
        blink_on = (u8)!blink_on;
      HAL_GPIO_WritePin(RIGHT_GPIOx, RIGHT_PIN,
                        blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEFT_GPIOx, LEFT_PIN, GPIO_PIN_RESET);
      break;

    case APP_STEER_CENTER:
    default:
      app_trunL_close_left();
      app_trunL_close_right();
      break;
    }
  }
}
