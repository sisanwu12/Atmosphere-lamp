/**
 * @file		app_gonio.c
 * @brief		用于定义操作该模块的函数
 * @note		角度测量模块（磁编码器 PWM 输入捕获）
 * @author	王广平
 * @date		2025/11/24
 *
 * @note（重要）
 * 1) 本模块输出的“最终用于判断逻辑的角度”是 rel（相对角度）：
 *    rel = abs_angle - zero
 *    - abs_angle：传感器解码出的 0~360° 绝对角度（循环）
 *    - zero：第一次得到有效角度时自动记录为 0 点
 * 2) 方向盘“转不到一圈”也可能跨越 0°/360°（例如 359°->1°），会导致直接相减出现
 *    -358° 的大跳变。因此这里对 rel 做 wrap 到 [-180, +180]。
 */

#define __APP_GONIO_C

/* 头文件引用 */
#include "app_gonio.h"
#include "FreeRTOS.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "event_bus.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "task.h"
#include <stdio.h>

/* ============================== 静态全局变量 ============================== */
/* 定时器句柄 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

/* PWM 捕获值（单位：定时器 tick） */
static volatile u32 pulseWidth = 0; /* 高电平宽度（CCR2） */
static volatile u32 pwmPeriod = 0;  /* PWM 周期（CCR1） */
static volatile oboolean_t newData = bFALSE;

/**
 * @brief 方向盘 0 点（单位：度）
 * @note 第一次解码成功时自动记录当前 abs_angle 为 0 点。
 */
static float inital_value = 0;
static oboolean_t zero_inited = bFALSE;

/**
 * @brief 将角度差限制到 [-180, +180]（单位：度）
 * @note 只要传感器角度存在 0°/360° 回绕，就建议做这个处理。
 */
static inline float app_gonio_wrap_deg_180(float delta_deg)
{
  while (delta_deg > 180.0f)
    delta_deg -= 360.0f;
  while (delta_deg < -180.0f)
    delta_deg += 360.0f;
  return delta_deg;
}

/* ============================== 初始化与驱动 ============================== */
RESULT_Init app_gonio_init()
{
  RESULT_Init ret = ERR_Init_Start;

  /* 初始化GPIO引脚（TIM3_CH1/PA6） */
  bsp_gpio_Init(GONIO_GPIOx, GONIO_PIN, GPIO_MODE_AF_INPUT, GPIO_NOPULL,
                GPIO_SPEED_FREQ_HIGH);

  /* 初始化 TIM3：72MHz/72 = 1MHz（1 tick = 1us） */
  bsp_timer_SetStruct(&APP_GONIO_TIM, GONIO_TIMx, 72 - 1, TIM_COUNTERMODE_UP,
                      0xFFFF, TIM_CLOCKDIVISION_DIV1,
                      TIM_AUTORELOAD_PRELOAD_DISABLE, 0);

  /* 输入捕获初始化 */
  HAL_TIM_IC_Init(&APP_GONIO_TIM);

  /* CH1：上升沿捕获（周期） */
  TIM_IC_InitTypeDef sConfig = {0};
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI; /* TI1 直连 */
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_1);

  /* CH2：下降沿捕获（高电平宽度），复用 TI1 */
  sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfig.ICSelection = TIM_ICSELECTION_INDIRECTTI; /* TI1 间接输入 */
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_2);

  /**
   * @brief PWM 输入模式：复位从模式
   * @note 上升沿触发复位计数器，从而：
   * - CCR1：周期（上一周期计数）
   * - CCR2：高电平宽度（从上升沿到下降沿）
   */
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  HAL_TIM_SlaveConfigSynchro(&APP_GONIO_TIM, &sSlaveConfig);

  /* 使能 TIM3 中断 */
  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  /* 开始输入捕获中断 */
  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&APP_GONIO_TIM, TIM_CHANNEL_2);

  ret = ERR_Init_Finished;
  return ret;
}

/**
 * @brief 获取当前相对角度（最终用于判断逻辑的值）
 * @retval -1 表示暂无新数据或本次数据无效
 */
float app_gonio_GetAngleDeg(void)
{
  if (!newData)
    return -1;
  newData = bFALSE;

  /* 过滤明显无效的捕获：period==0 / high==0 / high>=period */
  if (pwmPeriod == 0 || pulseWidth == 0 || pulseWidth >= pwmPeriod)
    return -1;

  float duty = (float)pulseWidth / (float)pwmPeriod;
  if (duty < 0)
    duty = 0;
  if (duty > 1.0f)
    duty = 1.0f;

  /* 绝对角度（0~360 循环） */
#if GONIO_PWM_DECODE_USE_LOW_TIME
  float abs_angle = (1.0f - duty) * 360.0f;
#else
  float abs_angle = duty * 360.0f;
#endif

  /* 规范化到 [0, 360) */
  if (abs_angle < 0)
    abs_angle = 0;
  if (abs_angle >= 360.0f)
    abs_angle = 0;

  /**
   * @note 自动校零：第一次有效角度作为 0 点
   * - 这一帧返回 -1，避免初始化瞬间误触发转向/回正事件
   */
  if (!zero_inited)
  {
    zero_inited = bTRUE;
    inital_value = abs_angle;
    return -1;
  }

  /* 相对角度：wrap 到 [-180, +180] */
  return app_gonio_wrap_deg_180(abs_angle - inital_value);
}

