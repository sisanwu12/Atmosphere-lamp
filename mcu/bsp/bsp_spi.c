/**
 * @file		bsp_spi.c
 * @brief		用于定义该模块的函数
 * @author	王广平
 */

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_spi.h"

/**
 * @brief 时钟使能函数
 * @date	2025/12/7
 */
static inline void spi_rcc_enable(SPI_TypeDef *SPIx)
{
  switch ((u32)SPIx)
  {
  case (u32)SPI1:
    __HAL_RCC_SPI1_CLK_ENABLE();
    break;
  case (u32)SPI2:
    __HAL_RCC_SPI2_CLK_ENABLE();
    break;
  }
}

RESULT_Init bsp_spi_Init(SPI_HandleTypeDef *hspi, SPI_TypeDef *SPIx, u32 Mode,
                         u32 Direction, u32 DataSize, u32 CLKPolarity,
                         u32 CLKPhase, u32 NSS, u32 BaudRatePrescaler,
                         u32 FirstBit, u32 TIMode, u32 CRCCalculation,
                         u32 CRCPolynomial)
{
  spi_rcc_enable(SPIx);
  hspi->Instance = SPIx;
  hspi->Init.Mode = Mode;
  hspi->Init.Direction = Direction;
  hspi->Init.DataSize = DataSize;
  hspi->Init.CLKPolarity = CLKPolarity;
  hspi->Init.CLKPhase = CLKPhase;
  hspi->Init.NSS = NSS;
  hspi->Init.BaudRatePrescaler = BaudRatePrescaler;
  hspi->Init.FirstBit = FirstBit;
  hspi->Init.TIMode = TIMode;
  hspi->Init.CRCCalculation = CRCCalculation;
  hspi->Init.CRCPolynomial = CRCPolynomial;

  return HAL_SPI_Init(hspi) ? ERR_Init_ERROR_SPI : ERR_Init_Finished;
}