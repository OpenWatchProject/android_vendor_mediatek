/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef VCODECCAP_GENHEADER_H
#define VCODECCAP_GENHEADER_H

#include <linux/ion.h>

typedef void                VAL_VOID_T;         /* /< void type definition */
typedef char                VAL_BOOL_T;         /* /< char type definition */
typedef char                VAL_CHAR_T;         /* /< char type definition */
typedef signed char         VAL_INT8_T;         /* /< signed char type definition */
typedef signed short        VAL_INT16_T;        /* /< signed short type definition */
typedef signed int         VAL_INT32_T;        /* /< signed int type definition */
typedef unsigned char       VAL_UCHAR_T;        /* /< unsigned char type definition */
typedef unsigned char       VAL_UINT8_T;        /* /< unsigned char type definition */
typedef unsigned short      VAL_UINT16_T;       /* /< unsigned short definition */
typedef unsigned int       VAL_UINT32_T;       /* /< unsigned int type definition */
typedef unsigned long long  VAL_UINT64_T;       /* /< unsigned long long type definition */
typedef long long           VAL_INT64_T;        /* /< long long type definition */
typedef unsigned long       VAL_HANDLE_T;       /* /< unsigned int (handle) type definition */
typedef signed long         VAL_LONG_T;       /* / */
typedef unsigned long       VAL_ULONG_T;       /* / */
typedef unsigned long long  VAL_ADDRESS_T;

#define VAL_NULL        (0)                     /* /< VAL_NULL = 0 */
#define VAL_TRUE        (1)                     /* /< VAL_TRUE = 1 */
#define VAL_FALSE       (0)                     /* /< VAL_FALSE = 0 */

/**
 * @par Enumeration
 *   VENC_DRV_H264_VIDEO_PROFILE_T
 * @par Description
 *   This is the item used for h.264 encoder profile capability
 */
typedef enum __VENC_DRV_H264_VIDEO_PROFILE_T {
    VENC_DRV_H264_VIDEO_PROFILE_UNKNOWN              = 0,           /* /< Default value (not used) */
    VENC_DRV_H264_VIDEO_PROFILE_BASELINE             = (1 << 0),    /* /< Baseline */
    VENC_DRV_H264_VIDEO_PROFILE_CONSTRAINED_BASELINE = (1 << 1),    /* /< Constrained Baseline */
    VENC_DRV_H264_VIDEO_PROFILE_MAIN                 = (1 << 2),    /* /< Main */
    VENC_DRV_H264_VIDEO_PROFILE_EXTENDED             = (1 << 3),    /* /< Extended */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH                 = (1 << 4),    /* /< High */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH_10              = (1 << 5),    /* /< High 10 */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH422              = (1 << 6),    /* /< High 422 */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH444              = (1 << 7),    /* /< High 444 */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH_10_INTRA        = (1 << 8),    /* /< High 10 Intra (Amendment 2) */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH422_INTRA        = (1 << 9),    /* /< High 422 Intra (Amendment 2) */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH444_INTRA        = (1 << 10),   /* /< High 444 Intra (Amendment 2) */
    VENC_DRV_H264_VIDEO_PROFILE_CAVLC444_INTRA       = (1 << 11),   /* /< CAVLC 444 Intra (Amendment 2) */
    VENC_DRV_H264_VIDEO_PROFILE_HIGH444_PREDICTIVE   = (1 << 12),   /* /< High 444 Predictive (Amendment 2) */
    VENC_DRV_H264_VIDEO_PROFILE_SCALABLE_BASELINE    = (1 << 13),   /* /< Scalable Baseline (Amendment 3) */
    VENC_DRV_H264_VIDEO_PROFILE_SCALABLE_HIGH        = (1 << 14),   /* /< Scalable High (Amendment 3) */
    VENC_DRV_H264_VIDEO_PROFILE_SCALABLE_HIGH_INTRA  = (1 << 15),   /* /< Scalable High Intra (Amendment 3) */
    VENC_DRV_H264_VIDEO_PROFILE_MULTIVIEW_HIGH       = (1 << 16),   /* /< Multiview High (Corrigendum 1 (2009)) */
    VENC_DRV_H264_VIDEO_PROFILE_MAX                  = 0xFFFFFFFF   /* /< Max VENC_DRV_H264_VIDEO_PROFILE_T value */
} VENC_DRV_H264_VIDEO_PROFILE_T;

