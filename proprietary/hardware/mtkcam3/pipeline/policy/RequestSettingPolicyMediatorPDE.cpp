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

#define LOG_TAG "mtkcam-RequestSettingPolicyMediatorPDE"

#include <mtkcam3/pipeline/policy/IRequestSettingPolicyMediator.h>
#include <mtkcam3/pipeline/policy/InterfaceTableDef.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>

#include "MyUtils.h"
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;

#define ThisNamespace   RequestSettingPolicyMediator_PDE

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IRequestSettingPolicyMediator Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    evaluateRequest(
                        RequestOutputParams& out,
                        RequestInputParams const& in
                    ) -> int override;
private:
    bool                                            misFdEnabled = false;
    bool                                            misPureRaw = false;
    bool                                            mInit = false;
    bool                                            mbNeedPDE = false;  // for current SensorMode

};


/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<IRequestSettingPolicyMediator>
makeRequestSettingPolicyMediator_PDE(MediatorCreationParams const& params)
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
    CAM_TRACE_CALL();
    NSCam::Utils::CameraProfile profile(LOG_TAG, __FUNCTION__);

    fdintent::RequestOutputParams fdOut;
    fdintent::RequestInputParams fdIn;
    p2nodedecision::RequestOutputParams p2DecisionOut;
    p2nodedecision::RequestInputParams p2DecisionIn;
    featuresetting::RequestOutputParams featureOut;
    featuresetting::RequestInputParams featureIn;
    capturestreamupdater::RequestOutputParams capUpdaterOut;
    capturestreamupdater::RequestInputParams capUpdaterIn;
    android::sp<IImageStreamInfo>              pJpeg_YUV = nullptr;
    android::sp<IImageStreamInfo>              pThumbnail_YUV = nullptr;
    bool noP2Node = !in.pConfiguration_PipelineNodesNeed->needP2CaptureNode && !in.pConfiguration_PipelineNodesNeed->needP2StreamNode;

    // get isType3PDSensorWithoutPDE by SensorMode
    if (!mInit)
    {
        std::shared_ptr<NSCamHW::HwInfoHelper> infohelper = std::make_shared<NSCamHW::HwInfoHelper>(mPipelineStaticInfo->sensorId[0]);
        if( ! infohelper->updateInfos() ) {
            MY_LOGE("cannot properly update infos");
        } else {
            mbNeedPDE = infohelper->isType3PDSensorWithoutPDE((*in.pSensorMode)[0]);
            MY_LOGD("SensorMode %d, isType3PDSensorWithoutPDE %d", (*in.pSensorMode)[0], mbNeedPDE);
        }
        mInit = true;
    }

    // PDE Policy
    bool bNeedPDE = mbNeedPDE;
    if (bNeedPDE)
    {
        if (in.pRequest_AppImageStreamInfo->pAppImage_Jpeg) // Normal/ZSL Capture, VSS
        {
            bNeedPDE = false;
        }
        else if (in.pRequest_AppImageStreamInfo->hasVideoConsumer)  // Video Preview, Video Record
        {
            misPureRaw = !misPureRaw;
            if (!misPureRaw)
            {
                bNeedPDE = false;
            }
        }
        else // Normal Preview, ZSL Preview, PreCapture
        {
            misPureRaw = !misPureRaw;
        }
    }
    MY_LOGD("bNeedPDE %d, misPureRaw %d", bNeedPDE, misPureRaw);

    // (1) is face detection intent?
    if (CC_LIKELY(mPolicyTable->fFaceDetectionIntent != nullptr))
    {
        fdIn.hasFDNodeConfigured = in.pConfiguration_PipelineNodesNeed->needFDNode;
        fdIn.isFdEnabled_LastFrame = misFdEnabled;
        fdIn.pRequest_AppControl = in.pRequest_AppControl;
        fdIn.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
        mPolicyTable->fFaceDetectionIntent(fdOut, fdIn);
        misFdEnabled = fdOut.isFDMetaEn;
    }

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
        p2DecisionIn.hasP2CaptureNode                = in.pConfiguration_PipelineNodesNeed->needP2CaptureNode;
        p2DecisionIn.hasP2StreamNode                 = in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
        p2DecisionIn.isFdEnabled                     = fdOut.isFDMetaEn;
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
        profile.stopWatch();
        mPolicyTable->mFeaturePolicy->evaluateRequest(&featureOut, &featureIn);
        profile.print_overtime('W', 10000000/*10ms*/, "FeatureSettingPolicy::evaluateRequest");

        out.needZslFlow                              = featureOut.needZslFlow;
        out.zslPolicyParams                          = featureOut.zslPolicyParams;
        out.needReconfiguration                      = featureOut.needReconfiguration;
        out.sensorMode                               = featureOut.sensorMode;
        out.reconfigCategory                         = featureOut.reconfigCategory;
        out.vboostControl                            = featureOut.vboostControl;
        //bNeedPDE
        out.zslPolicyParams.mPolicy = (enum eZslPolicy)((int)out.zslPolicyParams.mPolicy | eZslPolicy_PD_ProcessedRaw);
    }


    // (3) build every frames out param
    #define NOT_DUMMY (0)
    #define POST_DUMMY (1)
    #define PRE_DUMMY (2)
    auto buildOutParam = [&] (std::shared_ptr<featuresetting::RequestResultParams> const setting, int dummy, bool isMain) -> int
    {
        MY_LOGD("build out frame param +");
        std::shared_ptr<RequestResultParams> result = std::make_shared<RequestResultParams>();
        // build topology
        topology::RequestOutputParams topologyOut;
        topology::RequestInputParams topologyIn;
        if (CC_LIKELY(mPolicyTable->fTopology != nullptr))
        {
            topologyIn.isDummyFrame = dummy != 0;
            topologyIn.isFdEnabled  = (isMain) ? fdOut.isFdEnabled : false;
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

            if (bNeedPDE)
            {
                topologyOut.pNodesNeed->needPDENode = true;
                topologyOut.pNodeSet->push_back(eNODEID_PDENode);
                topologyOut.pEdges->addEdge(eNODEID_P1Node, eNODEID_PDENode);
            }
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
            if (setting != nullptr)
            {
			    if (bNeedPDE)
                {
                    setting->needP1Dma[0] |= P1_IMGO;
                }
                if (setting->needP1Dma.size() > 0 && topologyOut.pNodesNeed->needRaw16Node)
                {
                    setting->needP1Dma[0] |= P1_IMGO;
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
            iomapIn.isDummyFrame                    = dummy != 0;
            iomapIn.pPipelineStaticInfo             = mPipelineStaticInfo.get();
            // prepare output buffer
            iomapOut.pNodeIOMapImage          = &(result->nodeIOMapImage);
            iomapOut.pNodeIOMapMeta           = &(result->nodeIOMapMeta);
            if (dummy == NOT_DUMMY)
            {
                mPolicyTable->fIOMap_P2Node(iomapOut, iomapIn);
                if (bNeedPDE && iomapOut.bP2UseImgo)
                {
                    bNeedPDE = false;
                }
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

        // for ISP 3.0
        if (bNeedPDE && misPureRaw)
        {
            IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
            entry.push_back(NSIoPipe::NSCamIOPipe::EPipe_PURE_RAW, Type2Type< MINT32 >());
            result->additionalHal[0]->update(entry.tag(), entry);
            MY_LOGD( "set p1 pure raw type");
        }

        // update Metdata
        if (CC_LIKELY(mPolicyTable->pRequestMetadataPolicy != nullptr))
        {
            requestmetadata::EvaluateRequestParams metdataParams;
            metdataParams.requestNo                     = in.requestNo;
            metdataParams.isZSLMode                     = in.isZSLMode;
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

        if (isMain)
        {
            out.mainFrame = result;
            MY_LOGD("build mainFrame -");
        }
        else
        {
            switch (dummy)
            {
                case NOT_DUMMY:
                    out.subFrames.push_back(result);
                    break;
                case POST_DUMMY:
                    out.postDummyFrames.push_back(result);
                    break;
                case PRE_DUMMY:
                    out.preDummyFrames.push_back(result);
                    break;
                default:
                    MY_LOGW("Cannot be here");
                    break;
            }
        }

        MY_LOGD("build out frame param -");
        return OK;
    };
    if (noP2Node)
    {
        buildOutParam(nullptr, NOT_DUMMY, true);
        MY_LOGD("no p2 node process done");
        return OK;
    }

    buildOutParam(featureOut.mainFrame, NOT_DUMMY, true);
    for (auto const& it : featureOut.subFrames)
    {
        buildOutParam(it, NOT_DUMMY, false);
    }
    for (auto const& it : featureOut.postDummyFrames)
    {
        buildOutParam(it, POST_DUMMY, false);
    }
    for (auto const& it : featureOut.preDummyFrames)
    {
        buildOutParam(it, PRE_DUMMY, false);
    }

    return OK;
}

