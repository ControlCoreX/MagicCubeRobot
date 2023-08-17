#ifndef _DELAY_H_
#define _DELAY_H_

#include "stdint.h"

extern void delay_init(void);
extern void delay_us(uint32_t nus);
extern void delay_ms(uint32_t nms);

#endif //!< _DELAY_H_

