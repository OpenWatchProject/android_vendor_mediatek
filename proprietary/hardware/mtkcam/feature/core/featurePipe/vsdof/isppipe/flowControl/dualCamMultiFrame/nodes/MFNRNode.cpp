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
#include <custom/debug_exif/IDebugExif.h>
#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#include <camera_custom_stereo.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#include <tuple>
#endif

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>

// Module header file
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/aaa/ICaptureNR.h> // SWNR

// Local header file
#include "../DCMFIspPipeFlowUtility.h"
#include "MFNRNode.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "MFNRNode"
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
using namespace mfll;

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class MFNRWrap
 * @brief the wrap class of MFNR
 */
class MFNRWrap{
public:
    enum class Type
    {
        NORMAL,
        DUMMY
    };

    struct GMV
    {
        int x = 0;
        int y = 0;
    };

    MFNRWrap(IspPipeNodeID const nodeId, MINT32 framePerCap, MFNRWrap::Type isDummy = MFNRWrap::Type::NORMAL);

    ~MFNRWrap();

    MVOID createMFLLCore();

    MBOOL isFinished() const;

    MBOOL isDummy() const {return mIsDummy;};

    MBOOL queue(const IspPipeRequestPtr& pRequest);

    IspPipeNodeID getNodeId() {return mNodeId;}

    MFNRNode::RequestCollection&       getProcessedRequests(){return mvProcessedRequests;}

    static MINT32                      getRealIso(IMetadata* pHalMeta);

private:
    static mfll::MfllMode              getMfbMode(){return mfll::MfllMode_ZsdMfhr;};

    static mfll::NoiseReductionType    getPostNRMode(){return mfll::NoiseReductionType_None;};

    static MINT32                      getIsFullSizeMc(){return 1;};

    MINT32                             getFinalIso(IMetadata* pHalMeta);

    MINT64                             getRealShutterTime(IMetadata* pAppMeta);

    MINT64                             getFinalShutterTime(IMetadata* pAppMeta);

    MBOOL                              processMixedYuv(IImageBuffer* pBufCleanYuv);

    MBOOL                              setMfllCoreInputYuvFormat(IImageBuffer* pBufFSYuv);

    MVOID                              doExifUpdate_Mfll(IMetadata* const pHalOutMeta, const std::map<unsigned int, uint32_t>& data);

private:
    IspPipeNodeID                                   mNodeId;

    // MFNR Core library
    sp<mfll::IMfllCore>                             mpCore = nullptr;
    sp<mfll::IMfllCapturer>                         mpCapturer = nullptr;
    sp<mfll::IMfllMfb>                              mpMfb = nullptr;
    MINT32                                          mFramePerCapture = 0;
    MINT32                                          mFrameProcessed = 0;

    MBOOL                                           mIsDummy = MFALSE;

    std::vector<GMV>                                mvGmv;
    MFNRNode::RequestCollection                     mvProcessedRequests;
#if 1
    std::vector< sp<mfll::IMfllImageBuffer> >             mvTempMfllSMBufs;
#endif
};

namespace { // begin anonymous namespace


/*******************************************************************************
* Const Definition
********************************************************************************/
enum SwnrMode
{
    SWNR_MODE_DISABLE,
    SWNR_MODE_INNER_EXECUTE,
    SWNR_MODE_MFCNODE
};

const char*
swnrModeToName(MINT32 mode)
{
    #define MAKE_NAME_CASE(name) \
      case name: return #name
    {
      switch(mode)
      {
        MAKE_NAME_CASE(SWNR_MODE_DISABLE);
        MAKE_NAME_CASE(SWNR_MODE_INNER_EXECUTE);
        MAKE_NAME_CASE(SWNR_MODE_MFCNODE);
      };
    }
    #undef MAKE_NAME_CASE

    return "UNKNOWN";
}

} // end anonymous namespace


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFNRNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MFNRNode::
MFNRNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: IspPipeNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

MFNRNode::
MFNRNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
, mWrapPtr(std::make_unique<MFNRWrap>(getNodeId(), 0, MFNRWrap::Type::DUMMY))
{
    SCOPED_TRACER();

    MY_LOGD("ctor:%p", this);

    this->addWaitQueue(&mRequestQue);

    mSwnrMode = ::property_get_int32("vendor.dg.bmdenoise.swnr", SWNR_MODE_INNER_EXECUTE);
    MY_LOGD("mSwnrMode:%d(%s)", mSwnrMode, swnrModeToName(mSwnrMode));
}

MFNRNode::
~MFNRNode()
{
    SCOPED_TRACER();

    MY_LOGD("ctor:%p, wrapProcessingRequestsSize:%zu, processingRequestsSize:%zu",
        this, mWrapPtr->getProcessedRequests().size(), mProcessingRequests.size());

    int i = 0;
    for (auto it = mWrapPtr->getProcessedRequests().begin(); it != mWrapPtr->getProcessedRequests().end(); it++)
    {
        auto& item = *it;
        MY_LOGW("in ProcessingRequests(non-processing), #%d, req#:%d", i, item->getRequestNo());
        i++;
    }

    i = 0;
    for (auto it = mProcessingRequests.begin(); it != mProcessingRequests.end(); it++)
    {
        auto& item = *it;
        MY_LOGW("in ProcessingRequests(non-processing), #%d, req#:%d", i, item->getRequestNo());
        i++;
    }
}

