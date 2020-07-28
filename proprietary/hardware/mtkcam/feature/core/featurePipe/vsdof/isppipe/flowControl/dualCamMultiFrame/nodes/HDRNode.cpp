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
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file

// Local header file
#include "../DCMFIspPipeFlowUtility.h"
#include "HDRNode.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "HDRNode"
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
} // anonymous namespace
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IThirdPartyHDR
 * @brief interface for third party bokeh
 */
class IThirdPartyHDR : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyHDR Public Static Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IThirdPartyHDR* createInstance();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyHDR Public Operator
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
     * @brief do third party HDR
     * @param [in] fsYUV00Imgbuf the image buffer of the full size with EV-1
     * @param [in] fsYUV01Imgbuf the image buffer of the full size with EV0
     * @param [in] appMeta app metadata
     * @param [out] fsYUVHDRImgbuf the image buffer of the full size bokeh YUV
     * @return
     * - execute result
     */
    virtual MBOOL execute( const IImageBuffer& fsYUV00Imgbuf,
                           const IImageBuffer& fsYUV01Imgbuf,
                           const IMetadata& appMeta,
                           IImageBuffer& fsYUVHDRImgbuf) = 0;
    /**
     * @brief do third party HDR
     * @param [in] fsYUV00Imgbuf the image buffer of the full size with EV-1
     * @param [in] fsYUV01Imgbuf the image buffer of the full size with EV0
     * @param [in] appMeta app metadata
     * @param [in] fdResult face detection result info
     * @param [out] fsYUVHDRImgbuf the image buffer of the full size bokeh YUV
     * @return
     * - execute result
     */
    virtual MBOOL execute( const IImageBuffer& fsYUV00Imgbuf,
                           const IImageBuffer& fsYUV01Imgbuf,
                           const IMetadata& appMeta,
                           const FDResultInfo& fdResult,
                           IImageBuffer& fsYUVHDRImgbuf) = 0;

    virtual ~IThirdPartyHDR(){};
};

namespace { // begin anonymous namespace
/**
 * @class HDRNodeHelper
 * @brief Helper class for HDRNode
 */
class HDRNodeHelper
{
public:
    HDRNodeHelper() = delete;

public:

    static constexpr MINT32 EVRequestItem_EV    = 0;
    static constexpr MINT32 EVRequestItem_REQ   = 1;

    static constexpr MINT32 DesiredEV00 = -1;
    static constexpr MINT32 DesiredEV01 =  0;

public:
    /**
     * @brief try to get the output request
     * @param [in] table that include ev and corresponding request
     * @param [out] outputRequest the found output request
     * @return
     * - execute result
     */
    static MBOOL tryGetOutputRequest(const HDRNode::EVRequestTable& table, IspPipeRequestPtr& outputRequest);
    /**
     * @brief get ev value
     * @param [in] request the target request
     * @param [in] nodeID current node ID
     * @param [out] evValue ev value
     * @return
     * - execute result
     */
    static MBOOL tryGetExposureValue(const IspPipeRequestPtr& request, MINT32 nodeID, HDRNode::ExposureValue& evValue);
    /**
     * @brief determine the desired ev request is in the table or not
     * @param [in] table that include ev and corresponding request
     * @param [in] ev the desired exposure value
     * @return
     * - execute result
     */
    static MBOOL getIsDesiredEVRequestExisting(const HDRNode::EVRequestTable& table, HDRNode::ExposureValue ev);
    /**
     * @brief get desired request by exposure value
     * @param [in] table that include ev and corresponding request
     * @param [in] ev the desired exposure value
     * @param [out] request the found request that is with the desired exposure value
     * @return
     * - execute result
     */
    static MBOOL tryGetRequestByEV(const HDRNode::EVRequestTable& table, HDRNode::ExposureValue ev, IspPipeRequestPtr& request);
    /**
     * @brief determine the request is output or not
     * @param [in] request the target request
     * @return
     * - execute result
     */
    static MBOOL getIsOutputRequest(const IspPipeRequestPtr& request);
    /**
     * @brief print HDR related info
     * @param [in] appMeta app metadata
     * @param [in] fdResult face detection result info
     */
    static MVOID printHDRInfo(const IMetadata& appMeta, const FDResultInfo& fdResult);
    /**
     * @brief get the data ID name
     * @param [in] dataID the data ID
     * @return
     * - the name of argument dataID
     */
    static const char* getDataIDName(IspPipeDataHandler::DataID dataID);
};

/**
 * @class ThirdPartyHDRBase
 * @brief base(abstract) class for third party HDR
 */
class ThirdPartyHDRBase : public IThirdPartyHDR
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IThirdPartyHDR Public Operator
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

