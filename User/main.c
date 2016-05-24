#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "uart4.h"
#include "uart1.h"
#include "Flash.h"
#include <stdio.h>
#include "boot_CFG.h"
#include "Cryptographic.h"
#include "crypto.h"
#include "stm32f10x_rcc.h"
#include "private.h"
#include "string.h"

#define resend_times 5

typedef  void (*pFunction)(void);
u16 parameter_app[24];
u8 send_data[30];
u8 txBuffer[27] =
{
    0x7c,
    0x1B,
    0x00,
    0xA2,
    0xFF,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x7C
};

u16 er[27] =
{
    0x7c,
    0x1B,
    0x00,
    0xA2,
    0xFF,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x7C
};

pFunction Jump_To_Application;

void iap_Loader_App(u32 ApplicationAddress)
{
    u32  JumpAddress;
    if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
        {
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) ApplicationAddress);
            Jump_To_Application();
        }
}

void clock_init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4
                           ,ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|
                           RCC_APB2Periph_AFIO
                           , ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}


bool is_protocol(void)
{
    if((0x005A==flash_read_halfword(appUpdateIfoAddress))&&(0x00A5==flash_read_halfword(appUpdateIfoAddress+2)))
        {
            return true;
        }
    else
        {
            return false;
        }
}

void copy_from_app(void)
{
    u16 temp=0;
    for(u8 i=0; i<19; i++)
        {
            temp=flash_read_halfword(0x0803F000+(i<<1));
            parameter_app[i]=temp;
            if(i>=2)
                {
                    txBuffer[i+3] = temp & 0x00FF;
                }
            flash_write(bootUpdateIfoAddress,parameter_app,24);
        }
}

void data_wifi_processed(u8 * buf,u16 len)
{
    addAES(buf+1,len-4);
    *(buf+len-1)=0x7c;
    addcrc(buf,len);
    len=trans_7c_set(buf,len);
}

bool delay(u32 timeout)
{
    while(1!=receive_ok)
        {
            for(u8 i=0; i<timeout/10; i++)
                {
                    Delay_us(10);
                }
            return false;
        }
    return true;
}



u8 update_app(u32 addr,u32 package)
{
    u8 error_count=0;
    for(u16 i=1; i<=package; i++)
        {
            copy_from_app();
            memcpy(send_data,txBuffer,27);
            send_data[22]=i&0x00ff;
            send_data[23]=i>>8;
            data_wifi_processed(send_data,27);
resend:
            before_send_uart4();
            wifi_send(send_data,27);
            bool temp=delay(100);
            if(temp==true)//接收成功
                {
                    if(receiveDataPakageProcess(buffer,bufferindex))//数据校验正确
                        {
                            if(i%2)
                                {
                                    FLASH_ErasePage(addr+(i-1)*packge_size);
                                }
                            writeFlash(addr+(i-1)*packge_size,&buffer[24],bufferindex-27);
                        }
                    else//失败
                        {
                            error_count++;
                            if(error_count<resend_times)
                                {
                                    goto resend;
                                }
                            //进入软件重启
//                            error_count=flash_read_halfword(bootAppNumAddress);
//                            write_flage(bootUpdateIfoAddress,bootAppNumAddress,error_count+1);
//                            NVIC_SystemReset();
                            return 1;
                        }

                }
            else//接收失败
                {
                    error_count++;
                    if(error_count<resend_times)
                        {
                            goto resend;
                        }
                    //进入软件重启
//                    error_count=flash_read_halfword(bootAppNumAddress);
//                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,error_count+1);
//                    NVIC_SystemReset();
                    return 2;

                }

        }
    //更新成功，写相应的标志位然后重启
//    write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
//    write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,1);//将boot更新完成的标志置1
//    NVIC_SystemReset();
    return 0;
}


/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
u16 temp_=0;
int main(void)
{


    u8 updateinfo = 0;
    /* LED 端口初始化 */
    clock_init();

    led_Init();

    Uart4_Init(115200);
    usart1_conf(115200);
    SysTick_Init();

    Flash_Init();

//    NVIC_SystemReset();//软件重启
//    FLASH_ErasePage(paraAddress);

//    flash_write(paraAddress,er,27);
//    ad=paraAddress;
//    for(u8 i=0;i<26;i++)
//    {
//        tem[i]=flash_read_halfword(ad);
//        ad+=2;
//    }
//    tem[2]=2;
//    tem[3]=1;
//      FLASH_ErasePage(paraAddress);
//    flash_write(paraAddress,(u16*)tem,13);
//    write_flage(paraAddress,paraAddress+16,5);
//
//    temp_=flash_read_halfword(paraAddress+16);

    printf("test\r\n");
    if(0xFFFF==flash_read_halfword(bootAppUpdateStausAddress))
        {
            FLASH_ProgramHalfWord(bootAppUpdateStausAddress,0);
        }
    if(0==flash_read_halfword(bootAppUpdateStausAddress))
        {
            if(true==is_protocol())
                {
                    updateinfo=flash_read_halfword(appUpdateFlagAddress);//需要更新
                    if(update_master==(update_master&updateinfo))
                        {
                            if(update_master_NO1==(update_master_NO1&updateinfo))
                                {
                                    //更新程序
                                    copy_from_app();//需要更新的信息拷贝过来
                                    if(0==update_app(appStartAdress,(txBuffer[21]<<8)|txBuffer[20]))
                                        {
                                            write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
                                            write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,1);//将boot更新完成的标志置1
                                            write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                                            write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                                            write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                                            NVIC_SystemReset();
                                        }
                                    else//更新失败
                                        {
                                            u8 _count=flash_read_halfword(bootAppNumAddress);
                                            write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                                            write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                                            NVIC_SystemReset();
                                        }
                                }
                            else
                                {
                                    copy_from_app();//需要更新的信息拷贝过来
                                    //更新程序
                                    if(0==update_app(appBackStartAdress,(txBuffer[21]<<8)|txBuffer[20]))
                                        {
                                            write_flage(isbackup,isbackup,1);
                                            write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
                                            write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,1);//将boot更新完成的标志置1
                                            write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                                            write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                                            write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                                            NVIC_SystemReset();
                                        }
                                    else
                                        {
                                            u8 _count=flash_read_halfword(bootAppNumAddress);
                                            write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                                            write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                                            NVIC_SystemReset();
                                        }

                                }

                        }
                    else if(update_slave==(update_slave&updateinfo))
                        {
                            //更新控制板的程序
                            //进行相应的初始化操作。
                            //初始化串口1



                        }
                }
            else
                {
                    goto jump;
                }
        }
    else   //将boot的更新完成指令写成0，然后跳转到相应的程序中去
        {
            write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,0);//将boot更新完成的标志置1
            goto jump;
        }
jump:
    if(1==flash_read_halfword(isbackup))
        {
            USART_DeInit(USART1);            
            iap_Loader_App(appBackStartAdress);
        }
    else
        {
            USART_DeInit(USART1);
            iap_Loader_App(appStartAdress);
        }

    for(;;)
        {
        }
}
/*********************************************END OF FILE**********************/
