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

// Standard C header file
#include <functional>
#include <chrono>
#include <sstream>
// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file

// Module header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
// Local header file
#include "../DCMFIspPipeFlowUtility.h"
#include "DepthNode.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "DepthNode"
#include <featurePipe/core/include/PipeLog.h>

using namespace std;

namespace {
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
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ )


using NSCam::NSCamFeature::NSFeaturePipe::CamThread;
using AutoCamThreadDependency =std::unique_ptr<CamThread, std::function<MVOID(CamThread*)>>;

AutoCamThreadDependency
createAutoCamThreadDependency(CamThread* pCamThread)
{
    pCamThread->incExtThreadDependency();
    return AutoCamThreadDependency( pCamThread, [](CamThread* p)
        {
            p->decExtThreadDependency();
        });
}
#define AUTO_THREAD_DEPENDENCY() auto auto_camThread_dependency = ::createAutoCamThreadDependency(this)


using AutoTimer = std::unique_ptr<const char, std::function<void(const char*)>>;

AutoTimer
createAutoTimer(const char* text)
{
    using Timing = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime = std::chrono::duration<float, std::milli>;

    Timing startTime = std::chrono::high_resolution_clock::now();
    return AutoTimer(text, [startTime](const char* p)
            {
                Timing endTime = std::chrono::high_resolution_clock::now();
                DuationTime duationTime = endTime - startTime;
                MY_LOGD("%s, elapsed(ms):%.4f", p, duationTime.count());
            }
        );
}
#define AUTO_TIMER(TEXT) auto auto_timer = ::createAutoTimer(TEXT)
} // anonymous namespace for MARCO function

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {
/*******************************************************************************
* Const Definition
********************************************************************************/


/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IThirdPartyDepth
 * @brief interface for third party depth
 */
class IThirdPartyDepth : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyDepth Public Static Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IThirdPartyDepth* createInstance();

    static IThirdPartyDepth* getDefaultInstance();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyDepth Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init
     * @return
     * - execute result
     */
    virtual MBOOL init() = 0;
    /**
     * @brief uninit
     * @return
     * - execute result
     */
    virtual MBOOL uninit() = 0;
    /**
     * @brief do third party depth
     * @param [in] fsYUVMain1ImageBuf the image buffer of the main1 full YUV
     * @param [in] fsYUVMain2ImageBuf the image buffer of the main2 full YUV
     * @param [in] rsYUVMain1ImageBuf the image buffer of the main1 resize YUV
     * @param [in] hal1Meta main1 hal metadata
     * @param [in] hal2Meta main2 hal metadata
     * @param [out] depthMapImageBuf the image buffer of depthmap
     * @return
     * - execute result
     */
    virtual MBOOL execute(
            const IImageBuffer& fsYUVMain1ImageBuf,
            const IImageBuffer& fsYUVMain2ImageBuf,
            const IImageBuffer& rsYUVMain1ImageBuf,
            const IMetadata& hal1Meta,
            const IMetadata& hal2Meta,
            IImageBuffer& depthMapImageBuf) = 0;
    /**
     * @brief do third party depth
     * @param [in] fsYUVMain1ImageBuf the image buffer of the main1 full YUV
     * @param [in] fsYUVMain2ImageBuf the image buffer of the main2 full YUV
     * @param [in] rsYUVMain1ImageBuf the image buffer of the main1 resize YUV
     * @param [in] hal1Meta main1 hal metadata
     * @param [in] hal2Meta main2 hal metadata
     * @param [in] fdResult face detection result info
     * @param [out] depthMapImageBuf the image buffer of depthmap
     * @return
     * - execute result
     */
    virtual MBOOL execute(
            const IImageBuffer& fsYUVMain1ImageBuf,
            const IImageBuffer& fsYUVMain2ImageBuf,
            const IImageBuffer& rsYUVMain1ImageBuf,
            const IMetadata& hal1Meta,
            const IMetadata& hal2Meta,
            const FDResultInfo& fdResult,
            IImageBuffer& depthMapImageBuf) = 0;
    virtual ~IThirdPartyDepth(){};
};

