#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "tucomm.h"

//! 这个文件主要对 对串口进行读写 进行了封装, 提供几个函数, 方便上层应用调用.

//! 对上位机需要读写, 而对STM32就只需要写, 不需要读. 考虑到这个问题, 所以读写函数张的不一样.


//#define stdout 1 //!< Because I don't want to include stdio.h!


typedef struct
{
	uint8_t  head;
	uint8_t  type;
	uint16_t len;
	uint8_t  data[256];//!< 这些缓冲区的大小, 后期统一改一下.
}pack_t;


//! 这个函数的参数设计比较复杂, 不用管它, 不直接调用它, 而是调用封装后的宏.
void tucomm_general_write(int fd, uint8_t type, uint16_t len_or_index, uint8_t* buf, uint32_t param1, uint32_t param2, uint32_t param3)
{
	static pack_t pack;
	pack.head = '$';

	if(type == 'c')//!< 如果是命令.
	{
		pack.type = 'c';
		pack.len = len_or_index;

		memcpy(&(pack.data[0]), &param1, 4);
		memcpy(&(pack.data[4]), &param2, 4);
		memcpy(&(pack.data[8]), &param3, 4);

		write(fd, &pack, 4U + 3U * 4U);
	}
	else if(type == 'd')//!< 如果是数据.
	{
		pack.type = 'd';
		pack.len = len_or_index;

		memcpy(&(pack.data[0]), buf, pack.len);

		write(fd, &pack, 4U + pack.len);
	}
}


// //! n's unit is word. It will return how many integer data received.
// void tucomm_read_data(int fd, int32_t* buf, uint16_t* n)
// {
// 	static uint8_t temp_buf[TUCOM_CONFIG_DATA_MAX_LENGTH_IN_BYTE + 4U] = {'\0'};
// 	static uint16_t len = 0U;

// 	read(fd, temp_buf, 512U);
// 	if(*temp_buf == '$')
// 	{
// 		len = *(temp_buf + 2U) + ((*(temp_buf + 3U)) << 8U);
// 		memcpy(buf, &(temp_buf[4U]), len);
// 		*n = len / sizeof(int);
// 	}
// 	else
// 	{
// 		*n = 0U;	
// 	}
// }
