/*!
 * @file: test_task.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:47:46
 * @brief: 没啥好说的. 优先级也不用太高. 还有要根据你想要测试的功能来自己调节合适的任务栈大小, 如果测试任务比较复杂, 可能128个word不够.
 */

#ifndef _USER_TEST_TASK_H_
#define _USER_TEST_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//! test task
extern void test_task_routine(void* param);
#define TEST_TASK_STACK_SIZE  128U
#define TEST_TASK_PRIO        2U
extern TaskHandle_t test_task_handle;


#endif //!< _USER_TEST_TASK_H_