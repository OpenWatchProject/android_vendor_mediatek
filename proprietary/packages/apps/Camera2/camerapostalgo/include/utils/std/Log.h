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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_LOG_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_LOG_H_

#ifdef  MTK_LOG_ENABLE
#undef  MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

/******************************************************************************
 *
 ******************************************************************************/
#include <inttypes.h>
#include <sys/cdefs.h>
#include <unistd.h>

/******************************************************************************
 *
 *  Usage:
 *      [.c/.cpp]
 *          #define LOG_TAG "<your-module-name>"
 *          #include <mtkcam/utils/std/Log.h>
 *
 *      [Android.mk]
 *          LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
 *          PS:
 *              Only needed in EXECUTABLE and SHARED LIBRARY.
 *              Not needed in STATIC LIBRARY.
 *
 *  Note:
 *      1)  Make sure to define LOG_TAG 'before' including this file.
 *      2)  LOG_TAG should follow the syntax of system property naming.
 *              Allowed:    '0'~'9', 'a'~'z', 'A'~'Z', '.', '-', or '_'
 *              Disallowed: '/'
 *      3)  In your module public API header files,
 *              Do not define LOG_TAG.
 *              Do not include this file.
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT

#ifdef CAM_ULOG_MODULE_ID

#ifndef LOG_TAG
#error "Please define LOG_TAG before include Log.h"
#endif

#include "ULog.h"

// Consider following cases:
// 1. Included Log.h but not ULog.h, nor define CAM_ULOG_MODULE_ID
//    => Legacy/customer module, use old defintion
// 2. Included Log.h and defined CAM_ULOG_MODULE_ID
//   2.1 Defined USING_MTK_LDVT
//      => Old definition
//   2.2 Not defined USING_MTK_LDVT
//      => Forward to CAM_ULOGMx()
// 3. Included ULog.h
//   3.1 Defined USING_MTK_LDVT
//      => ULog.h will include Log.h and forward to CAM_LOGx()
//   3.3 Not defined USING_MTK_LDVT
//      => Full ULog
// 4. Included both Log.h & ULog.h
//   The same as 3.

CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);

#define CAM_LOGV(fmt, arg...) CAM_ULOGMV(fmt, ##arg)
#define CAM_LOGD(fmt, arg...) CAM_ULOGMD(fmt, ##arg)
#define CAM_LOG_DECKEY(fmt, arg...) CAM_ULOGM_DECKEY(fmt, ##arg)
#define CAM_LOGI(fmt, arg...) CAM_ULOGMI(fmt, ##arg)
#define CAM_LOGW(fmt, arg...) CAM_ULOGMW(fmt, ##arg)
#define CAM_LOGE(fmt, arg...) CAM_ULOGME(fmt, ##arg)

#else // not defined CAM_ULOG_MODULE_ID
//
    #include <log/log.h>
    #include <android/log.h>
    #define CAM_LOGV(fmt, arg...)   do{ if(0!=mtkcam_testLog(LOG_TAG, 'V')) ALOGV(fmt, ##arg); } while(0)
    #define CAM_LOGD(fmt, arg...)   do{ if(0!=mtkcam_testLog(LOG_TAG, 'D')) ALOGD(fmt, ##arg); } while(0)
    #define CAM_LOGI(fmt, arg...)   do{ if(0!=mtkcam_testLog(LOG_TAG, 'I')) ALOGI(fmt, ##arg); } while(0)
    #define CAM_LOGW(fmt, arg...)   do{ if(0!=mtkcam_testLog(LOG_TAG, 'W')) ALOGW(fmt, ##arg); } while(0)
    #define CAM_LOGE(fmt, arg...)   do{ if(0!=mtkcam_testLog(LOG_TAG, 'E')) ALOGE(fmt " (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__); } while(0)

    __BEGIN_DECLS
        void setLogLevelToEngLoad(bool is_camera_on_off_timing, bool set_log_level_to_eng, int logCount = -1);
        int mtkcam_testLog(char const* tag, int prio);
        //#pragma message __BASE_FILE__
        static inline void mtkcam_static_test_LOG_TAG() {
            #if 1   //A build warning message will output if LOG_TAG == NULL.
            ALOGW(
                "[FIXME: mtkcam log] Please define LOG_TAG 'before' including this file !!! %s", LOG_TAG
            );
            #endif
            #if 1   //A build error message will output if LOG_TAG == NULL.
            char const build_error[] =
                "[FIXME: mtkcam log] Please define LOG_TAG 'before' including this file !!!" LOG_TAG;
            (void)build_error;
            #endif
        }
    __END_DECLS
//
#endif

#else //using LDVT

#ifndef DBG_LOG_TAG
#define DBG_LOG_TAG
#endif

#include <uvvf.h>
#define NEW_LINE_CHAR   "\n"

#define CAM_LOGV(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#define CAM_LOGD(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#define CAM_LOGI(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Info>: Show general system information. Like OS version, start/end of Service...
#define CAM_LOGW(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#define CAM_LOGE(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg) // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#define BASE_LOG_AST(cond, fmt, arg...)     \
    do {        \
        if (!(cond))        \
            VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
    } while (0)

#endif
//
//  ASSERT
#define CAM_LOGA(...) \
    do { \
        CAM_LOGE("[Assert] " __VA_ARGS__); \
        while(1) { ::usleep(500 * 1000); } \
    } while (0)
//
//
//  FATAL
#define CAM_LOGF(...) \
    do { \
        CAM_LOGE("[Fatal] " __VA_ARGS__); \
        LOG_ALWAYS_FATAL_IF(1, "(%s){#%d:%s}""\r\n", __FUNCTION__, __LINE__, __FILE__); \
    } while (0)

/******************************************************************************
 *
 ******************************************************************************/
#define CAM_LOGV_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGV(__VA_ARGS__); } }while(0)
#define CAM_LOGD_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGD(__VA_ARGS__); } }while(0)
#define CAM_LOGI_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGI(__VA_ARGS__); } }while(0)
#define CAM_LOGW_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGW(__VA_ARGS__); } }while(0)
#define CAM_LOGE_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGE(__VA_ARGS__); } }while(0)
#define CAM_LOGA_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGA(__VA_ARGS__); } }while(0)
#define CAM_LOGF_IF(cond, ...)      do { if ( (cond) ) { CAM_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/

#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_LOG_H_

