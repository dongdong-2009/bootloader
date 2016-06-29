#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "uart4.h"
#include "uart1.h"
#include "Flash.h"
#include <stdio.h>
#include "boot_CFG.h"
#include "stdbool.h"
#include "crypto.h"
#include "stm32f10x_rcc.h"
#include "private.h"

#include "string.h"

#define resend_times 5

u8 _is_back=0;

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

pFunction Jump_To_Application;

static void iap_Loader_App(u32 ApplicationAddress)
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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA		  // 开启APB时钟
                           |RCC_APB2Periph_GPIOB
                           |RCC_APB2Periph_GPIOC
                           |RCC_APB2Periph_GPIOD
                           |RCC_APB2Periph_GPIOE
                           |RCC_APB2Periph_USART1
                           |RCC_APB2Periph_AFIO
                           ,ENABLE);
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_USART2|
        RCC_APB1Periph_USART3|
        RCC_APB1Periph_UART4|
        RCC_APB1Periph_UART5
        |RCC_APB1Periph_TIM3
        ,ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}


static bool is_protocol(void)
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

static void copy_from_app(void)
{
    u16 temp=0;
    for(u8 i=0; i<19; i++)
        {
            temp=flash_read_halfword(appUpdateIfoAddress+(i<<1));
            parameter_app[i]=temp;
            if(i>=2)
                {
                    txBuffer[i+3] = temp & 0x00FF;
                }
        }
//    for(u8 j=0;j<50;j++)
//    {
//        parameter_app[j]=flash_read_halfword(appUpdateIfoAddress+(j<<1));
//    }
    FLASH_ErasePage(bootUpdateIfoAddress);
    flash_write(bootUpdateIfoAddress,parameter_app,24);
}

static u16 crc_7c(u8 * buf,u16 len)
{
    *(buf+len-1)=0x7c;
    addcrc(buf,len);
    return trans_7c_set(buf,len);
}

static u16 data_wifi_processed(u8 * buf,u16 len)
{
    addAES(buf+1,len-4);
    *(buf+len-1)=0x7c;
    addcrc(buf,len);
    return trans_7c_set(buf,len);
}


//static void __delay(int time)
//{
//    for(u32 i=0;i<time;i++)
//    {
//        for(u32 j=0;j<1000;j++);
//    }
//}
static bool delay(u32 timeout)
{

    for(u32 i=0; i<timeout; i++)
        {
            if(1!=receive_ok)
                {
                    Delay_us(1000);
                }
            else
                {
                    return true;
                }
        }
    return false;
}


bool delay_u1(u32 timeout)
{
    for(u32 i=0; i<timeout; i++)
        {
            if(1!=receive_slave)
                {
                    Delay_us(1000);
                }
            else
                {
                    return true;
                }
        }
    return false;
}


u16 sa_dat_process(u8 *p,u16 len)
{
    u16 clrLen = 0;
    u8 crcCheckStatus;

    clrLen = trans_7c_clr(p,len);
    crcCheckStatus = checkcrc(p,clrLen);

    if(crcCheckStatus == 1)
        {
            return clrLen;
        }
    else
        {
            return 0;
        }
}


u8 info_slave(void)
{
    u8 err_sa=0;
    u16 len =0;
//    copy_from_app();
    memcpy(send_data,txBuffer,25);
    send_data[1]=25;
    send_data[17]=0x02;
    //将升级指示标志改为1
    send_data[3]=0xA3;
    len = crc_7c(send_data,25);
resend_sa:
    before_send_sa();
    MASTER_SEND(send_data,len);
    if(true==delay_u1(2000))
        {
            return 0;
        }
    else
        {
            err_sa++;
            if(err_sa<err_threshold)
                {
                    goto resend_sa;
                }
            return 1;//从机应答失败
        }
//return false;

}

u8 __info(void)
{
    u16 err_num=0;
    u16 len =0;
    memcpy(send_data,txBuffer,25);
    send_data[1]=25;
    send_data[17]=0x02;
    send_data[3]=0xA3;
    len = crc_7c(send_data,25);
__re_info:
    before_send_sa();
    MASTER_SEND(send_data,len);
    if(true==delay_u1(1000))
        {
            return 0;
        }
    else
        {
            err_num++;
            if(err_num<5)
                {
                    goto  __re_info;
                }
            return 1;//从机应答失败
        }

}

