/*!
 * @file: tucomm.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日17:49:31
 * @brief:  1. 这个文件就建立了一个存储命令函数的函数指针数组, 把这个数组extern出去给data_proc_task任务, 它根据数据包去调用指定命令.
 *          2. 为了敲命令行的时候好敲, 所以命名尽量短一些.
 */

#include "tucomm.h"
#include "log.h"
#include "step_motor.h"
#include "limit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "servo.h"


/*!
 * @brief: 将4个uint8_t拼接为一个uint32_t的函数.
 * @param: data: 按字节存放的, 表示一个uint32_t类型数据的数组.
 * @retval: 转化后的uint32_t类型的数据.
 * @note: STM32为ARM的核, 其为little endian模式, 所以低字节在低地址处.
 */
uint32_t uint8_to_uint32(uint8_t* data)
{
    uint32_t res = *(data + 0) + ((*(data + 1)) << 8) + ((*(data + 2)) << 16) + ((*(data + 3)) << 24);
    return res;
}


//! ==============================================================================================================

#define NUM_OF_CMD  7U //!< 命令的个数.


//! 整个系统复位. 支持任何时间进行复位.
void reset(uint32_t param1, uint32_t param2, uint32_t param3)
{
    step_motor_run(4U, 1U, ENOUGH_STEPS);//!< 4号步进电机复位.
    step_motor_run(2U, 0U, ENOUGH_STEPS);//!< 2号步进电机复位.
    vTaskDelay(1000U);
    while(stepper_rotating == 1U)//!< 如果有电机在旋转就卡在这. 防止长导轨和竖导轨卡到一块.
    {
        vTaskDelay(500);
    }

    step_motor_run(1U, 1U, ENOUGH_STEPS);//!< 1号步进电机. 20000足够多, 就是要让它一直转, 直到碰到限位开关.
    // step_motor_run(9U, 1U, ENOUGH_STEPS);//!< 9号步进电机.

    TIM_SetCompare1(TIM4, POS_1_PULSE);//!< 舵机复位.

    // FreeRTOS_loginfo("system reset!\r\n");
}

//! 步进电机旋转, 需要指定编号, 方向, 步数. (最基础命令).
void step(uint32_t index, uint32_t dir, uint32_t steps)
{
    step_motor_run((uint8_t)index, (uint8_t)dir, steps);

    // FreeRTOS_loginfo("exec step\r\n");
}

//! 舵机转动, 指定比较寄存器的值cmp.
void servo(uint32_t cmp, uint32_t param2, uint32_t param3)
{
    TIM_SetCompare1(TIM4, cmp);

    // FreeRTOS_loginfo("servo rotate\r\n");
}

//! 魔方转动, 0顺时针, 1逆时针. 90°或180°. (从下往上看时的顺时针和逆时针).
//! 命名: magic cube rotate.
void mcrota(uint32_t dir, uint32_t angle, uint32_t param3)
{
    dir = 1U - dir;//!< 如果dir是0, 那经过这一步就变成1; 如果是1, 就变成了0. (因为那个地方用了一个齿轮, 所以要这样搞一下)

    if(angle == 90U)
        step_motor_run(0U, (uint8_t)dir, 377U);//!< 这里377是由步进电机脉冲数和大小齿轮的齿数比计算得到的.
    else if(angle == 180)
        step_motor_run(0U, (uint8_t)dir, 377U * 2U);

    // FreeRTOS_loginfo("magic cube rotate\r\n");
}

//! 长导轨, 将魔方送进去或出来. 1是进入, 0是出来.
//! 命名: magic cube in or out.
void mcio(uint32_t io, uint32_t param2, uint32_t param3)
{
    if(io == 1U)
        step_motor_run(1U, 0U, LONG_GUIDEWAY_STEPS);
    else if(io == 0U)
        step_motor_run(1U, 1U, ENOUGH_STEPS);

    // FreeRTOS_loginfo("long guideway in/out\r\n");
}

//! 下面导轨向上向下到位. 1是向上, 0是向下.
//! 命名: down guideway to up or down.
void d2ud(uint32_t ud, uint32_t param2, uint32_t param3)
{
    if(ud == 1U)
        step_motor_run(2U, 1U, DOWN_GUIDEWAY_STEPS);
    else if(ud == 0U)
        step_motor_run(2U, 0U, ENOUGH_STEPS);

    // FreeRTOS_loginfo("below guideway up/down\r\n");
}

//! 水平方向夹紧和松开魔方. 1是夹紧, 0是松开.
//! 命名: horizontal open or close.
void hooc(uint32_t oc, uint32_t param2, uint32_t param3)
{
    if(oc == 1U)
        step_motor_run(4U, 0U, HORIZONTAL_GUIDEWAY_STEPS);
    else if(oc == 0U)
        step_motor_run(4U, 1U, ENOUGH_STEPS);

    // FreeRTOS_loginfo("close/open magic cube\r\n");
}



//! 函数指针数组. 只需要把这个函数指针数组交给要执行命令的地方.
cmd_func_ptr cmd_funcs[NUM_OF_CMD] = 
{
    reset,
    step,
    servo,
    mcrota,
    mcio,
    d2ud,
    hooc
};

