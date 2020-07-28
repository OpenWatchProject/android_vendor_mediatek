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
#include <algorithm>
#include <sstream>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
// Module header file
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
// Local header file
#include "../DCMFIspPipeFlowUtility.h"
#include "BokehNode.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "BokehNode"
#include <featurePipe/core/include/PipeLog.h>

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
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );

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

using NSCam::NSIoPipe::NSSImager::IImageTransform;
using namespace android::NSShot;

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IThirdPartyBokeh
 * @brief interface for third party bokeh
 */
class IThirdPartyBokeh : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyBokeh Public Static Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IThirdPartyBokeh* createInstance();

    static IThirdPartyBokeh* getDefaultInstance();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyBokeh Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /*
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
     * @brief do third party bokeh
     * @param [in] fsYUVCleanImgbuf the image buffer of the full size clean YUV
     * @param [in] depthMapImgBuf the image buffer of the depthmap
     * @param [in] appMeta app metadata
     * @param [in] fdResult face detection result info
     * @param [out] fsYUVBokehImgbuf the image buffer of the full size bokeh YUV
     * @return
     * - execute result
     */
    virtual MBOOL execute(
        const IImageBuffer& fsYUVCleanImgbuf,
        const IImageBuffer& depthMapImgBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& fsYUVBokehImgbuf) = 0;
    /**
     * @brief do third party bokeh
     * @param [in] fsYUVMain1ImageBuf the image buffer of the main1 full YUV
     * @param [in] fsYUVMain2ImageBuf the image buffer of the main2 full YUV
     * @param [in] appMeta app metadata
     * @param [in] fdResult face detection result info
     * @param [out] depthMapImgBuf the image buffer of the depthmap
     * @param [out] fsYUVCleanImgbuf the image buffer of the full size clean YUV
     * @param [out] fsYUVBokehImgbuf the image buffer of the full size bokeh YUV
     * @return
     * - execute result
     */
    virtual MBOOL execute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf) = 0;

    virtual ~IThirdPartyBokeh(){};
};

/**
 * @class class RequestReceivedCounter
 * @brief manager for receiving request count
 */
class RequestReceivedCounter final
{
public:
    using RequestNum = MINT32;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
   ~RequestReceivedCounter();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RequestReceivedCounter Public Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief set the max. received count
     * @param [in] value new value
     */
    MVOID setMaxReceivedCount(MINT32 value);
    /**
     * @brief get the max. received count
     * @return
     * - the received count
     */
    MINT32 getMaxReceivedCount();
    /**
     * @brief decrease received count for the specific request, if it is not existing create the new one
     * @param [in] requestNum request number
     * @return
     * - old value
     */
    MINT32 decreaseReceivedCount(RequestNum requestNum);
    /**
     * @brief get received count for the specific request
     * @param [in] requestNum request number
     */
    MINT32 getReceivedCount(RequestNum requestNum);
    /**
     * @brief remove the specific request
     * @param [in] requestNum request number
     */
    MVOID removeRequest(RequestNum requestNum);
    /**
     * @brief determine whether received all requests or did not
     * @param [in] requestNum request number
     * @return
     * - execute result
     */
    MBOOL isReceivedAllRequests(RequestNum requestNum);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RequestReceivedCounter Priver Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 mReceivedCount;

    std::map<RequestNum, MINT32> mTable;
};

namespace { // begin anonymous namespace
/**
 * @class BokehNodeHelper
 * @brief Helper class for DepthNode
 */
class BokehNodeHelper
{
public:
    BokehNodeHelper() = delete;

public:
    /**
     * @brief determine the target is request output or not
     * @param [in] requestPrt the target request
     * @return
     * - execute result
     */
    static MBOOL getIsOutputRequest(const IspPipeRequestPtr& requestptr);
    /**
     * @brief print bokeh related info
     * @param [in] appMeta app metadata
     * @param [in] fdResultInfo face detection result info
     */
    static MVOID printBokehInfo(const IMetadata& appMeta, const FDResultInfo& fdResultInfo);
    /**
     * @brief get the data ID name
     * @param [in] dataID the data ID
     * @return
     * - the name of atgument dataID
     */
    static const char* getDataIDName(IspPipeDataHandler::DataID dataID);
};

/**
 * @class ThirdPartyBokehBase
 * @brief base(abstract) class for third party bokeh
 */
class ThirdPartyBokehBase : public IThirdPartyBokeh
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyBokeh Public Operator
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
        const IImageBuffer& fsYUVCleanImgbuf,
        const IImageBuffer& depthMapImgBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& fsYUVBokehImgbuf) override
    {
        return onExecute(fsYUVCleanImgbuf, depthMapImgBuf, appMeta, halMeta, fdResult, fsYUVBokehImgbuf);
    }

    MBOOL execute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf) override
    {
        return onExecute(fsYUVMain1ImageBuf, fsYUVMain2ImageBuf, appMeta, halMeta, fdResult, depthMapImgBuf, fsYUVCleanImgbuf, fsYUVBokehImgbuf);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehBase protected Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ThirdPartyBokehBase() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    virtual MBOOL onInit() { return MTRUE; };

    virtual MBOOL onUninit() { return MTRUE; };

    virtual MBOOL onExecute(
        const IImageBuffer& fsYUVCleanImgbuf,
        const IImageBuffer& depthMapImgBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& fsYUVBokehImgbuf)
    {
        return MTRUE;
    };

    virtual MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf)
    {
        return MTRUE;
    };
};

