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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// loging
#define LOG_TAG "DCMFPlugin"
#include <mtkcam/utils/std/Log.h>

// Standard C header file

// Android system/core header file
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/feature/stereo/pipe/IIspPipeRequest.h>
// Local header file
#include "../TPNodePlugin.h"
#include "DCMFPlugin.h"

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );

#define WRITE_PERMISSION 0660

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

using namespace NSCam::NSCamFeature::NSFeaturePipe::DualCamThirdParty;
using NSCam::NSIoPipe::NSSImager::IImageTransform;
// mount the implementation of third party
REGISTER_TPNODE_PLUGIN(MTK_PLUGIN_MODE_DCMF_3RD_PARTY, DCMFPlugin);

/******************************************************************************
 *
 ******************************************************************************/
namespace
{

template <typename T>
inline MBOOL
tryGetMetadata(IMetadata* pMetadata, MUINT32 const tag, T & rVal)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
//
MVOID
dumpImage(TPNodePlugin::RequestParams const& params)
{
    // TODO
    // refactor this hard code
    SCOPED_TRACER();

    // dump data by using property: debug.stereo.dumpcapturedata
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.di.stereo.dumpcapturedata", cLogLevel, "0");
    MINT32 value = ::atoi(cLogLevel);

    IImageBuffer* pInputYuv_main1 = params.in[0].pFullYuv;
    IImageBuffer* pResizeYuv_main1 = params.in[0].pResizedYuv;
    IImageBuffer* pInputYuv_main2 = params.in[1].pFullYuv;
    IImageBuffer* pResizeYuv_main2 = params.in[1].pResizedYuv;
    IImageBuffer* pOutputYuv = params.out.pFullYuv;
    IImageBuffer* pOutClean = params.out.pClean;
    IImageBuffer* pOutputDepth = params.out.pDepth;

    if(value > 0)
    {
        // regenerate filename if need.
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        auto msFilename = std::string("/sdcard/vsdof/tpnode_ut/dcmf_isppipe/")+std::to_string(millis)+std::string("_")+std::to_string(params.uRequestNo);
        MY_LOGD("filename(%s)", msFilename.c_str());

        bool isSuccess = false;
        isSuccess = NSCam::Utils::makePath(msFilename.c_str(), WRITE_PERMISSION);

        if(isSuccess)
        {
            if(pInputYuv_main1) pInputYuv_main1->saveToFile((msFilename+std::string("/")+std::string("inputYUV_main1")).c_str());
            if(pResizeYuv_main1) pResizeYuv_main1->saveToFile((msFilename+std::string("/")+std::string("resizeYUV_main1")).c_str());
            if(pInputYuv_main2) pInputYuv_main2->saveToFile((msFilename+std::string("/")+std::string("inputYUV_main2")).c_str());
            if(pResizeYuv_main2) pResizeYuv_main2->saveToFile((msFilename+std::string("/")+std::string("resizeYUV_main2")).c_str());

            if(pOutputYuv) pOutputYuv->saveToFile((msFilename+std::string("/")+std::string("resultYUV")).c_str());
            if(pOutClean) pOutClean->saveToFile((msFilename+std::string("/")+std::string("cleanImg")).c_str());
            if(pOutputDepth) pOutputDepth->saveToFile((msFilename+std::string("/")+std::string("depthmap")).c_str());
        }
        else
        {
            MY_LOGE("makePath failed!");
        }
    }
}
//
const char*
id2Name(MINT32 id)
{
    #define MAKE_NAME_CASE(name)  case name: return #name
    {
        switch(id)
        {
            MAKE_NAME_CASE(PBID_IN_FULLSIZE_YUV_MAIN1);
            MAKE_NAME_CASE(PBID_IN_RESIZE_YUV_MAIN1);
            MAKE_NAME_CASE(PBID_IN_BINING_YUV_MAIN1);
            MAKE_NAME_CASE(PBID_IN_FULLSIZE_YUV_MAIN2);
            MAKE_NAME_CASE(PBID_IN_RESIZE_YUV_MAIN2);
            MAKE_NAME_CASE(PBID_OUT_BOKEH_YUV);
            MAKE_NAME_CASE(PBID_OUT_CLEAN_YUV);
            MAKE_NAME_CASE(PBID_OUT_THIRDPARTY_DEPTH);

            MAKE_NAME_CASE(PBID_IN_APP_META);
            MAKE_NAME_CASE(PBID_IN_HAL_META_MAIN1);
            MAKE_NAME_CASE(PBID_IN_HAL_META_MAIN2);
            MAKE_NAME_CASE(PBID_OUT_APP_META);
            MAKE_NAME_CASE(PBID_OUT_HAL_META);
        }
        return "unknown";
    }
    #undef MAKE_NAME_CASE
}
//
MVOID
pushImageBuffer(sp<IIspPipeRequest>& request, const BufferSetting& setting, IImageBuffer* pImgBuf)
{
    const char* name = id2Name(setting.bufferID);
    const char* type = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? "INPUT" : "OUTPUT";
    if(pImgBuf != nullptr)
    {
        MSize imgSize = pImgBuf->getImgSize();

        const static MINT32 invalidatedImgSize = 2;
        if((imgSize.w > invalidatedImgSize) && (imgSize.h > invalidatedImgSize))
        {
            sp<IImageBuffer> spImaBuf = pImgBuf;
            request->pushRequestImageBuffer(setting, spImaBuf);
            MY_LOGD("push imageBuffer to request: name: %s, addr: %p, type: %s, size: %dx%d",
                name, pImgBuf, type, spImaBuf->getImgSize().w, spImaBuf->getImgSize().h);
        }
        else
        {
            MY_LOGD("not push imageBuffer to request, name: %s, type: %s, size: %dx%d", name, type, imgSize.w, imgSize.h);
        }
    }
    else
    {
        MY_LOGD("not push imageBuffer to request, name: %s, type: %s", name, type);
    }
}
//
MVOID
pushMetadata(sp<IIspPipeRequest>& request, const BufferSetting& setting, IMetadata* pMetadata)
{
    const char* name = id2Name(setting.bufferID);
    const char* type = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? "INPUT" : "OUTPUT";
    if(pMetadata != nullptr)
    {
        request->pushRequestMetadata(setting, pMetadata);
        MY_LOGD("push metadata to request: name: %s, addr: %p, type: %s",
            name, pMetadata, type ? "INPUT" : "OUTPUT");
    }
    else
    {
        MY_LOGD("not push metadata to request, name: %s, type: %s", name, type);
    }
};
//
constexpr MUINT32 gReqNumBase = 1000;
constexpr MUINT32 gLastReqNum = gReqNumBase - 1;
MUINT32
convertReqNumFromPlugin2IspPipe(MUINT32 pluginReqNum, MUINT32 serialNum, MBOOL isLastReq = MFALSE)
{
    if(serialNum > gReqNumBase)
    {
        MY_LOGD("invalidated serialNum: %u, base: %u", serialNum, gReqNumBase);
    }
    return (pluginReqNum*gReqNumBase) + (isLastReq?gLastReqNum : serialNum);
}
//
MUINT32
convertReqNumFromIspPipe2Plugin2(MUINT32 ispPipeReqNum)
{
    return ispPipeReqNum/gReqNumBase;
}
//
MBOOL
isLastIspPipeReqNum(MUINT32 ispPipeReqNum)
{
    return (ispPipeReqNum%gReqNumBase == gLastReqNum);
}
//
} // anonymous namespace
/******************************************************************************
 *
 ******************************************************************************/
