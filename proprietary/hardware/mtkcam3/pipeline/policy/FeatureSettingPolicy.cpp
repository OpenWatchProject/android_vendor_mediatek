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
#define LOG_TAG "mtkcam-FeatureSettingPolicy"
//
#include <algorithm>
#include <tuple>
// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
// dual cam
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/vhdr/HDRPolicyHelper.h>
#include <camera_custom_stereo.h>
#include <isp_tuning.h>
#include <mtkcam/aaa/aaa_hal_common.h>
//
#include "FeatureSettingPolicy.h"
#include <mtkcam/utils/hw/IScenarioControlV3.h>
// debug exif
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <debug_exif/cam/dbg_cam_param.h>
// aee
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FEATURE_SETTING_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::featuresetting;
using namespace NSCam::v3::pipeline::policy::scenariomgr;
using namespace NSCam::v3::pipeline::model;
using namespace NS3Av3; //IHal3A
using namespace NSCamHW; //HwInfoHelper
using namespace NSCam::v3::pipeline::model;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

// TODO: rename
#define SENSOR_INDEX_MAIN   (0)
#define SENSOR_INDEX_SUB1   (1)
#define SENSOR_INDEX_SUB2   (2)
// TODO: disable thsi flag before MP
#define DEBUG_FEATURE_SETTING_POLICY  (0)

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
class PluginWrapper
{
public:
    using PluginPtr      = typename TPlugin::Ptr;
    using ProviderPtr    = typename TPlugin::IProvider::Ptr;
    using InterfacePtr   = typename TPlugin::IInterface::Ptr;
    using SelectionPtr   = typename TPlugin::Selection::Ptr;
    using Selection      = typename TPlugin::Selection;
    using Property       = typename TPlugin::Property;
    using ProviderPtrs   = std::vector<ProviderPtr>;
public:
    PluginWrapper(const std::string& name, MUINT64 uSupportedFeatures = ~0, MINT32 iUniqueKey = -1, const MBOOL isPhysical = MFALSE);
    ~PluginWrapper();
public:
    auto getName() const -> const std::string&;
    auto isKeyFeatureExisting(MINT64 combinedKeyFeature, MINT64& keyFeature) const -> MBOOL;
    auto tryGetKeyFeature(MINT64 combinedKeyFeature, MINT64& keyFeature, MINT8& keyFeatureIndex) const -> MBOOL;
    auto getProvider(MINT64 combinedKeyFeature, MINT64& keyFeature) -> ProviderPtr;
    auto getProviders() -> ProviderPtrs;
    auto createSelection() const -> SelectionPtr;
    auto offer(Selection& sel) const -> MVOID;
    auto keepSelection(const uint32_t requestNo, ProviderPtr& providerPtr, SelectionPtr& sel) -> MVOID;
    auto pushSelection(const uint32_t requestNo, const uint8_t frameCount) -> MVOID;
    auto cancel() -> MVOID;
private:
    using ProviderPtrMap = std::unordered_map<MUINT64, ProviderPtr>;
    using SelectionPtrMap = std::unordered_map<ProviderPtr, std::vector<SelectionPtr>>;
private:
    const std::string       mName;
    const MINT64            mSupportedFeatures;
    const MINT32            mUniqueKey;
    PluginPtr               mInstancePtr;
    ProviderPtrs            mProviderPtrsSortedByPriorty;
    SelectionPtrMap         mTempSelectionPtrMap;
    InterfacePtr            mInterfacePtr;
    const MBOOL             mIsPhysical;
};

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
PluginWrapper<TPlugin>::
PluginWrapper(const std::string& name, MUINT64 uSupportedFeatures, MINT32 uniqueKey, const MBOOL isPhysical)
: mName(name)
, mSupportedFeatures(uSupportedFeatures)
, mUniqueKey(uniqueKey)
, mIsPhysical(isPhysical)
{
    CAM_ULOGM_APILIFE();
    MY_LOGD("ctor:%p, name:%s, supportedFeatures:%#" PRIx64 ", uniqueKey:%d",
        this, mName.c_str(), mSupportedFeatures, mUniqueKey);
    mInstancePtr = TPlugin::getInstance(mUniqueKey);
    if (mInstancePtr) {
        mInterfacePtr = mInstancePtr->getInterface();
        auto& providers = mInstancePtr->getProviders(/*mSupportedFeatures*/);
        mProviderPtrsSortedByPriorty = providers;
        std::sort(mProviderPtrsSortedByPriorty.begin(), mProviderPtrsSortedByPriorty.end(),
            [] (const ProviderPtr& p1, const ProviderPtr& p2) {
                return p1->property().mPriority > p2->property().mPriority;
            }
        );

        for (auto& provider : mProviderPtrsSortedByPriorty) {
            const Property& property = provider->property();
            MY_LOGD("find provider... name:%s, algo(%#" PRIx64"), priority(0x%x)", property.mName, property.mFeatures, property.mPriority);
        }
    }
    else {
        MY_LOGW("cannot get instance for %s features strategy", mName.c_str());
    }
}

