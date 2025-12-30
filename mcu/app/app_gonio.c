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
#include <stdio.h>

/* 静态全局变量 */
/* 定时器句柄 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};

/**
 * @brief 零点偏置（单位：度）
 * @note
 * - 该值用于把“绝对角度”转换为“相对角度(rel)”，即 rel = abs_unwrapped -
 * inital_value。
 * - 为了方便你找到 0 点，本工程默认在运行时自动校零：
 *   第一次拿到有效角度后，将当前位置作为零点（建议上电时让方向盘处于回正位置）。
 */
static float inital_value = 0;
/* us */
static volatile u32 pulseWidth = 0;
/* PWM 周期（tick），用于计算占空比/角度 */
static volatile u32 pwmPeriod = 0;
/* 是否有新数据标记 */
static volatile oboolean_t newData = bFALSE;

<<<<<<< HEAD
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
=======
/* 零点是否已初始化 */
static oboolean_t zero_inited = bFALSE;

/**
 * @brief 将角度差限制到 [-180, +180]（单位：度）
 * @note
 * 即使方向盘“转不到一圈”，传感器的绝对角度依然可能跨越 0°/360°，
 * 例如从 359° 到 1°，直接相减会得到 -358°，会误触发右转/左转判断。
 */
static inline float app_gonio_wrap_deg_180(float delta_deg)
{
  while (delta_deg > 180.0f)
    delta_deg -= 360.0f;
  while (delta_deg < -180.0f)
    delta_deg += 360.0f;
  return delta_deg;
>>>>>>> 1f20ba5 (完善转向代码)
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

  /**
   * @note
   * 这里是输入捕获功能（测量外部 PWM），应使用 IC 初始化而不是 PWM 初始化。
   * 使用 IC 初始化后，HAL 会按输入捕获模式配置基础寄存器，更符合预期。
   */
  HAL_TIM_IC_Init(&APP_GONIO_TIM);

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
  /**
   * @note
   * 磁编码器（如 AS5048A）的 PWM 信号通常只接到一个引脚（本工程为
   * TIM3_CH1/PA6）。 要在同一根线同时捕获“上升沿/下降沿”，CH2 必须配置为
   * INDIRECTTI， 即：复用 TI1 的输入（而不是去读 CH2 自己的引脚
   * TIM3_CH2/PA7）。
   *
   * 如果这里仍然用 DIRECTTI，那么你只接 PA6 时，CH2 永远捕获不到下降沿，
   * newData 就不会置位，任务里自然“什么数据都没有”。
   */
  sConfig.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfig, TIM_CHANNEL_2);

  /**
   * @brief PWM 输入模式：使用“复位从模式”得到周期与高电平宽度
   * @note
   * - 触发源：TI1（PA6）
   * - 触发动作：上升沿触发时将计数器清零（Reset）
   *
   * 在这种配置下：
   * - CH1（上升沿）捕获的值 ≈ PWM 周期（上一周期的计数值）
   * - CH2（下降沿，复用 TI1）捕获的值 = 高电平宽度（从上升沿到下降沿的计数值）
   *
   * 优点：不需要自己做 rise/fall 的差值计算，也更不容易出现“脉宽常为
   * 0/1”的情况。
   */
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  HAL_TIM_SlaveConfigSynchro(&APP_GONIO_TIM, &sSlaveConfig);

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
<<<<<<< HEAD
  return angle;
=======
  newData = bFALSE;

  /**
   * @note
   * 不再假设 PWM 周期固定为 2ms，而是用输入捕获测出来的 pwmPeriod 计算占空比。
   * 这样无论你的传感器 PWM 周期是多少，都能得到正确的“0~360°”映射。
   */
  if (pwmPeriod == 0)
    return -1;

  /**
   * @note
   * 过滤明显无效的捕获结果：
   * - high == 0：通常表示下降沿没捕获到/噪声触发
   * - high >= period：占空比非法
   *
   * 这些数据如果参与角度计算，会出现你日志里那种“abs=0/360 跳变”的现象。
   */
  if (pulseWidth == 0 || pulseWidth >= pwmPeriod)
    return -1;

  float duty = (float)pulseWidth / (float)pwmPeriod;
  if (duty < 0)
    duty = 0;
  if (duty > 1.0f)
    duty = 1.0f;

  /* 某些传感器角度编码在低电平宽度里，可通过宏切换 */
#if GONIO_PWM_DECODE_USE_LOW_TIME
  float abs_angle = (1.0f - duty) * 360.0f;
#else
  float abs_angle = duty * 360.0f;
#endif

  /**
   * @note
   * 将绝对角度限制到 [0, 360)：
   * - 360° 与 0° 等价
   */
  if (abs_angle < 0)
    abs_angle = 0;
  if (abs_angle >= 360.0f)
    abs_angle = 0;

  /**
   * @brief 自动校零
   * @note
   * 第一次拿到有效角度时，把当前位置当作“0 点”，避免你一直找不到 0。
   * 校零完成的这一帧返回 -1，防止初始化瞬间误触发转向/回正事件。
   */
  if (!zero_inited)
  {
    zero_inited = bTRUE;
    inital_value = abs_angle;
    return -1;
  }

  /**
   * @note
   * 返回相对角度（最终用于判断逻辑的值）：
   * rel = abs - zero，并把差值限制到 [-180, +180]，避免跨 0 点跳变误判。
   */
  return app_gonio_wrap_deg_180(abs_angle - inital_value);
