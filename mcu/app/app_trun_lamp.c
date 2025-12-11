/**
 * @file		app_trun_lamp.c
 * @brief		用于定义操作该模块的函数
 * @note		基础转向灯模块
 * @author	王广平
 **/

#define __APP_TRUN_LAMP_C

/* 头文件引用 */
#include "app_trun_lamp.h"
#include "bsp_gpio.h"
#include "event_bus.h"
#include "stm32f1xx_hal_gpio.h"

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
  HAL_GPIO_WritePin(LEFT_GPIOx, RIGHT_PIN, GPIO_PIN_RESET);
  return ERR_RUN_Finished;
}

void app_trunL_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();
  u8 state = 0; /* 简易状态机 */

  while (1)
  {
    EventBits_t bits =
        xEventGroupWaitBits(evt, EVT_TURN_LEFT | EVT_TURN_RIGHT | EVT_TURN_BACK,
                            pdTRUE, pdFALSE, portMAX_DELAY);

    if (bits & EVT_TURN_LEFT)
    {
      state = 1; /* 左转状态 */
    }
    else if (bits & EVT_TURN_RIGHT)
    {
      state = 2; /* 右转状态 */
    }
    else if (bits & EVT_TURN_BACK)
    {
      state = 0; /* 回正状态/常态 */
    }

    switch (state)
    {
    case 1: // 左转状态
      app_trunL_open_left();
      break;

    case 2: // 右转状态
      app_trunL_open_right();
      break;

    case 0: // 回正状态/常态
    default:
      app_trunL_close_left();
      app_trunL_close_right();
      break;
    }
    vTaskDelay(500); // 闪烁周期
  }
}