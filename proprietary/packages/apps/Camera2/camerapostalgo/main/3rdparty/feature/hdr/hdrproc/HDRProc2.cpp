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

#define DEBUG_LOG_TAG "HDR_PROC"
#define LOG_TAG DEBUG_LOG_TAG

#include "IHDR.h"
#include "HDRProc2.h"

#include <utils/KeyedVector.h>

#include <Debug.h>

using namespace NSCam;
using namespace NSCam::HDR2;

/******************************************************************************
*   Function Prototype.
*******************************************************************************/
//
sp<IHDR>
createHDRInstance(char const * const pszShotName,
            uint32_t const u4ShotMode, int32_t const i4OpenId);
//

/******************************************************************************
*
*******************************************************************************/

static const int32_t moduleVersion = HDR_MODULE_API_VERSION_2_3;

ANDROID_SINGLETON_STATIC_INSTANCE(HDRProc2);

static Mutex sHDRListLock;
static DefaultKeyedVector<hdr_object_t, sp<IHDR>> sHDRList;

HDRProc2::HDRProc2()
{
    HDR_LOGD("ctor(%s)", __FUNCTION__);
    sHDRList.clear();
}

HDRProc2::~HDRProc2()
{
    uninitLocked();
}

MBOOL HDRProc2::init(MINT32 openID, HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    sp<IHDR> obj = createHDRInstance("hdr", 0, openID);
    if (obj == NULL)
    {
        HDR_LOGE("[HDRProc2Init] init HDRProc2 failed");
        return MFALSE;
    }

    // add HDR instance
    {
        handle.id = reinterpret_cast<hdr_object_t>(obj.get());
        sHDRList.add(handle.id, obj);
        HDR_LOGD("add HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
    }

    HDR_LOGD("[HDRProc2Init] init HDRProc2 done for camera(%d)", openID);

    return MTRUE;
}

MBOOL HDRProc2::uninit(const HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    return uninitLocked(&handle);
}

MBOOL HDRProc2::uninitLocked(const HDRHandle* handle)
{
    if (sHDRList.isEmpty())
        return MTRUE;

    if (handle == nullptr)
    {
        // clear all HDR instances
        sHDRList.clear();
        HDR_LOGD("remove all HDR instances");
    }
    else
    {
        // clear specific HDR instance
        sHDRList.removeItem(handle->id);
        HDR_LOGD("remove HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle->id));
    }

    return MTRUE;
}

MBOOL HDRProc2::setParam(
        const HDRHandle& handle,
        MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
    if ((paramId <= HDRProcParam_Begin) || (paramId >= HDRProcParam_Num))
    {
        HDR_LOGE("[HDRProc2::setParam] invalid paramId:%d", paramId);
        return MFALSE;
    }

    AutoMutex l(sHDRListLock);

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    return sHDRList.valueFor(handle.id)->setParam(paramId, iArg1, iArg2);
}

MBOOL HDRProc2::getParam(
        const HDRHandle& handle,
        MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2)
{
    if ((paramId <= HDRProcParam_Begin) || (paramId >= HDRProcParam_Num))
    {
        HDR_LOGE("[HDRProc2::getParam] invalid paramId:%d", paramId);
        return MFALSE;
    }

    AutoMutex l(sHDRListLock);

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    return sHDRList.valueFor(handle.id)->getParam(paramId, rArg1, rArg2);
}

MBOOL HDRProc2::setShotParam(
        const HDRHandle& handle,
        MSize& pictureSize, MSize& postviewSize, MRect& cropRegion)
{
    MBOOL ret = MTRUE;

    HDRProc_ShotParam param {pictureSize, postviewSize, cropRegion};

    HDR_LOGD("[HDRProc2::setShotParam] size(%dx%d) postview(%dx%d) crop(%d,%d,%dx%d)",
            pictureSize.w, pictureSize.h,
            // TODO: check if postivew setting can be removed
            postviewSize.w, postviewSize.h,
            cropRegion.leftTop().x, cropRegion.leftTop().y,
            cropRegion.width(), cropRegion.height());

    {
        AutoMutex l(sHDRListLock);

        if (sHDRList.indexOfKey(handle.id) < 0)
        {
            HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                    reinterpret_cast<uintptr_t>(handle.id));
            return MFALSE;
        }

        if (!sHDRList.valueFor(handle.id)->setShotParam(&param))
        {
            HDR_LOGE("[HDRProc2::setShotParam] setShotParam failed");
            ret = MFALSE;
        }
    }

    return ret;
}

MBOOL HDRProc2::prepare(const HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    MBOOL ret = MTRUE;

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    const sp<IHDR>& obj(sHDRList.valueFor(handle.id));

    ret = obj->updateInfo();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Prepare] updateInfo failed");
        goto lbExit;
    }

    ret = obj->prepare();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Prepare] createSourceAndSmallImg fail");
    }

