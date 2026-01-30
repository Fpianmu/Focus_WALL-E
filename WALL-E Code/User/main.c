#include "stm32f10x.h"                  // Device header
#include "sys.h"
#include "Delay.h"
#include "usart.h"			//串口1调试信息打印
#include "PAJ7620.h"	//PAJ7620
#include "timer.h"			//定时器中断
#include "OLED.h"			//OLED
#include "pid.h"
#include "Motor.h"
#include "exti.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "HCSR04.h"
#include "Function.h"

/**
  * @brief  系统初始化
  * @param  无
  * @retval 无
  */
void System_Init(void)
{
	//串口初始化
	Serial_Init();
	//电机、舵机初始化
	Motor_Init();
	
	//PID初始化
	PID_Init(0.01,0.1,0.1);   //kp,ki,kd  
	PID_Work(1);
	
	//超声波模块初始化
	HC_SR04_Init();	
	
    // I2C初始化
    GS_i2c_init();
    
    // PAJ7620U2初始化
	while(!paj7620u2_init())
	{
		Delay_ms(500);
	}
    
    // 手势识别初始化
    Gesture_Init();
    
    // 外部中断初始化
    EXTI_Config();
	
	//UI初始化	
	OLED_Init();
    
}

/*
瓦力机器人采用语音交互的形式编写，根据用户命令实时反馈，
在各种语音交互中，请确保为瓦力留足时间接取处理命令，确保环境安静命令清晰
*/


int main(void)
{
    // 系统初始化
    System_Init();
	//清屏
	OLED_Clear();
    //GS_Write_Byte(0x41, 0x00);//将0x41寄存器置0 失能手势识别模块
	GS_Write_Byte(0x41, 0xFF);//将0x41寄存器置0 使能手势识别模块
    while(1)
    {
		while (!Serial_GetRxFlag());  //程序多采用while堵塞来确保接受命令和处理命令
		if (Serial_RxPacket[1] == 0x00 && Serial_RxPacket[2] == 0x00)
		{
			while (!Serial_GetRxFlag());
			if (Serial_RxPacket[1] == 0x02 && Serial_RxPacket[2] == 0x00)
			{
				//剪刀石头布
				judge1();
			}
			if (Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x00)
			{
				//萝卜纸巾大开门模式
				judge2();
			}
			if (Serial_RxPacket[1] == 0x04 && Serial_RxPacket[2] == 0x00)
			{
				while(!Serial_GetRxFlag())
				{
					//手势识别	
					Gesture_Process_Handler();
				}
			}
			if (Serial_RxPacket[1] == 0x05 && Serial_RxPacket[2] == 0x00)
			{
				while(!Serial_GetRxFlag())
				{
					//跟随模式
					PID_Follow();
				}
				Move(0);//听到其他命令后立即停止跟随，停止运动
			}
			Delay_ms(20);
	}
}
}
