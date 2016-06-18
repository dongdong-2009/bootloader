#include "uart4.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "string.h"
volatile u8 receive_ok=0;
u8 buffer[2048];
u8 isdata=0;
u32 bufferindex=0;
void Uart4_Init(int baud_rate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
    USART_Init(UART4,&USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
}



static void UART4_SEND_CHAR(char ch)
{
    USART_SendData(UART4, ch);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

//static void wifi_send_char(char ch)
//{
//    UART4_SEND_CHAR(ch);
//}

void wifi_send(u8 * buf,u32 len)
{
    while(len--)
        {
            UART4_SEND_CHAR(*buf++);
        }
}

void before_send_uart4(void)
{
    isdata=0;
    bufferindex=0;
    receive_ok=0;
    memset(buffer,0,2048);
}

void UART4_IRQHandler(void)
{

    unsigned char dat;
    if (USART_GetITStatus(UART4, USART_IT_RXNE) == RESET)
        {
            return;
        }

    dat = USART_ReceiveData(UART4);
        if(1==receive_ok)
        {
            return ;
        }
//    USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        
    if(1==isdata)
        {
            if(0x7c==dat)
                {
                    
                    if(bufferindex<1000)
                    {
                    isdata=1;
                    bufferindex=0;
                    buffer[bufferindex++]=dat;
                    return ;                        
                    }
                    buffer[bufferindex++]=dat;
                    isdata=0;
                    receive_ok=1;
                                               
                }
            else
                {
                    buffer[bufferindex++]=dat;
                }
            return;
        }
    if(0x7c==dat)
        {
            buffer[bufferindex++]=dat;
            isdata=1;
        }

}