MBOOL
MFNRNode::
onInit()
{
    SCOPED_TRACER();

    return MTRUE;
}

MBOOL
MFNRNode::
onUninit()
{
    SCOPED_TRACER();

    return MTRUE;
}

MBOOL
MFNRNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
MFNRNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
MFNRNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    MBOOL ret = MTRUE;
    MY_LOGD("reqID=%d", pRequest->getRequestNo());

    switch(dataID)
    {
        case ROOT_TO_MFNR:
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
MFNRNode::
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
MFNRNode::
onThreadLoop()
{
    auto getIsOutputRequest = [](const IspPipeRequestPtr& requestPrt) -> MBOOL
    {
        return requestPrt->isRequestBuffer(BID_OUT_CLEAN_FSYUV);
    };

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

    MY_LOGD("reqID=%d", pRequest->getRequestNo());

    AUTO_THREAD_DEPENDENCY();
    AutoProfileLogging profile("MFNRNode::threadLoop", pRequest->getRequestNo());

    MBOOL isOK = MTRUE;
    MBOOL isFinishedMFNR = MFALSE;
    RequestCollection* requestCollection = nullptr;
    if(isEnableMFNR(pRequest))
    {
        AutoProfileLogging profile("MFNRNode::onThreadLoop::executeMFNR", pRequest->getRequestNo());
        auto stopWatch = pRequest->getStopWatchCollection()->getStopWatch(StopWatchType::Auto, "execute_mfnr");

        // enter mfnr process
        if(executeMFNR(pRequest))
        {
            isFinishedMFNR = mWrapPtr->isFinished();
            if(isFinishedMFNR)
            {
                // SWNR applied after MFNR
                switch(mSwnrMode)
                {
                    case SWNR_MODE_INNER_EXECUTE:
                        {
                            const MINT32 executeSWNRRealIso = ::getSWNRExecutedRealIsoThreshold();
                            const MINT32 realIso = getIsoFromMeta(pRequest);
                            const MBOOL isExecuteSWNR = (realIso >= executeSWNRRealIso);
                            MY_LOGD("decide to execute swnr, realIso:%d, executeSWNRRealIso:%d, isExecuteSWNR:%d",
                                realIso, executeSWNRRealIso, isExecuteSWNR);

                            if(isExecuteSWNR)
                            {
                                MY_LOGD("execute swnr by inner, req#:%d", pRequest->getRequestNo());
                                if(!executeSWNR(pRequest))
                                {
                                    MY_LOGE("failed to execute swnr, req#:%d", pRequest->getRequestNo());
                                    isOK = MFALSE;
                                }
                            }
                        }
                        break;
                    case SWNR_MODE_MFCNODE:
                        {
                            IMetadata* pHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), BID_META_OUT_HAL);
                            if(pHalMeta == nullptr)
                            {
                                MY_LOGE("failed to get halMeta to execute swnr by MFCNode, req#:%d", pRequest->getRequestNo());
                                isOK = MFALSE;
                            }
                            else
                            {
                                MY_LOGD("execute swnr by MFCNode, req#:%d", pRequest->getRequestNo());
                                trySetMetadata<MINT32>(pHalMeta, MTK_NR_MODE_SWNR, MTK_NR_MODE_SWNR);
                            }
                        }
                        break;
                    default:
                        {
                            MY_LOGD("disable swnr, req#:%d", pRequest->getRequestNo());
                        }
                        break;
                }
            }
        }
        else
        {
            MY_LOGE("reqID=%d MFNR execute failed!", pRequest->getRequestNo());
            isOK = MFALSE;
        }
        requestCollection = &mWrapPtr->getProcessedRequests();
    }
    else
    {
        AutoProfileLogging profile("MFNRNode::hande_by_pass", pRequest->getRequestNo());
        auto stopWatch = pRequest->getStopWatchCollection()->getStopWatch(StopWatchType::Auto, "hande_by_pass");

        mProcessingRequests.push_back(pRequest);
        MY_LOGD("push request to requestCollection(size:%zu), req#:%d", mProcessingRequests.size(), pRequest->getRequestNo());

        MBOOL const isOutputRequest = getIsOutputRequest(pRequest);
        if(isOutputRequest)
        {
            MY_LOGD("get outputRequest, req#:%d", pRequest->getRequestNo());
            if(!handleByPass(pRequest))
            {
                MY_LOGE("failed to handleByPass, req#:%d", pRequest->getRequestNo());
                isOK = MFALSE;
            }
            isFinishedMFNR = MTRUE;
        }
        requestCollection = &mProcessingRequests;
    }

    if(isFinishedMFNR || !isOK)
    {
        MY_LOGD("handle data, reason, req#:%d, isFinishedMFNR:%d, not-isOK:%d", pRequest->getRequestNo(), isFinishedMFNR, !isOK);

        using HandleData = MBOOL (MFNRNode::*)(DataID, const IspPipeRequestPtr&);
        HandleData handleDataOperatorPtr = &MFNRNode::handleDataAndDump;
        DataID dataId = MFNR_TO_BOKEH;
        if(!isOK)
        {
            handleDataOperatorPtr = &MFNRNode::handleData;
            dataId = ERROR_OCCUR_NOTIFY;
        }

        for(auto& item : *requestCollection)
        {
            if(getIsOutputRequest(item) && isOK)
            {
                MY_LOGD("set output BID_OUT_CLEAN_FSYUV is ready, req#:%d", item->getRequestNo());
                item->setOutputBufferReady(BID_OUT_CLEAN_FSYUV);
            }

            MY_LOGD("dispatch to next node, req#:%d, dataId:%d", item->getRequestNo(), dataId);
            (this->*handleDataOperatorPtr)(dataId, item);
            item->getBufferHandler()->onProcessDone(getNodeId());
        }
        // TODO: refactor
        requestCollection->clear();

        MY_LOGD("requestContainerSize, wrap:%zu, processingRequests:%zu", mWrapPtr->getProcessedRequests().size(), mProcessingRequests.size());
    }
    return isOK;
}

