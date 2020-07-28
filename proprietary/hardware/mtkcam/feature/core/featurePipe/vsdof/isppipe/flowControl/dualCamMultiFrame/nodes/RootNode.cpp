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

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file

// Local header file
#include "RootNode.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "RootNode"
#include <featurePipe/core/include/PipeLog.h>

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
namespace {

/**
 * @class DCMFFrature
 * @brief the mode of Dual-Camera Multi-Frame(DCMF)
 */
enum DCMFFrature
{
    DCMF_INVALID_MODE = -1,
    DCMF_BOKEH_MODE_NORMAL,
    DCMF_MFNR_BOKEH_MODE_NORMAL,
    DCMF_HDR_BOKEH_MODE_NORMAL,
    // special flow
    DCMF_HDR_BOKEH_MODE_FD_TO_HDR,
    DCMF_MFNR_BOKEH_MODE_FD_TO_DEPTH,
};

/**
 * @class RootNodeHelper
 * @brief Helper class for RootNode
 */
class RootNodeHelper
{
public:
    /**
     * @brief get in HDRBokeh mode suport FDNode to HDRNode or doesn't
     * @return
     * - execute result
     */
    static MBOOL getIsSuportedFDToHDRInHDRBokehMode();
    /**
     * @brief get in MFNRBokeh mode suport FDNode to MFNRNode or doesn't
     * @return
     * - execute result
     */
    static MBOOL getIsSuportedFDToDepthInMFNRBokehMode();
    /**
     * @brief try to get the DCMF feature mode
     * @param [in] request target request
     * @param [in] nodeID the node ID
     * @param [out] mode the DCMF feature mode of the arugment request is
     * @return
     * - execute result
     */
    static MBOOL tryGetDCMFFeatureMode(const IspPipeRequestPtr& request, MINT32 nodeID, MINT32& mode);
    /**
     * @brief try to get the feature warning
     * @param [in] request target request
     * @param [in] nodeID the node ID
     * @param [out] mode the feature warning of the arugment request is
     * @return
     * - execute result
     */
    static MBOOL tryGetFeatureWarning(const IspPipeRequestPtr& request, MINT32 nodeID, MINT32& mode);
    /**
     * @brief determine the request is output request or not
     * @param [in] requestPtr target request
     * @return
     * - execute result
     */
    static MBOOL getIsOutputRequest(const IspPipeRequestPtr& requestPtr);
        /**
     * @brief convert the public DCMFFratureMode to the corresponding name
     * @param [in] mode public DCMF frature mode
     * @return
     * - execute result
     */
     static const char* publicDCMFFeatureModeToName(MINT32 mode);
    /**
     * @brief convert the internal DCMFFratureMode to the corresponding name
     * @param [in] mode internal DCMF frature mode
     * @return
     * - execute result
     */
    static const char* internalDCMFFeatureModeToName(MINT32 mode);
    /**
     * @brief decision the DCMF feature mode for internal using
     * @param [in] publicMode the public DCMF feature mode
     * @param [in] internalMode the internal DCMF feature mode
     * @return
     * - the internal DCMF feature mode
     */
    static MBOOL tryGetInternalDCMFFeatureMode(MINT32 publicMode, MINT32& internalMode);
};

} // anonymous namespace
/*******************************************************************************
* Const Definition
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RootNodeHelper Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
RootNodeHelper::
getIsSuportedFDToHDRInHDRBokehMode()
{
    static const MINT32 value = 0;
    const MINT32 newValue = ::property_get_int32("vendor.isppipe.flowCtr.fd2hdr", value);
    if(value != newValue)
    {
        MY_LOGW("force suporting FD To HDR in HDRBokehMode, old:%d, new:%d", value, newValue);
    }
    return newValue;
}

MBOOL
RootNodeHelper::
getIsSuportedFDToDepthInMFNRBokehMode()
{
    static const MINT32 value = 0;
    const MINT32 newValue = ::property_get_int32("vendor.isppipe.flowCtr.fd2depth", value);
    if(value != newValue)
    {
        MY_LOGD("force suporting FD To Depth in MFNRBokehMode, old:%d, new:%d", value, newValue);
    }
    return newValue;
}

MBOOL
RootNodeHelper::
tryGetDCMFFeatureMode(const IspPipeRequestPtr& request, MINT32 nodeID, MINT32& mode)
{
    MBOOL bRet = MFALSE;

    IMetadata* main1HalMeta = request->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    if(main1HalMeta == nullptr)
    {
        MY_LOGE("Failed to get main1 hal meta");
    }
    else
    {
        if(IMetadata::getEntry<MINT32>(main1HalMeta, MTK_STEREO_DCMF_FEATURE_MODE, mode))
        {
            bRet = MTRUE;
        }
        else
        {
            MY_LOGW("Failed to get app meta: MTK_STEREO_DCMF_FEATURE_MODE, main1HalMeta:%p", main1HalMeta);
        }
    }
    return bRet;
}

MBOOL
RootNodeHelper::
tryGetFeatureWarning(const IspPipeRequestPtr& request, MINT32 nodeID, MINT32& mode)
{
    MBOOL bRet = MFALSE;

    IMetadata* inAppMeta = request->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_APP);
    if(inAppMeta == nullptr)
    {
        MY_LOGE("Failed to get in-app meta");
    }
    else
    {
        if(IMetadata::getEntry<MINT32>(inAppMeta, MTK_STEREO_FEATURE_WARNING, mode))
        {
            bRet = MTRUE;
        }
        else
        {
            MY_LOGD("Failed to get app meta: MTK_STEREO_FEATURE_WARNING, appMeta:%p", inAppMeta);
        }
    }
    return bRet;
}

MBOOL
RootNodeHelper::
getIsOutputRequest(const IspPipeRequestPtr& requestPtr)
{
    // Node:
    // if there are new output appended, please modify this function
    MBOOL hasDepthMap = requestPtr->isRequestBuffer(BID_OUT_DEPTHMAP);
    MBOOL hasCleanFSYUV = requestPtr->isRequestBuffer(BID_OUT_CLEAN_FSYUV);
    MBOOL hasBOKEHFSYUV = requestPtr->isRequestBuffer(BID_OUT_BOKEH_FSYUV);

    MY_LOGD("determine output request, hasDepthMap:%d, hasCleanFSYUV:%d, hasBOKEHFSYUV:%d",
        hasDepthMap, hasCleanFSYUV, hasBOKEHFSYUV);

    return (hasDepthMap && hasCleanFSYUV && hasBOKEHFSYUV);
}

const char*
RootNodeHelper::
publicDCMFFeatureModeToName(MINT32 mode)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(mode)
      {
        MAKE_NAME_CASE(MTK_DCMF_FEATURE_BOKEH);
        MAKE_NAME_CASE(MTK_DCMF_FEATURE_MFNR_BOKEH);
        MAKE_NAME_CASE(MTK_DCMF_FEATURE_HDR_BOKEH);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}

const char*
RootNodeHelper::
internalDCMFFeatureModeToName(MINT32 mode)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(mode)
      {
        MAKE_NAME_CASE(DCMF_INVALID_MODE);
        MAKE_NAME_CASE(DCMF_BOKEH_MODE_NORMAL);
        MAKE_NAME_CASE(DCMF_MFNR_BOKEH_MODE_NORMAL);
        MAKE_NAME_CASE(DCMF_HDR_BOKEH_MODE_NORMAL);
        MAKE_NAME_CASE(DCMF_HDR_BOKEH_MODE_FD_TO_HDR);
        MAKE_NAME_CASE(DCMF_MFNR_BOKEH_MODE_FD_TO_DEPTH);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}

MBOOL
RootNodeHelper::
tryGetInternalDCMFFeatureMode(MINT32 publicMode, MINT32& internalMode)
{
    MY_LOGD("publicMode:%d(%s)", publicMode, RootNodeHelper::publicDCMFFeatureModeToName(publicMode));

    MINT32 bRet = MTRUE;
    switch(publicMode)
    {
        case MTK_DCMF_FEATURE_BOKEH:
        {
            internalMode = DCMF_BOKEH_MODE_NORMAL;
            break;
        }
        case MTK_DCMF_FEATURE_MFNR_BOKEH:
        {
            if(RootNodeHelper::getIsSuportedFDToDepthInMFNRBokehMode())
            {
                MY_LOGD("suport FDNode to DepthNode in MFNRBokehMode");
                internalMode = DCMF_MFNR_BOKEH_MODE_FD_TO_DEPTH;
            }
            else
            {
                internalMode = DCMF_MFNR_BOKEH_MODE_NORMAL;
            }
            break;
        }
        case MTK_DCMF_FEATURE_HDR_BOKEH:
        {
            if(RootNodeHelper::getIsSuportedFDToHDRInHDRBokehMode())
            {
                MY_LOGD("suport FDNode to HDRNode in HDRBokehMode");
                internalMode = DCMF_HDR_BOKEH_MODE_FD_TO_HDR;
            }
            else
            {
                internalMode = DCMF_HDR_BOKEH_MODE_NORMAL;
            }
            break;
        }
        default:
        {
            MY_LOGE("unknown public DCMF feature mode, value:%d", publicMode);
            bRet = MFALSE;
        }
    }

    if(bRet)
    {
        MY_LOGD("mapping DCMF feature mode, public:%d(%s), internal:%d(%s)",
            publicMode, RootNodeHelper::publicDCMFFeatureModeToName(publicMode),
            internalMode, RootNodeHelper::internalDCMFFeatureModeToName(internalMode));
    }
    return bRet;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RootNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
RootNode::
RootNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: RootNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

RootNode::
RootNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
, mProcessingFeatureMode(DCMF_INVALID_MODE)
{
    SCOPED_TRACER();

    this->addWaitQueue(&mRequestQue);
}

RootNode::
~RootNode()
{
    SCOPED_TRACER();
}

MBOOL
RootNode::
onInit()
{
    SCOPED_TRACER();

    return MTRUE;
}

MBOOL
RootNode::
onUninit()
{
    SCOPED_TRACER();

    return MTRUE;
}

MBOOL
RootNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
RootNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
RootNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    MY_LOGD("reqID=%d", pRequest->getRequestNo());

    MBOOL ret = MTRUE;
    switch(dataID)
    {
        case ROOT_ENQUE:
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
RootNode::
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
RootNode::
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

    MBOOL bRet = MTRUE;
    // mark on-going-request start
    this->incExtThreadDependency();
    {
        AutoProfileLogging profile("RootNode::threadLoop", pRequest->getRequestNo());

        MINT32 publicDCMFFeatureMode = -1;
        MINT32 internalDCMFFeatureMode = DCMF_INVALID_MODE;

        if(!RootNodeHelper::tryGetDCMFFeatureMode(pRequest, getNodeId(), publicDCMFFeatureMode) || !RootNodeHelper::tryGetInternalDCMFFeatureMode(publicDCMFFeatureMode, internalDCMFFeatureMode))
        {
            MY_LOGE("Failed to get the DCMF feature mode");

            bRet = MFALSE;
        }
        else
        {
            MY_LOGD("get the internal DCMF feature mode:%d(%s)",
                internalDCMFFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(internalDCMFFeatureMode));

            if(mProcessingFeatureMode == DCMF_INVALID_MODE)
            {
                 MY_LOGD("set processingFeatureMode, new:%d(%s), old:%d(%s)",
                    internalDCMFFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(internalDCMFFeatureMode),
                    mProcessingFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(mProcessingFeatureMode));

                 mProcessingFeatureMode = internalDCMFFeatureMode;
            }

            if(internalDCMFFeatureMode != mProcessingFeatureMode)
            {
                MY_LOGE("the wrong flow, the currentMode is different with processingMode, cur:%d(%s), proc:%d(%s)",
                    internalDCMFFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(internalDCMFFeatureMode),
                    mProcessingFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(mProcessingFeatureMode));

                bRet = MFALSE;
            }
            else
            {
                const MINT32 featureWarningforEnableDepthAndBokeh = 0;
                MINT32 featureWarning = featureWarningforEnableDepthAndBokeh;
                if(RootNodeHelper::tryGetFeatureWarning(pRequest, getNodeId(), featureWarning) && (featureWarning != featureWarningforEnableDepthAndBokeh))
                {
                    MY_LOGW("disableDepthAndBokeh, featureWarning:%d", featureWarning);
                    pRequest->getRequestAttr().isDisableDepth = MTRUE;
                    pRequest->getRequestAttr().isDisableBokeh = MTRUE;
                }

                switch(mProcessingFeatureMode)
                {
                    case DCMF_BOKEH_MODE_NORMAL:
                    {
                        pRequest->getRequestAttr().isPureBokeh = MTRUE;

                        this->handleDataAndDump(ROOT_TO_FD, pRequest);
                        break;
                    }
                    case DCMF_MFNR_BOKEH_MODE_NORMAL:
                    {
                        this->handleDataAndDump(ROOT_TO_MFNR, pRequest);
                        this->handleDataAndDump(ROOT_TO_DEPTH, pRequest);
                        this->handleDataAndDump(ROOT_TO_FD, pRequest);
                        break;
                    }
                    case DCMF_MFNR_BOKEH_MODE_FD_TO_DEPTH:
                    {
                        pRequest->getRequestAttr().isFDToDepthFlow = MTRUE;

                        this->handleDataAndDump(ROOT_TO_FD, pRequest);
                        this->handleDataAndDump(ROOT_TO_MFNR, pRequest);
                        break;
                    }
                    case DCMF_HDR_BOKEH_MODE_NORMAL:
                    {
                        this->handleDataAndDump(ROOT_TO_HDR, pRequest);
                        this->handleDataAndDump(ROOT_TO_DEPTH, pRequest);
                        this->handleDataAndDump(ROOT_TO_FD, pRequest);
                        break;
                    }
                    case DCMF_HDR_BOKEH_MODE_FD_TO_HDR:
                    {
                        pRequest->getRequestAttr().isFDToHDRFlow = MTRUE;

                        this->handleDataAndDump(ROOT_TO_FD, pRequest);
                        this->handleDataAndDump(ROOT_TO_DEPTH, pRequest);
                        break;
                    }
                    default:
                    {
                        MY_LOGE("unknown feature mode, req#:%d, featureMode:%d", pRequest->getRequestNo(), mProcessingFeatureMode);

                        bRet = MFALSE;
                        break;
                    }
                }

                if(RootNodeHelper::getIsOutputRequest(pRequest))
                {
                    MY_LOGD("current request is output request, reset processingFeatureMode into invalid, req#:%d, processingFeatureMode:%d(%s)",
                        pRequest->getRequestNo(), mProcessingFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(mProcessingFeatureMode));

                    mProcessingFeatureMode = DCMF_INVALID_MODE;
                }
            }
        }

        if(!bRet)
        {
            MY_LOGE("error occure, dispatch to error notify, featureMode, public:%d(%s), internal:%d(%s), processing:%d(%s), req#:%d",
                publicDCMFFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(publicDCMFFeatureMode),
                internalDCMFFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(internalDCMFFeatureMode),
                mProcessingFeatureMode, RootNodeHelper::internalDCMFFeatureModeToName(mProcessingFeatureMode), pRequest->getRequestNo());

            this->handleData(ERROR_OCCUR_NOTIFY, pRequest);
        }
        // launch onProcessDone
        pRequest->getBufferHandler()->onProcessDone(getNodeId());
    }
    // mark on-going-request end
    this->decExtThreadDependency();
    return bRet;
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

