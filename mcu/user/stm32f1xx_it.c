
#include "FreeRTOS.h"
#include "app_gonio.h"
#include "task.h"

void xPortSysTickHandler(void);

void SysTick_Handler(void)
{
  HAL_IncTick();
  /**
   * @note
   * 这里不要在“调度器未启动”时就调用 xPortSysTickHandler()。
   * 否则 SysTick 中断会进入 FreeRTOS 的 tick 处理流程，但此时任务列表/调度器
   * 还未初始化完成，可能触发 HardFault，表现为：
   * - HAL_Delay() 永远不返回
   * - 程序卡死在某个初始化阶段（例如你点阵的 START 自检）
   */
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    xPortSysTickHandler();
  }
}

/* 定时器3中断函数 */
void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(app_gonio_getTIMHandle()); }

/* 定时器中断回调函数 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /**
   * @note
   * 这里的形参 htim 是 TIM_HandleTypeDef*（句柄指针），不能直接和 TIM3（外设基地址）
   * 做比较。应当比较：
   * - htim->Instance == TIM3
   * 或
   * - htim == app_gonio_getTIMHandle()
   *
   * 若比较写错，会导致输入捕获回调里永远不进入处理函数，从而“读不到磁编码器数据”。
   */
  if (htim != NULL && htim == app_gonio_getTIMHandle())
  {
    app_gonio_dispose_ISP();
  }
}
