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

#include "camera_custom_mvhdr3expo_flicker.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <string.h>

#define LOG_TAG "camera_custom_mvhdr3expo_flicker"

#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define MY_LOGD(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE(fmt, ##arg)

#define MY_LOG_IF(cond, ...)    do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

MUINT32 getMVHDR3ExpoFLKBufSize()
{
    return (MVHDR3EXPOFLK_WIDTH * MVHDR3EXPOFLK_HEIGHT); // FLK_BUFF
}

MVOID decodeMVHDR3ExpoFLKStatistic(MVOID *pFLKStatPointer, MVOID *pOutputDataPointer, MUINT32 &u4MVHDRFLK_Mode)
{
    if( NULL == pFLKStatPointer || NULL == pOutputDataPointer )
    {
        MY_LOGE("[%s] In/Out Buffer is invalid", __FUNCTION__);
        return;
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mvhdr3expoflk.custom.enable", value, "0");
    MBOOL bDebugEnable = (MBOOL)atoi(value);

    memset(pOutputDataPointer, 0, getMVHDR3ExpoFLKBufSize() * sizeof(MUINT32));

    MY_LOG_IF(bDebugEnable, "[%s] stat W/H: %d/%d u4MVHDRFLK_Mode: %d", __FUNCTION__, MVHDR3EXPOFLK_WIDTH, MVHDR3EXPOFLK_HEIGHT, u4MVHDRFLK_Mode);

    MUINT32 *pOutput     = reinterpret_cast<MUINT32 *>(pOutputDataPointer);
    char    *pFLKStat    = reinterpret_cast<char *>(pFLKStatPointer);

    for (MUINT32 u4FLKIdx = 0; u4FLKIdx < (MVHDR3EXPOFLK_WIDTH * MVHDR3EXPOFLK_HEIGHT); u4FLKIdx ++) { // initialization
        pOutput[u4FLKIdx] = 0;
    }

    /* DECODE */
    for (MUINT32 u4FLKIdx = 0, u4FLKOutIdx = 0; u4FLKIdx < MVHDR3EXPOFLK_OUTSIZE; u4FLKIdx +=5) {
        pOutput[u4FLKOutIdx++] = (
        //((pFLKStat[u4FLKIdx+2] & 0x0E ) >> 2 ) |// 1~0
        //((pFLKStat[u4FLKIdx  ] >>   2 ) << 2 ) |// 7~2
        //((pFLKStat[u4FLKIdx+1] & 0x03 ) << 8 )  // 8~9
        ((pFLKStat[u4FLKIdx  ]        ) << 2 ) |// 9:2
        ((pFLKStat[u4FLKIdx+1] & 0xc0 ) >> 6 )  // 1:0
        );

        pOutput[u4FLKOutIdx++] = (
        ((pFLKStat[u4FLKIdx+2]        ) << 2 ) |// 9:2
        ((pFLKStat[u4FLKIdx+3] & 0xc0 ) >> 6 )  // 1:0
        );

        MY_LOG_IF(bDebugEnable, "[%s] pFLKStat[%03d-%03d] %d,%d,%d,%d,%d", __FUNCTION__, 
        u4FLKIdx, u4FLKIdx+4, 
        pFLKStat[u4FLKIdx], 
        pFLKStat[u4FLKIdx + 1], 
        pFLKStat[u4FLKIdx + 2], 
        pFLKStat[u4FLKIdx + 3], 
        pFLKStat[u4FLKIdx + 4]);
        MY_LOG_IF(bDebugEnable, "[%s] pOutput[%03d-%03d] %d,%d", __FUNCTION__, u4FLKOutIdx-2, u4FLKOutIdx-1, pOutput[u4FLKOutIdx - 2], pOutput[u4FLKOutIdx - 1]);
    }
}
