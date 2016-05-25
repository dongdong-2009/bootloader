#include <stdio.h>
#include "crypto.h"

/** @addtogroup STM32_Crypto_Examples
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Key to be used for AES encryption/decryption */
uint8_t Key[CRL_AES128_KEY] = {
    0xa0, 0x5a, 0x93, 0x83, 0x46, 0xb4, 0x56, 0xb8,
    0xe2, 0x55, 0x4e, 0xfa, 0x33, 0x55, 0x91, 0x57
};
//a0 5a 93 83 46 b4 56 b8 e2 55 4e fa 33 55 91 57

/* Initialization Vector, used only in non-ECB modes */
uint8_t IV[CRL_AES_BLOCK] = {
    0xf0 , 0xf1 , 0xf2 , 0xf3 , 0xf4 , 0xf5 , 0xf6 , 0xf7,
    0xf8 , 0xf9 , 0xfa , 0xfb , 0xfc , 0xfd , 0xfe , 0xff
};


/* Buffer to store the output data */
//uint8_t OutputMessage[PLAINTEXT_LENGTH];
uint8_t OutputMessage[cryptLen]= {0};

/* Size of the output data */
uint32_t OutputMessageLength = 0;

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


/**
  * @brief  AES CTR Encryption example.
  * @param  InputMessage: pointer to input message to be encrypted.
  * @param  InputMessageLength: input data message length in byte.
  * @param  AES128_Key: pointer to the AES key to be used in the operation
  * @param  InitializationVector: pointer to the Initialization Vector (IV)
  * @param  IvLength: IV length in bytes.
  * @param  OutputMessage: pointer to output parameter that will handle the encrypted message
  * @param  OutputMessageLength: pointer to encrypted message length.
  * @retval error status: can be AES_SUCCESS if success or one of
  *         AES_ERR_BAD_CONTEXT, AES_ERR_BAD_PARAMETER, AES_ERR_BAD_OPERATION
  *         if error occured.
  */
int32_t STM32_AES_CTR_Encrypt(uint8_t* InputMessage,
                              uint32_t InputMessageLength,
                              uint8_t  *AES128_Key,
                              uint8_t  *InitializationVector,
                              uint32_t  IvLength,
                              uint8_t  *OutputMessage,
                              uint32_t *OutputMessageLength)
{
    AESCTRctx_stt AESctx;

    uint32_t error_status = AES_SUCCESS;

    int32_t outputLength = 0;

    /* DeInitialize STM32 Cryptographic Library */
    Crypto_DeInit();

    /* Set flag field to default value */
    AESctx.mFlags = E_SK_DEFAULT;

    /* Set key size to 16 (corresponding to AES-128) */
    AESctx.mKeySize = 16;

    /* Set iv size field to IvLength*/
    AESctx.mIvSize = IvLength;



    /* Initialize the operation, by passing the key.
     * Third parameter is NULL because CTR doesn't use any IV */
    error_status = AES_CTR_Encrypt_Init(&AESctx, AES128_Key, InitializationVector );

    /* check for initialization errors */
    if (error_status == AES_SUCCESS) {
        /* Encrypt Data */
        error_status = AES_CTR_Encrypt_Append(&AESctx,
                                              InputMessage,
                                              InputMessageLength,
                                              OutputMessage,
                                              &outputLength);

        if (error_status == AES_SUCCESS) {
            /* Write the number of data written*/
            *OutputMessageLength = outputLength;
            /* Do the Finalization */
            error_status = AES_CTR_Encrypt_Finish(&AESctx, OutputMessage + *OutputMessageLength, &outputLength);
            /* Add data written to the information to be returned */
            *OutputMessageLength += outputLength;
        }
    }

    return error_status;
}


/**
  * @brief  AES CTR Decryption example.
  * @param  InputMessage: pointer to input message to be decrypted.
  * @param  InputMessageLength: input data message length in byte.
  * @param  AES128_Key: pointer to the AES key to be used in the operation
  * @param  InitializationVector: pointer to the Initialization Vector (IV)
  * @param  IvLength: IV length in bytes.
  * @param  OutputMessage: pointer to output parameter that will handle the decrypted message
  * @param  OutputMessageLength: pointer to decrypted message length.
  * @retval error status: can be AES_SUCCESS if success or one of
  *         AES_ERR_BAD_CONTEXT, AES_ERR_BAD_PARAMETER, AES_ERR_BAD_OPERATION
  *         if error occured.
  */
int32_t STM32_AES_CTR_Decrypt(uint8_t* InputMessage,
                              uint32_t InputMessageLength,
                              uint8_t  *AES128_Key,
                              uint8_t  *InitializationVector,
                              uint32_t  IvLength,
                              uint8_t  *OutputMessage,
                              uint32_t *OutputMessageLength)
{
    AESCTRctx_stt AESctx;

    uint32_t error_status = AES_SUCCESS;

    int32_t outputLength = 0;
    /* DeInitialize STM32 Cryptographic Library */
    Crypto_DeInit();

    /* Set flag field to default value */
    AESctx.mFlags = E_SK_DEFAULT;

    /* Set key size to 16 (corresponding to AES-128) */
    AESctx.mKeySize = 16;

    /* Set iv size field to IvLength*/
    AESctx.mIvSize = IvLength;

    /* Initialize the operation, by passing the key.
     * Third parameter is NULL because CTR doesn't use any IV */
    error_status = AES_CTR_Decrypt_Init(&AESctx, AES128_Key, InitializationVector );

    /* check for initialization errors */
    if (error_status == AES_SUCCESS) {
        /* Decrypt Data */
        error_status = AES_CTR_Decrypt_Append(&AESctx,
                                              InputMessage,
                                              InputMessageLength,
                                              OutputMessage,
                                              &outputLength);

        if (error_status == AES_SUCCESS) {
            /* Write the number of data written*/
            *OutputMessageLength = outputLength;
            /* Do the Finalization */
            error_status = AES_CTR_Decrypt_Finish(&AESctx, OutputMessage + *OutputMessageLength, &outputLength);
            /* Add data written to the information to be returned */
            *OutputMessageLength += outputLength;
        }
    }

    return error_status;
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer, pBuffer1: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer identical to pBuffer1
  *         FAILED: pBuffer differs from pBuffer1
  */
TestStatus Buffercmp(const uint8_t* pBuffer, uint8_t* pBuffer1, uint16_t BufferLength)
{
    while (BufferLength--) {
        if (*pBuffer != *pBuffer1) {
            return FAILED;
        }

        pBuffer++;
        pBuffer1++;
    }

    return PASSED;
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
