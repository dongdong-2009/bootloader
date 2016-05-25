#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "uart4.h"
#include "uart1.h"
#include "Flash.h"
#include <stdio.h>
#include "boot_CFG.h"

#include "crypto.h"
#include "stm32f10x_rcc.h"
#include "private.h"

#include "string.h"

#define resend_times 5

typedef  void (*pFunction)(void);
u16 parameter_app[24];

u8 send_data[30];

u8 txBuffer[27] = {
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
    if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000) {
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
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
}


bool is_protocol(void)
{
    if((0x005A==flash_read_halfword(appUpdateIfoAddress))&&(0x00A5==flash_read_halfword(appUpdateIfoAddress+2))) {
        return true;
    } else {
        return false;
    }
}

void copy_from_app(void)
{
    u16 temp=0;
    for(u8 i=0; i<19; i++) {
        temp=flash_read_halfword(0x0803F000+(i<<1));
        parameter_app[i]=temp;
        if(i>=2) {
            txBuffer[i+3] = temp & 0x00FF;
        }
        flash_write(bootUpdateIfoAddress,parameter_app,24);
    }
}

void crc_7c(u8 * buf,u16 len)
{
    *(buf+len-1)=0x7c;
    addcrc(buf,len);
    len=trans_7c_set(buf,len);
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
     
        for(u8 i=0; i<timeout/10; i++) {
            if(1!=receive_ok)
            {
            Delay_us(10);
            }
            else {
            return true;
            }
        }
        return false;
}


bool delay_u1(u32 timeout)
{
    while(1!=receive_salver) {
        for(u8 i=0; i<timeout/10; i++) {
            Delay_us(10);
        }
        return false;
    }
    return true;
}


u16 sa_dat_process(u8 *p,u16 len)
{
    u16 clrLen = 0;
    u8 crcCheckStatus;

    clrLen = trans_7c_clr(p,len);
    crcCheckStatus = checkcrc(p,clrLen);

    if(crcCheckStatus == 1) {
        return clrLen;
    } else {
        return 0;
    }
}


u8 update_salver(void)
{
    u8 err_sa=0;
    usart1_conf(115200);
    //发送一条升级指令给从机
    copy_from_app();
    memcpy(send_data,txBuffer,25);
    send_data[1]=25;
    send_data[3]=0xA4;
    crc_7c(send_data,25);
resend_sa:
    before_send_sa();
    MASTER_SEND(send_data,25);
    if(true==delay_u1(100)) {
        //数据处理
        if(sa_dat_process(u1_buffer,u1_bufferindex)>0) {

            //接收到从机的确认升级
            if((0xA4==u1_buffer[3])&&(0==memcmp(&send_data[5],&u1_buffer[5],17))) {
                //接受数据正常
                return 0;
            } else { //异常
                err_sa++;
                if(err_sa<5) {
                    goto resend_sa;
                }
                return 2;
            }
        }
        return 3;
    } else {
        err_sa++;
        if(err_sa<5) {
            goto resend_sa;
        }
        return 1;//从机应答失败
    }

}


