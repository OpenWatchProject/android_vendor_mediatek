/*------------------------------------------------------------------------------
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 *----------------------------------------------------------------------------*/

#ifndef __MK_DEBUG_H__
#define __MK_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MK_FATAL    0x0001
#define MK_ERROR    0x0002
#define MK_WARN     0x0004
#define MK_DEBUG    0x0008
#define MK_INFO     0x0010
#define MK_VERBOSE  0x0020

typedef enum
{
    FLAGS_ENABLE_PROFILING     = 0,   ///< Use systrace to profiling
    FLAGS_DEBUG_BIT_TRUE       = 1,   ///< to check bit true
    FLAGS_EXTERNAL_MEMORY      = 2,   ///<
    FLAGS_MAX,
}EN_FLAGS;

#if defined(_RELEASE)
#define MKPrintf(DbgLvl, ...)
#define MKTrace()
#else
#define MKPrintf(DbgLvl, ...) MKDebug_Printf(DbgLvl, __FILE__, __LINE__, __VA_ARGS__)
#define MKTrace() MKDebug_Printf(MK_VERBOSE, __FILE__, __LINE__, __func__)
#endif
#define ERR_IF_NULL_RETURN_VALUE(p,v)    if(!p){ MKPrintf(MK_ERROR,"invalid parameter"); return v; }
#define CTL_FLAG(f)    MKDebug_Flags(f, NULL)

void MKDebug_Printf(
    unsigned long uDebugLevel,
    const char* szFileName,
    unsigned long uLine,
    const char* szFormat,
    ...);
void MKDebug_SetLevel(unsigned long level);
void MKDebug_SetFlags(unsigned long flags);
bool MKDebug_Flags(EN_FLAGS eFlags, void **ppValue);

#ifdef __cplusplus
}
#endif

#endif  /* __MK_DEBUG_H__ */
