#ifndef __AT_CMD__H
#define __AT_CMD__H
#include "string.h"
#include "uart4.h"
#include "bsp_SysTick.h"
#include "stdbool.h"

void ATCommand(u8* cmd);

bool ATcheckreply(u8* cmd,u8* reply,u16 timeout);

bool rec_send_none(u8* reply,u16 timeout);

#endif

