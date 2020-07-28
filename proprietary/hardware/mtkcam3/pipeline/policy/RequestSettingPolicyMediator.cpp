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

#define LOG_TAG "mtkcam-RequestSettingPolicyMediator"

#include <mtkcam3/pipeline/policy/IRequestSettingPolicyMediator.h>
#include <mtkcam3/pipeline/policy/InterfaceTableDef.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam3/pipeline/stream/StreamId.h>
#include <mtkcam/utils/std/ULog.h>

#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;
using namespace NSCam::v3::Utils;


#define ThisNamespace   RequestSettingPolicyMediator_Default

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

};


/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<IRequestSettingPolicyMediator>
makeRequestSettingPolicyMediator_Default(MediatorCreationParams const& params)
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
    // [Jpeg pack]
    android::sp<IImageStreamInfo>              pJpeg_Sub_YUV = nullptr;
    android::sp<IImageStreamInfo>              pDepth_YUV = nullptr;
    bool                                       isAppRawOutFromP2C = false;

    bool noP2Node = !in.pConfiguration_PipelineNodesNeed->needP2CaptureNode && !in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
    // (1) is face detection intent?
    if (CC_LIKELY(mPolicyTable->fFaceDetectionIntent != nullptr))
    {
        fdIn.hasFDNodeConfigured = in.pConfiguration_PipelineNodesNeed->needFDNode;
        /*if (in.pRequest_ParsedAppMetaControl->control_remosaicEn && mPipelineStaticInfo->is4CellSensor)
        {
            MY_LOGD("is 4cell sensor and ap set remosaic capture...disable FD");
            fdIn.hasFDNodeConfigured = false;
        }*/
        fdIn.isFdEnabled_LastFrame = misFdEnabled;
        fdIn.pRequest_AppImageStreamInfo = in.pRequest_AppImageStreamInfo;
        fdIn.pRequest_AppControl = in.pRequest_AppControl;
        fdIn.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
        mPolicyTable->fFaceDetectionIntent(fdOut, fdIn);
        if (fdOut.hasFDMeta)
        {
            misFdEnabled = fdOut.isFDMetaEn;
        }
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
        // [Jpeg pack] 0 means fine to capture bokeh image.
        {
            if(in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_Sub_YUV != nullptr)
            {
                capUpdaterIn.pConfiguration_HalImage_Jpeg_Sub_YUV  = &(in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_Sub_YUV);
                capUpdaterIn.isSupportJpegPack = true;
            }
            if(in.pConfiguration_StreamInfo_NonP1->pHalImage_Depth_YUV != nullptr)
            {
                capUpdaterIn.pConfiguration_HalImage_Depth_YUV = &(in.pConfiguration_StreamInfo_NonP1->pHalImage_Depth_YUV);
            }
            capUpdaterOut.pHalImage_Jpeg_Sub_YUV         = &pJpeg_Sub_YUV;
            capUpdaterOut.pHalImage_Depth_YUV            = &pDepth_YUV;
        }
        mPolicyTable->fCaptureStreamUpdater(capUpdaterOut, capUpdaterIn);
    }

    // (2.2) P2Node decision: the responsibility of P2StreamNode and P2CaptureNode
    if (CC_LIKELY(mPolicyTable->fP2NodeDecision != nullptr))
    {
        p2DecisionIn.requestNo                       = in.requestNo;
        p2DecisionIn.hasP2CaptureNode                = in.pConfiguration_PipelineNodesNeed->needP2CaptureNode;
        p2DecisionIn.hasP2StreamNode                 = in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
        p2DecisionIn.isFdEnabled                     = fdOut.isFdEnabled && !mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV;
        p2DecisionIn.pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
        p2DecisionIn.pConfiguration_StreamInfo_P1    = &((*(in.pConfiguration_StreamInfo_P1))[0]); // use main1 info
        p2DecisionIn.pRequest_AppControl             = in.pRequest_AppControl;
        p2DecisionIn.pRequest_AppImageStreamInfo     = in.pRequest_AppImageStreamInfo;
        p2DecisionIn.pRequest_ParsedAppMetaControl   = in.pRequest_ParsedAppMetaControl;
        p2DecisionIn.needThumbnail                   = (pThumbnail_YUV != nullptr);
        mPolicyTable->fP2NodeDecision(p2DecisionOut, p2DecisionIn);
    }

    // (2.3) feature setting
    if (CC_LIKELY(mPolicyTable->mFeaturePolicy != nullptr))
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
        featureIn.needRawOutput                      = in.pRequest_AppImageStreamInfo->hasRawOut;
        featureIn.needYuvOutputFromP2C               = p2DecisionOut.needYuvOutputFromP2C;
        for( size_t i = 0; i < mPipelineStaticInfo->sensorId.size(); i++)
        {
            featureIn.sensorMode.push_back((*in.pSensorMode)[i]);
        }
        profile.stopWatch();
        mPolicyTable->mFeaturePolicy->evaluateRequest(&featureOut, &featureIn);
        profile.print_overtime('W', 10000000/*10ms*/, "FeatureSettingPolicy::evaluateRequest");

        out.needZslFlow                              = featureOut.needZslFlow;
        out.zslPolicyParams                          = featureOut.zslPolicyParams;
        out.bCshotRequest                            = featureOut.bCshotRequest;
        out.needReconfiguration                      = featureOut.needReconfiguration;
        out.sensorMode                               = featureOut.sensorMode;
        out.reconfigCategory                         = featureOut.reconfigCategory;
        out.vboostControl                            = featureOut.vboostControl;
        out.keepZslBuffer                            = featureOut.keepZslBuffer;
    }

    //  (2.4) update needReconfiguration for 4cell DNG w/o p2CaptureNode
    // skip sensor mode changing for sw 4cell sensor
    /*if( mPipelineStaticInfo->is4CellSensor && !mPipelineUserConfiguration->pParsedAppConfiguration->isSuperNightMode &&
        !p2DecisionOut.needP2CaptureNode && in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() > 0 )
    {
        out.needReconfiguration = true;
        out.sensorMode.clear();
        out.sensorMode.push_back(SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
        out.reconfigCategory    = ReCfgCtg::NO;
    }*/

    // if zsl is enable and need output raw, the raw need to be output from P2C
    auto checkIfNeedRefineCaptureNodeDecision = [](featuresetting::RequestOutputParams const& featureOut,
                                                   ParsedAppImageStreamInfo const* pRequest_AppImageStreamInfo,
                                                   bool const& isNeedP2ProcessRaw,
                                                   bool const& isP1DirectAppRaw)
                                                   -> bool
    {
        if(pRequest_AppImageStreamInfo && isP1DirectAppRaw)
        {
            return ((featureOut.needZslFlow || isNeedP2ProcessRaw) && pRequest_AppImageStreamInfo->hasRawOut);
        }
        return false;
    };
    //
    if ( isAppRawOutFromP2C = checkIfNeedRefineCaptureNodeDecision( featureOut,in.pRequest_AppImageStreamInfo,
                                                                   (mPipelineStaticInfo->isNeedP2ProcessRaw && in.pRequest_ParsedAppMetaControl->control_processRaw),
                                                                    mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectAppRaw) )
    {
        MY_LOGD("need output raw from p2c");
        if(in.pRequest_AppImageStreamInfo->hasRawOut)
        {
            bool captureNodeAddRawOut = false;
            //priv raw
            if(in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv)
            {
                p2DecisionOut.vImageStreamId_from_CaptureNode.push_back(in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv->getStreamId());
                captureNodeAddRawOut = true;
                MY_LOGD("Add Priv raw (%#" PRIxPTR ") to CaptureNode",in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv->getStreamId());
            }
            //logical raw16
            for(const auto& item:in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16)
            {
                auto iter = in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(item.first);
                if(iter != in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                {
                    p2DecisionOut.vImageStreamId_from_CaptureNode.push_back(iter->second->getStreamId());
                    captureNodeAddRawOut = true;
                    MY_LOGD("Add sensor(%d) raw16 (%#" PRIxPTR ") to CaptureNode",iter->first,iter->second->getStreamId());
                }
            }
            //physical raw16
            for(const auto& item:in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
            {
                auto iter = in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.find(item.first);
                if(iter != in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.end())
                {
                    std::vector<StreamId_T> temp;
                    for( const auto &n : iter->second ) {
                        temp.push_back(n->getStreamId());
                        MY_LOGD("Add sensor(%d) physical raw (%#" PRIxPTR ") to CaptureNode",iter->first,n->getStreamId());
                    }
                    p2DecisionOut.vImageStreamId_from_CaptureNode_Physical.emplace(iter->first, temp);
                    captureNodeAddRawOut = true;

                }
            }
            //metadata & set needP2CaptureNode
            if(captureNodeAddRawOut == true)
            {
                p2DecisionOut.vMetaStreamId_from_CaptureNode.push_back(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId());
                p2DecisionOut.vMetaStreamId_from_CaptureNode.push_back(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId());
                p2DecisionOut.needP2CaptureNode = true;
            }
        }
    }
    //
    if (mPipelineStaticInfo->is4CellSensor
     && out.needReconfiguration
     && out.reconfigCategory != ReCfgCtg::STREAMING)
    {
        // for 4cell reconfig mode for capture, force disable streaming node node
        MY_LOGD("Force disable streaming node for 4cell reconfig");
        p2DecisionOut.needP2StreamNode = false;
        for (int i = 0; i < p2DecisionOut.vImageStreamId_from_StreamNode.size(); i++)
        {
            if (p2DecisionOut.vImageStreamId_from_StreamNode[i] < eSTREAMID_BEGIN_OF_INTERNAL)
            {
                p2DecisionOut.vImageStreamId_from_CaptureNode.push_back(p2DecisionOut.vImageStreamId_from_StreamNode[i]);
            }
        }
        if( p2DecisionOut.vImageStreamId_from_CaptureNode.size()>0 )
        {
            p2DecisionOut.needP2CaptureNode =true;
        }
        p2DecisionOut.vImageStreamId_from_StreamNode.clear();
    }

    // (3) build every frames out param
    #define NOT_DUMMY (0)
    #define POST_DUMMY (1)
    #define PRE_DUMMY (2)
    #define PRE_SUB (3)
    auto buildOutParam = [&] (std::shared_ptr<featuresetting::RequestResultParams> const setting, int dummy, bool isMain) -> int
    {
        std::shared_ptr<RequestResultParams> result = std::make_shared<RequestResultParams>();
        bool isDummyFrame = (dummy == POST_DUMMY || dummy == PRE_DUMMY);
        // build topology
        topology::RequestOutputParams topologyOut;
        topology::RequestInputParams topologyIn;
        if (CC_LIKELY(mPolicyTable->fTopology != nullptr))
        {
            //For 2 Physical Raw
            std::vector<uint32_t>* dmalist = nullptr;
            std::vector<uint32_t>  dmaTemp;
            if(setting == nullptr)
            {
                if(in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size()==0)
                {
                    dmalist = nullptr;
                }
                else
                {
                    dmaTemp.resize(mPipelineStaticInfo->sensorId.size(),0);
                    uint32_t dma = P1_IMGO;
                    dma |= P1_RRZO;
                    for(const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
                    {
                        int const index = mPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                        if (index >= 0)
                        {
                            dmaTemp[index] = (dma);
                        }
                    }
                    dmalist = &(dmaTemp);
                }
            }
            else
            {
                dmalist = &(setting->needP1Dma);
            }
            topologyIn.isDummyFrame = isDummyFrame;
            topologyIn.isMainFrame = isMain;
            topologyIn.isFdEnabled  = (isMain) ? fdOut.isFdEnabled : false;
            topologyIn.useP1FDYUV   = mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV;
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
            topologyIn.pvNeedP1Dma = dmalist;
            topologyIn.isAppRawOutFromP2C= isAppRawOutFromP2C;

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
                if ( setting->needP1Dma.size() > 0
                && ( in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get() ))
                {
                    setting->needP1Dma[0] |= P1_IMGO;
                }
                if (setting->needP1Dma.size() > 0 && mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV
                                                  && isMain && fdOut.isFdEnabled)
                {
                    setting->needP1Dma[0] |= P1_FDYUV;
                }
                if (setting->needP1Dma.size() > 0 && mPipelineStaticInfo->isP1DirectScaledYUV && isMain)
                {
                    setting->needP1Dma[0] |= P1_SCALEDYUV;
                }
                for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16 )
                {
                    int const index = mPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                    MY_LOGD("Raw16 index : %d", index);
                    if ( isMain && index >= 0 && setting->needP1Dma.size() > (size_t)index )
                    {
                        setting->needP1Dma[index] |= P1_IMGO;
                    }
                }

                for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical )
                {
                    int const index = mPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                    MY_LOGD("Raw16 index(physical) : %d", index);
                    if ( isMain && index >= 0 && setting->needP1Dma.size() > index )
                    {
                        setting->needP1Dma[index] |= P1_IMGO;
                    }
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
            iomapIn.pvImageStreamId_from_CaptureNode_Physical = &(p2DecisionOut.vImageStreamId_from_CaptureNode_Physical);
            iomapIn.pvImageStreamId_from_StreamNode_Physical = &(p2DecisionOut.vImageStreamId_from_StreamNode_Physical);
            iomapIn.pvMetaStreamId_from_CaptureNode = &(p2DecisionOut.vMetaStreamId_from_CaptureNode);
            iomapIn.pvMetaStreamId_from_StreamNode = &(p2DecisionOut.vMetaStreamId_from_StreamNode);
            iomapIn.pvMetaStreamId_from_CaptureNode_Physical = &(p2DecisionOut.vMetaStreamId_from_CaptureNode_Physical);
            iomapIn.pvMetaStreamId_from_StreamNode_Physical = &(p2DecisionOut.vMetaStreamId_from_StreamNode_Physical);
            iomapIn.isMainFrame                     = isMain;
            iomapIn.pPipelineStaticInfo              = mPipelineStaticInfo.get();
            iomapIn.isDummyFrame                    = isDummyFrame;
            iomapIn.useP1FDYUV                      = mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV;
            iomapIn.isAppRawOutFromP2C                       = isAppRawOutFromP2C;
            // prepare output buffer
            iomapOut.pNodeIOMapImage          = &(result->nodeIOMapImage);
            iomapOut.pNodeIOMapMeta           = &(result->nodeIOMapMeta);
            iomapOut.pvMetaStreamId_All_Physical = &(result->physicalMetaStreamIds);
            if ( !isDummyFrame )
            {
                mPolicyTable->fIOMap_P2Node(iomapOut, iomapIn);
            }
            mPolicyTable->fIOMap_NonP2Node(iomapOut, iomapIn);
        }
        // Check need to modify P1 IMGO format or not
        {
            bool needToUpdateHalP1StreamInfo =
                        ( featureOut.needUnpackRaw )
                    &&  ( !isDummyFrame )
                    &&  ( in.pRequest_AppImageStreamInfo->pAppImage_Input_RAW16 == nullptr )
                    &&  ( in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.empty() )
                    &&  ( in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.empty() )
                    &&  ( in.pConfiguration_StreamInfo_P1 != nullptr )
                    &&  ( in.pConfiguration_StreamInfo_P1->size() )
                            ;
            if (needToUpdateHalP1StreamInfo)
            {
                MINT32 fmt = eImgFmt_BAYER10_UNPAK;
                sp<IImageStreamInfo> imgoStreamInfo = (*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_Imgo;
                if ( imgoStreamInfo != nullptr ) {
                    IImageStreamInfo::BufPlanes_t bufPlanes;
                    auto infohelper = IHwInfoHelperManager::get()->getHwInfoHelper(mPipelineStaticInfo->sensorId[0]);
                    MY_LOGF_IF(infohelper == nullptr, "getHwInfoHelper");
                    // Get buffer plane by hwHepler
                    bool ret = infohelper->getDefaultBufPlanes_Imgo(bufPlanes, fmt, imgoStreamInfo->getImgSize());
                    MY_LOGF_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Imgo");

                    IImageStreamInfo::BufPlanes_t allocBufPlanes;
                    allocBufPlanes.count = 1;
                    allocBufPlanes.planes[0].sizeInBytes = bufPlanes.planes[0].sizeInBytes;
                    allocBufPlanes.planes[0].rowStrideInBytes = bufPlanes.planes[0].sizeInBytes;
                    auto pStreamInfo =
                        ImageStreamInfoBuilder(imgoStreamInfo.get())
                        .setBufPlanes(bufPlanes)
                        .setImgFormat(fmt)
                        .setAllocBufPlanes(allocBufPlanes)
                        .setAllocImgFormat(eImgFmt_BLOB)
                        .build();
                    MY_LOGI("Format change %s", pStreamInfo->toString().c_str());
                    // Update stream information for per-frame control
                    result->vUpdatedImageStreamInfo[imgoStreamInfo->getStreamId()] = pStreamInfo;
                }
            }
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
            if( pJpeg_Sub_YUV != nullptr)
            {
                result->vUpdatedImageStreamInfo[pJpeg_Sub_YUV->getStreamId()] = pJpeg_Sub_YUV;
            }
            if( pDepth_YUV != nullptr)
            {
                result->vUpdatedImageStreamInfo[pDepth_YUV->getStreamId()] = pDepth_YUV;
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
            metdataParams.isZSLMode                     = in.isZSLMode;
            metdataParams.pRequest_AppImageStreamInfo   = in.pRequest_AppImageStreamInfo;
            metdataParams.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
            metdataParams.pSensorSize                   = in.pSensorSize;
            metdataParams.pAdditionalApp                = result->additionalApp;
            metdataParams.pAdditionalHal                = &result->additionalHal;
            metdataParams.pRequest_AppControl           = in.pRequest_AppControl;
            metdataParams.needReconfigure               = featureOut.needReconfiguration;
            metdataParams.pSensorId                     = &mPipelineStaticInfo->sensorId;
            metdataParams.fixedRRZOSize                 = featureOut.fixedRRZOSize;
            metdataParams.needExif                      = (p2DecisionOut.needP2CaptureNode || in.pRequest_AppImageStreamInfo->hasRawOut);
            for (size_t i = 0; i < in.pConfiguration_StreamInfo_P1->size(); i++)
            {
                if ((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo != nullptr)
                {
                    metdataParams.RrzoSize.push_back((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo->getImgSize());
                }
                if (i == 0 && (*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_FDYuv != nullptr)
                {
                    metdataParams.FDYuvSize = (*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_FDYuv->getImgSize();
                }
            }
            mPolicyTable->pRequestMetadataPolicy->evaluateRequest(metdataParams);
        }

        if (isMain)
        {
            out.mainFrame = result;
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
                case PRE_SUB:
                    out.preSubFrames.push_back(result);
                    break;
                default:
                    MY_LOGW("Cannot be here");
                    break;
            }
        }
        return OK;
    };
    if (noP2Node && !in.pRequest_AppImageStreamInfo->hasRawOut)
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
    for (auto const& it : featureOut.preSubFrames)
    {
        buildOutParam(it, PRE_SUB, false);
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

