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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "MultiFrameNode.h"

#include "core/DebugControl.h"
#define PIPE_CLASS_TAG "MultiFrameNode"
#define PIPE_TRACE TRACE_MULTIFRAME_NODE
#include <core/PipeLog.h>
#include <sstream>
#include "pipe/FeaturePlugin.h"

#if 0
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_MULTIFRAME);
#endif

using namespace NSCam::NSPipelinePlugin;

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


/******************************************************************************
*
******************************************************************************/
class MultiFrameInterface : public MultiFramePlugin::IInterface
{
public:
    virtual MERROR offer(MultiFramePlugin::Selection& sel)
    {
        sel.mRequestCount = 0;
        sel.mFrontDummy = 0;
        sel.mPostDummy = 0;

        sel.mIBufferFull
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_I422)
            .addSupportFormat(eImgFmt_BAYER10)
            .addSupportFormat(eImgFmt_BAYER10_UNPAK)
            .addSupportFormat(eImgFmt_RAW16)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportSize(eImgSize_Full)
            .addSupportSize(eImgSize_Specified);

        sel.mIBufferSpecified
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_I422)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_Y8)
            .addSupportSize(eImgSize_Specified)
            .addSupportSize(eImgSize_Quarter);

        sel.mIBufferResized
            .addSupportFormat(eImgFmt_FG_BAYER10)
            .addSupportSize(eImgSize_Resized);

        if (sel.mRequestIndex == 0) {
            sel.mOBufferFull
                .addSupportFormat(eImgFmt_NV12)
                .addSupportFormat(eImgFmt_YV12)
                .addSupportFormat(eImgFmt_YUY2)
                .addSupportFormat(eImgFmt_NV21)
                .addSupportFormat(eImgFmt_I420)
                .addSupportFormat(eImgFmt_I422)
                .addSupportFormat(eImgFmt_Y8)
                .addSupportFormat(eImgFmt_BAYER10)
                .addSupportFormat(eImgFmt_BAYER12_UNPAK)
                .addSupportFormat(eImgFmt_BAYER10_UNPAK)
                .addSupportFormat(eImgFmt_RAW16)
                .addSupportFormat(eImgFmt_MTK_YUV_P010)
                .addSupportSize(eImgSize_Full);

            sel.mOBufferThumbnail
                .addSupportFormat(eImgFmt_NV12)
                .addSupportFormat(eImgFmt_YV12)
                .addSupportFormat(eImgFmt_YUY2)
                .addSupportFormat(eImgFmt_NV21)
                .addSupportSize(eImgSize_Arbitrary);
        }
        return OK;
    };

    virtual ~MultiFrameInterface() {};
};

REGISTER_PLUGIN_INTERFACE(MultiFrame, MultiFrameInterface);

/******************************************************************************
*
******************************************************************************/
class MultiFrameCallback : public MultiFramePlugin::RequestCallback
{
public:
    MultiFrameCallback(MultiFrameNode* pNode)
        : mpNode(pNode)
    {
    }

    virtual void onAborted(MultiFramePlugin::Request::Ptr pPlgRequest)
    {
        MY_LOGD("onAborted request: %p", pPlgRequest.get());
        onCompleted(pPlgRequest, UNKNOWN_ERROR);
    }

    virtual void onCompleted(MultiFramePlugin::Request::Ptr pPlgRequest, MERROR result)
    {
        RequestPtr pRequest = mpNode->findRequest(pPlgRequest);

        if (pRequest == NULL) {
            MY_LOGE("unknown request happened: %p, result %d", pPlgRequest.get(), result);
            return;
        }

        *pPlgRequest = MultiFramePlugin::Request();
        MY_LOGD("onCompleted request: %p, result %d", pPlgRequest.get(), result);
        mpNode->onRequestFinish(pRequest);
    }

    virtual void onNextCapture(MultiFramePlugin::Request::Ptr pPlgRequest)
    {
        if (pPlgRequest == NULL) {
            MY_LOGE("unknown request happened: %p", pPlgRequest.get());
            return;
        }

        mpNode->onNextCapture(pPlgRequest);
    }

    virtual ~MultiFrameCallback() { };
private:
    MultiFrameNode* mpNode;
};


