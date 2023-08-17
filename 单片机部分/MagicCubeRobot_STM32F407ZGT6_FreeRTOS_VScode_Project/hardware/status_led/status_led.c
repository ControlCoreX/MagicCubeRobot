/*!
 * @file: status_led.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:36:08
 * @brief: 	这个文件就用于状态指示灯对应的引脚的初始化.
 * 			这个里面我都使用了宏, 后面改用其它引脚控制的灯也比较方便, 只需要在status_led.h中改就可以.
 */

#include "status_led.h"


void status_led_init(void)
{
	RCC_AHB1PeriphClockCmd(STATUS_LED_GPIO_PORT_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = STATUS_LED_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(STATUS_LED_GPIO_PORT, &GPIO_InitStructure);
    
    STATUS_LED_OFF;
}