namespace { // begin anonymous namespace
/**
 * @class DepthNodeHelper
 * @brief Helper class for DepthNode
 */
class DepthNodeHelper
{
public:
    DepthNodeHelper() = delete;

public:
    /**
     * @brief determine the target is intput or not
     * @param [in] requestPrt the target request
     * @return
     * - execute result
     */
    static inline MBOOL getIsInputRequest(const IspPipeRequestPtr& requestPrt);
    /**
     * @brief determine the target is input or not
     * @param [in] requestPrt the target request
     * @return
     * - execute result
     */
    static inline MBOOL getIsOutputRequest(const IspPipeRequestPtr& requestPrt);
    /**
     * @brief try to get the input request form given RequestCollection
     * @param [in] requestCollection the RequestCollection that search the input request
     * @param [out] inputRequest the found input request
     * @return
     * - execute result
     */
    static MBOOL tryGetInputRequest(const DepthNode::RequestCollection& requestCollection, IspPipeRequestPtr& inputRequest);
    /**
     * @brief try to get the output request form given RequestCollection
     * @param [in] requestCollection the RequestCollection that search the output request
     * @param [out] outputRequest the found output request
     * @return
     * - execute result
     */
    static MBOOL tryGetOutputRequest(const DepthNode::RequestCollection& requestCollection, IspPipeRequestPtr& outputRequest);
    /**
     * @brief try to get the depth map image size
     * @param [in] halMeta halMeta data
     * @param [out] depthMapSize the depth map image size
     * @return
     * - execute result
     */
    static MBOOL tryGetDepthMapSize(const IMetadata& halMeta, MSize& depthMapSize);
    /**
     * @brief try to get the depth map image buffer
     * @param [in] request the request the try to get the depth map image buffer
     * @param [in] nodeID current node ID
     * @param [out] depthMapSize the depth map image buffer
     * @return
     * - execute result
     */
    static MBOOL tryRequestDepthImgBuffer(const IspPipeRequestPtr& requestPtr, MINT32 nodeID, IImageBuffer*& pDepthImgBuffer);
    /**
     * @brief get the data ID name
     * @param [in] dataID the data ID
     * @return
     * - the name of argument dataID
     */
    static const char* getDataIDName(IspPipeDataHandler::DataID dataID);
    /**
     * @brief print depth related info
     * @param [in] halMeta the hal metadata
     * @param [in] fdResult face detection result info
     */
    static MVOID printDepthInfo(const IMetadata& halMeta, const FDResultInfo& fdResult);
    /**
     * @brief copy memery by the specific value to input imageBuf
     * @param [in] imageBuf image buffer that want to set the specific value
     * @param [in] value the specific value [0, 255]
     */
    static MVOID copyMemByValue(IImageBuffer& imageBuf, MUINT8 value);
};

/**
 * @class ThirdPartyDepthBase
 * @brief base(abstract) class for third party depth
 */
class ThirdPartyDepthBase : public IThirdPartyDepth
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyDepth Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init() override
    {
        return onInit();
    }

    MBOOL uninit() override
    {
        return onUninit();
    }

    MBOOL execute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        IImageBuffer& depthMapImageBuf) override
    {
        return onExecute(fsYUVMain1ImageBuf, fsYUVMain2ImageBuf, rsYUVMain1ImageBuf, hal1Meta, hal2Meta, depthMapImageBuf);
    }

    MBOOL execute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImageBuf) override
    {
        return onExecute(fsYUVMain1ImageBuf, fsYUVMain2ImageBuf, rsYUVMain1ImageBuf, hal1Meta, hal2Meta, fdResult, depthMapImageBuf);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDepthBase protected Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ThirdPartyDepthBase() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDepthBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    virtual MBOOL onInit() { return MTRUE; };

    virtual MBOOL onUninit() { return MTRUE; };

    virtual MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        IImageBuffer& depthMapImageBuf)
    {
        return MTRUE;
    };

    virtual MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImageBuf)
    {
        return MTRUE;
    }
};

