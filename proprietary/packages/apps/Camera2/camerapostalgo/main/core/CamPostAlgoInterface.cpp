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
#include <CamPostAlgoInterface.h>
#include <CamPostAlgoProviderMgr.h>

#include <gui/ConsumerBase.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <gui/view/Surface.h>
#include <gui/BufferQueue.h>
#include <gui/BufferItem.h>
#include <gui/BufferItemConsumer.h>
#include <ui/GraphicBuffer.h>
#include <hardware/gralloc.h>
#include <utils/String8.h>
#include <utils/Errors.h>
#include <cutils/properties.h>
#include <chrono>

#include <system/window.h>
#include <thread>
#include <vector>

#include <campostalgo/Stream.h>
#include<campostalgo/FeatureResult.h>
#include <campostalgo/pipe/PipeInfo.h>
#include <processor/ParamConvert.h>
#include <processor/PA_Request.h>
#include <log/Log.h>

using namespace com::mediatek::campostalgo;
using ::android::binder::Status;
using ::android::sp;
using ::std::vector;
using android::NO_ERROR;
using NSPA::PABufferItem;
using NSPA::FeatureConfigParam;
using NSPA::MetadataPack;

#define LOG_TAG "CamPostAlgoInterface"

ClientDeathRecipient::ClientDeathRecipient(wp<CamPostAlgoInterface> interface) :
        mInterface(interface) {

}

void ClientDeathRecipient::binderDied(const wp<IBinder>& who) {
    ALOGW("Client 0x%p died and do disconnected!", who.unsafe_get());
    sp<CamPostAlgoInterface> intf(mInterface.promote());
    if (intf != nullptr) {
        intf->disconnect();
    }
}

CamPostAlgoInterface::CamPostAlgoInterface(int32_t id) :
        mId(id), mStat(STAT_UNKNOWN) {
    mName = android::String8::format("AINTF-%d", id);
    ALOGI("CamPostAlgoInterface %id created!", mId);
}

CamPostAlgoInterface::~CamPostAlgoInterface() {
    Mutex::Autolock _l(mInterfaceLock);
    ALOGD("CamPostAlgoInterface destroyed!");
    mStat = STAT_DESTROYED;
}

Status CamPostAlgoInterface::getStat(int32_t* _aidl_return) {
    Mutex::Autolock _l(mInterfaceLock);
    *_aidl_return = mStat;
    return Status::ok();
}

Status CamPostAlgoInterface::disconnect() {
    ALOGD("CamPostAlgoInterface disconnect");
    Mutex::Autolock _l(mInterfaceLock);
    if (mStat != STAT_DISCONNECTED && mStreamManager != nullptr) {
        mCaptureProcessor.uninit();
        mPreviewProcessor.uninit();
        mStreamManager->destroy();
        mStreamManager = nullptr;
        mStat = STAT_DISCONNECTED;
        if (mClientCallback != nullptr) {
            IInterface::asBinder(mClientCallback)->unlinkToDeath(
                    wp < ClientDeathRecipient > (mClientDeathRecipient.get()));
            //shall we notify to client that disconnect done?
            mClientDeathRecipient = nullptr;
            mClientCallback = nullptr;
        }
    }
    ALOGD("disconnect done!");
    return Status::ok();
}

Status CamPostAlgoInterface::flush(int64_t* _aidl_return) {
    ALOGI("CamPostAlgoInterface flush");
    Mutex::Autolock _l(mInterfaceLock);
    mCaptureProcessor.flush();
    mPreviewProcessor.flush();
    return Status::ok();
}