/**
 * @class ThirdPartyBokehUtility
 * @brief utility class for third party bokeh
 */
class ThirdPartyBokehUtility final
{
public:
    ThirdPartyBokehUtility() = delete;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehUtility Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief get DoF level
     * @param [in] appMeta app metadata
     * @param [out] dofLevel DoF level
     * @return
     * - execute result
     */
    static MBOOL tryGetDofLevel(const IMetadata& appMeta, MINT32& dofLevel);
    /**
     * @brief get AF status
     * @param [in] appMeta app metadata
     * @param [out] afStatus AF status
     * @return
     * - execute result
     */
    static MBOOL tryGetAFStatus(const IMetadata& appMeta, MUINT8& afStatus);
    /**
     * @brief get AF ROI
     * @param [in] appMeta app metadata
     * @param [out] afRoi AF ROI
     * @return
     * - execute result
     */
    static MBOOL tryGetAFRoi(const IMetadata& appMeta, MRect& afRoi);
};
/**
 * @class DefaultThirdPartyBokeh
 * @brief default implementation for third party bokeh
 */
class DefaultThirdPartyBokeh final: public ThirdPartyBokehBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DefaultThirdPartyBokeh Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DefaultThirdPartyBokeh();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL onExecute(
        const IImageBuffer& fsYUVCleanImgbuf,
        const IImageBuffer& depthMapImgBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& fsYUVBokehImgbuf) override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf) override;


};

/**
 * @class ThirdPartyBokehImp
 * @brief implementation for third party bokeh
 */
class ThirdPartyBokehImp final: public ThirdPartyBokehBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehImp Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyBokehImp();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyBokehBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL onInit() override;

    MBOOL onUninit() override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVCleanImgbuf,
        const IImageBuffer& depthMapImgBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& fsYUVBokehImgbuf) override;

    MBOOL onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf) override;
};
/*******************************************************************************
* Const Definition
********************************************************************************/


} // end anonymous namespace

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RequestReceivedCounter Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
RequestReceivedCounter::
~RequestReceivedCounter()
{
    for(auto& item : mTable)
    {
        MY_LOGW("un-remove item: key(reqNum):%d, receivedCount:%d",
            item.first, item.second);
    }
}

MVOID
RequestReceivedCounter::
setMaxReceivedCount(MINT32 value)
{
    MY_LOGD("set value, new:%d, old:%d", value, mReceivedCount);
    mReceivedCount = value;
}

MINT32
RequestReceivedCounter::
getMaxReceivedCount()
{
    return mReceivedCount;
}

MINT32
RequestReceivedCounter::
decreaseReceivedCount(RequestNum requestNum)
{
    MINT32 old = 0;

    auto it = mTable.find(requestNum);
    if(it != mTable.end())
    {
        MINT32 old = it->second;
        it->second++;
    }
    else
    {
        MY_LOGD("add new item, requestNum:%d", requestNum);
        mTable[requestNum] = 1;
    }
    return old;
}

MINT32
RequestReceivedCounter::
getReceivedCount(RequestNum requestNum)
{
    MINT32 ret = -1;

    auto it = mTable.find(requestNum);
    if(it != mTable.end())
    {
        ret = mTable[requestNum];
    }
    else
    {
        MY_LOGW("item is not existing, requestNum:%d", requestNum);
    }
    return ret;
}

MVOID
RequestReceivedCounter::
removeRequest(RequestNum requestNum)
{
    auto it = mTable.find(requestNum);
    if(it != mTable.end())
    {
        mTable.erase(it);
    }
    else
    {
        MY_LOGW("item is not existing, requestNum:%d", requestNum);
    }
}

