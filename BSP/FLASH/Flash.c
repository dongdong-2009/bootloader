#include "Flash.h"
#include "stm32f10x_flash.h"


u16 flash_read_halfword(u32 addr)
{
    return *(u16*)addr;
}

u8 flash_read_char(u32 addr)
{
    return *(u8*)addr;
}

bool flash_write(u32 addr, u16 *buf,u32 len)
{
    for(u32 i=0; i<len; i++)
        {
            if(FLASH_ProgramHalfWord(addr,*buf)!=FLASH_COMPLETE)
                {
                    return false;
                }
//            FLASH_ProgramHalfWord(addr,*buf);
            addr+=2;
            buf++;
        }
    return true;
}


void writeFlash(u32 WriteAddr, u8 *pBuffer,u16 len)
{

    for(u16 i = 0; i< (len >> 1); i++)
        {
            FLASH_ProgramHalfWord(WriteAddr+i*2,((u16)(*(pBuffer+2*i+1)<<8))|(*(pBuffer+2*i)));
        }

}

bool flash_check(u32 addr,u16 * buf,u32 len)
{
    while(len--)
        {
            if((*buf)!=flash_read_halfword(addr))
                {
                    return false;
                }
            addr+=2;
            buf++;
        }
    return true;
}

void Flash_Init(void)
{
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}


bool write_flage(u32 addr,u32 specify,u16 value)
{
    const u8 len=100;
    u16 temp_flag[len];
    u32 temp_addr=addr;
    u8 index = (specify - addr)>>1;
    for(u16 i=0; i<len; i++)
        {
            temp_flag[i]=flash_read_halfword(addr);
            addr+=2;
        }
    temp_flag[index] = value;
    FLASH_ErasePage(temp_addr);
        
    bool st=flash_write(temp_addr,temp_flag,len);
    return st;
}
