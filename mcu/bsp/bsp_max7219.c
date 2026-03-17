/**
 * @file bsp_max7219.c
 * @brief MAX7219 board support driver implementation.
 */

#include "bsp_max7219.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"

static SPI_HandleTypeDef BSP_MAX7219_HANDLE = {0};
static bool BSP_MAX7219_READY = false;

RESULT_Init bsp_max7219_init(void)
{
  RESULT_Init ret = bsp_gpio_AFPP_Init(
      BSP_MAX7219_GPIOx, BSP_MAX7219_CLK_PIN | BSP_MAX7219_DIN_PIN);
  if (ret != ERR_Init_Finished)
    return ret;

  ret = bsp_gpio_OTPP_Init(BSP_MAX7219_GPIOx, BSP_MAX7219_CS_PIN);
  if (ret != ERR_Init_Finished)
    return ret;

  HAL_GPIO_WritePin(BSP_MAX7219_GPIOx, BSP_MAX7219_CS_PIN, GPIO_PIN_SET);

  ret = bsp_spi_Init(&BSP_MAX7219_HANDLE, BSP_MAX7219_SPI, SPI_MODE_MASTER,
                     SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW,
                     SPI_PHASE_1EDGE, SPI_NSS_SOFT, SPI_BAUDRATEPRESCALER_32,
                     SPI_FIRSTBIT_MSB, SPI_TIMODE_DISABLE,
                     SPI_CRCCALCULATION_DISABLE, 0);
  if (ret != ERR_Init_Finished)
    return ret;

  BSP_MAX7219_READY = true;

  if (bsp_max7219_set_test_mode(false) != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;
  if (bsp_max7219_write_register(0x09, 0x00) != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;
  if (bsp_max7219_write_register(0x0A, 0x0F) != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;
  if (bsp_max7219_write_register(0x0B, 0x07) != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;
  if (bsp_max7219_write_register(0x0C, 0x01) != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;
  if (bsp_max7219_clear() != ERR_RUN_Finished)
    return ERR_Init_ERROR_SPI;

  return ERR_Init_Finished;
}

RESULT_RUN bsp_max7219_write_register(u8 addr, u8 data)
{
  u8 buf[2] = {addr, data};

  if (!BSP_MAX7219_READY)
    return ERR_RUN_ERROR_UNST;

  HAL_GPIO_WritePin(BSP_MAX7219_GPIOx, BSP_MAX7219_CS_PIN, GPIO_PIN_RESET);
  if (HAL_SPI_Transmit(&BSP_MAX7219_HANDLE, buf, 2, HAL_MAX_DELAY) != HAL_OK)
  {
    HAL_GPIO_WritePin(BSP_MAX7219_GPIOx, BSP_MAX7219_CS_PIN, GPIO_PIN_SET);
    return ERR_RUN_ERROR_CALL;
  }
  HAL_GPIO_WritePin(BSP_MAX7219_GPIOx, BSP_MAX7219_CS_PIN, GPIO_PIN_SET);

  return ERR_RUN_Finished;
}

RESULT_RUN bsp_max7219_write_rows(const u8 rows[8])
{
  u8 row = 0;

  if (rows == NULL)
    return ERR_RUN_ERROR_UDIP;

  for (row = 1; row <= 8; row++)
  {
    RESULT_RUN ret = bsp_max7219_write_register(row, rows[row - 1]);
    if (ret != ERR_RUN_Finished)
      return ret;
  }

  return ERR_RUN_Finished;
}

RESULT_RUN bsp_max7219_clear(void)
{
  u8 row = 0;

  for (row = 1; row <= 8; row++)
  {
    RESULT_RUN ret = bsp_max7219_write_register(row, 0x00);
    if (ret != ERR_RUN_Finished)
      return ret;
  }

  return ERR_RUN_Finished;
}

RESULT_RUN bsp_max7219_set_test_mode(bool enable)
{
  return bsp_max7219_write_register(0x0F, enable ? 0x01 : 0x00);
}
