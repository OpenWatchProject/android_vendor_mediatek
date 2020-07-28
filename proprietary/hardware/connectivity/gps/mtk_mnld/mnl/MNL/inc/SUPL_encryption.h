/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2008]. All rights reserved.
*
*************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   SUPL_encryption.h
 *
 * Description:
 * ------------
 *   Prototype of encryption/decryption function for SUPL PMTK command
 *
 ****************************************************************************/

#ifndef SUPL_ENCRYPTION_H
#define SUPL_ENCRYPTION_H

int SUPL_encrypt(unsigned char *plain, unsigned char *cipher, unsigned int length);
int SUPL_decrypt(unsigned char *plain, unsigned char *cipher, unsigned int length);

#endif

