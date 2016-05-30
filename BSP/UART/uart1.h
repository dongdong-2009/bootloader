#ifndef __UART1__H
#define __UART1__H
#include "stm32f10x.h"

extern u8 receive_slave;
extern u8 u1_buffer[2048];
extern u32 u1_bufferindex;
void usart1_conf(u32 baud_rate);

void MASTER_SEND(u8 * buf,u32 len);

void before_send_sa(void);
#endif

