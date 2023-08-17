/*!
 * @file: tucomm.h
 * @author: 屠文博-201-201404010126
 * @date: 2023年5月17日17:46:51
 * @brief:  这个文件没啥好说的, 看下面的注释就好了.
 */

#ifndef _TUCOMM_TUCOMM_H_
#define _TUCOMM_TUCOMM_H_

#include <stdint.h>


typedef void (*cmd_func_ptr)(uint32_t param1, uint32_t param2, uint32_t param3);//!< 命令函数的函数指针类型.

extern uint32_t uint8_to_uint32(uint8_t* data);//!< 4个字节的数据拼接成一个uint32_t类型的数据.

extern void reset(uint32_t param1, uint32_t param2, uint32_t param3);//!< 系统复位函数, 命令里面就这一个需要外部调用, 其他的就别调用了.


#endif //!< _TUCOMM_TUCOMM_H_
