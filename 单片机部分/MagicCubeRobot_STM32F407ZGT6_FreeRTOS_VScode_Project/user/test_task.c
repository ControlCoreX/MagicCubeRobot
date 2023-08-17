/*!
 * @file: test_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:49:28
 * @brief: 这个就是测试任务, 如果需要的话你自己写点东西就好了, 不需要测试什么的话, 它运行一次就直接把自己删掉了.
 */

#include "log.h"
#include "test_task.h"

TaskHandle_t test_task_handle = NULL;

void test_task_routine(void* param)
{
    (void)param;

    // step_motor_run(0, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(1, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(2, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(3, 0, 400);
    // vTaskDelay(3000);    
    // step_motor_run(5, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(6, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(7, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(8, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(9, 0, 400);
    // vTaskDelay(3000);
    // step_motor_run(10, 0, 400);
    // vTaskDelay(3000);

    vTaskDelete(NULL);
    // for( ; ; )
    // {
        // TIM_SetCompare1(TIM4, 5);
        // vTaskDelay(3000);
        // TIM_SetCompare1(TIM4, 15);
        // vTaskDelay(3000);
        // TIM_SetCompare1(TIM4, 25);
        
        // vTaskDelay(2000);
        // step_motor_run(0, 1, 1200);
        // vTaskDelay(2000);

    // }
}

