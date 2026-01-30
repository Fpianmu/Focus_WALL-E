#include "stm32f10x.h"                  // Device header
#include "timer.h"

void Motor_Init(void)
{
	PWM_Init();
}
void Servo_SetAngle(float Angle)
{
	PWM_Setcompare3(Angle / 180 * 2000 + 500);
}
void Move(int16_t Speed)
{
	if (Speed > 0)
	{
		PWM_SetCompare2(Speed);
		PWM_SetCompare1(0);
		PWM_SetCompare3(Speed);
		PWM_SetCompare4(0);
	}
	else if (Speed < 0)
	{
		PWM_SetCompare2(0);
		PWM_SetCompare1(-Speed);
		PWM_SetCompare3(0);
		PWM_SetCompare4(-Speed);
	}
	else
	{
		PWM_SetCompare1(0);
		PWM_SetCompare2(0);
		PWM_SetCompare3(0);
		PWM_SetCompare4(0);
	}
}
void rotate(int16_t Speed)
{
	if (Speed > 0)
	{
		PWM_SetCompare2(Speed);
		PWM_SetCompare1(0);
		PWM_SetCompare3(0);
		PWM_SetCompare4(Speed);
	}
	else if (Speed < 0)
	{
		PWM_SetCompare2(0);
		PWM_SetCompare1(-Speed);
		PWM_SetCompare3(-Speed);
		PWM_SetCompare4(0);
	}
	else
	{
		PWM_SetCompare1(0);
		PWM_SetCompare2(0);
		PWM_SetCompare3(0);
		PWM_SetCompare4(0);
	}
}
void Left(int16_t Speed)
{
		PWM_SetCompare1(0);
		PWM_SetCompare2(0);
		PWM_SetCompare4(0);
		PWM_SetCompare3(Speed);
}
void Right(int16_t Speed)
{
		PWM_SetCompare2(Speed);
		PWM_SetCompare1(0);
		PWM_SetCompare3(0);
		PWM_SetCompare4(0);
}