template<typename TPlugin>
PluginWrapper<TPlugin>::
~PluginWrapper()
{
    MY_LOGD("dtor:%p name:%s, uniqueKey:%d",
        this, mName.c_str(), mUniqueKey);
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
getName() const -> const std::string&
{
    return mName;
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
isKeyFeatureExisting(MINT64 combinedKeyFeature, MINT64& keyFeature) const -> MBOOL
{
    MINT8  keyFeatureIndex = 0;
    return tryGetKeyFeature(combinedKeyFeature, keyFeature, keyFeatureIndex);
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
tryGetKeyFeature(MINT64 combinedKeyFeature, MINT64& keyFeature, MINT8& keyFeatureIndex) const -> MBOOL
{
    for(MUINT8 i=0; i < mProviderPtrsSortedByPriorty.size(); i++) {
        auto providerPtr = mProviderPtrsSortedByPriorty.at(i);
        keyFeature = providerPtr->property().mFeatures;
        if ((keyFeature & combinedKeyFeature) != 0) {
            keyFeatureIndex = i;
            return MTRUE;
        }
    }

    // if no plugin found, must hint no feature be chose.
    keyFeature = 0;
    keyFeatureIndex = 0;
    return MFALSE;
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
getProvider(MINT64 combinedKeyFeature, MINT64& keyFeature) -> ProviderPtr
{
    MINT8  keyFeatureIndex = 0;
    return tryGetKeyFeature(combinedKeyFeature, keyFeature, keyFeatureIndex) ? mProviderPtrsSortedByPriorty[keyFeatureIndex] : nullptr;
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
getProviders() -> ProviderPtrs
{
    ProviderPtrs ret;
    ret = mProviderPtrsSortedByPriorty;
    return std::move(ret);
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
createSelection() const -> SelectionPtr
{
    return mInstancePtr->createSelection();
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
offer(Selection& sel) const -> MVOID
{
    mInterfacePtr->offer(sel);
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
keepSelection(const uint32_t requestNo, ProviderPtr& providerPtr, SelectionPtr& sel) -> MVOID
{
    if (mTempSelectionPtrMap.find(providerPtr) != mTempSelectionPtrMap.end()) {
        mTempSelectionPtrMap[providerPtr].push_back(sel);
        MY_LOGD("%s: selection size:%zu, requestNo:%u",getName().c_str(), mTempSelectionPtrMap[providerPtr].size(), requestNo);
    }
    else {
        std::vector<SelectionPtr> vSelection;
        vSelection.push_back(sel);
        mTempSelectionPtrMap[providerPtr] = vSelection;
        MY_LOGD("%s: new selection size:%zu, requestNo:%u", getName().c_str(), mTempSelectionPtrMap[providerPtr].size(), requestNo);
    }
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
pushSelection(const uint32_t requestNo, const uint8_t frameCount) -> MVOID
{
    for (auto item : mTempSelectionPtrMap) {
        auto providerPtr = item.first;
        auto vSelection  = item.second;
        MY_LOGD("%s: selection size:%zu, frameCount:%d", getName().c_str(), vSelection.size(), frameCount);
        if (frameCount > 1 && vSelection.size() == 1) {
            auto sel = vSelection.front();
            for (size_t i = 0; i < frameCount; i++) {
                MY_LOGD("%s: duplicate selection for multiframe(count:%d, index:%zu)", getName().c_str(), frameCount, i);

                auto pSelection = std::make_shared<Selection>(*sel);
                pSelection->mTokenPtr = Selection::createToken(mUniqueKey, requestNo, i, pSelection->mState.mSensorId, mIsPhysical);
                mInstancePtr->pushSelection(providerPtr, pSelection);
            }
        }
        else {
            for (auto sel : vSelection) {
                mInstancePtr->pushSelection(providerPtr, sel);
            }
        }
        vSelection.clear();
    }
    mTempSelectionPtrMap.clear();
}

template<typename TPlugin>
auto
PluginWrapper<TPlugin>::
cancel() -> MVOID
{
    for (auto item : mTempSelectionPtrMap) {
        auto providerPtr = item.first;
        auto vSelection  = item.second;
        if (providerPtr.get()) {
            //providerPtr->cancel();
        }
        MY_LOGD("%s: selection size:%zu", getName().c_str(), vSelection.size());
        vSelection.clear();
    }
    mTempSelectionPtrMap.clear();
}

#define DEFINE_PLUGINWRAPER(CLASSNAME, PLUGINNAME)                                                                      \
class FeatureSettingPolicy::CLASSNAME final: public PluginWrapper<NSCam::NSPipelinePlugin::PLUGINNAME>                  \
{                                                                                                                       \
public:                                                                                                                 \
    /*Feature Provider*/                                                                                                \
    CLASSNAME(MUINT64 uSupportedFeatures, MINT32 iUniqueKey, const MBOOL isPhysical)                                                            \
    : PluginWrapper<NSCam::NSPipelinePlugin::PLUGINNAME>(#PLUGINNAME, uSupportedFeatures, iUniqueKey, isPhysical)       \
    {                                                                                                                   \
    }                                                                                                                   \
}
DEFINE_PLUGINWRAPER(MFPPluginWrapper, MultiFramePlugin);
DEFINE_PLUGINWRAPER(RawPluginWrapper, RawPlugin);
DEFINE_PLUGINWRAPER(YuvPluginWrapper, YuvPlugin);
DEFINE_PLUGINWRAPER(BokehPluginWraper, BokehPlugin);
DEFINE_PLUGINWRAPER(DepthPluginWraper, DepthPlugin);
DEFINE_PLUGINWRAPER(FusionPluginWraper, FusionPlugin);
#undef DEFINE_PLUGINWRAPER

/******************************************************************************
 *
 ******************************************************************************/
FeatureSettingPolicy::SensorInfoUpdater::
SensorInfoUpdater(
    RequestInputParams const* in,
    bool const& isCaptureUpdate,
    CreationParams &policyParams,
    std::vector<SensorInfo>* sensorInfoList
)
{
    if(sensorInfoList == nullptr) {
        MY_LOGE("sensorInfoList is nullptr");
        return;
    }
    mSensorInfoList = sensorInfoList;
    if(in->pMultiCamReqOutputParams != nullptr) {
        if(policyParams.bIsLogicalCam) {
            auto masterId = in->pMultiCamReqOutputParams->masterId;
            auto streamingSensorList = in->pMultiCamReqOutputParams->streamingSensorList;
            if(isCaptureUpdate) {
                masterId = in->pMultiCamReqOutputParams->prvMasterId;
                streamingSensorList = in->pMultiCamReqOutputParams->prvStreamingSensorList;
            }
            buildSensorInfoList(policyParams, masterId, streamingSensorList);
            MY_LOGI_IF(
                    isCaptureUpdate,
                    "Logical m(%d:%d) s(%d:%d)",
                    (*mSensorInfoList)[0].sensorId, (*mSensorInfoList)[0].sensorIdx,
                    (*mSensorInfoList)[1].sensorId, (*mSensorInfoList)[1].sensorIdx);
        }
        else {
            buildSensorInfoList(
                policyParams,
                policyParams.pPipelineStaticInfo->openId,
                std::vector<uint32_t>());
            MY_LOGI_IF(
                    isCaptureUpdate,
                    "physical m(%d:%d)",
                    (*mSensorInfoList)[0].sensorId, (*mSensorInfoList)[0].sensorIdx);
        }
    }
    else {
        bool isManuallyAddedDevice = true;
        for(auto& id:policyParams.pPipelineStaticInfo->sensorId) {
            if(policyParams.pPipelineStaticInfo->openId == id) {
                // if sensor id is same to open id, it means physical sensor device.
                // (In current added device rule, physical sensor id will same to
                // device Id.)
                isManuallyAddedDevice &= false;
            }
        }
        if(isManuallyAddedDevice) {
            // for multicam device, it cannot use openid as master id.
            // otherwise, index will be -1.
            // so, it has to use first sensor id in sensor id list.
            buildSensorInfoList(
                policyParams,
                policyParams.pPipelineStaticInfo->sensorId[0],
                std::vector<uint32_t>());
        }
        else {
            buildSensorInfoList(
                policyParams,
                policyParams.pPipelineStaticInfo->openId,
                std::vector<uint32_t>());
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
FeatureSettingPolicy::SensorInfoUpdater::
~SensorInfoUpdater()
{
    if(mSensorInfoList != nullptr)
    {
        mSensorInfoList->clear();
    }
}
/******************************************************************************
 *
 ******************************************************************************/
bool
FeatureSettingPolicy::SensorInfoUpdater::
buildSensorInfoList(
    CreationParams &policyParams,
    uint32_t const& masterId,
    std::vector<uint32_t> const& sensorList
)
{
    auto getIndex = [&policyParams](uint32_t const id) {
        auto index = policyParams.pPipelineStaticInfo->getIndexFromSensorId(id);
        if(index == -1) {
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
            aee_system_exception(
                LOG_TAG,
                NULL,
                DB_OPT_DEFAULT,
                "\nCRDISPATCH_KEY:(%d) invalid index(-1)", id);
#endif
        }
        return index;
    };
    mSensorInfoList->push_back(
                            SensorInfo{
                                    (MINT32)masterId,
                                    getIndex(masterId)
                                    });
    for(auto&& id:sensorList) {
        if(id != masterId) {
            mSensorInfoList->push_back(
                                    SensorInfo{
                                            (MINT32)id,
                                            getIndex(id)
                                            });
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
FeatureSettingPolicy::
FeatureSettingPolicy(
    CreationParams const& params
)
    :mPolicyParams(params)
{
    CAM_ULOGM_APILIFE();
    MY_LOGI("(%p) ctor, openId(%d), sensors_count(%zu), is_dual_cam(%d)",
            this,
            mPolicyParams.pPipelineStaticInfo->openId,
            mPolicyParams.pPipelineStaticInfo->sensorId.size(),
            mPolicyParams.pPipelineStaticInfo->isDualDevice);
    mbDebug = ::property_get_int32("vendor.debug.camera.featuresetting.log", DEBUG_FEATURE_SETTING_POLICY);
    // forced feature strategy for debug
    mForcedKeyFeatures = ::property_get_int64("vendor.debug.featuresetting.keyfeature", -1);
    mForcedFeatureCombination = ::property_get_int64("vendor.debug.featuresetting.featurecombination", -1);

    mForcedIspTuningHint = ::property_get_int32("vendor.debug.featuresetting.isptuninghint", -1);

    // multicam mode flag init
    auto& pParsedAppConfiguration = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto& pParsedMultiCamInfo = pParsedAppConfiguration->pParsedMultiCamInfo;
    if(pParsedMultiCamInfo != nullptr) {
        mDualDevicePath = pParsedMultiCamInfo->mDualDevicePath;
        miMultiCamFeatureMode = pParsedMultiCamInfo->mDualFeatureMode;
        // single flow:
        // 1. dual device && dual device not equal to multicam.
        // 2. not dual device
        bool bUsingSingleFlow = ((mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->pParsedMultiCamInfo->mSupportPhysicalOutput == false)
                                && (miMultiCamFeatureMode == -1));

        if (bUsingSingleFlow) {
            MY_LOGD("using single flow");
        }
        else {
            // check need sync af
            MY_LOGD("multicam flow. feature mode(%d)", miMultiCamFeatureMode);
            mMultiCamStreamingUpdater =
                            std::bind(
                                &FeatureSettingPolicy::updateMultiCamStreamingData,
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2);
        }
    }

    // create Hal3A for 3A info query
    mHal3a.clear();
    for (auto& id : mPolicyParams.pPipelineStaticInfo->sensorId) {
        std::shared_ptr<IHal3A> hal3a
        (
            MAKE_Hal3A(id, LOG_TAG),
            [](IHal3A* p){ if(p) p->destroyInstance(LOG_TAG); }
        );
        if (hal3a.get()) {
            mHal3a.push_back(hal3a);
        }
        else {
            MY_LOGE("sensorId(%d) MAKE_Hal3A failed, it nullptr", id);
        }
    }

    {
        sp<IMetadataProvider> metaProvider = NSMetadataProviderManager::valueFor(mPolicyParams.pPipelineStaticInfo->sensorId[SENSOR_INDEX_MAIN]);
        mHDRHelper = std::make_shared<HDRPolicyHelper>(mPolicyParams.pPipelineStaticInfo->sensorId[SENSOR_INDEX_MAIN], metaProvider->getMtkStaticCharacteristics());
    }

}

FeatureSettingPolicy::
~FeatureSettingPolicy()
{
    MY_LOGI("(%p) dtor", this);
}

auto
FeatureSettingPolicy::
collectDefaultSensorInfo(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> bool
{
    //CAM_ULOGM_APILIFE();
    bool ret = true;
    // collect parsed info for common strategy (only per-frame requirement)
    // Note: It is per-frames query behavior here.
    // TODO: collect parsed info for common strategy
    auto sensorModeSize = in->sensorMode.size();
    auto sensorIdSize = mPolicyParams.pPipelineStaticInfo->sensorId.size();
    if (sensorModeSize != sensorIdSize) {
        MY_LOGF("input sesnorMode size(%zu) != sensorId(%zu), cannot strategy the feature policy correctly",
                sensorModeSize, sensorIdSize);
        return false;
    }
    // keep first request config as default setting (ex: defualt sensor mode).
    if (mDefaultConfig.bInit == false) {
        MY_LOGI("keep the first request config as default config");
        mDefaultConfig.sensorMode = in->sensorMode;
        mDefaultConfig.bInit = true;
    }
    // use the default setting, features will update it later.
    out->sensorMode = mDefaultConfig.sensorMode;

    return ret;
}

auto
FeatureSettingPolicy::
collectParsedStrategyCaptureInfo(
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> bool
{
    //CAM_ULOGM_APILIFE();
    bool ret = true;
    // collect parsed info for capture feature strategy
    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    if (CC_UNLIKELY(in->pRequest_ParsedAppMetaControl == nullptr)) {
        MY_LOGW("cannot get ParsedMetaControl, invalid nullptr");
    }
    else {
        parsedInfo.isZslModeOn = mConfigOutputParams.isZslMode;
        parsedInfo.isZslFlowOn = in->pRequest_ParsedAppMetaControl->control_enableZsl;
    }
    // obtain latest real iso information for capture strategy
    {
        CaptureParam_T captureParam;
        if (mainSensorIndex >= 0) {
            auto hal3a = mHal3a[mainSensorIndex];
            if (hal3a.get()) {
                std::lock_guard<std::mutex> _l(mHal3aLocker);
                ExpSettingParam_T expParam;
                hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);
                hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&captureParam, 0);
            }
            else {
                MY_LOGE("create IHal3A instance failed! cannot get current real iso for strategy");
                ::memset(&captureParam, 0, sizeof(CaptureParam_T));
                ret = false;
            }
            parsedInfo.realIso = captureParam.u4RealISO;
            parsedInfo.exposureTime = captureParam.u4Eposuretime; //us

            // query flash status from Hal3A
            int isFlashOn = 0;
            hal3a->send3ACtrl(NS3Av3::E3ACtrl_GetIsFlashOnCapture, (MINTPTR)&isFlashOn, 0);
            if (isFlashOn) {
                parsedInfo.isFlashOn = true;
            }
        } else {
            MY_LOGE("create IHal3A instance failed! master sensorId index:%d", mainSensorIndex);
            ret = false;
        }
    }
    // get info from AppControl Metadata
    {
        auto pAppMetaControl = in->pRequest_AppControl;
        //
        MUINT8 aeLock = MTK_CONTROL_AE_LOCK_OFF;
        MINT32 evSetting = 0;
        MINT32 manualIso = 0;
        MINT64 manualExposureTime = 0;
        if (IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_CONTROL_AE_MODE, parsedInfo.aeMode)) {
            MY_LOGD("App set AE mode = %d", parsedInfo.aeMode);
        }
        if (IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_CONTROL_AE_LOCK, aeLock)) {
            MY_LOGD("App set AE lock = %d", aeLock);
        }
        if (parsedInfo.aeMode  == MTK_CONTROL_AE_MODE_OFF) {
            MY_LOGI("it is manual AE (mode:%d, lock:%d)", parsedInfo.aeMode, aeLock);
            parsedInfo.isAppManual3A = MTRUE;
            if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_SENSOR_SENSITIVITY, manualIso) && manualIso > 0) {
                parsedInfo.realIso = manualIso;
                MY_LOGD("App set ISO = %d", parsedInfo.realIso);
            }
            if (IMetadata::getEntry<MINT64>(pAppMetaControl, MTK_SENSOR_EXPOSURE_TIME, manualExposureTime) && manualExposureTime > 0) {
                parsedInfo.exposureTime = manualExposureTime/1000; //ns to us
                MY_LOGD("App set shutter = %d", parsedInfo.exposureTime);
            }
            if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, evSetting)) {
                parsedInfo.evSetting = evSetting;
                MY_LOGD("App set EV = %d", parsedInfo.evSetting);
            }
        }
        else if (aeLock == MTK_CONTROL_AE_LOCK_ON) {
            // AE lock only, but not AE mode off.
            // the 3A setting (iso, shutter) maybe lock during preview
            if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, evSetting)) {
                parsedInfo.isAppManual3A = MTRUE;
                parsedInfo.evSetting = evSetting;
                MY_LOGI("It is AE lock(mode:%d, lock:%d) and App set EV = %d",parsedInfo.aeMode, aeLock, parsedInfo.evSetting);
            }
        }
        // check for debug
        if (parsedInfo.isZslFlowOn && parsedInfo.isAppManual3A) {
            MY_LOGW("unknow behavior: App request manual AE and ZSL at the same time.");
        }
        if (!IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_FLASH_MODE, parsedInfo.flashMode)) {
            MY_LOGD("get metadata MTK_FLASH_MODE failed! cannot get current flash mode for strategy");
        }
        MY_LOGD("App request aeMode:%d, flashMode:%d", parsedInfo.aeMode, parsedInfo.flashMode);
        if (parsedInfo.aeMode  == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
            (parsedInfo.aeMode == MTK_CONTROL_AE_MODE_ON && parsedInfo.flashMode == MTK_FLASH_MODE_TORCH)) {
            parsedInfo.isFlashOn = true;
        }
        MINT32 cshot = 0;
        MUINT8 captureIntent = MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE;
        if(!IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CSHOT_FEATURE_CAPTURE, cshot)) {
            MY_LOGD_IF(2 <= mbDebug, "cannot get MTK_CSHOT_FEATURE_CAPTURE");
        }
        if(!IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_CONTROL_CAPTURE_INTENT, captureIntent)){
            MY_LOGD_IF(2 <= mbDebug, "cannot get MTK_CONTROL_CAPTURE_INTENT");
        }
        parsedInfo.captureIntent = captureIntent;
        MBOOL bFastCapture = !(captureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE || captureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT || captureIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG);
        MY_LOGD("cshot %d, captureIntent %d, bFastCapture %d", cshot, captureIntent, bFastCapture);
        if (cshot || bFastCapture) {
            parsedInfo.isCShot       = true;
            parsedInfo.isFastCapture = bFastCapture;
        }

        // app request frame for isp hidl flow usage
        MINT32 isp_tuning_hint = -1;
        if (in->pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv || in->pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Raw
            || (in->pRequest_AppControl != nullptr && in->pRequest_ParsedAppMetaControl->control_isp_tuning != 0)) {
            if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, isp_tuning_hint)) {
                MY_LOGD("get metadata MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING:%d", isp_tuning_hint);
                parsedInfo.ispTuningHint = isp_tuning_hint;
            }
            else {
                parsedInfo.ispTuningHint = MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_DEFAULT_NONE;
                MY_LOGD("App request IspTuningData_Yuv w/o MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING (set as default:%d)", parsedInfo.ispTuningHint);
            }
        }
        if (mForcedIspTuningHint >= 0) {
            parsedInfo.ispTuningHint = mForcedIspTuningHint;
            MY_LOGI("force ispTuningHint(%d) for feature setting policy", parsedInfo.ispTuningHint);
        }
        // app hint to request frame count/index for multiframe reprocessing
        parsedInfo.ispTuningFrameCount = -1;
        parsedInfo.ispTuningFrameIndex = -1;
        if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT, parsedInfo.ispTuningFrameCount)) {
            MY_LOGD("get metadata MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT:%d", parsedInfo.ispTuningFrameCount);
        }
        if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, parsedInfo.ispTuningFrameIndex)) {
            MY_LOGD("get metadata MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX:%d", parsedInfo.ispTuningFrameIndex);
        }

        if (parsedInfo.ispTuningFrameCount > 0) {
            if (parsedInfo.ispTuningFrameIndex == 0){
                parsedInfo.ispTuningOutputFirstFrame = true;
                MY_LOGD("It is first frame to output for app due to app set frame hint(count:%d, index:%d)",
                        parsedInfo. ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
            }
            if (parsedInfo.ispTuningFrameIndex+1 == parsedInfo.ispTuningFrameCount){
                parsedInfo.ispTuningOutputLastFrame = true;
                MY_LOGD("It is last frame to output for app due to app set frame hint(count:%d, index:%d)",
                        parsedInfo. ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
            }
        }
    }
    // check
    if (in->pRequest_AppImageStreamInfo->pAppImage_Input_Yuv) {
        parsedInfo.isYuvReprocess = true;
    }
    if (in->pRequest_AppImageStreamInfo->pAppImage_Input_Priv || in->pRequest_AppImageStreamInfo->pAppImage_Input_RAW16) {
        parsedInfo.isRawReprocess = true;
    }
    if (parsedInfo.ispTuningHint >= 0) {
        if (in->needRawOutput) {
            // is request output RAW16
            parsedInfo.isRawRequestForIspHidl = true;
        }
        if (in->needP2CaptureNode) {
            // is request output cpature IQ yuv
            parsedInfo.isYuvRequestForIspHidl = true;
        }
    }

    //after doing capture, vhdr need to add dummy frame
    if (!mHDRHelper->notifyDummy()) {
        MY_LOGE("HDRPolciyHelper notifyDummy failed");
    }

    parsedInfo.freeMemoryMBytes = (NSCam::NSMemoryInfo::getFreeMemorySize()/1024/1024); // convert Bytes to MB

    MY_LOGD("strategy info for capture feature(isZsl(mode:%d, flow:%d), (isCShot/isFastCapture:%d/%d), isFlashOn:%d, iso:%d, shutterTimeUs:%d), reprocess(raw:%d, yuv:%d), freeMem:%dMB, request_for_isp_hidl(raw:%d, yuv:%d)",
            parsedInfo.isZslModeOn, parsedInfo.isZslFlowOn, parsedInfo.isCShot, parsedInfo.isFastCapture, parsedInfo.isFlashOn, parsedInfo.realIso, parsedInfo.exposureTime,
            parsedInfo.isRawReprocess, parsedInfo.isYuvReprocess, parsedInfo.freeMemoryMBytes, parsedInfo.isRawRequestForIspHidl, parsedInfo.isYuvRequestForIspHidl);
    return ret;
}

auto
FeatureSettingPolicy::
updateCaptureDebugInfo(
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in __unused
) -> bool
{

    if (out->mainFrame.get() == nullptr) {
        MY_LOGE("no mainFrame to update debug exif info, cannot update debug exif");
        return false;
    }

    // collect all frames' Hal Metadata for debug exif updating
    // main frame
    std::vector<MetadataPtr> vHalMeta;
    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    vHalMeta.push_back(out->mainFrame->additionalHal[mainSensorIndex]);
    // subframes
    for (size_t i = 0; i < out->subFrames.size(); i++) {
        auto subFrame = out->subFrames[i];
        if (subFrame.get()) {
            vHalMeta.push_back(subFrame->additionalHal[mainSensorIndex]);
        }
    }

    for (size_t i = 0; i < vHalMeta.size(); i++) {
        auto pHalMeta = vHalMeta[i];
        if (CC_UNLIKELY(pHalMeta.get() == nullptr)) {
            MY_LOGE("pHalMeta[%zu] is invalid nullptr!!", i);
            return false;
        }
        IMetadata::setEntry<MINT32>(pHalMeta.get(), MTK_FEATURE_FREE_MEMORY_MBYTE, parsedInfo.freeMemoryMBytes);
    }

    return true;
}

auto
FeatureSettingPolicy::
getCaptureP1DmaConfig(
    uint32_t &needP1Dma,
    RequestInputParams const* in,
    uint32_t sensorIndex,
    bool needRrzoBuffer //TODO: modify default value to false after ZSL support partial P1 buffer request
) -> bool
{
    bool ret = true;

    size_t streamInfoP1Num = (*(in->pConfiguration_StreamInfo_P1)).size();
    if (sensorIndex >= streamInfoP1Num) {
        MY_LOGE("index:%d is out of in->pConfiguration_StreamInfo_P1 num:%zu", sensorIndex, streamInfoP1Num);
        return false;
    }

    // IMGO
    if ((*(in->pConfiguration_StreamInfo_P1))[sensorIndex].pHalImage_P1_Imgo != nullptr) {
        needP1Dma |= P1_IMGO;
    }
    else {
        MY_LOGI("The current pipeline config without IMGO output");
    }
    // RRZO
    if ((*(in->pConfiguration_StreamInfo_P1))[sensorIndex].pHalImage_P1_Rrzo != nullptr) {
        if (in->needP2StreamNode || miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF || needRrzoBuffer) {
            // must need rrzo for streaming and vsdof
            needP1Dma |= P1_RRZO;
        }
        else {
            // capture reuest will decide the requirement of rrzo by triggered provider
            MY_LOGD("needP2StreamNode(%d), miMultiCamFeatureMode(%d), the rrzo will request by triggered provider",
                    in->needP2StreamNode, miMultiCamFeatureMode);
        }
    }
    else {
        MY_LOGI("The current pipeline config without RRZO output");
    }
    // LCSO
    if ((*(in->pConfiguration_StreamInfo_P1))[sensorIndex].pHalImage_P1_Lcso != nullptr) {
        needP1Dma |= P1_LCSO;
    }
    else {
        MY_LOGD("The current pipeline config without LCSO output");
    }
    if ( !(needP1Dma & (P1_IMGO|P1_RRZO)) ) {
        MY_LOGW("P1Dma output without source buffer, sensorIndex(%u)", sensorIndex);
        ret = false;
    }
    return ret;
}

auto
FeatureSettingPolicy::
updateRequestResultParams(
    std::shared_ptr<RequestResultParams> &requestParams,
    MetadataPtr pOutMetaApp_Additional,
    MetadataPtr pOutMetaHal_Additional,
    uint32_t needP1Dma,
    uint32_t sensorIndex,
    MINT64 featureCombination,
    MINT32 requestIndex,
    MINT32 requestCount
) -> bool
{
    //CAM_ULOGM_APILIFE();
    auto sensorNum = mPolicyParams.pPipelineStaticInfo->sensorId.size();
    if (sensorIndex >= sensorNum) {
        MY_LOGE("sensorIndex:%u is out of current open sensor num:%zu", sensorIndex, sensorNum);
        return false;
    }
    auto sensorId = mPolicyParams.pPipelineStaticInfo->sensorId[sensorIndex];
    MY_LOGD_IF(2 <= mbDebug, "updateRequestResultParams for sensorIndex:%u (physical sensorId:%d)", sensorIndex, sensorId);
    MetadataPtr pOutMetaApp = std::make_shared<IMetadata>();
    MetadataPtr pOutMetaHal = std::make_shared<IMetadata>();
    if (pOutMetaApp_Additional.get() != nullptr) {
        *pOutMetaApp += *pOutMetaApp_Additional;
    }
    if (pOutMetaHal_Additional.get() != nullptr) {
        *pOutMetaHal += *pOutMetaHal_Additional;
    }
    //check ISP profile
    MUINT8 ispProfile = 0;
    if (IMetadata::getEntry<MUINT8>(pOutMetaHal.get(), MTK_3A_ISP_PROFILE, ispProfile)) {
        MY_LOGD_IF(2 <= mbDebug, "updated isp profile(%d)", ispProfile);
    }
    else {
        MY_LOGD_IF(2 <= mbDebug, "no updated isp profile in pOutMetaHal");
    }

    //check RAW processed type
    MINT32 rawType = -1;
    if (IMetadata::getEntry<MINT32>(pOutMetaHal.get(), MTK_P1NODE_RAW_TYPE, rawType)) {
        MY_LOGD("indicated rawType(%d) by featureCombination(%#" PRIx64")", rawType, featureCombination);
    }
    else {
        MY_LOGD_IF(2 <= mbDebug, "no indicated MTK_P1NODE_RAW_TYPE in pOutMetaHal");
    }

    // per-frame update MTK_3A_ISP_MDP_TARGET_SIZE
    IMetadata::setEntry<MSize>(pOutMetaHal.get(), MTK_3A_ISP_MDP_TARGET_SIZE, mMdpTargetSize);
    MY_LOGD_IF(2 <= mbDebug, "MdpTargetSize(%dx%d)", mMdpTargetSize.w, mMdpTargetSize.h);

    if (featureCombination != -1) {
        MY_LOGD_IF(2 <= mbDebug, "update featureCombination=%#" PRIx64"", featureCombination);
        if (mPolicyParams.bIsLogicalCam)
            IMetadata::setEntry<MINT64>(pOutMetaHal.get(), MTK_FEATURE_CAPTURE, featureCombination);
        else
            IMetadata::setEntry<MINT64>(pOutMetaHal.get(), MTK_FEATURE_CAPTURE_PHYSICAL, featureCombination);
    }
    if (requestIndex || requestCount) {
        MY_LOGD_IF(2 <= mbDebug, "update MTK_HAL_REQUEST_INDEX=%d, MTK_HAL_REQUEST_COUNT=%d", requestIndex, requestCount);
        IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HAL_REQUEST_INDEX, requestIndex);
        IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HAL_REQUEST_COUNT, requestCount);
    }
    if (CC_UNLIKELY(2 <= mbDebug)) {
        (*pOutMetaApp).dump();
        (*pOutMetaHal).dump();
    }
    const MBOOL isMainSensorIndex = (sensorIndex == mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx);
    if (requestParams.get() == nullptr) {
        MY_LOGD_IF(2 <= mbDebug, "no frame setting, create a new one");
        requestParams = std::make_shared<RequestResultParams>();
        // App Metadata, just main sensor has the app metadata
        if(isMainSensorIndex) {
            requestParams->additionalApp = pOutMetaApp;
        };
        // HAl Metadata
        MY_LOGD_IF(2 <= mbDebug, "new requestParams and resize hal meta size to compatible with sensor index:%u", sensorIndex);
        requestParams->additionalHal.resize(sensorIndex+1, nullptr);
        requestParams->additionalHal[sensorIndex] = pOutMetaHal;
        //P1Dma requirement
        MY_LOGD_IF(2 <= mbDebug, "resize needP1Dma size to compatible with sensor index:%u", sensorIndex);
        requestParams->needP1Dma.resize(sensorIndex+1, 0);
        requestParams->needP1Dma[sensorIndex] |= needP1Dma;
    }
    else {
        MY_LOGD_IF(2 <= mbDebug, "frame setting has been created by previous policy, update it");
        // App Metadata, just main sensor has the app metadata
        if (isMainSensorIndex) {
            if (requestParams->additionalApp.get() != nullptr) {
                MY_LOGD_IF(2 <= mbDebug, "[App] append the setting");
                *(requestParams->additionalApp) += *pOutMetaApp;
            }
            else {
                MY_LOGD_IF(2 <= mbDebug, "no app metadata, use the new one");
                requestParams->additionalApp = pOutMetaApp;
            }
        }
        // HAl Metadata
        MY_LOGD_IF(2 <= mbDebug, "[Hal] metadata size(%zu) %d",
                    requestParams->additionalHal.size(),
                    sensorIndex);
        auto additionalHalSize = requestParams->additionalHal.size();
        if(sensorIndex >= additionalHalSize)
        {
            MY_LOGD_IF(2 <= mbDebug, "resize hal meta size to compatible with sensor index:%u", sensorIndex);
            requestParams->additionalHal.resize(sensorIndex+1, nullptr);
            requestParams->additionalHal[sensorIndex] = pOutMetaHal;
        }
        else if (requestParams->additionalHal[sensorIndex].get() != nullptr) {
            MY_LOGD_IF(2 <= mbDebug, "[Hal] append the setting");
            *(requestParams->additionalHal[sensorIndex]) += *pOutMetaHal;
        }
        else
        {
            requestParams->additionalHal[sensorIndex] = pOutMetaHal;
        }
        //P1Dma requirement
        if (sensorIndex >= requestParams->needP1Dma.size()) {
            MY_LOGD_IF(2 <= mbDebug, "resize needP1Dma size to compatible with sensor index:%u", sensorIndex);
            requestParams->needP1Dma.resize(sensorIndex+1, 0);
        }
        requestParams->needP1Dma[sensorIndex] |= needP1Dma;
    }
    return true;
}