class NSCam::v3::DCMFPluginImp
{
public:
    DCMFPlugin& getParent() { return mParent; }
    //
    const std::string getName() {return mName; }
    //
    virtual MVOID handleInit() { }
    //
    virtual MVOID handleUninit() { }
    //
    virtual MVOID handleProcess(TPNodePlugin::RequestParams const& requestParams) { }
    //
    virtual MVOID handleOnFrameQueue(TPNodePlugin::PartialParams const& partialParams) { }
    //
    virtual ~DCMFPluginImp() { }
    //
protected:
    DCMFPluginImp(const std::string& name, DCMFPlugin& parent)
    : mName(name)
    , mParent(parent)
    {

    }
    //
private:
    const std::string mName;
    DCMFPlugin&         mParent;
};
/******************************************************************************
 *
 ******************************************************************************/
class DCMFPluginDufaultImp : public NSCam::v3::DCMFPluginImp
{
public:
    DCMFPluginDufaultImp(DCMFPlugin& parent);

    MVOID handleProcess(TPNodePlugin::RequestParams const& requestParams) override;
};
/******************************************************************************
 *
 ******************************************************************************/
class DCMFPluginIspPipeImp : public NSCam::v3::DCMFPluginImp
{
public:
    DCMFPluginIspPipeImp(DCMFPlugin& parent);