/******************************************************************************
*
******************************************************************************/
MultiFrameNode::MultiFrameNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : FeatureNode(nid, name, 0, policy, priority)//Yuqi CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_MULTIFRAME)
    , mAbortingRequestNo(-1)
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) ctor", this);
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

MultiFrameNode::~MultiFrameNode()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) dtor", this);
    TRACE_FUNC_EXIT();
}

MBOOL MultiFrameNode::onData(FeatureDataHandler::DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "R:%d I/C:%d/%d %s arrived",
                pRequest->getRequestNo(),
                pRequest->getActiveFrameIndex(), pRequest->getActiveFrameCount(),
                PathID2Name(id));
    const NodeID_T* nodeId = GetPath(id);
    NodeID_T sink = nodeId[1];

    if (sink == NID_MULTIRAW)
        pRequest->addParameter(PID_MULTIFRAME_TYPE, 0);
    else if (sink == NID_MULTIYUV)
        pRequest->addParameter(PID_MULTIFRAME_TYPE, 1);

    MBOOL ret = MTRUE;

    if (pRequest->isReadyFor(sink))
        mRequests.enque(pRequest);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MultiFrameNode::onAbort(RequestPtr &pRequest)
{
    Mutex::Autolock _l(mOperLock);
    MBOOL found = MFALSE;

    if (!pRequest->isCancelled())
        return MTRUE;

    RequestPair p;
    {
        Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            RequestPair& rPair = *it;
            if (rPair.mpCapRequest == pRequest) {
                p = rPair;
                found = MTRUE;
                break;
            }
        }
    }

    auto pProvider = p.mpPlgProvider;
    if (!found) {
        // pick a pProvider
        for (size_t i = 0 ; i < mProviderMap.size() ; i++) {
            FeatureID_T featId = mProviderMap.keyAt(i);
            if (pRequest->hasFeature(featId)) {
                pProvider =  mProviderMap.valueFor(featId);
                break;
            }
        }
    }

    // Clear the selection repository
    if (mAbortingRequestNo != pRequest->getRequestNo()) {
        do
        {
            // The rest requests will not queue to capture pipe, while abort() happend
            // Remove the rest selections.
            auto pFrontSel = mPlugin->frontSelection(pProvider);
            if (pFrontSel == NULL || pFrontSel->mRequestIndex == 0) {
                break;
            }
            auto pPopSel = mPlugin->popSelection(pProvider);
            MY_LOGW("Erase Selection C/I: %u/%d",
                pPopSel->mRequestCount, pPopSel->mRequestIndex);
        } while(true);
    }

    if (found) {
        MY_LOGI("+, R Num: %d",
            pRequest->getRequestNo());
        std::vector<PluginRequestPtr> vpPlgRequests;
        vpPlgRequests.push_back(p.mpPlgRequest);

        pProvider->abort(vpPlgRequests);
        MY_LOGI("-, R Num: %d",
            pRequest->getRequestNo());
    }

    mAbortingRequestNo = pRequest->getRequestNo();

    return MTRUE;
}

