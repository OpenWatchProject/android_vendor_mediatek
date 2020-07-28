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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <processor/PA_FeatureProcessor.h>

#include <chrono>
#include <thread>

#include <header_base.h>
#include <processor/PA_Request.h>
#include <campostalgo/pipe/PipeInfo.h>
#include <utils/metadata/IMetadata.h>
#include <processor/PA_Param.h>
#include <utils/imgbuf/IImageBuffer.h>
#include <utils/ImageBufferHelper.h>
#include <mtk/mtk_platform_metadata_tag.h>


#define LOG_TAG "PostAlgo/Processor"

using namespace NSCam;
//using namespace NSCam::NSPipelinePlugin;
//using namespace NSCam::NSCamFeature::NSFeaturePipe::NSCapture;
//using namespace NSCam::Utils::ULog;
using namespace com::mediatek::campostalgo;
using namespace com::mediatek::campostalgo::NSFeaturePipe;
using com::mediatek::campostalgo::StreamType;

#define DUMP_BUFFER_CAPTURE     "debug.pas.hal.dumpcapture.enabled"
#define DUMP_BUFFER_PREVIEW     "debug.pas.hal.dumppreview.enabled"
static int gDumpBufferCaptureEnabled = ::property_get_int32(DUMP_BUFFER_CAPTURE, 0);
static int gDumpBufferPreviewEnabled = ::property_get_int32(DUMP_BUFFER_PREVIEW, 0);


#define PA_THREAD_NAME "PA_THREAD"

namespace NSPA
{

#define CLASS_TAG    ReleaseBundle
#define PA_TRACE        TRACE_RELEASE_BUNDLE
#include "LogHeader.h"

class ReleaseBundle : public virtual RefBase
{
public:
    ReleaseBundle(const char* pName, MINT32 requestNo, MINT32 frameNo, FeatureProcessor* pProcessor, const ILog& log)
        : mName(pName)
        , mRequestNo(requestNo)
        , mFrameNo(frameNo)
        , mpProcessor(pProcessor)
    {
//        TRACE_S_FUNC_ENTER("name:%s, R/F Num:%d/%d",mName.c_str(), mRequestNo, mFrameNo);
//        TRACE_S_FUNC_EXIT("name:%s, R/F Num:%d/%d",mName.c_str(), mRequestNo, mFrameNo);
    }

    ~ReleaseBundle()
    {
        MY_LOGI("+, name:%s, R/F Num:%d/%d",mName.c_str(), mRequestNo, mFrameNo);
        if (mpProcessor != NULL) {
            mpProcessor->notifyRelease(mRequestNo, mFrameNo);
        }
        MY_LOGI("-, name:%s, R/F Num:%d/%d",mName.c_str(), mRequestNo, mFrameNo);
    };

private:
    const std::string       mName;
    const MINT32            mRequestNo;
    const MINT32            mFrameNo;

    FeatureProcessor*       mpProcessor;
};


#define CLASS_TAG    P2BufferHandle
#define PA_TRACE        TRACE_P2_BUFFER_HANDLE
#include "LogHeader.h"

class P2BufferHandle : public virtual BufferHandle
{
public:
    P2BufferHandle(const sp<PA_Request>& pRequest, ID_IMG id, sp<ReleaseBundle> pBundle, FeatureProcessor* pProcessor)
        : mpRequest(pRequest)
//        , mpP2Img(NULL)
        , mImgId(id)
//        , mpImageBuffer(NULL)
//        , mpReleaseBundle(pBundle)
        , mpProcessor(pProcessor)
    {
        TRACE_S_FUNC_ENTER("imgId:0x%" PRIx32, mImgId);
        TRACE_S_FUNC_EXIT(mLog);
    }

    P2BufferHandle(const sp<PA_Request> pRequest,/* sp<P2Img> pP2Img,*/ FeatureProcessor* pProcessor)
        : mpRequest(pRequest)
//        , mpP2Img(pP2Img)
        , mImgId(OUT_YUV)
//        , mpImageBuffer(NULL)
        , mpProcessor(pProcessor)
    {
        TRACE_S_FUNC_ENTER("imgId:0x%" PRIx32, mImgId);
        TRACE_S_FUNC_EXIT(mLog);
    }

    virtual MERROR acquire(MINT usage = eBUFFER_USAGE_HW_TEXTURE)
    {
        (void)usage;
        TRACE_S_FUNC_ENTER("imgId:0x%" PRIx32 "usage:0x%" PRIx32, mImgId, usage);
//        ALOGD("imgId:0x%" PRIx32 "usage:0x%" PRIx32, mImgId, usage);
        if(mpRequest->isValidImg(mImgId)) {
             mpImageBuffer = mpRequest->getImg(mImgId);
        }

        if (mpImageBuffer == NULL)
        {
            MY_LOGE("working buffer: create image buffer failed");
            return UNKNOWN_ERROR;
        }
        if (!(mpImageBuffer->lockBuf(LOG_TAG, usage)))
        {
            MY_LOGE("working buffer: lock image buffer failed");
            return UNKNOWN_ERROR;
        }

        ALOGV("acquired Buffer 0x%p", mpImageBuffer.get());

        #if 0
        if (mImgId == OUT_YUV) {
//            mpP2Img->updateResult(MTRUE);
//            mpImageBuffer = mpP2Img->getIImageBufferPtr();
        }
        else if (mpRequest->isValidImg(mImgId)) {
//            mpP2Img = mpRequest->getImg(mImgId);
//            mpP2Img->updateResult(MTRUE);
//            mpImageBuffer = mpP2Img->getIImageBufferPtr();
        }
        else {
            MY_LOGW("failed to acquire, imgId:0x%" PRIx32, mImgId);
            return BAD_VALUE;
        }
        #endif
        TRACE_S_FUNC_EXIT(mLog);
        return OK;
    }

    virtual IImageBuffer* native()
    {
        return mpImageBuffer.get();
    }

