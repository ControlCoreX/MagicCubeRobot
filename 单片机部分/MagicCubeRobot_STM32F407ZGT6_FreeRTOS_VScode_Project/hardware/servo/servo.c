/*!
 * @file: servo.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:15:04
 * @brief:  该文件用于驱动舵机的定时器TIM4的初始化, 使用TIM4的CH1, 对应引脚PD12.
 *          这里主要说一下舵机的使用.
 *          舵机要转动需要提供周期为20ms, 高电平持续时间为0.5ms~2.5ms的PWM脉冲, 则0.5ms~2.5ms对应转角为0°~180°.
 *          TIM4频率为84MHz, 我采用了840分频, 所以0.01ms计数器加1, 所以想要20ms的周期, 重装载寄存器值应为2000, 
 *          则产生0.5ms~2.5ms的高电平, 对应的比较寄存器的值应在50~250变化.
 *          之前我设定的重装载值为200, 比较寄存器的值为5~25, 结果发现这样舵机一步就是9°, 太大了, 所以就又扩大了10倍, 使每步变成了0.9°.
 * @note:   我使用信号发生器测试过了, 其实稍微小于0.5ms的脉冲也是可以的. (我想应该是这种舵机精度很低, 结构比较简单, 误差比较大的原因)
 *          在该系统中, 舵机仅需要3个位置, 从下到上记为POS1, POS2, POS3. 我在servol.h中定义了3个宏表示这几个位置对应的比较寄存器的值, 
 *          这3个值分别为47, 153, 236, 是经过测试的. 如果后期因为某些原因对不齐时, 可以自己在servo.h中调整这几个值.
 */

#include "servo.h"
#include "stm32f4xx.h"


//! TIM4, CH1, PD12
static void timer4_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
    
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}
    
 
static void timer4_mode_config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 840 - 1;//!< 0.01ms加1.
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 2000 - 1;//!< 计数周期20ms. (则高电平持续时间对应的pulse为50~250)
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = POS_1_PULSE;//!< 初始时刻在下面.
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_Cmd(TIM4, ENABLE);
    TIM_CtrlPWMOutputs(TIM4, ENABLE);
}


void servo_init(void)
{
    timer4_gpio_config();
    timer4_mode_config();
}