MBOOL MultiFrameNode::onInit()
{
    TRACE_FUNC_ENTER();
    FeatureNode::onInit();
    MY_LOGI("(%p) uniqueKey:%d, sensorIndex:%d, sensorIndex2:%d", this, mUsageHint.mPluginUniqueKey, mSensorIndex, mSensorIndex2);
    mPlugin = MultiFramePlugin::getInstance(mUsageHint.mPluginUniqueKey, {mSensorIndex, mSensorIndex2});

    FeatureID_T featId;
    auto& pProviders = mPlugin->getProviders();
    mpInterface = mPlugin->getInterface();

    for (auto& pProvider : pProviders) {
        const MultiFramePlugin::Property& rProperty = pProvider->property();
        featId = NULL_FEATURE;

        if (rProperty.mFeatures & MTK_FEATURE_MFNR)
            featId = FID_MFNR;
        else if (rProperty.mFeatures & MTK_FEATURE_AINR)
            featId = FID_AINR;
        else if (rProperty.mFeatures & MTK_FEATURE_HDR)
            featId = FID_HDR;
        else if (rProperty.mFeatures & TP_FEATURE_MFNR)
            featId = FID_MFNR_3RD_PARTY;
        else if (rProperty.mFeatures & TP_FEATURE_HDR)
            featId = FID_HDR_3RD_PARTY;
        else if (rProperty.mFeatures & TP_FEATURE_HDR_DC)
            featId = FID_HDR2_3RD_PARTY;
        else if (rProperty.mFeatures & MTK_FEATURE_AINR_FOR_HDR)
            featId = FID_AINR_YHDR;

        if (featId != NULL_FEATURE) {
            MY_LOGD_IF(mLogLevel, "Find plugin: %s", FeatID2Name(featId));
            mProviderMap.add(featId, pProvider);

            if (rProperty.mInitPhase == ePhase_OnPipeInit && mInitMap.count(featId) <= 0)
            {
                std::function<void()> func = [featId, pProvider]() {
                    MY_LOGD("Init Plugin: %s on init phase +", FeatID2Name(featId));
                    pProvider->init();
                    MY_LOGD("Init Plugin: %s on init phase -", FeatID2Name(featId));
                };

                mInitMap[featId] = mpTaskQueue->addFutureTask(func);
            }
        }
    }

    mpCallback = make_shared<MultiFrameCallback>(this);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MultiFrameNode::onUninit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) uniqueKey:%d, sensorIndex:%d, sensorIndex2:%d", this, mUsageHint.mPluginUniqueKey, mSensorIndex, mSensorIndex2);

    for (size_t i = 0 ; i < mProviderMap.size() ; i++) {
        ProviderPtr pProvider = mProviderMap.valueAt(i);
        FeatureID_T featId = mProviderMap.keyAt(i);

        if (mInitMap.count(featId) > 0) {
            if (!mInitFeatures.hasBit(featId)) {
                MY_LOGD("Wait for initilizing + Feature: %s", FeatID2Name(featId));
                mInitMap[featId].wait();
                MY_LOGD("Wait for initilizing - Feature: %s", FeatID2Name(featId));
                mInitFeatures.markBit(featId);
            }
            pProvider->uninit();
        }
    }

    mProviderMap.clear();
    mInitMap.clear();
    mInitFeatures.clear();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MultiFrameNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MultiFrameNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MultiFrameNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    if (!waitAllQueue())
    {
        TRACE_FUNC("Wait all queue exit");
        return MFALSE;
    }

    RequestPtr pRequest;
    if (!mRequests.deque(pRequest)) {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    } else if (pRequest == NULL) {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    onRequestProcess(pRequest);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID MultiFrameNode::onFlush()
{
    TRACE_FUNC_ENTER();
    FeatureNode::onFlush();
    TRACE_FUNC_EXIT();
}

MBOOL MultiFrameNode::onRequestProcess(RequestPtr& pRequest)
{
    Mutex::Autolock _l(mOperLock);
    incExtThreadDependency();

    MINT32 frameType = pRequest->getParameter(PID_MULTIFRAME_TYPE);
    NodeID_T nodeId = frameType == 0 ? NID_MULTIRAW : NID_MULTIYUV;

    MINT32 requestNo = pRequest->getRequestNo();
    //yuqi MINT32 frameNo = pRequest->getFrameNo();
    if (pRequest->isCancelled()) {
        MY_LOGD("Cancel, R Num: %d", requestNo);
        pRequest->decNodeReference(nodeId);
        onRequestFinish(pRequest);
        return MFALSE;
    }

    FeatureID_T featureId = NULL_FEATURE;
    CAM_TRACE_FMT_BEGIN("mf:process|r%d", requestNo);
    MY_LOGI("(%p)+, R Num: %d", this, requestNo);


    sp<FeatureNodeRequest> pNodeReq =
        pRequest->getNodeRequest(nodeId);

    if (pNodeReq == NULL) {
        MY_LOGE("should not be here if no node request, type:%d", frameType);
        pRequest->addParameter(PID_FAILURE, 1);
        onRequestFinish(pRequest);
        return MFALSE;
    }

    //TypeID_T uFullType = frameType == 0 ? TID_MAN_FULL_RAW : TID_MAN_FULL_YUV;

    auto pPlgRequest = mPlugin->createRequest();

    pPlgRequest->mIBufferFull       = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_YUV, INPUT);
    // ISP 3.0: For R2Y MFNR
    /*
    if (nodeId == NID_MULTIYUV && pPlgRequest->mIBufferFull == NULL)
        pPlgRequest->mIBufferFull   = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_RAW, INPUT);
    */

    pPlgRequest->mIBufferSpecified  = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_SPEC_YUV, INPUT);
    pPlgRequest->mIBufferResized    = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_RSZ_RAW, INPUT);
    pPlgRequest->mIBufferLCS        = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_LCS, INPUT);
    pPlgRequest->mOBufferFull       = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV, OUTPUT);

    pPlgRequest->mIMetadataDynamic  = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_P1_DYNAMIC);
    pPlgRequest->mIMetadataApp      = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_APP);
    pPlgRequest->mIMetadataHal      = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_HAL);
    pPlgRequest->mOMetadataApp      = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_APP);
    pPlgRequest->mOMetadataHal      = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_HAL);

    // update frame info
    pPlgRequest->mRequestIndex = pRequest->getPipelineFrameIndex();
    pPlgRequest->mRequestCount = pRequest->getPipelineFrameCount();

    //pPlgRequest->mRequestBSSIndex = pRequest->getBSSFrameIndex();
    //pPlgRequest->mRequestBSSCount = pRequest->getBSSFrameCount();

    pPlgRequest->mRequestBSSIndex = pPlgRequest->mRequestIndex;
    pPlgRequest->mRequestBSSCount = pPlgRequest->mRequestCount;


    // pick a pProvider
    ProviderPtr pProvider = NULL;
    for (size_t i = 0 ; i < mProviderMap.size() ; i++) {
        FeatureID_T featId = mProviderMap.keyAt(i);
        if (pRequest->hasFeature(featId)) {
            pProvider =  mProviderMap.valueFor(featId);
            featureId = featId;
            break;
        }
    }

    MBOOL ret = MFALSE;
    if (pProvider != NULL)
    {
        if (!mInitFeatures.hasBit(featureId) && (mInitMap.count(featureId) > 0)) {
            MY_LOGD("Wait for initilizing + Feature: %s", FeatID2Name(featureId));
            mInitMap[featureId].wait();
            MY_LOGD("Wait for initilizing - Feature: %s", FeatID2Name(featureId));
            mInitFeatures.markBit(featureId);
        }

        {
            Mutex::Autolock _l(mPairLock);
            auto& rPair = mRequestPairs.editItemAt(mRequestPairs.add());
            rPair.mpCapRequest = pRequest;
            rPair.mpPlgRequest = pPlgRequest;
            rPair.mpPlgProvider = pProvider;
        }
        pProvider->process(pPlgRequest, mpCallback);
        ret = MTRUE;
    }
    else
    {
        MY_LOGE("do not execute a plugin");
        pRequest->addParameter(PID_FAILURE, 1);
        onRequestFinish(pRequest);
    }

    MY_LOGI("(%p)-, R Num: %d", this, requestNo);
    CAM_TRACE_FMT_END();
    return MTRUE;
}

