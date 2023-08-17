/*!
 * @file: limit_task.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:24:33
 * @brief: 没啥好说的吧. 注意任务优先级高一点就好了.
 */

#ifndef _USER_LIMIT_TASK_H_
#define _USER_LIMIT_TASK_H_

#include "FreeRTOS.h"
#include "task.h"

extern void limit_task_routine(void* param);
#define LIMIT_TASK_STACK_SIZE  128U
#define LIMIT_TASK_PRIO        10U
extern TaskHandle_t limit_task_handle;


#endif //! _USER_LIMIT_TASK_H_
