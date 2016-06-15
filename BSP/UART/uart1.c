#include "uart4.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stdio.h"
#include "string.h"

u8 u1_buffer[2048];

u8 is_slave=0;
u32 u1_bufferindex=0;;
u8 receive_slave=0;

void usart1_conf(u32 baud_rate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef   USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure);

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART5, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}




static void USART1_SEND_CHAR(char ch)
{
    USART_SendData(UART5, ch);
    while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}


void UART5_IRQHandler(void)
{
    if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
        {

            u8 dat = USART_ReceiveData(UART5);

//            USART_ClearITPendingBit(USART1, USART_IT_RXNE);
            if(1==is_slave )
                {
                    if(0x7c==dat )  //判断一下是不是一帧数据
                        {
                            if(u1_bufferindex<10)//数据帧起始字节
                                {
                                    is_slave=1;
                                    u1_bufferindex=0;
                                    u1_buffer[u1_bufferindex++]=dat;
                                    return ;
                                }
                            is_slave=0;
                            receive_slave=1;
                            u1_buffer[u1_bufferindex++]=dat;
                        }
                    else
                        {
                            u1_buffer[u1_bufferindex++]=dat;
                        }
                    return ;
                }
            if(0x7c==dat)
                {
                    is_slave=1;
                    u1_buffer[u1_bufferindex++]=dat;
                }

        }
}

void MASTER_SEND(u8 * buf,u32 len)
{
    while(len--)
        {
            USART1_SEND_CHAR(*buf++);
        }
}


void before_send_sa(void)
{
    memset(u1_buffer,0,2048);
    receive_slave=0;
    is_slave =0;
    u1_bufferindex=0;

}