    MBOOL execute( const IImageBuffer& fsYUV00Imgbuf,
                   const IImageBuffer& fsYUV01Imgbuf,
                   const IMetadata& appMeta,
                   IImageBuffer& fsYUVHDRImgbuf) override
    {
        return onExecute(fsYUV00Imgbuf, fsYUV01Imgbuf, appMeta, fsYUVHDRImgbuf);
    }

    MBOOL execute( const IImageBuffer& fsYUV00Imgbuf,
                   const IImageBuffer& fsYUV01Imgbuf,
                   const IMetadata& appMeta,
                   const FDResultInfo& fdResult,
                   IImageBuffer& fsYUVHDRImgbuf) override
    {
        return onExecute(fsYUV00Imgbuf, fsYUV01Imgbuf, appMeta, fdResult, fsYUVHDRImgbuf);
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRBase protected Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ThirdPartyHDRBase() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    virtual MBOOL onInit() { return MTRUE; };

    virtual MBOOL onUninit() { return MTRUE; };

    virtual MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                             const IImageBuffer& fsYUV01Imgbuf,
                             const IMetadata& appMeta,
                             IImageBuffer& fsYUVHDRImgbuf) { return MTRUE; };

    virtual MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                             const IImageBuffer& fsYUV01Imgbuf,
                             const IMetadata& appMeta,
                             const FDResultInfo& fdResult,
                             IImageBuffer& fsYUVHDRImgbuf) { return MTRUE;  }
};

/**
 * @class ThirdPartyHDRUtility
 * @brief utility class for third party HDR
 */
class ThirdPartyHDRUtility final
{
public:
    ThirdPartyHDRUtility() = delete;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRUtility Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief get iso value
     * @param [in] appMeta app metadata
     * @param [out] isoValue iso value
     * @return
     * - execute result
     */
    static MBOOL tryGetIsoValue(const IMetadata& appMeta, MINT32& isoValue);
};
/**
 * @class DefaultThirdPartyHDR
 * @brief default implementation for third party HDR
 */
class DefaultThirdPartyHDR final: public ThirdPartyHDRBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DefaultThirdPartyHDR Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DefaultThirdPartyHDR();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                     const IImageBuffer& fsYUV01Imgbuf,
                     const IMetadata& appMeta,
                     IImageBuffer& fsYUVHDRImgbuf) override;

    MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                     const IImageBuffer& fsYUV01Imgbuf,
                     const IMetadata& appMeta,
                     const FDResultInfo& fdResult,
                     IImageBuffer& fsYUVHDRImgbuf) override;
};

/**
 * @class ThirdPartyHDRImp
 * @brief implementation for third party HDR
 */
class ThirdPartyHDRImp final: public ThirdPartyHDRBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRImp Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyHDRImp();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyHDRBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL onInit() override;

    MBOOL onUninit() override;

    MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                     const IImageBuffer& fsYUV01Imgbuf,
                     const IMetadata& appMeta,
                     IImageBuffer& fsYUVHDRImgbuf) override;

    MBOOL onExecute( const IImageBuffer& fsYUV00Imgbuf,
                     const IImageBuffer& fsYUV01Imgbuf,
                     const IMetadata& appMeta,
                     const FDResultInfo& fdResult,
                     IImageBuffer& fsYUVHDRImgbuf) override;
};
/*******************************************************************************
* Const Definition
********************************************************************************/


} // end anonymous namespace

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HDRNodeHelper Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
HDRNodeHelper::
tryGetOutputRequest(const HDRNode::EVRequestTable& table, IspPipeRequestPtr& outputRequest)
{
    IspPipeRequestPtr found = nullptr;
    for(auto& item : table)
    {
        if(HDRNodeHelper::getIsOutputRequest(std::get<HDRNodeHelper::EVRequestItem_REQ>(item)))
        {
            if(found == nullptr)
            {
                found = std::get<HDRNodeHelper::EVRequestItem_REQ>(item);
                MY_LOGD("found the output request: req#=%d", found->getRequestNo());
            }
            else
            {
                IspPipeRequestPtr foundAgain = std::get<HDRNodeHelper::EVRequestItem_REQ>(item);
                MY_LOGE("found the more than one output request: currentFoundReq#=%d, perivousFoundReq#=%d",
                    foundAgain->getRequestNo(), found->getRequestNo());

                return MFALSE;
            }
        }
    }
    return (outputRequest = found) != nullptr;
}

