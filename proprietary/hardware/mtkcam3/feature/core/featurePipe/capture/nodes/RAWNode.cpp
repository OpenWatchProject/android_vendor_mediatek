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

#include "RAWNode.h"

#define PIPE_CLASS_TAG "RAWNode"
#define PIPE_TRACE TRACE_RAW_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <sstream>
#include "../CaptureFeaturePlugin.h"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_RAW);

using namespace NSCam::NSPipelinePlugin;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

/******************************************************************************
*
******************************************************************************/
class RawInterface : public RawPlugin::IInterface
{
public:
    virtual MERROR offer(RawPlugin::Selection& sel)
    {
        sel.mIBufferFull
            .addSupportFormat(eImgFmt_BAYER10)
            .addSupportFormat(eImgFmt_BAYER10_UNPAK)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull
            .addSupportFormat(eImgFmt_BAYER10)
            .addSupportFormat(eImgFmt_BAYER10_UNPAK)
            .addSupportSize(eImgSize_Full);

        return OK;
    };

    virtual ~RawInterface() {};
};

REGISTER_PLUGIN_INTERFACE(Raw, RawInterface);

/******************************************************************************
*
******************************************************************************/
class RawCallback : public RawPlugin::RequestCallback
{
public:
    RawCallback(RAWNode* pNode)
        : mpNode(pNode)
    {
    }

    virtual void onAborted(RawPlugin::Request::Ptr pPluginReq)
    {
        *pPluginReq = RawPlugin::Request();
        MY_LOGD("onAborted request: %p", pPluginReq.get());
    }

    virtual void onCompleted(RawPlugin::Request::Ptr pPluginReq, MERROR result)
    {
        RequestPtr pRequest = mpNode->findRequest(pPluginReq);

        if (pRequest == NULL) {
            MY_LOGE("unknown request happened: %p, result %d", pPluginReq.get(), result);
            return;
        }

        *pPluginReq = RawPlugin::Request();
        MY_LOGD("onCompleted request:%p, result:%d",
                pPluginReq.get(), result);

        mpNode->onRequestFinish(pRequest);
    }

    virtual void onNextCapture(RawPlugin::Request::Ptr pPlgRequest)
    {
        RequestPtr pRequest = mpNode->findRequest(pPlgRequest);

        if (pRequest == NULL) {
            MY_LOGE("unknown request happened: %p", pPlgRequest.get());
            return;
        }

        const MUINT32 frameCount = pRequest->getPipelineFrameCount();
        const MUINT32 frameIndex = pRequest->getPipelineFrameIndex();
        MY_LOGW("not support next capture at (%dI%d)", frameIndex, frameCount);
    }

    virtual ~RawCallback() { };
private:
    RAWNode* mpNode;
};


RAWNode::RAWNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_RAW)
    , CaptureFeatureNode(nid, name, 0, policy, priority)
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) ctor", this);
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

RAWNode::~RAWNode()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) dtor", this);
    TRACE_FUNC_EXIT();
}

