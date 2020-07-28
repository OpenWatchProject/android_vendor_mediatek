/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_H_
//
//// MTKCAM
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//// MTKCAM3
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam3/3rdparty/core/scenario_mgr.h>
#include <mtkcam3/main/standalone_isp/IFeatureSettingPolicy_ISP.h>
//
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

using namespace NSCam::v3::pipeline::policy::scenariomgr;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace featuresetting_isp {

struct ParsedStrategyInfo
{
    // strategy info for isp feature
    bool isCShot     = false;
    //
    int32_t ispTuningHint = -1; // hint to request buffer for reprocessing
    IspProcessInOutType ispProcessInOutType = eIspProcessInOutType_Unknow;
    //
    int32_t freeMemoryMBytes = -1;
};

/******************************************************************************
 *
 ******************************************************************************/
class FeatureSettingPolicy_ISP : public IFeatureSettingPolicy_ISP
{
    // for MultiFrame key feature (MFNR, HDR, 3rd party, etc)
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin MultiFramePlugin;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::IProvider::Ptr MFP_ProviderPtr;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::IInterface::Ptr MFP_InterfacePtr;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::Selection MFP_Selection;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::RecommendedParam MFP_RecommendedParam;

    // for Raw domain key feature (SW 4Cell, etc)
    typedef NSCam::NSPipelinePlugin::RawPlugin RawPlugin;
    typedef NSCam::NSPipelinePlugin::RawPlugin::IProvider::Ptr Raw_ProviderPtr;
    typedef NSCam::NSPipelinePlugin::RawPlugin::IInterface::Ptr Raw_InterfacePtr;
    typedef NSCam::NSPipelinePlugin::RawPlugin::Selection Raw_Selection;
    typedef NSCam::NSPipelinePlugin::RawPlugin::RecommendedParam Raw_RecommendedParam;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    class MFPPluginWrapper;
    using MFPPluginWrapperPtr = std::shared_ptr<MFPPluginWrapper>;
    class RawPluginWrapper;
    using RawPluginWrapperPtr = std::shared_ptr<RawPluginWrapper>;

private:
    CreationParams mPolicyParams;
    // Key feature plug-in interfaces and provider
    // MultiFrame:
    MFPPluginWrapperPtr mMFPPluginWrapperPtr;
    // Raw plugin: (for 4Cell key feature)
    RawPluginWrapperPtr mRawPluginWrapperPtr;

    // feature configure input data
    ConfigurationInputParams                     mConfigInputParams;
    // feature configure output data
    ConfigurationOutputParams                    mConfigOutputParams;

    // current free memory during request for debug
    int32_t mCurrentFreeMemoryMBytes = -1;

    // property for debug
    MINT32 mbDebug;
    MINT64 mForcedKeyFeatures = -1;
    MINT64 mForcedFeatureCombination = -1;
    MINT32 mForcedIspTuningHint = -1;

    MINT32                                       mPipelineUniqueKey = -1;

    // keep output requirement info by main frame for multiple frame's sub-frames
    bool mIsOutputRaw = false;
    bool mIsOutputYuv = false;
    bool mIsOutputJpeg = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // IFeatureSettingPolicy_ISP Interfaces.
    virtual auto    evaluateConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
                    ) -> int override;

    virtual auto    evaluateRequest(
                        RequestParams* requestParams
                    ) -> int override;

private:
    // FeatureSettingPolicy_ISP Interfaces.
    // Evaluate Feature Configuration Interfaces:
    virtual auto    evaluateCaptureConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
    ) -> bool;

    // Evaluate Feature Setting Interfaces:
    virtual auto    evaluateCaptureSetting(
                        RequestParams* requestParams,
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto    getCaptureProvidersByScenarioFeatures(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const*
                    ) -> bool;

    // for capture strategy
    virtual auto queryPolicyState(
                        RequestParams* requestParams,
                        NSPipelinePlugin::Policy::StateIspHidl& state,
                        ParsedStrategyInfo const& parsedInfo
                    ) -> bool;

    virtual auto updatePolicyDecision(
                        RequestParams* requestParams,
                        NSPipelinePlugin::Policy::DecisionIspHidl const& decision
                    ) -> bool;

    virtual auto strategySingleRawPlugin(
                        RequestParams* requestParams,
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;


    virtual auto strategyMultiFramePlugin(
                        RequestParams* requestParams,
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto strategyNormalSingleCapture(
                        RequestParams* requestParams,
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto dumpRequestOutputParams(
                        RequestParams* requestParams,
                        bool forcedEnable
                    ) -> bool;

    virtual auto updatePluginSelection(
                        bool isFeatureTrigger,
                        uint8_t frameCount = 1
                    ) -> bool;

    virtual auto strategyCaptureFeature(
                        RequestParams* requestParams,
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto updateRequestResultParams(
                        RequestParams* requestParams,
                        NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
                        NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional,
                        MINT64 featureCombination = -1,
                        MINT32 requestIndex = 0,
                        MINT32 requestCount = 0
                    ) -> bool;

    virtual auto collectParsedStrategyInfo(
                        RequestParams* requestParams,
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto updateCaptureDebugInfo(
                        RequestParams* requestParams,
                        ParsedStrategyInfo& parsedInfo
                    ) -> bool;

    virtual auto queryRecommendedSetting(
                        RecommendedParams* recommendedParms
                    ) -> int;

    virtual auto queryCaptureRecommendedSetting(
                        RecommendedParams* recommendedParams
                    ) -> bool;

    virtual auto queryCaptureFeatureRecommendedSetting(
                        RecommendedParams* recommendedParams,
                        MINT64 combinedKeyFeature /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                    ) -> bool;

    virtual auto queryMultiFramePluginRecommendedSetting(
                        RecommendedParams* recommendedParams,
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                    ) -> bool;

public:
    // FeatureSettingPolicy_ISP Interfaces.
    FeatureSettingPolicy_ISP(CreationParams const& params);

    ~FeatureSettingPolicy_ISP();
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace featuresetting_isp
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_H_

