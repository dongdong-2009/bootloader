#ifndef __FLASH__H
#define __FLASH__H
#include "stm32f10x.h"
#include "stdbool.h"

u8 flash_read_char(u32 addr) ;
u16 flash_read_halfword(u32 addr);
bool flash_write(u32 addr, u16 *buf,u32 len);
bool flash_check(u32 addr,u16 * buf,u32 len);
void Flash_Init(void);
void writeFlash(u32 WriteAddr, u8 *pBuffer,u16 len);
bool write_flage(u32 addr,u32 specify,u16 value);
#endif
