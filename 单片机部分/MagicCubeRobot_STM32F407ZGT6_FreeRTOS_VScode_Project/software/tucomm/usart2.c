/*!
 * @file: usart2.c
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日16:02:57
 * @brief:  1. 使用资源USART2-引脚PA2, PA3-DMA1-数据流5-请求4.
 *          2. 串口波特率115200. 
 *          3. 这个文件实现了通过串口空闲中断来接收不定长数据的驱动, 使用DMA+串口空闲中断的方法优点, 首先可以接收不定长数据, 其次中断的次数少, 节省CPU资源.
 *          流程为: 从串口接收到数据包, 通过帧头判断是否为有效的数据包, 若无效则无视; 若有效则根据判断接收到的是命令还是数据, 决定唤醒命令处理任务还是数据处理任务.
 *                 然后将data域中的内容复制到全局数组grecv_data中. 命令处理任务或数据处理任务从里面去拿.
 *          4. 这个组件我只设计了接收的部分, 而没有设计单片机向树莓派发送的部分. 虽然也可以正常实现功能, 但也是一个缺憾了, 后来者可以补上!
 *          
 * @note:   1. 经过考虑和反复迭代, 发现传输命令时, 并不需要把对应的函数名发过来, 而只需要二者约定好, 然后传输过来一个索引就可以了, 这样也能实现执行对应函数的目的.
 *          所以在传输命令时, 就使用数据包中的len成员来存储索引.
 *          2. 传输命令时往往还需要参数, 所以这里也设计了可以传输参数, 规定参数个数最多为3个(基本足够用, 当不够用时, 修改代码也比较好修改). 所以数据包中的len表示
 *          命令对应函数的索引, data域存储的是带有的参数, 长度固定为4*3=12个字节. (即使不需要那么多参数也读12个字节, 这样好处理, 思路清晰)
 *          3. 由于命令带的参数是uint32_t类型的(因为数据可能是很大的, 单纯一个字节可能存不下), 数据是字节类型的, 二者不一样, 比较难以统一进行处理, 后来经过考虑, 
 *          我把命令的参数也当成字节类型进行传输, 即通信信道是字节流的. 那么在传输命令参数时, 命令处理任务自动去拼接就行了, 把4个字节拼接成一个uint32_t即可.
 *          4. 数据包中data域设置的很大, 那只是最大那么大, 在实际用的时候不填满, 也不把整个数组全发送出去, 而是有效数据有多少就发送多少, 效率较高.
 */

#include "usart2.h"
#include "stm32f4xx.h"
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"


#define NUM_OF_ARGS 3U //!< 最多支持的命令的参数的个数.

uint16_t grecv_num = 0U;//!< 从串口接收到的字节数.
uint8_t grecv_buf[USART2_DMA_RECV_BUF_SIZE];//!< 从串口接收到的数据全部都放到这里.

uint16_t grecv_len = 0U;//!< 接收到的data域中的有效字节数.
uint8_t grecv_data[USART2_DMA_RECV_BUF_SIZE - 4U];//!< data域的有效内容, 称为"数据共享数组".


//! USART2_RX请求是 DMA1数据流5，请求4.
static void dma1_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    
    DMA_DeInit(DMA1_Stream5);
    while(DMA_GetCmdStatus(DMA1_Stream5) == ENABLE);
    
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);//!< 源地址, 就是串口2的接收数据寄存器.
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)grecv_buf;//!< 目的地址, 就是那个缓冲区.
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = USART2_DMA_RECV_BUF_SIZE;//!< 缓冲区的大小.
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream5, &DMA_InitStructure);
}


//! 串口2, PA2, PA3.
void usart2_dma_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200U;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART2, &USART_InitStructure);
    
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//!< 使能串口空闲中断.
    
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7U;//!< 优先级较高, 可以受FreeRTOS管理, 即可以在里面调用FreeRTOS的接口.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0U;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


    dma1_init();
    DMA_Cmd(DMA1_Stream5, ENABLE);
    while(DMA_GetCmdStatus(DMA1_Stream5) == DISABLE);
    
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    
	USART_Cmd(USART2, ENABLE);
}