    MVOID handleInit() override;

    MVOID handleProcess(TPNodePlugin::RequestParams const& requestParams) override;

    MVOID handleOnFrameQueue(TPNodePlugin::PartialParams const& partialParams) override;

private:
    static MVOID onRequestFinished(MVOID* tag, PipeResultState state, sp<IIspPipeRequest>& request);

private:
    template<typename T>
    using UniquePtr                 = std::unique_ptr<T, std::function<MVOID(T*)>>;
    using ProcessingJobTable        = std::map<MUINT32, TPNodePlugin::RequestParams>;
    using PartialParamsCollection   = std::vector<TPNodePlugin::PartialParams>;
    using ProcessingFrame           = std::tuple<MUINT32, PartialParamsCollection>;

private:
    UniquePtr<IIspPipe>     mpIspPipe;

    mutable std::mutex      mLocker;
    ProcessingJobTable      mProcessingJobs;

    std::unique_ptr<ProcessingFrame> mProcessingFramePtr;
};
/******************************************************************************
 *
 ******************************************************************************/
DCMFPluginDufaultImp::
DCMFPluginDufaultImp(DCMFPlugin& parent)
: DCMFPluginImp("DufaultImp", parent)
{

}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPluginDufaultImp::
handleProcess(TPNodePlugin::RequestParams const& requestParams)
{
    SCOPED_TRACER();

    // use first frame to make a gray image
    IMetadata* pHalMeta = requestParams.in[0].pHalMeta;
    IMetadata* pAppMeta = requestParams.in[0].pAppMeta;
    IImageBuffer* pInputYuv = requestParams.in[0].pFullYuv;
    IImageBuffer* pResizeYuv = requestParams.in[0].pResizedYuv;
    IImageBuffer* pOutputYuv = requestParams.out.pFullYuv;
    IImageBuffer* pOutClean = requestParams.out.pClean;

    PluginId_T pluginId = 0;
    IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_MODE, pluginId);
    MY_LOGD("process request with plugin(%d)", pluginId);

    if(::property_get_int32("vendor.dg.stereo.cpumcpy", 0) == 1)
    {
        MY_LOGD("cpu cpy");
        for (size_t i = 0; i < pInputYuv->getPlaneCount(); i++)
        {
            void *pInVa = (void *) (pInputYuv->getBufVA(i));
            void *pOutVa = (void *) (pOutputYuv->getBufVA(i));
            void *pOutCleanVa = (void *) (pOutClean->getBufVA(i));
            MUINT32 uBufSize = pInputYuv->getBufSizeInBytes(i);

            memcpy(pOutVa, pInVa, uBufSize);
            memcpy(pOutCleanVa, pInVa, uBufSize);
        }
    }
    else
    {
        MY_LOGD("mdp cpy");
        // use IImageTransform to handle image cropping
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if (transform.get() == nullptr)
        {
            MY_LOGE("IImageTransform is nullptr, cannot generate output");
            return;
        }

        MRect crop = MRect(pInputYuv->getImgSize().w, pInputYuv->getImgSize().h);
        MBOOL ret = transform->execute(
                pInputYuv, // src
                pOutputYuv,// dst1
                pOutClean, // dst2
                crop, // ratio crop (dst1)
                crop, // ratio crop (dst2)
                0,    // no transform (dst1)
                0,    // no transform (dst2)
                3000  // timeout
            );

        if(!ret)
        {
            MY_LOGE("Failed doIImageTransform!");
            return;
        }
    }
    pOutputYuv->syncCache(eCACHECTRL_FLUSH);
    pOutClean->syncCache(eCACHECTRL_FLUSH);

    IImageBuffer* pOutputDepth = requestParams.out.pDepth;
    void *pDepthVa = (void *) (pOutputDepth->getBufVA(0));
    MUINT32 uDepthSize = pOutputDepth->getBufSizeInBytes(0);
    MY_LOGD("process depth va(%p) size(%u)", pDepthVa, uDepthSize);
    memset(pDepthVa, 100, uDepthSize);
    pOutputDepth->syncCache(eCACHECTRL_FLUSH);

    MY_LOGD("call parent reponse");
    getParent().response(requestParams, android::OK);
}
/******************************************************************************
 *
 ******************************************************************************/