/**
 * @class DefaultThirdPartyDepth
 * @brief default implementation for third party depth
 */
class DefaultThirdPartyDepth final: public ThirdPartyDepthBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DefaultThirdPartyDepth Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DefaultThirdPartyDepth();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DefaultThirdPartyDepth non-virtual interface (NVI)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        IImageBuffer& depthMapImageBuf) override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImageBuf) override;
};

/**
 * @class ThirdPartyDepthImp
 * @brief implementation for third party depth
 */
class ThirdPartyDepthImp final: public ThirdPartyDepthBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDepthImp Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyDepthImp();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDepthBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL onInit() override;

    MBOOL onUninit() override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        IImageBuffer& depthMapImageBuf) override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IImageBuffer& rsYUVMain1ImageBuf,
        const IMetadata& hal1Meta,
        const IMetadata& hal2Meta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImageBuf) override;
};


} // end anonymous namespace


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DepthNodeHelper::
getIsInputRequest(const IspPipeRequestPtr& requestPrt)
{
    return requestPrt->isRequestBuffer(BID_IN_FSYUV_MAIN1)
            && requestPrt->isRequestBuffer(BID_IN_FSYUV_MAIN2)
            && requestPrt->isRequestBuffer(BID_IN_RSYUV_MAIN2)
            && requestPrt->isRequestBuffer(BID_META_IN_HAL_MAIN1)
            && requestPrt->isRequestBuffer(BID_META_IN_HAL_MAIN2);
}

MBOOL
DepthNodeHelper::
getIsOutputRequest(const IspPipeRequestPtr& requestPrt)
{
    return requestPrt->isRequestBuffer(BID_OUT_DEPTHMAP);
}

MBOOL
DepthNodeHelper::
tryGetInputRequest(const DepthNode::RequestCollection& requestCollection, IspPipeRequestPtr& inputRequest)
{
    IspPipeRequestPtr found = nullptr;
    for(auto& item : requestCollection)
    {
        if(DepthNodeHelper::getIsInputRequest(item))
        {
            if(found == nullptr)
            {
                found = item;
                MY_LOGD("found the input request: req#=%d", found->getRequestNo());
            }
            else
            {
                MY_LOGE("found the more than one input request: currentFoundReq#=%d, perivousFoundReq#=%d",
                    item->getRequestNo(), found->getRequestNo());

                return MFALSE;
            }
        }
    }
    return (inputRequest = found) != nullptr;
}

MBOOL
DepthNodeHelper::
tryGetOutputRequest(const DepthNode::RequestCollection& requestCollection, IspPipeRequestPtr& outputRequest)
{
    IspPipeRequestPtr found = nullptr;
    for(auto& item : requestCollection)
    {
        if(DepthNodeHelper::getIsOutputRequest(item))
        {
            if(found == nullptr)
            {
                found = item;
                MY_LOGD("found the output request: req#=%d", found->getRequestNo());
            }
            else
            {
                MY_LOGE("found the more than one output request: currentFoundReq#=%d, perivousFoundReq#=%d",
                    item->getRequestNo(), found->getRequestNo());

                return MFALSE;
            }
        }
    }
    return (outputRequest = found) != nullptr;
}

MBOOL
DepthNodeHelper::
tryGetDepthMapSize(const IMetadata& halMeta, MSize& depthMapSize)
{
    MBOOL bRet = MFALSE;

    IMetadata::IEntry entry = halMeta.entryFor(MTK_STEREO_DCMF_DEPTHMAP_SIZE);
    if(!entry.isEmpty())
    {
        depthMapSize.w = entry.itemAt(0, Type2Type<MINT32>());
        depthMapSize.h = entry.itemAt(1, Type2Type<MINT32>());

        bRet = MTRUE;
    }
    else
    {
        MY_LOGD("failed to get hal meta: MTK_STEREO_DCMF_DEPTHMAP_SIZE");
    }
    return bRet;
}

