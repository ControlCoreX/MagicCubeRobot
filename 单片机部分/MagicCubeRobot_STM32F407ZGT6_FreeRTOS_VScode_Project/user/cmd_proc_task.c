/*!
 * @file: com_proc_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日18:22:15
 * @brief:  1. 这个文件里面就实现了一下任务处理任务的任务函数.
 *          2. 当接收到命令时, 这个任务就会被唤醒, 把命令的参数解析出来, 然后通过函数指针数组调用指定命令.
 */

#include "tucomm.h"
#include "log.h"
#include "step_motor.h"
#include "cmd_proc_task.h"


SemaphoreHandle_t cmd_proc_task_binsem = NULL;
TaskHandle_t cmd_proc_task_handle = NULL;

void cmd_proc_task_routine(void* param)
{
    extern uint16_t grecv_len;
    extern uint8_t grecv_data[];
    extern cmd_func_ptr cmd_funcs[];

    (void)param;
    uint32_t param1 = 0U, param2 = 0U, param3 = 0U;

    for( ; ; )
    {
        xSemaphoreTake(cmd_proc_task_binsem, portMAX_DELAY);

        //! 先把3个参数解析出来.
        param1 = uint8_to_uint32(&grecv_data[0]);
        param2 = uint8_to_uint32(&grecv_data[4]);
        param3 = uint8_to_uint32(&grecv_data[8]);

        // printf("param1 = %d\r\nparam2 = %d\r\nparam3 = %d\r\n", param1, param2, param3);
        
        cmd_funcs[grecv_len](param1, param2, param3);
    }
}

