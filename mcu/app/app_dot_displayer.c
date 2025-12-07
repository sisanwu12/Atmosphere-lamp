/**
 * @file		app_dot_displayer.c
 * @brief		定义操作该模块的函数
 * @author	王广平
 */

#define APP_DOT_DISPLAYER_C

/* 头文件引用 */
#include "app_dot_displayer.h"
#include "ERR.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"

/* 全局变量 */
static SPI_HandleTypeDef DOT_DISPLAYER_SPI = {0};

RESULT_Init app_dotD_Init()
{
  RESULT_Init RET;
  /* 初始化GPIO */
  /* 时钟线与数据线初始化 */
  RET = bsp_gpio_AFPP_Init(DOT_GPIOx, CLK_PIN | DIN_PIN);
  if (RET != ERR_Init_Finished)
    return RET;
  /* 片选线初始化 */
  RET = bsp_gpio_OTPP_Init(DOT_GPIOx, CS_PIN);
  if (RET != ERR_Init_Finished)
    return RET;
  /* 拉高片选线 */
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  /* SPI 初始化 */
  RET = bsp_spi_Init(&DOT_DISPLAYER_SPI, DOT_SPI, SPI_MODE_MASTER,
                     SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW,
                     SPI_PHASE_1EDGE, SPI_NSS_SOFT, SPI_BAUDRATEPRESCALER_32,
                     SPI_FIRSTBIT_MSB, SPI_TIMODE_DISABLE,
                     SPI_CRCCALCULATION_DISABLE, 0);
  if (RET != ERR_Init_Finished)
    return RET;

  /* 初始化完成 */
  return ERR_Init_Finished;
}

RESULT_RUN app_dotD_Write(u8 addr, u8 data)
{
  RESULT_RUN RET;
  u8 buf[2] = {addr, data};

  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_RESET);
  RET = (RESULT_RUN)HAL_SPI_Transmit(&DOT_DISPLAYER_SPI, buf, 2, HAL_MAX_DELAY);
  if (RET != ERR_RUN_Finished)
    return RET;
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  return ERR_RUN_Finished;
}