#if 0
uint16_t usart2_write(void* buf, uint16_t cnt)
{
    uint8_t* ptr = (uint8_t*)buf;
    
	while(cnt > 0)
	{
		USART_ClearFlag(USART2, USART_IT_TC);
		USART_SendData(USART2, *ptr);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);		
		ptr++;
        cnt--;
	}
    
    return cnt;
}
#endif


/*!
 * @brief: 这个函数就实现判断数据包是否有效, 如果无效, 就返回错误标志; 如果有效, 则它就将data中的有效数据从grecv_buf拷贝到grecv_data中, 并将需要的信息传出来.
 * @param: len: 看到它是一个指针变量, 是个传出参数, 当数据包无效时, 它被赋值为255, 表示数据包无效.
 *              当数据包有效时, 如果是命令, 则它的值表示索引; 如果是数据, 则它表示数据的有效长度.
 * @retval: 'c': 表示该数据包为命令.
 *          'd': 表示该数据包为数据.
 *          '0': 表示该数据包无效.
 *          中断服务函数要根据这个返回值决定唤醒命令处理任务还是数据处理任务.
 * 
 */
static uint8_t usart2_read(uint16_t* len)
{
    static uint8_t type = '0';

    if(*grecv_buf == '$')//!< 如果数据包有效.
    {
        *len = *(grecv_buf + 2U) + ((*(grecv_buf + 3U)) << 8);//!< data域的长度拿出来. 可能是数据长度, 也可能是函数索引. 直接通过参数传出去了.
        
        if(*(grecv_buf + 1U) == 'c')//!< 如果是命令.
        {
            type = 'c';
            memcpy(grecv_data, &(grecv_buf[4U]), NUM_OF_ARGS * 4U);//!< 将参数(12个byte)拷贝到数据共享数组.
        }
        else if(*(grecv_buf + 1U) == 'd')//!< 如果是数据.
        {
            type = 'd';
            memcpy(grecv_data, &(grecv_buf[4U]), *len);//!< 将数据拷贝到数据共享数组中.
        }
    }
    else//!< 如果数据包无效, 下面这些值都可以表示数据包无效.
    {
        type = '0';
        *len = 255U;
    }

    return type;
}


//! 这个函数功能也能简单, 就是当发送串口空闲中断时, 执行一次这个函数, 它把接收到的数据拷贝到grecv_data中, 然后唤醒对应的任务.
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
    {
        extern SemaphoreHandle_t cmd_proc_task_binsem;     
        extern SemaphoreHandle_t data_proc_task_binsem;

        static uint16_t useless;
        useless = USART2->SR;//!< 这两步用于清除中断标志位.
        useless = USART2->DR;
        (void)useless;
        
        DMA_Cmd(DMA1_Stream5, DISABLE);
         
        grecv_num = USART2_DMA_RECV_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream5);//!< 获取此次接收到的字节数.

        static uint8_t type = '0';
        static uint16_t len = 255U;
        type = usart2_read(&len);//!< 把数据拷贝到grecv_data.

        if(len != 255U)//!< 如果数据包有效.
        {
            grecv_len = len;//!< 把接收到的字节数(或者命令索引)传出去. 

            static BaseType_t higher_task_woken = pdFALSE;

            if(type == 'c')
            {
                //! 唤醒命令处理任务.
                xSemaphoreGiveFromISR(cmd_proc_task_binsem, &higher_task_woken);
            }
            else if(type == 'd')
            {
                //! 唤醒数据处理任务.
                xSemaphoreGiveFromISR(data_proc_task_binsem, &higher_task_woken);
            }

            if(higher_task_woken != pdFALSE)//!< 如果需要, 进行一次任务调度.
                taskYIELD();          
        }//!< 'if(len != 255U)'

        DMA_SetCurrDataCounter(DMA1_Stream5, USART2_DMA_RECV_BUF_SIZE);
        DMA_Cmd(DMA1_Stream5, ENABLE);
    }//!< end of 'if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)'
}
