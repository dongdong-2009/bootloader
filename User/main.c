
#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "uart4.h"

#include "Flash.h"
#include <stdio.h>
#include "boot_CFG.h"

#include "crypto.h"
#include "stm32f10x_rcc.h"

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

void process_send_data(u8 * buf,u16 len)
{
    addAES(buf,len);
    
    
}
#include "string.h"

bool update_app(u32 addr,u32 package)
{
    for(u16 i=1; i<=package; i++)
        {
        copy_from_app();
        memcpy(send_data,txBuffer,27);
        send_data[22]=i&0x00ff;
        send_data[23]=i>>8;
            
            
            
//        wifi_send(buf,len);
//        delay();
//if()//���յ����ݣ����д���
//{

//}
//else//���ݽ��մ�������ط������޴Σ���Ȼʧ��ѡ������
//{

//}



        }

}


/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
u16 temp;
int main(void)
{
    u8 updateinfo = 0;
    /* LED �˿ڳ�ʼ�� */
    clock_init();

    led_Init();

    Uart4_Init(115200);

    Flash_Init();
    if(0xFFFF==flash_read_halfword(bootAppUpdateStausAddress))
        {
            FLASH_ProgramHalfWord(bootAppUpdateStausAddress,0);
        }
    if(0==flash_read_halfword(bootAppUpdateStausAddress))
        {
            if(true==is_protocol())
                {
                    updateinfo=flash_read_halfword(appUpdateFlagAddress);//��Ҫ����
                    if(update_master==(update_master&updateinfo))
                        {
                            if(update_master_NO1==(update_master_NO1&updateinfo))
                                {
                                    //���³���


                                    FLASH_ProgramHalfWord(bootAppUpdateStausAddress,1);//�������
                                }
                            else
                                {
                                    //���³���

                                    FLASH_ProgramHalfWord(isbackup,1);
                                }

                        }
                    else if(update_slave==(update_slave&updateinfo))
                        {
                            //���¿��ư�ĳ���



                        }
                    else//�������
                        {
                            goto jump;
                        }

                }
            else
                {
                    goto jump;
                }
        }
    else   //��boot�ĸ������ָ��д��0��Ȼ����ת����Ӧ�ĳ�����ȥ
        {

            goto jump;
        }
jump:
    if(1==flash_read_halfword(isbackup))
        {
            iap_Loader_App(appBackStartAdress);
        }
    else
        {
            iap_Loader_App(appStartAdress);
        }
//iap_Loader_App(appStartAdress);
    /* ����SysTick Ϊ10us�ж�һ�� */
//	SysTick_Init();

    for(;;)
        {

        }
}
/*********************************************END OF FILE**********************/