auto
FeatureSettingPolicy::
queryPolicyState(
    Policy::State& state,
    uint32_t sensorIndex,
    ParsedStrategyInfo const& parsedInfo,
    RequestOutputParams const* out,
    RequestInputParams const* in
) -> bool
{
    auto sensorNum = mPolicyParams.pPipelineStaticInfo->sensorId.size();
    if (sensorIndex >= sensorNum) {
        MY_LOGE("sensorIndex:%u is out of current open sensor num:%zu", sensorIndex, sensorNum);
        return false;
    }
    int32_t sensorId = mPolicyParams.pPipelineStaticInfo->sensorId[sensorIndex];

    if (in && in->pRequest_AppImageStreamInfo) {
        // provide the App request image buffers type and purpose
        // for feature plugins execute suitable behavior
        state.pParsedAppImageStreamInfo = in->pRequest_AppImageStreamInfo;
    }
    else {
        MY_LOGW("in->pRequest_AppImageStreamInfo is invalid nullptr");
    }

    state.mZslPoolReady    = parsedInfo.isZslModeOn;
    state.mZslRequest      = parsedInfo.isZslFlowOn;
    state.mFlashFired      = parsedInfo.isFlashOn;
    state.mExposureTime    = parsedInfo.exposureTime;
    state.mRealIso         = parsedInfo.realIso;
    state.mAppManual3A     = parsedInfo.isAppManual3A;
    state.mFreeMemoryMBytes = parsedInfo.freeMemoryMBytes;
    state.mSensorId         = sensorId;
    MY_LOGD("policy sensor index %d", sensorId);
    // check manual 3A setting.
    if (out->mainFrame.get()) {
        IMetadata const* pAppMeta = out->mainFrame->additionalApp.get();
        MUINT8 aeMode = MTK_CONTROL_AE_MODE_ON;
        if (IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, aeMode) &&
            aeMode  == MTK_CONTROL_AE_MODE_OFF)
        {
            MINT32 manualIso = 0;
            MINT64 manualExposureTime = 0;
            if (IMetadata::getEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, manualIso) &&
                IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, manualExposureTime) &&
                manualIso > 0 && manualExposureTime > 0)
            {
                state.mRealIso        = manualIso;
                state.mExposureTime   = manualExposureTime/1000; //ns to us
                // not support Zsl for manual AE
                state.mZslPoolReady = false;
                state.mZslRequest   = false;
                MY_LOGI("capture frame setting has been set as manual iso(%u -> %u)/exposure(%u -> %u)us by previous pluign, and must set Zsl off",
                        parsedInfo.realIso, state.mRealIso, parsedInfo.exposureTime, state.mExposureTime);
            }
            else {
                MY_LOGD("it is not manual iso(%d)/exposure(%" PRId64 " ns)", manualIso, manualExposureTime);
            }
        }
        else {
            MY_LOGD_IF(mbDebug, "it is not manual ae mode(%d)", aeMode);
        }
    }
    else {
        MY_LOGD_IF(mbDebug, "no need to check mainFram info(%p)", out->mainFrame.get());
    }

    // get sensor info (the info is after reconfigure if need)
    state.mSensorMode   = out->sensorMode[sensorIndex];
    uint32_t needP1Dma = 0;
    if (!getCaptureP1DmaConfig(needP1Dma, in, sensorIndex) ){
        MY_LOGE("P1Dma output is invalid: 0x%X", needP1Dma);
        return false;
    }
    HwInfoHelper helper(sensorId);
    if (!helper.updateInfos()) {
        MY_LOGE("HwInfoHelper cannot properly update infos");
        return false;
    }
    //
    uint32_t pixelMode = 0;
    MINT32 sensorFps = 0;
    if (!helper.getSensorSize(state.mSensorMode, state.mSensorSize) ||
        !helper.getSensorFps(state.mSensorMode, sensorFps) ||
        !helper.queryPixelMode(state.mSensorMode, sensorFps, pixelMode)) {
        MY_LOGE("cannot get params about sensor");
        return false;
    }
    //
    int32_t bitDepth = 10;
    helper.getRecommendRawBitDepth(bitDepth);
    //
    MINT format = 0;
    size_t stride = 0;
    if (needP1Dma & P1_IMGO) {
        // use IMGO as source for capture
        if (!helper.getImgoFmt(bitDepth, format) ||
            !helper.alignPass1HwLimitation(pixelMode, format, true/*isImgo*/, state.mSensorSize, stride) ) {
            MY_LOGE("cannot query raw buffer info about imgo");
            return false;
        }
    }
    else {
        // use RRZO as source for capture
        auto rrzoSize = (*(in->pConfiguration_StreamInfo_P1))[sensorIndex].pHalImage_P1_Rrzo->getImgSize();
        MY_LOGW("no IMGO buffer, use RRZO size(%d, %d) as capture source image (for better quality, not suggest to use RRZO to capture)",
                rrzoSize.w, rrzoSize.h);
    }

    // get dualcam state info
    state.mMultiCamFeatureMode = miMultiCamFeatureMode;
    const MBOOL isDualCamVSDoFMode = (state.mMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF);
    if (isDualCamVSDoFMode) {
        StereoSizeProvider::getInstance()->getDualcamP2IMGOYuvCropResizeInfo(
                sensorId,
                state.mDualCamDedicatedFov,
                state.mDualCamDedicatedYuvSize);
        MY_LOGD("dualcam indicated P2A output yuv(sensorId:%d, FOV(x:%d,y:%d,w:%d,h:%d), yuv size(%dx%d))",
            sensorId,
            state.mDualCamDedicatedFov.p.x, state.mDualCamDedicatedFov.p.y,
            state.mDualCamDedicatedFov.s.w, state.mDualCamDedicatedFov.s.h,
            state.mDualCamDedicatedYuvSize.w, state.mDualCamDedicatedYuvSize.h);
    } else {
        MY_LOGD("multicam feature mode, sensorId:%d, mode:%d", sensorId, state.mMultiCamFeatureMode);
    }

    MY_LOGD("zslPoolReady:%d, zslRequest:%d, flashFired:%d, appManual3A(%d), exposureTime:%u, realIso:%u",
        state.mZslPoolReady, state.mZslRequest, state.mFlashFired, state.mAppManual3A, state.mExposureTime, state.mRealIso);
    MY_LOGD("sensor(Id:%d, mode:%u, size(%d, %d))",
        sensorId, state.mSensorMode,
        state.mSensorSize.w, state.mSensorSize.h);
    return true;
}

auto
FeatureSettingPolicy::
updatePolicyDecision(
    RequestOutputParams* out,
    uint32_t sensorIndex,
    Policy::Decision const& decision,
    RequestInputParams const* in __unused
) -> bool
{
    // update Zsl requirement
    out->needZslFlow = decision.mZslEnabled;
    out->zslPolicyParams.mPolicy    |= decision.mZslPolicy.mPolicy;
    out->zslPolicyParams.mTimestamp = decision.mZslPolicy.mTimestamp;
    out->zslPolicyParams.mTimeouts  = decision.mZslPolicy.mTimeouts;
    if (out->needZslFlow) {
        MY_LOGD("update needZslFlow(%d), zsl policy(0x%X), timestamp:%" PRId64 ", timeouts:%" PRId64 "",
                out->needZslFlow, out->zslPolicyParams.mPolicy,
                out->zslPolicyParams.mTimestamp, out->zslPolicyParams.mTimeouts);
    }
    if (decision.mSensorMode != SENSOR_SCENARIO_ID_UNNAMED_START) {
        if (sensorIndex >= out->sensorMode.size()) {
            MY_LOGW("default sensorMode size(%zu) is not compatible with sensor index:%u",
                    out->sensorMode.size(), sensorIndex);
            return false;
        }
        out->sensorMode[sensorIndex] = decision.mSensorMode;
        MY_LOGD("feature request sensorMode:%d", decision.mSensorMode);
    }

    // update Raw buffer format requirement
    if (CC_UNLIKELY(decision.mNeedUnpackRaw == true)) {
        bool originalNeedUnpackRaw = out->needUnpackRaw;
        out->needUnpackRaw |= decision.mNeedUnpackRaw;
        MY_LOGI("set needUnpackRaw(%d|%d => %d)", originalNeedUnpackRaw, decision.mNeedUnpackRaw, out->needUnpackRaw);
    }

    out->keepZslBuffer = false;
    // update setting for boostControl
    updateBoostControl(out, decision.mBoostControl);

    return true;
}

template <typename T>
auto
FeatureSettingPolicy::
syncMasterMetaToSlave(
    MetadataPtr masterMeta,
    MetadataPtr slaveMeta,
    IMetadata::Tag_t tag
) -> void
{
    T val = 0;
    if (IMetadata::getEntry<T>(masterMeta.get(), tag, val)) {
        MY_LOGD("update additional meta %X(%d)", tag, val);
        IMetadata::setEntry<T>(slaveMeta.get(), tag, val);
    } else {
        MY_LOGD("cannot find tag %X", tag);
    }
}

auto
FeatureSettingPolicy::
updateDualCamRequestOutputParams(
    RequestOutputParams* out,
    MetadataPtr pOutMetaApp_Additional,
    MetadataPtr pOutMetaHal_Additional,
    uint32_t mainCamP1Dma,
    uint32_t sub1CamP1Dma,
    MINT64 featureCombination,
    unordered_map<uint32_t, MetadataPtr>& halMetaPtrList,
    RequestInputParams const* in
) -> bool
{
    CAM_ULOGM_APILIFE();

    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    const MINT32 subSensorIndex = mSensorInfoList[SENSOR_INDEX_SUB1].sensorIdx;

    const MBOOL isZslFlow = (out->needZslFlow == MTRUE);
    if (isZslFlow) {
        out->zslPolicyParams.mPolicy |= eZslPolicy_DualFrameSync;
    }
    //
    BoostControl boostControl;
    boostControl.boostScenario = IScenarioControlV3::Scenario_ContinuousShot;
    FEATURE_CFG_ENABLE_MASK(boostControl.featureFlag, IScenarioControlV3::FEATURE_STEREO_CAPTURE);
    updateBoostControl(out, boostControl);
    MY_LOGD("update boostControl for DualCam Capture (boostScenario:0x%X, featureFlag:0x%X)",
            boostControl.boostScenario, boostControl.featureFlag);
    auto masterId = in->pMultiCamReqOutputParams->masterId;
    MetadataPtr pOutMetaHal_Additional_Master = std::make_shared<IMetadata>();
    *pOutMetaHal_Additional_Master += *pOutMetaHal_Additional;
    {
        auto iter = halMetaPtrList.find(masterId);
        if(iter != halMetaPtrList.end())
        {
            *pOutMetaHal_Additional_Master += *iter->second;
        }
    }
    MetadataPtr pOutMetaHal_Additional_Slave = std::make_shared<IMetadata>();
    *pOutMetaHal_Additional_Slave += *pOutMetaHal_Additional;
    *pOutMetaHal_Additional_Slave += *out->mainFrame->additionalHal[mainSensorIndex];
    {
        for(auto&& item:in->pMultiCamReqOutputParams->streamingSensorList)
        {
            if(item != masterId)
            {
                auto iter = halMetaPtrList.find(item);
                if(iter != halMetaPtrList.end())
                {
                    *pOutMetaHal_Additional_Slave += *iter->second;
                    break;
                }
            }
        }
    }
    // update mainFrame
    // main1 mainFrame
    updateRequestResultParams(
        out->mainFrame,
        pOutMetaApp_Additional,
        pOutMetaHal_Additional_Master,
        mainCamP1Dma,
        mainSensorIndex,
        featureCombination);
    // main2 mainFrame
    updateRequestResultParams(
        out->mainFrame,
        nullptr, // sub sensor no need to set app metadata
        pOutMetaHal_Additional_Slave, // duplicate main1 metadata
        sub1CamP1Dma,
        subSensorIndex,
        featureCombination);
    // update subFrames
    MY_LOGD("update subFrames size(%zu)", out->subFrames.size());
    for (size_t i = 0; i < out->subFrames.size(); i++) {
        auto subFrame = out->subFrames[i];
        if (subFrame.get()) {
            MY_LOGI("subFrames[%zu] has existed(addr:%p)", i,subFrame.get());
            // main1 subFrame
            updateRequestResultParams(
                subFrame,
                pOutMetaApp_Additional,
                pOutMetaHal_Additional_Master,
                mainCamP1Dma,
                mainSensorIndex,
                featureCombination);
            // main2 subFrame
            updateRequestResultParams(
                subFrame,
                nullptr, // sub sensor no need to set app metadata
                pOutMetaHal_Additional_Slave, // duplicate main1 metadata
                sub1CamP1Dma,
                subSensorIndex,
                featureCombination);
        }
        else {
            MY_LOGE("subFrames[%zu] is invalid", i);
        }

    }
    // update preDummyFrames
    MY_LOGD("update preDummyFrames size(%zu)", out->preDummyFrames.size());
    for (size_t i = 0; i < out->preDummyFrames.size(); i++) {
        auto preDummyFrame = out->preDummyFrames[i];
        if (preDummyFrame.get()) {
            MY_LOGE("preDummyFrames[%zu] has existed(addr:%p)",i, preDummyFrame.get());

            MetadataPtr pOutMetaHal_Additional_Slave_PreDummy = std::make_shared<IMetadata>();
            *pOutMetaHal_Additional_Slave_PreDummy += *pOutMetaHal_Additional_Slave;
            syncMasterMetaToSlave<MBOOL>(preDummyFrame->additionalHal[mainSensorIndex], pOutMetaHal_Additional_Slave_PreDummy, MTK_3A_DUMMY_BEFORE_REQUEST_FRAME);
            // main1 subFrame
            updateRequestResultParams(
                preDummyFrame,
                pOutMetaApp_Additional,
                pOutMetaHal_Additional_Master,
                mainCamP1Dma,
                mainSensorIndex,
                featureCombination);
            // main2 subFrame
            updateRequestResultParams(
                preDummyFrame,
                nullptr, // sub sensor no need to set app metadata
                pOutMetaHal_Additional_Slave_PreDummy, // duplicate main1 metadata
                sub1CamP1Dma,
                subSensorIndex,
                featureCombination);
        }
        else {
            MY_LOGE("preDummyFrames[%zu] is invalid", i);
        }
    }
    // update postDummyFrames
    MY_LOGD("update postDummyFrames size(%zu)", out->postDummyFrames.size());
    for (size_t i = 0; i < out->postDummyFrames.size(); i++) {
        auto postDummyFrame = out->postDummyFrames[i];
        if (postDummyFrame.get()) {
            MY_LOGI("postDummyFrames[%zu] has existed(addr:%p)", i, postDummyFrame.get());

            MetadataPtr pOutMetaHal_Additional_Slave_PostDummy = std::make_shared<IMetadata>();
            *pOutMetaHal_Additional_Slave_PostDummy += *pOutMetaHal_Additional_Slave;
            syncMasterMetaToSlave<MBOOL>(postDummyFrame->additionalHal[mainSensorIndex], pOutMetaHal_Additional_Slave_PostDummy, MTK_3A_DUMMY_AFTER_REQUEST_FRAME);

            // main1 subFrame
            updateRequestResultParams(
                postDummyFrame,
                pOutMetaApp_Additional,
                pOutMetaHal_Additional_Master,
                mainCamP1Dma,
                mainSensorIndex,
                featureCombination);
            // main2 subFrame
            updateRequestResultParams(
                postDummyFrame,
                nullptr, // sub sensor no need to set app metadata
                pOutMetaHal_Additional_Slave_PostDummy, // duplicate main1 metadata
                sub1CamP1Dma,
                subSensorIndex,
                featureCombination);
        }
        else
        {
            MY_LOGE("postDummyFrames[%zu] is invalid", i);
        }
    }
    return true;
};

auto
FeatureSettingPolicy::
updateRequestOutputAllFramesMetadata(
    RequestOutputParams* out,
    MetadataPtr pOutMetaApp_Additional,
    MetadataPtr pOutMetaHal_Additional
) -> bool
{
    if (out->mainFrame.get() == nullptr) {
        MY_LOGW("no mainFrame could be updated!!");
        return false;
    }
    //
    // update mainFrame
    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    for (size_t sensorIndex = 0; sensorIndex < out->mainFrame->additionalHal.size(); sensorIndex++) {
        MY_LOGD_IF(mbDebug, "update mainFrame, sensor index:%zu", sensorIndex);
        updateRequestResultParams(
            out->mainFrame,
            (sensorIndex == mainSensorIndex) ? pOutMetaApp_Additional : nullptr, // sub sensor no need to set app metadata
            pOutMetaHal_Additional,
            0,
            sensorIndex);
    }
    // update subFrames
    MY_LOGD_IF(mbDebug, "update subFrames size(%zu)", out->subFrames.size());
    for (size_t i = 0; i < out->subFrames.size(); i++) {
        auto& subFrame = out->subFrames[i];
        if (subFrame.get()) {
            MY_LOGD_IF(mbDebug, "subFrames[%zu] has existed(addr:%p)", i, subFrame.get());
            for (size_t sensorIndex = 0; sensorIndex < subFrame->additionalHal.size(); sensorIndex++) {
                MY_LOGD_IF(mbDebug, "update subFrame, sensor index:%zu", sensorIndex);
                updateRequestResultParams(
                    subFrame,
                    (sensorIndex == mainSensorIndex) ? pOutMetaApp_Additional : nullptr, // sub sensor no need to set app metadata
                    pOutMetaHal_Additional,
                    0,
                    sensorIndex);
            }
        }
        else {
            MY_LOGW("subFrames[%zu] is invalid", i);
        }
    }
    //
    return true;
}

