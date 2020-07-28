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
#define __DEBUG
#define LOG_TAG "MtkCam/VsdofVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;

#include "VsdofVendorImp.h"
#include "controller/VsdofCtrler.h"
// mtkcam
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

// AOSP
#include <sys/prctl.h>
#include <cutils/compiler.h>

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


//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------

#define FEATURE_SLOT_SIZE (1)
//
using namespace android;
using namespace NSCam::plugin;
//
wp<VsdofVendor> VsdofVendorImp::mThis = NULL;
MINT32 VsdofVendorImp::mTotalRefCount = 0;
ControllerContainer<BaseVsdofCtrler> VsdofVendorImp::sUnusedCtrlers;
static Mutex VsdofVendorCreateLock;
//
static std::mutex mVsdofCtrlerLock;
static std::weak_ptr<BaseVsdofCtrler> mVsdofCtrlerSignleTone;


// ----------------------------------------------------------------------------
// helper function
// ----------------------------------------------------------------------------
static
inline
std::shared_ptr<BaseVsdofCtrler>
create_Vsdof_controller(
    MINT32 mode
)
{
    std::shared_ptr<BaseVsdofCtrler> p = nullptr;
    if (MTK_PLUGIN_MODE_VSDOF_3rdParty == mode)
    {
        p = mVsdofCtrlerSignleTone.lock();
        if (p == nullptr)
        {
            p = std::shared_ptr<BaseVsdofCtrler>(new VsdofCtrler(mode));
            CAM_LOGD("[%s] new Vsdof ctrler %p", __FUNCTION__, p.get());
            mVsdofCtrlerSignleTone = p;
        }
        else
        {
            CAM_LOGD("[%s] old Vsdof ctrler %p", __FUNCTION__, p.get());
        }
    }
    else
    {
        CAM_LOGD("[%s] mode(%d) not support", __FUNCTION__, mode);
    }
    return p;
}



// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
sp<VsdofVendor> VsdofVendor::createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
{
    Mutex::Autolock _l(VsdofVendorCreateLock);
    sp<VsdofVendor> pInstance = VsdofVendorImp::mThis.promote();
    if (pInstance != NULL)
    {
        CAM_LOGI("id:%d createInstance() old %p", i4OpenId, pInstance.get());
    }
    else
    {
        VsdofVendorImp::mThis = new VsdofVendorImp(pcszName, i4OpenId, vendorMode);
        pInstance = VsdofVendorImp::mThis.promote();
        CAM_LOGI("id:%d createInstance() new %p", i4OpenId, pInstance.get());
    }
    return pInstance;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
VsdofVendorImp::
VsdofVendorImp(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
) : BaseVendor(i4OpenId, vendorMode, String8::format("Vsdof vendor")),
    mRefCount(0),
    mMain2Set(false)
{
    mRefCount = VsdofVendorImp::mTotalRefCount++;

    MY_LOGD(i4OpenId, "new VsdofVendorImp() 0x%" PRIx64 " from %s", vendorMode, pcszName);

    setThreadShotsSize(FEATURE_SLOT_SIZE);

    // get main1 & main2 openId
    if (!StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId))
    {
        MY_LOGE(i4OpenId, "get sensor id fail");
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
VsdofVendorImp::
~VsdofVendorImp()
{
    BaseVendor::syncAllThread();
    sUnusedCtrlers.clear();
    MY__LOGD("+");
}

// ----------------------------------------------------------------------------
// create streamid map for request frame
// this varible is set true here, and set false when
// frame is queued by p2.
// To avoid set streamid map twice.
// ----------------------------------------------------------------------------
void
VsdofVendorImp::
SetStreamMap(
    MINT32           openId,
    const InputInfo& in,
    std::shared_ptr<BaseVsdofCtrler> ctrler
)
{
    MINT32  idx = (openId == mMain1OpenId) ? 0 : 1;

    MY_LOGD(openId, "Create StreamID map");
    RequestFrame::StreamIdMap_Img map;
    if (CC_LIKELY(in.fullRaw.get()))
    {
        map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
        MY_LOGD(openId, "set source size=%dx%d StreamId:0x%x",
                in.fullRaw->getImgSize().w, in.fullRaw->getImgSize().h,
                in.fullRaw->getStreamId());

        mRawSize[idx] = in.fullRaw.get()->getImgSize();
    }
    else
    {
        MY_LOGE(openId, "no full size RAW");
    }


    if (CC_LIKELY(in.resizedRaw.get()))
    {
        map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
        MY_LOGD(openId, "set rrzo size=%dx%d StreamId:0x%x",
                in.resizedRaw->getImgSize().w, in.resizedRaw->getImgSize().h,
                in.resizedRaw->getStreamId());
    }
    else
    {
        MY_LOGD(openId, "no rrzo");
    }


    // ****************** Jpeg ********************************************
    if (CC_LIKELY(in.jpegYuv.get()))
    {
        map[RequestFrame::eRequestImg_FullOut] = in.jpegYuv->getStreamId();
        MY_LOGD(openId, "set jpegYuv size=%dx%d StreamId:0x%x",
                in.jpegYuv->getImgSize().w, in.jpegYuv->getImgSize().h,
                in.jpegYuv->getStreamId());

        mJpegYuvSize[idx] = in.jpegYuv->getImgSize();
    }
    else
    {
        MY_LOGE(openId, "no output YUV");
    }

    // ****************** Jpeg thumbnail *********************************
    if (CC_LIKELY(in.thumbnailYuv.get()))
    {
        map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
        MY_LOGD(openId, "set thumbnail size=%dx%d StreamId:0x%x",
                in.thumbnailYuv->getImgSize().w, in.thumbnailYuv->getImgSize().h,
                in.thumbnailYuv->getStreamId());
    }
    else
    {
        MY_LOGW(openId, "no thumbnail info");
    }

    // ****************** Post View **************************************
    if (CC_LIKELY(in.postview.get()) && (openId == mMain1OpenId))
    {
        map[RequestFrame::eRequestImg_PostviewOut] = in.postview->getStreamId();
        MY_LOGD(openId, "set postview size=%dx%d StreamId:0x%x",
                in.postview->getImgSize().w, in.postview->getImgSize().h,
                in.postview->getStreamId());
    }
    else
    {
        MY_LOGW(openId, "no postview info");
    }

    // ****************** Working Buffer for algorithm input *************
    if (CC_LIKELY(in.workingbuf.get()) && in.fullRaw.get())
    {
        map[RequestFrame::eRequestImg_WorkingBufferIn] = in.workingbuf->getStreamId();
        MY_LOGD(openId, "set workingbuf size=%dx%d StreamId:0x%x",
                in.workingbuf->getImgSize().w, in.workingbuf->getImgSize().h,
                in.workingbuf->getStreamId());
    }
    else
    {
        MY_LOGD(openId, "no working buffer info");
    }

    // ****************** depthmap info input from here *************
    if (in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End)
    {
        MY_LOGE(openId, "output yuv too much.");
    }
    for (size_t i = 0; i < in.vYuv.size(); ++i)
    {
        if (CC_LIKELY(in.vYuv[i].get()))
        {
            // depthmap output buffer map to eRequestImg_ReservedOut1 (setting in VsdofVendorShot)
            map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
        }
    }

    if(ctrler.get() != NULL)
    {
        ctrler->setStreamIdMapImg(std::move(map));
    }

}

// ----------------------------------------------------------------------------
// dump frame info
// ----------------------------------------------------------------------------
void
VsdofVendorImp::
DumpFrameInfo(
    MINT32  const              openId,
    BufferParam                bufParam)
{
    MY_LOGD(openId, "enque frame %d/%d", bufParam.vIn.size(), bufParam.vOut.size());
    for (size_t i = 0; i < bufParam.vIn.size(); i++)
    {
        if (bufParam.vIn[i] != NULL)
        {
            android::sp<IImageStreamInfo> info = bufParam.vIn[i]->getStreamInfo();
            MSize size = info->getImgSize();
            MY_LOGD(openId, "_in%d Stream(0x%x)(%dx%d)f(0x%x)", i, info->getStreamId(),
                    size.w, size.h, info->getImgFormat());
        }
    }

    for (size_t i = 0; i < bufParam.vOut.size(); i++)
    {
        if (bufParam.vOut[i] != NULL)
        {
            android::sp<IImageStreamInfo> info = bufParam.vOut[i]->getStreamInfo();
            MSize size = info->getImgSize();
            MY_LOGD(openId, "out%d Stream(0x%x)(%dx%d)f(0x%x)", i, info->getStreamId(),
                    size.w, size.h, info->getImgFormat());
        }
    }

}

status_t
VsdofVendorImp::
get(
    MINT32           openId,
    const InputInfo& in,
    FrameInfoSet&    out
)
{
    // wait only on Main1 if necessary (wait main2 ready)
    // if main1 & main2 do get(), it may sometimes cause NE
    if (openId == mMain1OpenId)
    {
        AutoMutex _l(mMyLock);

        if (!mMain2Set)
        {
            MY_LOGD(openId, "wait main2 vendor process (get & set) done");
            mMyCond.wait(mMyLock);
        }
        mMain2Set = false;      // reset
    }

    std::lock_guard<std::mutex> _l(mVsdofCtrlerLock);
    MY_LOGD(openId, "+");

    // check if there is VSDOF requirement
    auto VsdofCheck = std::find(
                           std::begin(in.combination),
                           std::end(in.combination),
                           MTK_PLUGIN_MODE_VSDOF_3rdParty);
    if (VsdofCheck == std::end(in.combination))
    {
        MY_LOGE(openId, "[%d] not find suitable combination for Vsdof vector.", openId);
        return UNKNOWN_ERROR;
    }

    // get an unused vsdof controller (if there is no unused controller, create a new one)
    std::shared_ptr<BaseVsdofCtrler> ctrler = NULL;
    ctrler = getCtrler(CTRLER_UNUSED, *VsdofCheck);
    if (ctrler.get() == NULL)
    {
        MY_LOGE(openId, "get controller fail");
        return UNKNOWN_ERROR;
    }

    MUINT32 frameCount = ctrler->getCaptureNum();
    MUINT32 frameDelayCount = ctrler->getDelayFrameNum();

    MY_LOGD(openId, "cap:%u, delay:%u", frameCount, frameDelayCount);

    // config capture setting for each frame
    for (MUINT32 i = 0 ; i < frameCount ; i++)
    {
        bool bLastFrame = ((i + 1) == frameCount);
        // default, copy from InputInfo
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        // modify hal control metadata
        IMetadata& halSetting(item.setting.halMeta);
        {
            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                &halSetting, MTK_FOCUS_PAUSE,
                bLastFrame ? 0 : 1);
            IMetadata::setEntry<MUINT8>(
                &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);
        }

        out.settings.push_back(item);
    }

    // add delay frame config if needs.
    for (MUINT32 i = 0 ; i < frameDelayCount; i++)
    {
        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;
        out.settings.push_back(item);
    }

    // set out info
    out.table.vendorMode = mMode;
    out.table.inCategory = FORMAT_YUV;
    out.table.outCategory = FORMAT_YUV;
    out.table.bufferType  = BUFFER_UNKNOWN;
    out.frameCount        = out.settings.size();

    // create streamid map for request frame
    SetStreamMap(openId,in,ctrler);

    MY_LOGD(openId, "-");
    return OK;
}
// ----------------------------------------------------------------------------
// Set information to vendor
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
set(
    MINT32      openId,
    const InputSetting& in
)
{
    if (openId == mMain1OpenId)
    {
        std::lock_guard<std::mutex> _l(mVsdofCtrlerLock);
        MY_LOGD(openId, "+");

        // step 1: take the first unused controller
        auto ctrler = takeCtrler(CTRLER_UNUSED);
        if (CC_UNLIKELY(ctrler.get() == NULL))
        {
            MY_LOGE(openId, "No unused controller");
            return UNKNOWN_ERROR;
        }

        // step 2: check capture num
        if (CC_UNLIKELY(ctrler->getCaptureNum() > in.vFrame.size()))
        {
            MY_LOGE(openId, "frame amount is not enough, controller expect=%d, input frame size=%zu",
                    ctrler->getCaptureNum(), in.vFrame.size());
            if (in.vFrame.size() <= 0)
            {
                MY_LOGE(openId, "frame size is 0");
                return UNKNOWN_ERROR;
            }
        }

        // step 3.1: set request to this controller
        for (MUINT32 i = 0; i < in.vFrame.size(); i++)
        {
            if (i >= ctrler->getCaptureNum())
            {
                break;
            }

            MY_LOGD(openId, "set request(%d) to this controller", in.vFrame[i].frameNo);
            ctrler->setBelong(in.vFrame[i].frameNo);
        }

        MY_LOGD(openId, "ctler(%p) callback:%p", ctrler.get(), in.pMsgCb.unsafe_get());

        // step 3.2: set callback message
        ctrler->setMessageCallback(in.pMsgCb);
        ctrler->setCaptureNum(1);           //static_cast<int>(in.vFrame.size()));

        // fork to process via ctrler
        MY_LOGD(openId, "Trigger Ctrler");

        pushCtrler(ctrler, CTRLER_USED);

        // Step4: fire Vsdof asynchronously and push this controller to
        //        used controller container
        asyncRun([this, ctrler]() mutable
        {
            ::prctl(PR_SET_NAME, "doVsdof", 0, 0, 0);
            ctrler->init();
            ctrler->execute();
            ctrler->waitExecution();
            removeCtrler(ctrler, CTRLER_USED);  // remove self from container
            ctrler = nullptr;                   // clear smart pointer after use, this scope may
                                                // not exit until std::future::get has invoked.
        });

        MY_LOGD(openId, "-");
    }

    if (openId == mMain2OpenId)
    {
        {
            AutoMutex _l(mMyLock);
            mMain2Set = true;
        }
        MY_LOGD(openId, "main2 vendor process (get & set) done");

        // notify main1 to process get
        mMyCond.signal();
    }

    return OK;
}
// ----------------------------------------------------------------------------
// Non-blocking function call.
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
queue(
    MINT32  const              openId,
    MUINT32 const              requestNo,
    wp<IVendor::IDataCallback> cb,
    BufferParam                bufParam,
    MetaParam                  metaParam
)
{
    MY_LOGI(openId, "queue frame, requestNo=%u", requestNo);

    // Step 1:
    // find the used controller by request number
    auto spCtrl = getCtrler(requestNo, CTRLER_USED);
    MY_LOGD(openId, "spCtrl(%p)", spCtrl.get());
    if (CC_UNLIKELY(spCtrl.get() == nullptr))
    {
        MY_LOGW(openId, "error, this frame is not belong to anyone.");
        drop(openId, cb, bufParam, metaParam);  // drop this frame if it is belong to no one
        return OK;
    }

    // Step 2:
    // enque frame to controller.
    {
        std::lock_guard<std::mutex> _l(mVsdofCtrlerLock);
        // dump frame info for debug
        DumpFrameInfo(openId,bufParam);

        spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
    }

    return OK;
}


// ----------------------------------------------------------------------------
// Flush.
// Callee should return all buffer to caller.
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
beginFlush(
    MINT32 openId
)
{
    MY_LOGD(openId, "+");
    drain(); // cancel flow, and drain.
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
endFlush(
    MINT32 openId
)
{
    MY_LOGD(openId, "+");

    return OK;
}
// ----------------------------------------------------------------------------
// Send command to specific vendor.
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
sendCommand(
    MINT32      cmd,
    MINT32      openId,
    MetaItem&   meta,
    MINT32&     arg1,
    MINT32&     arg2,
    void*       arg3
)
{
    MY_LOGD(openId, "cmd(%d)", cmd);

    switch (cmd)
    {
        case INFO_PROCESS_AND_BUFFER:
        {
            // create working buffer for main1/main2 YUV Input
            arg1 = true;                    // in this capture, iVendor should process or not
            arg2 = BUFFER_NO_WORKING_OUTPUT;

            MINT32  idx = (openId == mMain1OpenId) ? 0 : 1;

            // BUFFER_WORKING;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
            auto pStreamInfo =
                createImageStreamInfo(
                    "Wb:Image:NV21",
                    IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, eImgFmt_NV21, mRawSize[idx], 0
                );
            MY_LOGD(openId, "working buffer size=%d x %d (idx=%d)", mRawSize[idx].w, mRawSize[idx].h,idx);
            prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, false, openId);       // set to true for continuous buffer
            break;
        }
            /*default:
            {
                clearOneTimeBuffer();
                arg1 = false;
                arg2 = BUFFER_UNKNOWN;
                MY_LOGW(openId, "No need to process, shouldn't queue");
                break;
            }*/
    }


    return OK;
}
// ----------------------------------------------------------------------------
// Dump current status.
// ----------------------------------------------------------------------------
status_t
VsdofVendorImp::
dump(
    MINT32 openId
)
{
    MY_LOGD(openId, "+");

    return OK;
}
// ----------------------------------------------------------------------------
// cancel flow, and drain
// ----------------------------------------------------------------------------
void
VsdofVendorImp::
drain(bool bCancel /* = true */)
{
    MY__LOGD("+");

    std::lock_guard<std::mutex> _l(mVsdofCtrlerLock);
    // moves all controller from container(s)
    auto CC = std::move(mUsedCtrlers);

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseVsdofCtrler>::ContainerType Data_T;
    CC.iterate([bCancel](Data_T & data)
    {
        for (auto && itr : data)
            if (itr.get() && bCancel)
            {
                itr->doCancel();
            }

        for (auto && itr : data)
            if (itr.get())
            {
                itr->waitExecution();
            }

    });
}
// ----------------------------------------------------------------------------
// Get the container's pointer by selector.
// ----------------------------------------------------------------------------
ControllerContainer<BaseVsdofCtrler>*
VsdofVendorImp::
getCtrlerContainer(CTRLER u)
{
    switch (u)
    {
        case CTRLER_USED:
            return &mUsedCtrlers;
        case CTRLER_UNUSED:
            return &sUnusedCtrlers;
        default:
            MY_LOGE(-1, "get wrong controller container (%d)", u);
            return nullptr;
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
// Get the first controller from the specified container. If the container
// is empty, to create a controller and push back into container.
// ----------------------------------------------------------------------------
std::shared_ptr<BaseVsdofCtrler>
VsdofVendorImp::
getCtrler(CTRLER u, MINT32 mode)
{
    MY_LOGD(-1, "u:%d, mode:0x%x", u, mode);
    auto pCC = getCtrlerContainer(u);

    std::shared_ptr<BaseVsdofCtrler> ctrler;

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseVsdofCtrler>::ContainerType Data_T;

    pCC->iterate
    (
        [&ctrler, mode]
        (Data_T & data)
        {
            if (data.size() <= 0)
            {
                ctrler = create_Vsdof_controller(mode);
                data.push_back(ctrler);
            }
            else
            {
                ctrler = data.front();
            }
        }
    );
    return ctrler;
}

// ----------------------------------------------------------------------------
// Get the controller from the specified container by the request number.
// If the request number is not belong to any one, returns NULL.
// ----------------------------------------------------------------------------
std::shared_ptr<BaseVsdofCtrler>
VsdofVendorImp::
getCtrler(
    MUINT32 requestNo,
    CTRLER u
)
{
    return getCtrlerContainer(u)->belong_to(requestNo);
}

// ----------------------------------------------------------------------------
// Take the controller from the beginning of the container. If the container
// is empty, this method returns NULL.
// ----------------------------------------------------------------------------
std::shared_ptr<BaseVsdofCtrler>
VsdofVendorImp::
takeCtrler(
    CTRLER u
)
{
    return getCtrlerContainer(u)->take_first();
}

// ----------------------------------------------------------------------------
// Push back a controller to the specified container.
// ----------------------------------------------------------------------------
void
VsdofVendorImp::
pushCtrler(
    const std::shared_ptr<BaseVsdofCtrler>& c,
    CTRLER u
)
{
    getCtrlerContainer(u)->push_back(c);
}

// ----------------------------------------------------------------------------
// Removes the controller from the container.
// ----------------------------------------------------------------------------
bool
VsdofVendorImp::
removeCtrler(
    std::shared_ptr<BaseVsdofCtrler> c,
    CTRLER u
)
{
    return getCtrlerContainer(u)->remove(c);
}



