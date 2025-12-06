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

void Task_Angle(void *arg)
{
  for (;;)
  {
    float angle = app_gonio_GetAngleDeg();
    if (angle >= 0)
    {
      printf("Ang=%.2f deg\r\n", angle);
    }
    else
      printf("Ang Err\r\n");

    vTaskDelay(pdMS_TO_TICKS(1000)); // 10Hz 采样
  }
}

/**
 * @brief		主函数
 * @date		2025/12/4
 **/
int main(void)
{
  HAL_Init();
  app_debug_init();
  printf("Debug\r\n");
  app_trunSL_init();
  printf("TRUNSL\r\n");
  app_gonio_init();
  printf("gonio\r\n");

  app_trunSL_open_left();
  app_trunSL_open_right();

  xTaskCreate(Task_Angle, "Angle", 256, NULL, 2, NULL);

  vTaskStartScheduler();
  while (1)
    ;
}