auto
FeatureSettingPolicy::
strategyMultiFramePlugin(
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> bool
{
    auto provider = mMFPPluginWrapperPtr->getProvider(combinedKeyFeature, foundFeature);
    if (provider) {
        // for MultiFramePlugin key feature (ex: HDR, MFNR, 3rd party multi-frame algo,etc )
        // negotiate and query feature requirement
        auto pAppMetaControl = in->pRequest_AppControl;
        auto property =  provider->property();
        auto pSelection = mMFPPluginWrapperPtr->createSelection();
        const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
        MFP_Selection& sel = *pSelection;
        sel.mRequestIndex = 0;
        mMFPPluginWrapperPtr->offer(sel);
        //
        if (parsedInfo.ispTuningHint >= 0 && parsedInfo.ispTuningFrameCount <= 1) {
            MY_LOGD("isp hidl tuning request(tuning hint:%d), but frame count is %d, no need to check requirement for multiframe feature(%s)",
                    parsedInfo.ispTuningHint, parsedInfo.ispTuningFrameCount, property.mName);
            return false;
        }
        //
        uint32_t mainCamP1Dma = 0;
        if ( !getCaptureP1DmaConfig(mainCamP1Dma, in, mainSensorIndex, property.mNeedRrzoBuffer) ){
            MY_LOGE("main P1Dma output is invalid: 0x%X", mainCamP1Dma);
            return false;
        }
        // check the requirement of rrzo by provider's request
        MY_LOGI_IF(property.mNeedRrzoBuffer, "feature(%s) request rrzo dma buffer, P1Dma(0x%X)", property.mName, mainCamP1Dma);

        // update app metadata for plugin reference
        MetadataPtr pInMetaApp = std::make_shared<IMetadata>(*pAppMetaControl);
        sel.mIMetadataApp.setControl(pInMetaApp);
        // update previous Hal ouput for plugin reference
        if (out->mainFrame.get()) {
            auto pHalMeta = out->mainFrame->additionalHal[mainSensorIndex];
            if (pHalMeta) {
                MetadataPtr pInMetaHal = std::make_shared<IMetadata>(*pHalMeta);
                sel.mIMetadataHal.setControl(pInMetaHal);
            }
        }
        if (parsedInfo.ispTuningHint >= 0) {
            // if request frame from camera for isp hidl reprocessing.
            sel.mIspHidlStage = ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA;
        }
        // query state  for plugin provider negotiate
        if (!queryPolicyState(
                    sel.mState,
                    mainSensorIndex,
                    parsedInfo, out, in)) {
            MY_LOGE("cannot query state for plugin provider negotiate!");
            return false;
        }
        if (provider->negotiate(sel) == OK && sel.mRequestCount > 0) {
            MY_LOGD("MultiFrame request count : %d", sel.mRequestCount);
            if (!updatePolicyDecision( out, mainSensorIndex, sel.mDecision, in)) {
                MY_LOGW("update config info failed!");
                return false;
            }
            if (CC_LIKELY(sel.mDecision.mProcess)) {
                pSelection->mTokenPtr = MFP_Selection::createToken(mUniqueKey, in->requestNo, 0, sel.mState.mSensorId, !mPolicyParams.bIsLogicalCam);
                mMFPPluginWrapperPtr->keepSelection(in->requestNo, provider, pSelection);
            }
            else {
                MY_LOGD("%s(%s) bypass process, only decide frames requirement",
                        mMFPPluginWrapperPtr->getName().c_str(), property.mName);
                featureCombination &= ~foundFeature;
            }
            MetadataPtr pOutMetaApp_Additional = sel.mIMetadataApp.getAddtional();
            MetadataPtr pOutMetaHal_Additional = sel.mIMetadataHal.getAddtional();
            updateRequestResultParams(
                    out->mainFrame,
                    pOutMetaApp_Additional,
                    pOutMetaHal_Additional,
                    mainCamP1Dma,
                    mainSensorIndex,
                    featureCombination,
                    0,
                    sel.mRequestCount);

            auto getDummyFrames = [this]
            (
                RequestOutputParams* out,
                MFP_Selection& sel,
                const uint32_t camP1Dma,
                const uint32_t sensorIndex
            ) -> MBOOL
            {
                // get preDummyFrames if the key feature requiresd
                MY_LOGD("preDummyFrames count:%d", sel.mFrontDummy);
                if (sel.mFrontDummy > 0) {
                    for (MINT32 i = 0; i < sel.mFrontDummy; i++) {
                        MetadataPtr pAppDummy_Additional = sel.mIMetadataApp.getDummy();
                        MetadataPtr pHalDummy_Additional = sel.mIMetadataHal.getDummy();
                        IMetadata::setEntry<MBOOL>(pHalDummy_Additional.get(), MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, 1);
                        std::shared_ptr<RequestResultParams> preDummyFrame = nullptr;
                        updateRequestResultParams(
                                preDummyFrame,
                                pAppDummy_Additional,
                                pHalDummy_Additional,
                                camP1Dma,
                                sensorIndex);
                        //
                        out->preDummyFrames.push_back(preDummyFrame);
                    }
                }
                // get postDummyFrames if the key feature requiresd
                MY_LOGD("postDummyFrames count:%d", sel.mPostDummy);
                if (sel.mPostDummy > 0) {
                    for (MINT32 i = 0; i < sel.mPostDummy; i++) {
                        MetadataPtr pAppDummy_Additional = sel.mIMetadataApp.getDummy();
                        MetadataPtr pHalDummy_Additional = sel.mIMetadataHal.getDummy();
                        IMetadata::setEntry<MBOOL>(pHalDummy_Additional.get(), MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
                        std::shared_ptr<RequestResultParams> postDummyFrame = nullptr;
                        updateRequestResultParams(
                                postDummyFrame,
                                pAppDummy_Additional,
                                pHalDummy_Additional,
                                camP1Dma,
                                sensorIndex);
                        //
                        out->postDummyFrames.push_back(postDummyFrame);
                    }
                }
                return true;
            };
            // get the dummy frames if the first main negotiate return the front/rear dummy info.
            getDummyFrames(out, sel, mainCamP1Dma, mainSensorIndex);
            for (uint32_t i = 1; i < sel.mRequestCount; i++)
            {
                auto pSubSelection = mMFPPluginWrapperPtr->createSelection();
                if (CC_LIKELY(sel.mDecision.mProcess)) {
                    pSubSelection->mTokenPtr = MFP_Selection::createToken(mUniqueKey, in->requestNo, i, sel.mState.mSensorId, !mPolicyParams.bIsLogicalCam);
                    mMFPPluginWrapperPtr->keepSelection(in->requestNo, provider, pSubSelection);
                }
                else {
                    MY_LOGD("%s(%s) bypass process, only decide frames requirement",
                            mMFPPluginWrapperPtr->getName().c_str(), property.mName);
                    featureCombination &= ~foundFeature;
                }
                MFP_Selection& subsel = *pSubSelection;
                subsel.mState = sel.mState;
                subsel.mRequestIndex = i;
                mMFPPluginWrapperPtr->offer(subsel);
                subsel.mIMetadataApp.setControl(pInMetaApp);
                // update previous Hal ouput for plugin reference
                if (out->mainFrame.get()) {
                    auto pHalMeta = out->mainFrame->additionalHal[mainSensorIndex];
                    if (pHalMeta) {
                        MetadataPtr pInMetaHal = std::make_shared<IMetadata>(*pHalMeta);
                        sel.mIMetadataHal.setControl(pInMetaHal);
                    }
                }
                provider->negotiate(subsel);
                // add metadata
                pOutMetaApp_Additional = subsel.mIMetadataApp.getAddtional();
                pOutMetaHal_Additional = subsel.mIMetadataHal.getAddtional();
                std::shared_ptr<RequestResultParams> subFrame = nullptr;
                auto subFrameIndex = i - 1;
                if (out->subFrames.size() > subFrameIndex) {
                    MY_LOGI("subFrames size(%zu), subFrames[%d] has existed(addr:%p)",
                            out->subFrames.size(), subFrameIndex, (out->subFrames[subFrameIndex]).get());
                    subFrame = out->subFrames[subFrameIndex];
                    updateRequestResultParams(
                            subFrame,
                            pOutMetaApp_Additional,
                            pOutMetaHal_Additional,
                            mainCamP1Dma,
                            mainSensorIndex,
                            featureCombination,
                            i,
                            sel.mRequestCount);
                    //
                    out->subFrames[i] = subFrame;
                }
                else {
                    MY_LOGD("subFrames size(%zu), no subFrames[%d], must ceate a new one", out->subFrames.size(), subFrameIndex);
                    updateRequestResultParams(
                            subFrame,
                            pOutMetaApp_Additional,
                            pOutMetaHal_Additional,
                            mainCamP1Dma,
                            mainSensorIndex,
                            featureCombination,
                            i,
                            sel.mRequestCount);
                    //
                    out->subFrames.push_back(subFrame);
                }
                // get the dummy frames if the subframes negotiate return the front/rear dummy info.
                getDummyFrames(out, subsel, mainCamP1Dma, mainSensorIndex);
            }
            MY_LOGD("%s(%s), trigger provider(mRequestCount:%d) for foundFeature(%#" PRIx64")",
                    mMFPPluginWrapperPtr->getName().c_str(), property.mName, sel.mRequestCount, foundFeature);
        }
        else {
            MY_LOGD("%s(%s), no need to trigger provider(mRequestCount:%d) for foundFeature(%#" PRIx64")",
                    mMFPPluginWrapperPtr->getName().c_str(), property.mName, sel.mRequestCount, foundFeature);
            return false;
        }
    }
    else
    {
        MY_LOGD_IF(mbDebug, "no provider for multiframe key feature(%#" PRIx64")", combinedKeyFeature);
    }

    return true;
}

auto
FeatureSettingPolicy::
strategySingleRawPlugin(
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> bool
{
    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    auto provider = mRawPluginWrapperPtr->getProvider(combinedKeyFeature, foundFeature);
    if (provider) {
        // for RawPlugin key feature (ex: SW 4Cell) negotiate and query feature requirement
        uint32_t mainCamP1Dma = 0;
        if ( !getCaptureP1DmaConfig(mainCamP1Dma, in, mainSensorIndex) ){
            MY_LOGE("main P1Dma output is invalid: 0x%X", mainCamP1Dma);
            return false;
        }
        auto pAppMetaControl = in->pRequest_AppControl;
        auto property =  provider->property();
        auto pSelection = mRawPluginWrapperPtr->createSelection();
        Raw_Selection& sel = *pSelection;
        mRawPluginWrapperPtr->offer(sel);
        // update app metadata for plugin reference
        MetadataPtr pInMetaApp = std::make_shared<IMetadata>(*pAppMetaControl);
        sel.mIMetadataApp.setControl(pInMetaApp);
        // update previous Hal ouput for plugin reference
        if (out->mainFrame.get()) {
            auto pHalMeta = out->mainFrame->additionalHal[mainSensorIndex];
            if (pHalMeta) {
                MetadataPtr pInMetaHal = std::make_shared<IMetadata>(*pHalMeta);
                sel.mIMetadataHal.setControl(pInMetaHal);
            }
        }
        if (parsedInfo.ispTuningHint >= 0) {
            // if request frame from camera for isp hidl reprocessing.
            sel.mIspHidlStage = ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA;
        }
        // query state  for plugin provider strategy
        MINT32 mainSensorIdx = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
        if (!queryPolicyState(
                    sel.mState,
                    mainSensorIdx,
                    parsedInfo, out, in)) {
            MY_LOGE("cannot query state for plugin provider negotiate!");
            return false;
        }
        if (provider->negotiate(sel) == OK) {
            if (!updatePolicyDecision( out, mainSensorIdx, sel.mDecision, in)) {
                MY_LOGW("update config info failed!");
                return false;
            }
            //
            if (CC_LIKELY(sel.mDecision.mProcess)) {
                pSelection->mTokenPtr = Raw_Selection::createToken(mUniqueKey, in->requestNo, 0, sel.mState.mSensorId, !mPolicyParams.bIsLogicalCam);
                mRawPluginWrapperPtr->keepSelection(in->requestNo, provider, pSelection);
            }
            else {
                MY_LOGD("%s(%s) bypass process, only decide frames requirement",
                        mRawPluginWrapperPtr->getName().c_str(), property.mName);
                featureCombination &= ~foundFeature;
            }
            MetadataPtr pOutMetaApp_Additional = sel.mIMetadataApp.getAddtional();
            MetadataPtr pOutMetaHal_Additional = sel.mIMetadataHal.getAddtional();
            updateRequestResultParams(
                    out->mainFrame,
                    pOutMetaApp_Additional,
                    pOutMetaHal_Additional,
                    mainCamP1Dma,
                    mainSensorIdx,
                    featureCombination);
            //
            MY_LOGD("%s(%s), trigger provider for foundFeature(%#" PRIx64")",
                mRawPluginWrapperPtr->getName().c_str(), property.mName, foundFeature);
        }
        else {
            MY_LOGD("%s(%s), no need to trigger provider for foundFeature(%#" PRIx64")",
                mRawPluginWrapperPtr->getName().c_str(), property.mName, foundFeature);
            return false;
        }
    }
    else
    {
        MY_LOGD_IF(mbDebug, "no provider for single raw key feature(%#" PRIx64")", combinedKeyFeature);
    }

    return true;
}


auto
FeatureSettingPolicy::
strategyDualCamPlugin(
    MINT64 combinedKeyFeature __unused, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo __unused,
    RequestInputParams const* in,
    unordered_map<uint32_t, MetadataPtr>& halMetaPtrList
) -> bool
{
    CAM_ULOGM_APILIFE();
    MINT64 depthKeyFeature = 0;
    MINT64 bokehKeyFeature = 0;
    MINT64 fusionKeyFeature = 0;
    const MBOOL isDualCamVSDoFMode = (miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF);
    if(mDualDevicePath == DualDevicePath::MultiCamControl)
    {
        uint32_t mainCamP1Dma = 0;
        if ( !getCaptureP1DmaConfig(mainCamP1Dma, in, mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx) ) {
            MY_LOGE("main P1Dma output is invalid: 0x%X", mainCamP1Dma);
            return false;
        }
        uint32_t sub1CamP1Dma = 0;
        if ( !getCaptureP1DmaConfig(sub1CamP1Dma, in, mSensorInfoList[SENSOR_INDEX_SUB1].sensorIdx) ) {
            MY_LOGE("sub1 P1Dma output is invalid: 0x%X", sub1CamP1Dma);
            return false;
        }
        // for reprocess flow, main2 dma has to set 0.
        if(in->pRequest_AppImageStreamInfo->pAppImage_Input_Yuv != nullptr) {
            MY_LOGD("multicam yuv reprocess, no needs to connect main2");
            sub1CamP1Dma = 0;
        }

        if(isDualCamVSDoFMode)
        {
            const MBOOL hasFeatureVSDoF = mDepthPluginWraperPtr->isKeyFeatureExisting(featureCombination, depthKeyFeature)
                                        && mBokehPluginWraperPtr->isKeyFeatureExisting(featureCombination, bokehKeyFeature);
            const MBOOL hasFeatureFusion = mFusionPluginWraperPtr->isKeyFeatureExisting(featureCombination, fusionKeyFeature);

            if (hasFeatureVSDoF) {
                MY_LOGD("update DualCam request output params, depth:%#" PRIx64 ", bokeh:%#" PRIx64, depthKeyFeature, bokehKeyFeature);
                // TODO: update additional metadata for depth and bokeh
            }
            else if (hasFeatureFusion) {
                MY_LOGD("update DualCam request output params, fusion:%#" PRIx64, fusionKeyFeature);
                // TODO: update additional metadata for fusion
            }
        }

        // TODO: update additional metadata for dual cam
        MetadataPtr pOutMetaApp_Additional = std::make_shared<IMetadata>();
        MetadataPtr pOutMetaHal_Additional = std::make_shared<IMetadata>();

        updateDualCamRequestOutputParams(
                        out,
                        pOutMetaApp_Additional,
                        pOutMetaHal_Additional,
                        mainCamP1Dma,
                        sub1CamP1Dma,
                        featureCombination,
                        halMetaPtrList,
                        in);
    }
    else
    {
        MY_LOGD("doesn't find any feature, not dualcam(vsdof mode/multicam)");
    }
    //
    foundFeature = depthKeyFeature|bokehKeyFeature|fusionKeyFeature;
    if (foundFeature) {
        MY_LOGD("found feature(%#" PRIx64") for dual cam", foundFeature);
    }
    return true;
}

auto
FeatureSettingPolicy::
strategyNormalSingleCapture(
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> bool
{
    if (out->subFrames.size() > 0) {
        MY_LOGD_IF(mbDebug, "frames setting has been updated by multiframe plugin");
        return true;
    }

    CAM_ULOGM_APILIFE();

    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;

    // general single frame capture's sub feature combination and requirement
    uint32_t mainCamP1Dma = 0;
    if ( !getCaptureP1DmaConfig(mainCamP1Dma, in, mainSensorIndex) ){
        MY_LOGE("main P1Dma output is invalid: 0x%X", mainCamP1Dma);
        return false;
    }
    // zsl policy for general single frame capture
    if (parsedInfo.isZslModeOn && parsedInfo.isZslFlowOn) {
        if (out->needZslFlow == false) {
            // features may force to disable ZSL due to P1/3A exposure setting.
            MY_LOGI("ZSL has been disabled by feature(%#" PRIx64") requirement", combinedKeyFeature);
        }
        else if (parsedInfo.isCShot) {
            MY_LOGD("CShot always trigger ZSL if ZSL enable");
            out->needZslFlow = true;
            out->zslPolicyParams.mPolicy = eZslPolicy_None;
            out->zslPolicyParams.mTimeouts  = 0; //ms
        }
        else if (parsedInfo.isFlashOn || parsedInfo.isAppManual3A) {
            MY_LOGD("not support Zsl due to (isFlashOn:%d, isManual3A:%d, isZslModeOn:%d, isZslFlowOn:%d)",
                    parsedInfo.isFlashOn, parsedInfo.isAppManual3A, parsedInfo.isZslModeOn, parsedInfo.isZslFlowOn);
            out->needZslFlow = false;
            out->zslPolicyParams.mPolicy = eZslPolicy_None;
        }
        else if (parsedInfo.isRawReprocess || parsedInfo.isYuvReprocess) {
            if (out->needZslFlow) {
                MY_LOGW("not support zsl(%d) for raw/yuv reprocess, forced disable zsl", out->needZslFlow);
                out->needZslFlow = false;
                out->zslPolicyParams.mPolicy = eZslPolicy_None;
            }
        }
        else if (parsedInfo.ispTuningHint >= 0 && out->zslPolicyParams.mPolicy == eZslPolicy_None ) {  // if request Zsl frame for isp hidl reprocessing.
            out->needZslFlow = true;
            if (parsedInfo.ispTuningFrameCount > 0) { // app request frames for multiframe reprocessing
                out->zslPolicyParams.mPolicy |= eZslPolicy_AfState|eZslPolicy_ZeroShutterDelay|eZslPolicy_ContinuousFrame;
                MY_LOGD("app request frames(count:%d, index:%d) for multiframe reprocessing",
                        parsedInfo.ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
            }
            else {
                out->zslPolicyParams.mPolicy |= eZslPolicy_AfState|eZslPolicy_ZeroShutterDelay;
            }
        }
        else if (out->zslPolicyParams.mPolicy == eZslPolicy_None) {
            out->needZslFlow = true;
            out->zslPolicyParams.mPolicy |= eZslPolicy_AfState|eZslPolicy_ZeroShutterDelay;
        }
        else {
            MY_LOGI("zslPolicyParams has been set by plugins (mPolicy:0x%X, mTimeouts:%" PRId64 "",
                    out->zslPolicyParams.mPolicy, out->zslPolicyParams.mTimeouts);
        }
    }
    else {
        MY_LOGD("not support Zsl due to (isFlashOn:%d, isManual3A:%d, isZslModeOn:%d, isZslFlowOn:%d)",
            parsedInfo.isFlashOn, parsedInfo.isAppManual3A, parsedInfo.isZslModeOn, parsedInfo.isZslFlowOn);
        out->needZslFlow = false;
        out->zslPolicyParams.mPolicy = eZslPolicy_None;
    }
    //
    if (parsedInfo.isCShot && !parsedInfo.isFastCapture) {
        out->bCshotRequest = true;
        // boot scenario for CShot.
        BoostControl boostControl;
        boostControl.boostScenario = IScenarioControlV3::Scenario_ContinuousShot;
        updateBoostControl(out, boostControl);
        MY_LOGD("update boostControl for CShot (boostScenario:0x%X, featureFlag:0x%X)",
                boostControl.boostScenario, boostControl.featureFlag);
    }

    MetadataPtr pOutMetaApp = nullptr;
    if(parsedInfo.isFastCapture) {
        MY_LOGD("request is FastCapture, will be seen as CShot");
        pOutMetaApp = std::make_shared<IMetadata>();
        IMetadata::setEntry<MINT32>(pOutMetaApp.get(), MTK_CSHOT_FEATURE_CAPTURE, MTK_CSHOT_FEATURE_CAPTURE_ON);
    }

    MetadataPtr pOutMetaHal = std::make_shared<IMetadata>();
    if (parsedInfo.isYuvReprocess) {
        MY_LOGD("set MTK_HAL_REQUEST_IS_YUVREPROCESS to %d", parsedInfo.isYuvReprocess);
        IMetadata::setEntry<MBOOL>(pOutMetaHal.get(), MTK_HAL_REQUEST_IS_YUVREPROCESS, parsedInfo.isYuvReprocess);
    }

    // update request result (frames metadata)
    updateRequestResultParams(
        out->mainFrame,
        pOutMetaApp, /* no additional metadata from provider*/
        pOutMetaHal, /* no additional metadata from provider*/
        mainCamP1Dma,
        mainSensorIndex,
        featureCombination);

    MY_LOGD_IF(mbDebug, "trigger single frame feature:%#" PRIx64", feature combination:%#" PRIx64"",
            combinedKeyFeature, featureCombination);
    return true;
}

auto
FeatureSettingPolicy::
dumpRequestOutputParams(
    RequestOutputParams* out,
    RequestInputParams const* in,
    bool forcedEnable = false
) -> bool
{
    // TODO: refactoring for following code
    if (CC_UNLIKELY(in->needP2CaptureNode || in->needRawOutput || forcedEnable)) {
        CAM_ULOGM_APILIFE();
        MY_LOGD("req#:%u, needP2S(%d), needP2C(%d), needRawOut(%d)", in->requestNo, in->needP2StreamNode, in->needP2CaptureNode, in->needRawOutput);
        // dump sensor mode
        for (unsigned int i=0; i<out->sensorMode.size(); i++) {
            MY_LOGD("sensor(index:%d): sensorMode(%d)", i, out->sensorMode[i]);
        }

        // dump boostControl setting
        for (auto& control : out->vboostControl) {
            MY_LOGD("current boostControl(size:%zu): boostScenario(0x%X), featureFlag(0x%X)",
                    out->vboostControl.size(), control.boostScenario, control.featureFlag);
        }

        // dump frames count and needUnpackRaw info
        MY_LOGD("request frames count(mainFrame:%d, subFrames:%zu, preDummyFrames:%zu, postDummyFrames:%zu), needUnpackRaw(%d)",
                (out->mainFrame.get() != nullptr), out->subFrames.size(),
                out->preDummyFrames.size(), out->postDummyFrames.size(),
                out->needUnpackRaw);

        // dump mainFrame
        if(out->mainFrame.get()) {
            //
            for(size_t index = 0; index < out->mainFrame->needP1Dma.size(); index++) {
                MY_LOGD("mainFrame: needP1Dma, index:%zu, value:0x%X", index, out->mainFrame->needP1Dma[index]);
            }
            //
            if (CC_UNLIKELY(mbDebug)) {
                for(size_t index = 0; index < out->mainFrame->additionalHal.size(); index++) {
                    MY_LOGD("mainFrame: dump addition hal metadata for index:%zu, count:%u", index, out->mainFrame->additionalHal[index]->count());
                    out->mainFrame->additionalHal[index]->dump();

                    // dump MTK_FEATURE_CAPTURE/MTK_FEATURE_CAPTURE_PHYSICAL info for capture
                    if (CC_UNLIKELY(in->needP2CaptureNode || in->needRawOutput)) {
                        bool ret = false;
                        MINT64 featureCombination = 0;
                        if (mPolicyParams.bIsLogicalCam)
                            ret = IMetadata::getEntry<MINT64>(out->mainFrame->additionalHal[index].get(), MTK_FEATURE_CAPTURE, featureCombination);
                        else
                            ret = IMetadata::getEntry<MINT64>(out->mainFrame->additionalHal[index].get(), MTK_FEATURE_CAPTURE_PHYSICAL, featureCombination);
                        if (ret) {
                            MY_LOGD("mainFrame %s featureCombination=%#" PRIx64"",
                                    (mPolicyParams.bIsLogicalCam ? "MTK_FEATURE_CAPTURE" : "MTK_FEATURE_CAPTURE_PHYSICAL"),
                                    featureCombination);
                        }
                        else {
                            MY_LOGW("mainFrame w/o featureCombination");
                        }
                    }
                }
                MY_LOGD("dump addition app metadata");
                out->mainFrame->additionalApp->dump();
            }
        }
        else
        {
            MY_LOGW("failed to get main frame");
        }

        // dump subFrames
        if (CC_UNLIKELY(mbDebug)) {
            for (size_t i = 0; i < out->subFrames.size(); i++) {
                auto subFrame = out->subFrames[i];
                if (subFrame.get()) {
                    //
                    for(size_t index = 0; index < subFrame->needP1Dma.size(); index++) {
                        MY_LOGD("subFrame[%zu]: needP1Dma, index:%zu, value:0x%X", i, index, subFrame->needP1Dma[index]);
                    }
                    //
                    for(size_t index = 0; index < subFrame->additionalHal.size(); index++) {
                        MY_LOGD("subFrame[%zu]: dump addition hal metadata for index:%zu, count:%u", i, index, subFrame->additionalHal[index]->count());
                        subFrame->additionalHal[index]->dump();
                        // dump MTK_FEATURE_CAPTURE/MTK_FEATURE_CAPTURE_PHYSICAL info for capture
                        if (CC_UNLIKELY(in->needP2CaptureNode || in->needRawOutput)) {
                            bool ret = false;
                            MINT64 featureCombination = 0;
                            if (mPolicyParams.bIsLogicalCam)
                                ret = IMetadata::getEntry<MINT64>(subFrame->additionalHal[index].get(), MTK_FEATURE_CAPTURE, featureCombination);
                            else
                                ret = IMetadata::getEntry<MINT64>(subFrame->additionalHal[index].get(), MTK_FEATURE_CAPTURE_PHYSICAL, featureCombination);
                            if (ret) {
                                MY_LOGD("subFrame[%zu]: featureCombination=%#" PRIx64"", i, featureCombination);
                            }
                            else {
                                MY_LOGW("subFrame[%zu]: w/o featureCombination=%#" PRIx64"", i, featureCombination);
                            }
                        }
                    }
                    MY_LOGD("subFrame[%zu]: dump addition app metadata", i);
                    out->mainFrame->additionalApp->dump();
                }
            }
        }

        // reconfig & zsl info.
        MY_LOGD("needReconfiguration:%d, zsl(need:%d, policy:0x%X, timestamp:%" PRId64 ", timeouts:%" PRId64 ")",
                out->needReconfiguration, out->needZslFlow, out->zslPolicyParams.mPolicy, out->zslPolicyParams.mTimestamp, out->zslPolicyParams.mTimeouts);
    }
    return true;
}

auto
FeatureSettingPolicy::
updatePluginSelection(
    const uint32_t requestNo,
    bool isFeatureTrigger,
    uint8_t frameCount
) -> bool
{
    if (isFeatureTrigger) {
        mMFPPluginWrapperPtr->pushSelection(requestNo, frameCount);
        mRawPluginWrapperPtr->pushSelection(requestNo, frameCount);
    }
    else {
        mMFPPluginWrapperPtr->cancel();
        mRawPluginWrapperPtr->cancel();
    }

    return true;
}

auto
FeatureSettingPolicy::
strategyCaptureFeature(
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> bool
{
    CAM_ULOGM_APILIFE();
    MY_LOGD("strategy for combined key feature(%#" PRIx64"), feature combination(%#" PRIx64")",
            combinedKeyFeature, featureCombination);

    if (CC_UNLIKELY(mForcedKeyFeatures >= 0)) {
        combinedKeyFeature = mForcedKeyFeatures;
        MY_LOGW("forced key feature(%#" PRIx64")", combinedKeyFeature);
    }
    if (CC_UNLIKELY(mForcedFeatureCombination >= 0)) {
        featureCombination = mForcedFeatureCombination;
        MY_LOGW("forced feature combination(%#" PRIx64")", featureCombination);
    }

    RequestOutputParams temp_out;
    if (out->mainFrame.get()) {
        MY_LOGI("clear previous invalid frames setting");
        out->mainFrame = nullptr;
        out->subFrames.clear();
        out->preDummyFrames.clear();
        out->postDummyFrames.clear();
    }
    temp_out = *out;
    //
    MINT64 foundFeature = 0;
    auto const& sensorList = in->pMultiCamReqOutputParams->prvStreamingSensorList;
    //
    if (parsedInfo.isZslModeOn && parsedInfo.isZslFlowOn) {
        // Set ZSL decison default value by App ZSL config/request flag,
        // but features may force to disable ZSL due to P1/3A exposure setting,
        // the final capability will be confirmed by features.
        temp_out.needZslFlow = true;
        MY_LOGD("App request ZSL(config mode:%d, request flow:%d) for capture", parsedInfo.isZslModeOn, parsedInfo.isZslFlowOn);
    }
    //
    if (combinedKeyFeature) { /* not MTK_FEATURE_NORMAL */
        MINT64 checkFeatures = combinedKeyFeature;
        //
        do {
            if (!strategySingleRawPlugin(checkFeatures, featureCombination, foundFeature, &temp_out, parsedInfo, in)) {
                MY_LOGD("no need to trigger feature(%#" PRIx64") for features(key:%#" PRIx64", combined:%#" PRIx64")",
                        foundFeature, combinedKeyFeature, featureCombination);
                return false;
            }
            checkFeatures &= ~foundFeature;
        } while (foundFeature && checkFeatures); // to find next raw plugin until no foundfeature(==0)
        //
        MY_LOGD_IF(checkFeatures, "continue to find next plugin for %#" PRIx64"", checkFeatures);
        //
        if (!strategyMultiFramePlugin(checkFeatures, featureCombination, foundFeature, &temp_out, parsedInfo, in)) {
            MY_LOGD("no need to trigger feature(%#" PRIx64") for features(key:%#" PRIx64", combined:%#" PRIx64")",
                    foundFeature, combinedKeyFeature, featureCombination);
            return false;
        }
        checkFeatures &= ~foundFeature;
        //
        if (checkFeatures) {
            MY_LOGD("some key features(%#" PRIx64") still not found for features(%#" PRIx64")",
                    checkFeatures, combinedKeyFeature);
            return false;
        }
    }
    else {
        MY_LOGD("no combinated key feature, use default normal single capture");
    }
    // update basic requirement
    if (!strategyNormalSingleCapture(combinedKeyFeature, featureCombination, &temp_out, parsedInfo, in)) {
        MY_LOGW("update capture setting failed!");
        return false;
    }
    //
    if (parsedInfo.isCShot || parsedInfo.isYuvReprocess || parsedInfo.isRawReprocess) {
        MY_LOGD("no need dummy frames, isCShot(%d), isYuvReprocess(%d), isRawReprocess(%d)",
                parsedInfo.isCShot, parsedInfo.isYuvReprocess, parsedInfo.isRawReprocess);
    }
    else { // check and update dummy frames requirement for perfect 3A stable...
        updateCaptureDummyFrames(combinedKeyFeature, &temp_out, parsedInfo, in);
    }
    //
    if (!updateCaptureDebugInfo(&temp_out, parsedInfo, in)) {
        MY_LOGW("updateCaptureDebugInfo failed!");
    }
    //
    if(mDualDevicePath == DualDevicePath::MultiCamControl) {
        MY_LOGD("multicam scenario");
        unordered_map<uint32_t, MetadataPtr> halMetaPtrList;
        multicamSyncMetadata(in, out, halMetaPtrList);
        if (sensorList.size() > 1 && mPolicyParams.bIsLogicalCam) { // dual cam
            if (!strategyDualCamPlugin(combinedKeyFeature, featureCombination, foundFeature, &temp_out, parsedInfo, in, halMetaPtrList)) {
                MY_LOGD("no need to trigger feature(%#" PRIx64") for features(key:%#" PRIx64", combined:%#" PRIx64")",
                        foundFeature, combinedKeyFeature, featureCombination);
                return false;
            }
        }
        else {
            const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
            const MINT32 mainSensorId = mSensorInfoList[SENSOR_INDEX_MAIN].sensorId;
            MY_LOGD("capture id(%" PRIu32 ") index(%d)", mainSensorId, mainSensorIndex);
            updateRequestResultParams(
                            temp_out.mainFrame,
                            nullptr,
                            halMetaPtrList[mainSensorId],
                            0,
                            mainSensorIndex,
                            featureCombination);
            auto updateSubFrame = [this,
                                   &halMetaPtrList,
                                   &featureCombination,
                                   &mainSensorId](
                                    std::vector<std::shared_ptr<RequestResultParams>>& subFrames,
                                    const MINT32 sensorIndex) {
                for(auto&& sub:subFrames) {
                    updateRequestResultParams(
                                    sub,
                                    nullptr,
                                    halMetaPtrList[mainSensorId],
                                    0,
                                    sensorIndex,
                                    featureCombination);
                }
            };
            updateSubFrame(temp_out.subFrames, mainSensorIndex);
            updateSubFrame(temp_out.preDummyFrames, mainSensorIndex);
            updateSubFrame(temp_out.postDummyFrames, mainSensorIndex);
        }
    }

    {
        // default boost control for all capture behavior.
        // TODO: the boost control will be handled by isp driver, and IScenarioControlV3 will be phased-out after ISP6.x
        BoostControl boostControl;
        boostControl.boostScenario = IScenarioControlV3::Scenario_ContinuousShot;
        updateBoostControl(out, boostControl);
        MY_LOGD("update boostControl for default capture behavior (boostScenario:0x%X, featureFlag:0x%X)",
                boostControl.boostScenario, boostControl.featureFlag);
    }

    // update result
    *out = temp_out;

    return true;
}

auto
FeatureSettingPolicy::
updateCaptureDummyFrames(
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    RequestOutputParams* out,
    const ParsedStrategyInfo& parsedInfo,
    RequestInputParams const* in
) -> void
{
    CAM_ULOGM_APILIFE();

    int8_t preDummyCount = 0;
    int8_t postDummyCount = 0;

    const MINT32 mainSensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;

    if (out->preDummyFrames.size() || out->postDummyFrames.size()) {
        MY_LOGI("feature(%#" PRIx64") has choose dummy frames(pre:%zu, post:%zu)",
                combinedKeyFeature, out->preDummyFrames.size(), out->postDummyFrames.size());

        {
            MY_LOGD("update hint for dummy frames");
            MetadataPtr pPreDummyApp_Additional = std::make_shared<IMetadata>();
            MetadataPtr pPreDummyHal_Additional = std::make_shared<IMetadata>();
            MetadataPtr pPostDummyApp_Additional = std::make_shared<IMetadata>();
            MetadataPtr pPostDummyHal_Additional = std::make_shared<IMetadata>();

            // update dummy frame hint for 3A behavior control
            // TODO: workaround, must update the hint to be dummy frames hint, no need to set 3A setting here
            IMetadata::setEntry<MBOOL>(pPreDummyApp_Additional.get(), MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, 1);
            IMetadata::setEntry<MBOOL>(pPostDummyApp_Additional.get(), MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
            //
            MY_LOGD_IF(mbDebug, "update preDummyFrames size(%zu)", out->preDummyFrames.size());
            for (size_t i = 0; i < out->preDummyFrames.size(); i++) {
                auto& preDummyFrame = out->preDummyFrames[i];
                if (preDummyFrame.get()) {
                    MY_LOGD_IF(mbDebug, "preDummyFrameFrames[%zu] has existed(addr:%p)", i, preDummyFrame.get());
                    updateRequestResultParams(
                        preDummyFrame,
                        pPreDummyApp_Additional,
                        pPreDummyHal_Additional,
                        0,
                        mainSensorIndex);
                }
                else {
                    MY_LOGW("preDummyFrames[%zu] is invalid", i);
                }
            }
            //
            MY_LOGD_IF(mbDebug, "update postDummyFrames size(%zu)", out->postDummyFrames.size());
            for (size_t i = 0; i < out->postDummyFrames.size(); i++) {
                auto& postDummyFrame = out->postDummyFrames[i];
                if (postDummyFrame.get()) {
                    MY_LOGD_IF(mbDebug, "postDummyFrameFrames[%zu] has existed(addr:%p)", i, postDummyFrame.get());
                    updateRequestResultParams(
                        postDummyFrame,
                        pPostDummyApp_Additional,
                        pPostDummyHal_Additional,
                        0,
                        mainSensorIndex);
                }
                else {
                    MY_LOGW("postDummyFrames[%zu] is invalid", i);
                }
            }
        }

        return;
    }
    //
    auto hal3a = mHal3a[mainSensorIndex];
    if (hal3a.get() == nullptr) {
        MY_LOGW("cannot get hal3a sensorId(%d), it is nullptr!", mSensorInfoList[SENSOR_INDEX_MAIN].sensorId);
        return;
    }

    // lambda for choose maximum count
    auto updateDummyCount = [&preDummyCount, &postDummyCount]
    (
        int8_t preCount,
        int8_t postCount
    ) -> void
    {
        preDummyCount = std::max(preDummyCount, preCount);
        postDummyCount = std::max(postDummyCount, postCount);
    };

    // lambda to check manual 3A
    auto isManual3aSetting = []
    (
        IMetadata const* pAppMeta,
        IMetadata const* pHalMeta
    ) -> bool
    {
        if (pAppMeta && pHalMeta) {
            // check manual AE (method.1)
            MUINT8 aeMode = MTK_CONTROL_AE_MODE_ON;
            if (IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, aeMode)) {
                if (aeMode == MTK_CONTROL_AE_MODE_OFF) {
                    MY_LOGD("get MTK_CONTROL_AE_MODE(%d), it is manual AE", aeMode);
                    return true;
                }
            }
            // check manual AE (method.2)
            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            if (IMetadata::getEntry<IMetadata::Memory>(pHalMeta, MTK_3A_AE_CAP_PARAM, capParams)) {
                MY_LOGD("get MTK_3A_AE_CAP_PARAM, it is manual AE");
                return true;
            }
            // check manual WB
            MUINT8 awLock = MFALSE;
            if (IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AWB_LOCK, awLock) && awLock) {
                MY_LOGD("get MTK_CONTROL_AWB_LOCK(%d), it is manual WB", awLock);
                return true;
            }
        }
        else {
            MY_LOGW("no metadata(app:%p, hal:%p) to query hint", pAppMeta, pHalMeta);
        }

        return false;
    };
    //
    bool bIsManual3A = false;
    if (CC_LIKELY(out->mainFrame.get())) {
        IMetadata const* pAppMeta = out->mainFrame->additionalApp.get();
        IMetadata const* pHalMeta = out->mainFrame->additionalHal[mainSensorIndex].get();
        bIsManual3A = isManual3aSetting(pAppMeta, pHalMeta);
    }
    else {
        MY_LOGD("no metadata info due to no mainFrame");
    }
    //
    if (parsedInfo.ispTuningFrameCount > 0) {
        // Prevent 3rd party camera Apk from unstandard abnormal behavior,
        // only support dummy frames if App set manual AE with frame count index hint.
        MY_LOGD_IF(parsedInfo.isAppManual3A, "App set AE manaul(count:%d, index:%d)",
                   parsedInfo.ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
        bIsManual3A |= parsedInfo.isAppManual3A;
    }
    //
    if (bIsManual3A) {
        // get manual 3a delay frames count from 3a hal
        MUINT32 delayedFrames = 0;
        {
            std::lock_guard<std::mutex> _l(mHal3aLocker);
            hal3a->send3ACtrl(E3ACtrl_GetCaptureDelayFrame, reinterpret_cast<MINTPTR>(&delayedFrames), 0);
        }
        MY_LOGD("delayedFrames count:%d due to manual 3A", delayedFrames);
        //
        updateDummyCount(0, delayedFrames);
    }

    if (parsedInfo.isFlashOn) {
        rHAL3AFlashCapDummyInfo_T flashCapDummyInfo;
        rHAL3AFlashCapDummyCnt_T flashCapDummyCnt;
        flashCapDummyInfo.u4AeMode = parsedInfo.aeMode;
        flashCapDummyInfo.u4StrobeMode = parsedInfo.flashMode;
        flashCapDummyInfo.u4CaptureIntent = parsedInfo.captureIntent;
        hal3a->send3ACtrl(E3ACtrl_GetFlashCapDummyCnt, reinterpret_cast<MINTPTR>(&flashCapDummyInfo), reinterpret_cast<MINTPTR>(&flashCapDummyCnt));
        updateDummyCount(flashCapDummyCnt.u4CntBefore, flashCapDummyCnt.u4CntAfter);
        MY_LOGD("dummy frames count(pre:%d, post:%d) from Hal3A requirement, due to flash on(aeMode:%d, flashMode:%d, captureIntent:%d)",
                flashCapDummyCnt.u4CntBefore, flashCapDummyCnt.u4CntAfter,
                flashCapDummyInfo.u4AeMode, flashCapDummyInfo.u4StrobeMode, flashCapDummyInfo.u4CaptureIntent);
    }

    if (parsedInfo.ispTuningFrameCount > 0) {
        if (parsedInfo.ispTuningOutputFirstFrame == false) {
            preDummyCount = 0;
            MY_LOGD("only first frame need pre-dummy for 3A, update dummy frames count(pre:%d, post:%d) for this request(count:%d, index:%d)",
                    preDummyCount, postDummyCount, parsedInfo.ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
        }
        if (parsedInfo.ispTuningOutputLastFrame == false) {
            postDummyCount = 0;
            MY_LOGD("only last frame need post-dummy for 3A, update dummy frames count(pre:%d, post:%d) for this request(count:%d, index:%d)",
                    preDummyCount, postDummyCount, parsedInfo.ispTuningFrameCount, parsedInfo.ispTuningFrameIndex);
        }
    }

    MY_LOGD("dummy frames result(pre:%d, post:%d)", preDummyCount, postDummyCount);

    uint32_t camP1Dma = 0;
    //uint32_t sensorIndex = SENSOR_INDEX_MAIN;
    if ( !getCaptureP1DmaConfig(camP1Dma, in, mainSensorIndex) ){
        MY_LOGE("main P1Dma output is invalid: 0x%X", camP1Dma);
        return;
    }

    // update preDummyFrames
    for (MINT32 i = 0; i < preDummyCount; i++) {
        MetadataPtr pAppDummy_Additional = std::make_shared<IMetadata>();
        MetadataPtr pHalDummy_Additional = std::make_shared<IMetadata>();
        // update info for pre-dummy frames for flash/3a stable
        IMetadata::setEntry<MBOOL>(pHalDummy_Additional.get(), MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, 1);
        //
        std::shared_ptr<RequestResultParams> preDummyFrame = nullptr;
        updateRequestResultParams(
                preDummyFrame,
                pAppDummy_Additional,
                pHalDummy_Additional,
                camP1Dma,
                mainSensorIndex);
        //
        out->preDummyFrames.push_back(preDummyFrame);
    }

    // update postDummyFrames
    for (MINT32 i = 0; i < postDummyCount; i++) {
        MetadataPtr pAppDummy_Additional = std::make_shared<IMetadata>();
        MetadataPtr pHalDummy_Additional = std::make_shared<IMetadata>();
        // update info for post-dummy(delay) frames to restore 3A for preview stable
        IMetadata::setEntry<MBOOL>(pHalDummy_Additional.get(), MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
        //
        std::shared_ptr<RequestResultParams> postDummyFrame = nullptr;
        updateRequestResultParams(
                postDummyFrame,
                pAppDummy_Additional,
                pHalDummy_Additional,
                camP1Dma,
                mainSensorIndex);
        //
        out->postDummyFrames.push_back(postDummyFrame);
    }

    // check result
    if (out->preDummyFrames.size() || out->postDummyFrames.size()) {
        MY_LOGI("feature(%#" PRIx64") append dummy frames(pre:%zu, post:%zu) due to isFlashOn(%d), isManual3A(%d)",
                combinedKeyFeature, out->preDummyFrames.size(), out->postDummyFrames.size(),
                parsedInfo.isFlashOn, bIsManual3A);

        if (out->needZslFlow) {
            MY_LOGW("not support Zsl buffer due to isFlashOn(%d) or isManual3A(%d)", parsedInfo.isFlashOn, bIsManual3A);
            out->needZslFlow = false;
        }
    }

    return;
}

auto
FeatureSettingPolicy::
toTPIDualHint(
    ScenarioHint &hint
) -> void
{
    hint.isDualCam = mPolicyParams.pPipelineStaticInfo->isDualDevice;
    hint.dualDevicePath = static_cast<int32_t>(mDualDevicePath);
    hint.multiCamFeatureMode = miMultiCamFeatureMode;
    int stereoMode = StereoSettingProvider::getStereoFeatureMode();
    if( hint.isDualCam )
    {
        if( stereoMode & v1::Stereo::E_STEREO_FEATURE_VSDOF )
            hint.mDualFeatureMode = DualFeatureMode_MTK_VSDOF;
        else if( stereoMode & v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP )
            hint.mDualFeatureMode = DualFeatureMode_HW_DEPTH;
        else if( stereoMode & v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY )
            hint.mDualFeatureMode = DualFeatureMode_YUV;
        else if( stereoMode & v1::Stereo::E_DUALCAM_FEATURE_ZOOM)
            hint.mDualFeatureMode = DualFeatureMode_MTK_FOVA;
        else
            hint.mDualFeatureMode = DualFeatureMode_NONE;
    }
}

auto
FeatureSettingPolicy::
updateMulticamSensorControl(
    RequestInputParams const* in,
    uint32_t const& sensorId,
    MetadataPtr pHalMeta,
    uint32_t &dma
) -> void
{
    if(in == nullptr) {
        MY_LOGE("RequestInputParams is nullptr");
        return;
    }
    if(in->pMultiCamReqOutputParams == nullptr) {
        MY_LOGE("pMultiCamReqOutputParams is nullptr");
        return;
    }
    if(pHalMeta == nullptr)
    {
        MY_LOGE("pHalMeta is nullptr");
        return;
    }
    bool exist = false;
    // 1. check go to standby control
    {
        // if sensor wants to go to standby, it has to add MTK_P1_SENSOR_STATUS_HW_STANDBY,
        // to hal metadata tag.
        if(std::find(
                    in->pMultiCamReqOutputParams->goToStandbySensorList.begin(),
                    in->pMultiCamReqOutputParams->goToStandbySensorList.end(),
                    sensorId) != in->pMultiCamReqOutputParams->goToStandbySensorList.end())
        {
            // exist in go to standby list.
            exist = true;
            IMetadata::setEntry<MINT32>(
                                        pHalMeta.get(),
                                        MTK_P1NODE_SENSOR_STATUS,
                                        MTK_P1_SENSOR_STATUS_HW_STANDBY);
            MY_LOGI("req(%" PRIu32 ") sensorid(%" PRIu32 ") set standby flag",
                                    in->requestNo, sensorId);
        }
    }
    // 2. check resume control
    {
        // if sensor wants to resume, it has to add MTK_P1_SENSOR_STATUS_STREAMING,
        // to hal metadata tag.
        if(std::find(
                    in->pMultiCamReqOutputParams->resumeSensorList.begin(),
                    in->pMultiCamReqOutputParams->resumeSensorList.end(),
                    sensorId) != in->pMultiCamReqOutputParams->resumeSensorList.end())
        {
            // exist in resume standby list.
            exist = true;
            IMetadata::setEntry<MINT32>(
                                        pHalMeta.get(),
                                        MTK_P1NODE_SENSOR_STATUS,
                                        MTK_P1_SENSOR_STATUS_STREAMING);
            MY_LOGI("req(%" PRIu32 ") sensorid(%" PRIu32 ") set resume flag",
                                    in->requestNo, sensorId);
        }
    }
    // 3. check continuous standby
    {
        // if sensor is already in standby mode, it has to set dma to 0.
        // otherwise, IOMap will request P1 to output image. And, it will
        // cause preview freeze.
        if(std::find(
                    in->pMultiCamReqOutputParams->standbySensorList.begin(),
                    in->pMultiCamReqOutputParams->standbySensorList.end(),
                    sensorId) != in->pMultiCamReqOutputParams->standbySensorList.end())
        {
            exist = true;
            dma = 0;
        }
    }
    // 4. check MarkErrorSensorList
    {
        // if sensor id is exist in MarkErrorSensorList, it means no needs to output image
        // and no need output p1 dma buffer
        if(std::find(
                    in->pMultiCamReqOutputParams->markErrorSensorList.begin(),
                    in->pMultiCamReqOutputParams->markErrorSensorList.end(),
                    sensorId) != in->pMultiCamReqOutputParams->markErrorSensorList.end())
        {
            exist = true;
            dma = 0;
        }
    }
    // 4. check is streaming
    {
        if(std::find(
                    in->pMultiCamReqOutputParams->streamingSensorList.begin(),
                    in->pMultiCamReqOutputParams->streamingSensorList.end(),
                    sensorId) != in->pMultiCamReqOutputParams->streamingSensorList.end())
        {
            exist = true;
        }
    }
    if(!exist)
    {
        // if sensor id not exist in above list, it means current sensor is not active.
        dma = 0;
    }
    return;
}

auto
FeatureSettingPolicy::
queryMulticamTgCrop(
    RequestInputParams const* in,
    uint32_t const& sensorId,
    MRect &tgCrop,
    bool &isCapture
) -> bool
{
    bool ret = false;
    if(in == nullptr || in->pMultiCamReqOutputParams == nullptr) {
        return ret;
    }
    if(isCapture)
    {
        // tg crop
        auto iter = in->pMultiCamReqOutputParams->tgCropRegionList_Cap.find(sensorId);
        if(iter != in->pMultiCamReqOutputParams->tgCropRegionList_Cap.end()) {
            tgCrop = iter->second;
            ret = true;
        }
    }
    else
    {
        // tg crop
        auto iter = in->pMultiCamReqOutputParams->tgCropRegionList.find(sensorId);
        if(iter != in->pMultiCamReqOutputParams->tgCropRegionList.end()) {
            tgCrop = iter->second;
            ret = true;
        }
    }
    return ret;
}

auto
FeatureSettingPolicy::
queryMulticamScalerCrop(
    RequestInputParams const* in,
    uint32_t const& sensorId,
    MRect &scalerCrop,
    bool &isCapture
) -> bool
{
    bool ret = false;
    if(in == nullptr || in->pMultiCamReqOutputParams == nullptr) {
        return ret;
    }
    if(isCapture)
    {
        // scaler crop
        auto iter = in->pMultiCamReqOutputParams->sensorScalerCropRegionList_Cap.find(sensorId);
        if(iter != in->pMultiCamReqOutputParams->sensorScalerCropRegionList_Cap.end()) {
            scalerCrop = iter->second;
            ret = true;
        }
    }
    else
    {
        // scaler crop
        auto iter = in->pMultiCamReqOutputParams->sensorScalerCropRegionList.find(sensorId);
        if(iter != in->pMultiCamReqOutputParams->sensorScalerCropRegionList.end()) {
            scalerCrop = iter->second;
            ret = true;
        }
    }
    return ret;
}

bool
FeatureSettingPolicy::
isPhysicalStreamUpdate()
{
    // for physical stream update
    // Physical setting:
    // isDualDevice is false, but contain feature mode.
    bool ret = false;
    auto& pParsedAppConfiguration = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto& pParsedMultiCamInfo = pParsedAppConfiguration->pParsedMultiCamInfo;
    if (pParsedMultiCamInfo != nullptr &&
        (!mPolicyParams.pPipelineStaticInfo->isDualDevice) &&
        (DualDevicePath::MultiCamControl == pParsedMultiCamInfo->mDualDevicePath)&&
        (-1 != pParsedMultiCamInfo->mDualFeatureMode)) {
        ret = true;
    }
    return ret;
}

auto
FeatureSettingPolicy::
getStreamDuration(
    const IMetadata::IEntry& entry,
    const MINT64 format,
    const MSize& size,
    MINT64& duration
) -> bool
{
    if(entry.isEmpty()) {
        MY_LOGW("Static meta : MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS entry is empty!");
        return false;
    }
    else
    {
        for (size_t i = 0, count = entry.count(); i + 3 < count; i += 4) {
            if (entry.itemAt(i    , Type2Type<MINT64>()) == format &&
                entry.itemAt(i + 1, Type2Type<MINT64>()) == (MINT64)size.w &&
                entry.itemAt(i + 2, Type2Type<MINT64>()) == (MINT64)size.h)
            {
                duration = entry.itemAt(i + 3, Type2Type<MINT64>());
                return true;
            }
        }
    }
    return false;
}

bool
FeatureSettingPolicy::
multicamSyncMetadata(
    RequestInputParams const* in,
    RequestOutputParams* out,
    unordered_map<uint32_t, MetadataPtr>& halMetaPtrList
)
{
    auto const& sensorIdList = mPolicyParams.pPipelineStaticInfo->sensorId;
    auto const& vStreamingSensorList = in->pMultiCamReqOutputParams->prvStreamingSensorList;
    auto updateSensorCrop = [&in, this](
                            int32_t index,
                            IMetadata* metadata)
    {
        if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
        {
            // vsdof crop info is compute by StereSizeProvider.
            MRect crop;
            queryVsdofSensorCropRect(index, in, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_P1NODE_SENSOR_CROP_REGION, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_DUALZOOM_CAP_CROP, crop);
            MY_LOGD("b_sc(%d, %d, %dx%d)", crop.p.x, crop.p.y, crop.s.w, crop.s.h);
            queryVsdof3APreviewCropRect(index, in, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_3A_PRV_CROP_REGION, crop);
            MY_LOGD("3a b_sc(%d, %d, %dx%d)", crop.p.x, crop.p.y, crop.s.w, crop.s.h);
        }
        else
        {
            uint32_t sensorId = mPolicyParams.pPipelineStaticInfo->sensorId[index];
            MRect tgCrop, scalerCrop;
            bool isCapture = true;
            if(queryMulticamTgCrop(in, sensorId, tgCrop, isCapture))
            {
                IMetadata::setEntry<MRect>(metadata, MTK_P1NODE_SENSOR_CROP_REGION, tgCrop);
                IMetadata::setEntry<MRect>(metadata, MTK_DUALZOOM_CAP_CROP, tgCrop);
                IMetadata::setEntry<MRect>(metadata, MTK_3A_PRV_CROP_REGION, tgCrop);
            }
            if(queryMulticamScalerCrop(in, sensorId, scalerCrop, isCapture))
            {
                IMetadata::setEntry<MRect>(metadata, MTK_SENSOR_SCALER_CROP_REGION, scalerCrop);
            }
            MY_LOGD("index(%d) tg(%d, %d, %dx%d) z_sc(%d, %d, %dx%d)", index,
                                tgCrop.p.x, tgCrop.p.y, tgCrop.s.w, tgCrop.s.h,
                                scalerCrop.p.x, scalerCrop.p.y, scalerCrop.s.w, scalerCrop.s.h);
        }
        return true;
    };
    auto sensorIdToIdx = [&sensorIdList](MINT32 id)
    {
        for(size_t i=0;i<sensorIdList.size();i++)
        {
            if(sensorIdList[i] == id)
            {
                return (MINT32)i;
            }
        }
        return -1;
    };
    if(vStreamingSensorList.size() > 1)
    {
        // init halMetaPtrList
        for(auto sensorId:sensorIdList)
        {
            halMetaPtrList.insert({sensorId, std::make_shared<IMetadata>()});
        }
        IMetadata::IEntry msTag(MTK_STEREO_SYNC2A_MASTER_SLAVE);
        auto masterId = in->pMultiCamReqOutputParams->masterId;
        msTag.push_back(masterId, Type2Type<MINT32>());
        for(auto&& sensorId:vStreamingSensorList)
        {
            if(masterId != sensorId)
                msTag.push_back(sensorId, Type2Type<MINT32>());
        }
        // a. query sync infomation.
        MINT64 hwsyncToleranceTime = ::getFrameSyncToleranceTime();
        MINT32 syncFailBehavior = MTK_FRAMESYNC_FAILHANDLE_CONTINUE;
        for(auto&& sensorId:vStreamingSensorList)
        {
            auto iter = halMetaPtrList.find(sensorId);
            if(iter != halMetaPtrList.end() && iter->second != nullptr)
            {
                // set sync id.
                int32_t index = sensorIdToIdx(sensorId);
                MINT32 syncType = Utils::Imp::HW_RESULT_CHECK;
                if(index == -1) continue;
                {
                    // sync target for synchelper
                    IMetadata::IEntry tag(MTK_FRAMESYNC_ID);
                    for(auto&& syncId:vStreamingSensorList) {
                        if(iter->first != syncId) {
                            tag.push_back(syncId, Type2Type<MINT32>());
                        }
                    }
                    iter->second->update(tag.tag(), tag);
                    MY_LOGD("set sync tag");
                }
                {
                    if(in->pMultiCamReqOutputParams->needSynchelper_3AEnq)
                    {
                        syncType |= Utils::Imp::ENQ_HW;
                    }
                    // set tolerance time
                    IMetadata::setEntry<MINT64>(iter->second.get(), MTK_FRAMESYNC_TOLERANCE, hwsyncToleranceTime);
                    // set sync fail behavior
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_FRAMESYNC_FAILHANDLE, syncFailBehavior);
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_FRAMESYNC_TYPE, syncType);
                }
                if(in->pMultiCamReqOutputParams->needSync2A) {
                    const MBOOL isZslFlow = (out->needZslFlow == MTRUE);
                    if(isZslFlow) {
                        if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
                            IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF_BY_FRAME);
                        else
                            IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM_BY_FRAME);
                    }
                    else {
                        IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE);
                        MY_LOGI("non-zsl flow, needZslFlow:%d", out->needZslFlow);
                    }
                }
                else {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE);
                }
                if(in->pMultiCamReqOutputParams->needSyncAf) {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNCAF_MODE, NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON);
                }
                else {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNCAF_MODE, NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF);
                }
                if(in->pMultiCamReqOutputParams->needFramesync) {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON);
                }
                // set master & slave
                iter->second->update(msTag.tag(), msTag);
            }
        }
    }
    // add crop
    for(auto&& sensorId:vStreamingSensorList)
    {
        auto iter = halMetaPtrList.find(sensorId);
        if(iter == halMetaPtrList.end() || iter->second == nullptr)
        {
            halMetaPtrList.insert({sensorId, std::make_shared<IMetadata>()});
        }
        int32_t index = sensorIdToIdx(sensorId);
        MY_LOGI("crop update id(%" PRIu32 ") index(%d)", sensorId, index);
        // update sensor crop
        if(index != -1 && !out->needZslFlow)
            updateSensorCrop(index, halMetaPtrList[sensorId].get());
    }
    return true;
}