bool slave_update(void)
{
    u16 len=0;
    u8 __err=0;
_re_info:
    if(0==__info())   //从机升级应答正常    此处有肯能出现第一次更新不成功，从夫发起的状况
        {

            while(1)
                {
u1_rec_ok:
                    if(sa_dat_process(u1_buffer,u1_bufferindex)>0)
                        {
                            if(0xA2==u1_buffer[3])
                                {
                                    len=data_wifi_processed(u1_buffer,27);
re:
                                    before_send_uart4();
                                    wifi_send(u1_buffer,len);
                                    bool temp=delay(5000);
                                    if(temp==true)   //接收成功
                                        {
                                            len = receiveDataPakageProcess(buffer,bufferindex);
                                            if(len>0)   //数据校验正确
                                                {
                                                    len=crc_7c(buffer,len);
__re_send_slave:
                                                    before_send_sa();
                                                    MASTER_SEND(buffer,len);
                                                    if(true==delay_u1(10000))
                                                        {
                                                            //接收成功
                                                            goto u1_rec_ok;
                                                        }
                                                    else
                                                        {
                                                            __err++;
                                                            if(__err<5)
                                                                {
                                                                    goto __re_send_slave;
                                                                }

                                                            return false;
                                                        }
                                                }
                                            else     //失败
                                                {
                                                    __err++;
                                                    if(__err<5)
                                                        {
                                                            goto re;
                                                        }
                                                    return false;
                                                }
                                        }
                                    else     //接收失败
                                        {
                                            __err++;
                                            if(__err<5)
                                                {
                                                    goto re;
                                                }
                                            return false;
                                        }
                                }
                            if(0xA3==u1_buffer[3])
                                {
                                    return false;
                                }
                            if(0xFF==u1_buffer[3])     //更新完成，重启
                                {
                                    return true;
                                }
                            else
                                {
                                    goto _re_info;
                                }
                        }
                    else     //接收失败
                        {
                            __err++;
                            if(__err<5)
                                {
                                    goto re;
                                }
                            return false;
                        }

                }
        }
    else     //应答异常
        {
            return false;
        }
}

