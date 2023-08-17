/*!
 * @file: step_motor.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:48:54
 * @brief:  这个文件没啥好说的吧.
 *          就19~23行定义了几个关于几个导轨的宏, 当出现导轨运动终点为止不准确的时候, 可以调下面几个宏的值.
 *          其中宏ENOUGH_STEPS表示的是足够的步数, 目的就是让滑轨步进电机一直转, 直到它触发限位开关为止, 这个步数20000是跑不完的.
 *          对, 如果要初始化就调用step_motor_init()函数.
 *          要转动步进电机, 就调用step_motor_run()函数.
 */

#ifndef _STEP_MOTOR_H_
#define _STEP_MOTOR_H_

#include "stm32f4xx.h"


//! 那些导轨需要从复位位置到达需要的位置, 所需要的步数.
#define LONG_GUIDEWAY_STEPS         8110U
#define DOWN_GUIDEWAY_STEPS         8850U + 1900U + 450U
#define HORIZONTAL_GUIDEWAY_STEPS   220U
#define ENOUGH_STEPS                20000U


typedef struct step_motor_struct
{
    uint8_t dir;//!< 方向: 0顺时针, 1逆时针.
    uint32_t steps;//!< 剩余的脉冲数.
}step_motor;

extern volatile uint8_t stepper_rotating;//!< 是否有电机在转的标志.

extern void step_motor_init(void);
extern void step_motor_run(uint8_t index, uint8_t dir, uint32_t steps);

#endif //!< _STEP_MOTOR_H_
