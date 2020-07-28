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
#ifndef _AAA_LOG_H_
#define _AAA_LOG_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

//#include <Log.h>
#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_LOG_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_LOG_H_

/******************************************************************************
 *
 ******************************************************************************/
#include <inttypes.h>

/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
//
#if   defined(MTKCAM_LOG_LEVEL)
#define CAM_LOG_LEVEL       MTKCAM_LOG_LEVEL
#elif defined(MTKCAM_LOG_LEVEL_DEFAULT)
#define CAM_LOG_LEVEL       MTKCAM_LOG_LEVEL_DEFAULT
#else
#define CAM_LOG_LEVEL       4
#endif
//
#include <log/log.h>
#define CAM_LOGV(fmt, arg...)    do{ if(CAM_LOG_LEVEL >= 4) ALOGV(fmt, ##arg); } while(0)
#define CAM_LOGD(fmt, arg...)    do{ if(CAM_LOG_LEVEL >= 3) ALOGD(fmt, ##arg); } while(0)
#define CAM_LOGI(fmt, arg...)    do{ if(CAM_LOG_LEVEL >= 2) ALOGI(fmt, ##arg); } while(0)
#define CAM_LOGW(fmt, arg...)    do{ if(CAM_LOG_LEVEL >= 1) ALOGW(fmt, ##arg); } while(0)
#define CAM_LOGE(fmt, arg...)    ALOGE(fmt" (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__)
//
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

#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_LOG_H_


/******************************************************************************
 *
 *  Usage:
 *      [.c/.cpp]
 *          #define LOG_TAG "<your-module-name>"
 *          #include <mtkcam/utils/std/Log.h>
 *
 *      [Android.mk]
 *          LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.camera.3a.log
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
#include <log/log.h>
#ifndef USING_MTK_LDVT
//
#include <android/log.h>
#define AAA_LOGV(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'V')) ALOGV(fmt, ##arg); } while(0)
#define AAA_LOGD(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'D')) ALOGD(fmt, ##arg); } while(0)
#define AAA_LOGI(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'I')) ALOGI(fmt, ##arg); } while(0)
#define AAA_LOGW(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'W')) ALOGW(fmt, ##arg); } while(0)
#define AAA_LOGE(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'E')) ALOGE(fmt " (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__); } while(0)

__BEGIN_DECLS
int aaa_testLog(char const* tag, int prio);
__END_DECLS
//
#else //using LDVT

#ifndef DBG_LOG_TAG
#define DBG_LOG_TAG
#endif

#include <uvvf.h>
#define NEW_LINE_CHAR   "\n"

#define AAA_LOGV(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#define AAA_LOGD(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#define AAA_LOGI(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Info>: Show general system information. Like OS version, start/end of Service...
#define AAA_LOGW(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#define AAA_LOGE(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg) // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#endif

/******************************************************************************
 *
 ******************************************************************************/
#define AAA_LOGV_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGV(__VA_ARGS__); } }while(0)
#define AAA_LOGD_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGD(__VA_ARGS__); } }while(0)
#define AAA_LOGI_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGI(__VA_ARGS__); } }while(0)
#define AAA_LOGW_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGW(__VA_ARGS__); } }while(0)
#define AAA_LOGE_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGE(__VA_ARGS__); } }while(0)


/******************************************************************************
 *   (1) GLOBAL_ENABLE_MY_xxx == 0
 *        --> force to disable.
 *   (2) GLOBAL_ENABLE_MY_xxx == 1
 *        --> ENABLE_MY_xxx in local file decides to enable/disable.
 *       (2.1) ENABLE_MY_xxx in local file == 1 --> enable.
 *       (2.2) ENABLE_MY_xxx in local file == 0 --> disable.
 *       (2.3) ENABLE_MY_xxx in local file undefine
 *              --> ENABLE_MY_xxx in global file decides to enable/disable.
 ******************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#undef  GLOBAL_ENABLE_MY_LOG
#define GLOBAL_ENABLE_MY_LOG        (1)

#undef  GLOBAL_ENABLE_MY_ERR
#define GLOBAL_ENABLE_MY_ERR        (1)

#undef  GLOBAL_ENABLE_MY_LOG_OBJ
#define GLOBAL_ENABLE_MY_LOG_OBJ    (1)

#undef  GLOBAL_ENABLE_MY_ASSERT
#define GLOBAL_ENABLE_MY_ASSERT     (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif

#ifndef ENABLE_MY_ERR
    #define ENABLE_MY_ERR           (1)
#endif

#ifndef ENABLE_MY_LOG_OBJ
    #define ENABLE_MY_LOG_OBJ       (1)
#endif

#ifndef ENABLE_MY_ASSERT
    #define ENABLE_MY_ASSERT        (1)
#endif

#ifndef ENABLE_TUNING_LEVEL
    #define ENABLE_TUNING_LEVEL        (0)
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (ENABLE_TUNING_LEVEL != 0 )
    #define TUNING_LOG(fmt, arg...) CAM_LOGE(fmt, ##arg)
#else
    #define TUNING_LOG(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (ENABLE_TUNING_LEVEL == 2)
    #define MY_LOG(fmt, arg...) TUNING_LOG(fmt, ##arg)
#elif (GLOBAL_ENABLE_MY_LOG != 0 && ENABLE_MY_LOG != 0)
    #define MY_LOG(fmt, arg...) CAM_LOGD(fmt, ##arg)
#else
    #define MY_LOG(fmt, arg...)
#endif

#define MY_LOG_IF(cond, ...)      do { if ( (cond) ) { MY_LOG(__VA_ARGS__); } }while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ERR != 0 && ENABLE_MY_ERR != 0)
    #define MY_ERR(fmt, arg...) CAM_LOGE("[%s()] Err: %5d:, " fmt, __FUNCTION__, __LINE__, ##arg)    
#else
    #define MY_ERR(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ASSERT != 0 && ENABLE_MY_ASSERT != 0)
    #define MY_ASSERT(x, str)\
        if (x) {} \
        else   {  \
            MY_ERR("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1); \
        }
#else
    #define MY_ASSERT(x, str)
#endif
#endif // _AAA_LOG_H_
