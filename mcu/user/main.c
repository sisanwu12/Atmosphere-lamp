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
#include "event_bus.h"
#include "task.h"

void Task_Angle(void *arg)
{
  for (;;)
  {
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
  event_bus_init();
  app_debug_init();
  app_trunL_init();
  app_gonio_init();
  app_dotD_Init();

  /* 添加方向盘角度检测线程 */
  xTaskCreate(Task_Angle, "Angle", 256, NULL, 2, NULL);

  vTaskStartScheduler();
  while (1)
    ;
}