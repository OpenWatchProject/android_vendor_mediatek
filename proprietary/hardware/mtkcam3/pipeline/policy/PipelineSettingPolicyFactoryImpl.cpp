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

#define LOG_TAG "mtkcam-PipelineSettingPolicyFactory"

#include <mtkcam3/pipeline/policy/IPipelineSettingPolicy.h>
//
#include <mtkcam3/pipeline/policy/IConfigPipelineNodesNeedPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigPipelineTopologyPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigSensorSettingPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigP1HwSettingPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigP1DmaNeedPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigStreamInfoPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigAppImageStreamInfoMaxBufNumPolicy.h>
//
#include <mtkcam3/pipeline/policy/IFaceDetectionIntentPolicy.h>
#include <mtkcam3/pipeline/policy/IP2NodeDecisionPolicy.h>
#include <mtkcam3/pipeline/policy/ITopologyPolicy.h>
#include <mtkcam3/pipeline/policy/ICaptureStreamUpdaterPolicy.h>
#include <mtkcam3/pipeline/policy/IIOMapPolicy.h>
#include <mtkcam3/pipeline/policy/IRequestMetadataPolicy.h>
#include <mtkcam3/pipeline/policy/IRequestSensorControlPolicy.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "PipelineSettingPolicyImpl.h"
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;


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
#define MAKE_PIPELINE_POLICY(_class_, ...) \
    std::make_shared<_class_>( \
        PipelineSettingPolicyImpl::CreationParams{ \
            .pPipelineStaticInfo        = params.pPipelineStaticInfo, \
            .pPipelineUserConfiguration = params.pPipelineUserConfiguration, \
            .pPolicyTable               = pPolicyTable, \
            .pMediatorTable             = pMediatorTable, \
        })


#define _POLICY_( _f_, _inst_ ) \
    pPolicyTable->_f_ = _inst_

#define _POLICY_IF_EMPTY_( _f_, _inst_ ) \
    if ( pPolicyTable->_f_ == nullptr ) { \
        _POLICY_( _f_, _inst_ ); \
    }

#define _MEDIATOR_( _f_, _maker_ ) \
    pMediatorTable->_f_ = \
        ( _maker_ (MediatorCreationParams{ \
                .pPipelineStaticInfo        = params.pPipelineStaticInfo, \
                .pPipelineUserConfiguration = params.pPipelineUserConfiguration, \
                .pPolicyTable               = pPolicyTable, \
                .pAppStreamInfoBuilderFactory = params.pAppStreamInfoBuilderFactory, \
        }))

#define _MEDIATOR_IF_EMPTY_( _f_, _maker_ ) \
    if ( pMediatorTable->_f_ == nullptr ) { \
        _MEDIATOR_( _f_, _maker_ ); \
    }

#define _FEATUREPOLICY_( _module_, _creator_) \
    pPolicyTable->_module_ = \
        ( _creator_( featuresetting::CreationParams { \
                        .pPipelineStaticInfo = params.pPipelineStaticInfo, \
                        .pPipelineUserConfiguration = params.pPipelineUserConfiguration, \
        }))

#define _FEATUREPOLICY_IF_EMPTY_( _module_, _creator_) \
    if ( pPolicyTable->_module_ == nullptr ) { \
        _FEATUREPOLICY_( _module_, _creator_); \
    }

#define _METADATAPOLICY_( _module_, _creator_) \
    pPolicyTable->_module_ = \
        ( _creator_( requestmetadata::CreationParams { \
                        .pPipelineStaticInfo = params.pPipelineStaticInfo, \
                        .pPipelineUserConfiguration = params.pPipelineUserConfiguration, \
                        .pRequestMetadataPolicy     = pPolicyTable->_module_, \
        }))

#define _METADATAPOLICY_IF_EMPTY_( _module_, _creator_) \
    if ( pPolicyTable->_module_ == nullptr ) { \
        _METADATAPOLICY_( _module_, _creator_); \
    }
