/**
 * @file		app_gonio.c
 * @brief		用于定义操作该模块的函数
 * @note		角度测量模块
 * @author	王广平
 * @date		2025/11/24
 **/

#define __APP_GONIO_C

/* 头文件引用 */
#include "app_gonio.h"
#include "FreeRTOS.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "event_bus.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "task.h"

/* 静态全局变量 */
/* 定时器句柄 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

/* 方向盘初始位置 */
static float inital_value = 180;
/* us */
static volatile u32 pulseWidth = 0;
/* 是否有新数据标记 */
static volatile oboolean_t newData = bFALSE;

/**
 * @brief 尝试读取一次“最新角度”（读取成功会消费 newData 标记）
 *
 * @param[out] out_angle_deg 角度输出（单位：度，范围约 -180~+180）
 * @retval bTRUE  读取到新数据
 * @retval bFALSE 暂无新数据
 *
 * @note
 * 1) app_gonio_GetAngleDeg() 会清除 newData，因此上层状态机里要避免
 *    “先读一次再做稳定判断”的写法，否则稳定判断阶段可能一直读不到新数据。
 * 2) 这里把“读取并消费”封装成 try-get 形式，方便稳定判断与状态机复用。
 */
static inline oboolean_t app_gonio_try_get_angle(float *out_angle_deg)
{
  if (out_angle_deg == NULL)
    return bFALSE;

  if (!newData)
    return bFALSE;
  newData = bFALSE;

  const float period_us = 2000.0f; // AS5048A PWM 周期固定 2ms
  float angle = ((float)pulseWidth / period_us) * 360.0f;

  /* 保护：范围必须在 0~360（异常脉宽时避免算出 NAN/INF） */
  if (angle < 0)
    angle = 0;
  if (angle > 360)
    angle = 360;

  *out_angle_deg = angle - inital_value;
  return bTRUE;
}

RESULT_Init app_gonio_init()
{
  RESULT_Init ret = ERR_Init_Start;

  /* 初始化GPIO引脚 */
  bsp_gpio_Init(GONIO_GPIOx, GONIO_PIN, GPIO_MODE_AF_INPUT, GPIO_NOPULL,
                GPIO_SPEED_FREQ_HIGH);

  /* 初始化TIM定时器 */
  bsp_timer_SetStruct(&APP_GONIO_TIM, GONIO_TIMx, 72 - 1, TIM_COUNTERMODE_UP,
                      0xFFFF, TIM_CLOCKDIVISION_DIV1,
                      TIM_AUTORELOAD_PRELOAD_DISABLE, 0);

  HAL_TIM_PWM_Init(&APP_GONIO_TIM);

  // 输入捕获配置
  TIM_IC_InitTypeDef sConfig = {0};
  // CH1: 上升沿捕获
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_1);

  // CH2: 下降沿捕获
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_2);

  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_2);

  ret = ERR_Init_Finished;
  return ret;
}

float app_gonio_GetAngleDeg(void)
{
  float angle = 0.0f;
  if (!app_gonio_try_get_angle(&angle))
    return -1; // 没有新数据
  return angle;
}

void app_gonio_dispose_ISP()
{
  static volatile u32 riseTime = 0;
  static volatile u32 fallTime = 0;
  if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    // 上升沿：记录开始时间
    riseTime = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_1);
  }
  else if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    // 下降沿：记录结束时间
    fallTime = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_2);

    if (fallTime >= riseTime)
      pulseWidth = fallTime - riseTime;
    else
      pulseWidth = (0xFFFF - riseTime) + fallTime;

    newData = bTRUE;
  }
}

/**
 * @brief 左转稳定判断函数
 * @return 是否稳定
 * @date    2025/12/9
 */
static inline oboolean_t isStableLeft()
{
  /* 只统计“拿到新数据”的次数，避免偶发丢采样导致误判为不稳定 */
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (angle < 90)
      return bFALSE;
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  return bTRUE;
}

/**
 * @brief 右转稳定判断函数
 * @return 是否稳定
 * @date    2025/12/9
 */
static inline oboolean_t isStableRight()
{
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (angle > -90)
      return bFALSE;
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  return bTRUE;
}

/**
 * @brief 回正稳定判断函数
 * @return 是否稳定
 * @date    2025/12/9
 */
static inline oboolean_t isStableCenter()
{
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (angle < -30 || angle > 30)
      return bFALSE;
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  return bTRUE;
}

void app_gonio_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();

  enum
  {
    STEER_CENTER = 0,
    STEER_LEFT,
    STEER_RIGHT
  } state = STEER_CENTER;

  while (1)
  {
    float angle = 0.0f;

    /* 没有新数据就等待下一次采样，避免用旧数据重复判断 */
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    // 左转判断（>=90° 持续稳定）
    if (angle >= 90)
    {
      if (state != STEER_LEFT) // 状态发生变化
      {
        if (isStableLeft()) // 连续采样稳定
        {
          state = STEER_LEFT;
          xEventGroupSetBits(evt, EVT_TURN_LEFT);
        }
      }
    }
    // 右转判断（<= -90° 持续稳定）
    else if (angle <= -90)
    {
      if (state != STEER_RIGHT)
      {
        if (isStableRight())
        {
          state = STEER_RIGHT;
          xEventGroupSetBits(evt, EVT_TURN_RIGHT);
        }
      }
    }
    // 回正判断（-30° ~ +30°）
    else if (angle <= 30 && angle >= -30)
    {
      if (state != STEER_CENTER)
      {
        if (isStableCenter())
        {
          state = STEER_CENTER;
          xEventGroupSetBits(evt, EVT_TURN_BACK);
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz 采样足够
  }
}

TIM_HandleTypeDef *app_gonio_getTIMHandle() { return &APP_GONIO_TIM; }