typedef enum __VENC_DRV_HEVC_VIDEO_PROFILE_T {
    VENC_DRV_HEVC_VIDEO_PROFILE_UNKNOWN              = 0,           /* /< Default value (not used) */
    VENC_DRV_HEVC_VIDEO_PROFILE_BASELINE             = (1 << 0),    /* /< Baseline */
    VENC_DRV_HEVC_VIDEO_PROFILE_CONSTRAINED_BASELINE = (1 << 1),    /* /< Constrained Baseline */
    VENC_DRV_HEVC_VIDEO_PROFILE_MAIN                 = (1 << 2),    /* /< Main */
    VENC_DRV_HEVC_VIDEO_PROFILE_EXTENDED             = (1 << 3),    /* /< Extended */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH                 = (1 << 4),    /* /< High */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH_10              = (1 << 5),    /* /< High 10 */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH422              = (1 << 6),    /* /< High 422 */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH444              = (1 << 7),    /* /< High 444 */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH_10_INTRA        = (1 << 8),    /* /< High 10 Intra (Amendment 2) */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH422_INTRA        = (1 << 9),    /* /< High 422 Intra (Amendment 2) */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH444_INTRA        = (1 << 10),   /* /< High 444 Intra (Amendment 2) */
    VENC_DRV_HEVC_VIDEO_PROFILE_CAVLC444_INTRA       = (1 << 11),   /* /< CAVLC 444 Intra (Amendment 2) */
    VENC_DRV_HEVC_VIDEO_PROFILE_HIGH444_PREDICTIVE   = (1 << 12),   /* /< High 444 Predictive (Amendment 2) */
    VENC_DRV_HEVC_VIDEO_PROFILE_SCALABLE_BASELINE    = (1 << 13),   /* /< Scalable Baseline (Amendment 3) */
    VENC_DRV_HEVC_VIDEO_PROFILE_SCALABLE_HIGH        = (1 << 14),   /* /< Scalable High (Amendment 3) */
    VENC_DRV_HEVC_VIDEO_PROFILE_SCALABLE_HIGH_INTRA  = (1 << 15),   /* /< Scalable High Intra (Amendment 3) */
    VENC_DRV_HEVC_VIDEO_PROFILE_MULTIVIEW_HIGH       = (1 << 16),   /* /< Multiview High (Corrigendum 1 (2009)) */
    VENC_DRV_HEVC_VIDEO_PROFILE_MAX                  = 0xFFFFFFFF   /* /< Max VENC_DRV_HEVC_VIDEO_PROFILE_T value */
} VENC_DRV_HEVC_VIDEO_PROFILE_T;

/**
 * @par Enumeration
 *   VENC_DRV_VIDEO_LEVEL_T
 * @par Description
 *   This is the item used for encoder level capability
 */
typedef enum __VENC_DRV_VIDEO_LEVEL_T {
    VENC_DRV_VIDEO_LEVEL_UNKNOWN = 0,       /* /< Default value (not used) */
    VENC_DRV_VIDEO_LEVEL_0,                 /* /< VC1 */
    VENC_DRV_VIDEO_LEVEL_1,                 /* /< H264, HEVC, VC1, MPEG4 */
    VENC_DRV_VIDEO_LEVEL_1b,                /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_1_1,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_1_2,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_1_3,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_2,                 /* /< H264, HEVC, VC1, MPEG4 */
    VENC_DRV_VIDEO_LEVEL_2_1,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_2_2,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_3,                 /* /< H264, HEVC, VC1, MPEG4 */
    VENC_DRV_VIDEO_LEVEL_3_1,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_3_2,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_4,                 /* /< H264, HEVC, VC1 */
    VENC_DRV_VIDEO_LEVEL_4_1,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_4_2,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_5,                 /* /< H264, HEVC, HEVC */
    VENC_DRV_VIDEO_LEVEL_5_1,               /* /< H264, HEVC */
    VENC_DRV_VIDEO_LEVEL_LOW,               /* /< VC1, MPEG2 */
    VENC_DRV_VIDEO_LEVEL_MEDIUM,            /* /< VC1, MPEG2 */
    VENC_DRV_VIDEO_LEVEL_HIGH1440,          /* /< MPEG2 */
    VENC_DRV_VIDEO_LEVEL_HIGH,              /* /< VC1, MPEG2 */
    VENC_DRV_VIDEO_LEVEL_6,                 /* /< H263 */
    VENC_DRV_VIDEO_LEVEL_7,               /* /< H263 */
    VENC_DRV_VIDEO_LEVEL_MAX = 0xFFFFFFFF   /* /< Max VENC_DRV_VIDEO_LEVEL_T value */
} VENC_DRV_VIDEO_LEVEL_T;

/**
 * @par Structure
 *  VAL_MEM_ADDR_T
 * @par Description
 *  This is a structure for memory address
 */
typedef struct _VAL_MEM_ADDR_T {                 /* union extend 64bits for TEE*/
    union {
        VAL_ULONG_T    u4VA;                       /* /< [IN/OUT] virtual address */
        VAL_UINT64_T u4VA_ext64;
    };
    union {
        VAL_ULONG_T    u4PA;                       /* /< [IN/OUT] physical address */
        VAL_UINT64_T u4PA_ext64;
    };
    union {
        VAL_ULONG_T    u4Size;                     /* /< [IN/OUT] size */
        VAL_UINT64_T u4Size_ext64;
    };
} VAL_MEM_ADDR_T;

