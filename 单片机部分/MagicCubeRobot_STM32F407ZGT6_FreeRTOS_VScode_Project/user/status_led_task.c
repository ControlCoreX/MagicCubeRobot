/*!
 * @file: status_led_task.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日20:41:54
 * @brief: 没有什么好说的.
 */

#include "status_led.h"
#include "bitband.h"
#include "status_led_task.h"

TaskHandle_t status_led_task_handle = NULL;

void status_led_task_routine(void* param)
{
    (void)param;

    for( ; ; )
    {
        STATUS_LED_ON;  vTaskDelay(160);
        STATUS_LED_OFF; vTaskDelay(100);
        STATUS_LED_ON;  vTaskDelay(160);
        STATUS_LED_OFF; vTaskDelay(3000);
    }
}
