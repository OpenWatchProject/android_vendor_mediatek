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

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mtkcam-FeatureSettingPolicy_ISP"

//
#include <algorithm>
#include <tuple>
// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// MTKCAM3
#include "FeatureSettingPolicy_ISP.h"
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FEATURE_SETTING_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::v3::pipeline::policy::featuresetting_isp;
using namespace NSCam::v3::pipeline::policy::scenariomgr;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

// TODO: rename
#define SENSOR_INDEX_MAIN   (0)
#define SENSOR_INDEX_SUB1   (1)

// TODO: disable thsi flag before MP
#define DEBUG_FEATURE_SETTING_POLICY  (0)

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
class PluginWrapper
{
public:
    using PluginPtr        = typename TPlugin::Ptr;
    using ProviderPtr      = typename TPlugin::IProvider::Ptr;
    using InterfacePtr     = typename TPlugin::IInterface::Ptr;
    using SelectionPtr     = typename TPlugin::Selection::Ptr;
    using Selection        = typename TPlugin::Selection;
    using Property         = typename TPlugin::Property;
    using RecommendedParam = typename TPlugin::RecommendedParam;
    using ProviderPtrs     = std::vector<ProviderPtr>;
public:
    PluginWrapper(const std::string& name, MUINT64 uSupportedFeatures = ~0, MINT32 iUniqueKey = -1);
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
    auto pushSelection(const uint8_t frameCount) -> MVOID;
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
};

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
PluginWrapper<TPlugin>::
PluginWrapper(const std::string& name, MUINT64 uSupportedFeatures, MINT32 uniqueKey)
: mName(name)
, mSupportedFeatures(uSupportedFeatures)
, mUniqueKey(uniqueKey)
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
pushSelection(const uint8_t frameCount) -> MVOID
{
    for (auto item : mTempSelectionPtrMap) {
        auto providerPtr = item.first;
        auto vSelection  = item.second;
        MY_LOGD("%s: selection size:%zu, frameCount:%d", getName().c_str(), vSelection.size(), frameCount);
        if (frameCount > 1 && vSelection.size() == 1) {
            auto sel = vSelection.front();
            for (size_t i = 0; i < frameCount; i++) {
                MY_LOGD("%s: duplicate selection for multiframe(count:%d, index:%zu)", getName().c_str(), frameCount, i);
                mInstancePtr->pushSelection(providerPtr, sel);
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
class FeatureSettingPolicy_ISP::CLASSNAME final: public PluginWrapper<NSCam::NSPipelinePlugin::PLUGINNAME>                  \
{                                                                                                                       \
public:                                                                                                                 \
    /*Feature Provider*/                                                                                        \
    CLASSNAME(MUINT64 uSupportedFeatures, MINT32 iUniqueKey)                           \
    : PluginWrapper<NSCam::NSPipelinePlugin::PLUGINNAME>(#PLUGINNAME, uSupportedFeatures, iUniqueKey)\
    {                                                                                                                   \
    }                                                                                                                   \
}
DEFINE_PLUGINWRAPER(MFPPluginWrapper, MultiFramePlugin);
DEFINE_PLUGINWRAPER(RawPluginWrapper, RawPlugin);
#undef DEFINE_PLUGINWRAPER

/******************************************************************************
 *
 ******************************************************************************/
FeatureSettingPolicy_ISP::
FeatureSettingPolicy_ISP(
    CreationParams const& params
)
    :mPolicyParams(params)
{
    CAM_ULOGM_APILIFE();
    MY_LOGI("(%p) ctor, openId(%d), sensors_count(%zu)",
            this,
            mPolicyParams.openId,
            mPolicyParams.sensorId.size());
    mbDebug = ::property_get_int32("vendor.debug.camera.featuresetting.log", DEBUG_FEATURE_SETTING_POLICY);
    // forced feature strategy for debug
    mForcedKeyFeatures = ::property_get_int64("vendor.debug.featuresetting.keyfeature", -1);
    mForcedFeatureCombination = ::property_get_int64("vendor.debug.featuresetting.featurecombination", -1);

    mForcedIspTuningHint = ::property_get_int32("vendor.debug.featuresetting.isptuninghint", -1);
}

FeatureSettingPolicy_ISP::
~FeatureSettingPolicy_ISP()
{
    MY_LOGI("(%p) dtor", this);
}

auto
FeatureSettingPolicy_ISP::
collectParsedStrategyInfo(
    RequestParams* requestParams,
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    //CAM_ULOGM_APILIFE();
    bool ret = true;

    // collect parsed info for common strategy (only per-frame requirement)
    // Note: It is per-frames query behavior here.
    // check request type
    int8_t frameCount = requestParams->frameCount;
    int8_t frameIndex = requestParams->frameIndex;
    bool isMultipleFrame = (frameCount > 1);

    if (frameIndex == 0) {
        // to keep output requirement info by main frame for multiple frame's sub-frames
        mIsOutputRaw = (requestParams->pAppImage_Output_RAW16 != nullptr);
        mIsOutputYuv = (requestParams->pAppImage_Output_Yuv != nullptr);
        mIsOutputJpeg = (requestParams->pAppImage_Output_Jpeg != nullptr);
        MY_LOGD("update and keep output(raw:%d, yuv:%d, jpeg:%d) requirement info by main frame", mIsOutputRaw, mIsOutputYuv, mIsOutputJpeg);
    }
    else {
        MY_LOGD("use previous output(raw:%d, yuv:%d, jpeg:%d) requirement info for sub-frame strategy", mIsOutputRaw, mIsOutputYuv, mIsOutputJpeg);
    }

    bool isInputRaw = (requestParams->pAppImage_Input_RAW16 != nullptr || requestParams->pAppImage_Input_Priv != nullptr);
    bool isInputYuv = (requestParams->pAppImage_Input_Yuv != nullptr);
    bool isOutputRaw = mIsOutputRaw;
    bool isOutputYuv = mIsOutputYuv;
    bool isOutputJpeg = mIsOutputJpeg;
    MY_LOGD("isp processing format(req#:%d): input(isMultipleFrame:%d(count:%d, index:%d), isRaw:%d, isYuv:%d), output(isRaw:%d, isYuv:%d, isJpeg:%d)",
            requestParams->requestNo, isMultipleFrame, frameCount, frameIndex, isInputRaw, isInputYuv, isOutputRaw, isOutputYuv, isOutputJpeg);
    //
    if (isInputRaw && isInputYuv) {
        MY_LOGW("not support input different format buffers for isp processing!");
        ret = false;
    }
    //
    if (isMultipleFrame) {
        // Multiple Frame Processing Type
        if (isInputRaw && isOutputJpeg) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_MultipleRawIn_JpegOut;
        }
        else if (isInputRaw && isOutputYuv) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_MultipleRawIn_YuvOut;
        }
        else if (isInputRaw && isOutputRaw) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_MultipleRawIn_RawOut;
        }
        else if (isInputYuv && isOutputJpeg) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_MultipleYuvIn_JpegOut;
        }
        else if (isInputYuv && isOutputYuv) {
            // not support yet, MFNR IQ must input multiple raw and output yuv.
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_NotSupport;
        }
        else {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_NotSupport;
        }
    }
    else {
        // Single Frame Processing Type
        if (isInputRaw && isOutputJpeg) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_SingleRawIn_JpegOut;
        }
        else if (isInputRaw && isOutputYuv) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_SingleRawIn_YuvOut;
        }
        else if (isInputYuv && isOutputJpeg) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_SingleYuvIn_JpegOut;
        }
        else if (isInputYuv && isOutputYuv) {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_SingleYuvIn_YuvOut;
        }
        else {
            parsedInfo.ispProcessInOutType = eIspProcessInOutType_NotSupport;
        }
    }

    // get info from AppControl Metadata
    {
        auto pAppMetaControl = requestParams->pRequest_AppControl;
        MINT32 cshot = 0;
        if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CSHOT_FEATURE_CAPTURE, cshot)) {
            if (cshot) parsedInfo.isCShot = true;
        }

        MINT32 isp_tuning_hint = -1;
        if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, isp_tuning_hint)) {
            MY_LOGD("get metadata MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING:%d", isp_tuning_hint);
            parsedInfo.ispTuningHint = isp_tuning_hint;
        }
        if (mForcedIspTuningHint >= 0) {
            parsedInfo.ispTuningHint = mForcedIspTuningHint;
            MY_LOGI("force ispTuningHint(%d) for feature setting policy", parsedInfo.ispTuningHint);
        }
    }
    // update free memory state for debug
    if (frameCount == 1 || frameIndex == 0) {
        mCurrentFreeMemoryMBytes = (NSCam::NSMemoryInfo::getFreeMemorySize()/1024/1024); // convert Bytes to MB
        MY_LOGD("update current free memory state:%dMB", mCurrentFreeMemoryMBytes);
    }
    parsedInfo.freeMemoryMBytes = mCurrentFreeMemoryMBytes;

    MY_LOGD("strategy info for isp feature(process type:%d, isCShot:%d),  freeMem:%dMB",
            parsedInfo.ispProcessInOutType, parsedInfo.isCShot, parsedInfo.freeMemoryMBytes);

    if (parsedInfo.ispProcessInOutType > eIspProcessInOutType_Support_List_Size) {
        MY_LOGW("unknow isp input/out requirement(0x%X). input(frameCount:%d, isRaw:%d, isYuv:%d), output(isRaw:%d, isYuv:%d, isJpeg:%d)",
                parsedInfo.ispProcessInOutType, frameCount, isInputRaw, isInputYuv, isOutputRaw, isOutputYuv, isOutputJpeg);
        ret = false;
    }

    return ret;
}

