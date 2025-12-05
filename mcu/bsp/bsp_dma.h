
#ifndef __BSP_DMA_H
#define __BSP_DMA_H

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f1xx_hal_dma.h"

/* 初始化结构体定义 */
typedef struct
{
  DMA_Channel_TypeDef *Channel;
  u32 Direction;
  u32 PeriphInc;
  u32 MemInc;
  u32 PeriphDataAlignment;
  u32 MemDataAlignment;
  u32 Mode;
  u32 Priority;
  u32 FIFOMode;
  u32 FIFOThreshold;
} DMA_Init_Config;

/* 函数声明 */
RESULT_Init bsp_dma_init(DMA_HandleTypeDef *hdma, DMA_Init_Config *cfg);
DMA_Init_Config bsp_dma_conf_usartRX(DMA_Channel_TypeDef *channel);
DMA_Init_Config bsp_dma_conf_PWM(DMA_Channel_TypeDef *channel);
#endif