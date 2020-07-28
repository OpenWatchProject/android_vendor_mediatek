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

#define LOG_TAG "mtkcam-RequestSettingPolicyMediatorMulticamZoom"

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


#define ThisNamespace   RequestSettingPolicyMediator_Multicam_Zoom

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
#include "MyUtils.h"

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IRequestSettingPolicyMediator Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    evaluateRequest(
                        RequestOutputParams& out,
                        RequestInputParams const& in
                    ) -> int override;
private:
    virtual auto    getSensorIndexBySensorId(
                        uint32_t sensorId,
                        uint32_t &index
                    ) -> bool;
    virtual auto    combineFeatureParam(
                        featuresetting::RequestOutputParams &target,
                        std::unordered_map<uint32_t,
                                    featuresetting::RequestOutputParams> &outList
                    ) -> bool;
    virtual auto    updateSensorModeToOutFeatureParam(
                        featuresetting::RequestOutputParams &out,
                        uint32_t sensorMode,
                        uint32_t sensorIndex
                    ) -> bool;
    virtual auto    updateBoostControlToOutFeatureParam(
                        featuresetting::RequestOutputParams &out,
                        BoostControl boostControl,
                        uint32_t sensorIndex
                    ) -> bool;
    virtual auto    updateRequestResultParams(
                        std::shared_ptr<featuresetting::RequestResultParams> &target,
                        std::shared_ptr<featuresetting::RequestResultParams> &slaveOut,
                        uint32_t sensorIndex
                    ) -> bool;
private:
    bool                                            misFdEnabled = false;

};