MBOOL
HDRNodeHelper::
tryGetExposureValue(const IspPipeRequestPtr& request, MINT32 nodeID, HDRNode::ExposureValue& evValue)
{
    MBOOL bRet = MFALSE;
    IMetadata* halMain1Meta = request->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    if(halMain1Meta == nullptr)
    {
        MY_LOGE("Failed to get appMetadata");
    }
    else
    {
        if(IMetadata::getEntry<MINT32>(halMain1Meta, MTK_STEREO_HDR_EV, evValue))
        {
            bRet = MTRUE;
        }
        else
        {
            MY_LOGW("Failed to get app meta: MTK_STEREO_HDR_EV");
        }
    }
    return bRet;
}

MBOOL
HDRNodeHelper::
getIsDesiredEVRequestExisting(const HDRNode::EVRequestTable& table, HDRNode::ExposureValue ev)
{
    auto predictor = [ev] (const HDRNode::EVRequestItem& item) { return std::get<HDRNodeHelper::EVRequestItem_EV>(item) == ev; };
    return (std::find_if(table.begin(), table.end(), predictor) != table.end());
}

MBOOL
HDRNodeHelper::
tryGetRequestByEV(const HDRNode::EVRequestTable& table, HDRNode::ExposureValue ev, IspPipeRequestPtr& request)
{
    MBOOL bRet = MFALSE;

    auto predictor = [ev] (const HDRNode::EVRequestItem& item) { return std::get<HDRNodeHelper::EVRequestItem_EV>(item) == ev; };
    auto item = std::find_if(table.begin(), table.end(), predictor);
    if(item != table.end())
    {
        request = std::get<HDRNodeHelper::EVRequestItem_REQ>(*item);
        bRet = MTRUE;

        MY_LOGD("found request, ev:%d, req#:%d, reqAddr:%p", ev, request->getRequestNo(), request.get());
    }
    return bRet;
}

MBOOL
HDRNodeHelper::
getIsOutputRequest(const IspPipeRequestPtr& request)
{
    return request->isRequestBuffer(BID_OUT_CLEAN_FSYUV);
}

