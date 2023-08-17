#ifndef _TUCOMM_TUCOMM_H_
#define _TUCOMM_TUCOMM_H_

// //! How many integer data do you want to transfer. Configurable item.
// #define TUCOM_CONFIG_DATA_MAX_LENGTH_IN_WORD 10U

//! 因为这个函数是用C写的, 但是如果要跟C++文件链接到一块, 就要用这个东西告诉编译器, 要使用C的规则.
#ifdef __cplusplus
extern "C" {
#endif

extern void tucomm_general_write(int fd, uint8_t type, uint16_t len_or_index, uint8_t* buf, uint32_t param1, uint32_t param2, uint32_t param3);

#ifdef __cplusplus
}
#endif


//! 命令带 索引 和 参数, 数据带 长度 和 buf.
#define tucomm_write_cmd(fd, index, param1, param2, param3) 	tucomm_general_write(fd, 'c', index, NULL, param1, param2, param3)
#define tucomm_write_data(fd, len, buf) 						tucomm_general_write(fd, 'd', len, buf, 0U, 0U, 0U)


#endif /* _TUCOMM_TUCOMM_H_ */