/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<IConfigSettingPolicyMediator> makeConfigSettingPolicyMediator_Default(MediatorCreationParams const&);
std::shared_ptr<IRequestSettingPolicyMediator> makeRequestSettingPolicyMediator_Default(MediatorCreationParams const&);
std::shared_ptr<IRequestSettingPolicyMediator> makeRequestSettingPolicyMediator_PDE(MediatorCreationParams const&);
std::shared_ptr<IRequestSettingPolicyMediator> makeRequestSettingPolicyMediator_SMVRBatch(MediatorCreationParams const&);
std::shared_ptr<IRequestSettingPolicyMediator> makeRequestSettingPolicyMediator_Multicam_Zoom(MediatorCreationParams const&);

/******************************************************************************
 *
 ******************************************************************************/
static
auto
decidePolicyAndMake(
    IPipelineSettingPolicyFactory::CreationParams const& params __unused,
    std::shared_ptr<PolicyTable> pPolicyTable __unused,
    std::shared_ptr<MediatorTable> pMediatorTable __unused
) -> std::shared_ptr<IPipelineSettingPolicy>
{
    auto const& pPipelineStaticInfo         = params.pPipelineStaticInfo;
    auto const& pPipelineUserConfiguration  = params.pPipelineUserConfiguration;
    auto const& pParsedAppConfiguration     = pPipelineUserConfiguration->pParsedAppConfiguration;
    auto const& pParsedAppImageStreamInfo   = pPipelineUserConfiguration->pParsedAppImageStreamInfo;
    auto const& pParsedMultiCamInfo          = pParsedAppConfiguration->pParsedMultiCamInfo;
    auto const& pParsedSMVRBatchInfo          = pParsedAppConfiguration->pParsedSMVRBatchInfo;

    ////////////////////////////////////////////////////////////////////////////
    //  Security Camera
    ////////////////////////////////////////////////////////////////////////////
    if  (pParsedAppImageStreamInfo->secureInfo.type != SecType::mem_normal) {
        MY_LOGD("Security policy used");
        _POLICY_( fConfigPipelineNodesNeed, makePolicy_Configuration_PipelineNodesNeed_Security() );
        _POLICY_( fConfigSensorSetting, makePolicy_Configuration_SensorSetting_Security() );
        _POLICY_( fConfigP1DmaNeed, makePolicy_Configuration_P1DmaNeed_Security() );
        goto lbDefault;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  DualCam [VSDOF]
    ////////////////////////////////////////////////////////////////////////////
    if(pPipelineStaticInfo->isDualDevice)
    {
        if(pParsedMultiCamInfo->mDualDevicePath == DualDevicePath::MultiCamControl)
        {
            if (pParsedAppConfiguration->isSuperNightMode)
            {
                CAM_ULOGMI("[%s] Super Night Mode", __FUNCTION__);
                _POLICY_( fP2NodeDecision,          makePolicy_P2NodeDecision_AppRaw16Reprocess(
                                                    makePolicy_P2NodeDecision_Default()) );
            }
            _POLICY_( fConfigPipelineNodesNeed,     makePolicy_Configuration_PipelineNodesNeed_multicam() );
            _POLICY_( fConfigSensorSetting,         makePolicy_Configuration_SensorSetting_Vsdof() );
            _POLICY_( fConfigP1HwSetting,           makePolicy_Configuration_P1HwSetting_multicam() );
            _POLICY_( fConfigP1DmaNeed,             makePolicy_Configuration_P1DmaNeed_MultiCam() );
            _POLICY_( fIOMap_P2Node,                makePolicy_IOMap_P2Node_multicam() );
            _POLICY_( fTopology,                    makePolicy_Topology_Multicam() );
            _POLICY_( pRequestSensorControlPolicy,  makePolicy_RequestSensorControl_Multicam_Zoom(
                                                    requestsensorcontrol::CreationParams{
                                                        .pPipelineStaticInfo = params.pPipelineStaticInfo,
                                                        .pPipelineUserConfiguration = params.pPipelineUserConfiguration,
                                                    }) );
            // for multicam zoom case, it has to create master and slave feature setting policy by themselves.
            if(pPolicyTable->mFeaturePolicy == nullptr)
            {
                auto featureParams = featuresetting::CreationParams {
                    .pPipelineStaticInfo = params.pPipelineStaticInfo,
                    .pPipelineUserConfiguration = params.pPipelineUserConfiguration,
                    .bIsLogicalCam = true,
                };
                pPolicyTable->mFeaturePolicy =
                        featuresetting::createFeatureSettingPolicyInstance(featureParams);
            }
            for(size_t i = 0;i<params.pPipelineStaticInfo->sensorId.size();i++)
            {
                auto physicalPipelineStaticInfo = std::make_shared<PipelineStaticInfo>();
                if(physicalPipelineStaticInfo != nullptr)
                {
                    // copy data from orginal data
                    auto physicSensorId = params.pPipelineStaticInfo->sensorId[i];
                    physicalPipelineStaticInfo->openId = physicSensorId;
                    physicalPipelineStaticInfo->sensorRawType = params.pPipelineStaticInfo->sensorRawType;
                    physicalPipelineStaticInfo->isType3PDSensorWithoutPDE = params.pPipelineStaticInfo->isType3PDSensorWithoutPDE;
                    physicalPipelineStaticInfo->is4CellSensor = params.pPipelineStaticInfo->is4CellSensor;
                    physicalPipelineStaticInfo->isVhdrSensor = params.pPipelineStaticInfo->isVhdrSensor;
                    physicalPipelineStaticInfo->isP1DirectFDYUV = params.pPipelineStaticInfo->isP1DirectFDYUV;
                    physicalPipelineStaticInfo->isP1DirectScaledYUV = params.pPipelineStaticInfo->isP1DirectScaledYUV;
                    physicalPipelineStaticInfo->isSupportBurstCap = params.pPipelineStaticInfo->isSupportBurstCap;
                    physicalPipelineStaticInfo->isDualDevice = false; // do fake.
                    physicalPipelineStaticInfo->sensorId = params.pPipelineStaticInfo->sensorId;
                    auto featureParams = featuresetting::CreationParams {
                        .pPipelineStaticInfo = physicalPipelineStaticInfo,
                        .pPipelineUserConfiguration = params.pPipelineUserConfiguration,
                        .bIsLogicalCam = false,
                    };
                    pPolicyTable->mFeaturePolicy_Physical.emplace(physicSensorId,
                        featuresetting::createFeatureSettingPolicyInstance(featureParams));
                }
            }
            //
            _MEDIATOR_( pRequestSettingPolicyMediator,  makeRequestSettingPolicyMediator_Multicam_Zoom );
            goto lbDefault;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  SMVR (Constrained Slow Motion)
    ////////////////////////////////////////////////////////////////////////////
    if (pParsedSMVRBatchInfo != nullptr)
    {
         //policy (configuration)
         MY_LOGD("SMVRBatch policy used");
        _POLICY_( fConfigSensorSetting,                          makePolicy_Configuration_SensorSetting_SMVRBatch() );
        _POLICY_( fConfigStreamInfo_AppImageStreamInfoMaxBufNum, makePolicy_Configuration_AppImageStreamInfoMaxBufNum_SMVR() );
        _POLICY_( fConfigStreamInfo_P1,                          makePolicy_Configuration_StreamInfo_P1_SMVRBatch() );

        //RequestMetadata (request)
        _METADATAPOLICY_(pRequestMetadataPolicy,                 requestmetadata::makePolicy_RequestMetadata_SMVRBatch);

        // mediator
        _MEDIATOR_( pRequestSettingPolicyMediator,               makeRequestSettingPolicyMediator_SMVRBatch);

        goto lbDefault;
    }
    else if (pParsedAppConfiguration->isConstrainedHighSpeedMode) {
         MY_LOGD("SMVRConstraint policy used");
        //policy (configuration)
        _POLICY_( fConfigSensorSetting,                         makePolicy_Configuration_SensorSetting_SMVR() );
        _POLICY_( fConfigStreamInfo_AppImageStreamInfoMaxBufNum,makePolicy_Configuration_AppImageStreamInfoMaxBufNum_SMVR() );
        _POLICY_( fConfigStreamInfo_P1,                         makePolicy_Configuration_StreamInfo_P1_SMVR() );
        goto lbDefault;
    }
    else
    {
         MY_LOGD("Neither SMVRConstraint or SMVRBatch policy used, pSMVRBatchInfo=%p", pParsedSMVRBatchInfo.get());
    }


    ////////////////////////////////////////////////////////////////////////////
    //  Super Night Mode Capture
    ////////////////////////////////////////////////////////////////////////////
    if  (pParsedAppConfiguration->isSuperNightMode || pParsedAppConfiguration->useP1DirectAppRaw) {
        CAM_ULOGMI("[%s] Super Night Mode", __FUNCTION__);
        if (pParsedAppConfiguration->isSuperNightMode)
        {
            _POLICY_( fConfigPipelineNodesNeed, makePolicy_Configuration_PipelineNodesNeed_AppRaw16Reprocess() );
            _POLICY_( fP2NodeDecision,          makePolicy_P2NodeDecision_AppRaw16Reprocess(
                                                makePolicy_P2NodeDecision_Default()) );
        }
        if  (pPipelineStaticInfo->is4CellSensor) {
            _POLICY_( fConfigSensorSetting, makePolicy_Configuration_SensorSetting_4Cell() );
        }

        _POLICY_( pRequestMetadataPolicy,   requestmetadata::makePolicy_RequestMetadata_AppRaw16Reprocess(
                                            requestmetadata::CreationParams{
                                                .pPipelineStaticInfo = params.pPipelineStaticInfo,
                                                .pPipelineUserConfiguration = params.pPipelineUserConfiguration,
                                                .pRequestMetadataPolicy = requestmetadata::makePolicy_RequestMetadata_Default(
                                                requestmetadata::CreationParams{
                                                    .pPipelineStaticInfo = params.pPipelineStaticInfo,
                                                    .pPipelineUserConfiguration = params.pPipelineUserConfiguration,
                                                }),
                                            }) );
        goto lbDefault;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Type3 PD Sensor + ISP3
    ////////////////////////////////////////////////////////////////////////////
    if  (pParsedAppConfiguration->isType3PDSensorWithoutPDE) {
        //mediator
        _MEDIATOR_( pRequestSettingPolicyMediator,  makeRequestSettingPolicyMediator_PDE );
        goto lbDefault;
    }
    ////////////////////////////////////////////////////////////////////////////
    //  4-Cell Sensor
    ////////////////////////////////////////////////////////////////////////////
    if  (pPipelineStaticInfo->is4CellSensor) {
        //policy (configuration)
        _POLICY_( fConfigSensorSetting, makePolicy_Configuration_SensorSetting_4Cell() );
        goto lbDefault;
    }
    ////////////////////////////////////////////////////////////////////////////
    //  Default (use the default policy if it's empty.)
    ////////////////////////////////////////////////////////////////////////////
lbDefault:
    {
        //policy (configuration)
        _POLICY_IF_EMPTY_( fConfigPipelineNodesNeed,         makePolicy_Configuration_PipelineNodesNeed_Default() );
        _POLICY_IF_EMPTY_( fConfigPipelineTopology,          makePolicy_Configuration_PipelineTopology_Default() );
        _POLICY_IF_EMPTY_( fConfigSensorSetting,             makePolicy_Configuration_SensorSetting_Default() );
        _POLICY_IF_EMPTY_( fConfigP1HwSetting,               makePolicy_Configuration_P1HwSetting_Default() );
        _POLICY_IF_EMPTY_( fConfigP1DmaNeed,                 makePolicy_Configuration_P1DmaNeed_Default() );
        _POLICY_IF_EMPTY_( fConfigStreamInfo_P1,             makePolicy_Configuration_StreamInfo_P1_Default() );
        _POLICY_IF_EMPTY_( fConfigStreamInfo_NonP1,          makePolicy_Configuration_StreamInfo_NonP1_Default() );
        _POLICY_IF_EMPTY_( fConfigStreamInfo_AppImageStreamInfoMaxBufNum,makePolicy_Configuration_AppImageStreamInfoMaxBufNum_Default() );

        //policy (request)
        _POLICY_IF_EMPTY_( fFaceDetectionIntent,             makePolicy_FDIntent_Default() );
        _POLICY_IF_EMPTY_( fP2NodeDecision,                  makePolicy_P2NodeDecision_Default() );
        _POLICY_IF_EMPTY_( fTopology,                        makePolicy_Topology_Default() );
        _POLICY_IF_EMPTY_( fCaptureStreamUpdater,            makePolicy_CaptureStreamUpdater_Default() );
        _POLICY_IF_EMPTY_( fIOMap_P2Node,                    makePolicy_IOMap_P2Node_Default() );
        _POLICY_IF_EMPTY_( fIOMap_NonP2Node,                 makePolicy_IOMap_NonP2Node_Default() );

        //RequestMetadata (request)
        _METADATAPOLICY_IF_EMPTY_( pRequestMetadataPolicy,  requestmetadata::makePolicy_RequestMetadata_Default );

        //feature
        _FEATUREPOLICY_IF_EMPTY_( mFeaturePolicy, featuresetting::createFeatureSettingPolicyInstance );

        //mediator
        _MEDIATOR_IF_EMPTY_( pConfigSettingPolicyMediator,   makeConfigSettingPolicyMediator_Default );
        _MEDIATOR_IF_EMPTY_( pRequestSettingPolicyMediator,  makeRequestSettingPolicyMediator_Default );

        ////////////////////////////////////////////////////////////////////////////
        //  for Debug Dump, use decorator pattern
        ////////////////////////////////////////////////////////////////////////////
        {
            int debugProcRaw = property_get_int32("vendor.debug.camera.cfg.ProcRaw", -1);
            if(debugProcRaw > 0)
            {
                MY_LOGD("vendor.debug.camera.cfg.ProcRaw=%d", debugProcRaw);
                _METADATAPOLICY_( pRequestMetadataPolicy,  requestmetadata::makePolicy_RequestMetadata_DebugDump );
            }
        }
	        ////////////////////////////////////////////////////////////////////////////
        //  HAL Jpeg for Debug & Tuning - decorator pattern
        ////////////////////////////////////////////////////////////////////////////
        if ( property_get_int32("vendor.debug.camera.haljpeg", 0) )
        {
            MY_LOGD("HAL Jpeg for debug & tuning");
            _POLICY_( fConfigPipelineNodesNeed, makePolicy_Configuration_PipelineNodesNeed_HalJpeg(pPolicyTable->fConfigPipelineNodesNeed) );
            _POLICY_( fConfigStreamInfo_NonP1,  makePolicy_Configuration_StreamInfo_NonP1_HalJpeg(pPolicyTable->fConfigStreamInfo_NonP1) );
            _POLICY_( fTopology, makePolicy_Request_Topology_HalJpeg(pPolicyTable->fTopology) );
            _POLICY_( fIOMap_NonP2Node, makePolicy_IOMap_NonP2Node_HalJpeg(pPolicyTable->fIOMap_NonP2Node) );
            //RequestMetadata (request)
            _METADATAPOLICY_IF_EMPTY_( pRequestMetadataPolicy,  requestmetadata::makePolicy_RequestMetadata_HalJpeg );
        }

        return MAKE_PIPELINE_POLICY(PipelineSettingPolicyImpl);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineSettingPolicyFactory::
createPipelineSettingPolicy(
    CreationParams const& params __unused
) -> std::shared_ptr<IPipelineSettingPolicy>
{
    auto pPolicyTable = std::make_shared<PolicyTable>();
    if  (CC_UNLIKELY(pPolicyTable == nullptr)) {
        MY_LOGE("Fail to make_shared<PolicyTable>");
        return nullptr;
    }

    auto pMediatorTable = std::make_shared<MediatorTable>();
    if  (CC_UNLIKELY(pMediatorTable == nullptr)) {
        MY_LOGE("Fail to make_shared<MediatorTable>");
        return nullptr;
    }

    return decidePolicyAndMake(params, pPolicyTable, pMediatorTable);
}