MVOID
HDRNodeHelper::
printHDRInfo(const IMetadata& appMeta, const FDResultInfo& fdResult)
{
    SCOPED_TRACER();

    MINT32 isoValue = -1;
    if(!ThirdPartyHDRUtility::tryGetIsoValue(appMeta, isoValue))
    {
        MY_LOGW("failed to get iso value");
    }

    const std::string tag = "[HDR_info] ";
    std::ostringstream stringStream;
    stringStream << tag << "************************************" << std::endl;
    stringStream << "iso: " << isoValue << std::endl;
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
HDRNodeHelper::
getDataIDName(IspPipeDataHandler::DataID dataID)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(dataID)
      {
        MAKE_NAME_CASE(ROOT_TO_HDR);
        MAKE_NAME_CASE(FD_TO_HDR);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IThirdPartyHDR Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IThirdPartyHDR*
IThirdPartyHDR::
createInstance()
{
    MBOOL isCreateDefault = (::property_get_int32("vendor.dg.isppipe.hdr.default", 1) == 1 ) ? MTRUE : MFALSE;
    if(isCreateDefault)
    {
        MY_LOGD("create DefaultThirdPartyHDR");
        return new DefaultThirdPartyHDR();
    }
    return new ThirdPartyHDRImp();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  HDRNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
HDRNode::
HDRNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: HDRNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

HDRNode::
HDRNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
, mThirdPartyHDR(IThirdPartyHDR::createInstance())
{
    SCOPED_TRACER();

    MY_LOGD("ctor:%p", this);

    this->addWaitQueue(&mRequestQue);
}

HDRNode::
~HDRNode()
{
    SCOPED_TRACER();

    MY_LOGD("dctor:%p", this);
}

MBOOL
HDRNode::
onInit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyHDR->init();
    if(!bRet)
    {
        MY_LOGE("Failed to init 3rd part HDR");
    }
    return bRet;
}

MBOOL
HDRNode::
onUninit()
{
    SCOPED_TRACER();

    MBOOL bRet = mThirdPartyHDR->uninit();
    if(!bRet)
    {
        MY_LOGE("Failed to uninit 3rd part HDR");
    }
    return bRet;
}

MBOOL
HDRNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
HDRNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
HDRNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    MY_LOGD("reqID=%d, dataID=%d(%s)", pRequest->getRequestNo(), dataID, HDRNodeHelper::getDataIDName(dataID));

    MBOOL bRet = MTRUE;
    switch(dataID)
    {
        case ROOT_TO_HDR:
        case FD_TO_HDR:
            mRequestQue.enque(pRequest);
            break;
        default:
            MY_LOGW("Un-recognized dataID ID, id=%s reqID=%d", ID2Name(dataID), pRequest->getRequestNo());
            bRet = MFALSE;
            break;
    }
  return bRet;
}

MVOID
HDRNode::
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
HDRNode::
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

    MBOOL bRet = MFALSE;
    // mark on-going-request start
    this->incExtThreadDependency();
    {
        AutoProfileLogging profile("HDRNode::threadLoop", pRequest->getRequestNo());

        // 1. error check
        MBOOL isError = MFALSE;
        MINT32 exposureValue = 0;
        if(!HDRNodeHelper::tryGetExposureValue(pRequest, getNodeId(), exposureValue))
        {
            MY_LOGE("Failed to get exposure value, req#:%d", pRequest->getRequestNo());
            isError = MTRUE;
        }

        IspPipeRequestPtr existingRequestPtr = nullptr;
        if(HDRNodeHelper::tryGetRequestByEV(mEVRequestTable, exposureValue, existingRequestPtr))
        {
            MY_LOGE("the wrong flow, find the same ev-request, ev:%d, currentReq#:%d, existingReq#:%d",
                exposureValue, pRequest->getRequestNo(), existingRequestPtr->getRequestNo());
            isError = MTRUE;
        }

        if(!isError)
        {
            // 2. start to execute HDR
            mEVRequestTable.push_back(std::make_tuple(exposureValue, pRequest));
            MY_LOGD("insert ev-request into table: tableSize:%zu, ev:%d, req#:%d",
                mEVRequestTable.size(), exposureValue, pRequest->getRequestNo());

            const MBOOL isEV00Existing = HDRNodeHelper::getIsDesiredEVRequestExisting(mEVRequestTable, HDRNodeHelper::DesiredEV00);
            const MBOOL isEV01Existing = HDRNodeHelper::getIsDesiredEVRequestExisting(mEVRequestTable, HDRNodeHelper::DesiredEV01);
            if(!isEV00Existing || !isEV01Existing)
            {
                MY_LOGD("wait to excute third-party HDR: isEV00Existing:%d, isEV01Existing:%d",
                    isEV00Existing, isEV01Existing);

                bRet = MTRUE;
            }
            else
            {
                // assume when we get all of the desired EV requests and the output request is always ready

                MBOOL executeError = MTRUE;
                if(executeHDR())
                {
                    MY_LOGD("success to executeHDR");

                    for(auto& item : mEVRequestTable)
                    {
                        ExposureValue exposureValue = std::get<HDRNodeHelper::EVRequestItem_EV>(item);
                        IspPipeRequestPtr& tempRequest = std::get<HDRNodeHelper::EVRequestItem_REQ>(item);
                        if(HDRNodeHelper::getIsOutputRequest(tempRequest))
                        {
                            tempRequest->setOutputBufferReady(BID_OUT_CLEAN_FSYUV);
                            MY_LOGD("set the out buffer clearFSYUV is ready, req#:%d", tempRequest->getRequestNo());
                        }
                        this->handleDataAndDump(HDR_TO_BOKEH, tempRequest);
                        tempRequest->getBufferHandler()->onProcessDone(getNodeId());

                        MY_LOGD("dispatch request to next(HDR_TO_BOKEH), EV:%d, req#:%d", exposureValue, tempRequest->getRequestNo());
                    }
                    mEVRequestTable.clear();

                    bRet = MTRUE;
                }
                else
                {
                    MY_LOGE("Failed to executeHDR");

                    for(auto& item : mEVRequestTable)
                    {
                        ExposureValue exposureValue = std::get<HDRNodeHelper::EVRequestItem_EV>(item);
                        IspPipeRequestPtr& tempRequest = std::get<HDRNodeHelper::EVRequestItem_REQ>(item);
                        this->handleData(ERROR_OCCUR_NOTIFY, tempRequest);
                        tempRequest->getBufferHandler()->onProcessDone(getNodeId());

                        MY_LOGD("dispatch request to next(ERROR_OCCUR_NOTIFY), EV:%d, req#:%d", exposureValue, tempRequest->getRequestNo());
                    }
                    mEVRequestTable.clear();
                }
            }
        }
    }
    this->decExtThreadDependency();
    return bRet;
}

