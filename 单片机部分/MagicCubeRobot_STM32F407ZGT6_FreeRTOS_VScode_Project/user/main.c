/*!
 * @file: main.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:51:07
 * @brief: 这也没什么好说的, 就是通过god_init()私有函数初始化全部硬件, 然后创建start_task, 在start_task里面创建所有任务.
 */

#include "delay.h"
#include "usart1.h"
#include "FreeRTOS.h"
#include "task.h"
#include "status_led.h"
#include "step_motor.h"
#include "servo.h"
#include "limit.h"
#include "usart2.h"
#include "tucomm.h"



static void god_init(void)
{
    delay_init();
    usart1_init(115200U);
    status_led_init();
    step_motor_init();
    servo_init();
    limit_gpio_config();
    usart2_dma_init();


}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    god_init();

    extern void start_task_routine(void* param);
	static TaskHandle_t start_task_handle = NULL;

	if(pdPASS == xTaskCreate(start_task_routine, "start_task", 512U, NULL, 1U, &start_task_handle))
		vTaskStartScheduler();
	else
		return -1;
    
	for( ; ; );
    
    return 0;
}

