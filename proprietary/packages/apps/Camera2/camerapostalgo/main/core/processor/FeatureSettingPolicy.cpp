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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <processor/FeatureSettingPolicy.h>
#include <utils/imgbuf/IImageBuffer.h>
#include <utils/ImageBufferHelper.h>
#include <plugin/PipelinePluginType.h>
#include <unordered_map>

using ::com::mediatek::campostalgo::StreamType;
using namespace ::NSCam::NSPipelinePlugin;

#define LOG_TAG "POSTALGO_FSPolicy"

namespace NSPA {

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
class PluginWrapper {
public:
    using PluginPtr = typename TPlugin::Ptr;
    using ProviderPtr = typename TPlugin::IProvider::Ptr;
    using InterfacePtr = typename TPlugin::IInterface::Ptr;
    using SelectionPtr = typename TPlugin::Selection::Ptr;
    using Selection = typename TPlugin::Selection;
    using Property = typename TPlugin::Property;
    using ProviderPtrs = std::vector<ProviderPtr>;
public:
    PluginWrapper(const std::string& name, MINT32 iOpenId, MINT32 iOpenId2,
            MUINT64 uSupportedFeatures = ~0, MINT32 iUniqueKey = -1);
    ~PluginWrapper();
public:
    auto getName() const -> const std::string&;
    auto isKeyFeatureExisting(MINT64 combinedKeyFeature,
            MINT64& keyFeature) const -> MBOOL;
    auto tryGetKeyFeature(MINT64 combinedKeyFeature, MINT64& keyFeature,
            MINT8& keyFeatureIndex) const -> MBOOL;
    auto getProvider(MINT64 combinedKeyFeature,
            MINT64& keyFeature) -> ProviderPtr;
    auto getProviders() -> ProviderPtrs;
    auto createSelection() const -> SelectionPtr;
    auto offer(Selection& sel) const -> MVOID;
    auto keepSelection(const uint32_t requestNo, ProviderPtr& providerPtr,
            SelectionPtr& sel) -> MVOID;
    auto pushSelection(const uint32_t requestNo,
            const uint8_t frameCount) -> MVOID;
    auto cancel() -> MVOID;
    auto frontSelection(ProviderPtr pProvider, const uint32_t requestNo,
            const uint8_t frameIndex) -> SelectionPtr;
private:
    using ProviderPtrMap = std::unordered_map<MUINT64, ProviderPtr>;
    using SelectionPtrMap = std::unordered_map<ProviderPtr, std::vector<SelectionPtr>>;
private:
    const std::string mName;
    const MINT32 mOpenId1;
    const MINT32 mOpenId2;
    const MINT64 mSupportedFeatures;
    const MINT32 mUniqueKey;
    PluginPtr mInstancePtr;
    ProviderPtrs mProviderPtrsSortedByPriorty;
    SelectionPtrMap mTempSelectionPtrMap;
    InterfacePtr mInterfacePtr;
};

/******************************************************************************
 *
 ******************************************************************************/
template<typename TPlugin>
PluginWrapper<TPlugin>::PluginWrapper(const std::string& name, MINT32 iOpenId,
        MINT32 iOpenId2, MUINT64 uSupportedFeatures, MINT32 uniqueKey) :
        mName(name), mOpenId1(iOpenId), mOpenId2(iOpenId2), mSupportedFeatures(
                uSupportedFeatures), mUniqueKey(uniqueKey) {
    MY_LOGD(
            "ctor:%p, name:%s, openId:%d, openId2:%d, supportedFeatures:%#" PRIx64 ", uniqueKey:%d",
            this, mName.c_str(), mOpenId1, mOpenId2, mSupportedFeatures,
            mUniqueKey);
    mInstancePtr = TPlugin::getInstance(mUniqueKey, { mOpenId1, mOpenId2 });
    if (mInstancePtr) {
        mInterfacePtr = mInstancePtr->getInterface();
        auto& providers = mInstancePtr->getProviders(/*mSupportedFeatures*/);
        mProviderPtrsSortedByPriorty = providers;
        std::sort(mProviderPtrsSortedByPriorty.begin(),
                mProviderPtrsSortedByPriorty.end(),
                [] (const ProviderPtr& p1, const ProviderPtr& p2) {
                    return p1->property().mPriority > p2->property().mPriority;
                });

        for (auto& provider : mProviderPtrsSortedByPriorty) {
            const Property& property = provider->property();
            MY_LOGD(
                    "find provider... name:%s, algo(%#" PRIx64"), priority(0x%x)",
                    property.mName, property.mFeatures, property.mPriority);
        }
    } else {
        MY_LOGW("cannot get instance for %s features strategy", mName.c_str());
    }
}

template<typename TPlugin>
PluginWrapper<TPlugin>::~PluginWrapper() {
    MY_LOGD("dtor:%p name:%s, openId:%d, openId2:%d, uniqueKey:%d", this,
            mName.c_str(), mOpenId1, mOpenId2, mUniqueKey);
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::getName() const -> const std::string& {
    return mName;
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::isKeyFeatureExisting(MINT64 combinedKeyFeature,
        MINT64& keyFeature) const -> MBOOL {
    MINT8 keyFeatureIndex = 0;
    return tryGetKeyFeature(combinedKeyFeature, keyFeature, keyFeatureIndex);
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::tryGetKeyFeature(MINT64 combinedKeyFeature,
        MINT64& keyFeature, MINT8& keyFeatureIndex) const -> MBOOL {
    for (MUINT8 i = 0; i < mProviderPtrsSortedByPriorty.size(); i++) {
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
auto PluginWrapper<TPlugin>::getProvider(MINT64 combinedKeyFeature,
        MINT64& keyFeature) -> ProviderPtr {
    MINT8 keyFeatureIndex = 0;
    return tryGetKeyFeature(combinedKeyFeature, keyFeature, keyFeatureIndex) ?
            mProviderPtrsSortedByPriorty[keyFeatureIndex] : nullptr;
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::getProviders() -> ProviderPtrs {
    ProviderPtrs ret;
    ret = mProviderPtrsSortedByPriorty;
    return std::move(ret);
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::createSelection() const -> SelectionPtr {
    return mInstancePtr->createSelection();
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::offer(Selection& sel) const -> MVOID {
    mInterfacePtr->offer(sel);
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::keepSelection(const uint32_t requestNo,
        ProviderPtr& providerPtr, SelectionPtr& sel) -> MVOID {
    if (mTempSelectionPtrMap.find(providerPtr) != mTempSelectionPtrMap.end()) {
        mTempSelectionPtrMap[providerPtr].push_back(sel);
        MY_LOGD("%s: selection size:%zu, requestNo:%u", getName().c_str(),
                mTempSelectionPtrMap[providerPtr].size(), requestNo);
    } else {
        std::vector<SelectionPtr> vSelection;
        vSelection.push_back(sel);
        mTempSelectionPtrMap[providerPtr] = vSelection;
        MY_LOGD("%s: new selection size:%zu, requestNo:%u", getName().c_str(),
                mTempSelectionPtrMap[providerPtr].size(), requestNo);
    }
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::pushSelection(const uint32_t requestNo,
        const uint8_t frameCount) -> MVOID {
    for (auto item : mTempSelectionPtrMap) {
        auto providerPtr = item.first;
        auto vSelection = item.second;
        MY_LOGD("%s: selection size:%zu, frameCount:%d", getName().c_str(),
                vSelection.size(), frameCount);
        if (frameCount > 1 && vSelection.size() == 1) {
            auto sel = vSelection.front();
            for (size_t i = 0; i < frameCount; i++) {
                MY_LOGD(
                        "%s: duplicate selection for multiframe(count:%d, index:%zu)",
                        getName().c_str(), frameCount, i);

                auto pSelection = std::make_shared < Selection > (*sel);
                pSelection->mTokenPtr = Selection::createToken(mUniqueKey,
                        requestNo + i, i);
                mInstancePtr->pushSelection(providerPtr, pSelection);
            }
        } else {
            for (auto sel : vSelection) {
                mInstancePtr->pushSelection(providerPtr, sel);
            }
        }
        vSelection.clear();
    }
    mTempSelectionPtrMap.clear();
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::cancel() -> MVOID {
    for (auto item : mTempSelectionPtrMap) {
        auto providerPtr = item.first;
        auto vSelection = item.second;
        if (providerPtr.get()) {
            //providerPtr->cancel();
        }
        MY_LOGD("%s: selection size:%zu", getName().c_str(), vSelection.size());
        vSelection.clear();
    }
    mTempSelectionPtrMap.clear();
}

template<typename TPlugin>
auto PluginWrapper<TPlugin>::frontSelection(ProviderPtr pProvider, const uint32_t requestNo,
            const uint8_t frameIndex) -> SelectionPtr {
    auto token = Selection::createToken(mUniqueKey,
            requestNo, frameIndex);
    auto pFrontSel = mInstancePtr->frontSelection(pProvider, token);
    return pFrontSel;
}

#define DEFINE_PLUGINWRAPER(CLASSNAME, PLUGINNAME)                                                                      \
class FeatureSettingPolicy::CLASSNAME final: public PluginWrapper<PLUGINNAME>                  \
{                                                                                                                       \
public:                                                                                                                 \
    /*Dual Cam Feature Provider*/                                                                                       \
    CLASSNAME(MINT32 iOpenId, MINT32 iOpenId2, MUINT64 uSupportedFeatures, MINT32 iUniqueKey)                           \
    : PluginWrapper<PLUGINNAME>(#PLUGINNAME, iOpenId, iOpenId2, uSupportedFeatures, iUniqueKey)\
    {                                                                                                                   \
    }                                                                                                                   \
                                                                                                                        \
    /*Single Cam Feature Provider*/                                                                                     \
    CLASSNAME(MINT32 iOpenId, MUINT64 uSupportedFeatures, MINT32 iUniqueKey)                                            \
    : PluginWrapper<PLUGINNAME>(#PLUGINNAME, iOpenId, -1, uSupportedFeatures, iUniqueKey)      \
    {                                                                                                                   \
    }                                                                                                                   \
}
DEFINE_PLUGINWRAPER(MFPPluginWrapper, MultiFramePlugin);
DEFINE_PLUGINWRAPER(YuvPluginWrapper, YuvPlugin);

#undef DEFINE_PLUGINWRAPER


bool FeatureSettingPolicy::updatePluginSelection(
    const uint32_t requestNo,
    bool isFeatureTrigger,
    uint8_t frameCount
)
{
    if (isFeatureTrigger) {
        mMFPPluginWrapperPtr->pushSelection(requestNo, frameCount);
    }
    else {
        mMFPPluginWrapperPtr->cancel();
    }

    return true;
}

FeatureSettingPolicy::FeatureSettingPolicy(const CreationParams& cp) {
    mStreamType = cp.type;
    mIntfId = cp.mIntfId;
    mStreamAlgos = cp.algos;
    mUniqueKey = mIntfId << 8 | mStreamType;
}

eFeatureIndexMtk FeatureSettingPolicy::convertAlgoType2FeatureType(
        AlgoType type) {
    eFeatureIndexMtk ret = NO_FEATURE_NORMAL;
    switch (type) {
    case AlgoType::FILTER_PREVIEW:
        ret = MTK_FEATURE_FILTER_PREVIEW;
        break;
    case AlgoType::FILTER_CAPTURE:
        ret = MTK_FEATURE_FILTER_CAPTURE;
        break;
    case AlgoType::FILTER_MATRIX:
        ret = MTK_FEATURE_FILTER_MATRIX;
        break;
    case AlgoType::FB:
        ret = MTK_FEATURE_FB;
        break;
    case AlgoType::AUTOPANORAMA:
        ret = MTK_FEATURE_AUTORAMA;
        break;
    case AlgoType::PORTRAIT:
        ret = MTK_FEATURE_PORTRAIT;
        break;
    case AlgoType::HDR:
        ret = MTK_FEATURE_HDR;
        break;
    case AlgoType::VFB_PREVIEW:
        ret = MTK_FEATURE_VFB_PREVIEW;
        break;
    case AlgoType::VFB_CAPTURE:
        ret = MTK_FEATURE_VFB_CAPTURE;
        break;
    default:
        ALOGE("Not support algo type %d", type);
        return NO_FEATURE_NORMAL;
    }
    ALOGD("convertAlgoType2FeatureType from AlgoType %d to Feature Type %d", (int )type,
            (int )ret);
    return ret;
}

status_t FeatureSettingPolicy::evaluateRequest(const sp<PA_Request>& request) {
    //combined feature set;
    uint64_t set = 0;
    ///TODO: change to MTK_FEATURE_XXXX
    for (auto& algo : mStreamAlgos) {
        set |= convertAlgoType2FeatureType(algo);
    }
    request->setCombinedFeatureSet(set);

    if (request->getStreamType() == StreamType::PREVIEW) {
        evaluateStreamSetting(request);
    } else if (request->getStreamType() == StreamType::CAPTURE) {
        //check if multi frame aglgo
        evaluateCaptureSetting(request);
    }
    return OK;
}

status_t FeatureSettingPolicy::evaluateCaptureSetting(
        const sp<PA_Request>& request) {
    ALOGD("evaluateCaptureSettings [Capture] %d: featureset %" PRId64 ", unique %d",
            request->getRequestNo(), request->getCombinedFeatureSet(),
            mUniqueKey);

    mMFPPluginWrapperPtr = std::make_shared <MFPPluginWrapper> (
            -1, -1, request->getCombinedFeatureSet(), mUniqueKey);
    int64_t value = request->getCombinedFeatureSet();
    auto provider = mMFPPluginWrapperPtr->getProvider(value, value);
    if (provider == nullptr) {
        //normal case
        ALOGD("get no provider!");
        return android::OK;
    }

//    auto property =  provider->property();
    /*
     * 1. check if multi-frame request
     * 2. map frame index and request from request count
     * 3. remember the request info
     */
    auto handle_negotiate =
            [&](int i, Policy::State stat, MBOOL doProcess, int request_count)->void {
                ALOGD("handle multiframe index  %d", i);
                auto pSubSelection = mMFPPluginWrapperPtr->createSelection();
                pSubSelection->mIMetadataApp.setControl(
                        std::make_shared<IMetadata>(
                                *(request->getMetadataPack(IN_PAS)->getMetaPtr())));
                if (CC_LIKELY(doProcess)) {
                    pSubSelection->mTokenPtr = MFP_Selection::createToken(
                            mUniqueKey, request->getRequestNo() + i, i);
                    mMFPPluginWrapperPtr->keepSelection(request->getRequestNo() + i,
                            provider, pSubSelection);
                } else {
                    MY_LOGD(
                            "%s(%s) bypass process, only decide frames requirement",
                            mMFPPluginWrapperPtr->getName().c_str(),
                            provider->property().mName);
                }
                MFP_Selection& subsel = *pSubSelection;
                subsel.mState = stat;
                subsel.mRequestIndex = i;
                subsel.mRequestCount = request_count;
                mMFPPluginWrapperPtr->offer(subsel);
                provider->negotiate(subsel);
            };

    if (provider) {
        if (!fsMultiFrameCntx.doProcess) {
            ALOGD("begin to multiframe handle");
            auto pSelection = mMFPPluginWrapperPtr->createSelection();
            MFP_Selection& sel = *pSelection;
            sel.mRequestIndex = -1;
            mMFPPluginWrapperPtr->offer(sel);
            if (provider->negotiate(sel) == OK && sel.mRequestCount > 0) {
                /*
                 * capture multi frame mode
                 * Request Count
                 * frame index
                 */
                fsMultiFrameCntx.doProcess = true;
                fsMultiFrameCntx.request_count = (uint32_t) sel.mRequestCount;
                fsMultiFrameCntx.frame_index = 0;
                fsMultiFrameCntx.state = sel.mState;

                request->setFrameCount(fsMultiFrameCntx.request_count);
                request->setFrameIndex(fsMultiFrameCntx.frame_index++);
                ///TODO: calc frame index or request inex
/*                handle_negotiate(fsMultiFrameCntx.frame_index++,
                        fsMultiFrameCntx.state, fsMultiFrameCntx.doProcess,
                        fsMultiFrameCntx.request_count);*/
                for (uint32_t i = 0; i < sel.mRequestCount; i++) {
                    handle_negotiate(i, fsMultiFrameCntx.state,
                            fsMultiFrameCntx.doProcess,
                            fsMultiFrameCntx.request_count);
                 }
                updatePluginSelection(request->getRequestNo(), true,
                        fsMultiFrameCntx.request_count);
            }
        } else if (fsMultiFrameCntx.doProcess) {
            request->setFrameCount(fsMultiFrameCntx.request_count);
            request->setFrameIndex(fsMultiFrameCntx.frame_index++);

            /// sub frame
//            handle_negotiate(fsMultiFrameCntx.frame_index++,
//                    fsMultiFrameCntx.state, fsMultiFrameCntx.doProcess,
//                    fsMultiFrameCntx.request_count);
            if (fsMultiFrameCntx.frame_index
                    == fsMultiFrameCntx.request_count) {
                //reset
                ALOGD("finished multiframe request!");
                fsMultiFrameCntx = FS_CaptureMultiFrameContext_t();
            }
        }

        auto sel = mMFPPluginWrapperPtr->frontSelection(provider,request->getRequestNo(),request->getFrameIndex());
        if(sel != nullptr) {
            ALOGD("sel->mOBufferFull.getRequired %d",sel->mOBufferFull.getRequired());
             if(MTRUE == sel->mOBufferFull.getRequired()) {
                 request->setIsNeedWorkingBuffer(MTRUE);
             } else {
                 request->setIsNeedWorkingBuffer(MFALSE);
             }
        } else {
            ALOGE("frontSelection get sel is nullptr!");
        }
    }
    return OK;
}
status_t FeatureSettingPolicy::evaluateStreamSetting(
        const sp<PA_Request>& request) {
    return OK;
}

FeatureSettingPolicy::~FeatureSettingPolicy() {
}

} /* namespace NSPA */