MBOOL
RequestReceivedCounter::
isReceivedAllRequests(RequestNum requestNum)
{
    MBOOL ret = MFALSE;

    auto it = mTable.find(requestNum);
    if(it != mTable.end())
    {
        ret = (mTable[requestNum] >= mReceivedCount);
    }
    else
    {
        MY_LOGW("item is not existing, requestNum:%d", requestNum);
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BokehNodeHelper Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
BokehNodeHelper::
getIsOutputRequest(const IspPipeRequestPtr& requestptr)
{
    return requestptr->isRequestBuffer(BID_OUT_CLEAN_FSYUV) && requestptr->isRequestBuffer(BID_OUT_DEPTHMAP);
}

MVOID
BokehNodeHelper::
printBokehInfo(const IMetadata& appMeta, const FDResultInfo& fdResult)
{
    SCOPED_TRACER();

    MINT32 dofLevel = 0;
    if(ThirdPartyBokehUtility::tryGetDofLevel(appMeta, dofLevel))
    {
        MY_LOGD("dofLevel(%d)", dofLevel);
    }

    MUINT8 afStatus = 0;
    if(ThirdPartyBokehUtility::tryGetAFStatus(appMeta, afStatus))
    {
        MY_LOGD("af status(%d)", afStatus);
    }

    MRect afRoi;
    if(ThirdPartyBokehUtility::tryGetAFRoi(appMeta, afRoi))
    {
        MY_LOGD("afRoi(%d, %d, %d, %d)", afRoi.p.x, afRoi.p.y, afRoi.s.w, afRoi.s.h);
    }

    const std::string tag = "[bokeh_info] ";
    std::ostringstream stringStream;
    stringStream << tag << "************************************" << std::endl;
    stringStream << tag << "dofLevel: " << dofLevel << " ," << "afStatus: " << static_cast<MINT32>(afStatus) << " ,"
                        << "afRoi: " << "(" << afRoi.p.x << " ," << afRoi.p.y << " ," << afRoi.s.w << " ," << afRoi.s.h << ")" << " ,"
                        << "faceNum: "<< fdResult.miFaceNum << std::endl;
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

const char*
BokehNodeHelper::getDataIDName(IspPipeDataHandler::DataID dataID)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(dataID)
      {
        MAKE_NAME_CASE(FD_TO_BOKEH);
        MAKE_NAME_CASE(MFNR_TO_BOKEH);
        MAKE_NAME_CASE(DEPTH_TO_BOKEH);
        MAKE_NAME_CASE(HDR_TO_BOKEH);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BokehNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BokehNode::
BokehNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: BokehNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

BokehNode::
BokehNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
, mThirdPartyBokeh(IThirdPartyBokeh::createInstance())
, mRequestReceivedCounter(new RequestReceivedCounter())
{
    SCOPED_TRACER();

    MY_LOGD("ctor:%p", this);

    this->addWaitQueue(&mRequestQue);
}

BokehNode::
~BokehNode()
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

MVOID
BokehNode::
setMaxReceivedRequestCount(MINT32 value)
{
    mRequestReceivedCounter->setMaxReceivedCount(value);
}

MBOOL
BokehNode::
onInit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyBokeh->init();
    if(!bRet)
    {
        MY_LOGE("Failed to init 3rd part bokeh");
    }
    return bRet;
}

MBOOL
BokehNode::
onUninit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyBokeh->uninit();
    if(!bRet)
    {
        MY_LOGE("Failed to uninit 3rd part bokeh");
    }
    return bRet;
}

MBOOL
BokehNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
BokehNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
BokehNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    MY_LOGD("reqID=%d, dataID=%d(%s)", pRequest->getRequestNo(), dataID, BokehNodeHelper::getDataIDName(dataID));
    //
    MBOOL bRet = MTRUE;
    switch(dataID)
    {
        case FD_TO_BOKEH:
        case MFNR_TO_BOKEH:
        case DEPTH_TO_BOKEH:
        case HDR_TO_BOKEH:
            {
                mRequestQue.enque(pRequest);
            }
            break;
        default:
            MY_LOGW("Un-recognized dataID ID, id=%s reqID=%d", ID2Name(dataID), pRequest->getRequestNo());
            bRet = MFALSE;
            break;
    }
  return bRet;
}

MVOID
BokehNode::
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
BokehNode::
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
    AutoProfileLogging profile("BokehNode::onThreadLoop", pRequest->getRequestNo());

    MBOOL bRet = MTRUE;
    const MINT32 reqNum = pRequest->getRequestNo();

    auto predictor = [reqNum] (const IspPipeRequestPtr& requestPtr) {return requestPtr->getRequestNo() == reqNum; };
    if (std::find_if(mProcessingRequests.begin(), mProcessingRequests.end(), predictor) == mProcessingRequests.end())
    {
        auto it = mProcessingRequests.begin();
        for(; it != mProcessingRequests.end(); it++)
        {
            auto& item = *it;
            if( reqNum < item->getRequestNo())
            break;
        }
        mProcessingRequests.insert(it, pRequest);
        MY_LOGD("insert request to requestCollection(size:%zu), req#:%d", mProcessingRequests.size(), reqNum);

        // print the contain of processingRequests
        for(int i = 0; i < mProcessingRequests.size(); i++)
        {
            const MBOOL isCurrentReq = (mProcessingRequests[i]->getRequestNo() == reqNum);
            MY_LOGD("in ProcessingRequests, #%d, req#:%d %s",
                i, mProcessingRequests[i]->getRequestNo(), (isCurrentReq ? "(new)" : ""));
        }
    }
    else
    {
        MY_LOGD("existing request in requestCollection(size:%zu), req#:%d",
            mProcessingRequests.size(), reqNum);
    }

    MBOOL isExecuteBokeh = BokehNodeHelper::getIsOutputRequest(pRequest);
    if(isExecuteBokeh)
    {
        MBOOL isSuccess = MTRUE;
        if(pRequest->getRequestAttr().isPureBokeh)
        {
            // no need to check the received request count

            MY_LOGD("executePureBokeh, req#:%d", reqNum);
            if(executePureBokeh(pRequest))
            {
                MY_LOGD("success to executePureBokeh, req#:%d", reqNum);
                pRequest->setOutputBufferReady(BID_OUT_CLEAN_FSYUV);
                pRequest->setOutputBufferReady(BID_OUT_DEPTHMAP);
                pRequest->setOutputBufferReady(BID_OUT_BOKEH_FSYUV);
                pRequest->setOutputBufferReady(BID_META_OUT_APP);
                pRequest->setOutputBufferReady(BID_META_OUT_HAL);
            }
            else
            {
                MY_LOGE("falide to executePureBokeh, req#:%d", reqNum);
                isSuccess = MFALSE;
            }
        }
        else
        {
            mRequestReceivedCounter->decreaseReceivedCount(reqNum);

            MBOOL isCleanFSYUVReady = pRequest->getOutputBufferReady(BID_OUT_CLEAN_FSYUV);
            MBOOL isDepthReady = pRequest->getOutputBufferReady(BID_OUT_DEPTHMAP);
            MBOOL hasFDResult = (pRequest->getRequestAttr().fdResult != nullptr);
            MBOOL isReceivedAllRequests =  mRequestReceivedCounter->isReceivedAllRequests(reqNum);

            MY_LOGD("status for execute bokeh, cleanFSYUV:%d, depth:%d fdResult:%d, receivedRequests:%d/%d",
                    isCleanFSYUVReady, isDepthReady, hasFDResult,
                    mRequestReceivedCounter->getReceivedCount(reqNum), mRequestReceivedCounter->getMaxReceivedCount());

            if(!isCleanFSYUVReady || !isDepthReady || !hasFDResult || !isReceivedAllRequests)
            {
                isExecuteBokeh = MFALSE;
            }
            else
            {
                std::string executeName;
                decltype(&BokehNode::executeBokeh) executeBokehOperatorPtr = nullptr;
                if(pRequest->getRequestAttr().isDepthExecuteFailed)
                {
                    executeName = "executeEmptyBokeh";
                    executeBokehOperatorPtr = &BokehNode::executeEmptyBokeh;
                }
                else
                {
                   executeName = "executeBokeh";
                   executeBokehOperatorPtr = &BokehNode::executeBokeh;
                }

                MY_LOGD("%s, req#:%d", executeName.c_str(), reqNum);
                if((this->*executeBokehOperatorPtr)(pRequest))
                {
                    MY_LOGD("success to %s, req#:%d", executeName.c_str(), reqNum);
                    pRequest->setOutputBufferReady(BID_OUT_BOKEH_FSYUV);
                    pRequest->setOutputBufferReady(BID_META_OUT_APP);
                    pRequest->setOutputBufferReady(BID_META_OUT_HAL);

                    mRequestReceivedCounter->removeRequest(reqNum);
                }
                else
                {
                    MY_LOGE("failed to %s, req#:%d", executeName.c_str(), reqNum);
                    isSuccess = MFALSE;
                }
            }
        }

        if(isExecuteBokeh)
        {
            DataID dataId = BOKEH_OUT;
            auto handleDataOperatorPtr = &BokehNode::handleDataAndDump;

            if(!isSuccess)
            {
                MY_LOGE("failed to execute third-party bokeh, req#:%d", reqNum);
                dataId = ERROR_OCCUR_NOTIFY;
                handleDataOperatorPtr = &BokehNode::handleData;
            }

            // TODO: bed smell code, need to refactor
            const MINT32 outputReqNum = reqNum;
            for (auto it = mProcessingRequests.begin(); it != mProcessingRequests.end();)
            {
                auto& item = *it;
                if( item->getRequestNo() <= outputReqNum)
                {
                    MY_LOGD("dispatch to next node, req#:%d", item->getRequestNo());
                    (this->*handleDataOperatorPtr)(dataId, item);
                    item->getBufferHandler()->onProcessDone(getNodeId());

                    // remove item
                    mProcessingRequests.erase(it);
                    it = mProcessingRequests.begin();
                }
                else
                {
                    // move to next item
                    MY_LOGD("stay in processingRequests, req#:%d", item->getRequestNo());
                    ++it;
                }
            }
        }
        bRet = isSuccess;
    }
    return bRet;
}