MBOOL
DepthNodeHelper::
tryRequestDepthImgBuffer(const IspPipeRequestPtr& requestPtr, MINT32 nodeID, IImageBuffer*& pDepthImgBuffer)
{
    SCOPED_TRACER();

    IMetadata* halMain1Meta = requestPtr->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    if(halMain1Meta == nullptr)
    {
        MY_LOGE("failed to get halMain1Meta");
        return MFALSE;
    }

    MSize depthImgSize(0, 0);
    if(!DepthNodeHelper::tryGetDepthMapSize(*halMain1Meta, depthImgSize))
    {
        MY_LOGE("failed to get depthMapSize");
        return MFALSE;
    }
    MY_LOGD("get depthImgSize:%dx%d", depthImgSize.w, depthImgSize.h);

    {
        AUTO_TIMER("request internal depthImgBuffer time");
        pDepthImgBuffer = requestPtr->getBufferHandler()->requestBuffer(nodeID, BID_DEPTHMAP_INERNAL_Y8);
    }

    if(pDepthImgBuffer == nullptr)
    {
        MY_LOGE("failed to get internal depthImgBuffer, req#:%d", requestPtr->getRequestNo());
        return MFALSE;
    }

    MSize oldDepthImgSize = pDepthImgBuffer->getImgSize();
    if(oldDepthImgSize != depthImgSize)
    {
        MY_LOGD("adjust internal depthImgSize, new:%dx%d, old:%dx%d",
            depthImgSize.w, depthImgSize.h, oldDepthImgSize.w, oldDepthImgSize.h);

        pDepthImgBuffer->setExtParam(depthImgSize, 0);
    }
    return MTRUE;
}

const char*
DepthNodeHelper::
getDataIDName(IspPipeDataHandler::DataID dataID)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(dataID)
      {
        MAKE_NAME_CASE(ROOT_TO_DEPTH);
        MAKE_NAME_CASE(FD_TO_DEPTH);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}

MVOID
DepthNodeHelper::
printDepthInfo(const IMetadata& halMeta, const FDResultInfo& fdResult)
{
    SCOPED_TRACER();

    MSize depthMapSize(-1, 1);
    if(!DepthNodeHelper::tryGetDepthMapSize(halMeta, depthMapSize))
    {
        MY_LOGW("failed to get depth map size");
    }

    const std::string tag = "[Depth_info] ";
    std::ostringstream stringStream;
    stringStream << tag << "************************************" << std::endl;
    stringStream << "depthMapSize: (" << depthMapSize.w << ", " << depthMapSize.h << ")" << std::endl;
    for(int i = 0; i < fdResult.miFaceNum; i++)
    {
        const MRect& region = fdResult.mRIPDegree[i];
        stringStream << tag << "face#: "<< i << " ,"
                     << "degree: " << fdResult.mRIPDegree[i] << " ,"
                     << "region: " << "(" << region.p.x << " ," << region.p.y << " ," << region.s.w << " ," << region.s.h << ")" << std::endl;
    }
    stringStream << tag << "************************************" << std::endl;

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", stringStream.str().c_str());
}

