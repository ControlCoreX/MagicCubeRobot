/*!
 * @file: data_proc_task.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日18:29:51
 * @brief: 没啥好说, 任务优先级跟任务栈大小注意一下就好了.
 */

#ifndef _USER_DATA_PROC_TASK_H_
#define _USER_DATA_PROC_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


//! data proc task
extern SemaphoreHandle_t data_proc_task_binsem;
extern void data_proc_task_routine(void* param);
#define DATA_PROC_TASK_STACK_SIZE  512U
#define DATA_PROC_TASK_PRIO        3U
extern TaskHandle_t data_proc_task_handle;

#endif //! _USER_DATA_PROC_TASK_H_

