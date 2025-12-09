
#include "FreeRTOS.h"
#include "app_gonio.h"
#include "task.h"

void xPortSysTickHandler(void);

void SysTick_Handler(void)
{
  HAL_IncTick();
  xPortSysTickHandler();
}

/* 定时器3中断函数 */
void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(app_gonio_getTIMHandle()); }

/* 定时器中断回调函数 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim == TIM3)
  {
    app_gonio_dispose_ISP();
  }
}