MVOID
DepthNodeHelper::
copyMemByValue(IImageBuffer& imageBuf, MUINT8 value)
{
    MVOID* pimgVa = reinterpret_cast<MVOID*>(imageBuf.getBufVA(0));
    size_t imgSize = imageBuf.getBufSizeInBytes(0);
    MY_LOGD("va:%p ,size:%zu, value:%u", pimgVa, imgSize, value);
    memset(pimgVa, value, imgSize);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthNode::
DepthNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: DepthNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

DepthNode::
DepthNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
, mThirdPartyDepth(IThirdPartyDepth::createInstance())
, mDepthResultImg(nullptr)

{
    SCOPED_TRACER();

    MY_LOGD("ctor:%p", this);

    this->addWaitQueue(&mRequestQue);
}

DepthNode::
~DepthNode()
{
    SCOPED_TRACER();

    MY_LOGD("dctor:%p, processingRequestsSize:%zu", this, mProcessingRequests.size());

    // print the contain of processingRequests
    int i = 0;
    for (auto it = mProcessingRequests.begin(); it != mProcessingRequests.end(); it++)
    {
        auto& item = *it;
        MY_LOGW("in ProcessingRequests(non-processing), #%d, req#:%d", i, item->getRequestNo());
        i++;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
DepthNode::
onInit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyDepth->init();
    if(!bRet)
    {
        MY_LOGE("Failed to init 3rd part depth");
    }
    return bRet;
}

MBOOL
DepthNode::
onUninit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyDepth->uninit();
    if(!bRet)
    {
        MY_LOGE("Failed to uninit 3rd part depth");
    }
    return bRet;
}

MBOOL
DepthNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
DepthNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
DepthNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    MY_LOGD("reqID=%d, dataId:%d(%s)", pRequest->getRequestNo(), dataID, DepthNodeHelper::getDataIDName(dataID));

    MBOOL ret = MTRUE;
    switch(dataID)
    {
        case ROOT_TO_DEPTH:
        case FD_TO_DEPTH:
            mRequestQue.enque(pRequest);
            break;
        default:
            MY_LOGW("Un-recognized dataID ID, id=%s reqID=%d", ID2Name(dataID), pRequest->getRequestNo());
            ret = MFALSE;
            break;
    }
    return ret;
}

MVOID
DepthNode::
onFlush()
{
    SCOPED_TRACER();

    MY_LOGD("extDep=%d", this->getExtThreadDependency());
    IspPipeRequestPtr pRequest;
    while( mRequestQue.deque(pRequest) )
    {
        sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    IspPipeNode::onFlush();
}

MBOOL
DepthNode::
onThreadLoop()
{
    SCOPED_TRACER();

    IspPipeRequestPtr pRequest;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequestQue.deque(pRequest) )
    {
        MY_LOGE("mRequestQue.deque() failed");
        return MFALSE;
    }

    AUTO_THREAD_DEPENDENCY();
    AutoProfileLogging profile("DepthNode::onThreadLoop", pRequest->getRequestNo());

    mProcessingRequests.push_back(pRequest);
    MY_LOGD("push request to requestCollection(size:%zu), req#:%d", mProcessingRequests.size(), pRequest->getRequestNo());

    MBOOL isOK = MTRUE;
    const MBOOL isInputRequest = DepthNodeHelper::getIsInputRequest(pRequest);
    const MBOOL isOutputRequest = DepthNodeHelper::getIsOutputRequest(pRequest);
    const MBOOL isFDToDepthFlow = pRequest->getRequestAttr().isFDToDepthFlow;

    if(isFDToDepthFlow)
    {
        if(isOutputRequest)
        {
            if(executeDepth(mProcessingRequests) != nullptr)
            {
                MY_LOGD("success to executeDepth, req#:%d", pRequest->getRequestNo());
            }
            else
            {
                MY_LOGW("failed to executeDepth, req#:%d", pRequest->getRequestNo());
                pRequest->getRequestAttr().isDepthExecuteFailed = MTRUE;
            }
            pRequest->setOutputBufferReady(BID_OUT_DEPTHMAP);
        }
    }
    else
    {
        // input
        if(isInputRequest)
        {
            if(mDepthResultImg != nullptr)
            {
                MY_LOGE("get more than one inputRequest to executeDepth, req#:%d", pRequest->getRequestNo());
                isOK = MFALSE;
            }
            else
            {
                MY_LOGD("executeDepth, req#: %d", pRequest->getRequestNo());
                mDepthResultImg = executeDepth(mProcessingRequests);
                if(mDepthResultImg != nullptr)
                {
                    MY_LOGD("success to executeDepth, req#:%d", pRequest->getRequestNo());
                }
                else
                {
                    MY_LOGW("failed to executeDepth, req#:%d", pRequest->getRequestNo());
                }
            }
        }
        // output
        if(isOK && isOutputRequest)
        {
            MY_LOGD("get outputRequest, req#: %d", pRequest->getRequestNo());

            if(mDepthResultImg == nullptr)
            {
                MY_LOGW("set attritube for failed to execute depth, req#:%d", pRequest->getRequestNo());
                pRequest->getRequestAttr().isDepthExecuteFailed = MTRUE;
                pRequest->setOutputBufferReady(BID_OUT_DEPTHMAP);
            }
            else
            {
                IImageBuffer *pImgBuf_outDepthmap = nullptr;
                pRequest->getBufferHandler()->getEnqueBuffer(getNodeId(), BID_OUT_DEPTHMAP, pImgBuf_outDepthmap);

                const MSize srcSize = {mDepthResultImg->getImgSize().w, mDepthResultImg->getImgSize().h};
                const MSize dstSize = {pImgBuf_outDepthmap->getImgSize().w, pImgBuf_outDepthmap->getImgSize().h};
                MY_LOGD("copy deptpImgBuffer from internal to output, srcImgSize:%dx%d, dstImgSize:%dx%d",
                    srcSize.w, srcSize.h, dstSize.w, dstSize.h);

                MRect crop = MRect(srcSize.w, srcSize.h);
                if(DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_outDepthmap, mDepthResultImg, crop))
                {
                    MY_LOGD("success to execute imgCopy from internal to output, req#:%d", pRequest->getRequestNo());
                    pRequest->setOutputBufferReady(BID_OUT_DEPTHMAP);
                }
                else
                {
                    MY_LOGE("fail to execute imgCopy from internal to output");
                    isOK = MFALSE;
                }
            }
        }
    }

    // handle error or final
    if(!isOK || isOutputRequest)
    {
        DataID dataId = DEPTH_TO_BOKEH;
        MBOOL (DepthNode::*handleDataOperatorPtr) (DataID, const IspPipeRequestPtr&) = &DepthNode::handleDataAndDump;
        if(!isOK)
        {
            handleDataOperatorPtr = &DepthNode::handleData;
            dataId = ERROR_OCCUR_NOTIFY;
        }

        MY_LOGD("requests are finishing, the resion is, error:%d, isOutputRequest:%d", !isOK, isOutputRequest);
        for(auto& item : mProcessingRequests)
        {
            MY_LOGD("dispatch to next node, req#:%d", item->getRequestNo());
            (this->*handleDataOperatorPtr)(dataId, item);
            item->getBufferHandler()->onProcessDone(getNodeId());
        }
        mProcessingRequests.clear();
        mDepthResultImg = nullptr;

        MY_LOGD("processingRequests size:%zu", mProcessingRequests.size());
    }
    return isOK;
}

