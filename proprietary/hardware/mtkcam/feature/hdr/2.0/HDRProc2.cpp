/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#define DEBUG_LOG_TAG "PROC"

#include "HDR.h"
#include <mtkcam/feature/hdr/2.0/HDRProc2.h>
#include "Platform.h"

#include <mtkcam/v3/hal/aaa_hal_common.h>

#include <mtkcam/feature/hdr/2.0/utils/Debug.h>

using namespace NSCam;
using namespace NS3Av3;

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(HDRProc2);

static Mutex sHDRLock;
static NSCam::HDR* sHDR = nullptr;

HDRProc2::HDRProc2()
    : mSceneHandle(-1)
{
}

HDRProc2::~HDRProc2()
{
    uninitLocked();
}

MBOOL HDRProc2::init(MINT32 openID)
{
    AutoMutex l(sHDRLock);

    if (sHDR == nullptr)
    {
        sHDR = new NSCam::HDR("hdr", 0, openID);
        if (sHDR == nullptr)
        {
            HDR_LOGE("[HDRProc2Init] init HDRProc2 failed");
            return MFALSE;
        }
    }

    HDR_LOGD("[HDRProc2Init] init HDRProc2 done for camera(%d)", openID);

    return MTRUE;
}

MBOOL HDRProc2::uninit()
{
    AutoMutex l(sHDRLock);

    // exit performance mode
    Platform::getInstance().exitPerfMode(mSceneHandle);

    return uninitLocked();
}

MBOOL HDRProc2::uninitLocked()
{
    if (sHDR == nullptr)
        return MTRUE;

    delete sHDR;
    sHDR = nullptr;

    return MTRUE;
}

MBOOL HDRProc2::setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
    if ((paramId <= HDRProcParam_Begin) || (paramId >= HDRProcParam_Num))
    {
        HDR_LOGE("[HDRProc2::setParam] invalid paramId:%d", paramId);
        return MFALSE;
    }

    AutoMutex l(sHDRLock);

    return sHDR->setParam(paramId, iArg1, iArg2);
}

MBOOL HDRProc2::getParam(MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2)
{
    if ((paramId <= HDRProcParam_Begin) || (paramId >= HDRProcParam_Num))
    {
        HDR_LOGE("[HDRProc2::getParam] invalid paramId:%d", paramId);
        return MFALSE;
    }

    AutoMutex l(sHDRLock);

    return sHDR->getParam(paramId, rArg1, rArg2);
}

MBOOL HDRProc2::setShotParam(
        MSize& pictureSize, MSize& postviewSize, MRect& cropRegion)
{
    MBOOL ret = MTRUE;

    HDRProc_ShotParam param { pictureSize, postviewSize, cropRegion };

    HDR_LOGD("[HDRProc2::setShotParam] size(%dx%d) postview(%dx%d) crop(%d,%d,%dx%d)",
            pictureSize.w, pictureSize.h,
            // TODO: check if postivew setting can be removed
            postviewSize.w, postviewSize.h,
            cropRegion.leftTop().x, cropRegion.leftTop().y,
            cropRegion.width(), cropRegion.height());

    {
        AutoMutex l(sHDRLock);
        if (!sHDR->setShotParam(&param))
        {
            HDR_LOGE("[HDRProc2::setShotParam] setShotParam failed");
            ret = MFALSE;
        }
    }

    return ret;
}

MBOOL HDRProc2::prepare()
{
    AutoMutex l(sHDRLock);

    MBOOL ret = MTRUE;

    // enter performance mode
    mSceneHandle = Platform::getInstance().enterPerfMode();

    ret = sHDR->updateInfo_cam3();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Prepare] updateInfo_cam3 failed");
        goto lbExit;
    }

    ret = sHDR->EVBracketCapture_cam3();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Prepare] createSourceAndSmallImg_cam3 fail");
    }

lbExit:
    return ret;
}

MBOOL HDRProc2::addInputFrame(
        MINT32 frameIndex, const sp<IImageBuffer>& inBuffer)
{
    AutoMutex l(sHDRLock);

    return sHDR->addInputFrame_cam3(frameIndex, inBuffer);
}

MBOOL HDRProc2::addOutputFrame(
        HDROutputType type, sp<IImageBuffer>& outBuffer)
{
    AutoMutex l(sHDRLock);

    return sHDR->addOutputFrame_cam3(type, outBuffer);
}

MBOOL HDRProc2::start()
{
    AutoMutex l(sHDRLock);

    MBOOL ret = MTRUE;

    ret = sHDR->process_cam3();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Start] process_cam3 failed");
    }

    return ret;
}

MBOOL HDRProc2::release()
{
    AutoMutex l(sHDRLock);

    MBOOL ret = MTRUE;

    ret = sHDR->release_cam3();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Release] release_cam3 failed");
    }

    return ret;
}

MBOOL HDRProc2::getHDRCapInfo(
        MINT32& i4FrameNum,
        Vector<MUINT32>& vu4Eposuretime,
        Vector<MUINT32>& vu4SensorGain,
        Vector<MUINT32>& vu4FlareOffset)
{
    MBOOL ret = MTRUE;
    Vector<NS3Av3::CaptureParam_T> rCap3AParam;

    {
        AutoMutex l(sHDRLock);
        sHDR->getCaptureInfo_cam3(rCap3AParam, i4FrameNum);
    }

    HDR_LOGD("[HDRProc2::getHDRCapInfo] hdrFrameNum(%d)", i4FrameNum);

    Vector<NS3Av3::CaptureParam_T>::iterator it = rCap3AParam.begin();
    while (it != rCap3AParam.end())
    {
        HDR_LOGD("=================\n" \
                "[HDRProc2::getHDRCapInfo] u4Eposuretime(%d) " \
                " u4AfeGain(%d) u4IspGain(%d) u4RealISO(%d) u4FlareOffset(%d)",
                it->u4Eposuretime, it->u4AfeGain, it->u4IspGain,
                it->u4RealISO, it->u4FlareOffset);

        vu4Eposuretime.push_back(it->u4Eposuretime);
        vu4SensorGain.push_back(it->u4AfeGain);
        vu4FlareOffset.push_back(it->u4FlareOffset);

        it++;
    }

    return ret;
}

MVOID HDRProc2::setCompleteCallback(
        HDRProcCompleteCallback_t completeCB, MVOID* user)
{
    AutoMutex l(sHDRLock);

    return sHDR->setCompleteCallback(completeCB, user);
}
