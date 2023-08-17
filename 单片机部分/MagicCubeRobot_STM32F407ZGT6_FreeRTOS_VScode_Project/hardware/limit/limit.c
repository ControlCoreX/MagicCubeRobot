/*!
 * @file: limit.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日20:56:32
 * @brief:  这个文件用来初始化用于复位功能的4个限位开关对应的引脚.
 *          1号电机对应导轨用的PD0, 2号电机对应导轨用的PD1, 
 *          4号电机对应导轨用的PD3, 9号电机对应导轨用的PD3.
 * @note:   1. 电路板上我一共预留了6个装限位开关的引脚, PD0~PD5, 但是这里只用了其中4个.
 *          2. 限位开关我用的是低电平触发, 所以这里配置为了上拉模式.
 */

#include "limit.h"
#include "stm32f4xx.h"


//! 4个限位开关, 使用PD0, PD1, PD2, PD3.
void limit_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}
