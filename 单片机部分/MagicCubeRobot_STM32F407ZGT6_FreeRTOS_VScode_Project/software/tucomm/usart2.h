/*!
 * @file: usart2.h
 * @author: 屠文博-自卓201-201404010126
 * @date: 2023年5月17日15:52:11
 * @brief:	这个文件定义了数据包的格式pack_t.
 *			定义了一个宏USART2_DMA_RECV_BUF_SIZE, 它用来控制数据包的最大长度(字节数), 后续需要改动的话从这里改.
 * 
 * @note:	1. 每次只能处理一个数据包. 
 * 			即接收数据包时, 如果一次发送了几个数据包, 即两个数据包中间隔的时间过短, 没有导致串口总线空闲, 则会被认为是一个数据包, 而此时会出错.
 * 			后面可以改进数据包的格式, 在其中加入帧头和帧尾, 这样就可以判断出数据包的个数.
 * 			2. 数据包中的data域, 在叫法上, 非特别区分命令和数据的场合, 都称为数据, 注意理解注释的意思.
 * 			3. 
 */

#ifndef _TUCOMM_USART2_H_
#define _TUCOMM_USART2_H_

#include "stdint.h" //!< 为了使用uint8_t类似这种类型.

#define USART2_DMA_RECV_BUF_SIZE    128U

typedef struct
{
	uint8_t  head;//!< 帧头就是'$'.
	uint8_t  type;//!< 用于标记是命令还是数据. 如果type=='c', 则这个数据包就是命令; 如果type=='d', 则这个数据包是数据.
	uint16_t len;//!< 表示后面data域中有效的数据长度, 字节数.
	uint8_t  data[USART2_DMA_RECV_BUF_SIZE - 4U];//!< 存储数据的地方.
}pack_t;


extern void usart2_dma_init(void);


#endif //!< _TUCOMM_USART2_H_