MBOOL
MFNRNode::
isEnableMFNR(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    // MY_LOGW("temp always by-pass mfnr");
    // return MFALSE;

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    MBOOL ret = MFALSE;

    // check whether do mfnr or not
    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);

    MINT32 enableMfb = 0;
    if(!tryGetMetadata<MINT32>(pInHalMeta, MTK_STEREO_ENABLE_MFB, enableMfb))
    {
        MY_LOGW("reqID=%d, Cannot find MTK_STEREO_ENABLE_MFB meta!, by-pass by default", pRequest->getRequestNo());
    }

    MINT32 debug = property_get_int32("vendor.dg.isppipe.mfnr", enableMfb);

    if( debug != enableMfb )
    {
        MY_LOGW("enableMfb(%d)->(%d)", enableMfb, debug);
        enableMfb = debug;
    }

    ret = (enableMfb == 1) ? MTRUE : MFALSE;
    MY_LOGD("ret(%d)", ret);
    return ret;
}

MBOOL
MFNRNode::
executeMFNR(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    MBOOL bRet = MTRUE;

    // check whether do mfnr or not
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);

    MINT32 framePerCap = 0;
    if(!tryGetMetadata<MINT32>(pInHalMeta, MTK_STEREO_FRAME_PER_CAPTURE, framePerCap))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_STEREO_FRAME_PER_CAPTURE meta!", pRequest->getRequestNo());
        return MFALSE;
    }

    // do mfnr
    if( mWrapPtr->isFinished() )
    {
        MY_LOGD("create new mfnr wrapper, req#:%d", pRequest->getRequestNo());
        mWrapPtr = std::make_unique<MFNRWrap>(getNodeId(), framePerCap, MFNRWrap::Type::NORMAL);
    }
    else
    {
        MY_LOGD("queue to on-going mfnr wrapper, req#:%d", pRequest->getRequestNo());
    }
    mWrapPtr->queue(pRequest);

    return bRet;
}

MBOOL
MFNRNode::
handleByPass(const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    MBOOL bRet = MTRUE;

    // by-pass
    if( !mWrapPtr->isFinished() )
    {
        MY_LOGW("still have unfinished mfnr request!");
    }

    // input main1 full size YUV
    IImageBuffer *pImgBuf_inFSYUVMain1 = nullptr;
    // output clean full size YUV
    IImageBuffer *pImgBuf_outCleanFSYUV = nullptr;

    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inFSYUVMain1);
    // this output buffer will be provided with final request only
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_outCleanFSYUV);
    if( !bRet )
    {
        MY_LOGE("reqID=%d Failed to get I/O buffers, BID_IN_FSYUV_MAIN1:%p, BID_OUT_CLEAN_FSYUV:%p",
            pRequest->getRequestNo(), pImgBuf_inFSYUVMain1, pImgBuf_outCleanFSYUV);
    }
    else
    {
        // no process, directly mem copy
        AutoProfileLogging profile("MFNRNode::memCpy", pRequest->getRequestNo());
        // ----------------------------------------
        MRect crop = MRect(pImgBuf_inFSYUVMain1->getImgSize().w, pImgBuf_inFSYUVMain1->getImgSize().h);
        bRet = DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_outCleanFSYUV, pImgBuf_inFSYUVMain1, crop);
        // ----------------------------------------
    }

    return bRet;
}

