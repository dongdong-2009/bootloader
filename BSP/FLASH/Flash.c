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
    while(len--)
        {
            if(FLASH_ProgramHalfWord(addr,*buf)!=FLASH_COMPLETE)
                {
                    return 1;
                }
            addr+=2;
            buf++;
        }
    return 0;

}
bool flash_check(u32 addr,u16 * buf,u32 len)
{
	while(len--)
	{	
		if((*buf)!=flash_read_halfword(addr))
		{
			return 1;		
		}
		addr+=2;
		buf++;
	}
	
    return 0;
}

void Flash_Init(void)
{
    FLASH_Unlock();
//使用结束之后需要使用Flash_Lock();重新将flash置于上锁状态
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