RequestPtr MultiFrameNode::findRequest(PluginRequestPtr& pPlgRequest)
{
    Mutex::Autolock _l(mPairLock);
    for (const auto& rPair : mRequestPairs) {
        if (pPlgRequest == rPair.mpPlgRequest) {
            return rPair.mpCapRequest;
        }
    }

    return NULL;
}

MBOOL MultiFrameNode::onRequestFinish(RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    //yuqi MINT32 frameNo = pRequest->getFrameNo();
    CAM_TRACE_FMT_BEGIN("mf:finish|r%d", requestNo);
    MY_LOGI("(%p)R Num: %d", this, requestNo);

    {
        Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            if ((*it).mpCapRequest == pRequest) {
                mRequestPairs.erase(it);
                break;
            }
        }
    }

    if (pRequest->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0)
    {
        if (pRequest->getParameter(PID_THUMBNAIL_TIMING) == NSPipelinePlugin::eTiming_MultiFrame)
        {
            MUINT32 frameCount = pRequest->getActiveFrameCount();
            MUINT32 frameIndex = pRequest->getActiveFrameIndex();

            if (frameCount == frameIndex + 1)
            {
                if (pRequest->mpCallback != NULL) {
                    MY_LOGD("Nofity next capture at (%dI%d)", frameIndex, frameCount);
                    pRequest->mpCallback->onContinue(pRequest);
                } else {
                    MY_LOGW("have no request callback instance!");
                }
            }
        }
    }

    MINT32 frameType = pRequest->getParameter(PID_MULTIFRAME_TYPE);
    dispatch(pRequest, frameType == 0 ? NID_MULTIRAW: NID_MULTIYUV);

    decExtThreadDependency();
    CAM_TRACE_FMT_END();
    return MTRUE;
}

