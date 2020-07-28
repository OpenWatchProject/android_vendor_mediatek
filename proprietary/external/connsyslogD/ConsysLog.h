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

/******************************************************************************
 *
 * Filename:
 * ---------
 *   consyslog.h
 *
 * Description:
 * ------------
 *   Log macro definition used in  logger
 *
 *****************************************************************************/

#ifndef __CONSYS_LOG_H__
#define __CONSYS_LOG_H__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <utils/Log.h>  //Logcat
#include <cutils/log.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG CONSYS_LOG_TAG

// Log enable/disable
#define LOG_ENABLE 1
#define V_ENABLE 0
#define D_ENABLE 1
#define I_ENABLE 1
#define W_ENABLE 1
#define E_ENABLE 1

#define DUMP_ROW_LEN 16

#if LOG_ENABLE && V_ENABLE
#define LOGV(...) ALOGV(__VA_ARGS__);
#else
#define LOGV(...)
#endif

#if LOG_ENABLE && D_ENABLE
#define LOGD(...) ALOGD(__VA_ARGS__);
#else
#define LOGD(...)
#endif

#if LOG_ENABLE && I_ENABLE
#define LOGI(...) ALOGI(__VA_ARGS__);
#else
#define LOGI(...) LOGI(__VA_ARGS__);
#endif

#if LOG_ENABLE && W_ENABLE
#define LOGW(...) ALOGW(__VA_ARGS__);
#else
#define LOGW(...) LOGW(__VA_ARGS__);
#endif

#if LOG_ENABLE && E_ENABLE
#define LOGE(...) ALOGE(__VA_ARGS__);
#else
#define LOGE(...) LOGE(__VA_ARGS__);
#endif

#if LOG_ENABLE
#define ASSERT(...) assert(__VA_ARGS__);
#else
#define ASSERT(...)
#endif

#endif
