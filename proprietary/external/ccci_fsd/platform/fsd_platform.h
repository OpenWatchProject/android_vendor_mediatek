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

#ifndef __FSD_PLATFORM_H__
#define __FSD_PLATFORM_H__
#include "ccci_fsd.h"

#include <android/log.h>
#include <log/log.h>
#include <cutils/properties.h>
#include "power.h"

#define FS_WAKE_LOCK_NAME "ccci_fsd"
#define FS_WAKE_LOCK() acquire_wake_lock(PARTIAL_WAKE_LOCK, FS_WAKE_LOCK_NAME)
#define FS_WAKE_UNLOCK() release_wake_lock(FS_WAKE_LOCK_NAME)

//----------------debug macro and function define--------------//
#define ENABLE_ERROR_LOG
#define ENABLE_DEBUG_LOG

#define FSD_ERR_MTKLOG(md_id, ...)      do{ \
    if(md_id == 0) \
        __android_log_print(ANDROID_LOG_ERROR, MD1_FS_TAG, __VA_ARGS__); \
        else if(md_id == 1) \
            __android_log_print(ANDROID_LOG_ERROR, MD2_FS_TAG, __VA_ARGS__); \
        else if(md_id == 2) \
            __android_log_print(ANDROID_LOG_ERROR, MD3_FS_TAG, __VA_ARGS__); \
        else if(md_id == 4) \
            __android_log_print(ANDROID_LOG_ERROR, MD5_FS_TAG, __VA_ARGS__); \
        else \
            __android_log_print(ANDROID_LOG_ERROR, MD_COMN_TAG, __VA_ARGS__); \
    }while(0)

#define FSD_VER_MTKLOG(md_id, ...)      do{ \
        if(md_id == 0) \
            __android_log_print(ANDROID_LOG_VERBOSE, MD1_FS_TAG, __VA_ARGS__); \
        else if(md_id == 1) \
            __android_log_print(ANDROID_LOG_VERBOSE, MD2_FS_TAG, __VA_ARGS__); \
        else if(md_id == 2) \
            __android_log_print(ANDROID_LOG_VERBOSE, MD3_FS_TAG, __VA_ARGS__); \
        else if(md_id == 4) \
            __android_log_print(ANDROID_LOG_VERBOSE, MD5_FS_TAG, __VA_ARGS__); \
        else \
            __android_log_print(ANDROID_LOG_VERBOSE, MD_COMN_TAG, __VA_ARGS__); \
    }while(0)

#define FSD_DEBUG_MTKLOG(md_id, ...)      do{ \
        if(md_id == 0) \
            __android_log_print(ANDROID_LOG_DEBUG, MD1_FS_TAG, __VA_ARGS__); \
        else if(md_id == 1) \
            __android_log_print(ANDROID_LOG_DEBUG, MD2_FS_TAG, __VA_ARGS__); \
        else if(md_id == 2) \
            __android_log_print(ANDROID_LOG_DEBUG, MD3_FS_TAG, __VA_ARGS__); \
        else if(md_id == 4) \
            __android_log_print(ANDROID_LOG_DEBUG, MD5_FS_TAG, __VA_ARGS__); \
        else \
            __android_log_print(ANDROID_LOG_DEBUG, MD_COMN_TAG, __VA_ARGS__); \
    }while(0)

#define FSD_INF_MTKLOG(md_id, ...)      do{ \
        if(md_id == 0) \
            __android_log_print(ANDROID_LOG_INFO, MD1_FS_TAG, __VA_ARGS__); \
        else if(md_id == 1) \
            __android_log_print(ANDROID_LOG_INFO, MD2_FS_TAG, __VA_ARGS__); \
        else if(md_id == 2) \
            __android_log_print(ANDROID_LOG_INFO, MD3_FS_TAG, __VA_ARGS__); \
        else if(md_id == 4) \
            __android_log_print(ANDROID_LOG_INFO, MD5_FS_TAG, __VA_ARGS__); \
        else \
            __android_log_print(ANDROID_LOG_INFO, MD_COMN_TAG, __VA_ARGS__); \
    }while(0)

#define FSD_WARN_MTKLOG(md_id, ...)      do{ \
        if(md_id == 0) \
            __android_log_print(ANDROID_LOG_WARN, MD1_FS_TAG, __VA_ARGS__); \
        else if(md_id == 1) \
            __android_log_print(ANDROID_LOG_WARN, MD2_FS_TAG, __VA_ARGS__); \
        else if(md_id == 2) \
            __android_log_print(ANDROID_LOG_WARN, MD3_FS_TAG, __VA_ARGS__); \
        else if(md_id == 4) \
            __android_log_print(ANDROID_LOG_WARN, MD5_FS_TAG, __VA_ARGS__); \
        else \
            __android_log_print(ANDROID_LOG_WARN, MD_COMN_TAG, __VA_ARGS__); \
    }while(0)

#ifdef ENABLE_ERROR_LOG
#define LOGE(...)    FSD_ERR_MTKLOG(md_id, __VA_ARGS__)
#define LOGE_COM(...)  FSD_ERR_MTKLOG(0xFF, __VA_ARGS__)
#else
#define LOGE(...)
#define LOGE_COM(...)
#endif

#ifdef ENABLE_DEBUG_LOG
extern char  debug_level;

#define LOGV(...)    do{ \
        if(debug_level <= ANDROID_LOG_VERBOSE) \
            FSD_VER_MTKLOG(md_id, __VA_ARGS__); \
    }while(0)

#define LOGD(...)    do{ \
        if(debug_level <= ANDROID_LOG_DEBUG) \
            FSD_DEBUG_MTKLOG(md_id, __VA_ARGS__); \
    }while(0)

#define LOGI(...)    do{ \
        if(debug_level <= ANDROID_LOG_INFO) \
            FSD_INF_MTKLOG(md_id, __VA_ARGS__); \
    }while(0)

#define LOGW(...)    do{ \
        if(debug_level <= ANDROID_LOG_WARN) \
            FSD_WARN_MTKLOG(md_id, __VA_ARGS__); \
    }while(0)
#else
#define LOGV(...)    {}

#define LOGD(...)    {}

#define LOGI(...)    {}

#define LOGW(...)    {}

#endif

void get_debug_log_level(void);

int FS_OTPLock(int devtype __attribute__((unused)));
int FS_OTPQueryLength(int devtype __attribute__((unused)), unsigned int * Length);
int FS_OTPRead(int devtype __attribute__((unused)), unsigned int  Offset, void * BufferPtr, unsigned int Length);
int FS_OTPWrite(int devtype __attribute__((unused)), unsigned int  Offset, void * BufferPtr, unsigned int Length);
int FS_OTP_init(int md_id);
int get_modem_status();

extern bool NVM_RestoreFromBinRegion_OneFile(int file_lid, const char* filename);
#endif