MBOOL
MFNRNode::
executeSWNR(const IspPipeRequestPtr& pRequest)
{
    auto printImgInfo = [](const char* msg, IImageBuffer const * img)
    {
        MY_LOGD("%s, size:%dx%d, bpp:%zu, planeCount:%zu, fmt:%d",
            msg, img->getImgSize().w, img->getImgSize().h, img->getImgBitsPerPixel(), img->getPlaneCount(), img->getImgFormat());
    };

    SCOPED_TRACER();

    AutoProfileLogging profile("MFNRNode::executeSWNR", pRequest->getRequestNo());
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    IMetadata* pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);

    // 0. get output clean image buffer
    IImageBuffer *pImgBuf_outCleanFSYUV = nullptr;
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV, pImgBuf_outCleanFSYUV);
    if(pImgBuf_outCleanFSYUV == nullptr)
    {
        MY_LOGE("reqID=%d Failed to get input buffers, BID_OUT_CLEAN_FSYUV:%p",
            pRequest->getRequestNo(), pImgBuf_outCleanFSYUV);

        return MFALSE;
    }

    // 1. request the internal(working) clean image buffer
    IImageBuffer *pImgBuf_internalCleanFSYUV = nullptr;
    {
        AUTO_TIMER("request internal cleanImgBuffer");

        pImgBuf_internalCleanFSYUV = pBufferHandler->requestBuffer(getNodeId(), BID_CLEAN_INERNAL_YV12);

        if(pImgBuf_internalCleanFSYUV == nullptr)
        {
            MY_LOGE("reqID=%d Failed to get internalCleanFSYUV buffers, bfId:%d",
                pRequest->getRequestNo(), BID_CLEAN_INERNAL_YV12);

            return MFALSE;
        }

        printImgInfo("request internalClean", pImgBuf_internalCleanFSYUV);
    }

    // 2. copy imgage bugger from outClean to internalClean
    {
        AutoProfileLogging profile("MFNRNode::executeSWNR::copy_out_to_internal", pRequest->getRequestNo());

        const MSize srcSize = pImgBuf_outCleanFSYUV->getImgSize();
        const MSize dstSize = pImgBuf_internalCleanFSYUV->getImgSize();
        if(dstSize != srcSize)
        {
            MY_LOGD("adjust internal cleanImgBuffer, new:%dx%d, old:%dx%d",
                srcSize.w, srcSize.h, dstSize.w, dstSize.h);

            pImgBuf_internalCleanFSYUV->setExtParam(srcSize, 0);
        }

        printImgInfo("before mdp internalClean", pImgBuf_internalCleanFSYUV);

        const MRect crop = MRect(srcSize.w, srcSize.h);
        if(!DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_internalCleanFSYUV, pImgBuf_outCleanFSYUV, crop))
        {
            MY_LOGE("failed to execute imgMemoryCopy from outClean to internalClean");
            return MFALSE;
        }
    }

    // 3. do swnr
    {
        AutoProfileLogging profile("MFNRNode::executeSWNR::doSWNR", pRequest->getRequestNo());

        MINT32 openId = getOpenIdFromMeta(pRequest);
        MINT32 iso = getIsoFromMeta(pRequest);

        ISwNR::SWNRParam swnrParam;
        // swnrParam.isDualCam = MTRUE;
        std::unique_ptr<ISwNR> swnr = std::unique_ptr<ISwNR>(MAKE_SwNR(openId));
        swnrParam.iso = iso;
        swnrParam.isMfll = MTRUE;

        MY_LOGD("Run SwNR with iso=%d, openId:%d", swnrParam.iso, openId);
        if (!swnr->doSwNR(swnrParam, pImgBuf_internalCleanFSYUV))
        {
            MY_LOGE("SWNR failed!");
            return MFALSE;
        }
        else
        {
            if (pOutHalMeta != nullptr)
            {
                MY_LOGD("SWNR update exif to hal out");
                trySetMetadata<MUINT8>(pOutHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                swnr->getDebugInfo((*pOutHalMeta));
            }
        }
    }

    // 4. format transform back to NV21, copy imgage bugger from internalClean to outClean
    {
        AutoProfileLogging profile("MFNRNode::executeSWNR::copy_internal_to_out", pRequest->getRequestNo());

        const MSize srcSize = pImgBuf_internalCleanFSYUV->getImgSize();
        const MRect crop = MRect(srcSize.w, srcSize.h);
        if(!DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_outCleanFSYUV, pImgBuf_internalCleanFSYUV, crop))
        {
            MY_LOGE("failed to execute imgMemoryCopy from outClean to internalClean");
            return MFALSE;
        }
    }
    return MTRUE;
}

MINT32
MFNRNode::
getOpenIdFromMeta(const IspPipeRequestPtr& pRequest)
{
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);

    MINT32 openId = 0;
    if(tryGetMetadata<MINT32>(pInHalMeta, MTK_STEREO_FEATURE_OPEN_ID, openId))
    {
        MY_LOGD("openId(%d)", openId);
    }
    else
    {
        MY_LOGE("can not get open id");
    }
    return openId;
}

MINT32
MFNRNode::
getIsoFromMeta(const IspPipeRequestPtr& pRequest)
{
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);

    return MFNRWrap::getRealIso(pInHalMeta);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFNRWrap Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MFNRWrap::
