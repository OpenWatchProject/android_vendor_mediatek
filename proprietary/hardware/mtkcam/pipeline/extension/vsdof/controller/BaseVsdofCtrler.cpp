/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "MtkCam/BaseVsdofCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "BaseVsdofCtrler.h"
// log
#include <mtkcam/utils/std/Log.h>
// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
#define MY__LOGV(fmt, arg...)        CAM_LOGV("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGD(fmt, arg...)        CAM_LOGD("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGI(fmt, arg...)        CAM_LOGI("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGW(fmt, arg...)        CAM_LOGW("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGE(fmt, arg...)        CAM_LOGE("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGA(fmt, arg...)        CAM_LOGA("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGF(fmt, arg...)        CAM_LOGF("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)

#define MY_LOGV(id, fmt, arg...)     CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)     CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)     CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)     CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)     CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)     CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)     CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

using namespace android;
using namespace NSCam::plugin;


MINT32 BaseVsdofCtrler::mTotalRefCount = 0;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
BaseVsdofCtrler::
BaseVsdofCtrler(
    MINT32 mode
) : BaseController(__CALLERNAME__)
    , mMode(mode)
    , mRefCount(0)
    , mCaptureNum(0)
    , mDelayFrameNum(0)
{
    mRefCount = BaseVsdofCtrler::mTotalRefCount++;

    if (!StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId))
    {
        MY__LOGE("Cannot get sensor ids from StereoSettingProvider!");
    }
    MY__LOGD("new BaseVsdofCtrler() id:(%d,%d) %d", mMain1OpenId, mMain2OpenId, mMode);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
BaseVsdofCtrler::
~BaseVsdofCtrler()
{
    MY__LOGD("+");
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
intptr_t
BaseVsdofCtrler::
job(
    intptr_t arg1 __attribute__((unused)),
    intptr_t arg2 __attribute__((unused))
)
{
    MY__LOGD("+");
    // get pair frame and do vsdof process
    for (auto i = 0; i < getCaptureNum(); i++)
    {
        auto main1Frame = dequeFrame();
        auto main2Frame = dequeFrame();
        if (main1Frame.get() == NULL || main1Frame->isBroken())
        {
            if (main1Frame.get() == NULL)
            {
                MY__LOGE("dequed a empty main1Frame, idx=%zu", i);
            }
            else if (main1Frame->isBroken())
            {
                MY__LOGE("dequed a main1Frame but marked as error, idx=%zu", i);
            }
        }
        if (main2Frame.get() == NULL || main2Frame->isBroken())
        {
            if (main2Frame.get() == NULL)
            {
                MY__LOGE("dequed a empty main2Frame, idx=%zu", i);
            }
            else if (main2Frame->isBroken())
            {
                MY__LOGE("dequed a main2Frame but marked as error, idx=%zu", i);
            }
        }
        doDualCamProcess(main1Frame, main2Frame);
    }
    MY__LOGD("-");
    return 0;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseVsdofCtrler::
enqueFrame(
    MINT32 openId,
    MUINT32 requestNo,
    const IVendor::BufferParam& bufParam,
    const IVendor::MetaParam& metaParam,
    wp<IVendor::IDataCallback> cb
)
{
    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;
    MY_LOGD(openId, "requestNo:%u is enqued.", requestNo);
    {
        std::lock_guard<T_LOCKER> _l(_locker);
        std::shared_ptr<RequestFrame> frame( new RequestFrame(openId,
                                                              m_callerName,
                                                              bufParam,
                                                              metaParam,
                                                              cb,
                                                              m_streamIdMap_img,
                                                              m_streamIdMap_meta,
                                                              requestNo,
                                                              getEnquedTimes()));
        enqueToSpecificQueue(openId, frame);
        // check pair
        if (mMain1Frames.size() > 0 && mMain2Frames.size() > 0)
        {
            pushPairToIncomingQueue();
        }
    }
lbExit:
    return;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseVsdofCtrler::
clearFrames()
{
    MY__LOGD("+");
    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;
    std::lock_guard<T_LOCKER> _l(_locker);
    mMain1Frames.clear();
    mMain2Frames.clear();
    BaseController::clearFrames();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseVsdofCtrler::
doCancel()
{
    MY__LOGD("+");
    // add feature pipe flush here if we need
    //
    BaseController::doCancel();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseVsdofCtrler::
init()
{
    MY__LOGD("not implement");
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseVsdofCtrler::
doDualCamProcess(
    std::shared_ptr<RequestFrame> main1Frame,
    std::shared_ptr<RequestFrame> main2Frame
)
{
    MY__LOGD("not implement");
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseVsdofCtrler::
enqueToSpecificQueue(
    MINT32 openId,
    std::shared_ptr<RequestFrame> frame
)
{
    MY_LOGD(openId, "+");
    bool ret = false;
    if (openId == mMain1OpenId)
    {
        mMain1Frames.push_back(frame);
        ret = true;
    }
    else if (openId == mMain2OpenId)
    {
        mMain2Frames.push_back(frame);
        ret = true;
    }
    else
    {
        MY_LOGE(openId, "invaild open id (%d)", openId);
    }
    return ret;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseVsdofCtrler::
pushPairToIncomingQueue()
{
    MY__LOGD("+");

    // step1. pop main1 queue first
    auto main1Frame = mMain1Frames.front();
    mMain1Frames.pop_front();

    // step2. pop main2 queue
    auto main2Frame = mMain2Frames.front();
    mMain2Frames.pop_front();

    BaseController::enqueFrame(main1Frame);
    BaseController::enqueFrame(main2Frame);
}