IImageBuffer*
DepthNode::
executeDepth(RequestCollection& requestCollection)
{
    SCOPED_TRACER();

    IspPipeRequestPtr inputRequest = nullptr;
    if(!DepthNodeHelper::tryGetInputRequest(requestCollection, inputRequest))
    {
        MY_LOGE("failed to get the input request");
        return nullptr;
    }

    const MBOOL isDisableDepth = inputRequest->getRequestAttr().isDisableDepth;
    const MBOOL isFDToDepthFlow = inputRequest->getRequestAttr().isFDToDepthFlow;
    MY_LOGD("flow control of executing depth, isFDToDepthFlow:%d, isDisableDepth:%d", isFDToDepthFlow, isDisableDepth);

    sp<PipeBufferHandler> inputBufferHandlerPtr = inputRequest->getBufferHandler();
     // input main1 full YUV
    IImageBuffer *pImgBuf_inFSYUVMain1 = nullptr;
    // input main2 full YUV
    IImageBuffer *pImgBuf_inFSYUVMain2 = nullptr;
    // input main1 resize YUV
    IImageBuffer *pImgBuf_inRSYUVMain1 = nullptr;
    // output depthmap
    IImageBuffer *pImgBuf_outDepthmap = nullptr;
    // main1 hal data
    IMetadata* pHalMetaMain1 = nullptr;
    // main2 hal data
    IMetadata* pHalMetaMain2 = nullptr;
    // fd result
    FDResultInfo* pFDResultInfo = nullptr;

    // input
    {
        inputBufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inFSYUVMain1);
        inputBufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN2, pImgBuf_inFSYUVMain2);
        inputBufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_IN_RSYUV_MAIN2, pImgBuf_inRSYUVMain1);
        pHalMetaMain1 = inputBufferHandlerPtr->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
        pHalMetaMain2 = inputBufferHandlerPtr->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN2);

        const MBOOL isValidatedInput = pImgBuf_inFSYUVMain1
                                            && pImgBuf_inFSYUVMain2
                                            && pImgBuf_inRSYUVMain1
                                            && pHalMetaMain1
                                            && pHalMetaMain2;
        if(!isValidatedInput)
        {
            MY_LOGE("invalid input, main1FSYUV:%p, main2FSYUV:%p, main1RSYUV:%p, halMetaMain1: %p, halMetaMain2: %p",
                pImgBuf_inFSYUVMain1, pImgBuf_inFSYUVMain2, pImgBuf_inRSYUVMain1, pHalMetaMain1, pHalMetaMain2);

            return nullptr;
        }
    }
    // output
    {
        // note: isFDToDepthFlow need the output request, otherwise doesn't
        if(isFDToDepthFlow)
        {
            IspPipeRequestPtr outputRequest = nullptr;
            if(!DepthNodeHelper::tryGetOutputRequest(requestCollection, outputRequest))
            {
                MY_LOGE("failed to get the output request");
                return nullptr;
            }
            outputRequest->getBufferHandler()->getEnqueBuffer(getNodeId(), BID_OUT_DEPTHMAP, pImgBuf_outDepthmap);
            pFDResultInfo = outputRequest->getRequestAttr().fdResult.get();

            const MBOOL isValidatedOutput = pImgBuf_outDepthmap && pFDResultInfo;
            if(!isValidatedOutput)
            {
                MY_LOGE("invalid output, outDepthmap:%p, fdResultInfo:%p",pImgBuf_outDepthmap, pFDResultInfo);
                return nullptr;
            }
        }
        else
        {
            MSize outputDepthMapSize(0, 0);
            if(!DepthNodeHelper::tryRequestDepthImgBuffer(inputRequest, getNodeId(), pImgBuf_outDepthmap))
            {
                MY_LOGE("failed to get requestDepthImgBuffer");
                return nullptr;
            }
        }
    }
    // execute depth
    {
        IThirdPartyDepth* pThirdPartyDepth = nullptr;
        if(isDisableDepth)
        {
            MY_LOGD("disableDepth, using defaultThirdPartyDepthInstance");
            pThirdPartyDepth = IThirdPartyDepth::getDefaultInstance();
        }
        else
        {
            MY_LOGD("enableDepth, using thirdPartyDepthInstance");
            pThirdPartyDepth = mThirdPartyDepth.get();
        }

        MBOOL isOk = MFALSE;
        {
            AutoProfileLogging profile("DepthNode::execute_3rd_party_depth", inputRequest->getRequestNo());
            sp<IStopWatchCollection> stopWatchCollection = inputRequest->getStopWatchCollection();
            auto stopWatch = stopWatchCollection->getStopWatch(StopWatchType::Auto, "execute_3rd_party_depth");

            pImgBuf_inFSYUVMain1->syncCache(eCACHECTRL_FLUSH);
            pImgBuf_inFSYUVMain2->syncCache(eCACHECTRL_FLUSH);
            pImgBuf_inRSYUVMain1->syncCache(eCACHECTRL_FLUSH);
            pImgBuf_outDepthmap->syncCache(eCACHECTRL_FLUSH);

            MBOOL isOK = MFALSE;
            if(isFDToDepthFlow)
            {
                isOK = pThirdPartyDepth->execute( *pImgBuf_inFSYUVMain1,
                                                  *pImgBuf_inFSYUVMain2,
                                                  *pImgBuf_inRSYUVMain1,
                                                  *pHalMetaMain1,
                                                  *pHalMetaMain2,
                                                  *pFDResultInfo,
                                                  *pImgBuf_outDepthmap);
            }
            else
            {
                isOK = pThirdPartyDepth->execute( *pImgBuf_inFSYUVMain1,
                                                  *pImgBuf_inFSYUVMain2,
                                                  *pImgBuf_inRSYUVMain1,
                                                  *pHalMetaMain1,
                                                  *pHalMetaMain2,
                                                  *pImgBuf_outDepthmap);
            }

            if(!isOK)
            {
                MY_LOGW("failed to execute third-party depth");
                return nullptr;
            }
        }
    }
    return pImgBuf_outDepthmap;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyDepth Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IThirdPartyDepth*