MBOOL
BokehNode::
executeBokeh(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // input clean full size YUV
    IImageBuffer *pImgBuf_inCleanFSYUV = nullptr;
    // input depthmap
    IImageBuffer *pImgBuf_inDepthmap = nullptr;
    // output bokeh full size YUV
    IImageBuffer *pImgBuf_outBokehFSYUV = nullptr;
    // meta data - app
    IMetadata* appMeta = nullptr;
    // meta data - hal
    IMetadata* halMeta = nullptr;
    // fd result
    FDResultInfo* pFDResulrInfo = nullptr;

    MBOOL bRet = MTRUE;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_inCleanFSYUV);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_DEPTHMAP, pImgBuf_inDepthmap);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_BOKEH_FSYUV, pImgBuf_outBokehFSYUV);
    bRet &= ((appMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP)) != nullptr);
    bRet &= ((halMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1)) != nullptr);
    bRet &= ((pFDResulrInfo = pRequest->getRequestAttr().fdResult.get()) != nullptr);

    if(!bRet)
    {
        MY_LOGE("failed to get input buffers, req#: %d, cleanFSYUV:%p, depth:%p, bokehFSYUV:%p, metaInApp:%p, metaInHal:%p, fdInfo=%p",
            pRequest->getRequestNo(), pImgBuf_inCleanFSYUV, pImgBuf_inDepthmap, pImgBuf_outBokehFSYUV, appMeta, halMeta, pFDResulrInfo);
    }
    else
    {
        IThirdPartyBokeh* pThirdPartyBokeh = nullptr;
        if(pRequest->getRequestAttr().isDisableBokeh)
        {
            MY_LOGD("disableBokeh, using defaultThirdPartyBokehInstance");
            pThirdPartyBokeh = IThirdPartyBokeh::getDefaultInstance();
        }
        else
        {
            MY_LOGD("enableBokeh, using thirdPartyBokehInstance");
            pThirdPartyBokeh = mThirdPartyBokeh.get();
        }

        AutoProfileLogging profile("BokehNode::execute_third_party_bokeh", pRequest->getRequestNo());
        sp<IStopWatchCollection> stopWatchCollection = pRequest->getStopWatchCollection();
        auto stopWatch = stopWatchCollection->getStopWatch(StopWatchType::Auto, "execute_third_party_bokeh");

        MY_LOGD("execute third-party bokeh, req#: %d", pRequest->getRequestNo());
        bRet = pThirdPartyBokeh->execute(*pImgBuf_inCleanFSYUV, *pImgBuf_inDepthmap, *appMeta, *halMeta, *pFDResulrInfo, *pImgBuf_outBokehFSYUV);
    }
    return bRet;
}

