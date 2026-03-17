/**
 * @file system_boot.h
 * @brief System boot orchestration and clock contract.
 */

#ifndef __SYSTEM_BOOT_H
#define __SYSTEM_BOOT_H

#include "ERR.h"

#define SYSTEM_BOOT_SYSCLK_HZ 72000000UL
#define SYSTEM_BOOT_APB1_HZ 36000000UL
#define SYSTEM_BOOT_APB2_HZ 72000000UL
#define SYSTEM_BOOT_APB1_TIMER_HZ (SYSTEM_BOOT_APB1_HZ * 2UL)

RESULT_Init system_boot_run(void);

#endif
