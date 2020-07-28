/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _DBG_AAA_PARAM_H_
#define _DBG_AAA_PARAM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 3A debug info
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AAATAG(module_id, tag, line_keep)   \
( (MINT32)                                  \
  ((MUINT32)(0x00000000) |                  \
   (MUINT32)((module_id & 0xff) << 24) |    \
   (MUINT32)((line_keep & 0x01) << 23) |    \
   (MUINT32)(tag & 0xffff))                 \
)

#define MODULE_NUM(total_module, tag_module)      \
((MINT32)                                         \
 ((MUINT32)(0x00000000) |                         \
  (MUINT32)((total_module & 0xff) << 16) |        \
  (MUINT32)(tag_module & 0xff))                   \
)

typedef struct
{
    MUINT32 u4FieldID;
    MUINT32 u4FieldValue;
}  AAA_DEBUG_TAG_T;

#define MAXIMUM_AAA_DEBUG_COMM_SIZE 32
typedef union
{
    struct  {
        MUINT32 chkSum;
        MUINT32 ver;
    };
    MUINT8   Data[MAXIMUM_AAA_DEBUG_COMM_SIZE];
} AAA_DEBUG_COMM_T;
static_assert( sizeof(AAA_DEBUG_COMM_T) == MAXIMUM_AAA_DEBUG_COMM_SIZE,
        "AAA_DEBUG_COM_T size mismatch");

#include "dbg_ae_param.h"
#include "dbg_af_param.h"
#include "dbg_awb_param.h"
#include "dbg_flash_param.h"
#include "dbg_flicker_param.h"
#include "dbg_shading_param.h"
#include "dbg_isp_param.h"
//
#define AAA_DEBUG_AE_MODULE_ID          0x6001
#define AAA_DEBUG_AF_MODULE_ID          0x6002
#define AAA_DEBUG_AWB_MODULE_ID         0x6003
#define AAA_DEBUG_FLASH_MODULE_ID       0x6004
#define AAA_DEBUG_FLICKER_MODULE_ID     0x6005
#define AAA_DEBUG_SHADING_MODULE_ID     0x6006
#define AAA_DEBUG_AWB_DATA_MODULE_ID    0x6007
#define AAA_DEBUG_AE_PLINE_MODULE_ID    0x6008
#define AAA_DEBUG_SHADTBL2_MODULE_ID    0x6009
//

typedef struct
{
    MUINT32           u4Size;
    AAA_DEBUG_COMM_T  rAE;
    AAA_DEBUG_COMM_T  rAF;
    AAA_DEBUG_COMM_T  rFLASH;
    AAA_DEBUG_COMM_T  rFLICKER;
    AAA_DEBUG_COMM_T  rSHADING;
} COMMON_DEBUG_INFO1_T;

typedef struct
{
    MUINT32           u4Size;
    AAA_DEBUG_COMM_T  rAWB;
    AAA_DEBUG_COMM_T  rISP;
} COMMON_DEBUG_INFO2_T;

typedef struct
{
    struct Header
    {
        MUINT32  u4KeyID;
        MUINT32  u4ModuleCount;
        MUINT32  u4AEDebugInfoOffset;
        MUINT32  u4AFDebugInfoOffset;
        MUINT32  u4FlashDebugInfoOffset;
        MUINT32  u4FlickerDebugInfoOffset;
        MUINT32  u4ShadingDebugInfoOffset;
        COMMON_DEBUG_INFO1_T  rCommDebugInfo;
    } hdr;

    // TAG
    AE_DEBUG_INFO_T       rAEDebugInfo;
    AF_DEBUG_INFO_T       rAFDebugInfo;
    FLASH_DEBUG_INFO_T    rFlashDebugInfo;
    FLICKER_DEBUG_INFO_T  rFlickerDebugInfo;
    SHADING_DEBUG_INFO_T  rShadigDebugInfo;

} AAA_DEBUG_INFO1_T;

typedef struct
{
    struct Header
    {
        MUINT32  u4KeyID;
        MUINT32  u4ModuleCount;
        MUINT32  u4AWBDebugInfoOffset;
        MUINT32  u4ISPDebugInfoOffset;
        MUINT32  u4ISPP1RegDataOffset;
        MUINT32  u4ISPP2RegDataOffset;
        MUINT32  u4MFBRegInfoOffset;
        MUINT32  u4AWBDebugDataOffset;
        COMMON_DEBUG_INFO2_T  rCommDebugInfo;
    } hdr;

    // TAG
    AWB_DEBUG_INFO_T rAWBDebugInfo;

    // None TAG
    // ISP Tag, P1 Table, P2 Table, MFB Table
    NSIspExifDebug::IspExifDebugInfo_T rISPDebugInfo;
    // AWB debug Table (Struct)
    AWB_DEBUG_DATA_T rAWBDebugData;

} AAA_DEBUG_INFO2_T;

#define DEFAULT_DATA (0xFF7C)
static_assert( sizeof(AAA_DEBUG_INFO1_T) <= DEFAULT_DATA, "Debug Info exceed EXIF limitation, please discuss with EXIF and Debug Parser owner for solution!" );
static_assert( sizeof(AAA_DEBUG_INFO2_T) <= DEFAULT_DATA, "Debug Info exceed EXIF limitation, please discuss with EXIF and Debug Parser owner for solution!" );

#endif // _DBG_AAA_PARAM_H_