MBOOL
HDRNode::
executeHDR()
{
    SCOPED_TRACER();

    MBOOL bRet = MFALSE;
    IspPipeRequestPtr ev00RequestPtr = nullptr;
    IspPipeRequestPtr ev01RequestPtr = nullptr;
    IspPipeRequestPtr outputRequestPtr = nullptr;
    if( HDRNodeHelper::tryGetRequestByEV(mEVRequestTable, HDRNodeHelper::DesiredEV00, ev00RequestPtr)
        && HDRNodeHelper::tryGetRequestByEV(mEVRequestTable, HDRNodeHelper::DesiredEV01, ev01RequestPtr)
        && HDRNodeHelper::tryGetOutputRequest(mEVRequestTable, outputRequestPtr))
    {
        sp<PipeBufferHandler> ev00BufferHandlerPtr = ev00RequestPtr->getBufferHandler();
        sp<PipeBufferHandler> ev01BufferHandlerPtr = ev01RequestPtr->getBufferHandler();
        sp<PipeBufferHandler> outputBufferHandlerPtr = outputRequestPtr->getBufferHandler();
        // input main1 full size ev00
        IImageBuffer *pImgBuf_inEV00FSYUVMain1 = nullptr;
        // input main1 full size ev01
        IImageBuffer *pImgBuf_inEV01FSYUVMain1 = nullptr;
        // output bokeh full size YUV
        IImageBuffer *pImgBuf_outCleanFSYUV = nullptr;
        // meta data
        IMetadata* appMeta = nullptr;

        MBOOL isReady = MTRUE;
        isReady &= ev00BufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inEV00FSYUVMain1);
        isReady &= ev01BufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inEV01FSYUVMain1);
        isReady &= outputBufferHandlerPtr->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_outCleanFSYUV);
        isReady &= ((appMeta = outputBufferHandlerPtr->requestMetadata(getNodeId(), BID_META_IN_APP)) != nullptr);
        if(!isReady)
        {
            MY_LOGE("failed to get buffers and metadata, ev00FSYUVMain1:%p, ev01FSYUVMain1:%p, cleanFSYUV:%p, metaInApp:%p",
                pImgBuf_inEV00FSYUVMain1, pImgBuf_inEV01FSYUVMain1, pImgBuf_outCleanFSYUV, appMeta);
        }
        else
        {
            AutoProfileLogging profile("HDRNode::doThirdPartyHDR", outputRequestPtr->getRequestNo());

            const MBOOL isNeedFDInfo = outputRequestPtr->getRequestAttr().isFDToHDRFlow;
            MY_LOGD("execute third-party bokeh, ev00Req#:%d, ev01Req#:%d, outReq#:%d, isNeedFDInfo:%d",
                    ev00RequestPtr->getRequestNo(), ev01RequestPtr->getRequestNo(), outputRequestPtr->getRequestNo(), isNeedFDInfo);

            if(isNeedFDInfo)
            {
                FDResultInfo* pFDResultInfo = outputRequestPtr->getRequestAttr().fdResult.get();
                if(pFDResultInfo == nullptr)
                {
                    MY_LOGE("failed to get the fdInfo, req#:%d", outputRequestPtr->getRequestNo());
                    bRet = MFALSE;
                }
                else
                {
                    bRet = mThirdPartyHDR->execute(*pImgBuf_inEV00FSYUVMain1, *pImgBuf_inEV01FSYUVMain1, *appMeta, *pFDResultInfo, *pImgBuf_outCleanFSYUV);
                }
            }
            else
            {
                bRet = mThirdPartyHDR->execute(*pImgBuf_inEV00FSYUVMain1, *pImgBuf_inEV01FSYUVMain1, *appMeta, *pImgBuf_outCleanFSYUV);
            }
        }
    }
    else
    {
        MY_LOGE("failed to get desired requests: outputRequestPtr:%p, ev00RequestPtr:%p, ev01RequestPtr:%p",
            outputRequestPtr.get(), ev00RequestPtr.get(), ev00RequestPtr.get());
    }
    return bRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyHDRUtility Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ThirdPartyHDRUtility::
