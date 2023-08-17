/*!
 * @file: user_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:42:56
 * @brief:  这个文件实现了start_task线程的线程函数.
 *          这个线程就执行一次就把自己删掉了, 它的任务就是创建系统中所有的线程和2个信号量.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log.h"
#include "status_led_task.h"
#include "test_task.h"
#include "cmd_proc_task.h"
#include "data_proc_task.h"
#include "limit_task.h"
#include "tucomm.h"


void start_task_routine(void* param)
{
    //! create cmd_proc_task_binsem
    cmd_proc_task_binsem = xSemaphoreCreateBinary();
    if(!cmd_proc_task_binsem)
        FreeRTOS_logerr("cmd_proc_task_binsem create failed!");


    //! create data_proc_task_binsem
    data_proc_task_binsem = xSemaphoreCreateBinary();
    if(!data_proc_task_binsem)
        FreeRTOS_logerr("data_proc_task_binsem create failed!");


    //! create status_led_task
    xTaskCreate((TaskFunction_t) status_led_task_routine,
                (const char *  ) "status_led_task",
                (uint16_t      ) STATUS_LED_TASK_STACK_SIZE,
                (void *        ) NULL,
                (UBaseType_t   ) STATUS_LED_TASK_PRIO,
                (TaskHandle_t *) &status_led_task_handle);
    
    if(!status_led_task_handle)
        FreeRTOS_logerr("status_led_task create failed!");


    //! create test_task
    xTaskCreate((TaskFunction_t) test_task_routine,
                (const char *  ) "test_task",
                (uint16_t      ) TEST_TASK_STACK_SIZE,
                (void *        ) NULL,
                (UBaseType_t   ) TEST_TASK_PRIO,
                (TaskHandle_t *) &test_task_handle);
    
    if(!test_task_handle)
        FreeRTOS_logerr("test_task create failed!");


    //! create cmd_proc_task
    xTaskCreate((TaskFunction_t) cmd_proc_task_routine,
                (const char *  ) "cmd_proc_task",
                (uint16_t      ) CMD_PROC_TASK_STACK_SIZE,
                (void *        ) NULL,
                (UBaseType_t   ) CMD_PROC_TASK_PRIO,
                (TaskHandle_t *) &cmd_proc_task_handle);
    
    if(!cmd_proc_task_handle)
        FreeRTOS_logerr("cmd_proc_task create failed!");


    //! create data_proc_task
    xTaskCreate((TaskFunction_t) data_proc_task_routine,
                (const char *  ) "data_proc_task",
                (uint16_t      ) DATA_PROC_TASK_STACK_SIZE,
                (void *        ) NULL,
                (UBaseType_t   ) DATA_PROC_TASK_PRIO,
                (TaskHandle_t *) &data_proc_task_handle);
    
    if(!data_proc_task_handle)
        FreeRTOS_logerr("data_proc_task create failed!");


    //! create limit_task
    xTaskCreate((TaskFunction_t) limit_task_routine,
                (const char *  ) "limit_task",
                (uint16_t      ) LIMIT_TASK_STACK_SIZE,
                (void *        ) NULL,
                (UBaseType_t   ) LIMIT_TASK_PRIO,
                (TaskHandle_t *) &limit_task_handle);
    
    if(!limit_task_handle)
        FreeRTOS_logerr("limit_task create failed!");


    reset(1U, 1U, 1U);//!< 整个系统复位.


    vTaskDelete(NULL);
}

