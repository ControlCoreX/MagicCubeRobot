/*!
 * @file: log.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日18:13:25
 * @brief:  1. 这个文件就定义了两个日志宏, 一个用于打印错误信息, 一个用于打印提示信息.
 * @note:   1. 看到有两个宏, NO_LOGERR和NO_LOGINFO, 这两个是日志关闭宏, 即如果定义了NO_LOGERR这个宏, 则打印错误信息的功能就被关闭了. 打印提示信息的宏也是如此.
 *          2. 注意看这里使用了GNU扩展语法: 语句表达式({}). 需要编译器开启GNU扩展语法.
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "stdio.h"

//#define NO_LOGERR
//#define NO_LOGINFO

#ifndef NO_LOGERR
# define FreeRTOS_logerr(err)   ({printf("error: %s\r\n", err);})
#else
# define FreeRTOS_logerr(err)  
#endif

#ifndef NO_LOGINFO
# define FreeRTOS_loginfo(info) ({printf("info: %s\r\n", info);})
#else
# define FreeRTOS_log_info(info)  
#endif

#endif //!< _LOG_H_
