/**
 * @file event_bus.c
 * @author 肆叁伍12
 * @brief
 * 定义、管理本项目事件组
 * @version 0.1
 * @date 2025-12-09
 */

/* 头文件引用 */
#include "event_bus.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

/* 全局事件组 */
static EventGroupHandle_t MAIN_EVT;

/**
 * @brief 事件组初始化函数
 * @date 2025/12/9
 */
RESULT_Init event_bus_init(void)
{
  MAIN_EVT = xEventGroupCreate();
  if (MAIN_EVT == NULL)
    return ERR_Init_ERROR_RTOS;

  return ERR_Init_Finished;
}

EventGroupHandle_t event_bus_getHandle(void) { return MAIN_EVT; }