MBOOL CamPostAlgoInterface::parseConfigParam(
        const NSPA::ConfigParams &configParam) {
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
//    sp<MWInfo> info = new MWInfo(configParam);
//    if (!info->isValid(mP2Info->mConfigInfo.mLog)) {
//        MY_S_LOGW(mLog, "invalid config param");
//    } else {
//        mMWInfo = info;
//        mP2Info = mP2Info->clone();
//        updateConfigInfo(mP2Info, mMWInfo);
//        ret = MTRUE;
//    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

Status CamPostAlgoInterface::configParams(int32_t type,
        const FeatureParam& param) {
    ALOGD("configParams");
    Mutex::Autolock _l(mInterfaceLock);
    NSPA::FeatureConfigParam params;
    params.mMetaPackPtr = new MetadataPack;
    // params.configFeatureParams(param);
    ParamConvert::getInstance()->convertFromParam(param,
            *params.mMetaPackPtr->getMetaPtr());

    switch ((StreamType) type) {
    case StreamType::CAPTURE:
        mCaptureProcessor.config(params);
        break;
    case StreamType::PREVIEW:
        mPreviewProcessor.config(params);
        break;
    default:
        ALOGE("configParams not support type %d!", type);
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    return Status::ok();
}

Status CamPostAlgoInterface::config(const vector<FeatureConfig>& configs,
        const ::android::sp<ICamPostAlgoCallback>& callback,
        FeatureResult* result) {
    Mutex::Autolock _l(mInterfaceLock);
    ALOGD("CamPostAlgoInterface config size %zu", configs.size());
    MBOOL ret = MFALSE;
    Status stat = Status::ok();
    if (configs.size() == 0) {
        ALOGE("config with nothting");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    if (mStat == STAT_CONFIGURED) {
        ALOGE("config again!");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }

    std::vector<AlgoType> captureAlgos;
    std::vector<AlgoType> previewAlgos;
    StreamType type;
    sp<FeaturePipeConfig> pipeConfig;

    if (mStreamManager.get() != nullptr) {
        mStreamManager->destroy();
        mStreamManager = nullptr;
    }
    mStreamManager = NSPA::IStreamManager::createInstance(
            getInterfaceName().string());
    //check callback
    if (callback != nullptr) {
        sp<IBinder> binder = IInterface::asBinder(callback);
        wp<CamPostAlgoInterface> intf(this);
        mClientDeathRecipient = new ClientDeathRecipient(intf);
        binder->linkToDeath(mClientDeathRecipient);
        mStreamManager->addInterfaceCallback(callback);
        mClientCallback = callback;
    } else {
        ALOGD("config w/o callback!");
    }

    for (const FeatureConfig& config : configs) {
        pipeConfig = config.getFeaturePipeConfig();
        type = pipeConfig->getStreamType();
        const std::vector<AlgoType>& algos = pipeConfig->getStreamAlgos();
        switch (type) {
        case StreamType::CAPTURE:
            captureAlgos.insert(captureAlgos.end(), algos.begin(), algos.end());
            break;
        case StreamType::PREVIEW:
            previewAlgos.insert(captureAlgos.end(), algos.begin(), algos.end());
            break;
        default:
            ALOGE("Not support type %d", type);
            break;
        }

        stat = mStreamManager->configure(config, result);
        if (!stat.isOk()) {
            ALOGE("Error to confiureHalStreams");
            return stat;
        }
    }

    //TODO:check if capture and preview and do the init
    /*
     * async runing policy
     * 0. parse algo info
     * 1. capture
     * 2. preview
     IMetadata::setEntry<MINT64>(pOutMetaHal.get(), MTK_FEATURE_CAPTURE, featureCombination);

     IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HAL_REQUEST_INDEX, requestIndex);
     IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HAL_REQUEST_COUNT, requestCount);
     evaluateCaptureSetting
     evaluateStreamSetting
     */

    mCaptureProcessor.init(NSPA::FeatureInitParam {
            .mType = StreamType::CAPTURE, .mInterfaceId = mId, .mAlgos =
                    captureAlgos, .mStreamManager = mStreamManager });

    NSPA::FeatureConfigParam cParams;
    ret = mCaptureProcessor.config(cParams);
    if (ret != MTRUE) {
        ALOGE("processor configure failed!");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }

    mPreviewProcessor.init(NSPA::FeatureInitParam {
            .mType = StreamType::PREVIEW, .mInterfaceId = mId, .mAlgos =
                    previewAlgos, .mStreamManager = mStreamManager });
    NSPA::FeatureConfigParam sParams;
    ret = mPreviewProcessor.config(sParams);
    if (ret != MTRUE) {
        ALOGE("processor configure failed!");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }

    mStat = STAT_CONFIGURED;
    ALOGD("configure done!");
    return Status::ok();
}

Status CamPostAlgoInterface::finalizeOutputConfig(
        const std::vector<Stream>& streams) {
    ALOGD("finalizeOutputConfig %zu!", streams.size());
    Mutex::Autolock _l(mInterfaceLock);

    if (mStat < STAT_INITED) {
        ALOGE("Error stat is %d", (int )mStat);
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }

    Vector<sp<android::Surface>> captures;
    Vector<sp<android::Surface>> previews;
    status_t res = android::OK;
    for (const auto& stream : streams) {
        const StreamType type = stream.getStreamType();
        switch (type) {
        case StreamType::CAPTURE:
            captures.push_back(stream.getSurface());
            break;
        case StreamType::PREVIEW:
            previews.push_back(stream.getSurface());
            break;
        default:
            ALOGW("output configure not support type %d!", type);
        }
    }
    res = mStreamManager->configureStreams(StreamType::CAPTURE, captures);
    if (res != android::NO_ERROR) {
        ALOGE("%s: error to configure streams type %d, %s(%d)", __FUNCTION__,
                StreamType::CAPTURE, strerror(-res), res);
        return Status::fromExceptionCode(res);
    }
    res = mStreamManager->configureStreams(StreamType::PREVIEW, previews);
    if (res != android::NO_ERROR) {
        ALOGE("%s: error to configure streams type %d, %s(%d)", __FUNCTION__,
                StreamType::CAPTURE, strerror(-res), res);
        return Status::fromExceptionCode(res);
    }
    return Status::ok();
}

NSPA::FeatureProcessor* CamPostAlgoInterface::getFeatureProcessor(
        StreamType type) {
    //TODO: add lock here
    using namespace com::mediatek::campostalgo;
    switch (type) {
    case CAPTURE:
        return &mCaptureProcessor;
    case PREVIEW:
        return &mPreviewProcessor;
    case TINY:
    case UNKNOWN:
    default:
        ALOGE("unsupport processor type %d!", type);
        return (NSPA::FeatureProcessor*) NULL;
    }
}

int32_t CamPostAlgoInterface::getInterfaceId() {
    return mId;
}

const android::String8 CamPostAlgoInterface::getInterfaceName() {
    return mName;
}

status_t CamPostAlgoInterface::initialize() {
    ALOGI("CamPostAlgoInterface %d initialize", mId);

    /*    IFeaturePipe::UsageHint Usage = IFeaturePipe::UsageHint();
     Usage.mDualMode = 0;
     Usage.mSupportedScenarioFeatures = ~0;
     Usage.mPluginUniqueKey = 0;
     mpFeatureRequestCallback = new FeatureRequestCallback();
     mpFeaturePipe = init_feature_pipe(Usage, mpFeatureRequestCallback);*/

    return OK;
}