auto
FeatureSettingPolicy::
updateBoostControl(
    RequestOutputParams* out,
    const BoostControl& boostControl
) -> bool
{
    bool bUpdated = false;
    bool ret = true;

    if (boostControl.boostScenario != IScenarioControlV3::Scenario_None) { // check valid
        for (auto& control : out->vboostControl) {
            if (control.boostScenario == boostControl.boostScenario) {
                int32_t originalFeatureFlag = control.featureFlag;
                control.featureFlag |= boostControl.featureFlag;
                MY_LOGD("update boostControl(size:%zu): boostScenario(0x%X), featureFlag(0x%X|0x%X => 0x%X)",
                        out->vboostControl.size(), control.boostScenario, originalFeatureFlag, boostControl.featureFlag, control.featureFlag);
                bUpdated = true;
            }
            else {
                MY_LOGD_IF(mbDebug, "current boostControl(size:%zu): boostScenario(0x%X), featureFlag(0x%X)",
                        out->vboostControl.size(), control.boostScenario, control.featureFlag);
            }
        }
        //
        if (!bUpdated) {
            out->vboostControl.push_back(boostControl);
            MY_LOGD("add boostControl(size:%zu): boostScenario(0x%X), featureFlag(0x%X)",
                    out->vboostControl.size(), boostControl.boostScenario, boostControl.featureFlag);
        }
        ret = true;
    }
    else {
        if (boostControl.featureFlag != IScenarioControlV3::FEATURE_NONE) {
            MY_LOGW("invalid boostControl(boostScenario:0x%X, featureFlag:0x%X)",
                    boostControl.boostScenario, boostControl.featureFlag);
            ret = false;
        }
        else {
            MY_LOGD_IF(mbDebug, "no need to update boostControl(boostScenario:0x%X, featureFlag:0x%X)",
                    boostControl.boostScenario, boostControl.featureFlag);
            ret = true;
        }
    }

    return ret;
}

