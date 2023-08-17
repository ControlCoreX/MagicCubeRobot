#include "stm32f4xx.h"
#include "delay.h"
#include "FreeRTOS.h"	  
#include "task.h"


extern void xPortSysTickHandler(void);


void delay_init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	while(1 == SysTick_Config(168000));
}


void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0U;
	uint32_t reload = SysTick->LOAD;    	 
	ticks = nus * 168U;
	told = SysTick->VAL;
    
	while(1)
	{
		tnow = SysTick->VAL;	
		if(tnow != told)
		{	    
			if(tnow < told)
				tcnt += told - tnow;
			else 
				tcnt += reload - tnow + told;	    
			told = tnow;
			if(tcnt >= ticks) break;
		}  
	}									    
}


void delay_ms(uint32_t nms)
{	
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{		
		if(nms >= 1)
		{
   			vTaskDelay(nms / 1);
		}
		nms %= 1;  
	}
	delay_us((uint32_t)(nms * 1000));
}


void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();	
    }
}

