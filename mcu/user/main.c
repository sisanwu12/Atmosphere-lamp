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

void MAX7219_ScanTest(void)
{
  HAL_Delay(200);
  // 每行最左
  for (uint8_t r = 1; r <= 8; r++)
  {
    for (uint8_t i = 1; i <= 8; i++)
      app_dotD_Write(i, 0x00);
    app_dotD_Write(r, 0x80);
    HAL_Delay(300);
  }
  HAL_Delay(500);
  // 每行最右
  for (uint8_t r = 1; r <= 8; r++)
  {
    for (uint8_t i = 1; i <= 8; i++)
      app_dotD_Write(i, 0x00);
    app_dotD_Write(r, 0x01);
    HAL_Delay(300);
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

  MAX7219_ScanTest();

  xTaskCreate(Task_Angle, "Angle", 256, NULL, 2, NULL);

  vTaskStartScheduler();
  while (1)
    ;
}