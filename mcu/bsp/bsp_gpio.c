/**
 * @file		bsp_gpio.c
 * @brief		用于定义该模块的函数
 * @author	王广平
 * @date		2025/11/23
 */

/* 头文件引用 */
#include "bsp_gpio.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_rcc_ex.h"

/**
 * @brief		GPIO时钟使能函数
 */
static inline void bsp_rcc_enable(GPIO_TypeDef *GPIOx)
{
  switch ((u32)GPIOx)
  {
  case (u32)GPIOA:
    __HAL_RCC_GPIOA_CLK_ENABLE();
  case (u32)GPIOB:
    __HAL_RCC_GPIOB_CLK_ENABLE();
  case (u32)GPIOC:
    __HAL_RCC_GPIOC_CLK_ENABLE();
  case (u32)GPIOD:
    __HAL_RCC_GPIOD_CLK_ENABLE();
  case (u32)GPIOE:
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
}

/**
 * @brief		gpio初始化函数
 * @param		GPIOx 		指定GPIO组
 * @param		GPIOpin		指定GPIO引脚
 * @param		GPIOMode	指定GPIO模式
 * @param		GPIOPull	指定GPIO上拉下拉
 * @param		GPIOSpeed	指定GPIO速度
 * @return	初始化结果
 * @date		2025/11/24
 */
RESULT_Init bsp_gpio_Init(GPIO_TypeDef *GPIOx, u32 GPIOpin, u32 GPIOMode,
                          u32 GPIOPull, u32 GPIOSpeed)
{
  RESULT_Init ret = ERR_Init_Start;
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使能 GPIO 时钟 */
  bsp_rcc_enable(GPIOx);
  /* 配置 GPIO */
  GPIO_InitStruct.Pin = GPIOpin;
  GPIO_InitStruct.Mode = GPIOMode;
  GPIO_InitStruct.Pull = GPIOPull;
  GPIO_InitStruct.Speed = GPIOSpeed;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

  ret = ERR_Init_Finished;
  return ret;
}