#ifndef __M26__H
#define __M26__H
#include "string.h"
#include "uart4.h"
#include "bsp_SysTick.h"
#include "stdbool.h"
#include "AT_cmd.h"
#include "stdio.h"

u32 gsm_checkbandrate(void);
bool gsm_save_rate(u32 rate);
bool gsm_isregister(void);
u8 gsm_signal(void);
bool gsm_gprs(void);
bool gsm_CPIN(void);
bool gsm_setconnectmode(void);
bool gsm_gprs_attach(void);
bool gsm_set_sendpra(void);
bool gsm_connect_server(void);
u8 is_gsm(void);
bool GSM_Init(void);  //≥ı ºªØGSM;
#endif
