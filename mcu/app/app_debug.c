/**
 * @file		app_debug.c
 * @brief		定义操作该模块的函数
 * @author	王广平
 */

#define APP_DEBUG_C

/* 头文件引用 */
#include "app_debug.h"
#include "bsp_gpio.h"
#include "bsp_usart.h"
#include "stdio.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"

/* 静态全局变量 */
static UART_HandleTypeDef Debug_USART = {0};

/**
 * @brief		重定向printf函数
 * @date		2025/11/12
 */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&Debug_USART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/**
 * @brief   debug初始化函数
 *
 * @return  初始化结果
 * @date    2025/12/4
 *
 */
RESULT_Init app_debug_init()
{
  RESULT_Init ret = ERR_Init_Start;

  /* 初始化GPIO引脚 */
  bsp_gpio_Init(Debug_GPIOx, Debug_RX | Debug_TX, GPIO_MODE_AF_PP, GPIO_NOPULL,
                GPIO_SPEED_FREQ_HIGH);

  /* 初始化USART */
  bsp_usart_init(&Debug_USART, Debug_USARTx, Debug_BRate);
  ret = ERR_Init_Finished;
  return ret;
}

/**
 * @brief   调用USART发送错误信息函数，本质就是bsp_usart_SendStr函数
 * @param   usart		指定发送的通道
 * @param		res_run	指定发送的运行错误信息
 * @author	王广平
 * @date	2025/12/4
 */
void ERR_ShowBy_USART_RUN(RESULT_RUN res_run)
{
  if (res_run != ERR_RUN_Finished)
    printf("Run Err:");
  switch (res_run)
  {
  case ERR_RUN_Finished:
    printf("Run Finished");
    break;
  case ERR_RUN_ERROR_UNST:
    printf("UnInit");
    break;
  case ERR_RUN_BUSY:
    printf("Busy");
    break;
  case ERR_RUN_TIMEOUT:
    printf("TimeOut");
    break;
  case ERR_RUN_ERROR_UDIP:
    printf("UDparam");
    break;
  case ERR_RUN_ERROR_CALL:
    printf("FunCall");
    break;
  case ERR_RUN_ERROR_ERIP:
    printf("ErrInPut");
    break;
  }
  printf("\r\n");
}

/**
 * @brief   调用USART发送错误信息函数，本质就是bsp_usart_SendStr函数
 * @param   usart			指定发送的通道
 * @param		res_Init	指定发送的初始化错误信息
 * @author	王广平
 * @date	2025/12/4
 */
void ERR_ShowBy_USART_Init(RESULT_Init res_init)
{
  switch (res_init)
  {
  case ERR_Init_Start:
    printf("Init Start:");
    return;
  case ERR_Init_Finished:
    printf("Init Finished\r\n");
    return;
  case ERR_Init_ERROR_TIM:
    printf("TIM");
    break;
  case ERR_Init_ERROR_GPIO:
    printf("GPIO");
    break;
  case ERR_Init_ERROR_USART:
    printf("USART");
    break;
  case ERR_Init_ERROR_PWM:
    printf("PWM");
    break;
  case ERR_Init_ERROR_SPI:
    printf("SPI");
    break;
  case ERR_Init_ERROR_ADC:
    printf("ADC");
    break;
  }
  printf(" Err Init\r\n");
}