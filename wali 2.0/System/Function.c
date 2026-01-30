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


int16_t pre_dis = 0;
int guess_gesture = 0;
static uint32_t random_seed = 0x12345678;
int get_random(void)
{
	random_seed = (random_seed * 1103515245 + 12345) & 0x7fffffff;
	return (random_seed %3)+1;
}
uint8_t GetRandom1to3(void) {
    static uint32_t counter = 0;
    uint32_t tick = SysTick->VAL;  // 获取当前SysTick值
    uint32_t time = RTC_GetCounter();  // 获取RTC计数器值
    
    // 混合多个变化的源
    counter++;
    uint32_t random_val = tick ^ time ^ counter ^ (uint32_t)&counter;
    
    // 使用Xorshift算法增强随机性
    random_val ^= random_val << 13;
    random_val ^= random_val >> 17;
    random_val ^= random_val << 5;
    
    return (random_val % 3) + 1;
}
void judge1()
{
	OLED_ShowImage(0,0,128,64,Smile);  //UI反馈
	OLED_Update();
	while (!Serial_GetRxFlag());
	//在收到退出石头剪刀布模式之前一直进行该游戏
	while(Serial_RxPacket[1] != 0x02 || Serial_RxPacket[2] != 0x0A)
	{
			OLED_Clear();
			//读取用户所出结果
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
			//瓦力随机给出自己所出结果
			int rob = GetRandom1to3();
			switch(rob)
			{
				case 1:
					Serial_TxPacket[1] = 0x07;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					OLED_ShowImage(0,0,128,64,scissors);
					OLED_Update();
					break;
				case 2:
					Serial_TxPacket[1] = 0x08;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					OLED_ShowImage(0,0,128,64,rock);
					OLED_Update();
					break;
				case 3:
					Serial_TxPacket[1] = 0x09;
					Serial_TxPacket[2] = 0x00;
					Serial_SendPacket();
					OLED_ShowImage(0,0,128,64,paper);
					OLED_Update();
					break;
				default:
					break;
			}
			Delay_ms(1500);  //等待语音助手说完上一句话开始计算
			//比较双方结果，给出胜负
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
			//胜负结果结算给予UI反馈
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
			Servo_SetAngle(90);//自动回正
			while(Serial_RxPacket[1] != 0x03 || Serial_RxPacket[2] != 0x03)
			{
				//当接收到“真棒”指令之前，在萝卜，纸巾之间随机选择，直到选对为止
				if ((Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x01) || (Serial_RxPacket[1] == 0x03 && Serial_RxPacket[2] == 0x02))
				{
					int reaction = (cnt++)%2+1;
					OLED_ShowImage(0,0,128,64,heart);
					OLED_Update();
					Delay_ms(200);
					OLED_Clear();
					//识物模式表现为摆头偏转舵机
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