MVOID RAWNode::setBufferPool(const android::sp<CaptureBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mpBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL RAWNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "Frame %d: %s arrived", pRequest->getRequestNo(), PathID2Name(id));
    MBOOL ret = MTRUE;

    if (pRequest->isReadyFor(mNodeId)) {
        mRequests.enque(pRequest);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RAWNode::onInit()
{
    TRACE_FUNC_ENTER();
    CaptureFeatureNode::onInit();
    MY_LOGI("(%p) uniqueKey:%d", this, mUsageHint.mPluginUniqueKey);

    mPlugin = RawPlugin::getInstance(mUsageHint.mPluginUniqueKey);

    FeatureID_T featureId;
    auto& vpProviders = mPlugin->getProviders();
    mpInterface = mPlugin->getInterface();

    for (auto& pProvider : vpProviders) {
        const RawPlugin::Property& rProperty =  pProvider->property();
        featureId = NULL_FEATURE;

        if (rProperty.mFeatures & MTK_FEATURE_REMOSAIC)
            featureId = FID_REMOSAIC;

        if (featureId != NULL_FEATURE) {
            MY_LOGD_IF(mLogLevel, "%s finds plugin:%s", NodeID2Name(mNodeId), FeatID2Name(featureId));
            auto& item = mProviderPairs.editItemAt(mProviderPairs.add());
            item.mFeatureId = featureId;
            item.mpProvider = pProvider;

            pProvider->init();
        }
    }

    mpCallback = make_shared<RawCallback>(this);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RAWNode::onUninit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) uniqueKey:%d", this, mUsageHint.mPluginUniqueKey);

    for (ProviderPair& p : mProviderPairs) {
        ProviderPtr pProvider = p.mpProvider;
    }

    mProviderPairs.clear();
    mpCurProvider = NULL;

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RAWNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RAWNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL RAWNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    if (!waitAllQueue()) {
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

MBOOL RAWNode::onRequestProcess(RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    CAM_TRACE_FMT_BEGIN("raw:process|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGI("(%p)+, R/F/S Num: %d/%d/%d", this, requestNo, frameNo, masterId);

    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(mNodeId);
    if (pNodeReq == NULL) {
        MY_LOGE("should not be here if no node request");
        return MFALSE;
    }

    // pick a provider
    ProviderPtr pProvider = NULL;
    FeatureID_T featId;
    for (ProviderPair& p : mProviderPairs) {
        featId = p.mFeatureId;
        if (pRequest->hasFeature(featId)) {
            pProvider = p.mpProvider;
            break;
        }
    }

    if (pProvider == NULL) {
        MY_LOGE("do not execute a plugin");
        dispatch(pRequest);
        return MFALSE;
    }

    auto pPluginReq = mPlugin->createRequest();

    pPluginReq->mSensorId = pRequest->getMainSensorIndex();

    pPluginReq->mIBufferFull  = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_RAW, INPUT);
    pPluginReq->mOBufferFull  = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_RAW, OUTPUT);

    pPluginReq->mIMetadataDynamic = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_P1_DYNAMIC);
    pPluginReq->mIMetadataApp = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_APP);
    pPluginReq->mIMetadataHal = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_HAL);
    pPluginReq->mOMetadataApp = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_APP);
    pPluginReq->mOMetadataHal = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_HAL);

    {
        Mutex::Autolock _l(mPairLock);
        auto& rPair = mRequestPairs.editItemAt(mRequestPairs.add());
        rPair.mPipe = pRequest;
        rPair.mPlugin = pPluginReq;
        mpCurProvider = pProvider;
    }

    incExtThreadDependency();
    if (pProvider->process(pPluginReq, mpCallback) != OK) {
        onRequestFinish(pRequest);
        AEE_ASSERT("R/F Num: %d/%d, RAW plugin process failed! fid=%s\nCRDISPATCH_KEY:CaptureFeaturePipe",
                    pRequest->getRequestNo(), pRequest->getFrameNo(), FeatID2Name(featId));
        return MFALSE;
    }

    MY_LOGI("(%p)-, R/F/S Num: %d/%d/%d", this, requestNo, frameNo, masterId);
    CAM_TRACE_FMT_END();
    return MTRUE;
}

RequestPtr RAWNode::findRequest(PluginRequestPtr& pPluginReq)
{
    Mutex::Autolock _l(mPairLock);
    for (const auto& rPair : mRequestPairs) {
        if (pPluginReq == rPair.mPlugin) {
            return rPair.mPipe;
        }
    }

    return NULL;
}

MBOOL RAWNode::onRequestFinish(RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    CAM_TRACE_FMT_BEGIN("raw:finish|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGI("(%p)+, R/F/S Num: %d/%d/%d", this, requestNo, frameNo, masterId);

    {
        Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            if ((*it).mPipe == pRequest) {
                mRequestPairs.erase(it);
                break;
            }
        }
        if (mRequestPairs.empty()) {
            mpCurProvider = NULL;
        }
    }

    dispatch(pRequest);

    decExtThreadDependency();
    CAM_TRACE_FMT_END();
    MY_LOGI("(%p)-, R/F/S Num: %d/%d/%d", this, requestNo, frameNo, masterId);
    return MTRUE;
}

