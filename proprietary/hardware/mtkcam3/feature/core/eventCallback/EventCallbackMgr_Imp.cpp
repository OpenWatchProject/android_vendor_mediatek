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

#define LOG_TAG "MtkCam/EventCallbackMgr"

#include "EventCallbackMgr_Imp.h"

#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_PRERELEASE);

using namespace NSCam;
using namespace android;

#undef __func__
#define __func__ __FUNCTION__

#define ACSM_LOG(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define ACSM_INF(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define ACSM_WRN(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACSM_ERR(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif




/*******************************************************************************
* Get Instance
********************************************************************************/
EventCallbackMgr *EventCallbackMgr::getInstance()
{
    static EventCallbackMgr_Imp gEventCallbackMgr;
    return &gEventCallbackMgr;
}

/*******************************************************************************
* EventCallbackMgr_Imp  cons/destructors
********************************************************************************/
EventCallbackMgr_Imp::EventCallbackMgr_Imp()
{
}

EventCallbackMgr_Imp::~EventCallbackMgr_Imp()
{
}


/*******************************************************************************
* EventCallbackMgr_Imp  function Implementation
********************************************************************************/
MBOOL
EventCallbackMgr_Imp::registerRequestCB(RequestCBFunc func)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    mRequestCbFunc = func;
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
EventCallbackMgr_Imp::onRequestFinishedCB( int32_t const ImgReaderId,
                                          int32_t const frameNumber,
                                          int64_t const i8Timestamp,
                                          uint32_t          status)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;

    MY_LOGI(
        "ImgReaderId=(%d), frameNumber(%d), timestamp(%" PRId64 "), status(%d)",
        ImgReaderId, frameNumber, i8Timestamp, status
    );

    if (mRequestCbFunc == nullptr)
    {
        MY_LOGE("EventCallback(mRequestCbFunc) function is empty!");
        return ret;
    }

    ret = mRequestCbFunc(
                          ImgReaderId,
                          frameNumber,
                          status,
                          i8Timestamp
                       );
    FUNC_END;
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
EventCallbackMgr_Imp::onImageBufferFinishedCB( int32_t const ImgReaderId,
                                              int32_t const frameNumber,
                                              int64_t const i8Timestamp,
                                              uint32_t          status)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    FUNC_END;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
EventCallbackMgr_Imp::init()
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.EventCallback", 0);
    }
    return MTRUE;
}


