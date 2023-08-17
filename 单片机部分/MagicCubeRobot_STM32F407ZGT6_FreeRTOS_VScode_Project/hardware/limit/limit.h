/*!
 * @file: limit.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:02:30
 * @brief:  这个文件没啥好说的, 就定义了一个判断引脚(所连接的限位开关)是否触发的宏.
 *          使用示例: 
 *          if(LIMIT_STATUS(0) == TRIGGER)
 *          {
 *              //! 如果PD0对应的限位开关触发了, do something.
 *          }
 */

#ifndef _LIMIT_H_
#define _LIMIT_H_

#include "bitband.h"

#define TRIGGER     0 //!< 低电平触发.
#define NON_TRIGGER 1

#define LIMIT_STATUS(n) PDin(n)

extern void limit_gpio_config(void);


#endif //!< _LIMIT_H_
