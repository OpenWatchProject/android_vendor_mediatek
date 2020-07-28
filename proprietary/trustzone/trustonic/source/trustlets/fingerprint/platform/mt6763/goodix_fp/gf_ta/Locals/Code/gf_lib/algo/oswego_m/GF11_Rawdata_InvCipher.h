/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF11_RAWDATA_INVCIPHER_H__
#define __GF11_RAWDATA_INVCIPHER_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern void GF11_AES_Config_Init(unsigned char* AES_Config_Seed,unsigned char AES_Config_Seed_Length,unsigned char* AES_Config_Value,unsigned char* AES_Config_Value_Length);

extern void GF11_Rawdata_AES_InvCipher(unsigned char* GF11_Rawdata, unsigned short Data_Length, unsigned char Rawdata_Cipher_Mode);

#ifdef __cplusplus
}
#endif

#endif // __GF11_RAWDATA_INVCIPHER_H__
