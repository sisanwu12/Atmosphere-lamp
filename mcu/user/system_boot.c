/**
 * @file system_boot.c
 * @brief System boot orchestration and clock configuration.
 */

#include "system_boot.h"
#include "FreeRTOS.h"
#include "app_can.h"
#include "app_debug.h"
#include "app_dot_displayer.h"
#include "app_gonio.h"
#include "app_state.h"
#include "app_trun_lamp.h"
#include "event_bus.h"
#include "stm32f1xx_hal.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

static void Task_Angle(void *arg)
{
  (void)arg;
  app_gonio_dispose_Task();
}

static void Task_Trun(void *arg)
{
  (void)arg;
  app_trunL_dispose_Task();
}

static void Task_DotD(void *arg)
{
  (void)arg;
  app_dotD_dispose_Task();
}

static void Task_CAN(void *arg)
{
  (void)arg;
  app_can_dispose_Task();
}

static RESULT_Init system_boot_create_task(TaskFunction_t task_func,
                                           const char *name,
                                           uint16_t stack_depth)
{
  if (xTaskCreate(task_func, name, stack_depth, NULL, 2, NULL) != pdPASS)
    return ERR_Init_ERROR_RTOS;

  return ERR_Init_Finished;
}

static RESULT_Init SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;
  osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  osc.HSIState = RCC_HSI_ON;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLMUL = RCC_PLL_MUL9;

  if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    return ERR_Init_ERROR_CLOCK;

  clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    return ERR_Init_ERROR_CLOCK;

  SystemCoreClockUpdate();
  return ERR_Init_Finished;
}

RESULT_Init system_boot_run(void)
{
  RESULT_Init ret = ERR_Init_Start;
  bool debug_ready = false;

  HAL_Init();

  ret = SystemClock_Config();
  if (ret != ERR_Init_Finished)
    return ret;

  ret = event_bus_init();
  if (ret != ERR_Init_Finished)
    return ret;

  app_state_init();

  ret = app_debug_init();
  if (ret != ERR_Init_Finished)
    return ret;
  debug_ready = true;

  ret = app_trunL_init();
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = app_gonio_init();
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = app_dotD_Init();
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = app_can_init();
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = system_boot_create_task(Task_Angle, "Angle", 256);
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = system_boot_create_task(Task_Trun, "Trun", 128);
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = system_boot_create_task(Task_DotD, "Task_DotD", 256);
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  ret = system_boot_create_task(Task_CAN, "CAN", 128);
  if (ret != ERR_Init_Finished)
    goto boot_fail;

  return ERR_Init_Finished;

boot_fail:
  if (debug_ready)
  {
    printf("[BOOT] initialization failed\r\n");
    ERR_ShowBy_USART_Init(ret);
  }
  return ret;
}
