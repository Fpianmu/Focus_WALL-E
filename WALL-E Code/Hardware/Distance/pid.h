#ifndef __pid_H
#define __pid_H

typedef struct
{
	float kp;
	float ki;
	float kd;
	int16_t target;
	int16_t pre_error;
	int16_t last_error;
	int16_t error;
	int16_t target_speed;
	int16_t output_value;
	uint8_t work_status;
} PID_TypeDef ;
void PID_Init(float kp,float ki,float kd);
void PID_Work(uint8_t status);
int16_t PID_Control(int16_t dis);

#endif
