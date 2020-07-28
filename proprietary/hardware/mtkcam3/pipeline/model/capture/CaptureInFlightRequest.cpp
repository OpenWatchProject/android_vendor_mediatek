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

#define LOG_TAG "mtkcam-CaptureInFlightRequest"
//
#include "CaptureInFlightRequest.h"
//
#include "MyUtils.h"

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
/******************************************************************************
 *
 ******************************************************************************/
auto
ICaptureInFlightRequest::
createInstance(
    int32_t openId,
    std::string const& name
) -> android::sp<ICaptureInFlightRequest>
{
    sp<CaptureInFlightRequest> pInstance = new CaptureInFlightRequest(openId, name);
    if  ( CC_UNLIKELY(pInstance==nullptr) ) {
        MY_LOGE("create instance fail");
        return nullptr;
    }

    return pInstance;
}


/******************************************************************************
 *
 ******************************************************************************/
CaptureInFlightRequest::
CaptureInFlightRequest(int32_t openId, std::string const& name)
    : mOpenId(openId)
    , mUserName(name)
    , mLogLevel(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CaptureInFlightRequest::
registerListener(
    wp<ICaptureInFlightListener> const pListener
) -> int
{
    Mutex::Autolock _l(mListenerListLock);
    mlListener.push_back(pListener);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CaptureInFlightRequest::
removeListener(
    wp<ICaptureInFlightListener> const pListener
) -> int
{
    Mutex::Autolock _l(mListenerListLock);
    sp< ICaptureInFlightListener > l = pListener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener.");
        return OK;
    }
    List< wp<ICaptureInFlightListener> >::iterator item = mlListener.begin();
    while ( item != mlListener.end() ) {
        sp< ICaptureInFlightListener > lis = (*item).promote();
        if ( lis == nullptr || lis == l ) {
            // invalid listener or target to remove
            item = mlListener.erase(item);
            continue;
        }
        item++;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CaptureInFlightRequest::
insertRequest(
    uint32_t const requestNo,
    uint32_t const message __unused
) -> int
{
    {
        Mutex::Autolock _l(mMutex);
        CAM_TRACE_FMT_BEGIN("%s|r%dsize%zu", __FUNCTION__, requestNo, mvInflightCaptureRequestNo.size());
        bool bCShot = false;
        ssize_t const index = mvInflightCaptureRequestNo.indexOfKey(requestNo);
        if  ( 0 <= index )
        {
            MY_LOGW("requestNo(%d) already in", requestNo);
            return OK;
        }
        mvInflightCaptureRequestNo.add (requestNo, bCShot);
        MY_LOGD("insert capture RequestNo %d, size #:%zu", requestNo, mvInflightCaptureRequestNo.size());
    }

    auto ret = captureInFlightUpdated();
    CAM_TRACE_FMT_END();
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CaptureInFlightRequest::
removeRequest(
    uint32_t const requestNo
) -> int
{
    bool bUpdate = false;
    {
        Mutex::Autolock _l(mMutex);
        CAM_TRACE_FMT_BEGIN("%s|r%dsize%zu", __FUNCTION__, requestNo, mvInflightCaptureRequestNo.size());
        if (!mvInflightCaptureRequestNo.isEmpty())
        {
            ssize_t const index = mvInflightCaptureRequestNo.indexOfKey(requestNo);
            if  ( 0 <= index )
            {
                mvInflightCaptureRequestNo.removeItemsAt(index);
                MY_LOGD("remove capture RequestNo %d, size #:%zu", requestNo, mvInflightCaptureRequestNo.size());
                bUpdate = true;
            }
        }
    }

    auto ret = bUpdate ? captureInFlightUpdated() : OK;
    CAM_TRACE_FMT_END();
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
int
CaptureInFlightRequest::
captureInFlightUpdated()
{
    CaptureInFlightUpdated params;
    {
        Mutex::Autolock _l(mMutex);
        params.inFlightJpegCount  = mvInflightCaptureRequestNo.size();
    }
    Mutex::Autolock _l(mListenerListLock);
    List< wp<ICaptureInFlightListener> >::iterator item = mlListener.begin();
    while ( item != mlListener.end() ) {
        sp< ICaptureInFlightListener > lis = (*item).promote();
        if ( lis == nullptr ) {
            // invalid listener or target to remove
            item = mlListener.erase(item);
            continue;
        }
        lis->onCaptureInFlightUpdated(params);
        item++;
    }
    return OK;
}