auto
FeatureSettingPolicy::
evaluateCaptureSetting(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> bool
{
    if (in->needP2CaptureNode || in->needRawOutput) {

        CAM_TRACE_CALL();
        CAM_ULOGM_APILIFE();

        MY_LOGI("(%p) capture req#:%u", this, in->requestNo);

        bool isCaptureUpdate = true;
        SensorInfoUpdater updater(in, isCaptureUpdate, mPolicyParams, &mSensorInfoList);

        ParsedStrategyInfo parsedInfo;
        if (!collectParsedStrategyCaptureInfo(out, parsedInfo, in)) {
            MY_LOGE("collectParsedStrategyInfo failed!");
            return false;
        }

        ScenarioFeatures scenarioFeatures;
        CaptureScenarioConfig scenarioConfig;
        ScenarioHint scenarioHint;
        toTPIDualHint(scenarioHint);
        scenarioHint.operationMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->operationMode;
        scenarioHint.isSuperNightMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->isSuperNightMode;
        scenarioHint.isIspHidlTuningEnable = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->hasTuningEnable;
        scenarioHint.captureFeatureMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->pParsedMultiCamInfo->mCaptureFeatureMode;
        scenarioHint.isCShot = parsedInfo.isCShot;
        scenarioHint.isYuvReprocess = parsedInfo.isYuvReprocess;
        scenarioHint.isRawReprocess = parsedInfo.isRawReprocess;
        scenarioHint.isYuvRequestForIspHidl = parsedInfo.isYuvRequestForIspHidl;
        scenarioHint.isRawRequestForIspHidl = parsedInfo.isRawRequestForIspHidl;
        scenarioHint.ispTuningHint = parsedInfo.ispTuningHint;
        scenarioHint.isLogical = mPolicyParams.bIsLogicalCam;
        if (in->needRawOutput && scenarioHint.isRawRequestForIspHidl == false) {
            // App request Dng or Opaue
            scenarioHint.isRawOrDngRequest = true;
        }
        //TODO:
        //scenarioHint.captureScenarioIndex = ? /* hint from vendor tag */
        int32_t openId = mPolicyParams.pPipelineStaticInfo->openId;
        auto pAppMetadata = in->pRequest_AppControl;

        // Decide the master and slave sensor id of the stream
        if ((mDualDevicePath == DualDevicePath::MultiCamControl) && !mPolicyParams.bIsLogicalCam) {
            MINT32 masterId = (MINT32)in->pMultiCamReqOutputParams->prvMasterId;
            if (openId == masterId)
                scenarioHint.isMaster = MTRUE;
            else
                scenarioHint.isMaster = MFALSE;
        }

        MY_LOGD("openId(%d), sensorId(%d), subsensorId(%d), sensor nums %zu",
                 openId,
                 mSensorInfoList[SENSOR_INDEX_MAIN].sensorId,
                 (mSensorInfoList.size() > 1) ? mSensorInfoList[SENSOR_INDEX_SUB1].sensorId : -1,
                 mSensorInfoList.size());

        int32_t scenario = -1;
        if (!get_capture_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
            MY_LOGE("cannot get capture scenario");
            return false;
        }
        else {
            MY_LOGD("find scenario:%s for (openId:%d, scenario:%d)",
                    scenarioFeatures.scenarioName.c_str(), openId, scenario);
        }

        if (CC_UNLIKELY(scenarioConfig.needUnpackRaw == true)) {
            bool originalNeedUnpackRaw = out->needUnpackRaw;
            out->needUnpackRaw |= scenarioConfig.needUnpackRaw;
            MY_LOGI("set needUnpackRaw(%d|%d => %d) for scenario:%s",
                    originalNeedUnpackRaw, scenarioConfig.needUnpackRaw, out->needUnpackRaw, scenarioFeatures.scenarioName.c_str());
        }

        // No need to keep buffer in zsl
        out->keepZslBuffer = false;

        bool isFeatureTrigger = false;
        for (auto &featureSet : scenarioFeatures.vFeatureSet) {
            // evaluate key feature plugin and feature combination for feature strategy policy.
            if (strategyCaptureFeature( featureSet.feature, featureSet.featureCombination, out, parsedInfo, in )) {
                isFeatureTrigger = true;
                uint8_t frameCount = out->subFrames.size() + 1;
                MY_LOGI("trigger feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64") for req#%u",
                        featureSet.featureName.c_str(),
                        static_cast<MINT64>(featureSet.feature),
                        featureSet.featureCombinationName.c_str(),
                        static_cast<MINT64>(featureSet.featureCombination),
                        in->requestNo);
                updatePluginSelection(in->requestNo, isFeatureTrigger, frameCount);
                break;
            }
            else{
                isFeatureTrigger = false;
                MY_LOGD("no need to trigger feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64")",
                        featureSet.featureName.c_str(),
                        static_cast<MINT64>(featureSet.feature),
                        featureSet.featureCombinationName.c_str(),
                        static_cast<MINT64>(featureSet.featureCombination));
                updatePluginSelection(in->requestNo, isFeatureTrigger);
            }
        }

        if (mHDRHelper->isHDR() &&
            (in->pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0 ||
             in->pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() != 0))
        {
            // TODO: move to streaming policy.
            MY_LOGI("update streaming HDR requirement for P2S");
            MetadataPtr pOutMetaHal = std::make_shared<IMetadata>();
            MetadataPtr pOutMetaApp = std::make_shared<IMetadata>();
            HDRPolicyHelper::HDR_REQUEST_TYPE type = (in->needP2StreamNode) ? HDRPolicyHelper::HDR_REQUEST_PREVIEW_CAPTURE
                                                                            : HDRPolicyHelper::HDR_REQUEST_CAPTURE;
            IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HDR_FEATURE_HDR_HAL_MODE, mHDRHelper->getHDRHalRequestMode(type));
            IMetadata::setEntry<MUINT8>(pOutMetaHal.get(), MTK_3A_HDR_MODE, static_cast<MUINT8>(mHDRHelper->getHDRAppMode()));
            updateRequestOutputAllFramesMetadata(out, pOutMetaApp, pOutMetaHal);
        }

        if (!isFeatureTrigger) {
            MY_LOGE("no feature can be triggered!");
            return false;
        }

        MY_LOGD("capture request frames count(mainFrame:%d, subFrames:%zu, preDummyFrames:%zu, postDummyFrames:%zu)",
                (out->mainFrame.get() != nullptr), out->subFrames.size(),
                out->preDummyFrames.size(), out->postDummyFrames.size());
    }
    return true;
}

