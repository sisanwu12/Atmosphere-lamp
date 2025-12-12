/**
 * @file		bsp_timer.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数
 * @author	王广平
 **/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "ERR.h"
#include "__port_type__.h"
#include "stm32f1xx_hal_tim.h"

/**
 * @brief 	timer定时器初始化函数
 * @param		htim
 *	传入定时器结构体指针
 * @param		timx					指定定时器
 * @param		Prescaler			指定预分频系数
 * @param		CounterMode		指定计数模式
 * @param		Period				指定自动装填值
 * @param		ClockDivision	指定时钟分频系数
 * @param		AutoReloadPreload
 *	ARR预装载,指定定时器的周期（ARR）是马上生效，还是等下一次更新事件才生效
 * @param		RepetitionCounter
 * 指定定时器更新事件的触发频率为每 n+1 次触发一次
 * @return	初始化的结果
 * @date		2025/11/24
 **/
RESULT_Init bsp_timer_SetStruct(TIM_HandleTypeDef *htim, TIM_TypeDef *timx,
                                u32 Prescaler, u32 CounterMode, u32 Period,
                                u32 ClockDivision, u32 AutoReloadPreload,
                                u32 RepetitionCounter);

/**
 * @brief   PWM模式定时器初始化函数
 * @param   htim    传入定时器结构体
 * @param   OCMode  指定输出比较模式
 * @note
 * TIM_OCMODE_PWM1：计数器 < CCR 时输出有效电平
 * TIM_OCMODE_PWM2：计数器 < CCR 时输出无效电平
 *
 * @param   Pulse   指定脉冲宽度（占空比）
 * @note
 * 实际写入的是CCRx的值。
 * 周期由 ARR 定义，占空比 = Pulse / (ARR + 1)。
 *
 * @param   OCPolarity  指定主输出极性
 * @note
 * TIM_OCPOLARITY_HIGH：有效电平 = 高电平
 * TIM_OCPOLARITY_LOW： 有效电平 = 低电平
 *
 * @param   OCFastMode  指定是否开启快速模式
 * @note
 * TIM_OCFAST_DISABLE：禁用快速模式（常用）
 * TIM_OCFAST_ENABLE： 启用快速模式
 *
 * @param   OCIdleState 指定主输出空闲状态
 * @note
 * TIM_OCIDLESTATE_RESET：空闲时输出低电平
 * TIM_OCIDLESTATE_SET：  空闲时输出高电平
 *
 * @param   OCNIdleState  指定互补输出空闲状态
 * @note
 * 同 OCIdleState，该相是给高级定时器的互补输出配置的
 *
 * @param   OCNPolarity   指定互补输出极性
 * @note
 * 同样是给高级定时器的互补输出配置的
 *
 * @param   Channel   指定输出通道
 * @date 2025/12/12
 */
RESULT_Init bsp_timer_PWM_init(TIM_HandleTypeDef *htim, u32 OCMode, u32 Pulse,
                               u32 OCPolarity, u32 OCFastMode, u32 OCIdleState,
                               u32 OCNIdleState, u32 OCNPolarity, u32 Channel);

#endif