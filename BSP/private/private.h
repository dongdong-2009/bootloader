#ifndef __PRIVATE__H
#define __PRIVATE__H

#include "stm32f10x.h"
u16 crc_table(u8 *ptr,u16 len);

void addcrc(u8 *ptr,u16 len,u8 s);

u8 checkcrc(u8 *ptr,u16 len,u8 s);

u16 trans_7c_set(u8 *p_ad,u16 lengh);

u16 trans_7c_clr(u8 *p_ad,u16 lengh);

#endif