DCMFPluginIspPipeImp::
DCMFPluginIspPipeImp(DCMFPlugin& parent)
: DCMFPluginImp("IspPipeImp", parent)
{

}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPluginIspPipeImp::
handleInit()
{
    SCOPED_TRACER();

    // create isp pipe
    MY_LOGD("create IspPipe");
    IspPipeSetting pipeSetting;
    IspPipeOption pipeOption;
    mpIspPipe = UniquePtr<IIspPipe>(IIspPipe::createInstance(pipeSetting, pipeOption, IspPipeFlowControlType::DCMF), [](IIspPipe* ptr)
    {
        MY_LOGD("unit IspPipe");
        ptr->uninit();

        MY_LOGD("deleete IspPipe");
        delete ptr;
    });

    MY_LOGD("init IspPipe");
    mpIspPipe->init();

    MY_LOGD("sync IspPipe");
    mpIspPipe->sync();
}
//
MVOID
DCMFPluginIspPipeImp::
handleProcess(TPNodePlugin::RequestParams const& requestParams)
{
    SCOPED_TRACER();

    if(mProcessingFramePtr  == nullptr)
    {
        MY_LOGE("invalidated flow, processingFramePtr is nullptr");
        return;
    }

    const MUINT32 pluginReqNum = requestParams.uRequestNo;
    const MUINT32 processingReqNum = std::get<0>(*mProcessingFramePtr);
    if(pluginReqNum != processingReqNum)
    {
        MY_LOGE("invalidated processing pluginReqNum, current:%u, processing:%u", pluginReqNum, processingReqNum);
        return;
    }

    {
        std::lock_guard<std::mutex> guard(mLocker);
        {
            MUINT32 key = requestParams.uRequestNo;
            auto it = mProcessingJobs.find(key);
            if(it != mProcessingJobs.end())
            {
                MY_LOGE("request is existing, pluginReqNum: %u", key);
            }
            else
            {
                MY_LOGD("insert process job, pluginReqNum: %u", key);
                mProcessingJobs.insert(std::pair<MUINT32, TPNodePlugin::RequestParams>(key, requestParams));
                mProcessingFramePtr = nullptr;
            }

            MINT32 i = 0;
            for(auto item : mProcessingJobs)
            {
                MY_LOGD("in processingJobs, #:%d, key(req#):%u", i, item.first);
                i++;
            }
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPluginIspPipeImp::
handleOnFrameQueue(TPNodePlugin::PartialParams const& partialParams)
{
    SCOPED_TRACER();

    // using the same dummy buffer can't print this log, the will print the getBufVA error message
    // #define PRINTINFO(IMAGE) MY_LOGD("[info] "#IMAGE ",size: %dx%d, addr: %p, pa: %p, va: %p, haep: %p", \
    //     ((IMAGE != nullptr) ? IMAGE->getImgSize().w : -1), ((IMAGE != nullptr) ? IMAGE->getImgSize().h : -1),\
    //     IMAGE, ((IMAGE != nullptr) ? IMAGE->getBufPA(0) : 0), ((IMAGE != nullptr) ? IMAGE->getBufVA(0) : 0), ((IMAGE != nullptr) ? IMAGE->getImageBufferHeap() : 0))

    // {
    //     PRINTINFO(partialParams.in.pFullYuv);
    //     PRINTINFO(partialParams.in.pResizedYuv);
    //     PRINTINFO(partialParams.in.pBinningYuv);
    //     PRINTINFO(partialParams.out.pFullYuv);
    //     PRINTINFO(partialParams.out.pDepth);
    //     PRINTINFO(partialParams.out.pClean);
    // }
    // #undef PRINTINFO

    const MUINT32 pluginReqNum = partialParams.uRequestNo;
    if(mProcessingFramePtr == nullptr)
    {
        mProcessingFramePtr = std::make_unique<ProcessingFrame>(ProcessingFrame(pluginReqNum));
        MY_LOGD("create new, pluginReqNum: %u, processingFrame: %p", pluginReqNum, mProcessingFramePtr.get());
    }

    const MUINT32 processingReqNum = std::get<0>(*mProcessingFramePtr);
    if(pluginReqNum != processingReqNum)
    {
        MY_LOGE("current processing request is not done, current:%u, new:%u", processingReqNum, pluginReqNum);
        return;
    }

    PartialParamsCollection& partialParamsCollection = std::get<1>(*mProcessingFramePtr);
    MY_LOGD("pluginReqNum: %u", processingReqNum);

    partialParamsCollection.push_back(partialParams);

    const MINT32 pairCount = 2;
    const MINT32 partialParamsCount = partialParamsCollection.size();
    if(partialParamsCount%pairCount == 0)
    {
        const MINT32 pairNum = partialParamsCount/pairCount - 1; // to zero base
        const MINT32 main1Index = pairNum*pairCount;
        const MINT32 main2Index = pairNum*pairCount + 1;
        MY_LOGD("pair frame, pairNum:%d, main1Index:%d, main2Index:%d", pairNum, main1Index, main2Index);

        const MUINT32 ispPipeReqNum = ::convertReqNumFromPlugin2IspPipe(pluginReqNum, pairNum, partialParams.bIsLastPartial);
        sp<IIspPipeRequest> ispPipeRequest = IIspPipeRequest::createInstance(ispPipeReqNum, DCMFPluginIspPipeImp::onRequestFinished, (MVOID*)this);

        // main1
        const TPNodePlugin::PartialParams& main1PartialParams = partialParamsCollection[main1Index];
        pushImageBuffer(ispPipeRequest, {PBID_IN_FULLSIZE_YUV_MAIN1,  eBUFFER_IOTYPE_INPUT}, main1PartialParams.in.pFullYuv);
        pushImageBuffer(ispPipeRequest, {PBID_IN_RESIZE_YUV_MAIN1,    eBUFFER_IOTYPE_INPUT}, main1PartialParams.in.pResizedYuv);
        pushImageBuffer(ispPipeRequest, {PBID_IN_BINING_YUV_MAIN1,    eBUFFER_IOTYPE_INPUT}, main1PartialParams.in.pBinningYuv);

        pushMetadata(ispPipeRequest, {PBID_IN_HAL_META_MAIN1, eBUFFER_IOTYPE_INPUT}, main1PartialParams.in.pHalMeta);
        pushMetadata(ispPipeRequest, {PBID_IN_APP_META,       eBUFFER_IOTYPE_INPUT}, main1PartialParams.in.pAppMeta);

        // main2
        const TPNodePlugin::PartialParams& main2PartialParams = partialParamsCollection[main2Index];
        pushImageBuffer(ispPipeRequest, {PBID_IN_FULLSIZE_YUV_MAIN2,  eBUFFER_IOTYPE_INPUT}, main2PartialParams.in.pFullYuv);
        pushImageBuffer(ispPipeRequest, {PBID_IN_RESIZE_YUV_MAIN2,    eBUFFER_IOTYPE_INPUT}, main2PartialParams.in.pResizedYuv);
        pushMetadata(ispPipeRequest, {PBID_IN_HAL_META_MAIN2, eBUFFER_IOTYPE_INPUT}, main2PartialParams.in.pHalMeta);

        // last pair-frame
        if(partialParams.bIsLastPartial)
        {
            pushImageBuffer(ispPipeRequest, {PBID_OUT_BOKEH_YUV, eBUFFER_IOTYPE_OUTPUT},          partialParams.out.pFullYuv);
            pushImageBuffer(ispPipeRequest, {PBID_OUT_THIRDPARTY_DEPTH, eBUFFER_IOTYPE_OUTPUT},   partialParams.out.pDepth);
            pushImageBuffer(ispPipeRequest, {PBID_OUT_CLEAN_YUV, eBUFFER_IOTYPE_OUTPUT},          partialParams.out.pClean);

            pushMetadata(ispPipeRequest, {PBID_OUT_APP_META, eBUFFER_IOTYPE_OUTPUT},              partialParams.out.pAppMeta);
            pushMetadata(ispPipeRequest, {PBID_OUT_HAL_META, eBUFFER_IOTYPE_OUTPUT},              partialParams.out.pHalMeta);
        }

        sp<IStopWatchCollection> stopWatchCollection = IStopWatchCollection::createInstance("dcmf_isppipe", ispPipeReqNum);
        stopWatchCollection->begin("pipe_total", ispPipeRequest.get());
        ispPipeRequest->setStopWatchCollection(stopWatchCollection);

        MY_LOGD("enque request to ispPipe, pluginReqNum: %u, ispPipeReqNum: %u, addr: %p", pluginReqNum, ispPipeReqNum, ispPipeRequest.get());
        if(!mpIspPipe->enque(ispPipeRequest))
        {
            MY_LOGE("faile to enque request to ispPipe, pluginReqNum: %u, ispPipeReqNum: %u, addr: %p", pluginReqNum, ispPipeReqNum, ispPipeRequest.get());
        }
    }
    else
    {
        MY_LOGD("do not pair frame");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPluginIspPipeImp::
onRequestFinished(MVOID* tag, PipeResultState state, sp<IIspPipeRequest>& request)
{
    SCOPED_TRACER();

    const MUINT32 ispPipeReqNum = request->getRequestNo();
    const MUINT32 pluginReqNum = ::convertReqNumFromIspPipe2Plugin2(ispPipeReqNum);
    MY_LOGD("ispPipeReqNum: %u, pluginReqNum: %u", ispPipeReqNum, pluginReqNum);

    sp<IStopWatchCollection> stopWatchCollection = request->getStopWatchCollection();
    stopWatchCollection->end(request.get());
    stopWatchCollection->printResult();

    if(!::isLastIspPipeReqNum(ispPipeReqNum))
    {
        MY_LOGD("not last ispPipeReqNum");
    }
    else
    {
        DCMFPluginIspPipeImp* pImp = static_cast<DCMFPluginIspPipeImp*>(tag);
        TPNodePlugin::RequestParams params;
        std::lock_guard<std::mutex> guard(pImp->mLocker);
        {
            MINT32 key = pluginReqNum;
            auto it = pImp->mProcessingJobs.find(key);
            if(it == pImp->mProcessingJobs.end())
            {
                MY_LOGE("request is not existing, pluginReqNum: %u", pluginReqNum);
                return;
            }
            else
            {
                MY_LOGD("remove processing job, pluginReqNum: %u", pluginReqNum);
                params = pImp->mProcessingJobs[key];
                pImp->mProcessingJobs.erase(it);
            }

            MINT32 i = 0;
            for(auto item : pImp->mProcessingJobs)
            {
                MY_LOGD("in processingJobs, #:%d, key(req#):%u", i, item.first);
                i++;
            }
        }
        dumpImage(params);
        MY_LOGD("call parent response, pluginReqNum: %u", pluginReqNum);
        pImp->getParent().response(params, android::OK);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
DCMFPlugin::
DCMFPlugin()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.dg.dcmf.disable.isppipe", cLogLevel, "0");
    MINT32 disableIsppipeFlow = ::atoi(cLogLevel);
    if(disableIsppipeFlow)
    {
        mDCMFPluginImp = std::make_unique<DCMFPluginDufaultImp>(*this);
    }
    else
    {
        mDCMFPluginImp = std::make_unique<DCMFPluginIspPipeImp>(*this);
    }
    MY_LOGD("create DCMFPluginImp: name: %s", mDCMFPluginImp->getName().c_str());
}
/******************************************************************************
 *
 ******************************************************************************/
DCMFPlugin::
~DCMFPlugin()
{
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPlugin::
init()
{
    mDCMFPluginImp->handleInit();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPlugin::
uninit()
{
    mDCMFPluginImp->handleUninit();
}
/******************************************************************************
 *
 ******************************************************************************/
TPNodePlugin::PluginProfile&
DCMFPlugin::
profile()
{
    static PluginProfile pf = {
                    .id = MTK_PLUGIN_MODE_DCMF_3RD_PARTY,
                    .type = BLENDING
                };

    return pf;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPlugin::
process(RequestParams const& requestParams)
{
    const MUINT32 pluginReqNum = requestParams.uRequestNo;
    MY_LOGD("pluginReqNum: %u, count: %u", pluginReqNum, requestParams.uInCount);

    // dump param info
    for (size_t i = 0; i < requestParams.uInCount; i++)
    {
        MY_LOGD("[input-%zd] AppMeta: %p HalMeta: %p FullYuv: %p RszYuv: %p BinYuv: %p",
                i,
                requestParams.in[i].pAppMeta,
                requestParams.in[i].pHalMeta,
                requestParams.in[i].pFullYuv,
                requestParams.in[i].pResizedYuv,
                requestParams.in[i].pBinningYuv);
    }

    MY_LOGD("[output] AppMeta: %p HalMeta: %p FullYuv: %p Depth: %p Clean: %p",
            requestParams.out.pAppMeta,
            requestParams.out.pHalMeta,
            requestParams.out.pFullYuv,
            requestParams.out.pDepth,
            requestParams.out.pClean);

    mDCMFPluginImp->handleProcess(requestParams);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFPlugin::
onFrameQueue(PartialParams const& partialParams)
{
    const MUINT32 pluginReqNum = partialParams.uRequestNo;
    MY_LOGD("pluginReqNum: %u, bIsLastPartial: %d", pluginReqNum, partialParams.bIsLastPartial);

    MY_LOGD("[input] AppMeta: %p HalMeta: %p FullYuv: %p RszYuv: %p BinYuv: %p",
            partialParams.in.pAppMeta,
            partialParams.in.pHalMeta,
            partialParams.in.pFullYuv,
            partialParams.in.pResizedYuv,
            partialParams.in.pBinningYuv);

    MY_LOGD("[output] AppMeta: %p HalMeta: %p FullYuv: %p Depth: %p Clean: %p ",
            partialParams.out.pAppMeta,
            partialParams.out.pHalMeta,
            partialParams.out.pFullYuv,
            partialParams.out.pDepth,
            partialParams.out.pClean);

    mDCMFPluginImp->handleOnFrameQueue(partialParams);
}