MFNRWrap(IspPipeNodeID const nodeId, MINT32 framePerCap, MFNRWrap::Type isDummy)
: mNodeId(nodeId)
, mFramePerCapture(framePerCap)
, mIsDummy((isDummy == MFNRWrap::Type::DUMMY) ? MTRUE : MFALSE)
{
    MY_LOGD("(%p) framePerCap(%d), dummy(%d)", this, framePerCap, isDummy);

    if( !mIsDummy ){
        createMFLLCore();
    }
}

MFNRWrap::
~MFNRWrap()
{
    MY_LOGD("(%p) dtor", this);
}

MVOID
MFNRWrap::
createMFLLCore()
{
    AutoProfileLogging profile("MFNRWrap::createMFLLCore", -1);

    if(mpMfb != nullptr || mpCapturer != nullptr || mpCore!= nullptr){
        MY_LOGE("Should not create without destroy!  %p,%p,%p",
            mpMfb.get(),
            mpCapturer.get(),
            mpCore.get()
        );
        return;
    }

    {
        AutoProfileLogging profile("IMfllCore::createInstance", -1);
        mpCore = IMfllCore::createInstance(0, 0, IMfllCore::Type::MFHR);
    }

    if (mpCore.get() == nullptr) {
        MY_LOGE("create MFNR Core Library failed");
        return;
    }

    mpCapturer = mpCore->getCapturer();
    mpMfb = mpCore->getMfb();

    if (mpCapturer.get() == nullptr) {
        MY_LOGE("create MFNR Capturer failed");
        return;
    }

    if (mpMfb.get() == nullptr) {
        MY_LOGE("create MFNR MFB failed");
        return;
    }
}

MBOOL
MFNRWrap::
isFinished() const
{
    if( mIsDummy )
    {
        // dummy wrapper always finished
        return MTRUE;
    }

    if( mpCore == nullptr && mpCapturer == nullptr && mpMfb == nullptr &&
        ( mFramePerCapture != 0 && mFrameProcessed == mFramePerCapture)
    )
    {
        return MTRUE;
    }
    else
    {
        MY_LOGD("(%p,%p,%p) frames(%d/%d), not finished yet",
            mpCore.get(), mpCapturer.get(), mpMfb.get(),
            mFrameProcessed, mFramePerCapture
        );
        return MFALSE;
    }
}

