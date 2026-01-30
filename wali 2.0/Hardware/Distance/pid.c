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
	//int y;
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
void PeriodElapsedCallback(void)
{
    // 每0.02秒进行一次pid控制
//	int16_t output_value;
//	int16_t distance = sonar_mm();
//	output_value = PID_Control(distance);
//	Move(output_value);
}


void Timer1_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能定时器1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // 配置定时器1基本参数
    TIM_TimeBaseStructure.TIM_Period = 7200 - 1;        // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;     // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;      // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // 使能定时器更新中断
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM1, ENABLE);
}
// 定时器1中断服务函数
void TIM1_UP_IRQHandler(void)
{
    // 检查是否为更新中断
    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        // 清除中断标志位
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        
        // 增加计数器
        timer_counter++;
        
        // 每隔20次中断调用一次回调函数（约0.02秒）
        if(timer_counter >= 20)
        {
            timer_counter = 0;
            // 调用用户定义的回调函数
            PeriodElapsedCallback();
        }
    }
}



