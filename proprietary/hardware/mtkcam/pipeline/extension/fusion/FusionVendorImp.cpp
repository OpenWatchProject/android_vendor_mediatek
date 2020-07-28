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
#define LOG_TAG "FusionVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;

#include "FusionVendorImp.h"
#include "controller/FusionCtrler.h"
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

#define FEATURE_SLOT_SIZE (2)
//
using namespace android;
using namespace NSCam::plugin;
//
wp<FusionVendor> FusionVendorImp::mThis = NULL;
MINT32 FusionVendorImp::mTotalRefCount = 0;
ControllerContainer<BaseFusionCtrler> FusionVendorImp::sUnusedCtrlers;
static Mutex FusionVendorCreateLock;
//
static std::mutex mFusionCtrlerLock;
static std::weak_ptr<BaseFusionCtrler> mFusionCtrlerSignleTone;


// ----------------------------------------------------------------------------
// helper function
// ----------------------------------------------------------------------------
static
inline
std::shared_ptr<BaseFusionCtrler>
create_fusion_controller(
    MINT32 mode
)
{
    std::shared_ptr<BaseFusionCtrler> p = nullptr;
    if (MTK_PLUGIN_MODE_FUSION_3rdParty == mode)
    {
        p = mFusionCtrlerSignleTone.lock();
        if (p == nullptr)
        {
            p = std::shared_ptr<BaseFusionCtrler>(new FusionCtrler(mode));
            CAM_LOGD("[%s] new fusion ctrler %p", __FUNCTION__, p.get());
            mFusionCtrlerSignleTone = p;
        }
        else
        {
            CAM_LOGD("[%s] old fusion ctrler %p", __FUNCTION__, p.get());
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
sp<FusionVendor> FusionVendor::createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
{
    Mutex::Autolock _l(FusionVendorCreateLock);
    sp<FusionVendor> pInstance = FusionVendorImp::mThis.promote();
    if (pInstance != NULL)
    {
        CAM_LOGI("id:%d createInstance() old %p", i4OpenId, pInstance.get());
    }
    else
    {
        FusionVendorImp::mThis = new FusionVendorImp(pcszName, i4OpenId, vendorMode);
        pInstance = FusionVendorImp::mThis.promote();
        CAM_LOGI("id:%d createInstance() new %p", i4OpenId, pInstance.get());
    }
    return pInstance;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FusionVendorImp::
FusionVendorImp(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
) : BaseVendor(i4OpenId, vendorMode, String8::format("Fusion vendor"))
{
    mRefCount = FusionVendorImp::mTotalRefCount++;

    MY_LOGD(i4OpenId, "new FusionVendorImp() 0x%" PRIx64 " from %s", vendorMode, pcszName);

    setThreadShotsSize(1);

    // get main1 & main2 openId
    if (!StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId))
    {
        MY_LOGE(i4OpenId, "get sensor id fail");
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FusionVendorImp::
~FusionVendorImp()
{
    MY__LOGD("+");
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
get(
    MINT32           openId,
    const InputInfo& in,
    FrameInfoSet&    out
)
{
    // wait only on wide if necessary
    if (openId == mMain1OpenId)
    {
        AutoMutex _l(mMyLock);

        if (!mTeleSet)
        {
            MY_LOGD(openId, "wait tele vendor process (get & set) done");
            mMyCond.wait(mMyLock);
        }
        mTeleSet = false; // reset
    }

    std::lock_guard<std::mutex> _l(mFusionCtrlerLock);
    MY_LOGD(openId, "+");

    auto fusionCheck = std::find(
                           std::begin(in.combination),
                           std::end(in.combination),
                           MTK_PLUGIN_MODE_FUSION_3rdParty);
    if (fusionCheck == std::end(in.combination))
    {
        MY_LOGE(openId, "[%d] not find suitable combination for fusion vector.", openId);
        return UNKNOWN_ERROR;
    }

    std::shared_ptr<BaseFusionCtrler> ctrler = NULL;
    ctrler = getCtrler(CTRLER_UNUSED, *fusionCheck);
    if (ctrler.get() == NULL)
    {
        MY_LOGE(openId, "get controller fail");
        return UNKNOWN_ERROR;
    }

    MUINT32 frameCount = ctrler->getCaptureNum();
    MUINT32 frameDelayCount = ctrler->getDelayFrameNum();

    MY_LOGD(openId, "cap:%u, delay:%u", frameCount, frameDelayCount);

    // config capture setting
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


    // add delay frame if needs.
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
    // this varible is set true here, and set false when
    // frame is queued by p2.
    // To avoid set streamid map twice.
    //
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


        if (in.resizedRaw.get())
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

        if (in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End)
        {
            MY_LOGE(openId, "output yuv too much.");
        }

        for (size_t i = 0; i < in.vYuv.size(); ++i)
        {
            if (CC_LIKELY(in.vYuv[i].get()))
            {
                map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
            }
        }

        {
            ctrler->setStreamIdMapImg(std::move(map));
        }
    }

    MY_LOGD(openId, "-");
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
set(
    MINT32      openId,
    const InputSetting& in
)
{
    if (openId == mMain1OpenId)
    {
        std::lock_guard<std::mutex> _l(mFusionCtrlerLock);
        MY_LOGD(openId, "+");

        // step 1: take the first unused controller
        auto ctrler = takeCtrler(CTRLER_UNUSED);
        if (CC_UNLIKELY(ctrler.get() == NULL))
        {
            MY_LOGE(openId, "No unused controller");
            return UNKNOWN_ERROR;
        }
        // step 2:check capture num
        if (CC_UNLIKELY(ctrler->getCaptureNum() > in.vFrame.size()))
        {
            MY_LOGE(openId, "frame amount is not enough, re-set size to %zu",
                    in.vFrame.size());
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
        ctrler->setCaptureNum(1);//static_cast<int>(in.vFrame.size()));


        // fork to process via ctrler
        //if (openId == mMain1OpenId)
        {
            MY_LOGD(openId, "Trigger Ctrler");

            pushCtrler(ctrler, CTRLER_USED);

            // Step4: fire fusion asynchronously and push this controller to
            //        used controller container
            asyncRun([this, ctrler]() mutable
            {
                ::prctl(PR_SET_NAME, "doFusion", 0, 0, 0);
                ctrler->init();
                ctrler->execute();
                ctrler->waitExecution();
                removeCtrler(ctrler, CTRLER_USED); // remove self from container
                ctrler = nullptr; // clear smart pointer after use, this scope may
                // not exit until std::future::get has invoked.
            });
        }

        MY_LOGD(openId, "-");
    }

    if (openId == mMain2OpenId)
    {
        {
            AutoMutex _l(mMyLock);

            std::shared_ptr<BaseFusionCtrler> ctrler = NULL;
            ctrler = getCtrler(CTRLER_UNUSED, MTK_PLUGIN_MODE_FUSION_3rdParty);
            if (ctrler.get() == NULL)
            {
                MY_LOGE(openId, "get controller fail");
            }
            else
            {
                ctrler->setMessageCallback(in.pMsgCb, 1);
            }

            mTeleSet = true;
        }

        MY_LOGD(openId, "tele vendor process (get & set) done");

        // notify wide to process
        mMyCond.signal();
    }

    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
queue(
    MINT32  const              openId,
    MUINT32 const              requestNo,
    wp<IVendor::IDataCallback> cb,
    BufferParam                bufParam,
    MetaParam                  metaParam
)
{
    std::lock_guard<std::mutex> _l(mRequestLock);
    MY_LOGI(openId, "queue frame, requestNo=%u", requestNo);

    // Step 1:
    // find the used controller by request number
    auto spCtrl = getCtrler(requestNo, CTRLER_USED);
    MY_LOGD(openId, "spCtrl(%p)", spCtrl.get());
    if (CC_UNLIKELY(spCtrl.get() == nullptr))
    {
        MY_LOGW(openId, "error, this frame is not belong to anyone.");
        drop(openId, cb, bufParam, metaParam);
        return OK;
    }

    // Step 2:
    // enque frame to controller.
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

        spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
    }

    return OK;
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
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
FusionVendorImp::
endFlush(
    MINT32 openId
)
{
    MY_LOGD(openId, "+");

    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
sendCommand(
    MINT32      cmd,
    MINT32      openId,
    MetaItem&   meta,
    MINT32&     arg1,
    MINT32&     arg2,
    void*       arg3
)
{
    std::lock_guard<std::mutex> _l(mRequestLock);
    MY_LOGD(openId, "cmd(%d)", cmd);

    switch (cmd)
    {
        case INFO_PROCESS_AND_BUFFER:
        {
            arg1 = true;
            arg2 = BUFFER_NO_WORKING_OUTPUT;

            MINT32  idx = (openId == 0) ? 0 : 1;

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
            prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, true);
            // true: continue YUV buffer

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
//
// ----------------------------------------------------------------------------
status_t
FusionVendorImp::
dump(
    MINT32 openId
)
{
    MY_LOGD(openId, "+");

    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
FusionVendorImp::
drain(bool bCancel /* = true */)
{
    MY__LOGD("+");

    // moves all controller from container(s)
    auto CC = std::move(mUsedCtrlers);

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseFusionCtrler>::ContainerType Data_T;
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
//
// ----------------------------------------------------------------------------
ControllerContainer<BaseFusionCtrler>*
FusionVendorImp::
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
//
// ----------------------------------------------------------------------------
std::shared_ptr<BaseFusionCtrler>
FusionVendorImp::
getCtrler(CTRLER u, MINT32 mode)
{
    MY_LOGD(-1, "u:%d, mode:0x%x", u, mode);
    auto pCC = getCtrlerContainer(u);

    std::shared_ptr<BaseFusionCtrler> ctrler;

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseFusionCtrler>::ContainerType Data_T;

    pCC->iterate
    (
        [&ctrler, mode]
        (Data_T & data)
    {
        if (data.size() <= 0)
        {
            ctrler = create_fusion_controller(mode);
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
//
// ----------------------------------------------------------------------------
std::shared_ptr<BaseFusionCtrler>
FusionVendorImp::
getCtrler(
    MUINT32 requestNo,
    CTRLER u
)
{
    return getCtrlerContainer(u)->belong_to(requestNo);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::shared_ptr<BaseFusionCtrler>
FusionVendorImp::
takeCtrler(
    CTRLER u
)
{
    return getCtrlerContainer(u)->take_first();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
FusionVendorImp::
pushCtrler(
    const std::shared_ptr<BaseFusionCtrler>& c,
    CTRLER u
)
{
    getCtrlerContainer(u)->push_back(c);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
FusionVendorImp::
removeCtrler(
    std::shared_ptr<BaseFusionCtrler> c,
    CTRLER u
)
{
    return getCtrlerContainer(u)->remove(c);
}