auto
FeatureSettingPolicy_ISP::
updateCaptureDebugInfo(
    RequestParams* requestParams,
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    // update frames' Hal Metadata for debug exif
    auto pHalMeta = requestParams->pRequest_HalResult;
    if (CC_UNLIKELY(pHalMeta == nullptr)) {
        MY_LOGE("pHalMeta(%p) is invalid nullptr!!", pHalMeta);
        return false;
    }

    // update debug info here.
    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_FREE_MEMORY_MBYTE, parsedInfo.freeMemoryMBytes);

    return true;
}

auto
FeatureSettingPolicy_ISP::
updateRequestResultParams(
    RequestParams* requestParams,
    MetadataPtr pOutMetaApp_Additional,
    MetadataPtr pOutMetaHal_Additional,
    MINT64 featureCombination,
    MINT32 requestIndex,
    MINT32 requestCount
) -> bool
{
    //CAM_ULOGM_APILIFE();
    if (requestParams == nullptr) {
        MY_LOGE("invalid nullptr! (requestParams:%p)", requestParams);
        return false;
    }
    //
    MY_LOGD_IF(2 <= mbDebug, "updateRequestResultParams for req#:%d", requestParams->requestNo);
    //
    auto pOutMetaApp = requestParams->pRequest_AppControl;
    auto pOutMetaHal = requestParams->pRequest_HalResult;
    if (pOutMetaApp == nullptr || pOutMetaHal == nullptr) {
        MY_LOGE("invalid nullptr! (pOutMetaApp:%p, pOutMetaHal:%p)", pOutMetaApp, pOutMetaHal);
        return false;
    }

    if (pOutMetaApp_Additional.get() != nullptr) {
        *pOutMetaApp += *pOutMetaApp_Additional;
    }
    if (pOutMetaHal_Additional.get() != nullptr) {
        *pOutMetaHal += *pOutMetaHal_Additional;
    }
    if (featureCombination != -1) {
        MY_LOGD_IF(2 <= mbDebug, "update featureCombination=%#" PRIx64"", featureCombination);
        IMetadata::setEntry<MINT64>(pOutMetaHal, MTK_FEATURE_CAPTURE, featureCombination);
    }
    if (requestIndex || requestCount) {
        MY_LOGD_IF(2 <= mbDebug, "update MTK_HAL_REQUEST_INDEX=%d, MTK_HAL_REQUEST_COUNT=%d", requestIndex, requestCount);
        IMetadata::setEntry<MINT32>(pOutMetaHal, MTK_HAL_REQUEST_INDEX, requestIndex);
        IMetadata::setEntry<MINT32>(pOutMetaHal, MTK_HAL_REQUEST_COUNT, requestCount);
    }

    //check ISP profile for debug
    MUINT8 ispProfile = 0;
    if (IMetadata::getEntry<MUINT8>(pOutMetaHal, MTK_3A_ISP_PROFILE, ispProfile)) {
        MY_LOGI("not suggest set isp profile here! It has been set to %d", ispProfile);
    }
    // dump for debug
    if (CC_UNLIKELY(2 <= mbDebug)) {
        (*pOutMetaApp).dump();
        (*pOutMetaHal).dump();
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
queryPolicyState(
    RequestParams* requestParams,
    Policy::StateIspHidl& state,
    ParsedStrategyInfo const& parsedInfo
) -> bool
{
    state.requestNo  = requestParams->requestNo;
    state.frameCount = requestParams->frameCount;
    state.frameIndex = requestParams->frameIndex;

    state.pAppImage_Input_RAW16 = requestParams->pAppImage_Input_RAW16;
    state.pAppImage_Input_Priv = requestParams->pAppImage_Input_Priv;
    state.pAppImage_Input_Yuv = requestParams->pAppImage_Input_Yuv;
    state.pAppImage_Output_RAW16 = requestParams->pAppImage_Output_RAW16;
    state.pAppImage_Output_Yuv = requestParams->pAppImage_Output_Yuv;
    state.pAppImage_Output_Jpeg = requestParams->pAppImage_Output_Jpeg;
    state.mFreeMemoryMBytes = parsedInfo.freeMemoryMBytes;
    state.sensorId = mPolicyParams.openId;

    return true;
}

auto
FeatureSettingPolicy_ISP::
updatePolicyDecision(
    RequestParams* requestParams __unused,
    Policy::DecisionIspHidl const& decision __unused
) -> bool
{
    // TBD: reserved
    return true;
}

auto
FeatureSettingPolicy_ISP::
strategyMultiFramePlugin(
    RequestParams* requestParams,
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    auto provider = mMFPPluginWrapperPtr->getProvider(combinedKeyFeature, foundFeature);
    if (provider) {
        // for MultiFramePlugin key feature (ex: HDR, MFNR, 3rd party multi-frame algo,etc )
        // negotiate and query feature requirement
        if (requestParams == nullptr) {
            MY_LOGE("invalid nullptr! (requestParams:%p)", requestParams);
            return false;
        }

        auto property =  provider->property();
        auto pSelection = mMFPPluginWrapperPtr->createSelection();
        MFP_Selection& sel = *pSelection;
        sel.mRequestIndex = requestParams->frameIndex;
        mMFPPluginWrapperPtr->offer(sel);

        // update app metadata for plugin reference
        auto pAppMetaControl = requestParams->pRequest_AppControl;
        MetadataPtr pInMetaApp = std::make_shared<IMetadata>(*pAppMetaControl);
        sel.mIMetadataApp.setControl(pInMetaApp);
        // update hal metadata for plugin reference
        auto pHalMeta = requestParams->pRequest_HalResult;
        MetadataPtr pInMetaHal = std::make_shared<IMetadata>(*pHalMeta);
        sel.mIMetadataHal.setControl(pInMetaHal);

        sel.mIspHidlStage = ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL;
        // update previous Hal ouput for plugin reference
        // query state  for plugin provider negotiate
        if (!queryPolicyState(requestParams, sel.mStateIspHidl, parsedInfo)) {
            MY_LOGE("cannot query state for plugin provider negotiate!");
            return false;
        }
        if (provider->negotiate(sel) == OK) {
            sel.mRequestCount = requestParams->frameCount;
            MY_LOGD("MultiFrame request process count:%d, index:%d", sel.mStateIspHidl.frameCount, sel.mStateIspHidl.frameIndex);
            if (!updatePolicyDecision(requestParams, sel.mDecisionIspHidl)) {
                MY_LOGW("update config info failed!");
                return false;
            }
            //
            pSelection->mTokenPtr = MFP_Selection::createToken(mPipelineUniqueKey, requestParams->requestNo, requestParams->frameIndex, mPolicyParams.openId);
            mMFPPluginWrapperPtr->keepSelection(requestParams->requestNo, provider, pSelection);
            //
            MetadataPtr pOutMetaApp_Additional = sel.mIMetadataApp.getAddtional();
            MetadataPtr pOutMetaHal_Additional = sel.mIMetadataHal.getAddtional();
            updateRequestResultParams(
                    requestParams,
                    pOutMetaApp_Additional,
                    pOutMetaHal_Additional,
                    featureCombination,
                    sel.mStateIspHidl.frameIndex,
                    sel.mStateIspHidl.frameCount);

            MY_LOGD("%s(%s), trigger provider(count:%d, index:%d) for foundFeature(%#" PRIx64")",
                    mMFPPluginWrapperPtr->getName().c_str(), property.mName, sel.mStateIspHidl.frameCount, sel.mStateIspHidl.frameIndex, foundFeature);
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
FeatureSettingPolicy_ISP::
strategySingleRawPlugin(
    RequestParams* requestParams,
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    auto provider = mRawPluginWrapperPtr->getProvider(combinedKeyFeature, foundFeature);
    if (provider) {
        // for RawPlugin key feature (ex: SW 4Cell) negotiate and query feature requirement
        auto property =  provider->property();
        auto pSelection = mRawPluginWrapperPtr->createSelection();
        Raw_Selection& sel = *pSelection;
        mRawPluginWrapperPtr->offer(sel);
        // update app metadata for plugin reference
        auto pAppMetaControl = requestParams->pRequest_AppControl;
        MetadataPtr pInMetaApp = std::make_shared<IMetadata>(*pAppMetaControl);
        sel.mIMetadataApp.setControl(pInMetaApp);
        // update hal metadata for plugin reference
        auto pHalMeta = requestParams->pRequest_HalResult;
        MetadataPtr pInMetaHal = std::make_shared<IMetadata>(*pHalMeta);
        sel.mIMetadataHal.setControl(pInMetaHal);

        sel.mIspHidlStage = ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL;
        // query state  for plugin provider strategy
        if (!queryPolicyState(requestParams, sel.mStateIspHidl, parsedInfo)) {
            MY_LOGE("cannot query state for plugin provider negotiate!");
            return false;
        }
        if (provider->negotiate(sel) == OK) {
            if (!updatePolicyDecision(requestParams, sel.mDecisionIspHidl)) {
                MY_LOGW("update config info failed!");
                return false;
            }
            //
            pSelection->mTokenPtr = Raw_Selection::createToken(mPipelineUniqueKey, requestParams->requestNo, requestParams->frameIndex, mPolicyParams.openId);
            mRawPluginWrapperPtr->keepSelection(requestParams->requestNo, provider, pSelection);
            //
            MetadataPtr pOutMetaApp_Additional = sel.mIMetadataApp.getAddtional();
            MetadataPtr pOutMetaHal_Additional = sel.mIMetadataHal.getAddtional();
            updateRequestResultParams(
                    requestParams,
                    pOutMetaApp_Additional,
                    pOutMetaHal_Additional,
                    featureCombination,
                    sel.mStateIspHidl.frameIndex,
                    sel.mStateIspHidl.frameCount);
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
FeatureSettingPolicy_ISP::
strategyNormalSingleCapture(
    RequestParams* requestParams,
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    ParsedStrategyInfo& parsedInfo __unused
) -> bool
{
    if (requestParams->frameCount > 1) {
        MY_LOGD_IF(mbDebug, "frames setting has been updated by multiframe plugin");
        return true;
    }

    CAM_ULOGM_APILIFE();

    // general single frame capture's sub feature combination and requirement
    // update request result (frames metadata)
    updateRequestResultParams(
        requestParams,
        nullptr, /* no additional metadata from provider*/
        nullptr, /* no additional metadata from provider*/
        featureCombination,
        requestParams->frameIndex,
        requestParams->frameCount);

    MY_LOGD_IF(mbDebug, "trigger single frame feature:%#" PRIx64", feature combination:%#" PRIx64"",
            combinedKeyFeature, featureCombination);
    return true;
}

auto
FeatureSettingPolicy_ISP::
dumpRequestOutputParams(
    RequestParams* requestParams,
    bool forcedEnable = false
) -> bool
{
    CAM_ULOGM_APILIFE();
    auto requestNo  = requestParams->requestNo;
    auto frameCount = requestParams->frameCount;
    auto frameIndex = requestParams->frameIndex;

    // dump frame procsss info
    {
        // dump MTK_FEATURE_CAPTURE info
        MINT64 featureCombination = 0;
        if (IMetadata::getEntry<MINT64>(requestParams->pRequest_HalResult, MTK_FEATURE_CAPTURE, featureCombination)) {
            MY_LOGD("req#:%d(count:%d, index:%d) featureCombination=%#" PRIx64"", requestNo, frameCount, frameIndex, featureCombination);
        }
        else {
            MY_LOGW("req#:%d(count:%d, index:%d) w/o featureCombination", requestNo, frameCount, frameIndex);
        }
        //
        if (CC_UNLIKELY(forcedEnable)) {
            MY_LOGD("req#:%d(count:%d, index:%d) dump app control metadata", requestNo, frameCount, frameIndex);
            requestParams->pRequest_AppControl->dump();
            MY_LOGD("req#:%d(count:%d, index:%d) dump app result metadata", requestNo, frameCount, frameIndex);
            requestParams->pRequest_AppResult->dump();
            MY_LOGD("req#:%d(count:%d, index:%d) dump hal control/result metadata", requestNo, frameCount, frameIndex);
            requestParams->pRequest_HalResult->dump();
        }

        MY_LOGD("buffers for isp hidl reprocessing, input(priv_raw:%p, raw16:%p, yuv:%p), output(raw16:%p, yuv:%p, jpeg:%p)",
                requestParams->pAppImage_Input_Priv.get(),
                requestParams->pAppImage_Input_RAW16.get(),
                requestParams->pAppImage_Input_Yuv.get(),
                requestParams->pAppImage_Output_RAW16.get(),
                requestParams->pAppImage_Output_Yuv.get(),
                requestParams->pAppImage_Output_Jpeg.get());
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
updatePluginSelection(
    bool isFeatureTrigger,
    uint8_t frameCount
) -> bool
{
    if (isFeatureTrigger) {
        mMFPPluginWrapperPtr->pushSelection(frameCount);
        mRawPluginWrapperPtr->pushSelection(frameCount);
    }
    else {
        mMFPPluginWrapperPtr->cancel();
        mRawPluginWrapperPtr->cancel();
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
strategyCaptureFeature(
    RequestParams* requestParams,
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64 featureCombination, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    CAM_ULOGM_APILIFE();
    MY_LOGD("strategy for combined key feature(%#" PRIx64"), feature combination(%#" PRIx64")",
            combinedKeyFeature, featureCombination);

    if (CC_UNLIKELY(mForcedKeyFeatures >= 0)) {
        combinedKeyFeature = mForcedKeyFeatures;
        MY_LOGW("forced key feature(%#" PRIx64") for debug", combinedKeyFeature);
    }
    if (CC_UNLIKELY(mForcedFeatureCombination >= 0)) {
        featureCombination = mForcedFeatureCombination;
        MY_LOGW("forced feature combination(%#" PRIx64") for debug", featureCombination);
    }
    //
    MINT64 foundFeature = 0;
    if (combinedKeyFeature) { /* not MTK_FEATURE_NORMAL */
        MINT64 checkFeatures = combinedKeyFeature;
        //
        do {
            if (!strategySingleRawPlugin(requestParams, checkFeatures, featureCombination, foundFeature, parsedInfo)) {
                MY_LOGD("no need to trigger feature(%#" PRIx64") for features(key:%#" PRIx64", combined:%#" PRIx64")",
                        foundFeature, combinedKeyFeature, featureCombination);
                return false;
            }
            checkFeatures &= ~foundFeature;
        } while (foundFeature && checkFeatures); // to find next raw plugin until no foundfeature(==0)
        //
        MY_LOGD_IF(checkFeatures, "continue to find next plugin for %#" PRIx64"", checkFeatures);
        //
        if (!strategyMultiFramePlugin(requestParams, checkFeatures, featureCombination, foundFeature, parsedInfo)) {
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
    if (!strategyNormalSingleCapture(requestParams, combinedKeyFeature, featureCombination, parsedInfo)) {
        MY_LOGW("update capture setting failed!");
        return false;
    }
    //
    //
    if (!updateCaptureDebugInfo(requestParams, parsedInfo)) {
        MY_LOGW("updateCaptureDebugInfo failed!");
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
evaluateCaptureSetting(
    RequestParams* requestParams,
    ParsedStrategyInfo& parsedInfo
) -> bool
{
    CAM_TRACE_CALL();
    CAM_ULOGM_APILIFE();

    MY_LOGI("(%p) capture req#:%u (count:%d, index:%d)",
            this, requestParams->requestNo, requestParams->frameCount, requestParams->frameIndex);

    ScenarioFeatures scenarioFeatures;
    CaptureScenarioConfig scenarioConfig;
    ScenarioHint scenarioHint;
    scenarioHint.isIspHidl = true;
    scenarioHint.ispTuningHint = parsedInfo.ispTuningHint;
    scenarioHint.ispProcessInOutType = parsedInfo.ispProcessInOutType;
    scenarioHint.isCShot = parsedInfo.isCShot;
    //TODO:
    //scenarioHint.captureScenarioIndex = ? /* hint from vendor tag */
    int32_t openId = mPolicyParams.openId;
    auto pAppMetadata = requestParams->pRequest_AppControl;

    int32_t scenario = -1;
    if (!get_capture_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
        MY_LOGE("cannot get capture scenario openId:%d", openId);
        return false;
    }
    else {
        MY_LOGD("find scenario:%s for (openId:%d, scenario:%d)",
                scenarioFeatures.scenarioName.c_str(), openId, scenario);
    }

    bool isFeatureTrigger = false;
    for (auto &featureSet : scenarioFeatures.vFeatureSet) {
        // evaluate key feature plugin and feature combination for feature strategy policy.
        if (strategyCaptureFeature(requestParams, featureSet.feature, featureSet.featureCombination, parsedInfo)) {
            isFeatureTrigger = true;
            MY_LOGI("trigger feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64") for req#%u (count:%d, index:%d)",
                    featureSet.featureName.c_str(),
                    static_cast<MINT64>(featureSet.feature),
                    featureSet.featureCombinationName.c_str(),
                    static_cast<MINT64>(featureSet.featureCombination),
                    requestParams->requestNo,
                    requestParams->frameCount,
                    requestParams->frameIndex);
            updatePluginSelection(isFeatureTrigger, 1); //update each frame selection
            break;
        }
        else{
            isFeatureTrigger = false;
            MY_LOGD("no need to trigger feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64")",
                    featureSet.featureName.c_str(),
                    static_cast<MINT64>(featureSet.feature),
                    featureSet.featureCombinationName.c_str(),
                    static_cast<MINT64>(featureSet.featureCombination));
            updatePluginSelection(isFeatureTrigger);
        }
    }

    if (!isFeatureTrigger) {
        MY_LOGE("no feature can be triggered!");
        return false;
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
getCaptureProvidersByScenarioFeatures(
    ConfigurationOutputParams* out __unused,
    ConfigurationInputParams const* in __unused
) -> bool
{
    CAM_ULOGM_APILIFE();

    auto supportedFeatures = out->ispCaptureParams.supportedScenarioFeatures;
    auto pluginUniqueKey = in->pipelineUniqueKey;
    MY_LOGI("support features:%#" PRIx64 " uniqueKey:%d", supportedFeatures, pluginUniqueKey);

    mMFPPluginWrapperPtr = std::make_shared<MFPPluginWrapper>(supportedFeatures, pluginUniqueKey);
    mRawPluginWrapperPtr = std::make_shared<RawPluginWrapper>(supportedFeatures, pluginUniqueKey);

    // DualCam features move to app layer.

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
FeatureSettingPolicy_ISP::
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
    scenarioHint.isIspHidl = true;
    //
    int32_t openId = mPolicyParams.openId;
    auto pAppMetadata = mPolicyParams.sessionParams;

    int32_t scenario = -1;
    if (!get_capture_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
        MY_LOGE("cannot get capture scenario openId:%d", openId);
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
        out->ispCaptureParams.supportedScenarioFeatures |= (featureSet.feature | featureSet.featureCombination);
    }
    MY_LOGD("support features:%#" PRIx64"", out->ispCaptureParams.supportedScenarioFeatures);

    mPipelineUniqueKey = in->pipelineUniqueKey;
    out->ispCaptureParams.pluginUniqueKey = mPipelineUniqueKey;
    MY_LOGD("(%p) pipelineUniqueKey:%d", this, mPipelineUniqueKey);

    if (!getCaptureProvidersByScenarioFeatures(out, in)) {
        MY_LOGE("createCapturePluginByScenarioFeatures failed!");
        return false;
    }

    // query additional capture buffer usage count from multiframe features
    for (auto iter : mMFPPluginWrapperPtr->getProviders()) {
        const MultiFramePlugin::Property& property =  iter->property();
        auto supportedFeatures = out->ispCaptureParams.supportedScenarioFeatures;
        if ((property.mFeatures & supportedFeatures) != 0) {
            MY_LOGD("find provider(%s) algo(%#" PRIx64")", property.mName, property.mFeatures);
        }
        else {
            MY_LOGD("no need this provider(%s) algo(%#" PRIx64") at this sceanrio(%s)",
                    property.mName, property.mFeatures, scenarioFeatures.scenarioName.c_str());
        }
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
FeatureSettingPolicy_ISP::
evaluateConfiguration(
    ConfigurationOutputParams* out,
    ConfigurationInputParams const* in
) -> int
{
    CAM_TRACE_NAME("FeatureSettingPolicy_ISP::evaluateConfiguration");
    CAM_ULOGM_APILIFE();
    //
    if (CC_UNLIKELY(!evaluateCaptureConfiguration(out, in))) {
        CAM_ULOGME("evaluate capture configuration failed!");
        return -ENODEV;
    }
    // Default connfig params for feature strategy.
    mConfigInputParams = *in;
    mConfigOutputParams = *out;
    //
    return OK;
}

auto
FeatureSettingPolicy_ISP::
evaluateRequest(
    RequestParams* requestParams
) -> int
{
    CAM_TRACE_NAME("FeatureSettingPolicy_ISP::evaluateRequest");
    //CAM_ULOGM_APILIFE();
    // check setting valid.
    if CC_UNLIKELY(requestParams == nullptr) {
        CAM_ULOGME("invalid requestParams(%p)", requestParams);
        return -ENODEV;
    }
    ParsedStrategyInfo parsedInfo;
    if (!collectParsedStrategyInfo(requestParams, parsedInfo)) {
        MY_LOGE("collectParsedStrategyInfo failed!");
        return -ENODEV;
    }
    // P2 capture feature policy
    if (!evaluateCaptureSetting(requestParams, parsedInfo)) {
        MY_LOGE("evaluateCaptureSetting failed!");
        return -ENODEV;
    }
    // dump output request params for debug.
    dumpRequestOutputParams(requestParams, mbDebug);

    return OK;
}

// query recommended perframe setting for ISP HIDL user to invoke MTK image process features.
auto
FeatureSettingPolicy_ISP::
queryMultiFramePluginRecommendedSetting(
    RecommendedParams* recommendedParams,
    MINT64 combinedKeyFeature, /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    MINT64& foundFeature /*eFeatureIndexMtk and eFeatureIndexCustomer*/
) -> bool
{
    auto provider = mMFPPluginWrapperPtr->getProvider(combinedKeyFeature, foundFeature);
    if (provider) {
        // for MultiFramePlugin key feature (ex: HDR, MFNR, 3rd party multi-frame algo,etc )
        // query recommended setting from feature requirement
        if (recommendedParams == nullptr) {
            MY_LOGW("invalid nullptr! (recommendedParams:%p)", recommendedParams);
            return false;
        }
        else if (recommendedParams->pRequest_AppMetadata == nullptr) {
            MY_LOGW("invalid nullptr! (pRequest_AppMetadata:%p)", recommendedParams->pRequest_AppMetadata);
            return false;
        }

        auto property =  provider->property();
        //
        MFP_RecommendedParam param;
        param.pAppMetadata = recommendedParams->pRequest_AppMetadata;
        if (provider->queryRecommendedFrameSetting(param) == OK) {
            if (param.framesSetting.size() <= 0) {
                MY_LOGW("%s(%s), plugin return invalid(size:%zu) recommended setting",
                        mMFPPluginWrapperPtr->getName().c_str(), property.mName, param.framesSetting.size());
                return false;
            }
            else {
                for(size_t i=0; i < param.framesSetting.size(); i++) {
                    auto appMeta = param.framesSetting[i];
                    if (CC_UNLIKELY(appMeta.get() == nullptr)) {
                        MY_LOGW("%s(%s), plugin return invalid recommended setting, appMeta[%zu](%p) is nullptr",
                                mMFPPluginWrapperPtr->getName().c_str(), property.mName, i, appMeta.get());
                        // clear the invalid setting!
                        recommendedParams->framesSetting.clear();
                        return false;
                    }
                    recommendedParams->framesSetting.push_back(appMeta);
                }
                MY_LOGD("%s(%s), plugin return recommended setting(frame count:%zu)",
                        mMFPPluginWrapperPtr->getName().c_str(), property.mName, param.framesSetting.size());
            }
        }
        else {
            MY_LOGD("%s(%s), no recommended setting for foundFeature(%#" PRIx64")",
                    mMFPPluginWrapperPtr->getName().c_str(), property.mName, foundFeature);
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
FeatureSettingPolicy_ISP::
queryCaptureFeatureRecommendedSetting(
    RecommendedParams* recommendedParams,
    MINT64 combinedKeyFeature /*eFeatureIndexMtk and eFeatureIndexCustomer*/
) -> bool
{
    CAM_ULOGM_APILIFE();
    MY_LOGD("query recommended setting from key feature(%#" PRIx64")", combinedKeyFeature);

    if (CC_UNLIKELY(mForcedKeyFeatures >= 0)) {
        combinedKeyFeature = mForcedKeyFeatures;
        MY_LOGW("forced key feature(%#" PRIx64") for debug", combinedKeyFeature);
    }
    //
    MINT64 foundFeature = 0;
    if (combinedKeyFeature) { /* not MTK_FEATURE_NORMAL */
        MINT64 checkFeatures = combinedKeyFeature;
        //
        if (!queryMultiFramePluginRecommendedSetting(recommendedParams, checkFeatures, foundFeature)) {
            MY_LOGD("no recommended setting from feature(%#" PRIx64") for features(key:%#" PRIx64")",
                    foundFeature, combinedKeyFeature);
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
        MY_LOGD("no key feature, it's recommended to execute default normal single capture");
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
queryCaptureRecommendedSetting(
    RecommendedParams* recommendedParams
) -> bool
{
    CAM_ULOGM_APILIFE();

    if (CC_UNLIKELY(recommendedParams == nullptr)) {
        MY_LOGW("recommendedParams(%p) is invalid nullptr", recommendedParams);
        return false;
    }
    else if (CC_UNLIKELY(recommendedParams->pRequest_AppMetadata == nullptr)) {
        MY_LOGW("pRequest_AppMetadata(%p) is invalid nullptr", recommendedParams->pRequest_AppMetadata);
    }

    ScenarioFeatures scenarioFeatures;
    CaptureScenarioConfig scenarioConfig;
    ScenarioHint scenarioHint;
    scenarioHint.isIspHidl = true;
    scenarioHint.ispProcessInOutType = eIspProcessInOutType_QuerySupportedKeyFeature;
    auto pAppMetadata = recommendedParams->pRequest_AppMetadata;

    int32_t scenario = -1;
    if (!get_capture_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
        MY_LOGW("cannot get capture scenario");
        return false;
    }
    else {
        MY_LOGD("find scenario:%s(%d)", scenarioFeatures.scenarioName.c_str(), scenario);
    }

    if(mbDebug) {
        MY_LOGD("Dump appMeta+");
        const_cast<IMetadata*>(pAppMetadata)->dump();
        MY_LOGD("Dump appMeta-");
    }

    for (auto &featureSet : scenarioFeatures.vFeatureSet) {
        if (queryCaptureFeatureRecommendedSetting(recommendedParams, featureSet.feature)) {
            MY_LOGI("get recommended setting from feature:%s(%#" PRIx64")", featureSet.featureName.c_str(), static_cast<MINT64>(featureSet.feature));
            break;
        }
    }

    // check result for debug
    if (recommendedParams->framesSetting.size() <= 0) {
        MY_LOGI("no recommended setting(frames setting size:%zu) from capture scenario:%s(%d)",
                recommendedParams->framesSetting.size(), scenarioFeatures.scenarioName.c_str(), scenario);
    }

    return true;
}

auto
FeatureSettingPolicy_ISP::
queryRecommendedSetting(
    RecommendedParams* recommendedParams
) -> int
{
    CAM_TRACE_NAME("FeatureSettingPolicy_ISP::queryRecommendedSetting");
    if (!queryCaptureRecommendedSetting(recommendedParams)) {
        MY_LOGW("queryCaptureRecommendedSetting failed.");
        return -ENODEV;
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace featuresetting_isp {
auto createFeatureSettingPolicyInstance_ISP(
    CreationParams const& params
) -> std::shared_ptr<IFeatureSettingPolicy_ISP>
{
    CAM_ULOGM_APILIFE();

    // you have got an instance for feature setting policy.
    return std::make_shared<FeatureSettingPolicy_ISP>(params);
}
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
