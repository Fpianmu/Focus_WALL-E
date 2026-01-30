#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "timer.h"
#include "HCSR04.h"
#include "pid.h"
#include <math.h>

PID_TypeDef pid;
const float dt = 20;
volatile uint32_t timer_counter = 0;

void PID_Init(float kp,float ki,float kd)
{
	pid.kp = kp;
	pid.ki = ki;
	pid.kd = kd;
	
	pid.target = 30;
	pid.pre_error = 0;
	pid.last_error = 0;
	pid.error = 0;
	pid.target_speed = 0;
	pid.output_value = 0;
	pid.work_status = 0;
}
void PID_Work(uint8_t status)
{
		pid.work_status = status;
}
int16_t PID_Control(int16_t dis)
{
	dis/=10;
	if (pid.work_status)
	{
		//基础速度为0，则pid可以调节大于0小于0两个区间 人为设定浮动最大限度5厘米
		pid.error = dis-pid.target;
		if (pid.error <= 5 && pid.error >= -5) 
		{
			return pid.target_speed;
		}
		//增量式PID控制算法
			pid.output_value = pid.kp*(pid.error-pid.last_error) 
						+pid.ki*pid.error*dt
						+pid.kd*(pid.error - 2*pid.last_error + pid.pre_error)/dt;   //单位：cm/ms
		
		pid.pre_error = pid.last_error;
		pid.last_error = pid.error;
		if (pid.output_value >= 100)
		{
			pid.output_value = 100;
		}
		else if (pid.output_value <= -100)
		{
			pid.output_value = -100;
		}
		else if (pid.output_value >= -60 && pid.output_value < 0)
		{
			pid.output_value = -60;
		}
		else if (pid.output_value <= 60 && pid.output_value > 0)
		{
			pid.output_value = 60;
		}
		else
		{
			pid.output_value = pid.output_value;
		}
		return pid.output_value ;
	}
	else
	{
		return 0;
	}
}




