/**
 * @file    bsp_usart.c
 * @brief   定义操作该模块的函数
 * @author  王广平
 */

/* 引用头文件 */
#include "bsp_usart.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_rcc.h"
#include "string.h"

/**
 * @brief   初始化 USART 函数
 * @param   huart 		传入USART串口结构体
 * @param		USARTx		指定USART
 * @param		BaudRate	指定波特率
 * @return  初始化结果
 * @author	王广平
 * @date	2025/12/4
 */
RESULT_Init bsp_usart_init(UART_HandleTypeDef *huart, USART_TypeDef *USARTx,
                           u32 BaudRate)
{
  /* 开启时钟 */
  if (USARTx == USART1)
    __HAL_RCC_USART1_CLK_ENABLE();
  else if (USARTx == USART2)
    __HAL_RCC_USART2_CLK_ENABLE();

  /* 配置USART参数 */
  huart->Instance = USARTx;
  huart->Init.BaudRate = BaudRate;
  huart->Init.WordLength = UART_WORDLENGTH_8B;
  huart->Init.StopBits = UART_STOPBITS_1;
  huart->Init.Parity = UART_PARITY_NONE;
  huart->Init.Mode = UART_MODE_TX_RX;
  huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart->Init.OverSampling = UART_OVERSAMPLING_16;
  RESULT_Init RES = (RESULT_Init)HAL_UART_Init(huart) ? ERR_Init_ERROR_USART
                                                      : ERR_Init_Finished;
  if (USARTx == USART1)
  {
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  else if (USARTx == USART2)
  {
    HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
  return RES;
}

/**
 * @brief   调用USART发送字符串函数
 * @param   huart 传入USART结构体
 * @param		str		指定发送的字符串
 * @return  运行结果
 * @author	王广平
 * @date	2025/12/4
 */
RESULT_RUN bsp_usart_SendStr(UART_HandleTypeDef *huart, char *str)
{
  RESULT_RUN RES =
      (RESULT_RUN)HAL_UART_Transmit(huart, (uint8_t *)str, strlen(str), 100);
  return RES;
}