MVOID MultiFrameNode::onNextCapture(PluginRequestPtr& pPlgRequest)
{
    auto findRequestPair = [this](const PluginRequestPtr& pPlgRequest, RequestPair& rPair) -> MBOOL
    {
        Mutex::Autolock _l(mPairLock);
        for (const auto& item : mRequestPairs) {
            if (pPlgRequest == item.mpPlgRequest) {
                rPair = item;
                return MTRUE;
            }
        }
        return MFALSE;
    };

    RequestPair reqPair;
    if (!findRequestPair(pPlgRequest, reqPair)) {
        MY_LOGE("cannot find the corresponding requestPair: %p", pPlgRequest.get());
    }

    const RequestPtr& pRequest = reqPair.mpCapRequest;
    const MultiFramePlugin::Property& rProperty = reqPair.mpPlgProvider->property();
    const MBOOL isEnableNextCapture = (pRequest->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0);
    const MBOOL hasTiming = pRequest->hasParameter(PID_THUMBNAIL_TIMING);
    if (!isEnableNextCapture || !hasTiming)
    {
        MY_LOGD("failed to processor next capure, isEnableNextCapture:%d, hasTiming:%d", isEnableNextCapture, hasTiming);
        return;
    }

    const MBOOL timing = pRequest->getParameter(PID_THUMBNAIL_TIMING);
    if (rProperty.mThumbnailTiming == timing) {
        if (pRequest->mpCallback != NULL) {
            MY_LOGD("Nofity next capture at (%dI%d), R Num: %d, feature:%#012" PRIx64,
                pRequest->getActiveFrameIndex(), pRequest->getActiveFrameCount(),
                pRequest->getRequestNo(), rProperty.mFeatures);
            pRequest->mpCallback->onContinue(pRequest);
        } else {
            MY_LOGW("have no request callback instance!");
        }
    }
}

