/**
 * @file    bsp_dma.c
 * @brief   实现操作该模块的函数
 * @author  王广平
 */

#include "bsp_dma.h"
#include "stm32f1xx_hal_rcc.h"

/**
 * @brief   dma初始化函数
 * @param   DMAx  指定需用启用的DMA
 * @param   hdma  传入DMA句柄
 * @param   cfg   指定需用的配置结构体
 * @note
 * cfg 应当由对应功能的 bsp_dma_conf_x 函数配置
 *
 * @date    2025/12/2
 */
RESULT_Init bsp_dma_init(DMA_HandleTypeDef *hdma, DMA_Init_Config *cfg)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma->Init.Direction = cfg->Direction;
  hdma->Init.PeriphInc = cfg->PeriphInc;
  hdma->Init.MemInc = cfg->MemInc;
  hdma->Init.PeriphDataAlignment = cfg->PeriphDataAlignment;
  hdma->Init.MemDataAlignment = cfg->MemDataAlignment;
  hdma->Init.Mode = cfg->Mode;
  hdma->Init.Priority = cfg->Priority;

  return HAL_DMA_Init(hdma) ? ERR_Init_ERROR_DMA : ERR_Init_Finished;
}

/**
 * @brief   dma在usart串口获取数据模式下的配置函数
 * @param   stream  指定需用使用的 DMA_Stream
 * @param   channel 指定需用使用的通道
 * @date    2025/12/2
 */
DMA_Init_Config bsp_dma_conf_usartRX(DMA_Channel_TypeDef *channel)
{
  DMA_Init_Config cfg;
  cfg.Channel = channel;

  cfg.Direction = DMA_PERIPH_TO_MEMORY;
  cfg.PeriphInc = DMA_PINC_DISABLE;
  cfg.MemInc = DMA_MINC_ENABLE;

  cfg.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  cfg.MemDataAlignment = DMA_MDATAALIGN_BYTE;

  cfg.Mode = DMA_CIRCULAR;

  cfg.Priority = DMA_PRIORITY_HIGH;

  return cfg;
}

DMA_Init_Config bsp_dma_conf_PWM(DMA_Channel_TypeDef *channel)
{
  DMA_Init_Config cfg;
  cfg.Channel = channel;
  cfg.Direction = DMA_PERIPH_TO_MEMORY;
  cfg.PeriphInc = DMA_PINC_DISABLE;
  cfg.MemInc = DMA_MINC_DISABLE;
  cfg.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  cfg.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  cfg.Mode = DMA_CIRCULAR;
  cfg.Priority = DMA_PRIORITY_HIGH;
  return cfg;
}