IThirdPartyDepth::
createInstance()
{
    SCOPED_TRACER();

    MBOOL isCreateDefault = (::property_get_int32("vendor.dg.isppipe.depth.default", 1) == 1 ) ? MTRUE : MFALSE;
    if(isCreateDefault)
    {
        MY_LOGD("create DefaultThirdPartyDepth");
        return new DefaultThirdPartyDepth();
    }
    return new ThirdPartyDepthImp();
}

IThirdPartyDepth*
IThirdPartyDepth::
getDefaultInstance()
{
    SCOPED_TRACER();
    static std::unique_ptr<IThirdPartyDepth> defaultInstant(new DefaultThirdPartyDepth());
    return defaultInstant.get();
}

DefaultThirdPartyDepth::
DefaultThirdPartyDepth()
: ThirdPartyDepthBase()
{
    SCOPED_TRACER();
}

MBOOL
DefaultThirdPartyDepth::
onExecute(
    const IImageBuffer& fsYUVMain1ImageBuf,
    const IImageBuffer& fsYUVMain2ImageBuf,
    const IImageBuffer& rsYUVMain1ImageBuf,
    const IMetadata& hal1Meta,
    const IMetadata& hal2Meta,
    IImageBuffer& depthMapImageBuf)
{
    SCOPED_TRACER();

    DepthNodeHelper::copyMemByValue(depthMapImageBuf, 255);
    return MTRUE;
}