    virtual void release()
    {
        TRACE_S_FUNC_ENTER("imgId:0x%" PRIx32, mImgId);

        ALOGV("release Buffer 0x%p", mpImageBuffer.get());

        if (mpImageBuffer != NULL) {
            mpImageBuffer->unlockBuf(LOG_TAG);
            mpImageBuffer = NULL;
        }

//        mpP2Img = NULL;

        if (mpRequest != NULL) {
            // For now, do not support OUT_YUV early release
            //if (mImgId != OUT_YUV && mImgId != OUT_JPEG_YUV) {
                mpProcessor->releaseImage(mpRequest, mImgId);
            //}
//            mpReleaseBundle = NULL;
            mpRequest = NULL;
        }

        TRACE_S_FUNC_EXIT(mLog);
    }

    virtual MUINT32 getTransform()
    {
        ///TODO: let keep transform
//        if (mpP2Img == NULL)
//            return 0;
//        return mpP2Img->getTransform();
        return 0;
    }

    virtual ~P2BufferHandle()
    {
        TRACE_S_FUNC_ENTER("imgId:0x%" PRIx32, mImgId);

        ALOGV("destroy buffer handle!");

        if (mpRequest != NULL) {
            MY_LOGD("buffer(0x%" PRIx32 ") not released", mImgId);
            release();
        }
        TRACE_S_FUNC_EXIT(mLog);
    }

private:
    sp<PA_Request>       mpRequest;
//    sp<P2Img>           mpP2Img;
    ID_IMG              mImgId;
    sp<IImageBuffer> mpImageBuffer;
//    sp<ReleaseBundle>   mpReleaseBundle;
    FeatureProcessor*   mpProcessor;
};


#define CLASS_TAG    P2MetadataHandle
#define PA_TRACE        TRACE_P2_METADATA_HANDLE
#include "LogHeader.h"

class P2MetadataHandle : public virtual MetadataHandle
{
public:
    P2MetadataHandle(const sp<PA_Request>& pRequest, ID_META id,
            FeatureProcessor* pProcessor) :
            mpRequest(pRequest), mpMedataPack(NULL), mMetaId(id), mpProcessor(
                    pProcessor)
    {
        TRACE_S_FUNC_ENTER("metaId:0x%" PRIx32, mMetaId);
        TRACE_S_FUNC_EXIT(mLog);
    }

    virtual MERROR acquire()
    {
        TRACE_S_FUNC_ENTER("metaId:0x%" PRIx32, mMetaId);
        if (mpRequest->isValidMeta(mMetaId)) {
            mpMedataPack = mpRequest->getMetadataPack(mMetaId);
        }
        else
        {
            MY_LOGW("falied to acquire, metaId:0x%" PRIx32, mMetaId);
            return BAD_VALUE;
        }
        TRACE_S_FUNC_EXIT(mLog);
        return OK;
    }

    virtual IMetadata* native()
    {
        return mpMedataPack->getMetaPtr();
    }

    virtual void release()
    {
        TRACE_S_FUNC_ENTER("metaId:0x%" PRIx32, mMetaId);
        mpMedataPack = NULL;
        if( mpRequest != NULL ) {
            mpProcessor->releaseMeta(mpRequest, mMetaId);
            mpRequest = NULL;
        }
        TRACE_S_FUNC_EXIT(mLog);
    }