MBOOL
MFNRWrap::
queue(const IspPipeRequestPtr& pRequest)
{
    if( mIsDummy )
    {
        MY_LOGE("should not queue data to dummy wrapper!");
        return MFALSE;
    }

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    IMetadata* pInAppMeta  = pBufferHandler->requestMetadata(getNodeId(),  BID_META_IN_APP);
    IMetadata* pInHalMeta  = pBufferHandler->requestMetadata(getNodeId(),  BID_META_IN_HAL_MAIN1);
    IMetadata* pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(),  BID_META_OUT_HAL);

    // input
    IImageBuffer *pImgBuf_inFS_YUV_Main1 = nullptr;
    IImageBuffer *pImgBuf_inSM_YUV_Main1 = nullptr;

    MBOOL bufOk = pBufferHandler->getEnqueBuffer(getNodeId(), BID_IN_FSYUV_MAIN1, pImgBuf_inFS_YUV_Main1);
    bufOk &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_IN_SMYUV_MAIN1, pImgBuf_inSM_YUV_Main1);
    if( !bufOk )
    {
        MY_LOGE("reqID=%d Failed to get input buffers, BID_IN_FSYUV_MAIN1:%p, BID_IN_SMYUV_MAIN1:%p",
            pRequest->getRequestNo(), pImgBuf_inFS_YUV_Main1, pImgBuf_inSM_YUV_Main1);
        return MFALSE;
    }

    MINT32 openId = 0;
    if(tryGetMetadata<MINT32>(pInHalMeta, MTK_STEREO_FEATURE_OPEN_ID, openId))
    {
        MY_LOGD("openId(%d), req#:%d", openId, pRequest->getRequestNo());

    }
    else
    {
        MY_LOGE("can not get open id, req#:%d", pRequest->getRequestNo());
    }

    if( mFrameProcessed == 0)
    {
        MY_LOGD("init by first request, req#:%d, mfb(%d) postNR(%d) fullsizeMC(%d)",
        pRequest->getRequestNo(), getMfbMode(), getPostNRMode(), getIsFullSizeMc());
        // init capturer
        {
            AutoProfileLogging profile("MFNRWrap::initCapturer", -1);
            mpCapturer->setMfllCore(mpCore.get());
            mpCapturer->setShotMode(getMfbMode());
            mpCapturer->setPostNrType(getPostNRMode());
        }

        // init mfb
        {
            AutoProfileLogging profile("MFNRWrap::initMfb", -1);
            mpMfb->init(openId);
            mpMfb->setMfllCore(mpCore.get());
            mpMfb->setShotMode(getMfbMode());
            mpMfb->setPostNrType(getPostNRMode());
        }

        // init mfllCore
        {
            AutoProfileLogging profile("MFNRWrap::initCore", -1);

            mpCore->setCaptureResolution(
                pImgBuf_inFS_YUV_Main1->getImgSize().w,
                pImgBuf_inFS_YUV_Main1->getImgSize().h
            );

            setMfllCoreInputYuvFormat(pImgBuf_inFS_YUV_Main1);

            MfllConfig cfg;
            cfg.sensor_id = openId;
            cfg.capture_num = mFramePerCapture;
            cfg.blend_num = mFramePerCapture;
            cfg.mfll_mode = getMfbMode();
            cfg.post_nr_type = getPostNRMode();
            cfg.full_size_mc = getIsFullSizeMc();
            cfg.iso = getFinalIso(pInHalMeta);
            cfg.exp = getFinalShutterTime(pInAppMeta);
            cfg.original_iso = getRealIso(pInHalMeta);
            cfg.original_exp = getRealShutterTime(pInAppMeta);

            MY_LOGD("set mixing buffer to nullptr");
            mpCore->setMixingBuffer(mfll::MixYuvType_Base, nullptr);

            if( mpCore->init(cfg) != MfllErr_Ok )
            {
                MY_LOGE("Init MFLL Core returns fail, req#:%d", pRequest->getRequestNo());
                return MFALSE;
            }
        }

        // get gmv data
        {
            IMetadata::IEntry entry = pInHalMeta->entryFor(MTK_STEREO_BSS_RESULT);
            for(int i=0 ; i<mFramePerCapture ; i++)
            {
                MINT32 originIdx = entry.itemAt(i, Type2Type<MINT32>());
                MINT32 gmv_x = entry.itemAt(i+1, Type2Type<MINT32>());
                MINT32 gmv_y = entry.itemAt(i+2, Type2Type<MINT32>());

                GMV newGMV = {gmv_x, gmv_y};
                mvGmv.push_back(newGMV);
            }
        }
    }

    // queue data to mfll core
    MY_LOGD("queue data to mfll core, req#:%d(%d/%d) +", pRequest->getRequestNo(), mFrameProcessed + 1, mFramePerCapture);

    MUINT8 uRequiredExif = 0;
    IMetadata::getEntry<MUINT8>(pInHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, uRequiredExif);
    MY_LOGD("pInHalMeta uRequiredExif(%d)", static_cast<MINT32>(uRequiredExif));

    // add data to MFLLCore
    std::deque<void*> metaset;
    metaset.push_back(static_cast<void*>(pInAppMeta));
    metaset.push_back(static_cast<void*>(pInHalMeta));
    metaset.push_back(nullptr);
    mpMfb->setSyncPrivateData(metaset);

    sp<IMfllImageBuffer> mfllImgBuf = IMfllImageBuffer::createInstance("FS_YUV");
    mfllImgBuf->setImageBuffer(reinterpret_cast<void*>(pImgBuf_inFS_YUV_Main1));

    sp<IMfllImageBuffer> mfllImgBuf_small = IMfllImageBuffer::createInstance("SM_YUV");

#if 1
    mfllImgBuf_small->setImageFormat(mfll::ImageFormat_Y8);
    mfllImgBuf_small->setAligned(16, 16);
    mfllImgBuf_small->setResolution(pImgBuf_inSM_YUV_Main1->getImgSize().w, pImgBuf_inSM_YUV_Main1->getImgSize().h);
    if (mfllImgBuf_small->initBuffer() != MfllErr_Ok) {
        MY_LOGE("init mfll buffer(SM_YUV) failed, req#:%d", pRequest->getRequestNo());
        return MFALSE;
    }
    mvTempMfllSMBufs.push_back(mfllImgBuf_small);
    {
        IImageBuffer *pImgBuf_mfllImgBuf_small = static_cast<IImageBuffer*>(mfllImgBuf_small->getImageBuffer());
        MRect crop = MRect(pImgBuf_inSM_YUV_Main1->getImgSize().w, pImgBuf_inSM_YUV_Main1->getImgSize().h);
        MBOOL ret = DCMFIspPipeFlowUtility::imgMemoryCopy(pImgBuf_mfllImgBuf_small, pImgBuf_inSM_YUV_Main1, crop);

        if( !ret )
        {
            MY_LOGE("Failed doIImageTransform, req#:%d", pRequest->getRequestNo());
            return MFALSE;
        }
    }
#else
    // can not set buffer directly due to MC alg need 16-alignment mfllBuffer
    mfllImgBuf_small->setImageBuffer(reinterpret_cast<void*>(pImgBuf_inSM_YUV_Main1));