MBOOL
DefaultThirdPartyDepth::
onExecute(
    const IImageBuffer& fsYUVMain1ImageBuf,
    const IImageBuffer& fsYUVMain2ImageBuf,
    const IImageBuffer& rsYUVMain1ImageBuf,
    const IMetadata& hal1Meta,
    const IMetadata& hal2Meta,
    const FDResultInfo& fdResult,
    IImageBuffer& depthMapImageBuf)
{
    SCOPED_TRACER();

    DepthNodeHelper::printDepthInfo(hal1Meta, fdResult);
    DepthNodeHelper::copyMemByValue(depthMapImageBuf, 255);
    return MTRUE;
}

ThirdPartyDepthImp::
ThirdPartyDepthImp()
: ThirdPartyDepthBase()
{

}

MBOOL
ThirdPartyDepthImp::
onInit()
{
    // TODO:
    // init third party depth here
    return MTRUE;
}

MBOOL
ThirdPartyDepthImp::
onUninit()
{
    // TODO:
    // uninit third party depth here
    return MTRUE;
}

MBOOL
ThirdPartyDepthImp::
onExecute(
    const IImageBuffer& fsYUVMain1ImageBuf,
    const IImageBuffer& fsYUVMain2ImageBuf,
    const IImageBuffer& rsYUVMain1ImageBuf,
    const IMetadata& hal1Meta,
    const IMetadata& hal2Meta,
    IImageBuffer& depthMapImageBuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party depth here
    return MTRUE;
}

MBOOL
ThirdPartyDepthImp::
onExecute(
    const IImageBuffer& fsYUVMain1ImageBuf,
    const IImageBuffer& fsYUVMain2ImageBuf,
    const IImageBuffer& rsYUVMain1ImageBuf,
    const IMetadata& hal1Meta,
    const IMetadata& hal2Meta,
    const FDResultInfo& fdResult,
    IImageBuffer& depthMapImageBuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party depth here
    return MTRUE;
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