u8 update_app(u32 addr,u32 package)
{
    u8 error_count=0;
    u16 len=0;
    u16 temp_pakge=0;

    for(u16 i=1; i<=package; i++)
        {
            
//        copy_from_app();
            temp_pakge=i;
            memcpy(send_data,txBuffer,27);

            send_data[17]=_is_back;

            send_data[22]=i&0x00ff;
            send_data[23]=i>>8;
            len=data_wifi_processed(send_data,27);
resend:
            before_send_uart4();
            wifi_send(send_data,len);

//            send_com(send_data,27);

            bool temp=delay(10000);
            if(temp==true)   //接收成功
                {
                    u16 __len=receiveDataPakageProcess(buffer,bufferindex);
                    if(__len!=0)   //数据校验正确
                        {
                            if((buffer[22]|buffer[23]<<8)==temp_pakge)
                                {
                                    if(i%2)
                                        {
                                            FLASH_ErasePage(addr+(i-1)*packge_size);
                                        }
                                    writeFlash(addr+(i-1)*packge_size,&buffer[24],__len-27);
                                }
                            else
                                {
                                    error_count++;
                                    if(error_count<5)
                                        {
                                            goto resend;
                                        }
                                    return 1;

                                }
                        }
                    else     //失败
                        {
                            error_count++;
                            if(error_count<resend_times)
                                {
                                    goto resend;
                                }
                            return 1;
                        }
                }
            else     //接收失败
                {
                    error_count++;
                    if(error_count<resend_times)
                        {
                            goto resend;
                        }
                    return 2;
                }
        }
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
    u16 info=0;
    clock_init();

    led_Init();

    Uart4_Init(115200);

    SysTick_Init();

    Flash_Init();

    usart1_conf(115200);
    if(0xFFFF==flash_read_halfword(boot_version))
        {
            write_flage(boot_version,boot_version,0x0202);
        }

    if(0xFFFF==flash_read_halfword(boot_location_flag))
        {
            FLASH_ProgramHalfWord(boot_location_flag,0);
        }
    u8 up_info=flash_read_halfword(boot_location_flag);
    if(0==up_info)
        {
            if(true==is_protocol())
                {
                    updateinfo=flash_read_halfword(appUpdateFlagAddress);//需要更新
//                  updateinfo=0;//update_master;             //模拟测试升级从机板
                    if((update_master!=(update_master&updateinfo))\
                            &&(update_master_backup!=(update_master_backup&updateinfo))\
                            &&(update_slave!=(update_slave&updateinfo)))
                        {
                            if(0!=flash_read_halfword(bootAppUpdateStausAddress))
                                {
                                    write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,0);
                                }
                        }

                    if((update_master==(update_master&updateinfo))\
                            ||(update_master_backup==(update_master_backup&updateinfo))\
                            ||(update_slave==(update_slave&updateinfo)))
                        {
                            copy_from_app();//需要更新的信息拷贝过来
                        }
                    if(update_master==(update_master&updateinfo))
                        {
                            _is_back=0x01;
                            //更新程序
                            copy_from_app();//需要更新的信息拷贝过来
                            if(0==update_app(appStartAdress,(txBuffer[21]<<8)|txBuffer[20]))
                                {
                                    write_flage(isbackup,isbackup,0);
                                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
//                                    u16 info= flash_read_halfword(bootAppUpdateStausAddress) ;

                                    info|=update_master;
                                    write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,info);//将boot更新完成的标志置1
                                    write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                                    write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                                    write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,1);
//                                    NVIC_SystemReset();
                                }
                            else     //更新失败
                                {
                                    u8 _count=flash_read_halfword(bootAppNumAddress);
                                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                                    write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,0);
                                    goto jump;
                                }

                        }
                    if(update_master_backup==(update_master_backup&updateinfo))
                        {
                            _is_back=0x10;
//                            copy_from_app();
                            if(0==update_app(appBackStartAdress,(txBuffer[21]<<8)|txBuffer[20]))
                                {
                                    write_flage(isbackup,isbackup,1);
                                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
                                    u16 info= flash_read_halfword(bootAppUpdateStausAddress) ;
                                    info|=update_master_backup;
                                    write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,info);//将boot更新完成的标志置1
                                    write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                                    write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                                    write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,1);
//                                    NVIC_SystemReset();
                                }
                            else
                                {
                                    u8 _count=flash_read_halfword(bootAppNumAddress);
                                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                                    write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,0);
                                    goto jump;

                                }


                        }

                    if(update_slave==(update_slave&updateinfo))
                        {

                            if(true==slave_update())
                                {
                                    u16 info= flash_read_halfword(bootAppUpdateStausAddress) ;
                                    info|=update_slave;
                                    write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,info);//将boot更新完成的标志置1
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,1);
                                }
                            else
                                {
                                    u8 _count=flash_read_halfword(bootAppNumAddress);
                                    write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                                    write_flage(bootUpdateIfoAddress,boot_location_flag,0);
                                    goto jump;
                                }
                        }

                }
        }
    else     //将boot的更新完成指令写成0，然后跳转到相应的程序中去
        {
            write_flage(bootUpdateIfoAddress,boot_location_flag,0);//将boot更新完成的标志置1
            goto jump;
        }

jump:
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
    if(1==flash_read_halfword(boot_location_flag)) //有更新
        {
            goto reboot;
        }
    USART_DeInit(USART1);
    USART_DeInit(UART4);
    FLASH_Lock();
    if(1==flash_read_halfword(isbackup))
        {
            iap_Loader_App(appBackStartAdress);
        }
    else
        {
            iap_Loader_App(appStartAdress);
        }
reboot:
    NVIC_SystemReset();

    for(;;)
        {
            LED1(0);
            Delay_us(100000);
            LED1(1);
            Delay_us(100000);
        }
}
/*********************************************END OF FILE**********************/