#endif
    mpCapturer->queueSourceImage(mfllImgBuf);
    mpCapturer->queueSourceMonoImage(mfllImgBuf_small);

    MfllMotionVector mv;
    mv.x = mvGmv.at(mFrameProcessed).x;
    mv.y = mvGmv.at(mFrameProcessed).y;
    mpCapturer->queueGmv(mv);

    mvProcessedRequests.push_back(pRequest);
    MY_LOGD("queue data to mfll core, req#:%d(%d/%d) +", pRequest->getRequestNo(), mFrameProcessed + 1, mFramePerCapture);
    mFrameProcessed++;

    // grap result
    if( mFrameProcessed == mFramePerCapture){
        MY_LOGD("queue done, get result from mfll core, req#:%d", pRequest->getRequestNo());
        {
            AutoProfileLogging profile("MFNRWrap::doMfll", -1);
            MY_LOGD("doMfll, req#:%d +", pRequest->getRequestNo());
            mpCore->doMfll();
            MY_LOGD("doMfll, req#:%d -", pRequest->getRequestNo());

            // The OutHalMeta should contain all values of InHalMeta
            *pOutHalMeta = *pOutHalMeta + *pInHalMeta;

            // Exif udpate
            doExifUpdate_Mfll(
                pOutHalMeta,
                mpCore->getExifContainer()->getInfoMap()
            );
        }

        {
            AutoProfileLogging profile("MFNRWrap::process_mixed_yuv", -1);

            // output clean full size YUV
            IImageBuffer *pImgBuf_outCleanFSYUV = nullptr;

            // this output buffer will be provided with final request only
            bufOk = pBufferHandler->getEnqueBuffer(getNodeId(),  BID_OUT_CLEAN_FSYUV, pImgBuf_outCleanFSYUV);
            if( !bufOk )
            {
                MY_LOGE("reqID=%d Failed to get output buffers, BID_OUT_CLEAN_FSYUV:%p",
                    pRequest->getRequestNo(), pImgBuf_outCleanFSYUV);
                return MFALSE;
            }

            if( !processMixedYuv(pImgBuf_outCleanFSYUV) )
            {
                MY_LOGE("failed process mixed yuv, req#:%d", pRequest->getRequestNo());
                return MFALSE;
            }
        }

        mpMfb = nullptr;
        mpCapturer = nullptr;
        mpCore = nullptr;
    }

    return MTRUE;
}

MINT32
MFNRWrap::
getRealIso(IMetadata* pHalMeta)
{
    MINT32 __iso = 100;

    IMetadata exifMeta;
    if( IMetadata::getEntry<IMetadata>(pHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ){
        if( IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, __iso) ){
            MY_LOGD("MTK_3A_EXIF_AE_ISO_SPEED:%d", __iso);
        }else{
            MY_LOGW("Get ISO failed");
        }
    }else{
        MY_LOGW("Get 3A exif failed");
    }

    return __iso;;
}


MINT32
MFNRWrap::
getFinalIso(IMetadata* pHalMeta)
{
    // the same as real iso
    return getRealIso(pHalMeta);
}

MINT64
MFNRWrap::
getRealShutterTime(IMetadata* pAppMeta)
{
    MINT64 __exp = 1;
    auto b = IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, __exp);

    if(b){
        MY_LOGD("MTK_SENSOR_EXPOSURE_TIME:%d", __exp);
    }else{
        MY_LOGW("cant get MTK_SENSOR_EXPOSURE_TIME from appMeta!");
    }
    return __exp;
}

MINT64
MFNRWrap::
getFinalShutterTime(IMetadata* pAppMeta)
{
    // the same as real exp
    return getRealShutterTime(pAppMeta);
}

MBOOL
MFNRWrap::
processMixedYuv(IImageBuffer* pBufCleanYuv)
{
    SCOPED_TRACER();

    // input
    // retrieve mixed YUV image buffer
    auto mfllImgBuf = mpCore->retrieveBuffer(MfllBuffer_MixedYuv);
    if( mfllImgBuf.get() == nullptr )
    {
        MY_LOGE("mixed output buffer is NULL");
        return MFALSE;
    }

    IImageBuffer* pResult = static_cast<IImageBuffer*>(mfllImgBuf->getImageBuffer());
    if( pResult == nullptr )
    {
        MY_LOGE("IMfllImageBuffer is not NULL but result in it is NULL");
        return MFALSE;
    }

    if( pBufCleanYuv == nullptr)
    {
        MY_LOGE("pBufCleanYuv is NULL");
        return MFALSE;
    }

    {
        // use IImageTransform to handle image cropping
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if( transform.get() == nullptr )
        {
            MY_LOGE("IImageTransform is NULL, cannot generate output");
            return MFALSE;
        }

        MRect crop = MRect(pResult->getImgSize().w, pResult->getImgSize().h);
        MBOOL ret = DCMFIspPipeFlowUtility::imgMemoryCopy(pBufCleanYuv, pResult, crop);

        if( !ret )
        {
            MY_LOGE("Failed doIImageTransform!");
            return MFALSE;
        }
    }

#if 0
    // debug
    char writepath[1024];
    snprintf(writepath,
        1024, "sdcard/dcmfshot/mfnr_retrive_%dx%d_%d.yuv",
        pResult->getImgSize().w, pResult->getImgSize().h, pResult->getBufStridesInBytes(0)
    );
    pResult->saveToFile(writepath);

    snprintf(writepath,
        1024, "sdcard/dcmfshot/pBufCleanYuv_%dx%d_%d.yuv",
        pBufCleanYuv->getImgSize().w, pBufCleanYuv->getImgSize().h, pBufCleanYuv->getBufStridesInBytes(0)
    );
    pBufCleanYuv->saveToFile(writepath);
#endif
    return MTRUE;
}