auto
FeatureSettingPolicy::
evaluateReconfiguration(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> bool
{
    CAM_TRACE_CALL();
    //CAM_ULOGM_APILIFE();
    out->needReconfiguration = false;
    out->reconfigCategory = ReCfgCtg::NO;
    for (unsigned int i=0; i<in->sensorMode.size(); i++) {
        if (in->sensorMode[i] != out->sensorMode[i]) {
            MY_LOGD("sensor(index:%d): sensorMode(%d --> %d) is changed", i, in->sensorMode[i], out->sensorMode[i]);
            out->needReconfiguration = true;
        }

        if (mHDRHelper->needReconfiguration()) {
            out->needReconfiguration = true;
            out->reconfigCategory = ReCfgCtg::STREAMING;
            if (!mHDRHelper->handleReconfiguration()) {
                MY_LOGE("HDR handleReconfiguration failed(Category=%hhu)", out->reconfigCategory);
            }
        }

        MINT32 forceReconfig = ::property_get_bool("vendor.debug.camera.hal3.pure.reconfig.test", -1);
        if(forceReconfig == 1){
            out->needReconfiguration = true;
            out->reconfigCategory = ReCfgCtg::STREAMING;
        }
        else if(forceReconfig == 0){
            out->needReconfiguration = false;
            out->reconfigCategory = ReCfgCtg::NO;
        }

        // sensor mode is not the same as preview default (cannot execute zsl)
        if (out->needReconfiguration == true ||
            mDefaultConfig.sensorMode[i] != out->sensorMode[i]) {
            out->needZslFlow = false;
            out->zslPolicyParams.mPolicy = eZslPolicy_None;
            MY_LOGD("must reconfiguration, capture new frames w/o zsl flow");
        }
    }
    // zsl policy debug
    if (out->needZslFlow) {
        MY_LOGD("needZslFlow(%d), zsl policy(0x%X), timestamp:%" PRId64 ", timeouts:%" PRId64 "",
                out->needZslFlow, out->zslPolicyParams.mPolicy,
                out->zslPolicyParams.mTimestamp, out->zslPolicyParams.mTimeouts);
    }
    return true;
}

auto
FeatureSettingPolicy::
getCaptureProvidersByScenarioFeatures(
    ConfigurationOutputParams* out __unused,
    ConfigurationInputParams const* in __unused
) -> bool
{
    CAM_ULOGM_APILIFE();

    auto supportedFeatures = out->CaptureParams.supportedScenarioFeatures;
    auto pluginUniqueKey = in->uniqueKey;
    auto isPhysical = !mPolicyParams.bIsLogicalCam;
    MY_LOGI("support features:%#" PRIx64 " uniqueKey:%d isPhysical %d", supportedFeatures, pluginUniqueKey, isPhysical);
    //
    mMFPPluginWrapperPtr = std::make_shared<MFPPluginWrapper>(supportedFeatures, pluginUniqueKey, isPhysical);
    mRawPluginWrapperPtr = std::make_shared<RawPluginWrapper>(supportedFeatures, pluginUniqueKey, isPhysical);
    mYuvPluginWrapperPtr = std::make_shared<YuvPluginWrapper>(supportedFeatures, pluginUniqueKey, isPhysical);
    if (mPolicyParams.pPipelineStaticInfo->isDualDevice) {
        MY_LOGD("current multicam feature mode (%d)", miMultiCamFeatureMode);
        if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
        {
            mBokehPluginWraperPtr = std::make_shared<BokehPluginWraper>(supportedFeatures, pluginUniqueKey, isPhysical);
            mDepthPluginWraperPtr = std::make_shared<DepthPluginWraper>(supportedFeatures, pluginUniqueKey, isPhysical);
            mFusionPluginWraperPtr = std::make_shared<FusionPluginWraper>(supportedFeatures, pluginUniqueKey, isPhysical);
            // query preview process main2 period.
            // if value is 1, it means each main2 frame will send to p2 streaming node.
            // if value is 2, it means 2 main2 frames will send 1 frame to streaming node.
            mPeriodForStreamingProcessDepth = StereoSettingProvider::getMain2OutputFrequency();
            mPeriodForStreamingProcessDepth = (MUINT32)::property_get_int32("vendor.debug.camera.depthperiod", (MINT32)mPeriodForStreamingProcessDepth);
            if(mPeriodForStreamingProcessDepth == 0) mPeriodForStreamingProcessDepth = 1;
            MY_LOGD("mPeriodForStreamingProcessDepth(%d)", mPeriodForStreamingProcessDepth);
        }
        else
        {
            MY_LOGD("multicam or zoom scenario");
        }
    }
    // query sensor count
    mSensorCount = mPolicyParams.pPipelineStaticInfo->sensorId.size();

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
FeatureSettingPolicy::
evaluateCaptureConfiguration(
    ConfigurationOutputParams* out __unused,
    ConfigurationInputParams const* in __unused
) -> bool
{
    CAM_TRACE_CALL();
    CAM_ULOGM_APILIFE();

    // query features by scenario during config
    ScenarioFeatures scenarioFeatures;
    CaptureScenarioConfig scenarioConfig;
    ScenarioHint scenarioHint;
    toTPIDualHint(scenarioHint);
    scenarioHint.operationMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->operationMode;
    scenarioHint.isIspHidlTuningEnable = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->hasTuningEnable;
    scenarioHint.isSuperNightMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->isSuperNightMode;
    scenarioHint.captureFeatureMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->pParsedMultiCamInfo->mCaptureFeatureMode;
    scenarioHint.isLogical = mPolicyParams.bIsLogicalCam;
    //TODO:
    //scenarioHint.captureScenarioIndex = ?   /* hint from vendor tag */
    //scenarioHint.streamingScenarioIndex = ? /* hint from vendor tag */
    int32_t openId = mPolicyParams.pPipelineStaticInfo->openId;
    auto pAppMetadata = &mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->sessionParams;

    // set the dualFeatureMode for VSDOF or ZOOM
    auto const& pParsedAppConfiguration = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto& pParsedMultiCamInfo = pParsedAppConfiguration->pParsedMultiCamInfo;
    if (pParsedMultiCamInfo->mCaptureFeatureMode != -1) {
        out->CaptureParams.dualFeatureMode = pParsedMultiCamInfo->mCaptureFeatureMode;
        MY_LOGD("dualFeatureMode:%#" PRIx64"", out->CaptureParams.dualFeatureMode);
    }
    scenarioHint.captureFeatureMode = pParsedMultiCamInfo->mCaptureFeatureMode;

    int32_t scenario = -1;
    if (!get_capture_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
        MY_LOGE("cannot get capture scenario");
        return false;
    }
    else {
        MY_LOGD("find scenario:%s for (openId:%d, scenario:%d)",
                scenarioFeatures.scenarioName.c_str(), openId, scenario);
    }

    for (auto &featureSet : scenarioFeatures.vFeatureSet) {
        MY_LOGI("scenario(%s) support feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64")",
                scenarioFeatures.scenarioName.c_str(),
                featureSet.featureName.c_str(),
                static_cast<MINT64>(featureSet.feature),
                featureSet.featureCombinationName.c_str(),
                static_cast<MINT64>(featureSet.featureCombination));
        out->CaptureParams.supportedScenarioFeatures |= (featureSet.feature | featureSet.featureCombination);
    }
    MY_LOGD("support features:%#" PRIx64"", out->CaptureParams.supportedScenarioFeatures);

    mUniqueKey = in->uniqueKey;
    out->CaptureParams.pluginUniqueKey = in->uniqueKey;
    MY_LOGD("(%p) uniqueKey:%d", this, mUniqueKey);

    if (!getCaptureProvidersByScenarioFeatures(out, in)) {
        MY_LOGE("createCapturePluginByScenarioFeatures failed!");
        return false;
    }

    // query additional capture buffer usage count from multiframe features
    for (auto iter : mMFPPluginWrapperPtr->getProviders()) {
        const MultiFramePlugin::Property& property =  iter->property();
        auto supportedFeatures = out->CaptureParams.supportedScenarioFeatures;
        if ((property.mFeatures & supportedFeatures) != 0) {
            MY_LOGD("provider(%s) algo(%#" PRIx64"), zsdBufferMaxNum:%u, needRrzoBuffer:%d",
                    property.mName, property.mFeatures, property.mZsdBufferMaxNum, property.mNeedRrzoBuffer);
            if (property.mZsdBufferMaxNum > out->CaptureParams.additionalHalP1OutputBufferNum.imgo) {
                out->CaptureParams.additionalHalP1OutputBufferNum.imgo =  property.mZsdBufferMaxNum;
                out->CaptureParams.additionalHalP1OutputBufferNum.lcso =  property.mZsdBufferMaxNum; // for tuning, must pairing with imgo num
            }
            if (property.mNeedRrzoBuffer && property.mZsdBufferMaxNum > out->CaptureParams.additionalHalP1OutputBufferNum.rrzo) {
                out->CaptureParams.additionalHalP1OutputBufferNum.rrzo =  property.mZsdBufferMaxNum; // caputre feature request to use rrzo
            }
            // dualcam mode
            if (miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF) {
                // rrzo for depth/bokeh usage in dual cam vsdof mode
                MY_LOGI("additional rrzofor(%u->%u) for MTK_MULTI_CAM_FEATURE_MODE_VSDOF",
                        out->CaptureParams.additionalHalP1OutputBufferNum.rrzo,
                        out->CaptureParams.additionalHalP1OutputBufferNum.imgo);
                out->CaptureParams.additionalHalP1OutputBufferNum.rrzo = out->CaptureParams.additionalHalP1OutputBufferNum.imgo;
            }
        }
        else {
            MY_LOGD("no need this provider(%s) algo(%#" PRIx64"), zsdBufferMaxNum:%u at this sceanrio(%s)",
                    property.mName, property.mFeatures, property.mZsdBufferMaxNum, scenarioFeatures.scenarioName.c_str());
        }
    }
    // update scenarioconfig
    if (CC_UNLIKELY(scenarioConfig.maxAppJpegStreamNum == 0)) {
        MY_LOGW("invalid maxAppJpegStreamNum(%d), set to 1 at least", scenarioConfig.maxAppJpegStreamNum);
        out->CaptureParams.maxAppJpegStreamNum = 1;
    }
    else {
        out->CaptureParams.maxAppJpegStreamNum = scenarioConfig.maxAppJpegStreamNum;
    }
    if (CC_UNLIKELY(scenarioConfig.maxAppRaw16OutputBufferNum == 0)) {
        MY_LOGW("invalid maxAppRaw16OutputBufferNum(%d), set to 1 at least", scenarioConfig.maxAppRaw16OutputBufferNum);
        out->CaptureParams.maxAppRaw16OutputBufferNum = 1;
    }
    else {
        out->CaptureParams.maxAppRaw16OutputBufferNum = scenarioConfig.maxAppRaw16OutputBufferNum;
    }
    MY_LOGI("out->CaptureParams:%s", policy::toString(out->CaptureParams).c_str());

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
FeatureSettingPolicy::
evaluateZslConfiguration(
    ConfigurationOutputParams& out,
    ConfigurationInputParams const& in  __unused
) -> void
{
    auto pPipelineStaticInfo        = mPolicyParams.pPipelineStaticInfo;
    auto pPipelineUserConfiguration = mPolicyParams.pPipelineUserConfiguration;
    auto pParsedAppImageStreamInfo  = pPipelineUserConfiguration->pParsedAppImageStreamInfo;
    auto pSessionParams             = &pPipelineUserConfiguration->pParsedAppConfiguration->sessionParams;

    static bool mbEnableDefaultZslMode = ::property_get_bool("persist.vendor.camera.enable.default.zsl", 0);

    // check zsl enable tag in config stage
    MUINT8 bConfigEnableZsl = false;
    if ( IMetadata::getEntry<MUINT8>(pSessionParams, MTK_CONTROL_CAPTURE_ZSL_MODE, bConfigEnableZsl) )
    {
        MY_LOGI("ZSL mode in SessionParams meta (0x%x) : %d", MTK_CONTROL_CAPTURE_ZSL_MODE, bConfigEnableZsl);
    }
    else if (mbEnableDefaultZslMode)
    {
        // get default zsl mode
        auto pMetadataProvider = NSMetadataProviderManager::valueForByDeviceId(pPipelineStaticInfo->openId);
        IMetadata::IEntry const& entry = pMetadataProvider->getMtkStaticCharacteristics()
                                         .entryFor(MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE);
        if ( !entry.isEmpty() ) {
            bConfigEnableZsl = entry.itemAt(0, Type2Type<MUINT8>());
            MY_LOGI("default ZSL mode in static meta (0x%x) : %d", MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE, bConfigEnableZsl);
        }
    } else {
        MY_LOGI("MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE has no effect : mbEnableDefaultZslMode(%d)", mbEnableDefaultZslMode);
    }
    out.isZslMode = (bConfigEnableZsl) ? true: false;

    // VR mode do not to enable ZSL
    if ( pParsedAppImageStreamInfo->hasVideoConsumer )
    {
        MY_LOGI("Force to disable ZSL in VR");
        out.isZslMode = false;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
FeatureSettingPolicy::
evaluateConfiguration(
    ConfigurationOutputParams* out,
    ConfigurationInputParams const* in
) -> int
{
    CAM_TRACE_NAME("FeatureSettingPolicy::evaluateConfiguration");
    CAM_ULOGM_APILIFE();
    //
    evaluateZslConfiguration(*out, *in);
    //
    if (CC_UNLIKELY(!evaluateCaptureConfiguration(out, in))) {
        CAM_ULOGME("evaluate capture configuration failed!");
        return -ENODEV;
    }
    if (CC_UNLIKELY(!evaluateStreamConfiguration(out, in))) {
        CAM_ULOGME("evaluate stream configuration failed!");
        return -ENODEV;
    }
    // Default connfig params for feature strategy.
    mConfigInputParams = *in;
    mConfigOutputParams = *out;
    //
    // TODO: implement ommon feature configuration here.
    // 1. P1 IMGO, RRZO, LCSO, RSSO configuration
    //    and those cache buffer account for zsd flow
    return OK;
}

bool FeatureSettingPolicy::
updateMultiCamStreamingData(
    RequestOutputParams* out __unused,
    RequestInputParams const* in __unused
)
{
    if (in->needP2CaptureNode || in->pMultiCamReqOutputParams->prvStreamingSensorList.size() > 0) {
        MY_LOGD("not yet implement for stream feature setting evaluate with capture behavior");
        return true;
    }
    auto const& sensorIdList = mPolicyParams.pPipelineStaticInfo->sensorId;
    auto const& vStreamingSensorList = in->pMultiCamReqOutputParams->streamingSensorList;
    unordered_map<uint32_t, uint32_t> dmaList;
    unordered_map<uint32_t, MetadataPtr> halMetaPtrList;
    unordered_map<uint32_t, bool> dmaEnable;
    bool isYUVReprocessing = false;
    // is yuv reprocess flow
    if(in->pRequest_ParsedAppMetaControl != nullptr){
        if(in->pRequest_ParsedAppMetaControl->control_captureIntent ==
            MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG) {
                MY_LOGD("YUV reprocessing");
                isYUVReprocessing = true;
            }
    }
    // init halMetaPtrList
    for(auto sensorId:sensorIdList)
    {
        halMetaPtrList.insert({sensorId, std::make_shared<IMetadata>()});
        dmaEnable.insert({sensorId, false});
    }
    auto queryNeedP1DmaResult = [&in, &isYUVReprocessing, this](int index, MSize yuvSize)
    {
        uint32_t needP1Dma = 0;
        // if yuv reprocessing is true, return 0 to index not equal to 0.
        // also return 0, if this feature setting policy is used to physical stream.
        if((isYUVReprocessing && index != 0)) {
            return needP1Dma;
        }

        size_t streamInfoP1Num = (*(in->pConfiguration_StreamInfo_P1)).size();
        if (static_cast<size_t>(index) >= streamInfoP1Num) {
            MY_LOGE("index:%d is out of in->pConfiguration_StreamInfo_P1 num:%zu", index, streamInfoP1Num);
            return needP1Dma;
        }
        //
        if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo != nullptr)
        {
            needP1Dma |= P1_RRZO;
        }
        auto needImgo = [] (MSize imgSize, MSize rrzoSize) -> int
        {
            return (imgSize.w > rrzoSize.w) || (imgSize.h > rrzoSize.h);
        };
        if (((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Imgo != nullptr)&&
            ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo != nullptr)&&
            (needImgo(yuvSize, (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getImgSize())))
        {
            needP1Dma |= P1_IMGO;
        }
        if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso != nullptr)
        {
            needP1Dma |= P1_LCSO;
        }
        if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rsso != nullptr)
        {
            needP1Dma |= P1_RSSO;
        }
        // for zsl case, it has to set imgo output
        if(mConfigOutputParams.isZslMode)
        {
            needP1Dma |= P1_IMGO;
        }
        return needP1Dma;
    };
    auto sensorIdToIdx = [&sensorIdList](MINT32 id)
    {
        for(size_t i=0;i<sensorIdList.size();i++)
        {
            if(sensorIdList[i] == id)
            {
                return (MINT32)i;
            }
        }
        return -1;
    };
    auto updateIspProfile = [&in, this](
                            int32_t index,
                            IMetadata* metadata)
    {
        if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
        {
            // set isp profile
            MUINT8 ispprofile = NSIspTuning::EIspProfile_N3D_Preview;
            auto scaledYuvStreamInfo = (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_ScaledYuv;
            // main1 support p1 yuv.
            if(scaledYuvStreamInfo != nullptr)
            {
                MRect yuvCropRect;
                queryVsdofP1YuvCropRect(index, scaledYuvStreamInfo, yuvCropRect);
                IMetadata::setEntry<MRect>(metadata, MTK_P1NODE_YUV_RESIZER2_CROP_REGION, yuvCropRect);
                ispprofile = StereoSettingProvider::getISPProfileForP1YUV();
            }
            // if needs update isp profile, remove previous setting first.
            metadata->remove(MTK_3A_ISP_PROFILE);
            IMetadata::setEntry<MUINT8>(metadata, MTK_3A_ISP_PROFILE, ispprofile);
        }
        // if camsv path, it has to set hal metadata to covert mono img to yuv.
        if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo == nullptr)
        {
            IMetadata::setEntry<MINT32>(metadata, MTK_ISP_P2_IN_IMG_FMT, 5);
        }
        return true;
    };
    auto updateSensorCrop = [&in, this](
                            int32_t index,
                            IMetadata* metadata)
    {
        if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
        {
            // vsdof crop info is compute by StereSizeProvider.
            MRect crop;
            queryVsdofSensorCropRect(index, in, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_P1NODE_SENSOR_CROP_REGION, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_DUALZOOM_CAP_CROP, crop);
            queryVsdof3APreviewCropRect(index, in, crop);
            IMetadata::setEntry<MRect>(metadata, MTK_3A_PRV_CROP_REGION, crop);
        }
        else
        {
            uint32_t sensorId = mPolicyParams.pPipelineStaticInfo->sensorId[index];
            MRect tgCrop, scalerCrop;
            bool isCapture = false;
            if(queryMulticamTgCrop(in, sensorId, tgCrop, isCapture))
            {
                IMetadata::setEntry<MRect>(metadata, MTK_P1NODE_SENSOR_CROP_REGION, tgCrop);
                IMetadata::setEntry<MRect>(metadata, MTK_DUALZOOM_CAP_CROP, tgCrop);
                IMetadata::setEntry<MRect>(metadata, MTK_3A_PRV_CROP_REGION, tgCrop);
            }
            if(queryMulticamScalerCrop(in, sensorId, scalerCrop, isCapture))
            {
                IMetadata::setEntry<MRect>(metadata, MTK_SENSOR_SCALER_CROP_REGION, scalerCrop);
            }
        }
        return true;
    };
    // duplicate main1 metadata to each other.
    if(sensorIdList.size() > 1)
    {
        MINT32 index = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
        auto halMeta = out->mainFrame->additionalHal[index];
        auto appMeta = out->mainFrame->additionalApp;
        for(size_t i=1;i<sensorIdList.size();++i)
        {
            auto sensorId = sensorIdList[i];
            updateRequestResultParams(out->mainFrame,
                                  appMeta,
                                  halMeta,
                                  0,
                                  sensorIdToIdx(sensorId));
        }
    }
    // check sensor control
    {
        MSize imgSize = in->maxP2StreamSize;
        for(size_t i=0;i<sensorIdList.size();i++)
        {
            auto sensorId = sensorIdList[i];
            uint32_t dma = queryNeedP1DmaResult(i, imgSize);
            updateMulticamSensorControl(in, sensorId, halMetaPtrList[sensorId], dma);
            auto iter = dmaList.find(sensorId);
            if(iter != dmaList.end())
            {
                iter->second |= dma;
            }
            else
            {
                dmaList.insert({sensorId, dma});
            }
            // if dma value is 0, means no need output.
            if(dma != 0)
            {
                dmaEnable.emplace(sensorId, true);
            }
        }
    }

    // update metadata
    if(vStreamingSensorList.size() > 1)
    {
        // a. query sync infomation.
        MINT64 hwsyncToleranceTime = ::getFrameSyncToleranceTime();
        MINT32 syncFailBehavior = MTK_FRAMESYNC_FAILHANDLE_CONTINUE;
        // b. build master & slave tag
        IMetadata::IEntry msTag(MTK_STEREO_SYNC2A_MASTER_SLAVE);
        auto masterId = in->pMultiCamReqOutputParams->masterId;
        msTag.push_back(masterId, Type2Type<MINT32>());
        for(auto&& sensorId:vStreamingSensorList)
        {
            if(masterId != sensorId)
                msTag.push_back(sensorId, Type2Type<MINT32>());
        }
        for(auto&& sensorId:vStreamingSensorList)
        {
            auto iter = halMetaPtrList.find(sensorId);
            if(iter != halMetaPtrList.end() && iter->second != nullptr)
            {
                // set sync id.
                int32_t index = sensorIdToIdx(sensorId);
                MINT32 syncType = Utils::Imp::HW_RESULT_CHECK;
                if(index == -1) continue;
                {
                    // sync target for synchelper
                    IMetadata::IEntry tag(MTK_FRAMESYNC_ID);
                    for(auto&& syncId:vStreamingSensorList) {
                        if(iter->first != syncId) {
                            tag.push_back(syncId, Type2Type<MINT32>());
                        }
                    }
                    iter->second->update(tag.tag(), tag);
                }
                {
                    // synchelper
                    if(in->pMultiCamReqOutputParams->needSynchelper_3AEnq)
                    {
                        syncType |= Utils::Imp::ENQ_HW;
                    }
                    // set tolerance time
                    IMetadata::setEntry<MINT64>(iter->second.get(), MTK_FRAMESYNC_TOLERANCE, hwsyncToleranceTime);
                    // set sync fail behavior
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_FRAMESYNC_FAILHANDLE, syncFailBehavior);
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_FRAMESYNC_TYPE, syncType);
                }
                if(in->pMultiCamReqOutputParams->needSync2A) {
                    if(miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
                        IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF_BY_FRAME);
                    else
                        IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM_BY_FRAME);
                }
                else {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE);
                }
                if(in->pMultiCamReqOutputParams->needSyncAf) {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNCAF_MODE, NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON);
                }
                else {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNCAF_MODE, NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF);
                }
                if(in->pMultiCamReqOutputParams->needFramesync) {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON);
                }
                if(in->pMultiCamReqOutputParams->need3ASwitchDoneNotify) {
                    IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_NOTIFY,
                                                    (1<<NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON));
                    MY_LOGI("set 3a notify");
                }
                // set master & slave
                iter->second->update(msTag.tag(), msTag);
                // set isp profile
                updateIspProfile(index, iter->second.get());
                // update sensor crop
                updateSensorCrop(index, iter->second.get());
            }
        }
    }
    else
    {
        for(auto&& sensorId:vStreamingSensorList)
        {
            auto iter = halMetaPtrList.find(sensorId);
            if(iter != halMetaPtrList.end() && iter->second != nullptr)
            {
                int32_t index = sensorIdToIdx(sensorId);
                if(index == -1) continue;
                // set isp profile
                updateIspProfile(index, iter->second.get());
                // update sensor crop
                updateSensorCrop(index, iter->second.get());
                //If sensor less than 2, force SYNC2A and SYNCAF to OFF
                IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNC2A_MODE, NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE);
                IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_SYNCAF_MODE, NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF);
                IMetadata::setEntry<MINT32>(iter->second.get(), MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF);
            }
        }
    }
    // reset dma value according to dmaEnable.
    for(auto&& dma:dmaEnable)
    {
        if(!dma.second) {
            int32_t index = sensorIdToIdx(dma.first);
            if(index >= 0)
                out->mainFrame->needP1Dma[index] = 0;
            else
                MY_LOGA("SensorId to idx is negative, please check flow");
            }
    }
    // decide need skip depth compute or not
    bool needSkipSubImage = (in->requestNo % mPeriodForStreamingProcessDepth) != 0;
    out->mainFrame->needP2Process.resize(mSensorCount);
    for(auto&& sensorId:sensorIdList)
    {
        auto idx = sensorIdToIdx(sensorId);
        if(idx == -1)
        {
            MY_LOGA("SensorId to idx is -1, please check flow");
        }
        if(needSkipSubImage && idx != 0) // idx != 0 means not master cam.
        {
            out->mainFrame->needP2Process[idx] = 0;
        }
        else
        {
            out->mainFrame->needP2Process[idx] |= P2_STREAMING;
        }
        updateRequestResultParams(out->mainFrame,
                              nullptr,
                              halMetaPtrList[sensorId],
                              dmaList[sensorId],
                              sensorIdToIdx(sensorId));
    }
    return true;
}

