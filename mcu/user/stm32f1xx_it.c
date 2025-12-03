
#include "FreeRTOS.h"
#include "task.h"

void xPortSysTickHandler(void);

void SysTick_Handler(void)
{
  HAL_IncTick();
  xPortSysTickHandler();
}