lbExit:
    return ret;
}

MBOOL HDRProc2::addInputFrame(
        const HDRHandle& handle,
        MINT32 frameIndex, const sp<IImageBuffer>& inBuffer)
{
    AutoMutex l(sHDRListLock);

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    return sHDRList.valueFor(handle.id)->addInputFrame(frameIndex, inBuffer);
}

MBOOL HDRProc2::start(const HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    MBOOL ret = MTRUE;

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    ret = sHDRList.valueFor(handle.id)->process();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Start] process failed");
    }

    return ret;
}

MBOOL HDRProc2::release(const HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    MBOOL ret = MTRUE;

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return MFALSE;
    }

    ret = sHDRList.valueFor(handle.id)->release();
    if (MTRUE != ret)
    {
        HDR_LOGE("[HDRProc2::Release] release failed");
    }

    return ret;
}

#if 0
MBOOL HDRProc2::getHDRCapInfo(
        const HDRHandle& handle,
        MINT32& i4FrameNum,
        Vector<MUINT32>& vu4Eposuretime,
        Vector<MUINT32>& vu4SensorGain,
        Vector<MUINT32>& vu4FlareOffset)
{
    MBOOL ret = MTRUE;
    Vector<NS3Av3::CaptureParam_T> rCap3AParam;

    {
        AutoMutex l(sHDRListLock);

        if (sHDRList.indexOfKey(handle.id) < 0)
        {
            HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                    reinterpret_cast<uintptr_t>(handle.id));
            return MFALSE;
        }

        sHDRList.valueFor(handle.id)->getCaptureInfo(rCap3AParam, i4FrameNum);
    }

    HDR_LOGD("[HDRProc2::getHDRCapInfo] hdrFrameNum(%d)", i4FrameNum);

    Vector<NS3Av3::CaptureParam_T>::iterator it = rCap3AParam.begin();
    while (it != rCap3AParam.end())
    {
        HDR_LOGD("=================\n" \
                "[HDRProc2::getHDRCapInfo] u4Eposuretime(%u) " \
                " u4AfeGain(%u) u4IspGain(%u) u4RealISO(%u) u4FlareOffset(%u)",
                it->u4Eposuretime, it->u4AfeGain, it->u4IspGain,
                it->u4RealISO, it->u4FlareOffset);

        vu4Eposuretime.push_back(it->u4Eposuretime);
        vu4SensorGain.push_back(it->u4AfeGain);
        vu4FlareOffset.push_back(it->u4FlareOffset);

        it++;
    }

    return ret;
}

MBOOL HDRProc2::getHDRCapInfo(
        const HDRHandle& handle,
        std::vector<HDRCaptureParam>& vCapParam) const
{
    MBOOL ret = MTRUE;

    {
        AutoMutex l(sHDRListLock);

        if (sHDRList.indexOfKey(handle.id) < 0)
        {
            HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                    reinterpret_cast<uintptr_t>(handle.id));
            return MFALSE;
        }

        ret = sHDRList.valueFor(handle.id)->getCaptureInfo(vCapParam);
    }

    HDR_LOGD("[HDRProc2::getHDRCapInfo] hdrFrameNum(%zu)", vCapParam.size());

    return ret;
}
#endif
MVOID HDRProc2::setCompleteCallback(
        const HDRHandle& handle,
        HDRProcCompleteCallback_t completeCB, MVOID* user)
{
    AutoMutex l(sHDRListLock);

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return;
    }

    sHDRList.valueFor(handle.id)->setCompleteCallback(completeCB, user);
}

MINT32 HDRProc2::queryHDRAlgoType(const HDRHandle& handle)
{
    AutoMutex l(sHDRListLock);

    if (sHDRList.indexOfKey(handle.id) < 0)
    {
        HDR_LOGE("cannot find HDR(%#08" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(handle.id));
        return -1;
    }

    return sHDRList.valueFor(handle.id)->queryHDRAlgoType();
}
