#ifndef __UART4__H
#define __UART4__H
#include "stm32f10x.h"

extern u8 receive_ok;

extern u8 buffer[2048];
extern u32 bufferindex;

void Uart4_Init(int baud_rate);
void wifi_send(u8 * buf,u32 len);


#endif

