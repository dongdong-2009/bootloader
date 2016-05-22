#ifndef __Cryptographic__H
#define __Cryptographic__H
#include "stm32f10x.h"

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private function prototypes -----------------------------------------------*/
int32_t STM32_AES_CTR_Encrypt(uint8_t*  InputMessage,
                        uint32_t  InputMessageLength,
                        uint8_t  *AES128_Key,
                        uint8_t  *InitializationVector,
                        uint32_t  IvLength,
                        uint8_t  *OutputMessage,
                        uint32_t *OutputMessageLength);

int32_t STM32_AES_CTR_Decrypt(uint8_t*  InputMessage,
                        uint32_t  InputMessageLength,
                        uint8_t  *AES128_Key,
                        uint8_t  *InitializationVector,
                        uint32_t  IvLength,
                        uint8_t  *OutputMessage,
                        uint32_t *OutputMessageLength);

TestStatus Buffercmp(const uint8_t* pBuffer,
                     uint8_t* pBuffer1,
                     uint16_t BufferLength);


void addAES(u8 *p,u16 len);
void deAES(u8 *p,u16 len);

#endif
