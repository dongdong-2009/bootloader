#ifndef __UART4__H
#define __UART4__H
#include "stm32f10x.h"

void Uart4_Init(int baud_rate);
void wifi_send(char * buf,u32 len);

#endif

