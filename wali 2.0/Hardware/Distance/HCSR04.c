#include "stm32f10x.h"
#include "Delay.h"

#define TRIG_PIN GPIO_Pin_7
#define ECHO_PIN GPIO_Pin_1

// 全局变量
volatile uint32_t echo_start_time = 0;
volatile uint32_t echo_end_time = 0;
volatile uint8_t echo_received = 0;
volatile uint8_t measurement_done = 0;

/**
  * @brief  超声波模块初始化
  * @param  无
  * @retval 无
  */
void HC_SR04_Init(void)
{
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 初始化TRIG引脚（PA7）为推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化ECHO引脚（PB1）为浮空输入
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置外部中断
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 将PB1连接到EXTI1
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    
    // 配置EXTI1
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 上升沿和下降沿都触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 初始化定时器用于精确计时（TIM3，1MHz频率，1us计数）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF; // 最大计数值
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;   // 72MHz/72 = 1MHz，1us计数
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM3, ENABLE);
}

/**
  * @brief  发送触发脉冲
  * @param  无
  * @retval 无
  */
void HC_SR04_Trigger(void)
{
    // 确保ECHO为低电平状态
    while(GPIO_ReadInputDataBit(GPIOB, ECHO_PIN) == 1);
    
    // 发送10us以上的高电平脉冲
    GPIO_SetBits(GPIOA, TRIG_PIN);
    Delay_us(15);  // 15us脉冲
    GPIO_ResetBits(GPIOA, TRIG_PIN);
    
    // 重置测量状态
    measurement_done = 0;
    echo_received = 0;
}

/**
  * @brief  获取测量结果（毫米）
  * @param  无
  * @retval 距离值（毫米），0表示超时或测量失败
  */
int16_t HC_SR04_GetDistanceMM(void)
{
    uint32_t pulse_width = 0;
    uint32_t distance = 0;
    
    // 启动测量
    HC_SR04_Trigger();
    
    // 等待测量完成（最多等待60ms）
    uint32_t timeout = 60000; // 60ms超时
    while(!measurement_done && timeout--);
    
    if(measurement_done && echo_end_time > echo_start_time)
    {
        // 计算脉冲宽度（微秒）
        pulse_width = echo_end_time - echo_start_time;
        
        // 根据公式计算距离：距离 = (时间 * 声速) / 2
        // 声速按340m/s计算，转换为mm/us：0.34mm/us
        distance = (uint32_t)(pulse_width * 0.34f)/2;
        
        // 或者使用推荐的简化公式：距离 = 脉冲宽度 / 58（毫米）
        // distance = pulse_width / 58;
    }
    
    return distance;
}

/**
  * @brief  获取测量结果（米）
  * @param  无
  * @retval 距离值（米）
  */
float HC_SR04_GetDistanceM(void)
{
    uint32_t distance_mm = HC_SR04_GetDistanceMM();
    return (float)distance_mm / 1000.0f;
}

/**
  * @brief  EXTI1中断服务函数
  * @param  无
  * @retval 无
  */
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        if(GPIO_ReadInputDataBit(GPIOB, ECHO_PIN) == 1)
        {
            // 上升沿：ECHO信号开始，记录开始时间
            echo_start_time = TIM_GetCounter(TIM3);
            echo_received = 1;
        }
        else
        {
            // 下降沿：ECHO信号结束，记录结束时间
            if(echo_received)
            {
                echo_end_time = TIM_GetCounter(TIM3);
                measurement_done = 1;
            }
        }
        
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

/**
  * @brief  主函数示例
  * @param  无
  * @retval 无
  */