tryGetIsoValue(const IMetadata& appMeta, MINT32& isoValue)
{
    MBOOL bRet = MFALSE;

    if(IMetadata::getEntry<MINT32>(&appMeta, MTK_SENSOR_SENSITIVITY, isoValue))
    {
        bRet = MTRUE;
    }
    else
    {
        MY_LOGW("Failed to get app meta: MTK_SENSOR_SENSITIVITY");
    }
    return bRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DefaultThirdPartyHDR Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DefaultThirdPartyHDR::
DefaultThirdPartyHDR()
: ThirdPartyHDRBase()
{

}

MBOOL
DefaultThirdPartyHDR::
onExecute(
    const IImageBuffer& fsYUV00Imgbuf,
    const IImageBuffer& fsYUV01Imgbuf,
    const IMetadata& appMeta,
    IImageBuffer& fsYUVHDRImgbuf)
{
    SCOPED_TRACER();

    // to get the related parameters of bokeh, we can use the utility class
    // or use the argument appMeta directly, for example: use the utility class:
    MINT32 isoValue = -1;
    if(!ThirdPartyHDRUtility::tryGetIsoValue(appMeta, isoValue))
    {
        MY_LOGW("failed to get iso value");
    }

    MRect crop = MRect(fsYUVHDRImgbuf.getImgSize().w, fsYUVHDRImgbuf.getImgSize().h);
    return DCMFIspPipeFlowUtility::imgMemoryCopy(&fsYUVHDRImgbuf, &fsYUV01Imgbuf, crop);
}

MBOOL
DefaultThirdPartyHDR::
onExecute(
    const IImageBuffer& fsYUV00Imgbuf,
    const IImageBuffer& fsYUV01Imgbuf,
    const IMetadata& appMeta,
    const FDResultInfo& fdResult,
    IImageBuffer& fsYUVHDRImgbuf)
{
    SCOPED_TRACER();

    HDRNodeHelper::printHDRInfo(appMeta, fdResult);

    MRect crop = MRect(fsYUVHDRImgbuf.getImgSize().w, fsYUVHDRImgbuf.getImgSize().h);
    return DCMFIspPipeFlowUtility::imgMemoryCopy(&fsYUVHDRImgbuf, &fsYUV01Imgbuf, crop);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ThirdPartyHDRImp Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThirdPartyHDRImp::
ThirdPartyHDRImp()
: ThirdPartyHDRBase()
{

}

MBOOL
ThirdPartyHDRImp::
onInit()
{
    // TODO:
    // init third party HDR here
    return MTRUE;
}

MBOOL
ThirdPartyHDRImp::
onUninit()
{
    // TODO:
    // uninit third party HDR here
    return MTRUE;
}

MBOOL
ThirdPartyHDRImp::
onExecute(
    const IImageBuffer& fsYUV00Imgbuf,
    const IImageBuffer& fsYUV01Imgbuf,
    const IMetadata& appMeta,
    IImageBuffer& fsYUVHDRImgbuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party HDR here
    return MTRUE;
}

MBOOL
ThirdPartyHDRImp::
onExecute(
    const IImageBuffer& fsYUV00Imgbuf,
    const IImageBuffer& fsYUV01Imgbuf,
    const IMetadata& appMeta,
    const FDResultInfo& fdResult,
    IImageBuffer& fsYUVHDRImgbuf)
{
    SCOPED_TRACER();

    // TODO:
    // execute third party HDR here
    return MTRUE;
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

