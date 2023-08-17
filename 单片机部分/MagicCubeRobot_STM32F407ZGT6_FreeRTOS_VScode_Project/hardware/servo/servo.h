/*!
 * @file: servo.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:30:53
 * @brief:  这个文件没啥好说的, 就定义了3个确定舵机位置的宏, 当舵机对不齐时, 可以自己调整, 这几个值.
 *          初始化舵机, 只需要调用servo_init(void);
 */

#ifndef _SERVO_H_
#define _SERVO_H_

#define POS_1_PULSE 47
#define POS_2_PULSE 153
#define POS_3_PULSE 236

extern void servo_init(void);

#endif  //<! _SERVO_H_