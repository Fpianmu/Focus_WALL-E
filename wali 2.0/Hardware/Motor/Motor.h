#ifndef __MOTOR_H
#define __MOTOR_H

void Move(int16_t Speed);
void rotate(int16_t Speed);
void Servo_SetAngle(float Angle);
void Motor_Init(void);
void Left(int16_t Speed);
void Right(int16_t Speed);

#endif
