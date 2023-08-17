/*!
 * @file: limit_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:19:59
 * @brief:  实现limit_task线程的线程函数.
 *          就是在线程里一直检测限位开关的状态, 如果触发了限位开关, 就将指定的电机停掉.
 * @note:   1. 限位开关的检查频率根据导轨的速度来调节. 这里我设置为了10ms检测一次, 效果还是挺好的. 不能过快, 过快有点浪费资源. 不能过慢, 过慢失去了限位的意义;
 *          2. 这个任务的优先级我设置的是10, 比较高, 因为这属于比较紧急的任务.
 * 
 * @guideline:  限位开关的处理就应该是这样的! 正规的处理方案!
 *              一方面, 因为在实际工程中, 有些限位开关是关乎人命的, 所以要一直在检查限位开关有没有触发.
 *              另一方面, 这是多线程编程, 我之前写的那种在死循环里检查的, 不是很符合多线程的设计理念, 
 *              虽然也能用, 但是它是卡死的, 不太好, 就算那一部分坏了也不应该影响到其他部分的功能!
 */

#include "limit_task.h"
#include "limit.h"
#include "step_motor.h"

extern step_motor step_motors[];

TaskHandle_t limit_task_handle = NULL;

void limit_task_routine(void* param)
{
    (void)param;

    for( ; ; )
    {
        if(LIMIT_STATUS(0) == TRIGGER)
            step_motors[1].steps = 0U;//!< 1号电机停转.

        if(LIMIT_STATUS(1) == TRIGGER)
            step_motors[2].steps = 0U;

        if(LIMIT_STATUS(2) == TRIGGER)
            step_motors[4].steps = 0U;

        if(LIMIT_STATUS(3) == TRIGGER)
            step_motors[9].steps = 0U;

        vTaskDelay(10);//!< 控制限位开关的检查频率.
    }
}