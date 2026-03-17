/**
 * @file		app_dot_displayer.h
 * @brief		点阵显示应用层接口。
 * @author	王广平
 */

#ifndef __APP_DOT_DISPLAYER_H
#define __APP_DOT_DISPLAYER_H

/* 头文件引用 */
#include "ERR.h"

#ifndef APP_DOTD_TURN_COUNT
#define APP_DOTD_TURN_COUNT 0U
#endif

RESULT_Init app_dotD_Init(void);
void app_dotD_dispose_Task(void);

#endif
