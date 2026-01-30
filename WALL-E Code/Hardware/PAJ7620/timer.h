#ifndef __TIMER_H
#define __TIMER_H

//////////////////////////////////////////////////////////////////////////////////	 
// ������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
// STM32F103������ ��ʱ���жϷ������	   
// STM32ѧϰ�����⣬�����qȺ����: 643807576

#include "sys.h"

void PWM_Init(void);
void PWM_SetCompare3(uint16_t Compare);
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetCompare2(uint16_t Compare);
void PWM_SetCompare4(uint16_t Compare);
void PWM_Setcompare3(uint16_t Compare);

#endif
