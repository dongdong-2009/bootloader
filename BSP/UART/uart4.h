#ifndef __UART4__H
#define __UART4__H
#include "stm32f10x.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "boot_CFG.h"

extern volatile u8 receive_ok;

extern u8 buffer[2048];
extern u32 bufferindex;

void Uart4_Init(int baud_rate);
void wifi_send(u8 * buf,u32 len);
void before_send_uart4(void);

void GSM_USART_INIT(u32 rate);
 void UART4_SEND_CHAR(char ch);

#endif