MBOOL
BokehNode::
executePureBokeh(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input main1 full size YUV
    IImageBuffer *pImgBuf_inFSYUVMain1 = nullptr;
    // input main2 full size YUV
    IImageBuffer *pImgBuf_inFSYUVMain2 = nullptr;
    // output clean full size YUV
    IImageBuffer *pImgBuf_outCleanFSYUV = nullptr;
    // output depthmap
    IImageBuffer *pImgBuf_outDepthmap = nullptr;
    // output bokeh full size YUV
    IImageBuffer *pImgBuf_outBokehFSYUV = nullptr;
    // meta data - app
    IMetadata* appMeta = nullptr;
    // meta data - hal
    IMetadata* halMeta = nullptr;
    // fd result
    FDResultInfo* pFDResulrInfo = nullptr;

    MBOOL bRet = MTRUE;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inFSYUVMain1);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN2, pImgBuf_inFSYUVMain2);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_DEPTHMAP, pImgBuf_outDepthmap);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_outCleanFSYUV);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_BOKEH_FSYUV, pImgBuf_outBokehFSYUV);
    bRet &= ((appMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP)) != nullptr);
    bRet &= ((halMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1)) != nullptr);
    bRet &= ((pFDResulrInfo = pRequest->getRequestAttr().fdResult.get()) != nullptr);

    if(!bRet)
    {
        MY_LOGE("failed to get input buffers, req#: %d, main1FSYUV:%p, main2FSYUV:%p, depth:%p, cleanFSYUV:%p, bokehFSYUV:%p, metaInApp:%p, metaInHal:%p, fdInfo:%p",
            pRequest->getRequestNo(), pImgBuf_inFSYUVMain1, pImgBuf_inFSYUVMain2, pImgBuf_outDepthmap, pImgBuf_outCleanFSYUV, pImgBuf_outBokehFSYUV, appMeta, halMeta, pFDResulrInfo);
    }
    else
    {
        AutoProfileLogging profile("BokehNode::execute_third_party_pure_bokeh", pRequest->getRequestNo());
        sp<IStopWatchCollection> stopWatchCollection = pRequest->getStopWatchCollection();
        auto stopWatch = stopWatchCollection->getStopWatch(StopWatchType::Auto, "execute_third_party_pure_bokeh");

        MY_LOGD("execute third-party pure bokeh, req#: %d", pRequest->getRequestNo());
        bRet = mThirdPartyBokeh->execute( *pImgBuf_inFSYUVMain1,
                                          *pImgBuf_inFSYUVMain2,
                                          *appMeta,
                                          *halMeta,
                                          *pFDResulrInfo,
                                          *pImgBuf_outDepthmap,
                                          *pImgBuf_outCleanFSYUV,
                                          *pImgBuf_outBokehFSYUV);
    }
    return bRet;
}