MERROR MultiFrameNode::evaluate(NodeID_T nodeId, FeatureInferenceData& rInfer)
{
    auto& rSrcData = rInfer.getSharedSrcData();
    auto& rDstData = rInfer.getSharedDstData();
    auto& rFeatures = rInfer.getSharedFeatures();
    auto& rMetadatas = rInfer.getSharedMetadatas();

    MINT32 iRequestNum = rInfer.getRequestNum();
    MUINT8 uRequestCount = rInfer.getRequestCount();
    MINT8 uRequestIndex = rInfer.getRequestIndex();
    MUINT8 uDroppedCount = rInfer.getDroppedCount();
    MUINT8 uBSSBypassCount = rInfer.getBSSBypassCount();

    MBOOL isEvaluated = MFALSE;
    MBOOL isValid;
    MERROR status = OK;

    // Foreach all loaded plugin
    for (size_t i = 0 ; i < mProviderMap.size() ; i++) {

        FeatureID_T featId = mProviderMap.keyAt(i);
        if (!rInfer.hasFeature(featId)) {
            continue;
        } else if (isEvaluated) {
            MY_LOGE("has duplicated feature: %s", FeatID2Name(featId));
            continue;
        }

        MY_LOGD_IF(mLogLevel, "I/C: %d/%d dropCount=%d BSSBypass=%d isUnderBSS=%d", uRequestIndex, uRequestCount, uDroppedCount, uBSSBypassCount, rInfer.isUnderBSS());

        isValid = MTRUE;

        ProviderPtr pProvider = mProviderMap.valueAt(i);

        if (pProvider->property().mInitPhase == ePhase_OnRequest &&
            mInitMap.count(featId) <= 0)
        {
            std::function<void()> func = [featId, pProvider]() {
                MY_LOGD("Init Plugin: %s on evaluate phase +", FeatID2Name(featId));
                pProvider->init();
                MY_LOGD("Init Plugin: %s on evaluate phase -", FeatID2Name(featId));
            };

            mInitMap[featId] = mpTaskQueue->addFutureTask(func);
        }


        // Gate the evaluate for RAW-Domain or YUV-Domain
        {
            auto token = Selection::createToken(mUsageHint.mPluginUniqueKey, iRequestNum, uRequestIndex);
            auto pFrontSel = mPlugin->frontSelection(pProvider, token);
            if (pFrontSel == NULL) {
                MY_LOGW("Have no a selection, feature:%s, count:%u, index:%u, could be used in another domain. nodeID=%s",
                        FeatID2Name(featId), uRequestCount ,uRequestIndex, NodeID2Name(nodeId));
                return OK;
            }
                const Selection& rFrontSel = *pFrontSel;

            if (rFrontSel.mIBufferFull.getRequired() && rFrontSel.mIBufferFull.isValid()) {
                MINT fmt = rFrontSel.mIBufferFull.getFormats()[0];

                // To determine RAW or YUV domain algo should consider the specified buffer format
                if (rFrontSel.mIBufferSpecified.getRequired() && rFrontSel.mIBufferSpecified.isValid()) {
                    fmt = rFrontSel.mIBufferSpecified.getFormats()[0];
                }
            }
        }

        // Align the lastest set of selection while the first request
        if (uRequestIndex == 0)
        {
            auto token = Selection::createToken(mUsageHint.mPluginUniqueKey, iRequestNum, uRequestIndex);
            auto pFrontSel = mPlugin->frontSelection(pProvider, token);
            if (pFrontSel == NULL)
                break;

            // check the starting selection index should start at 0, if not, there might exist last capture(abort) selection
            if(pFrontSel->mRequestIndex != 0)
            {
                size_t num = mPlugin->numOfSelection(pProvider);
                if(num < uRequestCount)
                {
                    MY_LOGE("Error! Should not happen! I/C: %d/%d dropCount=%d BSSBypass=%d isUnderBSS=%d",
                            uRequestIndex, uRequestCount, uDroppedCount, uBSSBypassCount, rInfer.isUnderBSS());
                    return BAD_VALUE;
                }
                // pop selection
                do
                {
                    auto pPopSel = mPlugin->popSelection(pProvider);
                    MY_LOGW("align selection queue: erased selection C/I: %u/%d num:%zu",
                    pPopSel->mRequestCount, pPopSel->mRequestIndex, num);
                    pFrontSel = mPlugin->frontSelection(pProvider);
                    num = mPlugin->numOfSelection(pProvider);
                    MY_LOGW("current front selection: C/I: %u/%d num:%zu",
                    pFrontSel->mRequestCount, pFrontSel->mRequestIndex, num);
                }while(pFrontSel->mRequestIndex != 0 && num > uRequestCount);

            }
        } else {
            auto token = Selection::createToken(mUsageHint.mPluginUniqueKey, iRequestNum, uRequestIndex);
            auto pFrontSel = mPlugin->frontSelection(pProvider, token);
            if (pFrontSel == NULL) {
                MY_LOGE("have no selection to pop. C/I: %d/%d", uRequestCount, uRequestIndex);
                return OK;
            }
        }

        auto token = Selection::createToken(mUsageHint.mPluginUniqueKey, iRequestNum, uRequestIndex);
        auto pPopSel = mPlugin->popSelection(pProvider, token);
        if (pPopSel == NULL) {
            MY_LOGE("have no selection to pop. C/I: %d/%d", uRequestCount, uRequestIndex);
            return OK;
        }

        // Bypass BSS -> bypass Multiframe node
        if(rInfer.isBypassBSS() || rInfer.isDropFrame())
        {
            MY_LOGD("I/C: %d/%d , bypass MultiframeNode, BSS bypassed:%d dropFrame:%d",
                    uRequestIndex, uRequestCount, rInfer.isBypassBSS(), rInfer.isDropFrame());
            return OK;
        }
        else if(uRequestCount < 2)
        {
            MY_LOGD("I/C: %d/%d , Have not enough frames to blending. count: %d, dropped: %d",
                    uRequestIndex, uRequestCount, uRequestCount, uDroppedCount);
            return OK;
        }

        const Selection& rSel = *pPopSel;

        // should issue AEE if popped selection is not equal to request
        MY_LOGD("Pop Selection C/I: %u/%d; Request C/I :%u/%d",
                rSel.mRequestCount, rSel.mRequestIndex,
                uRequestCount ,uRequestIndex);

        // full size input
        if (rSel.mIBufferFull.getRequired()) {
            MY_LOGD("provider mIBufferFull required");
            if (rSel.mIBufferFull.isValid()) {
                MY_LOGD("provider mIBufferFull required valid");

                auto& src_0 = rSrcData.editItemAt(rSrcData.add());

                // Directly select the first format, using lazy strategy
                MINT fmt = rSel.mIBufferFull.getFormats()[0];
                // Check either RAW-Domain or YUV-Domain
                if ((fmt & eImgFmt_RAW_START) == eImgFmt_RAW_START) {
                    MY_LOGD("provider need input raw");
                    src_0.mTypeId = TID_MAN_FULL_RAW;
                   if (!rInfer.hasType(TID_MAN_FULL_RAW)) {
                       MY_LOGD("infer data not have input TID_MAN_FULL_RAW, invalid");
                        isValid = MFALSE;
                    }
                } else {
                    MY_LOGD("provider need input yuv");
                    src_0.mTypeId = TID_MAN_FULL_YUV;
                    if (!rInfer.hasType(TID_MAN_FULL_YUV)) {
                        MY_LOGD("infer data not have input TID_MAN_FULL_YUV, invalid");
                        isValid = MFALSE;
                    }
                }

                if (isValid) {
                    MY_LOGD("check src, valid");
                    src_0.mSizeId = rSel.mIBufferFull.getSizes()[0];
                    src_0.setFormat(fmt);
                    src_0.addSupportFormats(rSel.mIBufferFull.getFormats());

                    if (src_0.mSizeId == SID_SPECIFIED)
                        src_0.mSize = rSel.mIBufferFull.getSpecifiedSize();
                    else {
                        if ((fmt & eImgFmt_RAW_START) == eImgFmt_RAW_START)
                            src_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
                        else
                            src_0.mSize = rInfer.getSize(TID_MAN_FULL_YUV);
                    }

                    // alignment for MFNR
                    MUINT32 align_w;
                    MUINT32 align_h;
                    rSel.mIBufferFull.getAlignment(align_w, align_h);
                    src_0.mAlign.w = align_w;
                    src_0.mAlign.h = align_h;
                    if (align_w | align_h)
                        MY_LOGD("full buffer: align(%d,%d) size(%dx%d)",
                                align_w, align_h, src_0.mSize.w, src_0.mSize.h);
                }

            }
            else {
                MY_LOGD("check src, invalid");
                isValid = MFALSE;
            }
        }

        // specified size input
        if (isValid && rSel.mIBufferSpecified.getRequired()) {
            if (!rInfer.hasType(TID_MAN_SPEC_YUV))
                isValid = MFALSE;

            if (rSel.mIBufferSpecified.isValid()) {
                auto& src_1 = rSrcData.editItemAt(rSrcData.add());
                src_1.mTypeId = TID_MAN_SPEC_YUV;
                src_1.mSizeId = rSel.mIBufferSpecified.getSizes()[0];
                src_1.addSupportFormats(rSel.mIBufferSpecified.getFormats());
                if (src_1.mSizeId == SID_SPECIFIED)
                    src_1.mSize = rSel.mIBufferSpecified.getSpecifiedSize();
                else if (src_1.mSizeId == SID_QUARTER) {
                    MSize full = rInfer.getSize(TID_MAN_FULL_RAW);
                    src_1.mSize = MSize(full.w / 2, full.h / 2);
                }

                // alignment for MFNR
                MUINT32 align_w;
                MUINT32 align_h;
                rSel.mIBufferSpecified.getAlignment(align_w, align_h);
                src_1.mAlign.w = align_w;
                src_1.mAlign.h = align_h;
                if (align_w | align_h)
                    MY_LOGD("specified buffer: align(%d, %d) size(%d,%d)",
                            align_w, align_h, src_1.mSize.w, src_1.mSize.h);

            } else
                isValid = MFALSE;
        }

        // lcs
        if (isValid && rSel.mIBufferLCS.getRequired()) {
            if (!rInfer.hasType(TID_MAN_LCS))
                isValid = MFALSE;

            auto& src_2 = rSrcData.editItemAt(rSrcData.add());
            src_2.mTypeId = TID_MAN_LCS;
        }

        // resized raw
        if (isValid && rSel.mIBufferResized.getRequired()) {
            if (!rInfer.hasType(TID_MAN_RSZ_RAW))
                isValid = MFALSE;

            if (rSel.mIBufferResized.isValid()) {
                auto& src_3 = rSrcData.editItemAt(rSrcData.add());
                src_3.mTypeId = TID_MAN_RSZ_RAW;
                src_3.mSizeId = rSel.mIBufferResized.getSizes()[0];
                src_3.addSupportFormats(rSel.mIBufferResized.getFormats());
            } else
                isValid = MFALSE;
        }

        // face data
        const MultiFramePlugin::Property& rProperty = pProvider->property();
        if (rProperty.mFaceData == eFD_Current) {
            auto& src_1 = rSrcData.editItemAt(rSrcData.add());
            src_1.mTypeId = TID_MAN_FD;
            src_1.mSizeId = NULL_SIZE;
            rInfer.markFaceData(eFD_Current);
        }
        else if (rProperty.mFaceData == eFD_Cache) {
            rInfer.markFaceData(eFD_Cache);
        }
        else if (rProperty.mFaceData == eFD_None) {
            rInfer.markFaceData(eFD_None);
        }
        else {
            MY_LOGW("unknow faceDateType:%x", rInfer.mFaceDateType.value);
        }

        // full size output
        if (isValid && rSel.mOBufferFull.getRequired()) {
            MY_LOGD("provider mOBufferFull required");
            if (rSel.mOBufferFull.isValid()) {
                MY_LOGD("provider mOBufferFull required valid");
                auto& dst_0 = rDstData.editItemAt(rDstData.add());
                dst_0.mSizeId = rSel.mOBufferFull.getSizes()[0];
                dst_0.addSupportFormats(rSel.mOBufferFull.getFormats());
                if ((dst_0.getFormat()  & eImgFmt_RAW_START) == eImgFmt_RAW_START) {
                    dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
                    dst_0.mTypeId = TID_MAN_FULL_RAW;
                    MY_LOGD("evaluate define dst TID_MAN_FULL_RAW");
                } else {
                    //dst_0.mSize = rInfer.getSize(TID_MAN_FULL_YUV);
                    //dst_0.mTypeId = TID_MAN_FULL_YUV;
                    dst_0.mTypeId = TID_OUT_YUV;
                    MY_LOGD("evaluate define dst TID_OUT_YUV");
                }
            }
            else {
                MY_LOGD("provider mOBufferFull required invalid");
                isValid = MFALSE;
            }
        }

        if (isValid) {
            const MultiFramePlugin::Property& rProperty = pProvider->property();
            rInfer.markThumbnailTiming(rProperty.mThumbnailTiming);
            rInfer.setThumbnailDelay(rProperty.mThumbnailDelay);

            if (rSel.mIMetadataDynamic.getRequired())
                rMetadatas.push_back(MID_MAN_IN_P1_DYNAMIC);

            if (rSel.mIMetadataApp.getRequired())
                rMetadatas.push_back(MID_MAN_IN_APP);

            if (rSel.mIMetadataHal.getRequired())
                rMetadatas.push_back(MID_MAN_IN_HAL);

            if (uRequestIndex == 0 && rSel.mOMetadataApp.getRequired())
                rMetadatas.push_back(MID_MAN_OUT_APP);

            if (uRequestIndex == 0 && rSel.mOMetadataHal.getRequired())
                rMetadatas.push_back(MID_MAN_OUT_HAL);
        }

        if (isValid) {
            isEvaluated = MTRUE;
            rFeatures.push_back(featId);
            if(!rInfer.addNodeIO(nodeId, rSrcData, rDstData, rMetadatas, rFeatures, MTRUE))
            {
                status = BAD_VALUE;
            }
        } else
            MY_LOGW("%s has an invalid evaluation: %s", NodeID2Name(nodeId), FeatID2Name(featId));
    }

    return status;
}

std::string MultiFrameNode::getStatus(std::string& strDispatch)
{
    Mutex::Autolock _l(mPairLock);
    String8 str;
    if (mRequestPairs.size() > 0) {
        const MultiFramePlugin::Property& rProperty = mRequestPairs[0].mpPlgProvider->property();
        str = String8::format("(name:%s/algo:0x%016" PRIx64 ")", rProperty.mName, rProperty.mFeatures);
        if (strDispatch.size() == 0) {
            String8 strTmp = String8::format(" NOT Finish Provider: %s", rProperty.mName);
            strDispatch = strTmp.string();
        }
    }
    return str.string();
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com
