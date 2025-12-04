/**
 * @file		main.c
 * @author	王广平
 **/

/* 头文件引用 */
#include "FreeRTOS.h"
#include "app_basic_trunSL.h"
#include "app_debug.h"
#include "app_gonio.h"
#include "stdio.h"
#include "task.h"

/**
 * @brief		主函数
 * @date		2025/12/4
 **/
int main(void)
{
  HAL_Init();
  app_trunSL_init();
  app_gonio_init();
  app_debug_init();

  printf("all is init\r\n");

  while (1)
    ;
  // vTaskStartScheduler();
}