MBOOL
BokehNode::
executeEmptyBokeh(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // input clean full size YUV
    IImageBuffer *pImgBuf_inCleanFSYUV = nullptr;
    // output bokeh full size YUV
    IImageBuffer *pImgBuf_outBokehFSYUV = nullptr;

    MBOOL bRet = MTRUE;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_inCleanFSYUV);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_BOKEH_FSYUV, pImgBuf_outBokehFSYUV);


    if(!bRet)
    {
        MY_LOGE("failed to get input buffers, req#: %d, cleanFSYUV:%p, bokehFSYUV:%p",
            pRequest->getRequestNo(), pImgBuf_inCleanFSYUV, pImgBuf_outBokehFSYUV);
    }
    else
    {
        AutoProfileLogging profile("BokehNode::execute_empty_bokeh", pRequest->getRequestNo());
        sp<IStopWatchCollection> stopWatchCollection = pRequest->getStopWatchCollection();
        auto stopWatch = stopWatchCollection->getStopWatch(StopWatchType::Auto, "execute_empty_bokeh");

        MY_LOGD("execute third-party empty bokeh, req#: %d", pRequest->getRequestNo());

        MRect crop = MRect(pImgBuf_inCleanFSYUV->getImgSize().w, pImgBuf_inCleanFSYUV->getImgSize().h);
        bRet = DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_outBokehFSYUV, pImgBuf_inCleanFSYUV, crop);
    }
    return bRet;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyBokehUtility Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
ThirdPartyBokehUtility::
tryGetDofLevel(const IMetadata& appMeta, MINT32& dofLevel)
{
    MBOOL bRet = MFALSE;

    IMetadata::IEntry entry = appMeta.entryFor(MTK_STEREO_FEATURE_DOF_LEVEL);
    if(!entry.isEmpty())
    {
        dofLevel = entry.itemAt(0, Type2Type<MINT32>());
        bRet = MTRUE;
    }
    else
    {
        MY_LOGD("Failed to get app meta: MTK_STEREO_FEATURE_DOF_LEVEL");
    }
    return bRet;
}

MBOOL
ThirdPartyBokehUtility::
tryGetAFStatus(const IMetadata& appMeta, MUINT8& afStatus)
{
    MBOOL bRet = MFALSE;

    IMetadata::IEntry entry = appMeta.entryFor(MTK_CONTROL_AF_STATE);
    if(!entry.isEmpty())
    {
        afStatus = entry.itemAt(0, Type2Type<MUINT8>());
        bRet = MTRUE;
    }
    else
    {
        MY_LOGD("Failed to get app meta: MTK_CONTROL_AF_STATE");
    }
    return bRet;
}