MBOOL
MFNRWrap::
setMfllCoreInputYuvFormat(IImageBuffer* pBufFSYuv)
{
    switch(pBufFSYuv->getImgFormat()){
        case eImgFmt_NV21:
            mpCore->setInputYuvFormat(InputYuvFmt_Nv21);
            MY_LOGD("set input Nv21");
            break;
        case eImgFmt_NV12:
            mpCore->setInputYuvFormat(InputYuvFmt_Nv12);
            MY_LOGD("set input Nv12");
            break;
        default:
            MY_LOGW("unknown fmt(%d), set to NV21", pBufFSYuv->getImgFormat());
            mpCore->setInputYuvFormat(InputYuvFmt_Nv21);
    }

    return MTRUE;
}

MVOID
MFNRWrap::
doExifUpdate_Mfll(IMetadata* const pHalOutMeta, const std::map<unsigned int, uint32_t>& data)
{
#if DCMF_TEMP == 1
    // do nothing
#else
    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pHalOutMeta->entryFor(MTK_3A_EXIF_METADATA);
        if (entry.isEmpty()) {
            MY_LOGW("%s: no MTK_3A_EXIF_METADATA can be used", __FUNCTION__);
            return;
        }
        exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }

    // get exsited bss data and add to data
    IMetadata::Memory oldDebugInfoSet;
    MBOOL haveOldDebugInfo = IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, oldDebugInfoSet);
    MY_LOGD("exifMeta haveOldDebugInfo(%d)", haveOldDebugInfo);

    std::map<unsigned int, uint32_t> cloneData = data;

    if(haveOldDebugInfo){
        auto pTag = reinterpret_cast<debug_exif_field*>(oldDebugInfoSet.editArray());

        auto transferData = [&](MUINT32 key){
            const MUINT32 index = key;
            // pTag[index].u4FieldID    = (0x1000000 | index);
            MUINT32 value = pTag[index].u4FieldValue;
            MY_LOGD("key(%d)value(%d)", key, value);
            cloneData[key] = value;
        };

        // TODO: need refactor
        transferData(MF_TAG_BSS_ON);
        transferData(MF_TAG_BSS_VER);
        transferData(MF_TAG_BSS_FRAME_NUM);
        transferData(MF_TAG_BSS_GAIN_TH0);
        transferData(MF_TAG_BSS_GAIN_TH1);
        transferData(MF_TAG_BSS_MIN_ISP_GAIN);
        transferData(MF_TAG_BSS_LCSO_SIZE);
        /* YPF info */
        transferData(MF_TAG_BSS_YPF_EN);
        transferData(MF_TAG_BSS_YPF_FAC);
        transferData(MF_TAG_BSS_YPF_ADJTH);
        transferData(MF_TAG_BSS_YPF_DFMED0);
        transferData(MF_TAG_BSS_YPF_DFMED1);
        transferData(MF_TAG_BSS_YPF_TH0);
        transferData(MF_TAG_BSS_YPF_TH1);
        transferData(MF_TAG_BSS_YPF_TH2);
        transferData(MF_TAG_BSS_YPF_TH3);
        transferData(MF_TAG_BSS_YPF_TH4);
        transferData(MF_TAG_BSS_YPF_TH5);
        transferData(MF_TAG_BSS_YPF_TH6);
        transferData(MF_TAG_BSS_YPF_TH7);
        /* FD & eye info */
        transferData(MF_TAG_BSS_FD_EN);
        transferData(MF_TAG_BSS_FD_FAC);
        transferData(MF_TAG_BSS_FD_FNUM);
        transferData(MF_TAG_BSS_EYE_EN);
        transferData(MF_TAG_BSS_EYE_CFTH);
        transferData(MF_TAG_BSS_EYE_RATIO0);
        transferData(MF_TAG_BSS_EYE_RATIO1);
        transferData(MF_TAG_BSS_EYE_FAC);
        transferData(MF_TAG_BSS_AEVC_EN);
        /* Best shot and order */
        transferData(MF_TAG_BSS_ORDER_IDX);
        transferData(MF_TAG_BSS_BEST_IDX);
        /* GMV */
        for(int i=0 ; i<mFramePerCapture ; i++){
            transferData(MF_TAG_GMV_00 +i);
        }
    }

    MY_LOGD("size of mfll exif data(%d)", cloneData.size());

    /* set debug information into debug Exif metadata */
    DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
            cloneData,
            &exifMeta);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pHalOutMeta->update(entry_exif.tag(), entry_exif);
#endif
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

