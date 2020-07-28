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

/***************************************************************************************************
 * @file Log.h
 * @brief The header file defines the macro of Log
 **************************************************************************************************/

#ifndef __PKM_LOGS_H__
#define __PKM_LOGS_H__

#include <android/log.h>

#define PKM_MORE_LOG
#define PKM_MORE_LOG_PRINT
#define VERSION "v1.003"

// Mandatory Log ===================================================================================
#define PKM_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, ##__VA_ARGS__)
#define PKM_LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, ##__VA_ARGS__)
#define PKM_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, ##__VA_ARGS__)
#define PKM_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, ##__VA_ARGS__)
#define PKM_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, ##__VA_ARGS__)
// Mandatory Log ===================================================================================

// Detail Log ======================================================================================
#ifdef PKM_MORE_LOG
#define PKMM_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, ##__VA_ARGS__)
#define PKMM_LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, ##__VA_ARGS__)
#define PKMM_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, ##__VA_ARGS__)
#define PKMM_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, ##__VA_ARGS__)
#define PKMM_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, ##__VA_ARGS__)
#else
#define PKMM_LOGE(...)
#define PKMM_LOGW(...)
#define PKMM_LOGI(...)
#define PKMM_LOGD(...)
#define PKMM_LOGV(...)
#endif
// Detail Log ======================================================================================

// Debug Log (Most of time, the switch is OFF, or too much log will be show)========================
#ifdef PKM_ONLY_FOR_DEBUG
#define PKMD_LOG(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, ##__VA_ARGS__)
#else
#define PKMD_LOG(...)
#endif
// Debug Log =======================================================================================

#endif