#ifndef __EXTI_H
#define __EXTI_H

#include "stm32f10x.h"
#include "PAJ7620.h"

// 外部中断配置
void EXTI_Config(void);
void EXTI_Gesture_IRQHandler(void);
uint8_t EXTI_GetGestureFlag(void);
void EXTI_ClearGestureFlag(void);

// 手势处理函数
void Gesture_Process_Handler(void);

#endif
