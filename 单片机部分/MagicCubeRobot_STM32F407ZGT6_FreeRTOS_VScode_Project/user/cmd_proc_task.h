/*!
 * @file: cmd_proc_task.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日18:27:59
 * @brief:  这个文件就没啥说的了, 任务优先级跟任务栈的大小注意一下就好了.
 */

#ifndef _USER_CMD_PROC_TASK_H_
#define _USER_CMD_PROC_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//! cmd proc task
extern SemaphoreHandle_t cmd_proc_task_binsem;
extern void cmd_proc_task_routine(void* param);
#define CMD_PROC_TASK_STACK_SIZE  256U
#define CMD_PROC_TASK_PRIO        4U//!< 优先级要比data_proc_task要高, 因为有的命令需要打断解魔方的过程.
extern TaskHandle_t cmd_proc_task_handle;


#endif //! _USER_CMD_PROC_TASK_H_