/**
 * @brief 输入捕获中断回调转发（由 stm32f1xx_it.c 调用）
 */
void app_gonio_dispose_ISP()
{
  if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    pwmPeriod = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_1);
  }
  else if (APP_GONIO_TIM.Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    pulseWidth = HAL_TIM_ReadCapturedValue(&APP_GONIO_TIM, TIM_CHANNEL_2);
    newData = bTRUE;
  }
}

/**
 * @brief 转向判断线程
 * @note
 * 你的需求：
 * - 第一次有效数据为 0 点
 * - +90° 稳定为左转
 * - -90° 稳定为右转
 *
 * 实现方式：
 * - 每 20ms 读取一次新角度（无新数据则跳过）
 * - 连续 STABLE_COUNT 次满足条件才触发事件
 * - LEFT/RIGHT 状态下必须先回到 CENTER 才允许切换（避免误闪另一侧）
 */
void app_gonio_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();

  /* 串口打印节流（避免 printf 过于频繁拖慢系统） */
  TickType_t last_print_tick = 0;
  const TickType_t print_period = pdMS_TO_TICKS(200);

  /* 无数据提示节流 */
  TickType_t last_nodata_tick = 0;
  const TickType_t nodata_period = pdMS_TO_TICKS(1000);

  enum
  {
    STEER_CENTER = 0,
    STEER_LEFT,
    STEER_RIGHT
  } state = STEER_CENTER;

  const float TURN_ON_DEG = 90.0f; /* +90 左转 / -90 右转 */
  const float CENTER_DEG = 30.0f;  /* 回正判定阈值（绝对值） */
  const int STABLE_COUNT = 15;     /* 15*20ms≈300ms */
  int left_cnt = 0, right_cnt = 0, center_cnt = 0;

  while (1)
  {
    oboolean_t has_new = newData;
    float angle = app_gonio_GetAngleDeg();

    /* 无数据提示（注意：校零那一帧会返回 -1） */
    if (!has_new && angle == -1)
    {
      TickType_t now = xTaskGetTickCount();
      if ((now - last_nodata_tick) >= nodata_period)
      {
        printf("[GONIO] 未捕获到PWM输入，请检查：PA6(TIM3_CH1)接线/供电/磁铁距离/定时器中断\r\n");
        last_nodata_tick = now;
      }
    }

    if (angle == -1)
    {
      left_cnt = 0;
      right_cnt = 0;
      center_cnt = 0;
      vTaskDelay(20);
      continue;
    }

    /* 周期性打印（用于确认 0 点与 rel） */
    if (has_new)
    {
      TickType_t now = xTaskGetTickCount();
      if ((now - last_print_tick) >= print_period)
      {
        u32 period = pwmPeriod;
        u32 high = pulseWidth;

        u32 duty_x1000 = 0;
        u32 absH_x100 = 0;
        u32 absL_x100 = 0;
        if (period != 0)
        {
          duty_x1000 = (high * 1000UL) / period;
          absH_x100 = (high * 36000UL) / period;
          absL_x100 = 36000UL - absH_x100;
        }

        int32_t rel_x100 = (int32_t)(angle * 100.0f);
        int32_t zero_x100 = (int32_t)(inital_value * 100.0f);

        printf("[GONIO] high=%lu, period=%lu, duty=%lu/1000, absH=%lu.%02lu, absL=%lu.%02lu, rel=%ld.%02ld, zero=%ld.%02ld\r\n",
               (unsigned long)high, (unsigned long)period,
               (unsigned long)duty_x1000, (unsigned long)(absH_x100 / 100),
               (unsigned long)(absH_x100 % 100), (unsigned long)(absL_x100 / 100),
               (unsigned long)(absL_x100 % 100), (long)(rel_x100 / 100),
               (long)(rel_x100 < 0 ? (-rel_x100 % 100) : (rel_x100 % 100)),
               (long)(zero_x100 / 100),
               (long)(zero_x100 < 0 ? (-zero_x100 % 100) : (zero_x100 % 100)));

        last_print_tick = now;
      }
    }

    /* 转向状态机 */
    switch (state)
    {
    case STEER_CENTER:
      if (angle >= TURN_ON_DEG)
      {
        left_cnt++;
        right_cnt = 0;
      }
      else if (angle <= -TURN_ON_DEG)
      {
        right_cnt++;
        left_cnt = 0;
      }
      else
      {
        left_cnt = 0;
        right_cnt = 0;
      }

      if (left_cnt >= STABLE_COUNT)
      {
        left_cnt = 0;
        state = STEER_LEFT;
        xEventGroupSetBits(evt, EVT_TURN_LEFT);
      }
      else if (right_cnt >= STABLE_COUNT)
      {
        right_cnt = 0;
        state = STEER_RIGHT;
        xEventGroupSetBits(evt, EVT_TURN_RIGHT);
      }
      break;

    case STEER_LEFT:
    case STEER_RIGHT:
    default:
      if (angle <= CENTER_DEG && angle >= -CENTER_DEG)
        center_cnt++;
      else
        center_cnt = 0;

      if (center_cnt >= STABLE_COUNT)
      {
        center_cnt = 0;
        left_cnt = 0;
        right_cnt = 0;
        state = STEER_CENTER;
        xEventGroupSetBits(evt, EVT_TURN_BACK);
      }
      break;
    }

    vTaskDelay(20);
  }
}

TIM_HandleTypeDef *app_gonio_getTIMHandle() { return &APP_GONIO_TIM; }

