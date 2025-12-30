/**
 * @file		main.c
 * @author	王广平
 **/

/* 头文件引用 */
#include "FreeRTOS.h"
#include "app_can.h"
#include "app_debug.h"
#include "app_dot_displayer.h"
#include "app_gonio.h"
#include "app_trun_lamp.h"
#include "event_bus.h"
#include "task.h"

void Task_Angle(void *arg) { app_gonio_dispose_Task(); }

void Task_Trun(void *arg) { app_trunL_dispose_Task(); }

void Task_DotD(void *arg) { app_dotD_dispose_Task(); }

void Task_CAN(void *arg) { app_can_dispose_Task(); }

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
  app_can_init();

  /* 添加方向盘角度检测线程 */
  /**
   * @note
   * Angle 线程里会做串口打印（printf），栈占用明显高于纯逻辑处理。
   * 128（word）在某些 printf 配置下可能不够，容易出现“打印一会儿就停了”的情况。
   */
  xTaskCreate(Task_Angle, "Angle", 256, NULL, 2, NULL);
  /* 添加转向灯响应线程 */
  xTaskCreate(Task_Trun, "Trun", 128, NULL, 2, NULL);
  /* 添加点阵灯响应线程 */
  xTaskCreate(Task_DotD, "Task_DotD", 256, NULL, 2, NULL);
  /* 添加 CAN 协议解析线程（负责将加速/减速/停车报文转为事件） */
  xTaskCreate(Task_CAN, "CAN", 128, NULL, 2, NULL);

  vTaskStartScheduler();

  while (1)
    ;
}
