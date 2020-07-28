/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __TRUSTZONE_TA_HDCP_RX__
#define __TRUSTZONE_TA_HDCP_RX__

#define TZ_TA_HDCP2_RX_UUID   "69384a92-d60e-4d2d-bc38-a0a7ab726938"

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for HDCP2_RX TA */
#define TZCMD_HDCP2_RX_TEST                 0
#define TZCMD_HDCP2_RX_SetEncDcp2Key        1
#define TZCMD_HDCP2_RX_GetCertInfo          2
#define TZCMD_HDCP2_RX_DecryptRSAESOAEP     3
#define TZCMD_HDCP2_RX_KdKeyDerivation      4
#define TZCMD_HDCP2_RX_ComputeHprime        5
#define TZCMD_HDCP2_RX_ComputeLprime        6
#define TZCMD_HDCP2_RX_ComputeKh            7
#define TZCMD_HDCP2_RX_EncryptKmUsingKh     8
#define TZCMD_HDCP2_RX_DecryptKmUsingKh     9
#define TZCMD_HDCP2_RX_DecryptEKs           10
#define TZCMD_HDCP2_RX_KsXorLC128           11
#define TZCMD_HDCP2_RX_DataDecrypt          12
#define TZCMD_HDCP2_RX_SetRiv               13
#define TZCMD_HDCP2_RX_GetDebugInfo         14
#define TZCMD_HDCP2_RX_GetDebugInfo2        15
#define TZCMD_HDCP2_RX_2_2KdKeyDerivation   16
#define TZCMD_HDCP2_RX_Compute2_2Hprime     17
#define TZCMD_HDCP2_RX_Compute2_2Lprime     18

typedef enum HDCP2_RX_KEY_SRC{
    CODED_FACSMILE_KEY = 0,
    CODED_LICENSE_KEY,
    DRM_KEY,
    EBD_KEY
} E_HDCP2_RX_KEY_SRC;

#endif /* __TRUSTZONE_TA_VDEC__ */

