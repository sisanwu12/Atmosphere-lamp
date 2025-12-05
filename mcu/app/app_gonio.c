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
#include "bsp_dma.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"

/* 静态全局变量 */
static TIM_HandleTypeDef APP_GONIO_TIM = {0};
DMA_HandleTypeDef APP_GONIO_ch1 = {0};
DMA_HandleTypeDef APP_GONIO_ch2 = {0};
/* PWM 周期 */
static volatile u16 APP_GONIO_PERIOD = 0;
/* PWM 高电平 */
static volatile u16 APP_GONIO_PULSE = 0;

/**
 * @brief		角度测量模块初始化函数
 * @return	初始化结果
 * @date		2025/11/24
 **/
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
  TIM_IC_InitTypeDef sConfigIC = {0};
  sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING; // CH1: 周期
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;

  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfigIC, TIM_CHANNEL_1);

  sConfigIC.ICPolarity = TIM_ICPOLARITY_FALLING; // CH2: 脉宽
  HAL_TIM_IC_ConfigChannel(&APP_GONIO_TIM, &sConfigIC, TIM_CHANNEL_2);

  // 从模式触发（经典 PWM Input）
  TIM_SlaveConfigTypeDef slave = {0};
  slave.SlaveMode = TIM_SLAVEMODE_RESET;
  slave.InputTrigger = TIM_TS_TI1FP1;
  HAL_TIM_SlaveConfigSynchro(&APP_GONIO_TIM, &slave);

  /* 配置DMA */
  DMA_Init_Config cfg = bsp_dma_conf_PWM(GONIO_DMA_CH1);
  bsp_dma_init(&APP_GONIO_ch1, &cfg);
  __HAL_LINKDMA(&APP_GONIO_TIM, hdma[TIM_DMA_ID_CC1], APP_GONIO_ch1);
  cfg = bsp_dma_conf_PWM(GONIO_DMA_CH2);
  bsp_dma_init(&APP_GONIO_ch2, &cfg);
  __HAL_LINKDMA(&APP_GONIO_TIM, hdma[TIM_DMA_ID_CC2], APP_GONIO_ch2);

  // 启动 DMA
  HAL_DMA_Start(&APP_GONIO_ch1, (uint32_t)&TIM3->CCR1,
                (uint32_t)&APP_GONIO_PERIOD, 1);
  HAL_DMA_Start(&APP_GONIO_ch2, (uint32_t)&TIM3->CCR2,
                (uint32_t)&APP_GONIO_PULSE, 1);

  // 启动输入捕获 + DMA
  HAL_TIM_IC_Start_DMA(&APP_GONIO_TIM, TIM_CHANNEL_1,
                       (uint32_t *)&APP_GONIO_PERIOD, 1);
  HAL_TIM_IC_Start_DMA(&APP_GONIO_TIM, TIM_CHANNEL_2,
                       (uint32_t *)&APP_GONIO_PULSE, 1);

  ret = ERR_Init_Finished;
  return ret;
}

u16 app_gonio_GetAngle(void)
{
  u16 p = APP_GONIO_PERIOD;
  u16 h = APP_GONIO_PULSE;

  if (p == 0)
    return 0;

  float duty = (float)h / (float)p;
  float angle = duty * 360.0f;

  return (u16)angle;
}