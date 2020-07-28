/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHal3A.h>
//// MTKCAM3
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam3/3rdparty/core/scenario_mgr.h>
#include <mtkcam3/pipeline/policy/IFeatureSettingPolicy.h>

//
#include <mtkcam/utils/hw/IScenarioControlV3.h>
//
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace featuresetting {

struct DefaultConfigParams
{
    /**************************************************************************
     * Default(first request) config parameters
     *
     * The parameters related to keep the first request params is shown as below.
     *
     **************************************************************************/

    /**************************************************************************
     * Keep the defaul config for Feature Setting Policy
     *
     * Use the default config if features don't use dedicated config.
     *
     **************************************************************************/
    bool                                        bInit      = false;
    /**
     *  The default sensor setting
     */
    std::vector<uint32_t>                       sensorMode;

};

struct ParsedStrategyInfo
{
    // strategy info for common (per-frames info)
    uint32_t customHint = 0;

    // strategy info for capture feature
    bool isZslModeOn = false;
    bool isZslFlowOn = false;
    bool isFlashOn   = false;
    bool isCShot     = false;
    bool isFastCapture    = false;
    bool isRawReprocess   = false;
    bool isYuvReprocess   = false;
    bool isYuvRequestForIspHidl = false; // is request output cpature IQ yuv
    bool isRawRequestForIspHidl = false;
    //
    bool     isAppManual3A = false;
    uint32_t exposureTime  = 0;
    uint32_t realIso       = 0;
    int32_t  evSetting     = 0;
    uint8_t  aeMode        = MTK_CONTROL_AE_MODE_ON;
    uint8_t  flashMode     = MTK_FLASH_MODE_OFF;
    //
    int32_t ispTuningHint = -1; // hint to request buffer for reprocessing
    int32_t ispTuningFrameCount = -1; // hint to request frame count for multiframe reprocessing
    int32_t ispTuningFrameIndex = -1; // hint to request frame index for multiframe reprocessing
    bool  ispTuningOutputFirstFrame = false;
    bool  ispTuningOutputLastFrame  = false;
    //
    int32_t freeMemoryMBytes = -1;
    uint8_t captureIntent  = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    // strategy info for streaming feature (almost per-frames)
    // TODO: reserve
};

class HDRPolicyHelper;

/******************************************************************************
 *
 ******************************************************************************/
class FeatureSettingPolicy : public IFeatureSettingPolicy
{
    // for MultiFrame key feature (MFNR, HDR, 3rd party, etc)
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin MultiFramePlugin;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::IProvider::Ptr MFP_ProviderPtr;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::IInterface::Ptr MFP_InterfacePtr;
    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::Selection MFP_Selection;

    // for Raw domain key feature (SW 4Cell, etc)
    typedef NSCam::NSPipelinePlugin::RawPlugin RawPlugin;
    typedef NSCam::NSPipelinePlugin::RawPlugin::IProvider::Ptr Raw_ProviderPtr;
    typedef NSCam::NSPipelinePlugin::RawPlugin::IInterface::Ptr Raw_InterfacePtr;
    typedef NSCam::NSPipelinePlugin::RawPlugin::Selection Raw_Selection;

    // for Yuv domain key feature
    typedef NSCam::NSPipelinePlugin::YuvPlugin YuvPlugin;
    typedef NSCam::NSPipelinePlugin::YuvPlugin::IProvider::Ptr Yuv_ProviderPtr;
    typedef NSCam::NSPipelinePlugin::YuvPlugin::IInterface::Ptr Yuv_InterfacePtr;
    typedef NSCam::NSPipelinePlugin::YuvPlugin::Selection Yuv_Selection;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    class MFPPluginWrapper;
    using MFPPluginWrapperPtr = std::shared_ptr<MFPPluginWrapper>;
    class RawPluginWrapper;
    using RawPluginWrapperPtr = std::shared_ptr<RawPluginWrapper>;
    class YuvPluginWrapper;
    using YuvPluginWrapperPtr = std::shared_ptr<YuvPluginWrapper>;
    class BokehPluginWraper;
    using BokehPluginWraperPtr = std::shared_ptr<BokehPluginWraper>;
    class DepthPluginWraper;
    using DepthPluginWraperPtr = std::shared_ptr<DepthPluginWraper>;
    class FusionPluginWraper;
    using FusionPluginWraperPtr = std::shared_ptr<FusionPluginWraper>;

private:
    CreationParams mPolicyParams;
    // Key feature plug-in interfaces and provider
    // MultiFrame:
    MFPPluginWrapperPtr mMFPPluginWrapperPtr;
    // Raw plugin: (for 4Cell key feature)
    RawPluginWrapperPtr mRawPluginWrapperPtr;
    // Yuv plugin: (for 4Cell key feature)
    YuvPluginWrapperPtr mYuvPluginWrapperPtr;
    // Bokeh and Depth plugin: (for dualcam VSDOF key feature)
    BokehPluginWraperPtr                        mBokehPluginWraperPtr;
    //
    DepthPluginWraperPtr                        mDepthPluginWraperPtr;
    // Fusion plugin: (for dualcam fusion key feature)
    FusionPluginWraperPtr                       mFusionPluginWraperPtr;
    // default config
    DefaultConfigParams                          mDefaultConfig;

    // feature configure input data
    ConfigurationInputParams                     mConfigInputParams;
    // feature configure output data
    ConfigurationOutputParams                    mConfigOutputParams;

    // property for debug
    MINT32 mbDebug;
    MINT64 mForcedKeyFeatures = -1;
    MINT64 mForcedFeatureCombination = -1;
    MINT32 mForcedIspTuningHint = -1;

    // Hal3a
    std::mutex mHal3aLocker;
    std::vector<std::shared_ptr<NS3Av3::IHal3A>> mHal3a;

    //for 3A Isp Tuning MDP Target Size
    MSize mMdpTargetSize = MSize(0,0);

    // for multicam
    MINT32                                       miMultiCamFeatureMode = -1;
    DualDevicePath                               mDualDevicePath = DualDevicePath::Single;
    std::function<
                bool(
                    RequestOutputParams* out,
                    RequestInputParams const* in)>
                                                 mMultiCamStreamingUpdater = nullptr;
    //
    MINT32                                       mUniqueKey = -1;

    /******************************************************************************
     *
     ******************************************************************************/
    struct SensorInfo {
        MINT32 sensorId = -1;
        MINT32 sensorIdx = -1;
    };
    class SensorInfoUpdater{
    public:
        SensorInfoUpdater() = delete;
        SensorInfoUpdater(
                    RequestInputParams const* in,
                    bool const& isCaptureUpdate,
                    CreationParams &policyParams,
                    std::vector<SensorInfo>* sensorInfoList);
        ~SensorInfoUpdater();
    private:
        bool buildSensorInfoList(
                    CreationParams &policyParams,
                    uint32_t const& masterId,
                    std::vector<uint32_t> const& sensorList);
        std::vector<SensorInfo>* mSensorInfoList = nullptr;
    };
    // order: main, sub1, sub2...
    std::vector<SensorInfo> mSensorInfoList;
    /******************************************************************************
     *
     ******************************************************************************/

    // for dual cam [VSDOF]
    size_t                                       mSensorCount = 0;
    MUINT32                                      mPeriodForStreamingProcessDepth = 1;

    // for HDR
    std::shared_ptr<HDRPolicyHelper>               mHDRHelper = nullptr;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual auto    evaluateZslConfiguration(
                        ConfigurationOutputParams& out,
                        ConfigurationInputParams const& in
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // IFeatureSettingPolicy Interfaces.
    virtual auto    evaluateConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
                    ) -> int override;

    virtual auto    evaluateRequest(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> int override;

private:
    // FeatureSettingPolicy Interfaces.
    // Evaluate Feature Configuration Interfaces:
    virtual auto    evaluateCaptureConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
    ) -> bool;

    virtual auto    evaluateStreamConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
    ) -> bool;

    virtual auto    evaluateHDRConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
    ) -> bool;

    // Evaluate Feature Setting Interfaces:
    virtual auto    evaluateStreamSetting(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto    evaluateCaptureSetting(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto    getCaptureProvidersByScenarioFeatures(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const*
                    ) -> bool;

    // for capture strategy
    virtual auto queryPolicyState(
                        NSPipelinePlugin::Policy::State& state,
                        uint32_t sensorIndex,
                        ParsedStrategyInfo const& parsedInfo,
                        RequestOutputParams const* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto updateDualCamRequestOutputParams(
                        RequestOutputParams* out,
                        NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
                        NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional,
                        uint32_t mainCamP1Dma,
                        uint32_t sub1CamP1Dma,
                        MINT64 featureCombination,
                        unordered_map<uint32_t, NSPipelinePlugin::MetadataPtr>& halMetaPtrList,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto updateHDRMultiFrame(
                        RequestOutputParams* out,
                        RequestInputParams const* in,
                        NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
                        NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional
                    ) -> bool;

    virtual auto updatePolicyDecision(
                        RequestOutputParams* out,
                        uint32_t sensorIndex,
                        NSPipelinePlugin::Policy::Decision const& decision,
                        RequestInputParams const* in __unused
                    ) -> bool;

    virtual auto strategySingleRawPlugin(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;


    virtual auto strategyMultiFramePlugin(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto strategyDualCamPlugin(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in,
                        unordered_map<uint32_t, NSPipelinePlugin::MetadataPtr>& halMetaPtrList
                    ) -> bool;

    virtual auto strategyNormalSingleCapture(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto dumpRequestOutputParams(
                        RequestOutputParams* out,
                        RequestInputParams const* in,
                        bool forcedEnable
                    ) -> bool;

    virtual auto updatePluginSelection(
                        const uint32_t requestNo,
                        bool isFeatureTrigger,
                        uint8_t frameCount = 1
                    ) -> bool;

    virtual auto updateCaptureDummyFrames(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        const ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> void;

    virtual auto strategyCaptureFeature(
                        MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;

    //
    virtual auto evaluateReconfiguration(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto updateRequestResultParams(
                        std::shared_ptr<RequestResultParams> &requestParams,
                        NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
                        NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional,
                        uint32_t needP1Dma,
                        uint32_t sensorIndex,
                        MINT64 featureCombination = -1,
                        MINT32 requestIndex = 0,
                        MINT32 requestCount = 0
                    ) -> bool;

    virtual auto updateRequestOutputAllFramesMetadata(
                        RequestOutputParams* out,
                        NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
                        NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional
                    ) -> bool;

    virtual auto updateStreamData(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto collectDefaultSensorInfo(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto collectParsedStrategyCaptureInfo(
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto updateCaptureDebugInfo(
                        RequestOutputParams* out,
                        ParsedStrategyInfo& parsedInfo,
                        RequestInputParams const* in
                    ) -> bool;

    virtual auto getCaptureP1DmaConfig(
                        uint32_t &needP1Dma,
                        RequestInputParams const* in,
                        uint32_t sensorIndex = 0,
                        bool needRrzoBuffer = true
                    ) -> bool;

    virtual auto updateBoostControl(
                        RequestOutputParams* out,
                        const BoostControl& boostControl
                    ) -> bool;

    virtual bool updateMultiCamStreamingData(
                        RequestOutputParams* out,
                        RequestInputParams const* in
                    );

private:
    bool queryVsdofSensorCropRect(
                    RequestInputParams const* in,
                    MRect& sensor1,
                    MRect& sensor2);
    bool queryVsdofSensorCropRect(
                    int32_t index,
                    RequestInputParams const* in,
                    MRect& sensorCrop);
    bool queryVsdof3APreviewCropRect(
                    int32_t index,
                    RequestInputParams const* in,
                    MRect& sensorCrop);
    bool queryVsdofP1YuvCropRect(
                    int32_t index,
                    sp<IImageStreamInfo>& imgStreamInfo,
                    MRect& yuvCropRect);

    void toTPIDualHint(
                    scenariomgr::ScenarioHint &hint);

    void updateMulticamSensorControl(
                    RequestInputParams const* in,
                    uint32_t const& sensorId,
                    NSPipelinePlugin::MetadataPtr pHalMeta,
                    uint32_t &dma);

    bool queryMulticamTgCrop(
                    RequestInputParams const* in,
                    uint32_t const& sensorId,
                    MRect &tgCrop,
                    bool &isCapture);

    bool queryMulticamScalerCrop(
                    RequestInputParams const* in,
                    uint32_t const& sensorId,
                    MRect &scalerCrop,
                    bool &isCapture);

    bool isPhysicalStreamUpdate();

    bool getStreamDuration(
                    const IMetadata::IEntry& entry,
                    const MINT64 format,
                    const MSize& size,
                    MINT64& duration);

    bool multicamSyncMetadata(
                    RequestInputParams const* in,
                    RequestOutputParams* out,
                    unordered_map<uint32_t, NSPipelinePlugin::MetadataPtr>& halMetaPtrList);


    template <typename T>
    void syncMasterMetaToSlave(
                    NSPipelinePlugin::MetadataPtr masterMeta,
                    NSPipelinePlugin::MetadataPtr slaveMeta,
                    IMetadata::Tag_t tag);

public:
    // FeatureSettingPolicy Interfaces.
    FeatureSettingPolicy(CreationParams const& params);

    ~FeatureSettingPolicy();
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_H_

