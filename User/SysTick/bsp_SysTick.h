#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"
#include "stdbool.h"

void SysTick_Init(void);
void Delay_us(__IO u32 nTime);
#define Delay_ms(x) Delay_us(100*x)	 //µ¥Î»ms

bool delay(u32 timeout);

#endif /* __SYSTICK_H */