MBOOL
ThirdPartyBokehUtility::
tryGetAFRoi(const IMetadata& appMeta, MRect& afRoi)
{
    MBOOL bRet = MFALSE;

    IMetadata::IEntry entry = appMeta.entryFor(MTK_3A_FEATURE_AF_ROI);
    if(!entry.isEmpty())
    {
        const MINT32 roiType = entry.itemAt(0, Type2Type<MINT32>());
        const MINT32 roiCount = entry.itemAt(1, Type2Type<MINT32>());
        afRoi.p.x = entry.itemAt(2, Type2Type<MINT32>());
        afRoi.p.y = entry.itemAt(3, Type2Type<MINT32>());
        afRoi.s.w = entry.itemAt(4, Type2Type<MINT32>()) - afRoi.p.x;
        afRoi.s.h = entry.itemAt(5, Type2Type<MINT32>()) - afRoi.p.y;

        MY_LOGD("af roi info, type:%d, count:%d", roiType, roiCount);

        bRet = MTRUE;
    }
    else
    {
        MY_LOGD("Failed to get app meta: MTK_3A_FEATURE_AF_ROI");
    }
    return bRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyBokeh Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IThirdPartyBokeh*
IThirdPartyBokeh::
createInstance()
{
    MBOOL isCreateDefault = (::property_get_int32("vendor.dg.isppipe.bokeh.default", 1) == 1 ) ? MTRUE : MFALSE;
    if(isCreateDefault)
    {
        MY_LOGD("create DefaultThirdPartyBokeh");
        return new DefaultThirdPartyBokeh();
    }
    return new ThirdPartyBokehImp();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DefaultThirdPartyBokeh Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IThirdPartyBokeh*
IThirdPartyBokeh::
getDefaultInstance()
{
    SCOPED_TRACER();
    static std::unique_ptr<IThirdPartyBokeh> defaultInstant(new DefaultThirdPartyBokeh());
    return defaultInstant.get();
}

DefaultThirdPartyBokeh::
DefaultThirdPartyBokeh()
: ThirdPartyBokehBase()
{

}

MBOOL
DefaultThirdPartyBokeh::
onExecute(const IImageBuffer& fsYUVCleanImgbuf, const IImageBuffer& depthMapImgBuf, const IMetadata& appMeta, const IMetadata& halMeta, const FDResultInfo& fdResult, IImageBuffer& fsYUVBokehImgbuf)
{
    SCOPED_TRACER();

    // to get the related parameters of bokeh, we can use the utility class
    // or use the argument appMeta directly, for example: use the utility class:
    MINT32 dofLevel = 0;
    if(ThirdPartyBokehUtility::tryGetDofLevel(appMeta, dofLevel))
    {
        MY_LOGD("dofLevel(%d)", dofLevel);
    }

    MUINT8 afStatus = 0;
    if(ThirdPartyBokehUtility::tryGetAFStatus(appMeta, afStatus))
    {
        MY_LOGD("af status(%d)", afStatus);
    }

    MRect afRoi;
    if(ThirdPartyBokehUtility::tryGetAFRoi(appMeta, afRoi))
    {
        MY_LOGD("afRoi(%d, %d, %d, %d)", afRoi.p.x, afRoi.p.y, afRoi.s.w, afRoi.s.h);
    }

    MRect crop = MRect(fsYUVCleanImgbuf.getImgSize().w, fsYUVCleanImgbuf.getImgSize().h);
    MBOOL executeResult = DCMFIspPipeFlowUtility::imgMemoryCopy(&fsYUVBokehImgbuf, &fsYUVCleanImgbuf, crop);

    // when foreground postview is disabled, postview is produced by background
    MINT32 needBGPostview = 0;
    IMetadata::getEntry<MINT32>(&halMeta, MTK_STEREO_NEED_BG_POSTVIEW, needBGPostview);
    if(needBGPostview == 1)
    {
        AUTO_TIMER("background postview callback");
        MY_LOGD("background postview callback +");
        // postview size
        MSize postviewSize(0,0);
        if(!IMetadata::getEntry<MSize>(&halMeta, MTK_STEREO_DCMF_POSTVIEW_SIZE, postviewSize)){
            MY_LOGE("can not get postview size!");
        }

        // timestamp
        MINT64 timestamp = 0;
        if(!IMetadata::getEntry<MINT64>(&halMeta, MTK_DUALCAM_TIMESTAMP, timestamp)){
            MY_LOGE("can not get timestamp!");
        }

        // produce postview
        sp<PipeImgBufferPool> pBGPostviewBufferPool = nullptr;
        CREATE_IMGBUF_POOL(
            pBGPostviewBufferPool,
            "pBGPostviewBufferPool",
            postviewSize,
            eImgFmt_NV21,
            PipeImgBufferPool::USAGE_HW
        );
        ALLOCATE_BUFFER_POOL(pBGPostviewBufferPool, 1);

        sp<IImageBuffer> pImgBuf_postview = pBGPostviewBufferPool->requestIIBuffer()->getImageBuffer();

        MRect crop = MRect(fsYUVBokehImgbuf.getImgSize().w, fsYUVBokehImgbuf.getImgSize().h);
        DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_postview.get(), &fsYUVBokehImgbuf, crop);

        MY_LOGD("timestamp(%lld) localPool postviewSize(%dx%d)",
            timestamp, pImgBuf_postview->getImgSize().w, pImgBuf_postview->getImgSize().h
        );

        // AOSP or MMSDK
        MINT32 bgSupport = 0;
        IMetadata::getEntry<MINT32>(&halMeta, MTK_STEREO_BG_SERVICE_SUPPORT, bgSupport);

        if(bgSupport == 1){
            // MMSDK
            MY_LOGD("postview via mmsdk");
            ICallbackClientMgr::getInstance()->onCB_PostviewClient(
                timestamp,
                pImgBuf_postview.get()
            );
        }else{
            // AOSP
            MINT64 pCallbackClient = 0;
            MY_LOGD("postview via aosp");
            if(IMetadata::getEntry<MINT64>(&halMeta, MTK_STEREO_CALLBACK_CLIENT, pCallbackClient)){
                IShotCallback* pCallback = reinterpret_cast<IShotCallback*>(pCallbackClient);

                wp<IShotCallback> wpCallback(pCallback);
                sp<IShotCallback> spCallback = wpCallback.promote();

                if(spCallback != nullptr){
                    spCallback->onCB_PostviewClient(
                        timestamp,
                        pImgBuf_postview.get(),
                        0,    // never ask for offset
                        false // true:mmsdk callback ; fasle:AOSP callback
                    );
                }else{
                    MY_LOGW("can not promote IShotCallback, may already destroyed");
                }
            }
        }

        PipeImgBufferPool::destroy(pBGPostviewBufferPool);
        MY_LOGD("background postview callback -");
    }

    return executeResult;
}

MBOOL
DefaultThirdPartyBokeh::
onExecute(const IImageBuffer& fsYUVMain1ImageBuf, const IImageBuffer& fsYUVMain2ImageBuf, const IMetadata& appMeta, const IMetadata& halMeta, const FDResultInfo& fdResult, IImageBuffer& depthMapImgBuf, IImageBuffer& fsYUVCleanImgbuf, IImageBuffer& fsYUVBokehImgbuf)
{
    // print info
    {
        BokehNodeHelper::printBokehInfo(appMeta, fdResult);
    }
    // fill with depthImgBuf
    {
        MVOID* pDepthVa = reinterpret_cast<MVOID*>(depthMapImgBuf.getBufVA(0));
        MUINT32 uDepthSize = depthMapImgBuf.getBufSizeInBytes(0);
        MY_LOGD("process depth va(%p) size(%u)", pDepthVa, uDepthSize);
        memset(pDepthVa, 255, uDepthSize);
    }
    // fill with cleanImgBuf
    {
        AUTO_TIMER("copy fsYUVMain1ImgBuf to fsYUVCleanImgbuf");

        MRect crop = MRect(fsYUVMain1ImageBuf.getImgSize().w, fsYUVMain1ImageBuf.getImgSize().h);
        if(!DCMFIspPipeFlowUtility::imgMemoryCopy(&fsYUVCleanImgbuf, &fsYUVMain1ImageBuf, crop))
        {
            MY_LOGE("failed to copy fsYUVMain1ImgBuf to fsYUVCleanImgbuf");
            return MFALSE;
        }
    }
    // fill with bokehImgBuf
    {
        AUTO_TIMER("copy fsYUVMain1ImageBuf to fsYUVBokehImgbuf");

        MRect crop = MRect(fsYUVMain1ImageBuf.getImgSize().w, fsYUVMain1ImageBuf.getImgSize().h);
        if(!DCMFIspPipeFlowUtility::imgMemoryCopy(&fsYUVBokehImgbuf, &fsYUVMain1ImageBuf, crop))
        {
            MY_LOGE("failed to copy fsYUVMain1ImageBuf to fsYUVBokehImgbuf");
            return MFALSE;
        }
    }
    return MTRUE;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyBokehImp Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThirdPartyBokehImp::
ThirdPartyBokehImp()
: ThirdPartyBokehBase()
{

}

MBOOL
ThirdPartyBokehImp::
onInit()
{
    // TODO:
    // init third party bokeh here
    return MTRUE;
}

MBOOL
ThirdPartyBokehImp::
onUninit()
{
    // TODO:
    // uninit third party bokeh here
    return MTRUE;
}

MBOOL
ThirdPartyBokehImp::
onExecute(const IImageBuffer& fsYUVCleanImgbuf, const IImageBuffer& depthMapImgBuf,const IMetadata& appMeta, const IMetadata& halMeta, const FDResultInfo& fdResult, IImageBuffer& fsYUVBokehImgbuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party bokeh here
    // note: we need to fill with the result buffer, fsYUVBokehImgbuf
    return MTRUE;
}

MBOOL
ThirdPartyBokehImp::
onExecute(
        const IImageBuffer& fsYUVMain1ImageBuf,
        const IImageBuffer& fsYUVMain2ImageBuf,
        const IMetadata& appMeta,
        const IMetadata& halMeta,
        const FDResultInfo& fdResult,
        IImageBuffer& depthMapImgBuf,
        IImageBuffer& fsYUVCleanImgbuf,
        IImageBuffer& fsYUVBokehImgbuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party bokeh here
    // note: we need to fill with the result buffer, depthMapImgBuf and fsYUVBokehImgbuf
    return MTRUE;
};

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

