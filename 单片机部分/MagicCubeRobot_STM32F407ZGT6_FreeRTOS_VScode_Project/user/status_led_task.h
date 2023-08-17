/*!
 * @file: status_led_task.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:45:42
 * @brief: 没啥好说的. 注意它的任务优先级不用那么高, 因为它就是一个系统运行状态指示灯, 不是什么紧急任务, 优先级为1就好了.
 */

#ifndef _USER_STATUS_LED_TASK_H_
#define _USER_STATUS_LED_TASK_H_

#include "FreeRTOS.h"
#include "task.h"

//! status led task
extern void status_led_task_routine(void* param);
#define STATUS_LED_TASK_STACK_SIZE  64U
#define STATUS_LED_TASK_PRIO        1U
extern TaskHandle_t status_led_task_handle;

#endif //!< _USER_STATUS_LED_TASK_H_
