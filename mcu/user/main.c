/**
 * @file		main.c
 * @author	王广平
 **/

/* 头文件引用 */
#include "FreeRTOS.h"
#include "system_boot.h"
#include "task.h"

/**
 * @brief		主函数
 * @date		2025/12/4
 **/
int main(void)
{
  if (system_boot_run() == ERR_Init_Finished)
  {
    vTaskStartScheduler();
  }

  while (1)
    ;
}