u8 update_app(u32 addr,u32 package)
{
    u8 error_count=0;
    for(u16 i=1; i<=package; i++) {
//        copy_from_app();
        memcpy(send_data,txBuffer,27);
        send_data[22]=i&0x00ff;
        send_data[23]=i>>8;
        if(0x37==i)
        {
        char mmm=1;
        
        }
        data_wifi_processed(send_data,27);        
resend:
        before_send_uart4();
        wifi_send(send_data,27);
        bool temp=delay(10000);
        if(temp==true) { //接收成功
            if(receiveDataPakageProcess(buffer,bufferindex)) { //数据校验正确
                if(i%2) {
                    FLASH_ErasePage(addr+(i-1)*packge_size);
                }
                writeFlash(addr+(i-1)*packge_size,&buffer[24],bufferindex-27);
            } else { //失败
                error_count++;
                if(error_count<resend_times) {
                    goto resend;
                }
                return 1;
            }
        } else { //接收失败
            error_count++;
            if(error_count<resend_times) {
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
    /* LED 端口初始化 */
    clock_init();

    led_Init();

    Uart4_Init(115200);

    SysTick_Init();

    Flash_Init();

    if(0xFFFF==flash_read_halfword(bootAppUpdateStausAddress)) {
        FLASH_ProgramHalfWord(bootAppUpdateStausAddress,0);
    }
    if(0==flash_read_halfword(bootAppUpdateStausAddress)) {
        if(true==is_protocol()) {
            updateinfo=flash_read_halfword(appUpdateFlagAddress);//需要更新
            updateinfo=0X30;
            if(update_master==(update_master&updateinfo)) {
                if(update_master_NO1==(update_master_NO1&updateinfo)) {
                    //更新程序
                    copy_from_app();//需要更新的信息拷贝过来
                    if(0==update_app(appStartAdress,(txBuffer[21]<<8)|txBuffer[20])) {
                        write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
                        write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,1);//将boot更新完成的标志置1
                        write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                        write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                        write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                        NVIC_SystemReset();
                    } else { //更新失败
                        u8 _count=flash_read_halfword(bootAppNumAddress);
                        write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                        write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                        NVIC_SystemReset();
                    }
                } else {
                    copy_from_app();//需要更新的信息拷贝过来
                    //更新程序
                    if(0==update_app(appBackStartAdress,(txBuffer[21]<<8)|txBuffer[20])) {
                        write_flage(isbackup,isbackup,1);
                        write_flage(bootUpdateIfoAddress,bootAppNumAddress,0);
                        write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,1);//将boot更新完成的标志置1
                        write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,1);//新版本有效标志
                        write_flage(bootUpdateIfoAddress,bootVerByte_1_Add,txBuffer[18]);//写版本信息
                        write_flage(bootUpdateIfoAddress,bootVerByte_2_Add,txBuffer[19]);//同上
                        NVIC_SystemReset();
                    } else {
                        u8 _count=flash_read_halfword(bootAppNumAddress);
                        write_flage(bootUpdateIfoAddress,bootAppNumAddress,_count+1);
                        write_flage(bootUpdateIfoAddress,bootNewVerFlagAddress,0);//新版本无效标志
                        NVIC_SystemReset();
                    }

                }

            } else if(update_slave==(update_slave&updateinfo)) {
                u8 __err=0;
                if(0==update_salver()) { //从机升级应答正常
                    while(1) {
u1_rec_ok:
                        if(1==receive_salver) {
                            if(sa_dat_process(u1_buffer,u1_bufferindex)>0) {
                                if(0xA2==u1_buffer[3]) {
                                    data_wifi_processed(u1_buffer,27);
re:
                                    before_send_uart4();
                                    wifi_send(u1_buffer,27);
                                    bool temp=delay(100);
                                    if(temp==true) { //接收成功
                                        if(receiveDataPakageProcess(buffer,bufferindex)) { //数据校验正确
                                            crc_7c(buffer,bufferindex);
__re_send_sa:
                                            before_send_sa();
                                            MASTER_SEND(buffer,bufferindex);
                                            if(true==delay_u1(1000)) {
                                                //接收成功
                                                goto u1_rec_ok;
                                            } else {
                                                __err++;
                                                if(__err<5) {
                                                    goto __re_send_sa;
                                                }
                                                goto jump;
                                            }
                                        } else { //失败
                                            __err++;
                                            if(__err<5) {
                                                goto re;
                                            }
                                        }
                                    } else { //接收失败
                                        __err++;
                                        if(__err<5) {
                                            goto re;
                                        }
                                    }
                                } else if(0xFF==u1_buffer[3]) { //更新完成，重启
                                }
                            }
                        }
                    }
                } else { //应答异常
                    goto jump;
                }
            }
        } else {
            goto jump;
        }
    } else { //将boot的更新完成指令写成0，然后跳转到相应的程序中去
        write_flage(bootUpdateIfoAddress,bootAppUpdateStausAddress,0);//将boot更新完成的标志置1
        goto jump;
    }
jump:
    if(1==flash_read_halfword(isbackup)) {
        USART_DeInit(USART1);
        USART_DeInit(UART4);
        iap_Loader_App(appBackStartAdress);
    } else {
        USART_DeInit(USART1);
        USART_DeInit(UART4);
        iap_Loader_App(appStartAdress);
    }
//restart:
//        NVIC_SystemReset();
    for(;;) {
    }
}
/*********************************************END OF FILE**********************/
