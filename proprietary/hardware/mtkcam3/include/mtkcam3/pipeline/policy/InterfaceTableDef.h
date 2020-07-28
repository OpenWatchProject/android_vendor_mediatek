/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_INTERFACETABLEDEF_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_INTERFACETABLEDEF_H_
//
// Mediator
#include <mtkcam3/pipeline/policy/IConfigSettingPolicyMediator.h>
#include <mtkcam3/pipeline/policy/IRequestSettingPolicyMediator.h>
//
// Policy (feature setting)
#include <mtkcam3/pipeline/policy/IFeatureSettingPolicy.h>
//
// Policy (Configuration)
#include <mtkcam3/pipeline/policy/IConfigPipelineNodesNeedPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigPipelineTopologyPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigSensorSettingPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigP1HwSettingPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigP1DmaNeedPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigStreamInfoPolicy.h>
#include <mtkcam3/pipeline/policy/IConfigAppImageStreamInfoMaxBufNumPolicy.h>
//
// Policy (Request)
#include <mtkcam3/pipeline/policy/IFaceDetectionIntentPolicy.h>
#include <mtkcam3/pipeline/policy/IP2NodeDecisionPolicy.h>
#include <mtkcam3/pipeline/policy/ITopologyPolicy.h>
#include <mtkcam3/pipeline/policy/ICaptureStreamUpdaterPolicy.h>
#include <mtkcam3/pipeline/policy/IIOMapPolicy.h>
#include <mtkcam3/pipeline/policy/IRequestMetadataPolicy.h>
#include <mtkcam3/pipeline/policy/IRequestSensorControlPolicy.h>
//
#include <mtkcam3/pipeline/stream/IStreamInfoBuilder.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace pipelinesetting {


/**
 * Policy table definition
 */
struct PolicyTable
{
    FunctionType_Configuration_PipelineNodesNeedPolicy  fConfigPipelineNodesNeed = nullptr;
    FunctionType_Configuration_PipelineTopologyPolicy   fConfigPipelineTopology = nullptr;
    FunctionType_Configuration_SensorSettingPolicy      fConfigSensorSetting = nullptr;
    FunctionType_Configuration_P1HwSettingPolicy        fConfigP1HwSetting = nullptr;
    FunctionType_Configuration_P1DmaNeedPolicy          fConfigP1DmaNeed = nullptr;
    FunctionType_Configuration_StreamInfo_P1            fConfigStreamInfo_P1 = nullptr;
    FunctionType_Configuration_StreamInfo_NonP1         fConfigStreamInfo_NonP1 = nullptr;

    FunctionType_Configuration_AppImageStreamInfoMaxBufNumPolicy
                                                        fConfigStreamInfo_AppImageStreamInfoMaxBufNum = nullptr;

    FunctionType_FaceDetectionIntentPolicy              fFaceDetectionIntent = nullptr;
    FunctionType_P2NodeDecisionPolicy                   fP2NodeDecision = nullptr;
    FunctionType_TopologyPolicy                         fTopology = nullptr;
    FunctionType_CaptureStreamUpdaterPolicy             fCaptureStreamUpdater = nullptr;
    FunctionType_IOMapPolicy_P2Node                     fIOMap_P2Node = nullptr;
    FunctionType_IOMapPolicy_NonP2Node                  fIOMap_NonP2Node = nullptr;
    std::shared_ptr<requestmetadata::IRequestMetadataPolicy>
                                                        pRequestMetadataPolicy = nullptr;
    std::shared_ptr<requestsensorcontrol::IRequestSensorControlPolicy>
                                                        pRequestSensorControlPolicy = nullptr;

    /* for feature policy */
    // logical
    std::shared_ptr<featuresetting::IFeatureSettingPolicy>
                                                        mFeaturePolicy = nullptr;
    // physical
    std::unordered_map<uint32_t, std::shared_ptr<featuresetting::IFeatureSettingPolicy> >
                                                        mFeaturePolicy_Physical;

};


/**
 * Mediator table definition
 */
struct MediatorTable
{
    std::shared_ptr<IConfigSettingPolicyMediator>       pConfigSettingPolicyMediator = nullptr;
    std::shared_ptr<IRequestSettingPolicyMediator>      pRequestSettingPolicyMediator = nullptr;

};


////////////////////////////////////////////////////////////////////////////////


/**
 * Mediator Creation related Parameters
 */
struct MediatorCreationParams
{
    std::shared_ptr<PipelineStaticInfo const>       pPipelineStaticInfo;
    std::shared_ptr<PipelineUserConfiguration const>pPipelineUserConfiguration;
    std::shared_ptr<PolicyTable>                    pPolicyTable;
    std::shared_ptr<IStreamInfoBuilderFactory>      pAppStreamInfoBuilderFactory;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_INTERFACETABLEDEF_H_

