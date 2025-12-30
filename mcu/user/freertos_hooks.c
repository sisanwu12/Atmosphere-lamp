/**
 * @file    freertos_hooks.c
 * @brief   FreeRTOS 钩子函数（栈溢出/堆不足等）
 * @author  肆叁伍12
 * @date    2025/12/30
 *
 * @note
 * 这些钩子用于“系统出现严重错误”时辅助定位：
 * - 栈溢出：任务栈太小（printf/局部数组/递归等都可能导致）
 * - malloc 失败：堆不足（configTOTAL_HEAP_SIZE 太小或泄漏）
 *
 * 触发后一般会进入死循环，避免系统继续运行造成更大破坏。
 */

/* 头文件引用 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/**
 * @brief 栈溢出钩子
 * @param xTask      发生溢出的任务句柄
 * @param pcTaskName 发生溢出的任务名
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;

  /* 注意：此时系统可能已经不稳定，尽量只做最少的事情 */
  printf("[FreeRTOS] StackOverflow: task=%s\r\n",
         (pcTaskName != NULL) ? pcTaskName : "NULL");

  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}

/**
 * @brief malloc 失败钩子
 */
void vApplicationMallocFailedHook(void)
{
  printf("[FreeRTOS] MallocFailed: heap exhausted\r\n");

  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}

