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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "mtkcam-RequestSettingPolicyMediator_SMVRBatch"

#include <mtkcam3/pipeline/policy/IRequestSettingPolicyMediator.h>
#include <mtkcam3/pipeline/policy/InterfaceTableDef.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/std/ULog.h>

#include "MyUtils.h"

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;
using namespace NSCam::v3::Utils;


#define ThisNamespace   RequestSettingPolicyMediator_SMVRBatch

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
class ThisNamespace
    : public IRequestSettingPolicyMediator
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Static data (unchangable)
    std::shared_ptr<PipelineStaticInfo const>       mPipelineStaticInfo;
    std::shared_ptr<PipelineUserConfiguration const>mPipelineUserConfiguration;
    std::shared_ptr<PolicyTable const>              mPolicyTable;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    ThisNamespace(MediatorCreationParams const& params);
                    virtual ~ThisNamespace() { }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IRequestSettingPolicyMediator Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    evaluateRequest(
                        RequestOutputParams& out,
                        RequestInputParams const& in
                    ) -> int override;
private:
//    bool                                            misFdEnabled = false;

};


/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<IRequestSettingPolicyMediator>
makeRequestSettingPolicyMediator_SMVRBatch(MediatorCreationParams const& params)
{
    return std::make_shared<ThisNamespace>(params);
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(MediatorCreationParams const& params)
    : IRequestSettingPolicyMediator()
    , mPipelineStaticInfo(params.pPipelineStaticInfo)
    , mPipelineUserConfiguration(params.pPipelineUserConfiguration)
    , mPolicyTable(params.pPolicyTable)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
evaluateRequest(
    RequestOutputParams& out,
    RequestInputParams const& in
) -> int
{
//    fdintent::RequestOutputParams fdOut;
//    fdintent::RequestInputParams fdIn;
    p2nodedecision::RequestOutputParams p2DecisionOut;
    p2nodedecision::RequestInputParams p2DecisionIn;
    featuresetting::RequestOutputParams featureOut;
    featuresetting::RequestInputParams featureIn;
    capturestreamupdater::RequestOutputParams capUpdaterOut;
    capturestreamupdater::RequestInputParams capUpdaterIn;
    android::sp<IImageStreamInfo>              pJpeg_YUV = nullptr;
    android::sp<IImageStreamInfo>              pThumbnail_YUV = nullptr;

    bool noP2Node = !in.pConfiguration_PipelineNodesNeed->needP2CaptureNode && !in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
    // (1) is face detection intent? /* SMVRBatch: no need for FD */

    // (2.1) are any capture streams updated?
    if ( in.pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr && CC_LIKELY(mPolicyTable->fCaptureStreamUpdater != nullptr) )
    {
        capUpdaterIn.sensorID                        = mPipelineStaticInfo->sensorId[0];
        capUpdaterIn.pRequest_AppControl             = in.pRequest_AppControl;
        capUpdaterIn.pRequest_ParsedAppMetaControl   = in.pRequest_ParsedAppMetaControl;
        capUpdaterIn.isJpegRotationSupported         = true; // use property to control is support or not?
        capUpdaterIn.pConfiguration_HalImage_Jpeg_YUV      = &(in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV);
        capUpdaterIn.pConfiguration_HalImage_Thumbnail_YUV = &(in.pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV);
        // prepare out buffer
        capUpdaterOut.pHalImage_Jpeg_YUV             = &pJpeg_YUV;
        capUpdaterOut.pHalImage_Thumbnail_YUV        = &pThumbnail_YUV;

        mPolicyTable->fCaptureStreamUpdater(capUpdaterOut, capUpdaterIn);
    }

    // (2.2) P2Node decision: the responsibility of P2StreamNode and P2CaptureNode
    if (CC_LIKELY(mPolicyTable->fP2NodeDecision != nullptr))
    {
        p2DecisionIn.requestNo                       = in.requestNo;
        p2DecisionIn.hasP2CaptureNode                = false; // SMVRBathc: only use P2StreamNode
        p2DecisionIn.hasP2StreamNode                 = in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
        p2DecisionIn.isFdEnabled                     = false; // SMVRBathc: no need for fd
        p2DecisionIn.pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
        p2DecisionIn.pConfiguration_StreamInfo_P1    = &((*(in.pConfiguration_StreamInfo_P1))[0]); // use main1 info
        p2DecisionIn.pRequest_AppControl             = in.pRequest_AppControl;
        p2DecisionIn.pRequest_AppImageStreamInfo     = in.pRequest_AppImageStreamInfo;
        p2DecisionIn.pRequest_ParsedAppMetaControl   = in.pRequest_ParsedAppMetaControl;
        p2DecisionIn.needThumbnail                   = (pThumbnail_YUV != nullptr);
        mPolicyTable->fP2NodeDecision(p2DecisionOut, p2DecisionIn);
    }

    // (2.3) feature setting
    if (CC_LIKELY(mPolicyTable->mFeaturePolicy != nullptr)
        && (!noP2Node))
    {
        featureIn.requestNo                          = in.requestNo;
        featureIn.Configuration_HasRecording         = mPipelineUserConfiguration->pParsedAppImageStreamInfo->hasVideoConsumer;
        featureIn.maxP2CaptureSize                   = p2DecisionOut.maxP2CaptureSize;
        featureIn.maxP2StreamSize                    = p2DecisionOut.maxP2StreamSize;
        featureIn.needP2CaptureNode                  = p2DecisionOut.needP2CaptureNode;
        featureIn.needP2StreamNode                   = p2DecisionOut.needP2StreamNode;
        featureIn.pConfiguration_StreamInfo_P1       = in.pConfiguration_StreamInfo_P1;
        featureIn.pRequest_AppControl                = in.pRequest_AppControl;
        featureIn.pRequest_AppImageStreamInfo        = in.pRequest_AppImageStreamInfo;
        featureIn.pRequest_ParsedAppMetaControl      = in.pRequest_ParsedAppMetaControl;
        for( size_t i = 0; i < mPipelineStaticInfo->sensorId.size(); i++)
        {
            featureIn.sensorMode.push_back((*in.pSensorMode)[i]);
        }
        mPolicyTable->mFeaturePolicy->evaluateRequest(&featureOut, &featureIn);

        out.needZslFlow                              = false; /* SMVRBatch: no need for ZSL */
        // out.zslPolicyParams                          = featureOut.zslPolicyParams;
        out.needReconfiguration                      = featureOut.needReconfiguration;
        out.sensorMode                               = featureOut.sensorMode;
        out.reconfigCategory                         = featureOut.reconfigCategory;
        out.vboostControl                            = featureOut.vboostControl;
    }


    // (3) build every frames out param
    #define NOT_DUMMY (0)
    #define POST_DUMMY (1)
    #define PRE_DUMMY (2)
    auto buildOutParam = [&] (std::shared_ptr<featuresetting::RequestResultParams> const setting, int dummy, bool isMain) -> int
    {
        // SMVRBatch: dynamic change streamInfo for preview case
        auto pParsedAppConfiguration = mPipelineUserConfiguration->pParsedAppConfiguration;
        auto const& pParsedSMVRBatchInfo = pParsedAppConfiguration->pParsedSMVRBatchInfo;

        if (pParsedSMVRBatchInfo != nullptr)
        {
            MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel, "SMVRBatch: build out frame param +");
        }
        std::shared_ptr<RequestResultParams> result = std::make_shared<RequestResultParams>();
        // build topology
        topology::RequestOutputParams topologyOut;
        topology::RequestInputParams topologyIn;
        if (CC_LIKELY(mPolicyTable->fTopology != nullptr))
        {
            topologyIn.isDummyFrame = dummy != 0;
            topologyIn.isFdEnabled  = false; // SMVRBatch: no need for FD
            topologyIn.needP2CaptureNode = p2DecisionOut.needP2CaptureNode;
            topologyIn.needP2StreamNode  = (isMain) ? p2DecisionOut.needP2StreamNode : false; // p2 stream node don't need process sub frame
            topologyIn.pConfiguration_PipelineNodesNeed = in.pConfiguration_PipelineNodesNeed; // topology no need to ref?
            topologyIn.pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
            topologyIn.pPipelineStaticInfo = mPipelineStaticInfo.get();
            topologyIn.pRequest_AppImageStreamInfo = (isMain) ? in.pRequest_AppImageStreamInfo : nullptr;
            topologyIn.pRequest_ParsedAppMetaControl = (isMain) ? in.pRequest_ParsedAppMetaControl : nullptr;
            topologyIn.pvImageStreamId_from_CaptureNode = &(p2DecisionOut.vImageStreamId_from_CaptureNode);
            topologyIn.pvImageStreamId_from_StreamNode = &(p2DecisionOut.vImageStreamId_from_StreamNode);
            topologyIn.pvMetaStreamId_from_CaptureNode = &(p2DecisionOut.vMetaStreamId_from_CaptureNode);
            topologyIn.pvMetaStreamId_from_StreamNode = &(p2DecisionOut.vMetaStreamId_from_StreamNode);
            topologyIn.pvNeedP1Dma = (setting == nullptr) ? nullptr : &(setting->needP1Dma);
            // prepare output buffer
            topologyOut.pNodesNeed     = &(result->nodesNeed);
            topologyOut.pNodeSet       = &(result->nodeSet);
            topologyOut.pRootNodes     = &(result->roots);
            topologyOut.pEdges         = &(result->edges);
            mPolicyTable->fTopology(topologyOut, topologyIn);
        }
        // build P2 IO map
        iomap::RequestOutputParams iomapOut;
        iomap::RequestInputParams iomapIn;
        if ( CC_LIKELY(mPolicyTable->fIOMap_P2Node != nullptr)
          && CC_LIKELY(mPolicyTable->fIOMap_NonP2Node != nullptr) )
        {
            std::vector<uint32_t> needP1Dma;
            iomapIn.pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
            iomapIn.pConfiguration_StreamInfo_P1    = in.pConfiguration_StreamInfo_P1;
            iomapIn.pRequest_HalImage_Thumbnail_YUV = pThumbnail_YUV.get();
            iomapIn.pRequest_AppImageStreamInfo     = in.pRequest_AppImageStreamInfo;
            iomapIn.pRequest_ParsedAppMetaControl   = in.pRequest_ParsedAppMetaControl;
            if (setting != nullptr)
            {

                result->needKeepP1BuffersForAppReprocess = setting->needKeepP1BuffersForAppReprocess;

                // SMVRBatch: no need for
                // 1. private/opaque
                // 2. FD
                // 3. App_RAW16 output

                if (setting->needP1Dma.size() > 0 && mPipelineStaticInfo->isP1DirectScaledYUV && isMain)
                {
                    setting->needP1Dma[0] |= P1_SCALEDYUV;
                }

                iomapIn.pRequest_NeedP1Dma              = &(setting->needP1Dma);
            }
            else
            {
                for (size_t i = 0; i < topologyOut.pNodesNeed->needP1Node.size(); i++)
                {
                    uint32_t P1Dma = 0;
                    if (topologyOut.pNodesNeed->needP1Node[i] == true)
                    {
                        P1Dma = P1_IMGO;
                    }
                    needP1Dma.push_back(P1Dma);
                }
                iomapIn.pRequest_NeedP1Dma              = &(needP1Dma);
            }
            iomapIn.pRequest_PipelineNodesNeed      = topologyOut.pNodesNeed;
            iomapIn.pvImageStreamId_from_CaptureNode = &(p2DecisionOut.vImageStreamId_from_CaptureNode);
            iomapIn.pvImageStreamId_from_StreamNode = &(p2DecisionOut.vImageStreamId_from_StreamNode);
            iomapIn.pvMetaStreamId_from_CaptureNode = &(p2DecisionOut.vMetaStreamId_from_CaptureNode);
            iomapIn.pvMetaStreamId_from_StreamNode = &(p2DecisionOut.vMetaStreamId_from_StreamNode);
            iomapIn.isMainFrame                     = isMain;
            iomapIn.pPipelineStaticInfo              = mPipelineStaticInfo.get();
            iomapIn.isDummyFrame                    = dummy != 0;
            // prepare output buffer
            iomapOut.pNodeIOMapImage          = &(result->nodeIOMapImage);
            iomapOut.pNodeIOMapMeta           = &(result->nodeIOMapMeta);
            if (dummy == NOT_DUMMY)
            {
                mPolicyTable->fIOMap_P2Node(iomapOut, iomapIn);
            }
            mPolicyTable->fIOMap_NonP2Node(iomapOut, iomapIn);
        }
        //
        if (isMain)
        {
            if ( pJpeg_YUV != nullptr)
            {
                result->vUpdatedImageStreamInfo[pJpeg_YUV->getStreamId()] = pJpeg_YUV;
            }
            if ( pThumbnail_YUV != nullptr)
            {
                result->vUpdatedImageStreamInfo[pThumbnail_YUV->getStreamId()] = pThumbnail_YUV;
            }
        }
        if (setting != nullptr)
        {
            result->additionalApp = setting->additionalApp;
            result->additionalHal = setting->additionalHal;
        }
        else
        {
            result->additionalApp = std::make_shared<IMetadata>();
            for (size_t i = 0; i < topologyOut.pNodesNeed->needP1Node.size(); i++)
            {
                result->additionalHal.push_back(std::make_shared<IMetadata>());
            }
        }

        // update Metdata
        if (CC_LIKELY(mPolicyTable->pRequestMetadataPolicy != nullptr))
        {
            requestmetadata::EvaluateRequestParams metdataParams;
            metdataParams.requestNo                     = in.requestNo;
            metdataParams.isZSLMode                     = false; /* SMVRBatch: no need for ZSL */
            metdataParams.pRequest_AppImageStreamInfo   = in.pRequest_AppImageStreamInfo;
            metdataParams.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
            metdataParams.pSensorSize                   = in.pSensorSize;
            metdataParams.pAdditionalApp                = result->additionalApp;
            metdataParams.pAdditionalHal                = &result->additionalHal;
            metdataParams.pRequest_AppControl           = in.pRequest_AppControl;
            metdataParams.needReconfigure               = featureOut.needReconfiguration;
            metdataParams.pSensorId                     = &mPipelineStaticInfo->sensorId;
            for (size_t i = 0; i < in.pConfiguration_StreamInfo_P1->size(); i++)
            {
                if ((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo != nullptr)
                {
                    metdataParams.RrzoSize.push_back((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo->getImgSize());
                }
            }
            mPolicyTable->pRequestMetadataPolicy->evaluateRequest(metdataParams);
        }

        // SMVRBatch: dynamic change streamInfo for preview case
        if (pParsedSMVRBatchInfo)
        {
            bool needToUpdateHalP1StreamInfo =
                in.pRequest_AppImageStreamInfo->hasVideoConsumer == false // for preview
                && in.pConfiguration_StreamInfo_P1 != nullptr
                && in.pConfiguration_StreamInfo_P1->size()
                   ;

            if (needToUpdateHalP1StreamInfo)
            {
                // rrzo
                sp<IImageStreamInfo> spRrzoStreamInfo = (*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_Rrzo;
                if (spRrzoStreamInfo != nullptr && spRrzoStreamInfo->getAllocImgFormat() == eImgFmt_BLOB)
                {
                    auto pStreamInfo =
                        ImageStreamInfoBuilder(spRrzoStreamInfo.get())
                        .setBufCount(1)
                        .setStartOffset(0)
                        .setBufStep(0)
                        .build();
                    result->vUpdatedImageStreamInfo[spRrzoStreamInfo->getStreamId()] = pStreamInfo;
                }

                // lcso
                sp<IImageStreamInfo> spLcsoStreamInfo = (*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_Lcso;
                if (spLcsoStreamInfo != nullptr && spLcsoStreamInfo->getAllocImgFormat() == eImgFmt_BLOB)
                {
                    auto pStreamInfo =
                        ImageStreamInfoBuilder(spLcsoStreamInfo.get())
                        .setBufCount(1)
                        .setStartOffset(0)
                        .setBufStep(0)
                        .build();
                    result->vUpdatedImageStreamInfo[spLcsoStreamInfo->getStreamId()] = pStreamInfo;
                }
                MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel,
                    "SMVRBatch: requestNo=%d: set bufOffsets.size=1 for rrzo(ptr=%p, fmt=0x%x), lcso(ptr=%p, fmt=0x%x), eImgFmt_BLOB=0x%x",
                    in.requestNo,
                    spRrzoStreamInfo.get(), spRrzoStreamInfo->getAllocImgFormat(),
                    spLcsoStreamInfo.get(), spLcsoStreamInfo->getAllocImgFormat(),
                    eImgFmt_BLOB
                );
            }
        }

        //
        if (isMain)
        {
            out.mainFrame = result;
            MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel, "build mainFrame -");
        }

        MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel, "build out frame param -");
        return OK;
    };

    if (noP2Node)
    {
        buildOutParam(nullptr, NOT_DUMMY, true);
        MY_LOGW("SMVRBatch: no p2 node process, but should use P2StreamNode");
        return OK;
    }

    buildOutParam(featureOut.mainFrame, NOT_DUMMY, true);

    //!!NOTES: no need for sub frames, dummy frames

    return OK;
}