MERROR RAWNode::evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    (void) nodeId;
    MBOOL isValid;
    MBOOL isEvaluated = MFALSE;
    MERROR status = OK;

    const MINT32 iRequestNum = rInfer.getRequestNum();
    const MINT8 uRequestIndex = rInfer.getRequestIndex();
    MINT32 sensorId = rInfer.mSensorIndex;
    MBOOL isPhysical = rInfer.mIsPhysical;

    // Foreach all loaded plugin
    for (ProviderPair& p : mProviderPairs) {
        FeatureID_T featId = p.mFeatureId;

        if (!rInfer.hasFeature(featId)) {
            continue;
        } else if (isEvaluated) {
            MY_LOGE("has duplicated feature: %s", FeatID2Name(featId));
            continue;
        }

        auto& rSrcData = rInfer.getSharedSrcData();
        auto& rDstData = rInfer.getSharedDstData();
        auto& rFeatures = rInfer.getSharedFeatures();
        auto& metadatas = rInfer.getSharedMetadatas();

        isValid = MTRUE;

        ProviderPtr pProvider = p.mpProvider;
        const RawPlugin::Property& rProperty =  pProvider->property();

#if 1
        // should get selection from camera setting
        auto token = Selection::createToken(mUsageHint.mPluginUniqueKey, iRequestNum, uRequestIndex, sensorId, isPhysical);
        auto pSelection = mPlugin->popSelection(pProvider, token);
        if (pSelection == NULL) {
            MY_LOGE("can not pop the selection, feature:%s",
                    FeatID2Name(featId));
            rInfer.clearFeature(featId);
            return BAD_VALUE;
        }
        const Selection& sel = *pSelection;
#else
        Selection sel;
        mpInterface->offer(sel);
        sel.mIMetadataHal.setControl(rInfer.mpIMetadataHal);
        sel.mIMetadataApp.setControl(rInfer.mpIMetadataApp);
        sel.mIMetadataDynamic.setControl(rInfer.mpIMetadataDynamic);
        if (pProvider->negotiate(sel) != OK) {
            MY_LOGD("bypass %s after negotiation", FeatID2Name(featId));
            rInfer.clearFeature(featId);
            continue;
        }
#endif

        if (!rInfer.hasType(TID_MAN_FULL_RAW))
            isValid = MFALSE;

        // full size input
        if (isValid && sel.mIBufferFull.getRequired()) {
            if (sel.mIBufferFull.isValid()) {
                auto& src_0 = rSrcData.editItemAt(rSrcData.add());
                src_0.mTypeId = TID_MAN_FULL_RAW;
                src_0.mSizeId = sel.mIBufferFull.getSizes()[0];
                // Directly select the first format, using lazy strategy
                src_0.addSupportFormats(sel.mIBufferFull.getFormats());
                // In-place processing must add a output
                if (rProperty.mInPlace) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_MAN_FULL_RAW;
                    dst_0.mSizeId = src_0.mSizeId;
                    dst_0.copyFormats(src_0);
                    dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
                    dst_0.mInPlace = true;
                }
            } else
                isValid = MFALSE;
        }


        // full size output
        if (isValid && !rProperty.mInPlace && sel.mOBufferFull.getRequired()) {
            if (sel.mOBufferFull.isValid()) {
                auto& dst_0 = rDstData.editItemAt(rDstData.add());
                dst_0.mTypeId = TID_MAN_FULL_RAW;
                dst_0.mSizeId = sel.mOBufferFull.getSizes()[0];
                dst_0.addSupportFormats(sel.mOBufferFull.getFormats());
                dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
            } else
                isValid = MFALSE;
        }

        if (isValid) {
            if (sel.mIMetadataDynamic.getRequired())
                metadatas.push_back(MID_MAN_IN_P1_DYNAMIC);

            if (sel.mIMetadataApp.getRequired())
                metadatas.push_back(MID_MAN_IN_APP);

            if (sel.mIMetadataHal.getRequired())
                metadatas.push_back(MID_MAN_IN_HAL);

            if (sel.mIMetadataApp.getRequired())
                metadatas.push_back(MID_MAN_OUT_APP);

            if (sel.mOMetadataHal.getRequired())
                metadatas.push_back(MID_MAN_OUT_HAL);
        }

        if (isValid) {
            isEvaluated = MTRUE;
            rFeatures.push_back(featId);
            if(!rInfer.addNodeIO(mNodeId, rSrcData, rDstData, metadatas, rFeatures))
            {
                status = BAD_VALUE;
                break;
            }
        } else
            MY_LOGW("%s has invalid evaluation:%s",NodeID2Name(mNodeId), FeatID2Name(featId));
    }

    return status;
}

std::string RAWNode::getStatus(std::string& strDispatch)
{
    Mutex::Autolock _l(mPairLock);
    String8 str;
    if (mRequestPairs.size() > 0 && mpCurProvider) {
        const RawPlugin::Property& rProperty = mpCurProvider->property();
        str = String8::format("(name:%s/algo:0x%016" PRIx64 ")", rProperty.mName, rProperty.mFeatures);
        if (strDispatch.size() == 0) {
            String8 strTmp = String8::format(" NOT Finish Provider: %s", rProperty.mName);
            strDispatch = strTmp.string();
        }
    }
    return str.string();
}
} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
