/*!
 * @file: status_led.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:44:07
 * @brief:  这个文件也没啥好说的吧.
 *          就以后你要改引脚的话就改16行到26行中间的那几个宏就可以了.
 *          宏STATUS_LED_ENABLE_LEVEL表示, 使灯亮的电平. 高电平亮就设为1, 低电平亮就设为0.
 */

#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

#include "stm32f4xx.h"
#include "bitband.h"

/*param config macro===========================================================*/

#define STATUS_LED 				    PGout(3)

#define STATUS_LED_GPIO_PORT_CLK    RCC_AHB1Periph_GPIOG
#define STATUS_LED_GPIO_PORT		GPIOG
#define STATUS_LED_GPIO_PIN		    GPIO_Pin_3

#define STATUS_LED_ENABLE_LEVEL     0

/*=============================================================================*/


#if STATUS_LED_ENABLE_LEVEL == 0
# define STATUS_LED_ON              GPIO_ResetBits(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN)
# define STATUS_LED_OFF             GPIO_SetBits(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN)
#else
# define STATUS_LED_ON              GPIO_SetBits(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN)
# define STATUS_LED_OFF             GPIO_ResetBits(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN)
#endif


extern void status_led_init(void);


#endif //!< _STATUS_LED_H_

