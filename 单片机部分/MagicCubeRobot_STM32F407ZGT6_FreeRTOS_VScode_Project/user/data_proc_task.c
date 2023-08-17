/*!
 * @file: data_proc_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日18:33:36
 * @brief:  1. 这个文件就是实现了一下数据处理线程的线程函数.
 *          2. 流程: 
 *              1. 接收到数据(解魔方的步骤)时, 这个线程会被唤醒
 *              2. 把魔方送到位
 *              3. 然后解析步骤字符串
 *              4. 获得步进电机的步骤
 *              5. 把魔方送出来
 */

#include "log.h"
#include "step_motor.h"
#include "data_proc_task.h"


SemaphoreHandle_t data_proc_task_binsem = NULL;
TaskHandle_t data_proc_task_handle = NULL;


//! 负责6个面的步进电机的索引.
#define UP_STEPPER_INDEX        10U
#define FRONT_STEPPER_INDEX     6U
#define LEFT_STEPPER_INDEX      7U
#define BEHIND_STEPPER_INDEX    8U
#define RIGHT_STEPPER_INDEX     5U
#define DOWN_STEPPER_INDEX      3U


//! 步进电机结构体, 用于描述一个步骤, 然后把所有步骤存在一个结构体里, 这样遍历结构体控制步进电机转动就可以了.
typedef struct
{
    uint32_t index;//!< 该步骤中需要旋转的电机的索引.
    uint32_t dir;//!< 电机选装的方向. 0顺时针, 1逆时针.
    uint32_t steps;//!< 需要转动的步数.
}stepperstep_t;


//! 判断一个字符是哪一个字母, 判断之后返回对应位置电机的索引. (因为描述解法的字符串里都是用这些符号表示面的, 所以我要把这些符号对应到负责对应面的步进电机)
uint32_t which_letter(char letter)
{
    uint32_t stepper_index = 0U;

    switch(letter)
    {
        case 'U': stepper_index = UP_STEPPER_INDEX; break;
        case 'F': stepper_index = FRONT_STEPPER_INDEX; break;
        case 'L': stepper_index = LEFT_STEPPER_INDEX; break;
        case 'B': stepper_index = BEHIND_STEPPER_INDEX; break;
        case 'R': stepper_index = RIGHT_STEPPER_INDEX; break;
        case 'D': stepper_index = DOWN_STEPPER_INDEX; break;
        //!< 因为这个字符串是从上位机获得的, 所以不存在匹配不到的情况, 所以这里取消了default情况.
    }

    return stepper_index;
}


//! 参数1: slove字符串; 参数2: slove字符串的长度; 参数3: 存放步骤的结构体数组.
//! 返回值: 一共多少个步骤.
uint16_t get_stepperstep_form_slove(uint8_t* slove, uint16_t slove_len, stepperstep_t* steppersteps)
{
    uint16_t i = 0U;
    uint32_t step_index = 0U;

    //! 一次循环处理一个步骤, 即step_index递增1, 但是i并不是递增1.
    while(i < slove_len)
    {
        steppersteps[step_index].index = which_letter(*(slove + i));

        if(*(slove + i + 1U) == ' ' || (i + 1U) == slove_len)//!< 如果后面直接是空格, 或者到了末尾.
        {
            steppersteps[step_index].dir = 0U;//!< 顺时针转动.
            steppersteps[step_index].steps = 100U;//!< 400个脉冲转一圈, 给100个就是转90度.
            i += 2U;//!< 因为后面是空格, 所以就直接偏移2, 指向下一个字母.
        }
        else if(*(slove + i + 1U) == '2')//!< 如果后面是字符2.
        {
            steppersteps[step_index].dir = 0U;
            steppersteps[step_index].steps = 200U;//!< 400个脉冲转一圈, 给200个就是转180度.
            i += 3U;//!< 指向下一个字母.
        }
        else if(*(slove + i + 1U) == '\'')//!< 如果后面是字符单引号.
        {
            steppersteps[step_index].dir = 1U;//!< 逆时针旋转.
            steppersteps[step_index].steps = 100U;
            i += 3U;//!< 同样也是指向下一个字母.
        }

        step_index++;
    }

    return step_index;
}


void data_proc_task_routine(void* param)
{
    extern uint16_t grecv_len;
    extern uint8_t grecv_data[];

    static stepperstep_t steppersteps[30];//!< 因为它占据的内存太大到了, 定义成static类型的, 就不用放到栈上, 比较省空间.
    static uint16_t steppersteps_len = 0U;//!< 也就随着定义成static类型的吧.

    (void)param;

    for( ; ; )
    {
        xSemaphoreTake(data_proc_task_binsem, portMAX_DELAY);

        step_motor_run(1U, 0U, LONG_GUIDEWAY_STEPS);//!< 长导轨将魔方送入.
        vTaskDelay(1000);//!< CPU跑的太快, 要给一个中断的时间, CPU再去检查.
        while(stepper_rotating == 1U)//!< 如果有电机在旋转就卡在这.
            vTaskDelay(500);//!< 任何时候程序都不应该卡死, 防止低优先级任务饿死.

        step_motor_run(2U, 1U, DOWN_GUIDEWAY_STEPS);//!< 下面导轨往上顶到魔方.
        vTaskDelay(1000);
        while(stepper_rotating == 1U)
            vTaskDelay(500);

        vTaskDelay(500);
        step_motor_run(4U, 0U, HORIZONTAL_GUIDEWAY_STEPS);//!< 水平导轨夹紧魔方.
        vTaskDelay(1000);
        while(stepper_rotating == 1U)
            vTaskDelay(500);    

        steppersteps_len = get_stepperstep_form_slove(grecv_data, grecv_len, steppersteps);//!< 获取各个电机的步骤.

        for(uint8_t i = 0U; i < steppersteps_len; i++)//!< 遍历步骤结构体数组.
        {
            // printf("%d, %d, %d\r\n", steppersteps[i].index, steppersteps[i].dir, steppersteps[i].steps);//!< 将步骤打印出来.

            step_motor_run(steppersteps[i].index, steppersteps[i].dir, steppersteps[i].steps);
            vTaskDelay(500);
            while(stepper_rotating == 1U);//!< 如果有电机在旋转就卡在这.
        }

        step_motor_run(4U, 1U, ENOUGH_STEPS);//!< 松开魔方.
        vTaskDelay(500);

        step_motor_run(2U, 0U, ENOUGH_STEPS);//!< 下面导轨往下.
        vTaskDelay(1000);
        while(stepper_rotating == 1U)//!< 如果有电机在旋转就卡在这.
            vTaskDelay(500);

        step_motor_run(1U, 1U, ENOUGH_STEPS);//!< 长导轨把魔方送出来.
    }
}

