/**
 * @file		main.c
 * @author	王广平
 **/

/* 头文件引用 */
#include "FreeRTOS.h"
#include "app_debug.h"
#include "app_dot_displayer.h"
#include "app_gonio.h"
#include "app_trun_lamp.h"
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

    vTaskDelay(pdMS_TO_TICKS(1000));
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
  app_trunL_init();
  app_gonio_init();
  app_dotD_Init();

  app_trunL_open_left();
  app_trunL_open_right();

  xTaskCreate(Task_Angle, "Angle", 256, NULL, 2, NULL);

  vTaskStartScheduler();
  while (1)
    ;
}