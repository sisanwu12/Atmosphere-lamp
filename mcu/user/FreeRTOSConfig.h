#include "stm32f1xx_hal.h"

#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configTICK_RATE_HZ (1000)         // 1ms tick
#define configTOTAL_HEAP_SIZE (10 * 1024) // C8T6 内存有限，这样比较稳

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler

#define INCLUDE_vTaskDelay 1
#define configUSE_PREEMPTION 1
#define configUSE_TICKLESS_IDLE 0
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

#define configUSE_16_BIT_TICKS 0

#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 128

#define configUSE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1

#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 3
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH 256

/* ============================= 调试/健壮性配置 ============================= */
/**
 * @brief 栈溢出检测（强烈建议开启）
 * @note
 * - 1：只检测任务切换时的栈边界
 * - 2：更严格（推荐）
 */
#define configCHECK_FOR_STACK_OVERFLOW 2

/**
 * @brief malloc 失败钩子（堆不足时可定位问题）
 */
#define configUSE_MALLOC_FAILED_HOOK 1

// 断言方式
#define configASSERT(x)                                                        \
  if ((x) == 0)                                                                \
  {                                                                            \
    taskDISABLE_INTERRUPTS();                                                  \
    for (;;)                                                                   \
      ;                                                                        \
  }

// 中断优先级配置
#define configPRIO_BITS 4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY (15 << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (5 << 4)
