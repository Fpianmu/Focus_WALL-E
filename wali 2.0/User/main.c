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

static uint32_t random_seed = 0x12345678;
int guess_gesture = 0;
int16_t pre_dis = 0;
int get_random(void)
{
	random_seed = (random_seed * 1103515245 + 12345) & 0x7fffffff;
	return (random_seed %3)+1;
}
void judge1(void) ;
void judge2(void) ;
void PID_Follow(void) ;
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

/**
  * @brief  主函数
  * @param  无
  * @retval 无
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
		while (!Serial_GetRxFlag());
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
			Move(0);
		}
		Delay_ms(20);
	}
}
}


void judge1()
{
	OLED_ShowImage(0,0,128,64,Smile);
	OLED_Update();
	while (!Serial_GetRxFlag());
	while(Serial_RxPacket[1] != 0x02 || Serial_RxPacket[2] != 0x0A)
	{
			OLED_Clear();
			//读取用户所出结果，瓦力随机给出自己所出结果
			if (Serial_RxPacket[1] == 0x02 && Serial_RxPacket[2] == 0x01)
			{
				guess_gesture = 1; //剪刀
			}
			if (Serial_RxPacket[1] == 0x02 && Serial_RxPacket[2] == 0x02)
			{
				guess_gesture = 2; //石头
			}
			if (Serial_RxPacket[1] == 0x02 && Serial_RxPacket[2] == 0x03)
			{
				guess_gesture = 3; //布
			}
			int rob = get_random();
			switch(rob)
			{
				case 1:
					Serial_TxPacket[1] = 0x07;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					break;
				case 2:
					Serial_TxPacket[1] = 0x08;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					break;
				case 3:
					Serial_TxPacket[1] = 0x09;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					break;
				default:
					break;
			}
			//比较双方结果，给出胜负
			Delay_ms(1500);
			int win_or_lose = 0;
			if (rob == guess_gesture)
			{
				//平局
				Serial_TxPacket[1] = 0x0A;
				Serial_TxPacket[2] = 0x00;
				Serial_SendPacket();
				win_or_lose = 2;
			}
			else if (rob == 1 && guess_gesture == 2)
			{
				//rob lose
				Serial_TxPacket[1] = 0x05;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 0;
			}
			else if (rob == 1 && guess_gesture == 3)
			{
				//rob win
				Serial_TxPacket[1] = 0x06;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 1;
			}
			else if (rob == 2 && guess_gesture == 1)
			{
				//rob win
				Serial_TxPacket[1] = 0x06;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 1;
			}
			else if (rob == 2 && guess_gesture == 3)
			{
				//rob lose
				Serial_TxPacket[1] = 0x05;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 0;
			}
			else if (rob == 3 && guess_gesture == 1)
			{
				//rob lose
				Serial_TxPacket[1] = 0x05;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 0;
			}
			else if (rob == 3 && guess_gesture == 2)
			{
				//rob win
				Serial_TxPacket[1] = 0x06;
				Serial_TxPacket[2] = 0x01;
				Serial_SendPacket();
				win_or_lose = 1;
			}
			OLED_Clear();
			if (win_or_lose == 1)
			{
				OLED_ShowImage(0,0,128,64,happy);
			}
			else if (win_or_lose == 2)
			{
				OLED_ShowImage(0,0,128,64,heart);
			}
			else
			{
				OLED_ShowImage(0,0,128,64,cry);
			}
			OLED_Update();
			while (!Serial_GetRxFlag()) ;
	}
}
void judge2(void)
{
			int cnt = 1;
			while (!Serial_GetRxFlag()) ;
			Servo_SetAngle(90);
			while(Serial_RxPacket[1] != 0x03 || Serial_RxPacket[2] != 0x03)
			{
				//当接收到“真棒”指令之前，在萝卜，纸巾之间随机选择，直到选对为止
				if ((Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x01) || (Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x02))
				{
					int reaction = (cnt++)%2+1;
					OLED_ShowImage(0,0,128,64,heart);
					OLED_Update();
					Delay_s(1);
					OLED_Clear();
					if (reaction == 1)
					{
						Servo_SetAngle(45);
					}
					if (reaction == 2)
					{
						Servo_SetAngle(135);
					}
				}	
				while (!Serial_GetRxFlag()) ;
			}
			//听到真棒跳出循环，展示结果
			if (Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x03)
			{
				Servo_SetAngle(90);
				OLED_ShowImage(0,0,128,64,Smile);
				OLED_Update();
				rotate(100);
				Delay_ms(2300);
				Move(0);
				OLED_Clear();
			}
		
}
void PID_Follow()
{
	int16_t output_value;
	int16_t distance = HC_SR04_GetDistanceMM(); // 超声波测距 单位：mm
	distance = (pre_dis + distance)/2;   //超声波测距滤波减小误差
	output_value = PID_Control(distance);  //PID控制跟随
	Move(output_value);
	Delay_ms(50);
	pre_dis = distance; //更新
}
