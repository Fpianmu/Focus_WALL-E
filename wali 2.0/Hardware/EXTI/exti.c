#include "exti.h"
#include "i2c.h"
#include "usart.h"
#include "Delay.h"
#include "OLED.h"
#include "Motor.h"

// 全局变量
volatile uint8_t gesture_detected_flag = 0;
volatile uint32_t gesture_detected_time = 0;

/**
  * @brief  外部中断配置（PB5作为INT_N引脚）
  * @param  无
  * @retval 无
  */
void EXTI_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 开启GPIOB和AFIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 配置PB5为浮空输入（INT_N引脚）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 将PB5连接到EXTI5
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
    
    // 配置EXTI5
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发（INT_N低电平有效）
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
		
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;  // PB5属于EXTI9_5中断线
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //printf("External Interrupt (PB5) Configured Successfully!\r\n");
}

/**
  * @brief  获取手势检测标志
  * @param  无
  * @retval 手势检测标志
  */
uint8_t EXTI_GetGestureFlag(void)
{
    return gesture_detected_flag;
}

/**
  * @brief  清除手势检测标志
  * @param  无
  * @retval 无
  */
void EXTI_ClearGestureFlag(void)
{
    gesture_detected_flag = 0;
}

/**
  * @brief  手势处理中断服务函数
  * @param  无
  * @retval 无
  */
void EXTI_Gesture_IRQHandler(void)
{
    //uint8_t int_flag1 = 0, int_flag2 = 0;
    
    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        
				//OLED_ShowString(2,1,"CLICK");
			  EXTI_ClearITPendingBit(EXTI_Line5);
       
        gesture_detected_flag = 1;
        // 检查手势中断标志

				
    }
}

/**
  * @brief  手势处理函数（在主循环中调用）
  * @param  无
  * @retval 无
  */
void Gesture_Process_Handler(void)
{
    int Speed = 80;
	int wait_second = 300;
    if(gesture_detected_flag)
    {
        gesture_detected_flag = 0;
        
        // 读取手势结果寄存器
        gesture->detect = GS_Read_nByte(PAJ_GET_INT_FLAG1,2,&gesture->data[0]);//读取手势状态
        
        // 解析手势类型
        if(!gesture->detect)
        {
					gesture->type =(u16)gesture->data[1]<<8 | gesture->data[0];
					if(gesture->type)
            switch(gesture->type)
            {
                case GES_RIGHT:
										OLED_Clear();
										OLED_ShowString(2,1,"RIGHT",OLED_8X16);
										Right(Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_LEFT:
										OLED_Clear();
										OLED_ShowString(2,1,"LEFT",OLED_8X16);
										Left(Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_UP:
										OLED_Clear();
										OLED_ShowString(2,1,"UP",OLED_8X16);
										Move(Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_DOWN:
										OLED_Clear();
										OLED_ShowString(2,1,"DOWN",OLED_8X16);
										Move(-Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_FORWARD:
										OLED_Clear();
										OLED_ShowString(2,1,"FORWARD",OLED_8X16);
										Move(0);
										break;
                case GES_BACKWARD:
										OLED_Clear();
										OLED_ShowString(2,1,"BACKWARD",OLED_8X16);
										Move(0);
										break;
                case GES_CLOCKWISE:
										OLED_Clear();
										OLED_ShowString(2,1,"CLOCKWISE",OLED_8X16);
										rotate(Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_ANTI_CLOCKWISE:
										OLED_Clear();
										OLED_ShowString(2,1,"ANTI_CLOCKWISE",OLED_8X16);
										rotate(-Speed);
										Delay_ms(wait_second);
										Move(0);
										break;
                case GES_WAVE:
										OLED_Clear();
										OLED_ShowString(2,1,"WAVE",OLED_8X16);
										Move(0);
										break;
                default:
                    break;
            }
        }
    }
}

/**
  * @brief  EXTI9_5中断服务函数（处理PB5中断）
  * @param  无
  * @retval 无
  */
void EXTI9_5_IRQHandler(void)
{
    EXTI_Gesture_IRQHandler();
}