>>>>>>> 1f20ba5 (完善转向代码)
}

void app_gonio_dispose_ISP()
{
  /**
   * @note
   * 采用 PWM 输入模式后：
   * - 在 CH1 中断里读取“周期”（CCR1）
   * - 在 CH2 中断里读取“高电平宽度”（CCR2），并置位 newData
   */
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
 * @brief 左转稳定判断函数
 * @return 是否稳定
 * @date    2025/12/9
 */
static inline oboolean_t isStableLeft()
{
<<<<<<< HEAD
  /* 只统计“拿到新数据”的次数，避免偶发丢采样导致误判为不稳定 */
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
=======
  /**
   * @note
   * app_gonio_GetAngleDeg() 在没有新捕获数据或捕获无效时会返回 -1。
   * 稳定判断时需要跳过这些无效样本，否则会误判为“不稳定”。
   */
  for (int stable_count = 0; stable_count < 50;)
  {
    float angle = app_gonio_GetAngleDeg();
    if (angle == -1)
    {
      vTaskDelay(2);
>>>>>>> 1f20ba5 (完善转向代码)
      continue;
    }

    if (angle < 90)
      return bFALSE;
<<<<<<< HEAD
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
=======

    stable_count++;
    vTaskDelay(10);
>>>>>>> 1f20ba5 (完善转向代码)
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
<<<<<<< HEAD
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
=======
  for (int stable_count = 0; stable_count < 50;)
  {
    float angle = app_gonio_GetAngleDeg();
    if (angle == -1)
    {
      vTaskDelay(2);
>>>>>>> 1f20ba5 (完善转向代码)
      continue;
    }

    if (angle > -90)
      return bFALSE;
<<<<<<< HEAD
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
=======

    stable_count++;
    vTaskDelay(10);
>>>>>>> 1f20ba5 (完善转向代码)
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
<<<<<<< HEAD
  for (int ok = 0; ok < 50;)
  {
    float angle = 0.0f;
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(10));
=======
  for (int stable_count = 0; stable_count < 50;)
  {
    float angle = app_gonio_GetAngleDeg();
    if (angle == -1)
    {
      vTaskDelay(2);
>>>>>>> 1f20ba5 (完善转向代码)
      continue;
    }

    if (angle < -30 || angle > 30)
      return bFALSE;
<<<<<<< HEAD
    ok++;
    vTaskDelay(pdMS_TO_TICKS(10));
=======

    stable_count++;
    vTaskDelay(10);
>>>>>>> 1f20ba5 (完善转向代码)
  }

  return bTRUE;
}

void app_gonio_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();

  /**
   * @brief 串口打印节流
   * @note
   * - 磁编码器 PWM 周期约 2ms，如果每次采样都 printf，会严重拖慢系统。
   * - 这里把打印频率限制在 5Hz（200ms 一次），用于确认是否读到了磁铁旋转。
   */
  TickType_t last_print_tick = 0;
  const TickType_t print_period = pdMS_TO_TICKS(200);

  /**
   * @brief 无数据提示节流
   * @note
   * 如果一直没有捕获到 PWM（newData 一直为
   * false），也需要给出提示，便于排查接线/定时器。
   */
  TickType_t last_nodata_tick = 0;
  const TickType_t nodata_period = pdMS_TO_TICKS(1000);

  enum
  {
    STEER_CENTER = 0,
    STEER_LEFT,
    STEER_RIGHT
  } state = STEER_CENTER;

  /**
   * @note
   * 去抖/稳定判定使用“连续有效采样计数”，避免在 isStableXXX() 中阻塞 500ms，
   * 同时避免 GetAngleDeg() 被多处调用导致采样被消耗。
   *
   * 你的需求：
   * - 第一次有效角度作为 0 点（GetAngleDeg 内部已自动校零）
   * - +90° 稳定判定为左转
   * - -90° 稳定判定为右转
   */
  const float TURN_ON_DEG = 90.0f; /* 进入转向状态阈值：+90 左转 / -90 右转 */
  const float CENTER_DEG = 30.0f;  /* 回正判定阈值（绝对值） */
  const int STABLE_COUNT = 15;     /* 连续采样次数（15*20ms≈300ms） */
  int left_cnt = 0, right_cnt = 0, center_cnt = 0;

  while (1)
  {
<<<<<<< HEAD
    float angle = 0.0f;

    /* 没有新数据就等待下一次采样，避免用旧数据重复判断 */
    if (!app_gonio_try_get_angle(&angle))
    {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }
=======
    /* 先读出“是否有新数据”的标记，用于判断是否需要打印 */
    oboolean_t has_new = newData;
    float angle = app_gonio_GetAngleDeg();
>>>>>>> 1f20ba5 (完善转向代码)

    /* 一直无数据时，周期性提示（避免你以为串口没工作） */
    if (!has_new && angle == -1)
    {
      TickType_t now = xTaskGetTickCount();
      if ((now - last_nodata_tick) >= nodata_period)
      {
<<<<<<< HEAD
        if (isStableLeft()) // 连续采样稳定
        {
          state = STEER_LEFT;
          xEventGroupSetBits(evt, EVT_TURN_LEFT);
        }
=======
	printf("[GONIO] "
	       "未捕获到PWM输入，请检查：PA6(TIM3_CH1)接线/供电/磁铁距离/"
	       "定时器中断\r\n");
	last_nodata_tick = now;
>>>>>>> 1f20ba5 (完善转向代码)
      }
    }

    /* 没有有效角度数据则不做状态机判断，避免误触发回正/转向事件 */
    if (angle == -1)
    {
      left_cnt = 0;
      right_cnt = 0;
      center_cnt = 0;
      vTaskDelay(20);
      continue;
    }

    /* 如果捕获到了新 PWM 数据，则周期性打印脉宽与角度 */
    if (has_new && angle != -1)
    {
      TickType_t now = xTaskGetTickCount();
      if ((now - last_print_tick) >= print_period)
      {
<<<<<<< HEAD
        if (isStableRight())
        {
          state = STEER_RIGHT;
          xEventGroupSetBits(evt, EVT_TURN_RIGHT);
        }
=======
	/**
	 * @note
	 * - high/period：输入捕获的“高电平宽度/周期”（单位为 timer
	 * tick，二者比值即占空比）
	 * - absH：按“高电平”解码得到的绝对角度（0~360）
	 * - absL：按“低电平”解码得到的绝对角度（0~360）
	 * - rel：当前编译选定的相对角度（用于“回正/左转/右转”判断）
	 */
	/**
	 * @note
	 * 为了避免 printf 浮点格式化占用过大栈空间，这里使用“定点整数”打印：
	 * - duty_x1000：占空比 *1000
	 * - absH_x100 / absL_x100：角度 *100
	 * - rel_x100：相对角度 *100
	 */
	u32 period = pwmPeriod;
	u32 high = pulseWidth;

	u32 duty_x1000 = 0;
	u32 absH_x100 = 0; /* 高电平解码绝对角度 */
	u32 absL_x100 = 0; /* 低电平解码绝对角度 */

	if (period != 0)
	{
	  duty_x1000 = (high * 1000UL) / period;
	  absH_x100 = (high * 36000UL) / period;
	  absL_x100 = 36000UL - absH_x100;
	}

	/**
	 * @note
	 * 最终用于判断逻辑的是 rel（相对角度）。
	 * zero 为“上电自动校零”得到的 0 点（绝对角度）。
	 */
	int32_t rel_x100 = (int32_t)(angle * 100.0f);
	int32_t zero_x100 = (int32_t)(inital_value * 100.0f);

	printf("[GONIO] high=%lu, period=%lu, duty=%lu/1000, absH=%lu.%02lu, "
	       "absL=%lu.%02lu, rel=%ld.%02ld, zero=%ld.%02ld\r\n",
	       (unsigned long)high, (unsigned long)period,
	       (unsigned long)duty_x1000, (unsigned long)(absH_x100 / 100),
	       (unsigned long)(absH_x100 % 100),
	       (unsigned long)(absL_x100 / 100),
	       (unsigned long)(absL_x100 % 100), (long)(rel_x100 / 100),
	       (long)(rel_x100 < 0 ? (-rel_x100 % 100) : (rel_x100 % 100)),
	       (long)(zero_x100 / 100),
	       (long)(zero_x100 < 0 ? (-zero_x100 % 100) : (zero_x100 % 100)));
	last_print_tick = now;
>>>>>>> 1f20ba5 (完善转向代码)
      }
    }

    /**
     * @note
     * 状态机策略（避免“未回正就触发另一侧转向”）：
     * - 只有在 CENTER 状态下，才允许进入 LEFT/RIGHT；
     * - 在 LEFT/RIGHT 状态下，必须先稳定回到 CENTER 才会切换状态；
     * 这样可以消除抖动/跨0点跳变导致的“先亮另一侧再熄灭”的错觉。
     */
    switch (state)
    {
    case STEER_CENTER:
      /* 左转触发（连续稳定） */
      if (angle >= TURN_ON_DEG)
      {
<<<<<<< HEAD
        if (isStableCenter())
        {
          state = STEER_CENTER;
          xEventGroupSetBits(evt, EVT_TURN_BACK);
        }
=======
	left_cnt++;
	right_cnt = 0;
>>>>>>> 1f20ba5 (完善转向代码)
      }
      /* 右转触发（连续稳定） */
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
      /* 回正触发（连续稳定） */
      if (angle <= CENTER_DEG && angle >= -CENTER_DEG)
      {
	center_cnt++;
      }
      else
      {
	center_cnt = 0;
      }

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

    vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz 采样足够
  }
}

TIM_HandleTypeDef *app_gonio_getTIMHandle() { return &APP_GONIO_TIM; }
