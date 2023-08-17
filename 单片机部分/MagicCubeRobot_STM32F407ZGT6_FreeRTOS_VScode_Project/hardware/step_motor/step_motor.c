/*!
 * @file: step_motor.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月16日21:54:15
 * @brief:  这个文件用于步进电机的驱动.
 *          一共11个步进电机, 脉冲输出引脚使用PF0~PF10, 方向控制引脚使用PG0~PG10.
 *          使用TIM3, 定时器更新中断周期为1.25ms, 即1s中断800次, 即脉冲频率为800Hz. 如果驱动器调节细分至400个脉冲每圈, 则步进电机转速2r/s.
 *          控制步进电机我使用了一个结构体step_motor, 方便一些. 一共控制11个步进电机, 所以定义了一个结构体数组step_motors.
 * 
 * @note:   由于一共使用了11个步进电机, 数量较多, 如果每个都使用定时器来驱动的话, 有点浪费资源, 所以我使用了模拟脉冲(即手动拉高拉低引脚)的方法.
 *          那么也就只需要一个定时器, 11个用于产生脉冲的普通GPIO口即可.
 *          为了尽量减少中断的次数及中断频率, 减少CPU资源无故浪费, 我采用了下面的方法产生脉冲:
 *              进一次中断就产生一个脉冲, 即在中断服务函数中, 先把引脚拉高, 然后卡死延时5us, 再把引脚拉低.
 *          这里的5us是经过示波器和信号发生器测试的, 高电平持续4us以上步进电机驱动器的高速光耦都是能反应得过来的.
 *          另外,
 *          由于一共有11个步进电机, 如果挨个调用库函数拉高拉低引脚, 未免有点太慢太傻了. 所以这里我在处理时使用了编程中的bitmap方式, 即一个数据一共有16bit,
 *          我使用其中的低11bit, 每一位表示一个步进电机. 例如: 第2bit为1, 就表示第2个步进电机需要转动, 即第2个引脚需要产生脉冲.
 *          拉高拉低引脚时使用的也是直接操作寄存器的方法, 速度更快.
 * 
 *          !!>> 控制步进电机这部分我进行了精心设计, 想要转动一个步进电机, 只需要调用void step_motor_run(uint8_t index, uint8_t dir, uint32_t steps);函数即可,
 *          而且也仅仅需要调用这一个函数, 通过参数指定步进电机索引, 转动方向, 转动步数.
 *          而且11个步进电机之间互补相斥, 支持同一时间任意个数的步进电机旋转.
 */

#include "step_motor.h"
#include "bitband.h"
#include "delay.h"
#include "limit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limit_task.h"


#define NUM_OF_STEPPER_MOTORS   11U //!< 步进电机的数量.
step_motor step_motors[NUM_OF_STEPPER_MOTORS];//!< 定义控制11个步进电机的结构体数组.

volatile uint8_t stepper_rotating = 0U;//!< 有电机在转的标志. 有电机在转为1, 没有电机在转为0. 这个变量主要用于各步进电机之间的同步.


//! 11个步进电机方向控制引脚初始化. 使用PG0~PG10.
static void step_motor_dir_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
                                  | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}


//! 11个步进电机脉冲输出引脚初始化. 使用PF0~PF10.
static void step_motor_pulse_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
                                  | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    for(uint8_t i = 0U; i < 11; i++)//!< 保证初始时刻, 引脚全部为低.
    {
        PFout(i) = 0;
    }
}
    

//! 定时器模式初始化. 仅用于产生更新中断.
static void step_motor_timer3_mode_config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 840 - 1;//!< 0.01ms加1.
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 125 - 1;//!< 周期1.25ms.
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//!< 中断优先级为6, 较高, 可以在中断中使用FreeRTOS的接口.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_SetCounter(TIM3, 0U);//!< 初始的时候, 先把计数器清0.
}


//! 步进电机结构体中元素的初始化.
static void step_motor_struct_init(void)
{
    for(uint8_t i = 0U; i < 11U; i++)
    {
        step_motors[i].steps = 0U;
        step_motors[i].dir = 0U;
    }
}


//! 总的初始化.
void step_motor_init(void)
{
    step_motor_dir_gpio_config();
    step_motor_pulse_gpio_config();
    step_motor_timer3_mode_config();
    step_motor_struct_init();
}


