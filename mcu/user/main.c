/**
 * @file		main.c
 * @author	王广平
 **/

/* 头文件引用 */
#include "FreeRTOS.h"
#include "app_basic_trunSL.h"
#include "task.h"

/**
 * @brief		主逻辑循环
 * @date		2025-11-24
 **/
void MAIN_LOOP()
{

  while (1)
  {
  }
}

void Task_LLED(void *pvParameters)
{
  while (1)
  {
    app_trunSL_open_left();
    vTaskDelay(500);
  }
}

void Task_RLED(void *pvParameters)
{
  while (1)
  {
    app_trunSL_open_right();
    vTaskDelay(200);
  }
}

/**
 * @brief		主函数
 * @date		2025-11-24
 **/
int main(void)
{
  app_trunSL_init();

  xTaskCreate(Task_LLED, "LLED", 128, NULL, 1, NULL);

  xTaskCreate(Task_RLED, "ELED", 128, NULL, 1, NULL);

  vTaskStartScheduler();
  /* 主逻辑 */
  MAIN_LOOP();
}