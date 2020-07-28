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

#include "stdlib.h"
#include "stdio.h"
#include <android/log.h>
#include <cutils/properties.h>
//
#include <utils/Mutex.h>
using namespace android;

#ifndef LOG_TAG
#define LOG_TAG "AAALOGTAG"
#endif //LOG_TAG

#define GET_PROP_OFF(prop, init)\
{\
    property_get(prop, value, (init));\
    itmp = atoi(value);\
    if( itmp == 0 )\
    {\
        bEn = false;\
        return;\
    }\
}

#define GET_PROP_ON(prop, init)\
{\
    property_get(prop, value, (init));\
    itmp = atoi(value);\
    if( itmp != 0 )\
    {\
        bEn = true;\
        return;\
    }\
}

#define LOG_ON "1"
#define LOG_OFF "0"

#if MTKCAM_3A_LOG_DEFAULT == 1 // default log switch
#define DEFAULT_LOG LOG_ON  // default log on
#define BLOG_USER_LOAD 0
#else
#define DEFAULT_LOG LOG_OFF  // default log off
#define BLOG_USER_LOAD 1

#endif

class clog3A
{
public:
    static bool bEn;

    static class _init
    {
        public:
        _init()
        {
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            int itmp = 0;

            property_get("vendor.debug.3alog.enable", value, DEFAULT_LOG);    // check property
            itmp = atoi(value);
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, " 3alog = %d, default = %s", itmp, DEFAULT_LOG );

            if( itmp != 0 ) // enable log
            {   // check if off log manually
                GET_PROP_OFF("vendor.debug.af.enable", LOG_ON);
                GET_PROP_OFF("vendor.debug.ae.enable", LOG_ON);
                GET_PROP_OFF("vendor.debug.awb.enable", LOG_ON);

                bEn = true;
            }
            else if( itmp == 0 ) // disable log
            {   // check if on log manually
                GET_PROP_ON("vendor.debug.af.enable", LOG_OFF);
                GET_PROP_ON("vendor.debug.ae.enable", LOG_OFF);
                GET_PROP_ON("vendor.debug.awb.enable", LOG_OFF);

                bEn = false;
            }
        }
    } _initializer;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MY_LOG(...)          do { if ( (clog3A::bEn) ) { __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__); } }while(0)
#define MY_LOG_IF(cond, ...) do { if ( (cond) ) { __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__); } }while(0)
#define MY_ERR(fmt, arg...)  do { __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "[%s()] Err: %5d:, " fmt, __FUNCTION__, __LINE__, ##arg); }while(0)
#define MY_ASSERT(x, str)\
        if (x) {} \
        else   {  \
            MY_ERR("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1); \
        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define BUF_THRESHOLD   900
#define BUF_SIZE        2048
#define BUF_REAL_SIZE   BUF_SIZE-1

struct BLog
{
    const int32_t   mLevel;
    mutable Mutex   mLock;
    char*           mBuf;
    size_t          mLen;
    //
    BLog(int32_t level = ANDROID_LOG_DEBUG)
        : mLevel(level)
        , mLock()
        , mBuf(new char[BUF_SIZE]())
        , mLen(0){}
    ~BLog() {
        if ( mBuf != NULL )
            delete [] mBuf;
    }
};

#define DECLARE_BLOG    \
BLog mBlog;             \

#define BLOG(tag, fmt, args...) blog_print(mBlog, tag, fmt, ##args)
#define BLOG_FLUSH(tag) blog_flush(mBlog, tag)


void blog_print(BLog &blog, const char *tag, const char *fmt, ...);
void blog_flush(BLog &blog, const char *tag);
int blog_isUserLoad();


#endif // _AAA_LOG_H_