bool
FeatureSettingPolicy::
queryVsdofSensorCropRect(
    int32_t index,
    RequestInputParams const* in,
    MRect& sensorCrop
)
{
    bool ret = false;
    // get rrzo format
    auto rrzoStreamInfo = (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo;
    // get sensor crop
    MUINT32 q_stride;
    NSCam::MSize size;
    if(rrzoStreamInfo != nullptr)
    {
        StereoSizeProvider::getInstance()->getPass1Size(
                mPolicyParams.pPipelineStaticInfo->sensorId[index],
                (EImageFormat)rrzoStreamInfo->getImgFormat(),
                NSImageio::NSIspio::EPortIndex_RRZO,
                StereoHAL::eSTEREO_SCENARIO_PREVIEW, // in this mode, stereo only support zsd.
                (MRect&)sensorCrop,
                size,
                q_stride);
    }
    else
    {
        // if rrzo is null, assign imgo crop size
        auto imgoStreamInfo = (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Imgo;
        StereoSizeProvider::getInstance()->getPass1Size(
                mPolicyParams.pPipelineStaticInfo->sensorId[index],
                (EImageFormat)imgoStreamInfo->getImgFormat(),
                NSImageio::NSIspio::EPortIndex_IMGO,
                StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
                (MRect&)sensorCrop,
                size,
                q_stride);
    }
    ret = true;
    return ret;
}
bool
FeatureSettingPolicy::
queryVsdof3APreviewCropRect(
    int32_t index,
    RequestInputParams const* in,
    MRect& sensorCrop
)
{
    ENUM_STEREO_SENSOR sensor_enum;
    switch(index)
    {
        case 0:
            sensor_enum = StereoHAL::eSTEREO_SENSOR_MAIN1;
            break;
        case 1:
            sensor_enum = StereoHAL::eSTEREO_SENSOR_MAIN2;
            break;
        default:
            MY_LOGA("not support index(%d)", index);
            return false;
    }
    // get rrzo format
    auto RrzoStreamInfo = (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo;
    // get sensor crop
    if(RrzoStreamInfo != nullptr)
    {
        StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(
                sensor_enum,
                sensorCrop);
    }
    else
    {
        // if rrzo is null, asign imgo crop size
        StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(
                sensor_enum,
                sensorCrop,
                false);
    }
    return true;
}
bool
FeatureSettingPolicy::
queryVsdofP1YuvCropRect(
    int32_t index,
    sp<IImageStreamInfo>& imgStreamInfo,
    MRect& yuvCropRect
)
{
    bool ret = false;
    MUINT32 q_stride;
    NSCam::MSize size;

    if(imgStreamInfo == nullptr)
    {
        MY_LOGE("Should not happened!");
        goto lbExit;
    }
    StereoSizeProvider::getInstance()->getPass1Size(
            mPolicyParams.pPipelineStaticInfo->sensorId[index],
            (EImageFormat)imgStreamInfo->getImgFormat(),
            NSImageio::NSIspio::EPortIndex_CRZO_R2,
            StereoHAL::eSTEREO_SCENARIO_PREVIEW, // in this mode, stereo only support zsd.
            (MRect&)yuvCropRect,
            size,
            q_stride);
    ret = true;
lbExit:
    return ret;
}

auto
FeatureSettingPolicy::
evaluateRequest(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> int
{
    CAM_TRACE_NAME("FeatureSettingPolicy::evaluateRequest");
    //CAM_ULOGM_APILIFE();
    // check setting valid.
    if CC_UNLIKELY(out == nullptr || in == nullptr) {
        CAM_ULOGME("invalid in(%p), out(%p) for evaluate", in, out);
        return -ENODEV;
    }

    if (!collectDefaultSensorInfo(out, in)) {
        MY_LOGE("collectDefaultSensorInfo failed!");
        return -ENODEV;
    }

    // P2 capture feature policy
    if (!evaluateCaptureSetting(out, in)) {
        MY_LOGE("evaluateCaptureSetting failed!");
        return -ENODEV;
    }

    // P2 streaming feature policy
    if (!evaluateStreamSetting(out, in)) {
        MY_LOGE("evaluateStreamSetting failed!");
        return -ENODEV;
    }

    // update needReconfiguration info.
    if (!evaluateReconfiguration(out, in)) {
        MY_LOGE("evaluateReconfiguration failed!");
        return -ENODEV;
    }

    // dump output request params for debug.
    dumpRequestOutputParams(out, in, mbDebug);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace featuresetting {
auto createFeatureSettingPolicyInstance(
    CreationParams const& params
) -> std::shared_ptr<IFeatureSettingPolicy>
{
    CAM_ULOGM_APILIFE();
    // check the policy params is valid.
    if (CC_UNLIKELY(params.pPipelineStaticInfo.get() == nullptr)) {
        CAM_ULOGME("pPipelineStaticInfo is invalid nullptr");
        return nullptr;
    }
    if (CC_UNLIKELY(params.pPipelineUserConfiguration.get() == nullptr)) {
        CAM_ULOGME("pPipelineUserConfiguration is invalid nullptr");
        return nullptr;
    }
    int32_t openId = params.pPipelineStaticInfo->openId;
    if (CC_UNLIKELY(openId < 0)) {
        CAM_ULOGME("openId is invalid(%d)", openId);
        return nullptr;
    }
    if (CC_UNLIKELY(params.pPipelineStaticInfo->sensorId.empty())) {
        CAM_ULOGME("sensorId is empty(size:%zu)", params.pPipelineStaticInfo->sensorId.size());
        return nullptr;
    }
    for (unsigned int i=0; i<params.pPipelineStaticInfo->sensorId.size(); i++) {
        int32_t sensorId = params.pPipelineStaticInfo->sensorId[i];
        CAM_ULOGMD("sensorId[%d]=%d", i, sensorId);
        if (CC_UNLIKELY(sensorId < 0)) {
            CAM_ULOGME("sensorId is invalid(%d)", sensorId);
            return nullptr;
        }
    }
    // you have got an instance for feature setting policy.
    return std::make_shared<FeatureSettingPolicy>(params);
}
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
