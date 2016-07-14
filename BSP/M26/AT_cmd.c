#include "AT_cmd.h"
extern _bootloader_type bootloader_step;


void ATCommand(u8* cmd)
{
    do
        {
            UART4_SEND_CHAR(*cmd++);
        }
    while(*(cmd-1)!='\r');
}

u16 get_len(u8* buf)
{
    for(u16 i=0; i<100; i++)
        {
            if(*(buf+i)=='\r')
                {
                    return i+1;
                }
        }
    return 0;
}


bool ATcheckreply(u8* cmd,u8* reply,u16 timeout)
{
    bool check_sta=false;
    u16 reply_len=get_len(reply);
    bootloader_step.sta=init;
    
    Delay_us(1000*10);
    before_send_uart4();
    ATCommand(cmd);
    if(0==strncmp((char*)reply,"NONE",4))
        {
            check_sta=true;
        }
    else if(0==strncmp((char*)reply,"ANY",3))
        {
            check_sta = delay(timeout);
        }
    else
        {
            bool sta=delay(timeout);
            if(sta==true)
                {
                    if(0==strncmp((char*)buffer,(char*)reply,reply_len))
                        {
                            check_sta=true;
                        }
                    else
                        {
                            check_sta=false;
                        }

                }
            else
                {
                    check_sta=false;
                }

        }

    bootloader_step.sta=normal;
    return check_sta;
}

bool rec_send_none(u8* reply,u16 timeout)
{
    bool check_sta=false;
    u16 reply_len=get_len(reply);
    bootloader_step.sta=init;
    before_send_uart4();
    if(0==strncmp((char*)reply,"ANY",3))
        {
            check_sta = delay(timeout);
        }
    else
        {
            bool sta=delay(timeout);
            if(sta==true)
                {
                    if(0==strncmp((char*)buffer,(char*)reply,reply_len))
                        {
                            check_sta=true;
                        }
                    else
                        {
                            check_sta=false;
                        }

                }
            else
                {
                    check_sta=false;
                }

        }

    bootloader_step.sta=normal;
    return check_sta;
}