//! 这个里面就主要是给step和dir赋值.
void step_motor_run(uint8_t index, uint8_t dir, uint32_t steps)
{
    //! 进来先判断一下要转动的是不是导轨, 如果是且导轨已到达限位, 如果还想朝限位走, 则不让走. 如果不会发生危险, 则正常走就行了, 一个if也不会进.
    //! 之所以这样做是为了解决, 当限位开关已经触发的时候, 由于limit_task的检查频率(并非始终在检查, 而是有时间间隔的), 当再次让导轨撞限位时, 还是会撞一点, 的问题.

    if(LIMIT_STATUS(0) == TRIGGER && index == 1U && dir == 1U)//!< 这次调用使导轨0危险, 则本次调用无效.
        return ;

    if(LIMIT_STATUS(1) == TRIGGER && index == 2U && dir == 0U)//!< 或者 这次调用使导轨1危险, 则本次调用无效.
        return ;

    if(LIMIT_STATUS(2) == TRIGGER && index == 4U && dir == 1U)//!< 或者 这次调用使导轨2危险, 则本次调用无效.
        return ;

    if(LIMIT_STATUS(3) == TRIGGER && index == 9U && dir == 1U)//!< 或者 这次调用使导轨3危险, 则本次调用无效.
        return ;


    if((LIMIT_STATUS(0) == TRIGGER && index == 1U && dir == 0) \
        || (LIMIT_STATUS(1) == TRIGGER && index == 2U && dir == 1U) \
        || (LIMIT_STATUS(2) == TRIGGER && index == 4U && dir == 0) \
        || (LIMIT_STATUS(3) == TRIGGER && index == 9U && dir == 0))//!< 如果任何一个要从限位点出发, 那就挂起, 而从其他位置前进不需要挂起.
    {
        vTaskSuspend(limit_task_handle);
    }
    

    if((index == 1U && dir == 1U) \
        || (index == 2U && dir == 0) \
        || (index == 4U && dir == 1U) \
        || (index == 9U && dir == 1U))//!< 如果有任何一个要回去, 就要解挂.
    {
        vTaskResume(limit_task_handle);
    }
    
    //! 其余的就是普通情况了, 常规处理就好.
    step_motors[index].dir = dir;
    step_motors[index].steps = steps;
    
    if(dir == 0U)
        PGout(index) = 0;
    else
        PGout(index) = 1;

    TIM_Cmd(TIM3, ENABLE);
}


//! 这个里面就将那些脉冲不为0(即用户想让它转的那些), 就直接递减.
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        uint8_t index = 0U;
        uint8_t cnt = 0U;//!< 记录不需要旋转的步进电机的个数.
        int16_t pins_bitmap = 0U;//!< 步进电机位图, 使用低11位, 对应位置1, 表示该电机需要转动; 对应位清0表示该电机不需要转动.
    
        for(index = 0U; index < 11U; index++)//!< 挨个看每个电机, 看是否需要转动, 如果需要就指定位置1, 如果不需要就指定位清0.
        {
            if(step_motors[index].steps > 0U)
            {
                pins_bitmap |= (1 << index);//!< index需要转动, 就指定位置1, 使用 按位或运算.
                step_motors[index].steps--;
            }
            else
            {
                pins_bitmap &= (~(1 << index));//!< index不需要转动, 就指定位清0, 使用 按位与运算.
                cnt++;
            }
        }

        // printf("pins_bitmap = %d\r\n", pins_bitmap);

        GPIOF->BSRRL = pins_bitmap;//!< 把需要转动的脉冲引脚拉高.
        delay_us(5U);
        GPIOF->BSRRH = pins_bitmap;//!< 经过4us后就拉低. (通过示波器测试过, 4us的高电平就够了, 这里我留了5us, 无所谓了)

        if(cnt == 11U)//!< 如果没有需要转的电机, 那就关闭定时器.
        {
            TIM_Cmd(TIM3, DISABLE);
            TIM_SetCounter(TIM3, 0U);
            stepper_rotating = 0U;//!< 标记没有电机在旋转.
        }
        else
            stepper_rotating = 1U;//!< 标记有电机在旋转.
    }
}