/**
 * @par Structure
 *   VENC_DRV_TIMESTAMP_T
 * @par Description
 *   This is timestamp information and used as items for VENC_DRV_PARAM_FRM_BUF_T and VENC_DRV_PARAM_BS_BUF_T
 */
typedef struct __VENC_DRV_TIMESTAMP_T {
    VAL_UINT32_T    u4TimeStamp[2];     /* /< [IN] Timestamp information */
} VENC_DRV_TIMESTAMP_T;

/**
 * @par Structure
 *   VENC_DRV_PARAM_BS_BUF_T
 * @par Description
 *   This is bitstream buffer information and used as input parameter for\n
 *   eVEncDrvEncode()\n
 */
typedef struct __VENC_DRV_PARAM_BS_BUF_T {/*union extend 64bits for TEE */
    VAL_MEM_ADDR_T          rBSAddr;        /* /< [IN] Bitstream buffer address */
    union {
        VAL_ULONG_T         u4BSStartVA;    /* /< [IN] Bitstream fill start address */
        VAL_UINT64_T        u4BSStartVA_ext64;
    };
    union {
        VAL_ULONG_T         u4BSSize;       /* /< [IN] Bitstream size (filled bitstream in bytes) */
        VAL_UINT64_T        u4BSSize_ext64;
    };
    VENC_DRV_TIMESTAMP_T    rTimeStamp;     /* /< [IN] Time stamp information */
    VAL_UINT32_T            rSecMemHandle;  /* /< [IN/OUT] security memory handle for SVP */
    union {
        ion_user_handle_t pIonBufhandle;    /* /< [IN/OUT] ion handle */
        VAL_UINT64_T pIonBufhandle_ext64;
    };
    VAL_UINT32_T            u4IonDevFd;     /* /< [IN/OUT] ion device fd */
} VENC_DRV_PARAM_BS_BUF_T;


VAL_VOID_T enc_putbits(VAL_UINT8_T *in, VAL_INT32_T *bitcnt, VAL_INT32_T data, VAL_INT32_T data_length);
VAL_VOID_T enc_unsigned_exp_golomb(VAL_INT32_T ue, VAL_INT32_T *len, VAL_INT32_T *info);
VAL_INT32_T enc_log2ceil(VAL_INT32_T arg);


VAL_UINT32_T H264_GetProfile(VAL_UINT32_T u4Profile);
VAL_UINT32_T H264_GetLevel(VAL_UINT32_T u4Level, VAL_UINT32_T u4Profile);
VAL_UINT32_T H264_GetVencDpbSize(VAL_UINT32_T u4H264Level, VAL_UINT32_T u4ImageWidthInPels, VAL_UINT32_T u4ImageHeightInPels);
VAL_BOOL_T H264_EncodeSPS_SW(
        VENC_DRV_H264_VIDEO_PROFILE_T eProfile, VENC_DRV_VIDEO_LEVEL_T eLevel, VAL_UINT32_T width, VAL_UINT32_T height, VAL_UINT32_T prefer_WH_ratio, VENC_DRV_PARAM_BS_BUF_T* headerBitStreamBuf,
                             int32_t* headerSize);
VAL_BOOL_T H264_EncodePPS_SW(
        VENC_DRV_H264_VIDEO_PROFILE_T eProfile, VENC_DRV_PARAM_BS_BUF_T* headerBitStreamBuf, int32_t* headerSize);
VAL_BOOL_T H264_EncodeSPS_SW_VUI(
        VAL_UINT32_T width, VAL_UINT32_T height, VAL_UINT32_T prefer_WH_ratio, VAL_UINT8_T *pucBSAddrVA, VAL_INT32_T *pBitcnt);
VAL_BOOL_T H264_EncodeSPS_SW_Crop(
        VAL_UINT32_T width, VAL_UINT32_T height, VAL_UINT32_T prefer_WH_ratio, VAL_UINT8_T *pucBSAddrVA, VAL_INT32_T *pBitcnt);


VAL_UINT32_T HEVC_GetGeneralLevel_idc(VAL_UINT32_T u4Level);
VAL_BOOL_T HEVC_EncodeSPS_SW(VENC_DRV_VIDEO_LEVEL_T eLevel, VAL_UINT32_T width, VAL_UINT32_T height, VAL_UINT32_T prefer_WH_ratio,  VENC_DRV_PARAM_BS_BUF_T* headerBitStreamBuf,
                             int32_t* headerSize);
VAL_BOOL_T HEVC_EncodePPS_SW(VENC_DRV_PARAM_BS_BUF_T* headerBitStreamBuf, int32_t* headerSize);
VAL_BOOL_T HEVC_EncodeVPS_SW(VENC_DRV_VIDEO_LEVEL_T eLevel, VENC_DRV_PARAM_BS_BUF_T* headerBitStreamBuf, int32_t* headerSize);


#endif