/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<IRequestSettingPolicyMediator>
makeRequestSettingPolicyMediator_Multicam_Zoom(MediatorCreationParams const& params)
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
    fdintent::RequestOutputParams fdOut;
    fdintent::RequestInputParams fdIn;
    p2nodedecision::RequestOutputParams p2DecisionOut;
    p2nodedecision::RequestInputParams p2DecisionIn;
    // logical stream feature setting
    featuresetting::RequestOutputParams featureOut;
    featuresetting::RequestInputParams featureIn;
    // physical stream feature setting
    std::unordered_map<uint32_t, featuresetting::RequestOutputParams> vFeatureOut_Physical;
    capturestreamupdater::RequestOutputParams capUpdaterOut;
    capturestreamupdater::RequestInputParams capUpdaterIn;
    requestsensorcontrol::RequestOutputParams reqSensorControlOut;
    requestsensorcontrol::RequestInputParams reqSensorControlIn;
    android::sp<IImageStreamInfo>              pJpeg_YUV = nullptr;
    android::sp<IImageStreamInfo>              pThumbnail_YUV = nullptr;
    // [Jpeg pack]
    android::sp<IImageStreamInfo>              pJpeg_Sub_YUV = nullptr;
    android::sp<IImageStreamInfo>              pDepth_YUV = nullptr;
    //std::vector<uint32_t>                      vMasterSlaveSensorList;
    bool                                       supportFusion = true;
    bool                                       outputOnePhysicalImg = false;
    bool                                       isAppRawOutFromP2C = false;
    if((in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc.size() == 0 &&
        in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() == 0) &&
        in.pRequest_AppImageStreamInfo->outputPhysicalSensors.size() == 1)
    {
        outputOnePhysicalImg = true;
    }
    auto& pParsedAppConfiguration = mPipelineUserConfiguration->pParsedAppConfiguration;
    auto& pParsedMultiCamInfo = pParsedAppConfiguration->pParsedMultiCamInfo;

    bool noP2Node = !in.pConfiguration_PipelineNodesNeed->needP2CaptureNode && !in.pConfiguration_PipelineNodesNeed->needP2StreamNode;
    // (1) is face detection intent?
    if (CC_LIKELY(mPolicyTable->fFaceDetectionIntent != nullptr))
    {
        fdIn.hasFDNodeConfigured = in.pConfiguration_PipelineNodesNeed->needFDNode;
        if (in.pRequest_ParsedAppMetaControl->control_remosaicEn && mPipelineStaticInfo->is4CellSensor)
        {
            MY_LOGD("is 4cell sensor and ap set remosaic capture...disable FD");
            fdIn.hasFDNodeConfigured = false;
        }
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
        p2DecisionIn.isFdEnabled                     = fdOut.isFDMetaEn && !mPipelineStaticInfo->isP1DirectFDYUV;
        p2DecisionIn.pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
        p2DecisionIn.pConfiguration_StreamInfo_P1    = &((*(in.pConfiguration_StreamInfo_P1))[0]); // use main1 info
        p2DecisionIn.pRequest_AppControl             = in.pRequest_AppControl;
        p2DecisionIn.pRequest_AppImageStreamInfo     = in.pRequest_AppImageStreamInfo;
        p2DecisionIn.pRequest_ParsedAppMetaControl   = in.pRequest_ParsedAppMetaControl;
        p2DecisionIn.needThumbnail                   = (pThumbnail_YUV != nullptr);
        mPolicyTable->fP2NodeDecision(p2DecisionOut, p2DecisionIn);
    }

    // (2.3) check sensor control policy
    if (CC_LIKELY(mPolicyTable->pRequestSensorControlPolicy != nullptr))
    {
        reqSensorControlIn.requestNo           = in.requestNo;
        reqSensorControlIn.pRequest_AppControl = in.pRequest_AppControl;
        reqSensorControlIn.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
        reqSensorControlIn.pRequest_SensorMode = in.pSensorMode;
        reqSensorControlIn.needP2CaptureNode = p2DecisionOut.needP2CaptureNode;
        reqSensorControlIn.needP2StreamNode = p2DecisionOut.needP2StreamNode;
        reqSensorControlIn.needFusion = supportFusion;
        // if contains capture yuv or needs output logical Raw, set flag to true.
        reqSensorControlIn.bLogicalCaptureOutput = (p2DecisionOut.vImageStreamId_from_CaptureNode.size()>0)?true:false;
        reqSensorControlIn.bLogicalRawOutput = (in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size()>0)?true:false;
        reqSensorControlOut.vMetaStreamId_from_CaptureNode_Physical= &p2DecisionOut.vMetaStreamId_from_CaptureNode_Physical;
        reqSensorControlOut.vImageStreamId_from_CaptureNode_Physical= &p2DecisionOut.vImageStreamId_from_CaptureNode_Physical;
        reqSensorControlOut.vMetaStreamId_from_StreamNode_Physical= &p2DecisionOut.vMetaStreamId_from_StreamNode_Physical;
        reqSensorControlOut.vImageStreamId_from_StreamNode_Physical= &p2DecisionOut.vImageStreamId_from_StreamNode_Physical;
        //reqSensorControlOut.pCaptureSensorList = &pCaptureSensorList;
        //reqSensorControlOut.pMasterSlaveSensorList = &vMasterSlaveSensorList;
        reqSensorControlOut.pMultiCamReqOutputParams = &out.multicamReqOutputParams;
        mPolicyTable->pRequestSensorControlPolicy->evaluateRequest(reqSensorControlOut, reqSensorControlIn);
    }
    {
        // Handling two sensor streaming, but only one physical image will output case.
        if(outputOnePhysicalImg)
        {
            auto targetId = in.pRequest_AppImageStreamInfo->outputPhysicalSensors[0];
            auto updateList = [&targetId](std::vector<uint32_t>& streamingList)
            {
                for(auto iter = streamingList.begin(); iter != streamingList.end();)
                {
                    if(targetId != *iter)
                    {
                        iter = streamingList.erase(iter);
                    }
                    else
                    {
                        iter++;
                    }
                }
            };
            // filtering streamingSensorList and prvStreamingSensorList to 1 element.
            updateList(out.multicamReqOutputParams.streamingSensorList);
            updateList(out.multicamReqOutputParams.prvStreamingSensorList);
        }
    }

    // (2.4) feature setting
    // set feature in data first. (sensor irrelevant)
    {
        featureIn.requestNo                          = in.requestNo;
        featureIn.Configuration_HasRecording         = mPipelineUserConfiguration->pParsedAppImageStreamInfo->hasVideoConsumer;
        featureIn.maxP2CaptureSize                   = p2DecisionOut.maxP2CaptureSize;
        featureIn.maxP2StreamSize                    = p2DecisionOut.maxP2StreamSize;
        featureIn.needP2CaptureNode                  = p2DecisionOut.needP2CaptureNode;
        featureIn.needP2StreamNode                   = p2DecisionOut.needP2StreamNode;
        featureIn.pRequest_AppControl                = in.pRequest_AppControl;
        featureIn.pRequest_AppImageStreamInfo        = in.pRequest_AppImageStreamInfo;
        featureIn.pRequest_ParsedAppMetaControl      = in.pRequest_ParsedAppMetaControl;
        featureIn.pMultiCamReqOutputParams           = &out.multicamReqOutputParams;
        featureIn.needRawOutput                      = in.pRequest_AppImageStreamInfo->hasRawOut;
        featureIn.needYuvOutputFromP2C               = p2DecisionOut.needYuvOutputFromP2C;
        for( size_t i = 0; i < mPipelineStaticInfo->sensorId.size(); i++)
        {
            featureIn.sensorMode.push_back((*in.pSensorMode)[i]);
        }
        featureIn.pConfiguration_StreamInfo_P1 = in.pConfiguration_StreamInfo_P1;
    }
    auto PhysicalFeatureSettingPolicyUpdate = [this, &featureIn, &in, &vFeatureOut_Physical](
                                                std::vector<uint32_t>& sensorList){
        // update for physical stream feature setting
        for(size_t i=0;i<sensorList.size();++i)
        {
            uint32_t index = 0;
            auto id = sensorList[i];
            MY_LOGD("[%" PRId32 "] physical update [%" PRId32 "]", id, in.requestNo);
            if(getSensorIndexBySensorId(id, index))
            {
                auto p = mPolicyTable->mFeaturePolicy_Physical.find(id);
                if (p != mPolicyTable->mFeaturePolicy_Physical.end() && CC_LIKELY(p->second != nullptr))
                {
                    // pConfiguration_StreamInfo_P1 needs set it own self data.
                    //std::vector<ParsedStreamInfo_P1> configStreamInfo_P1 = {(*(in.pConfiguration_StreamInfo_P1))[index]};
                    //featureIn.pConfiguration_StreamInfo_P1 = &configStreamInfo_P1;
                    featuresetting::RequestOutputParams slaveOutParam;
                    p->second->evaluateRequest(&slaveOutParam, &featureIn);
                    vFeatureOut_Physical.emplace(id, slaveOutParam);
                }
            }
        }
    };
    bool isCaptureNoFusion = p2DecisionOut.needP2CaptureNode && !supportFusion;
    // means contain logical stream output.
    //MY_LOGD("vImageStreamId_from_StreamNode(%zu), vImageStreamId_from_CaptureNode(%zu) isCapnoF(%d)",
    //                p2DecisionOut.vImageStreamId_from_StreamNode.size(),
    //                p2DecisionOut.vImageStreamId_from_CaptureNode.size(),
    //                isCaptureNoFusion);

    bool needLogicalFeatureSettingPolicy =
                (p2DecisionOut.vImageStreamId_from_StreamNode.size() > 0) ||
                (p2DecisionOut.vImageStreamId_from_CaptureNode.size() > 0)||
                (in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() > 0) ||
                (in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get());
    bool needPhysicalFeatureSettingPolicy =
                (p2DecisionOut.vImageStreamId_from_StreamNode_Physical.size() > 0) ||
                (p2DecisionOut.vImageStreamId_from_CaptureNode_Physical.size() > 0)||
                (in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() > 0);
    if(needLogicalFeatureSettingPolicy && isCaptureNoFusion)
    {
        std::vector<uint32_t> sensorList;
        sensorList.push_back(out.multicamReqOutputParams.prvStreamingSensorList[0]);
        PhysicalFeatureSettingPolicyUpdate(sensorList);
    }
    else if(needLogicalFeatureSettingPolicy && !isCaptureNoFusion)
    {
        // call logical feature setting policy.
        if (CC_LIKELY(mPolicyTable->mFeaturePolicy != nullptr))
        {
            MY_LOGD("logical update [%" PRId32 "]", in.requestNo);
            mPolicyTable->mFeaturePolicy->evaluateRequest(&featureOut, &featureIn);
        }
    }

    if(needPhysicalFeatureSettingPolicy)
    {
        MY_LOGD("physical update");
        // check need update id.
        std::vector<uint32_t> sensorList;
        for(auto&& item:p2DecisionOut.vImageStreamId_from_StreamNode_Physical)
        {
            sensorList.push_back(item.first);
        }
        for(auto&& item:p2DecisionOut.vImageStreamId_from_CaptureNode_Physical)
        {
            if(find(sensorList.begin(), sensorList.end(), item.first) == sensorList.end())
            {
                sensorList.push_back(item.first);
            }
        }
        for(auto&& item:in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
        {
            if(find(sensorList.begin(), sensorList.end(), item.first) == sensorList.end())
            {
                sensorList.push_back(item.first);
            }
        }
        PhysicalFeatureSettingPolicyUpdate(sensorList);
    }
    // combine master & slave result
    combineFeatureParam(featureOut, vFeatureOut_Physical);
    out.needZslFlow                              = featureOut.needZslFlow;
    out.zslPolicyParams                          = featureOut.zslPolicyParams;
    out.bCshotRequest                            = featureOut.bCshotRequest;
    out.needReconfiguration                      = featureOut.needReconfiguration;
    out.sensorMode                               = featureOut.sensorMode;
    out.reconfigCategory                         = featureOut.reconfigCategory;
    out.vboostControl                            = featureOut.vboostControl;
    out.keepZslBuffer                            = featureOut.keepZslBuffer;
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
            auto checkIsSensorReady = [](uint32_t targetId,
                                          std::vector<uint32_t> const& vStreamingSensorIdList)
            {
                if(std::find(vStreamingSensorIdList.begin(),
                             vStreamingSensorIdList.end(),
                             targetId) == vStreamingSensorIdList.end())
                {
                    return true;
                }
                return false;
            };
            //
            bool captureNodeAddRawOut = false;
            //priv raw
            if(in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv)
            {
                p2DecisionOut.vImageStreamId_from_CaptureNode.push_back(in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv->getStreamId());
                captureNodeAddRawOut = true;
                MY_LOGD("Add Priv raw (%" PRIx64 ") to CaptureNode",in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv->getStreamId());
            }
            //logical raw16
            for(const auto& item:in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16)
            {
                if(checkIsSensorReady(item.first,out.multicamReqOutputParams.prvStreamingSensorList))
                {
                    auto iter = in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(item.first);
                    if(iter != in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                    {
                        p2DecisionOut.vImageStreamId_from_CaptureNode.push_back(iter->second->getStreamId());
                        captureNodeAddRawOut = true;
                        MY_LOGD("Add sensor(%d) raw16 (%" PRIx64 ") to CaptureNode",iter->first,iter->second->getStreamId());
                    }
                }
            }
            //physical raw16
            for(const auto& item:in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
            {
                if(checkIsSensorReady(item.first,out.multicamReqOutputParams.prvStreamingSensorList))
                {
                    auto iter = in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.find(item.first);
                    if(iter != in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.end())
                    {
                        std::vector<StreamId_T> temp;
                        for( const auto &n : iter->second ) {
                            temp.push_back(n->getStreamId());
                            MY_LOGD("Add sensor(%d) physical raw (%" PRIx64 ") to CaptureNode",iter->first,n->getStreamId());
                        }
                        p2DecisionOut.vImageStreamId_from_CaptureNode_Physical.emplace(iter->first, temp);
                        captureNodeAddRawOut = true;
                    }
                }
            }
            //metadata && set needP2CaptureNode
            if(captureNodeAddRawOut == true)
            {
                p2DecisionOut.vMetaStreamId_from_CaptureNode.push_back(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId());
                p2DecisionOut.vMetaStreamId_from_CaptureNode.push_back(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId());
                p2DecisionOut.needP2CaptureNode = true;
            }
        }
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
                        dmaTemp[index] = (dma);
                    }
                    if(out.multicamReqOutputParams.prvStreamingSensorList.size() > 0)
                    {
                        for(const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16)
                        {
                            int const index = mPipelineStaticInfo->getIndexFromSensorId(
                                                        out.multicamReqOutputParams.prvStreamingSensorList[0]);
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
            topologyIn.isDummyFrame = dummy != 0;
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
            topologyIn.pMultiCamReqOutputParams = &out.multicamReqOutputParams;
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
                if ( setting->needP1Dma.size() > 0
                && ( in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get() ))
                {
                    setting->needP1Dma[0] |= P1_IMGO;
                }
                if (setting->needP1Dma.size() > 0 && mPipelineStaticInfo->isP1DirectFDYUV && isMain && fdOut.isFdEnabled)
                {
                    setting->needP1Dma[0] |= P1_FDYUV;
                    // david add for main2 only case
                    setting->needP1Dma[1] |= P1_FDYUV;
                }
                if (setting->needP1Dma.size() > 0 && mPipelineStaticInfo->isP1DirectScaledYUV && isMain)
                {
                    setting->needP1Dma[0] |= P1_SCALEDYUV;
                }
                for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16 )
                {
                    if(out.multicamReqOutputParams.prvStreamingSensorList.size() > 0)
                    {
                        int const index = mPipelineStaticInfo->getIndexFromSensorId(
                                                out.multicamReqOutputParams.prvStreamingSensorList[0]);
                        MY_LOGD("Raw16 index : %d", index);
                        if ( isMain && (int)setting->needP1Dma.size() > index )
                        {
                            setting->needP1Dma[index] |= P1_IMGO;
                        }
                    }
                    else
                    {
                        MY_LOGE("capture raw fail");
                        return -EINVAL;
                    }
                }

                for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical )
                {
                    int const index = mPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                    MY_LOGD("Raw16 index(physical) : %d", index);
                    if ( isMain && setting->needP1Dma.size() > index )
                    {
                        setting->needP1Dma[index] |= P1_IMGO;
                    }
                }

                iomapIn.pRequest_NeedP1Dma              = &(setting->needP1Dma);
            }
            else
            {
                for (int i = 0; i < (int)topologyOut.pNodesNeed->needP1Node.size(); i++)
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
            iomapIn.isDummyFrame                    = dummy != 0;
            iomapIn.useP1FDYUV                      = mPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV;
            iomapIn.isAppRawOutFromP2C                       = isAppRawOutFromP2C;
            iomapIn.pMultiCamReqOutputParams = &out.multicamReqOutputParams;
            // prepare output buffer
            iomapOut.pNodeIOMapImage          = &(result->nodeIOMapImage);
            iomapOut.pNodeIOMapMeta           = &(result->nodeIOMapMeta);
            iomapOut.pvMetaStreamId_All_Physical = &(result->physicalMetaStreamIds);
            iomapOut.pMultiCamReqOutputParams = &out.multicamReqOutputParams;
            if (dummy == NOT_DUMMY)
            {
                mPolicyTable->fIOMap_P2Node(iomapOut, iomapIn);
            }
            mPolicyTable->fIOMap_NonP2Node(iomapOut, iomapIn);
        }
        // Check need to modify P1 IMGO format or not
        {
            bool needToUpdateHalP1StreamInfo =
                        ( featureOut.needUnpackRaw )
                    &&  ( dummy == NOT_DUMMY )
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

                    auto pStreamInfo =
                        ImageStreamInfoBuilder(imgoStreamInfo.get())
                        .setBufPlanes(bufPlanes)
                        .setImgFormat(fmt)
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
            for (int i = 0; i < (int)topologyOut.pNodesNeed->needP1Node.size(); i++)
            {
                result->additionalHal.push_back(std::make_shared<IMetadata>());
            }
        }

        // update Metdata
        if (CC_LIKELY(mPolicyTable->pRequestMetadataPolicy != nullptr))
        {
            std::vector<MSize> sensorSize;
            std::vector<int32_t> sensorId;
            requestmetadata::EvaluateRequestParams metdataParams;
            std::vector<AdditionalRawInfo> additionalRawInfoList;
            metdataParams.pSensorSize               = in.pSensorSize;
            metdataParams.pSensorId                 = &mPipelineStaticInfo->sensorId;
            metdataParams.requestNo                     = in.requestNo;
            metdataParams.isZSLMode                     = in.isZSLMode;
            metdataParams.pRequest_AppImageStreamInfo   = in.pRequest_AppImageStreamInfo;
            metdataParams.pRequest_ParsedAppMetaControl = in.pRequest_ParsedAppMetaControl;
            metdataParams.pAdditionalApp                = result->additionalApp;
            metdataParams.pAdditionalHal                = &result->additionalHal;
            metdataParams.pRequest_AppControl           = in.pRequest_AppControl;
            metdataParams.needReconfigure               = featureOut.needReconfiguration;
            metdataParams.fixedRRZOSize                 = featureOut.fixedRRZOSize;
            metdataParams.needExif                      = (p2DecisionOut.needP2CaptureNode || in.pRequest_AppImageStreamInfo->hasRawOut);
            for (size_t i = 0; i < in.pConfiguration_StreamInfo_P1->size(); i++)
            {
                if ((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo != nullptr)
                {
                    metdataParams.RrzoSize.push_back((*(in.pConfiguration_StreamInfo_P1))[i].pHalImage_P1_Rrzo->getImgSize());
                }
            }
            auto getConfigImgoSize = [&in](MINT32 index)
            {
                return (*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Imgo->getImgSize();
            };
            auto getConfigImgoStride = [&in](MINT32 index)
            {
                return (*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Imgo->getBufPlanes().planes[0].rowStrideInBytes;
            };
            // logical
            for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16 )
            {
                auto sensorListSize = mPipelineStaticInfo->sensorId.size();
                if(sensorListSize > 1 &&
                   out.multicamReqOutputParams.prvStreamingSensorList.size()>0)
                {
                    // multicam device: check prvStreamingSensorList data
                    int const index = mPipelineStaticInfo->getIndexFromSensorId(
                                                out.multicamReqOutputParams.prvStreamingSensorList[0]);
                    // set capacity
                    additionalRawInfoList.resize(sensorListSize, AdditionalRawInfo());
                    additionalRawInfoList[index].imgSize = getConfigImgoSize(index);
                    additionalRawInfoList[index].format = raw16.second->getImgFormat();

                    // Use config imgo stride if have packed raw tag
                    IMetadata::IEntry packedEntry = (metdataParams.pRequest_AppControl)->entryFor(MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE);
                    if( !packedEntry.isEmpty() && packedEntry.itemAt(0, Type2Type<MINT32>()) > 0){
                        additionalRawInfoList[index].stride = getConfigImgoStride(index);
                    }
                    else {
                        additionalRawInfoList[index].stride = raw16.second->getBufPlanes().planes[0].rowStrideInBytes;
                    }
                }
            }
            // physical
            for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical )
            {
                auto sensorListSize = mPipelineStaticInfo->sensorId.size();
                // multicam device: check prvStreamingSensorList data
                int const index = mPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                if(index < 0)
                {
                    MY_LOGE("cannot map sensor id(%" PRIu32 ")", raw16.first);
                    continue;
                }
                // set capacity
                additionalRawInfoList.resize(sensorListSize, AdditionalRawInfo());
                additionalRawInfoList[index].imgSize = getConfigImgoSize(index);
                additionalRawInfoList[index].format = raw16.second[0]->getImgFormat();
                additionalRawInfoList[index].stride = raw16.second[0]->getAllocBufPlanes().planes[0].rowStrideInBytes;
            }
            metdataParams.additionalRawInfo = &additionalRawInfoList;
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


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getSensorIndexBySensorId(
    uint32_t sensorId,
    uint32_t &index
) -> bool
{
    bool ret = false;
    for(size_t i=0;i<mPipelineStaticInfo->sensorId.size();i++)
    {
        if(sensorId == (uint32_t)mPipelineStaticInfo->sensorId[i])
        {
            index = i;
            ret = true;
            break;
        }
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
combineFeatureParam(
    featuresetting::RequestOutputParams &target,
    std::unordered_map<uint32_t,
                featuresetting::RequestOutputParams> &outList
) -> bool
{
    // append slave sensor mode to master
    for(auto&& outParam:outList) {
        // if dma > 1 means needs output p1 data.
        if(outParam.second.mainFrame->needP1Dma.size() > 0) {
            uint32_t sensorIndex;
            if(getSensorIndexBySensorId(outParam.first, sensorIndex)) {
                for(auto&& item:outParam.second.sensorMode) {
                    updateSensorModeToOutFeatureParam(
                                            target,
                                            item,
                                            sensorIndex);
                }
                for(auto&& item:outParam.second.vboostControl) {
                    updateBoostControlToOutFeatureParam(
                                            target,
                                            item,
                                            sensorIndex);
                }
                // main frame append/override
                updateRequestResultParams(target.mainFrame, outParam.second.mainFrame, sensorIndex);
                // subFrame
                for(auto&& subFrame:outParam.second.subFrames)
                {
                    MY_LOGD("build sub frame id(%d)", outParam.first);
                    if(subFrame == nullptr){
                        target.subFrames.push_back(nullptr);
                    }
                    else {
                        auto targetSubFrame = std::make_shared<featuresetting::RequestResultParams>();
                        updateRequestResultParams(targetSubFrame, subFrame, sensorIndex);
                        target.subFrames.push_back(targetSubFrame);
                    }
                }
                // preDummyFrames
                for(auto&& preDummyFrames:outParam.second.preDummyFrames)
                {
                    MY_LOGD("build preDummyFrames id(%d)", outParam.first);
                    if(preDummyFrames == nullptr){
                        target.preDummyFrames.push_back(nullptr);
                    }
                    else {
                        auto targetPreDummyFrame = std::make_shared<featuresetting::RequestResultParams>();
                        updateRequestResultParams(targetPreDummyFrame, preDummyFrames, sensorIndex);
                        target.preDummyFrames.push_back(targetPreDummyFrame);
                    }
                }
                // postDummyFrames
                for(auto&& postDummyFrames:outParam.second.postDummyFrames)
                {
                    MY_LOGD("build postDummyFrames id(%d)", outParam.first);
                    if(postDummyFrames == nullptr){
                        target.postDummyFrames.push_back(nullptr);
                    }
                    else {
                        auto targetpostDummyFrames = std::make_shared<featuresetting::RequestResultParams>();
                        updateRequestResultParams(targetpostDummyFrames, postDummyFrames, sensorIndex);
                        target.postDummyFrames.push_back(targetpostDummyFrames);
                    }
                }
                target.needZslFlow = (outParam.second.needZslFlow || target.needZslFlow);
                target.needReconfiguration = (outParam.second.needReconfiguration || target.needReconfiguration);
                target.zslPolicyParams = outParam.second.zslPolicyParams;
                target.reconfigCategory = outParam.second.reconfigCategory;
            }
        }
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateSensorModeToOutFeatureParam(
    featuresetting::RequestOutputParams &target,
    uint32_t sensorMode,
    uint32_t sensorIndex
) -> bool
{
    auto size=mPipelineStaticInfo->sensorId.size();
    if(size != target.sensorMode.size())
    {
        target.sensorMode.resize(size, 0);
    }
    target.sensorMode[sensorIndex] = sensorMode;
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateBoostControlToOutFeatureParam(
    featuresetting::RequestOutputParams &target,
    BoostControl boostControl,
    uint32_t sensorIndex
) -> bool
{
    auto size=mPipelineStaticInfo->sensorId.size();
    if(size != target.vboostControl.size())
    {
        target.vboostControl.resize(size, BoostControl());
    }
    target.vboostControl[sensorIndex] = boostControl;
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateRequestResultParams(
    std::shared_ptr<featuresetting::RequestResultParams> &target,
    std::shared_ptr<featuresetting::RequestResultParams> &physicalOut,
    uint32_t sensorIndex
) -> bool
{
    if(physicalOut == nullptr) {
        MY_LOGE("target(%p) slave out(%p)", target.get(), physicalOut.get());
        return false;
    }
    if(target == nullptr) {
        target = std::make_shared<featuresetting::RequestResultParams>();
        target->needKeepP1BuffersForAppReprocess =
                physicalOut->needKeepP1BuffersForAppReprocess;
        target->additionalApp = physicalOut->additionalApp;
    }
    // check slave size
    if(physicalOut->needP1Dma.size() > 1 &&
      (physicalOut->needP1Dma.size() != physicalOut->additionalHal.size())) {
        MY_LOGE("dma(%zu) hal size(%zu), need check flow",
                            physicalOut->needP1Dma.size(),
                            physicalOut->additionalHal.size());
        return false;
    }
    auto size=mPipelineStaticInfo->sensorId.size();
    // dma
    if(size != target->needP1Dma.size()) {
        target->needP1Dma.resize(size, 0);
        target->needP1Dma[sensorIndex] = physicalOut->needP1Dma[sensorIndex];
    }
    if(target->needP1Dma[sensorIndex] == 0) {
        target->needP1Dma[sensorIndex] = physicalOut->needP1Dma[sensorIndex];
    }
    else {
        target->needP1Dma[sensorIndex] |= physicalOut->needP1Dma[sensorIndex];
    }
    // hal
    if(size != target->additionalHal.size()) {
        target->additionalHal.resize(size, nullptr);
    }
    if(target->additionalHal[sensorIndex] == nullptr) {
        target->additionalHal[sensorIndex] = physicalOut->additionalHal[sensorIndex];
    }
    else {
        if(target->additionalHal[sensorIndex] != nullptr)
            *(target->additionalHal[sensorIndex]) += *(physicalOut->additionalHal[sensorIndex]);
    }
    return true;
}