    virtual ~P2MetadataHandle()
    {
        TRACE_S_FUNC_ENTER("metaId:0x%" PRIx32, mMetaId);
        if (mpRequest != NULL) {
            MY_LOGW("metadata(%d) not released", mMetaId);
            release();
        }
        TRACE_S_FUNC_EXIT(mLog);
    }

private:
    sp<PA_Request>     mpRequest;
    sp<MetadataPack> mpMedataPack;
    ID_META           mMetaId;
    FeatureProcessor* mpProcessor;
};

#define CLASS_TAG    CaptureProcessor
#define PA_TRACE        TRACE_CAPTURE_PROCESSOR
#include "LogHeader.h"

FeatureProcessor::FeatureProcessor(const std::string& name)
    : Processor(name)
    , mAbortingRequestNo(-1)
    , mLastRequestNo(-1)
    , mLastFrameCount(0)
    , mLastFrameIndex(0)
    , mLastMasterIndex(-1)
    , mType(StreamType::UNKNOWN)
    , mInterfaceId(-1)
    , mConfigMeta(nullptr)
{
    mDebugDrop = 1;//property_get_int32("vendor.debug.camera.p2c.drop", 0xFF);
    MY_LOGD("%s(%p) ctor", name.c_str(), this);
}

FeatureProcessor::~FeatureProcessor()
{
    MY_LOGD("%s(%p) dtor", mName.c_str(), this);
    this->uninit();
}

sp<IFeaturePipe> FeatureProcessor::getFeaturePipe(MINT32 sensorId)
{
    if ((MINT32)mFeaturePipeTable.size() > sensorId) {
        return mFeaturePipeTable[sensorId];
    } else {
        return sp<IFeaturePipe>();
    }
}

MBOOL FeatureProcessor::onInit(const FeatureInitParam &param)
{
    TRACE_S_FUNC_ENTER(log);

    mpStreamManager = param.mStreamManager.get();
    mType = param.mType;
    mInterfaceId = param.mInterfaceId;
    mpCallback = new CaptureRequestCallback(this);
    if (mpStreamManager.promote() != nullptr) {
        mConfigMeta = new MetadataPack(
                *mpStreamManager.promote()->getCommonMetadata(mType));
    } else {
        mConfigMeta = new MetadataPack;
    }
    mStreamAlgos = std::move(param.mAlgos);

    FeatureConfigParam fp;
    fp.mMetaPackPtr = new MetadataPack(*mConfigMeta);
    evaluateConfiguration(fp);

    // init and set callback to capturepipe with sensorID
    auto initCapturePipe = [&](MINT32 sensorID) -> MVOID {
        sp<IFeaturePipe> pFeaturePipe = getFeaturePipe(sensorID);
        if (pFeaturePipe == NULL) {
            MY_LOGE("OOM: cannot create FeaturePipe of sensorID %d", sensorID);
        } else {
            MY_LOGD("init and set callback to CapturePipe %d", sensorID);;
            pFeaturePipe->init();
            pFeaturePipe->setCallback(mpCallback);
        }
    };

    IFeaturePipe::UsageHint usage = IFeaturePipe::UsageHint();
    usage.mDualMode = 0;
    usage.mSupportedScenarioFeatures = ~0;
    usage.mPluginUniqueKey = mInterfaceId << 8 | mType;
    mFeaturePipeTable = {
        IFeaturePipe::createInstance(usage)
    };
    initCapturePipe(0);

    MY_LOGD("inited done with type %d!", (int ) mType);

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MVOID FeatureProcessor::onUninit()
{
    TRACE_S_FUNC_ENTER(mLog);

    ALOGD("Processor %s %s uninit size %zu!", mName.c_str(),
            getStreamTypeName(mType), mFeaturePipeTable.size());
    int i = 0;
    while(mFeaturePipeTable.size() > 0) {
        auto pipe = mFeaturePipeTable.back();
        if (pipe != NULL) {
            pipe->uninit();
        } else {
            ALOGE("oninit error!");
        }
        pipe = nullptr;
        mFeaturePipeTable.pop_back();
    }

    mpFeatureSettingPolicy = nullptr;

    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::onThreadStart()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:threadStart()");
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::onThreadStop()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:threadStop()");
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL FeatureProcessor::onConfig(const FeatureConfigParam &param)
{
    TRACE_S_FUNC_ENTER(mLog);
    evaluateConfiguration(param);
    TRACE_S_FUNC_EXIT(mLog);
    return MTRUE;
}

class P2MetadataHandle;

#define CHECK_ERROR(f, a) do { \
    if (f(a) != android::OK) { \
        ALOGW("%s: %s check failed!", __FUNCTION__, #f); \
        return MFALSE; \
    } \
} while (0);


MBOOL FeatureProcessor::onEnque(const sp<PA_Request> &pP2FrameRequest)
{

    StreamType stype = (StreamType)pP2FrameRequest->getStreamType();
    uint32_t rid = pP2FrameRequest->getRequestNo();
    MBOOL ret = MTRUE;
    MUINT64 fidFeatureSet = 0;
    FeatureFeatureID fid = NULL_FEATURE;
    ALOGD("%s:onEnque request id %d", getStreamTypeName(stype), rid);

    std::vector<AlgoType> streamAlgos;


    CHECK_ERROR(evaluateMetadata, pP2FrameRequest);
    CHECK_ERROR(evaluateRequest, pP2FrameRequest);
    CHECK_ERROR(evaluateIOBuffer, pP2FrameRequest);
    {
        android::AutoMutex _l(mRequestLock);
        mRequests.insert(
                std::make_pair(pP2FrameRequest->getRequestNo(),
                        pP2FrameRequest));
        streamAlgos = mStreamAlgos;
    }

    //dump buffer
    if ((stype == StreamType::CAPTURE && gDumpBufferCaptureEnabled)
            || (stype == StreamType::PREVIEW && gDumpBufferPreviewEnabled)) {
        dumpBuffer(pP2FrameRequest->mBI.mGraphicBuffer,
                String8::format("hal-t%s-%d", getStreamTypeName(stype), rid));
    }
    //check feature status
    {
        sp<IFeaturePipe> pFeaturePipe = getFeaturePipe(0);
        sp<IFeatureRequest> pRequest = pFeaturePipe->acquireRequest();

        /*MINT64 timestamp =
         pP2FrameRequest->getImg(IN_FULL) != nullptr ?
         pP2FrameRequest->getImg(IN_FULL)->getTimestamp() :
         systemTime() / 1000000000LL;
         ALOGD("timestamp %" PRId64, timestamp);*/
        MINT64 timestamp = systemTime() / 1000000000LL;

        pRequest->addParameter(PID_REQUEST_NUM,
                pP2FrameRequest->getRequestNo());
        pRequest->addParameter(PID_TIMESTAMP, timestamp);
        pRequest->addParameter(PID_FRAME_COUNT,
                pP2FrameRequest->getFrameCount());
        pRequest->addParameter(PID_FRAME_INDEX,
                pP2FrameRequest->getFrameIndex());
        for (auto it = streamAlgos.begin(); it < streamAlgos.end(); it++) {
            fid = convertAlgoType2FeatureId(*it);
            if (fid != NULL_FEATURE) {
                pRequest->addFeature(fid);
            }
        }


        //meta
         auto MapMetadata = [&](ID_META id, CaptureMetadataID metaId) -> void {
             if(pP2FrameRequest->isValidMeta(id)) {
                 pRequest->addMetadata(metaId, new P2MetadataHandle(pP2FrameRequest, id, this));
             }
         };

         MapMetadata(IN_PAS, MID_MAN_IN_APP);
         MapMetadata(OUT_PAS, MID_MAN_OUT_APP);

        // Image
        auto MapBuffer =
                [&](ID_IMG id, CaptureBufferID bufId, sp<ReleaseBundle> pBundle) -> MBOOL {
                    if (pP2FrameRequest->isValidImg(id)) {
                        MY_LOGI("MapBuffer img %d valid", id);
                        pRequest->addBuffer(bufId, new P2BufferHandle(pP2FrameRequest, id, pBundle, this));
                        return MTRUE;
                    }
                    return MFALSE;
                };

        MapBuffer(IN_FULL, BID_MAN_IN_YUV, NULL);
        MapBuffer(OUT_YUV, BID_MAN_OUT_YUV, NULL);
        MapBuffer(OUT_YUV0, BID_MAN_OUT_YUV0, NULL);
        MapBuffer(OUT_YUV1, BID_MAN_OUT_YUV1, NULL);
        MapBuffer(OUT_YUV2, BID_MAN_OUT_YUV2, NULL);
        MapBuffer(OUT_YUV3, BID_MAN_OUT_YUV3, NULL);
        MapBuffer(OUT_YUV4, BID_MAN_OUT_YUV4, NULL);
        MapBuffer(OUT_YUV5, BID_MAN_OUT_YUV5, NULL);
        MapBuffer(OUT_YUV6, BID_MAN_OUT_YUV6, NULL);
        MapBuffer(OUT_YUV7, BID_MAN_OUT_YUV7, NULL);

        ALOGD("enqueue request to FeaturePipe(%d), request id:%d", 0,
                pRequest->getRequestNo());
        pFeaturePipe->enque(pRequest);
    }

    TRACE_S_FUNC_EXIT(log);
    return ret;
}

MVOID FeatureProcessor::abortRequest(MINT32 reqrstNo)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:abort()");

    Vector<MINT32> masterIDs;
    Vector<sp<IFeatureRequest>> vpCapRequests;
    {
        /*Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {

            sp<PA_Request>& pP2Request = (*it).mpDetachP2Request;
            const P2FrameData& rFrameData = pP2Request->mP2Pack.getFrameData();
            sp<IFeatureRequest>& pCapRequest = (*it).mpCapRequest;
            MINT32 mID = rFrameData.mMasterSensorID;

            if (rFrameData.mMWFrameRequestNo == reqrstNo && !(*it).mAborting) {
                (*it).mAborting = MTRUE;
                vpCapRequests.push_back(pCapRequest);
                masterIDs.push_back(mID);
                // mark error
                pP2Request->updateResult(MFALSE);
            }
        }*/
    }

    mLastRequestNo = 0;
    mLastFrameCount = 0;
    mLastFrameIndex = 0;
    mLastMasterIndex = -1;

    int cnt = 0;
    for (auto& pCapRequest : vpCapRequests) {
        /*MY_LOGW("abort request, R/F Num:%d/%d",
                pCapRequest->getRequestNo(),
                pCapRequest->getFrameNo());*/
        MY_LOGW("abort request, R/F Num:%d",
                        pCapRequest->getRequestNo());
        sp<IFeaturePipe> pFeaturePipe = getFeaturePipe(0);
        pFeaturePipe->abort(pCapRequest);
        cnt++;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

FeatureFeatureID FeatureProcessor::convertAlgoType2FeatureId(AlgoType type) {
    FeatureFeatureID ret = NULL_FEATURE;
    switch (type) {
    case AlgoType::FILTER_PREVIEW:
        ret = FID_FILTER_PREVIEW;
        break;
    case AlgoType::FILTER_CAPTURE:
        ret = FID_FILTER_CAPTURE;
        break;
    case AlgoType::FILTER_MATRIX:
        ret = FID_FILTER_MATRIX;
        break;
    case AlgoType::FB:
        ret = FID_FB;
        break;
    case AlgoType::AUTOPANORAMA:
        ret = FID_AUTOPANORAMA;
        break;
    case AlgoType::PORTRAIT:
        ret = FID_PORTRAIT;
        break;
    case AlgoType::HDR:
        ret = FID_HDR;
        break;
    case AlgoType::VFB_PREVIEW:
        ret = FID_VFB_PREVIEW;
        break;
    case AlgoType::VFB_CAPTURE:
        ret = FID_VFB_CAPTURE;
        break;
    default:
        ALOGE("Not support algo type %d", type);
        return NULL_FEATURE;
    }
    ALOGD("convertAlgoType2FeatureId from AlgoType %d to Fid %d", (int )type,
            (int )ret);
    return ret;
}

status_t FeatureProcessor::evaluateConfiguration(const FeatureConfigParam &param) {
    android::AutoMutex _l(mRequestLock);
    bool doReset = false;
    if (param.mFlags & (1 << FeatureConfigParam::SET_AGLO)) {
        mStreamAlgos = std::move(param.mAlgos);
        doReset = true;
    }

    //update the config meta to the mConfigMeta, if the tag already existed, update the value
    //otherwise create a new entry.
    if (param.mMetaPackPtr != nullptr) {
        const IMetadata* metadata = param.mMetaPackPtr->getMetaPtr();
        for (int i = 0; i < metadata->count(); i++) {
            IMetadata::IEntry entry = metadata->entryAt(i);

/*            if (entry.tag() == MTK_POSTALGO_AUTORAMA_CMD) {
                mAutoRamaState = entry.itemAt(0, Type2Type<MINT32>());
                ALOGD("AutoRamaState is %d", mAutoRamaState);
            }*/
            if (entry.tag() == MTK_POSTALGO_JPEG_ORIENTATION) {
                mJpegOrientation = entry.itemAt(0, Type2Type<MINT32>());
                ALOGD("JPEG orientation is %d", mJpegOrientation);
            }
            mConfigMeta->update(entry.tag(), entry);
        }
    }

    if (mpFeatureSettingPolicy == nullptr || doReset) {
        mpFeatureSettingPolicy = new FeatureSettingPolicy(
                FeatureSettingPolicy::CreationParams { .type = mType, .mIntfId =
                        mInterfaceId, .algos = mStreamAlgos });
    }
    return android::OK;
}

status_t FeatureProcessor::evaluateRequest(const sp<PA_Request>& request) {
    android::AutoMutex _l(mRequestLock);
    if (mJpegOrientation != 0) {
        request->mJpegOrientation = mJpegOrientation;
        ALOGD("change request's jpeg orientation %d", mJpegOrientation);
    }
    //evaluate request
    if (mpFeatureSettingPolicy != nullptr) {
        return mpFeatureSettingPolicy->evaluateRequest(request);
    }
    return android::OK;
}

status_t FeatureProcessor::evaluateMetadata(const sp<PA_Request>& request) {
    android::AutoMutex _l(mRequestLock);
    const sp<MetadataPack> updateMeta = new MetadataPack(*mConfigMeta);
    ALOGD("r[%d]evaluateMetadata rama state %d", request->getRequestNo(),
            mAutoRamaState);
    if (mAutoRamaState == AUTORAMA_MERGE) {
        IMetadata::IEntry update;
        update.push_back(AUTORAMA_QUEUE, Type2Type<MINT32>());
        updateMeta->update(MTK_POSTALGO_AUTORAMA_CMD, update);
        ALOGD("received merged and send autorama queue cmd!");
        //no need to load autorama cmd from config
        mConfigMeta->getMetaPtr()->remove(MTK_POSTALGO_AUTORAMA_CMD);
    }
    request->addMetadata(IN_PAS, updateMeta);
    request->addMetadata(OUT_PAS, new MetadataPack());
    return android::OK;
}

status_t FeatureProcessor::evaluateIOBuffer(const sp<PA_Request>& request) {
    StreamType stype = (StreamType) request->getStreamType();
    const sp<GraphicBuffer>& halGb = request->getBufferItem().mGraphicBuffer;
    uint32_t h_height = halGb->getHeight();
    uint32_t h_width = halGb->getWidth();
    sp<ImageBufferInfo> imgBufferInfo = new ImageBufferInfo(
            request->getHalStream(), halGb,
            NSPA::convertImageBufferFromGraphicBuffer(halGb));
    request->addImg(IN_FULL, imgBufferInfo);

    //handle OUT_YUV
    /// preview for 9 yuv output
    sp<IStreamManager> sm = mpStreamManager.promote();
    if (sm == nullptr) {
        ALOGE("Error to get StreamManager");
        return android::INVALID_OPERATION;
    }
    sp<PipelineConfigure> fp = sm->getConfigure(stype);
    KeyedVector<int32_t, sp<ImageStreamInfo>>  app_streamInfos;

    ANativeWindow* anw = nullptr;
    ANativeWindowBuffer *buffer = NULL;
    sp<android::GraphicBuffer> ap_gb = nullptr;
    status_t err = android::OK;
    int fenceFd = -1;
    ID_IMG img_id = NO_IMG;

#define MAP_YUV_ID(i) \
            case i: \
            img_id = i>=0?OUT_YUV##i:OUT_YUV; \
            break;

    auto get_image_id = [&](int i) {
        if (ap_gb != nullptr) {
            switch(i) {
                MAP_YUV_ID(0);
                MAP_YUV_ID(1);
                MAP_YUV_ID(2);
                MAP_YUV_ID(3);
                MAP_YUV_ID(4);
                MAP_YUV_ID(5);
                MAP_YUV_ID(6);
                MAP_YUV_ID(7);
                default:
                img_id = NO_IMG;
                break;
            }
        } else {
            img_id = NO_IMG;
        }
    };

    {
        Mutex::Autolock _(fp->mPipeConfigLock);
        app_streamInfos = fp->mAppStreamInfoMap;
    }

    int32_t cformat = -1;
    for (int i = 0; i < app_streamInfos.size(); i++) {
        //dequeue buffer and add to output yuv
        const sp<ImageStreamInfo> streamInfo = app_streamInfos.valueAt(i);
        anw = streamInfo->mStream.get();
        //check format if it's blob

        /// capture & blob for JPEG capture
        /// preview & blob for autorama
        if ((cformat = ANativeWindow_getFormat(anw))
                        == HAL_PIXEL_FORMAT_BLOB) {
            int32_t width = 0;
            int32_t height = 0;
            int32_t orientation = 0;
            uint64_t u = 0;
            status_t res;
            //using pixel size
            if (StreamType::CAPTURE == stype) {
                if ((request->mJpegOrientation + 45) / 90 % 2 == 1) {
                    width = streamInfo->mHeight;
                    height = streamInfo->mWidth;
                } else {
                    width = streamInfo->mWidth;
                    height = streamInfo->mHeight;
                }
            } else if (StreamType::PREVIEW == stype) {
                android::AutoMutex _l(mRequestLock);
                if (mAutoRamaState == AUTORAMA_MERGE) {
                    //current should be queue state
                    const auto& entry = mConfigMeta->entryFor(
                            MTK_POSTALGO_AUTORAMA_BUFFER_SIZE);
                    if (entry.count() > 0) {
                        width = entry.itemAt(0, Type2Type<MINT32>());
                        height = entry.itemAt(1, Type2Type<MINT32>());
                        if ((request->mJpegOrientation + 45) / 90 % 2 == 1) {
                            int32_t temp = width;
                            width = height;
                            height = temp;
                        }
                        ALOGD("r[%d]AutoRama return buffer size %dx%d-r%d", request->getRequestNo(), width,
                                height, request->mJpegOrientation);
                        mAutoRamaState = AUTORAMA_UNKNOWN;
                    } else {
                        ALOGW("r[%d]don't received buffer size from autorama merge state!", request->getRequestNo());
                        width = h_width;
                        height = h_height;
                        request->getMetadataPack(IN_PAS)->update(
                                MTK_POSTALGO_AUTORAMA_CMD, AUTORAMA_CANCEL);
                        mConfigMeta->getMetaPtr()->remove(
                                MTK_POSTALGO_AUTORAMA_CMD);
                        mAutoRamaState = AUTORAMA_UNKNOWN;
                    }
                } else if (mAutoRamaState == AUTORAMA_UNKNOWN
                        || mAutoRamaState == AUTORAMA_QUEUE) {
                    //case not start or finished autorama
                    width = 1;
                    height = 1;
                } else {
                    //start
                    width = h_width;
                    height = h_height;
                }
            }


            /// JPEG only support NV21, NV12, YUYV, YVY2
            if(request->isNeedWorkingBuffer()) {
                res = native_window_get_consumer_usage(anw, &u);
                            ALOGD("%s:Request %d Blob format(width = %d, height = %d, orientation = %d) and using working buffer!",
                                    getStreamTypeName(stype), request->getRequestNo(), width,
                                    height, orientation);

                ap_gb = new GraphicBuffer(width, height,
                        HAL_PIXEL_FORMAT_YCRCB_420_SP, u);

                if (ap_gb == nullptr) {
                    err = android::NO_MEMORY;
                }
            }
        } else {
            //This call may block if no buffers are available.
            err = anw->dequeueBuffer(anw, &buffer,
                    &fenceFd);
            ALOGD("releaseFenceFd = %d", fenceFd);
            if (fenceFd > 0) {
                sp<Fence> releaseFence(new Fence(fenceFd));
                releaseFence->waitForever("postAlgo preview releaseFence");
            }
            if(err != android::OK) {
                ALOGE("Errot to dequeue buffer from ap %d/%p",
                        app_streamInfos.keyAt(i),
                        streamInfo->mStream.get());
            } else {
                ap_gb = android::GraphicBuffer::from(buffer);
            }
        }

        if(request->isNeedWorkingBuffer()) {
            if (err == android::OK) {
                if (i == 0) {
                    img_id = OUT_YUV;
                    imgBufferInfo = new ImageBufferInfo(streamInfo->mStream, ap_gb,
                            NSPA::convertImageBufferFromGraphicBuffer(ap_gb), request->mJpegOrientation, fenceFd);
                } else {
                    get_image_id(i - 1);
                    imgBufferInfo = new ImageBufferInfo(streamInfo->mStream, ap_gb,
                            NSPA::convertImageBufferFromGraphicBuffer(ap_gb), request->mJpegOrientation, fenceFd);
                }
                if (img_id != NO_IMG) {
                    request->addImg(img_id, imgBufferInfo);
                ALOGD("evalauteIOBuffer prepare image idx/Id %d/%d", i, img_id);
                }
            } else {
                ALOGV("Failed to dequeue buffer and drop it!");
                request->getHalConsumer()->releaseBuffer(request->mBI);
                request->mImg.clear();
                return android::UNKNOWN_ERROR;
            }
        }
    }
    return android::OK;
}


#if 0
MVOID FeatureProcessor::setupIOBufferInPlace(const sp<PA_Request>& request) {
    request->addImg(IN_FULL,
            NSPA::convertImageBufferFromGraphicBuffer(
                    request->getBufferItem().mGraphicBuffer));
    //current we only support one preview stream
    if (request->getStreamType() == StreamType::CAPTURE) {
        request->addImg(OUT_YUV,
                NSPA::convertImageBufferFromGraphicBuffer(
                        request->getBufferItem().mGraphicBuffer));
    } else if (request->getCombinedFeatureSet() & (1 << FID_FILTER_MATRIX)) {
        /// preview for 9 yuv output
        sp<IStreamManager> sm = mpStreamManager.promote();
        if (sm == nullptr) {
            ALOGD("Error to get StreamManager");
        }
        sp<PipelineConfigure> fp = sm->getConfigure(StreamType::PREVIEW);
        KeyedVector<int32_t, sp<android::Surface>> app_sfs;
        {
            Mutex::Autolock _(fp->mPipeConfigLock);
            app_sfs = fp->mAppStreams;
        }
        ///TODO:should we known the max preview size through app metadata ?
        if (app_sfs.size() > 9) {
            ALOGW("App Preview surface count/%zd beyond the output size/%d!",
                    app_sfs.size(), 9);
            return;
        }
        sp<ANativeWindow> aNativeWindow = nullptr;
        ANativeWindowBuffer *buffer = NULL;
        status_t err = android::OK;
        int fenceFd;
        ID_IMG img_id = NO_IMG;

#define MAP_YUV_ID(i) \
        case i: \
        img_id = i>=0?OUT_YUV##i:OUT_YUV; \
        break;


        sp<android::GraphicBuffer> ap_gb = nullptr;
        auto add_image =
                [&](int i, sp<android::GraphicBuffer>& gb ) -> bool {
                    if (gb != nullptr) {
                        switch(i) {
                            MAP_YUV_ID(0);
                            MAP_YUV_ID(1);
                            MAP_YUV_ID(2);
                            MAP_YUV_ID(3);
                            MAP_YUV_ID(4);
                            MAP_YUV_ID(5);
                            MAP_YUV_ID(6);
                            MAP_YUV_ID(7);
                            default:
                                img_id = NO_IMG;
                            break;
                        }
                        return request->addImg(img_id, NSPA::convertImageBufferFromGraphicBuffer(gb));
                    } else {
                        return false;
                    }
                };
        for (int i = 0; i < app_sfs.size(); i++) {
            //dequeue buffer and add to output yuv
            aNativeWindow = app_sfs.valueAt(i);
            err = aNativeWindow->dequeueBuffer(aNativeWindow.get(), &buffer,
                    &fenceFd);
            ap_gb = android::GraphicBuffer::from(buffer);
            if(i == 0) {
                request->addImg(OUT_YUV,
                        NSPA::convertImageBufferFromGraphicBuffer(ap_gb));
            } else {
                add_image(i-1, ap_gb);
            }
        }
    } else {
        request->addImg(OUT_YUV,
                NSPA::convertImageBufferFromGraphicBuffer(
                        request->getBufferItem().mGraphicBuffer));
    }
}
#endif

MVOID FeatureProcessor::notifyRelease(MINT32 requestNo, MINT32 frameNo)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:notifyRelease()");
#if 0
    Mutex::Autolock _l(mPairLock);

    auto it = mRequestPairs.begin();
    for (; it != mRequestPairs.end(); it++) {
        const sp<IFeatureRequest>& pCapRequest = (*it).mpCapRequest;
        if ((pCapRequest->getRequestNo() == requestNo) /*&& (pCapRequest->getFrameNo() == frameNo)*/) {
            const sp<IP2Frame>& pP2Frame = (*it).mpP2Frame;
            pP2Frame->endBatchRelease();
            pP2Frame->beginBatchRelease();

            MY_LOGI("notify release, R/F Num:%d/%d", requestNo, frameNo);
            break;
        }
    }
#endif
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::releaseImage(sp<PA_Request> pRequest, ID_IMG imgId)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:releaseImage()");
//    Mutex::Autolock _l(mPairLock);
    pRequest->releaseImageWithLock(imgId);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::releaseMeta(sp<PA_Request> pRequest, ID_META metaId)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:releaseMeta()");
//    Mutex::Autolock _l(mPairLock);
    pRequest->releaseMetaWithLock(metaId);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::onNotifyFlush()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:notifyFlush()");
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::waitRequestDone()
{
    TRACE_S_FUNC_ENTER(mLog);
#if 0
    android::Mutex::Autolock _l(mPairLock);

    while (mRequestPairs.size())
    {
        MY_LOGI("wait +, size: %zu", mRequestPairs.size());
        mPairCondition.wait(mPairLock);
        MY_LOGI("wait -, size: %zu", mRequestPairs.size());
    }
#endif
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::waitRequestAborted()
{
    TRACE_S_FUNC_ENTER(mLog);
#if 0
    android::Mutex::Autolock _l(mPairLock);
    auto CountAboringRequest = [&]() -> MUINT32 {
        MUINT32 count = 0;
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            if ((*it).mAborting && !(*it).mRestored) {
                count++;
            }
        }
        return count;
    };

    size_t aborting;
    while (aborting = CountAboringRequest())
    {
        MY_LOGI("wait +, size:%zu aborting: %zu", mRequestPairs.size(), aborting);
        mPairCondition.wait(mPairLock);
        MY_LOGI("wait -, size:%zu", mRequestPairs.size());
    }
#endif

    TRACE_S_FUNC_EXIT(mLog);
}

MVOID FeatureProcessor::onWaitFlush()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Capture:waitFlush()");
    // That could happened. The multi-frame request could be flushed before all frames are received.
    if (mLastFrameCount > 0 && mLastFrameCount != mLastFrameIndex + 1) {
        MY_LOGI("abort a request[%d] whose all frames are not queued.", mLastRequestNo);
        mAbortingRequestNo = mLastRequestNo;
        abortRequest(mLastRequestNo);
    }

    const MBOOL bgPreRelease = MFALSE; //mP2Info.getConfigInfo().mUsageHint.mBGPreRelease;
    MY_LOGI("(%p) flush +, bgPreRelease:%d", this, bgPreRelease);
    if (bgPreRelease) {
        waitRequestAborted();
#if 0
        Mutex::Autolock _l(mPairLock);
        const size_t requestPairsSize = mRequestPairs.size();

        // MUST wait all requests inference done before dispatching frame
        // because mfll's request is delay inference and
        // request will alloc input pipeline buffer in cross request's inference
        for (size_t i = 0; i < mRequestPairs.size(); ++i) {
           if (!mRequestPairs[i].mDetached) {
              mRequestPairs[i].mpCapRequest->waitInference();
           }
        }

        for (size_t i = 0; i < mRequestPairs.size(); ++i)
        {
            const sp<IP2Frame>& pP2Frame = mRequestPairs[i].mpP2Frame;
            const sp<PA_Request>& pDetachRequest = mRequestPairs[i].mpDetachP2Request;
            const MBOOL hadDetached = mRequestPairs[i].mDetached;
            const sp<IFeatureRequest>& pCapRequest = mRequestPairs[i].mpCapRequest;
//            const ILog log = spToILog(pDetachRequest);
            if (!hadDetached) {
                pDetachRequest->detachResourceWithLock(PA_Request::RES_ALL);
                pP2Frame->endBatchRelease();
                auto& rPair = mRequestPairs.editItemAt(i);
                sp<IP2Frame> p2FrameTmp = rPair.mpP2Frame;
                rPair.mpP2Frame = new P2FrameHolder(NULL);
                rPair.mDetached = MTRUE;
                MY_LOGI("detach request all resource, index:%zu/%zu, capReq-R/F Num:%d",
                    i, requestPairsSize, pCapRequest->getRequestNo());
            } else {
                MY_LOGI("request had be detached, index:%zu/%zu, capReq-R/F Num:%d",
                    i, requestPairsSize, pCapRequest->getRequestNo());
            }
        }

        //REMOVE ME!! Temporarily to avoid timing issue, onDispatchFrame is called in another thread so that
        // JpegNode has little probability to be called by flush(frame) after node's flush()
        if (mRequestPairs.size() > 0) {
            usleep(1000); //1ms
        }
#endif
    }
    else {
        for (auto iter = mFeaturePipeTable.begin(); iter != mFeaturePipeTable.end(); iter++) {
            /*if(iter->second != NULL)
                iter->second->flush();*/
            if (*iter != NULL) {
                (*iter)->flush();
            }
        }
    }
    if (!bgPreRelease) {
        waitRequestDone();
    }

    MY_LOGI("flush -");
    TRACE_S_FUNC_EXIT(mLog);
}

std::string FeatureProcessor::onGetStatus()
{
    std::string str;
    for (auto iter = mFeaturePipeTable.begin(); iter != mFeaturePipeTable.end(); iter++) {
        /*if(iter->second != NULL) {
            auto strTmp = iter->second->getStatus();
            if (!strTmp.empty()) {
                if (str.length() > 0) {
                    str += '\t';
                }
                str += strTmp;
            }
        }*/
        if (*iter != NULL) {
            auto strTmp = (*iter)->getStatus();
            if (!strTmp.empty()) {
                if (str.length() > 0) {
                    str += '\t';
                }
                str += strTmp;
            }
        }
    }
    return str;
}

CaptureRequestCallback::CaptureRequestCallback(FeatureProcessor* pProcessor)
    : mpProcessor(pProcessor)
{

}

void CaptureRequestCallback::onContinue(sp<IFeatureRequest> pCapRequest)
{
    TRACE_S_FUNC_ENTER(mpProcessor->mLog);
#if 0
    Mutex::Autolock _l(mpProcessor->mPairLock);
    auto it = mpProcessor->mRequestPairs.begin();
    for (; it != mpProcessor->mRequestPairs.end(); it++) {
        if ((*it).mpCapRequest == pCapRequest) {
            const sp<IP2Frame>& pP2Frame = (*it).mpP2Frame;
            const sp<PA_Request>& pDetachRequest = (*it).mpDetachP2Request;
//            const ILog log = spToILog(pDetachRequest);
            pP2Frame->notifyNextCapture(
                    static_cast<MUINT32>(mpProcessor->mRequestPairs.size()),
                    pCapRequest->getParameter(PID_CSHOT_REQUEST) >= 0);
            MY_LOGI("(%p) notify next capture, request count:%zu, R/F Num:%d",
                this, mpProcessor->mRequestPairs.size(), pCapRequest->getRequestNo());
            break;
        }
    }
#endif
    TRACE_S_FUNC_EXIT(mpProcessor->mLog);
}

void CaptureRequestCallback::onRestored(sp<IFeatureRequest> pCapRequest)
{
    TRACE_S_FUNC_ENTER(mpProcessor->mLog);
#if 0
    Mutex::Autolock _l(mpProcessor->mPairLock);
    auto it = mpProcessor->mRequestPairs.begin();
    for (; it != mpProcessor->mRequestPairs.end(); it++) {
        if ((*it).mpCapRequest == pCapRequest) {
            const sp<PA_Request>& pDetachRequest = (*it).mpDetachP2Request;
            MY_LOGI("(%p) restore an aborting(%d) request, request count:%zu, R/F Num:%d",
                this,
                (*it).mAborting, mpProcessor->mRequestPairs.size(),
                pCapRequest->getRequestNo());

            (*it).mRestored = MTRUE;
            // restore
            pDetachRequest->updateResult(MTRUE);
            mpProcessor->mPairCondition.broadcast();

            break;
        }
    }
#endif
    TRACE_S_FUNC_EXIT(mpProcessor->mLog);
}


void CaptureRequestCallback::onAborted(sp<IFeatureRequest> pCapRequest)
{
    TRACE_S_FUNC_ENTER(mpProcessor->mLog);
#if 0
    Mutex::Autolock _l(mpProcessor->mPairLock);
    auto it = mpProcessor->mRequestPairs.begin();
    for (; it != mpProcessor->mRequestPairs.end(); it++) {
        if ((*it).mpCapRequest == pCapRequest) {
            CAM_ULOG_DISCARD(MOD_P2_CAP_PROC, REQ_P2_CAP_REQUEST, pCapRequest->getFrameNo());
            const sp<IP2Frame>& pP2Frame = (*it).mpP2Frame;
            const sp<PA_Request>& pDetachRequest = (*it).mpDetachP2Request;
//            const ILog log = spToILog(pDetachRequest);
            pDetachRequest->updateResult(MFALSE);
            pDetachRequest->updateMetaResult(MFALSE);
            pDetachRequest->releaseResourceWithLock(PA_Request::RES_ALL);
            pP2Frame->endBatchRelease();
            mpProcessor->mRequestPairs.erase(it);
            mpProcessor->mPairCondition.broadcast();
            MY_LOGI("(%p) erased an aborted request, request count:%zu, R/F Num:%d",
                this, mpProcessor->mRequestPairs.size(), pCapRequest->getRequestNo());
            break;
        }
    }
#endif
    TRACE_S_FUNC_EXIT(mpProcessor->mLog);
}

void CaptureRequestCallback::onCompleted(sp<IFeatureRequest> pCapRequest, MERROR ret)
{
    TRACE_S_FUNC_ENTER(mpProcessor->mLog);
    int32_t frm_no = pCapRequest->getParameter(PID_REQUEST_NUM);
    int32_t frm_count = pCapRequest->getParameter(PID_FRAME_COUNT);
    int32_t frm_idx = pCapRequest->getParameter(PID_FRAME_INDEX);
    sp<PA_Request> request;
    {
        Mutex::Autolock _l(mpProcessor->mRequestLock);
        auto it = mpProcessor->mRequests.find(frm_no);
        if (it == mpProcessor->mRequests.end()) {
            ALOGE("error not found the request info %d ", frm_no);
            return;
        }
        request = it->second;
        mpProcessor->mRequests.erase(it);
       //if multi-frame, remove the previous request;
        if (frm_count > 1 && frm_idx != 0) {
            ALOGD("r[%d]ignore multi-frame request ,  C/I %d/%d", frm_no, frm_count, frm_idx);
            request->getHalConsumer()->releaseBuffer(request->mBI);
            return ;
        } else {
            ALOGD("r[%d]onCompleted , C/I-%d/%d!", frm_no, frm_count, frm_idx);
        }
        //update metadata
        const auto& metaPack = request->getMetadataPack(OUT_PAS);
        const auto& entry = metaPack->entryFor(MTK_POSTALGO_AUTORAMA_CMD);
        if (entry.count() > 0) {
            mpProcessor->mAutoRamaState = entry.itemAt(0, Type2Type<MINT32>());
            ALOGD("r[%d]update Autorama cmd stat %d", request->getRequestNo(),
                    mpProcessor->mAutoRamaState);
            if (mpProcessor->mAutoRamaState == AUTORAMA_CANCEL
                    || mpProcessor->mAutoRamaState == AUTORAMA_COMPLETE) {
                ALOGD("%s and remove Autorama tag",
                        mpProcessor->mAutoRamaState == AUTORAMA_CANCEL ?
                                "Cancel" : "Complete");
                mpProcessor->mConfigMeta->getMetaPtr()->remove(
                        MTK_POSTALGO_AUTORAMA_CMD);
                mpProcessor->mAutoRamaState = AUTORAMA_UNKNOWN;
            } else if (mpProcessor->mAutoRamaState == AUTORAMA_MERGE) {
                    const auto& bf_entry = metaPack->entryFor(
                            MTK_POSTALGO_AUTORAMA_BUFFER_SIZE);
                    if (!bf_entry.isEmpty()) {
                        mpProcessor->mConfigMeta->update(
                                MTK_POSTALGO_AUTORAMA_BUFFER_SIZE, bf_entry);
                    }
            } else if (mpProcessor->mAutoRamaState == AUTORAMA_QUEUE) {
                mpProcessor->mConfigMeta->update(MTK_POSTALGO_AUTORAMA_CMD,
                        entry);
            }
        }
    }

    StreamType type = (StreamType) request->getStreamType();

    sp<IStreamManager> streamMgr = mpProcessor->mpStreamManager.promote();
    if (streamMgr != nullptr) {
        streamMgr->updateResult(request);
    }
    TRACE_S_FUNC_EXIT(mpProcessor->mLog);
    ALOGD("onCompleted request id %d done!", frm_no);
}


} // namespace P2
