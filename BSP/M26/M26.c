#include "M26.h"
#include "Flash.h"

extern _bootloader_type bootloader_step;
//查询波特率
u32 gsm_checkbandrate(void)
{
    const u32 lut[]=
    {
        9600,
        115200,
        4800,
        38400,
        19200,
        57600,
        230100,
        460800,
        921600
    };
    char test[100];
    memset(test,0,sizeof(test)/sizeof(test[0]));
    sprintf(test,"AT\r");

    for(u8 i=0; i<sizeof(lut)/sizeof(u32); i++)
        {
            GSM_USART_INIT(lut[i]);
            Delay_us(1000);
            if(true==ATcheckreply((u8*)test,"ANY",2000))
                {
                    return lut[i];
                }
        }
    return 0;
}

//保存固定的波特率
bool gsm_save_rate(u32 rate)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+IPR=%d&W\r",rate);
    return ATcheckreply((u8*)send_buf,"OK\r",2000);
}

//查看gsm是否注册
bool gsm_isregister(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+CREG?\r");
    return ATcheckreply((u8*)send_buf,"+CREG: 0,1\r",2000);
}

//查看模块信号强度
u8 gsm_signal(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+CSQ\r");
    if(true==ATcheckreply((u8*)send_buf,"ANY\r",2000))
        {
            return (buffer[6]-'0')*10+(buffer[7]-'0');
        }
    return false;
}
//查看GPRS是否注册上
bool gsm_gprs(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+CGREG?\r");
    return ATcheckreply((u8*)send_buf,"+CGREG: 0,1\r",2000);
}



//查看GPRS是否插卡
bool gsm_CPIN(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+CPIN?\r");
    return ATcheckreply((u8*)send_buf,"+CPIN: READY\r",2000);
}


bool gsm_setconnectmode(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+QIMODE=1\r");
    return ATcheckreply((u8*)send_buf,"OK\r",2000);
}

//查询gprs附着
bool gsm_gprs_attach(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+CGATT?\r");
    return ATcheckreply((u8*)send_buf,"+CGATT: 1\r",2000);
}

//设置发送参数
bool gsm_set_sendpra(void)
{
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"AT+QITCFG=3,2,512,1\r");
    return ATcheckreply((u8*)send_buf,"OK\r",2000);
}

//连接tcp
static bool _gsm_connect_server(_server_pra* info)
{
    bool __sta=false;
    char send_buf[100];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    //read flash;

    info->IP[0]=123;
    info->IP[1]=59;
    info->IP[2]=136;
    info->IP[3]=48;
    info->PORT[0]=0x49;
    info->PORT[1]=0x12;
    u16 IP_PORT= (info->PORT[0]<<8)|info->PORT[1];
    sprintf(send_buf,"AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r",info->IP[0],info->IP[1],info->IP[2],info->IP[3],IP_PORT);
    if(ATcheckreply((u8*)send_buf,"OK\r",2000))
    {
    rec_send_none("ANY\r",8000);
    u16 len=strlen((char*)buffer)-2;
    if(0==strncmp((char*)buffer,"CONNECT OK\r",len))
    {
        __sta=true;
    }
}
     return __sta;
}


static void read_IPpra(_server_pra* info)
{
    info->IP[0]=flash_read_halfword(IP_ADRESS+2*1)&0xff;
    info->IP[1]=flash_read_halfword(IP_ADRESS+2*2)&0xff;
    info->IP[2]=flash_read_halfword(IP_ADRESS+2*3)&0xff;
    info->IP[3]=flash_read_halfword(IP_ADRESS+2*4)&0xff;
    info->PORT[0]=flash_read_halfword(IP_ADRESS+2*5)&0xff;
    info->PORT[1]=flash_read_halfword(IP_ADRESS+2*6)&0xff;
}


bool gsm_connect_server(void)
{
    read_IPpra(&bootloader_step.server_pra);
    return _gsm_connect_server(&bootloader_step.server_pra);
}


bool GSM_Init(void)  //初始化GSM
{
    u32 rate=gsm_checkbandrate();
    if(0==rate)
        {
            return false;
        }
    if(!gsm_save_rate(rate))
        {
            return false;
        }
    if(!gsm_CPIN())
        {
            return false;
        }

    if(!gsm_isregister())
        {
            return false;
        }
    if(!gsm_gprs())
        {
            return false;
        }
    if(!gsm_gprs_attach())
        {
            return false;
        }
    if(!gsm_setconnectmode())
        {
            return false;
        }
    if(!gsm_set_sendpra())
        {
            return false;
        }
    if(!gsm_connect_server())
        {
            return false;
        }
        return true;
}

//判断是不是GSM模块
u8 is_gsm(void)
{
    u8 __temp=0;
    char send_buf[20];
    memset(send_buf,0,sizeof(send_buf)/sizeof(send_buf[0]));
    sprintf(send_buf,"ATI\r");
    if(ATcheckreply((u8*)send_buf,"ANY\r",2000))
        {
            if(!strncmp((char*)buffer,"Quectel_Ltd",10))
                {
                    __temp=2;//GSM   modlus
                }
//    if(!strncmp((char*)buffer,"Quectel_Ltd",12))
//    {
//
//
//    }

        }

    return __temp;


}
