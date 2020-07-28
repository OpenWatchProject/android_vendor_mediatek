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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "ThirdPartyPostProcessor"

// Standard C header file
#include <sys/prctl.h>
#include <sys/resource.h>
// Android system/core header file
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
// Module header file
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
// tbl
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/table_data_set.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/fulltbl.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/single_cap_f_w_jpeg.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/single_cap_f_wo_jpeg.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/dual_cap_main1_IR.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/dual_cap_main2_IR.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/dual_cap_main2_R.h>
//
#include "../../LegacyPipeline/stereo/StereoLegacyPipeline.h"
#include "ThirdPartyPostProcessor.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif // SCOPE_TIMER

using namespace android;
using namespace android::NSPostProc;
using namespace NSCam::v1::NSLegacyPipeline;
#define OPEN_ID 100

namespace
{
    // type alias ProcessingBuilder
    constexpr MINT32 PROCESSINGREQNUM = 0;
    constexpr MINT32 BUILDERPTR       = 1;

    // type alias IPostProcRequestBuilder::CallbackBufferPoolItem
    constexpr MINT32 POOLNAME           = 0;
    constexpr MINT32 CALLBACKBUFFERPOOL = 1;

    constexpr MUINT32 gStartRequestNo = 8000;
    constexpr MUINT32 gMaxRequestNo   = 9000;

    template<typename T>
    T toOnebasedNumbering(T value)
    {
        return (value + 1);
    }

    template<typename Key, typename Value>
    MBOOL tryGetValue(std::map<Key, Value>& table, const Key& key, Value& value)
    {
        MBOOL bRet = MFALSE;
        auto it = table.find(key);
        if(it != table.end())
        {
            value = std::get<1>(*it);
            bRet = MTRUE;
        }
        return bRet;
    }

    template<typename Key, typename Value>
    MBOOL tryEarse(std::map<Key, Value>& table, const Key& key, Value& value)
    {
        MBOOL bRet = MFALSE;
        auto it = table.find(key);
        if(it != table.end())
        {
            value = it->second;
            table.erase(it);
            bRet = MTRUE;
        }
        return bRet;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
ThirdPartyPostProcessor::
ThirdPartyPostProcessor(
    PostProcessorType type
)
: ImagePostProcessorBase(type)
, miRequestNo(gStartRequestNo)
{
    FUNCTION_SCOPE;
    auto toString = [&type]()
    {
        switch(type)
        {
        case PostProcessorType::NONE:
            return "NONE";
        case PostProcessorType::BOKEH:
            return "BOKEH";
        case PostProcessorType::THIRDPARTY_MFNR:
            return "THIRDPARTY_MFNR";
        case PostProcessorType::THIRDPARTY_HDR:
            return "THIRDPARTY_HDR";
        case PostProcessorType::THIRDPARTY_BOKEH:
            return "THIRDPARTY_BOKEH";
        case PostProcessorType::THIRDPARTY_DCMF:
            return "THIRDPARTY_DCMF";
        case PostProcessorType::DUMMY:
            return "DUMMY";
        default:
            return "";
        }
    };

    // TODO: modify here when add the new inputbuffer
    {
        mInputBufferPoolTable.insert(std::make_pair(eSTREAMID_IMAGE_PIPE_YUV_00, std::make_tuple("yuv_00", nullptr)));
        mInputBufferPoolTable.insert(std::make_pair(eSTREAMID_IMAGE_PIPE_YUV_01, std::make_tuple("yuv_01", nullptr)));
    }

    MY_LOGD("Type [%s]", toString());
    {
        SCOPE_TIMER(t1, "build pipeline");
        gFrameDataTableSet.clear();
        // add table set
        // these table not use to descript full pipeline.
        // these table is use to descript how pipeline frame will go in full pipeline.
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("single_third_party_full_table"),
                        ThirdPartyType::FULL,
                        BackgroundServiceData::PP::BG_SRV_FULL::gBGMetaTbl,
                        BackgroundServiceData::PP::BG_SRV_FULL::gBGImgStreamTbl,
                        BackgroundServiceData::PP::BG_SRV_FULL::gBGConnectSetting));
        // MFNR/HDR possible flow
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("single_third_party_full_yuv_with_jpeg"),
                        ThirdPartyType::SINGLE_3RD_F_W_JPEG,
                        BackgroundServiceData::PP::SINGLE_CAP_F_W_JPEG::gBGMetaTbl,
                        BackgroundServiceData::PP::SINGLE_CAP_F_W_JPEG::gBGImgStreamTbl,
                        BackgroundServiceData::PP::SINGLE_CAP_F_W_JPEG::gBGConnectSetting));
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("single_third_party_full_yuv_without_jpeg"),
                        ThirdPartyType::SINGLE_3RD_F_WO_JPEG,
                        BackgroundServiceData::PP::SINGLE_CAP_F_WO_JPEG::gBGMetaTbl,
                        BackgroundServiceData::PP::SINGLE_CAP_F_WO_JPEG::gBGImgStreamTbl,
                        BackgroundServiceData::PP::SINGLE_CAP_F_WO_JPEG::gBGConnectSetting));
        // dual cam possible flow
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("dual_third_party_main1_IR_without_jpeg"),
                        ThirdPartyType::DUAL_3RD_FR_WO_JPEG,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN1_IR::gBGMetaTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN1_IR::gBGImgStreamTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN1_IR::gBGConnectSetting));
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("dual_third_party_main2_IR_with_jpeg"),
                        ThirdPartyType::DUAL_3RD_FR_W_JPEG,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_IR::gBGMetaTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_IR::gBGImgStreamTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_IR::gBGConnectSetting));
        gFrameDataTableSet.push_back(FrameDataTableSet(
                        string("dual_third_party_main2_R_with_jpeg"),
                        ThirdPartyType::DUAL_3RD_R_W_JPEG,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_R::gBGMetaTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_R::gBGImgStreamTbl,
                        BackgroundServiceData::PP::DUAL_CAP_MAIN2_R::gBGConnectSetting));
        // hardcode for dual cam case.
        // Todo: need to fix.
        mSensorId = 0;
        mSensorIdMain2 = 2;
        if(buildPipeline() != OK)
        {
            MY_LOGE("build postprocessor fail.");
        }
    }
    // create listener
    mpListener = new TPPPListener(this);
    if(mpListener == nullptr)
    {
        MY_LOGE("create listener fail");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
ThirdPartyPostProcessor::
~ThirdPartyPostProcessor()
{
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
ThirdPartyPostProcessor::
onLastStrongRef(
    const void* /* id */
)
{
    FUNCTION_SCOPE;
    flush();
    waitUntilDrained();
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyPostProcessor::
doPostProc(
    sp<PostProcRequestSetting> setting
)
{
    FUNCTION_SCOPE;

    status_t ret = UNKNOWN_ERROR;

    const MINT64 timestamp = setting->miTimeStamp;
    const MINT32 reqConut = setting->mvInputData.size();
    const MINT32 lastReqNum = reqConut - 1;
    const MINT32 lastFrameNum = (lastReqNum >= 0) ? (setting->mvInputData[lastReqNum].mImageQueue.size() - 1) : -1;
    const MINT32 frameCount = lastFrameNum + 1;
    MY_LOGD("timestamp:%lld, reqConut:%d, frameCount:%d, isFinalData:%d", timestamp, reqConut, frameCount, setting->mIsFinalData);


    MINT32 builderProcessingReqNum = -1;
    sp<IPostProcRequestBuilder> builderPtr = nullptr;
    {
        Mutex::Autolock _l(mPostProcRequestTableLock);
        const MINT64 key = timestamp;

        ProcessingBuilder temp;
        if(::tryGetValue(mPostProcRequestTable, key, temp))
        {
            builderProcessingReqNum = std::get<PROCESSINGREQNUM>(temp);
            builderPtr = std::get<BUILDERPTR>(temp);
            MY_LOGD("find builder: builderProcessingReqNum:%d, key(timestamp):%lld, value(addr):%p", builderProcessingReqNum, key, builderPtr.get());
        }
    }

    if(builderPtr == nullptr)
    {
        MY_LOGD("builder is not existing, create the new one");
        builderPtr = PostProcRequestBuilderFactory::createInstance(mpPipeline, mpImageStreamManager, setting);

        if(builderPtr == nullptr)
        {
            MY_LOGE("build builder fail. please check previous log");
            goto lbExit;
        }

        {
            Mutex::Autolock _l(mPostProcRequestTableLock);

            builderProcessingReqNum = miRequestNo;
            mPostProcRequestTable.insert(std::make_pair(timestamp, std::make_tuple(builderProcessingReqNum, builderPtr)));

            MY_LOGD("insert builder to table: builderProcessingReqNum:%d, key(timestamp):%lld, value(addr):%p, tableSize:%zu",
                builderProcessingReqNum, timestamp, builderPtr.get(), mPostProcRequestTable.size());
        }
        builderPtr->setEventCallback(mpListener);
    }

    // submit sub-frame to pipeline
    MY_LOGD("update pipelineReq#, new:%d, old:%d", builderProcessingReqNum, setting->mRequestNo);
    setting->mRequestNo = builderProcessingReqNum;

    builderPtr->setRequestSetting(setting);
    for(MINT32 reqNum = 0; reqNum < reqConut; reqNum++)
    {
        InputData& frame = setting->mvInputData[reqNum];
        const MINT32 frameCount = frame.mImageQueue.size();
        for(MINT32 frameNum = 0; frameNum < frameCount; frameNum++)
        {
            IMetadata appMetadata;
            IMetadata halMetadata;
            BufferList vDstStream;
            ThirdPartyType type;

            MY_LOGD("prepare request to pipeline, pipelineReq#:%d, data_req#:(%d/%d), data_frame#:(%d/%d)",
                    setting->mRequestNo, ::toOnebasedNumbering(reqNum), reqConut, ::toOnebasedNumbering(frameNum), frameCount);


            if(builderPtr->setInputBuffer(reqNum, frameNum, mInputBufferPoolTable) != OK)
            {
                MY_LOGE("set input buffer fail. please check previous log");
                goto lbExit;
            }

            if((setting->mIsFinalData) && (reqNum == lastReqNum) && (frameNum == lastFrameNum))
            {
                MY_LOGD("last sub-frame: pipelineReq#:%d, frame#:%d, sub-frame#:%d",
                        setting->mRequestNo, reqNum, frameNum);

                if(builderPtr->allocOutputBuffer() != OK)
                {
                    MY_LOGE("alloc output buffer fail. please check previous log");
                    goto lbExit;
                }

                if(builderPtr->getWorkingData(reqNum, frameNum, appMetadata, halMetadata, vDstStream, type) != OK)
                {
                    MY_LOGE("get preceding data fail. please check previous log");
                    goto lbExit;
                }
            }
            else
            {
                if(builderPtr->getPrecedingData(reqNum, frameNum, appMetadata, halMetadata, vDstStream, type) != OK)
                {
                    MY_LOGE("get preceding data fail. please check previous log");
                    goto lbExit;
                }
            }

            MY_LOGD("submit request to pipeline, pipelineReq#:%d, data_req#:(%d/%d), data_frame#:(%d/%d)",
                setting->mRequestNo, ::toOnebasedNumbering(reqNum), reqConut, ::toOnebasedNumbering(frameNum), frameCount);

            StereoLegacyPipeline* slPipeline = reinterpret_cast<StereoLegacyPipeline*>(mpPipeline.get());
            slPipeline->submitRequest(setting->mRequestNo, appMetadata, halMetadata, vDstStream,(MINT32)type);

            setting->mRequestNo++;
        }

        MINT32& previousBuilderProcessingReqNum = std::get<PROCESSINGREQNUM>(mPostProcRequestTable[timestamp]);
        MY_LOGD("update processorReq#, new:%d, old:%d", setting->mRequestNo, previousBuilderProcessingReqNum);
        previousBuilderProcessingReqNum = setting->mRequestNo;
    }

    if(setting->mIsFinalData)
    {
        MY_LOGD("last data, update processorReq#: new:%d, old:%d", setting->mRequestNo, miRequestNo);
        miRequestNo = setting->mRequestNo;

        if(miRequestNo > gMaxRequestNo)
        {
            MINT32 oldRequestNo = miRequestNo;
            miRequestNo = gStartRequestNo;
            MY_LOGD("reset processorReq#: old:%d, new:%d", oldRequestNo, miRequestNo);
        }
    }
    ret = OK;

lbExit:
    if((ret != OK) && (builderPtr != nullptr))
    {
        Mutex::Autolock _l(mPostProcRequestTableLock);

        auto it = mPostProcRequestTable.find(timestamp);
        if (it != mPostProcRequestTable.end())
        {
            ProcessingBuilder& processingBuilder = it->second;

            const MINT32 builderProcessingReqNum = std::get<PROCESSINGREQNUM>(processingBuilder);
            sp<IPostProcRequestBuilder> builderPtr = std::get<BUILDERPTR>(processingBuilder);
            mPostProcRequestTable.erase(it);

            MY_LOGD("remove builder: builderProcessingReqNum:%d, key(timestamp):%lld, value(addr):%p, tableSize:%zu",
                builderProcessingReqNum, timestamp, builderPtr.get(), mPostProcRequestTable.size());
        }
        else
        {
            MY_LOGE("builder is not existing, key(timestamp):%lld", timestamp);
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyPostProcessor::
onPostProcDone()
{
    status_t ret = UNKNOWN_ERROR;
    {
        Mutex::Autolock _l(mPostProcRequestTableLock);

        if(mPostProcRequestTable.size() > 0)
        {
            // Node: no key to identify witch builder had done his work, and we just pop the first one (FIFO)
            auto it = mPostProcRequestTable.begin();

            const MINT64 key = it->first;
            ProcessingBuilder& processingBuilder = it->second;

            const MINT32 builderProcessingReqNum = std::get<PROCESSINGREQNUM>(processingBuilder);
            sp<IPostProcRequestBuilder> builderPtr = std::get<BUILDERPTR>(processingBuilder);
            mPostProcRequestTable.erase(it);

            MY_LOGD("remove builder: builderProcessingReqNum:%d, key(timestamp):%lld, value(addr):%p, tableSize:%zu",
                builderProcessingReqNum, key, builderPtr.get(), mPostProcRequestTable.size());
        }
        else
        {
            MY_LOGE("builder is not existing, tableSize:%zu", mPostProcRequestTable.size());
        }

        for(auto item : mPostProcRequestTable)
        {
            const MINT64 key = item.first;
            ProcessingBuilder& processingBuilder = item.second;
            const MINT32 builderProcessingReqNum = std::get<PROCESSINGREQNUM>(processingBuilder);
            sp<IPostProcRequestBuilder>& builderPtr = std::get<BUILDERPTR>(processingBuilder);
            MY_LOGD("item in postProcRequestTable, key(timestamp):%lld, addr:%p, reqNum:%d",
                key, builderPtr.get(), builderProcessingReqNum);
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyPostProcessor::
onEvent(
    MINT32 capNo,
    PostProcCB callbackType,
    MUINT32 streamId,
    MBOOL bError,
    void* params1,
    void* params2)
{
    auto cbTypeToString = [&callbackType]()
    {
        switch(callbackType)
        {
            case PostProcCB::PROCESS_DONE:
                return "PROCESS_DONE";
            default:
                return "";
        }
        return "";
    };
    MY_LOGD("cap(%d) cb[%s] streamId(%u) bError(%d)", capNo, cbTypeToString(), streamId, bError);
    switch(callbackType)
    {
            case PostProcCB::PROCESS_DONE:
                onPostProcDone();
                break;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyPostProcessor::
buildPipeline()
{
    FUNCTION_SCOPE;

    if (mpPipeline != NULL)
    {
        MY_LOGE("Pipeline already be created");
        return OK;
    }
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder(OPEN_ID,
                                            (char *)"ThirdPartyPostPipeline",
                                            NSCam::v1::NSLegacyPipeline::StereoLegacyPipelineMode_T::STPipelineMode_PP);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return UNKNOWN_ERROR;
    }
    // create MetaStreamInfo
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(gFrameDataTableSet[0].metadata);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return UNKNOWN_ERROR;
    }
    // create ImageStreamInfo
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(gFrameDataTableSet[0].image);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return UNKNOWN_ERROR;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mSensorId,
                                            (MUINT32)mSensorIdMain2,
                                            gFrameDataTableSet[0].node
                                        };
    // create Node & Edge
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("postprocessor", &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return UNKNOWN_ERROR;
    }
    // update tbl
    {
        auto setProvider = [this, &pImageStreamManager](MINT32 streamId, char const* name)
        {
            sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);
            if(pImageInfo != NULL)
            {
                MY_LOGD("Prepare BufferProvider for %s, streamName: %s, maxBufferNum: %zu, minInitBufferNum: %zu",
                    name, pImageInfo->getStreamName(), pImageInfo->getMaxBufNum(), pImageInfo->getMinInitBufNum());

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pImageInfo);
                pPool->allocateBuffer(
                        pImageInfo->getStreamName(),
                        pImageInfo->getMaxBufNum(),
                        pImageInfo->getMinInitBufNum());
                pFactory->setImageStreamInfo(pImageInfo);
                pFactory->setUsersPool(pPool);

                auto it = mInputBufferPoolTable.find(streamId);
                if (it != mInputBufferPoolTable.end())
                {
                    IPostProcRequestBuilder::CallbackBufferPoolItem& item = it->second;
                    std::get<CALLBACKBUFFERPOOL>(item) = pPool;
                    MY_LOGD("add buffer pool into table: streamId:%d, poolAdr:%p", streamId, pPool.get());
                }

                android::sp<StreamBufferProvider> pProvider = pFactory->create(OPEN_ID, MTRUE);

                pImageStreamManager->updateBufProvider(
                                                pImageInfo->getStreamId(),
                                                pProvider,
                                                Vector<StreamId_T>());
            }
            else {
                MY_LOGD("%s did not config in ImageStreamMnger!!", name);
            }
        };
        // TP input
        for(auto item : mInputBufferPoolTable)
        {
            const MINT32 streamId = item.first;
            const char* poolName = std::get<POOLNAME>(item.second);
            setProvider(streamId, poolName);
        }
        // TP output
        setProvider(eSTREAMID_IMAGE_PIPE_YUV_JPEG,          "yuv jpeg");
        setProvider(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,     "yuv thumbnail");
        setProvider(eSTREAMID_IMAGE_PIPE_TP_OUT00,          "yuv tp out");
        setProvider(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV,          "depth");
        // TP jpeg
        setProvider(eSTREAMID_IMAGE_JPEG,                   "jpeg");
        // TP jpeg00
        setProvider(eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00,      "jpeg tp out00");
    }
    //
    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_CONTROL,
                eSTREAMID_META_APP_CONTROL);

    // Tell StereoLegacyPipelineBuilder we need to substitute default requestBuilder
    {
        sp<PipelineContext> pipelineContext = nullptr;
        sp<NSCam::StereoBasicParameters> userParams = new NSCam::StereoBasicParameters();
        pNodeConfigDataManager->configNode(
                                    pMetaStreamManager,
                                    pImageStreamManager,
                                    userParams,
                                    pipelineContext);

        Vector< sp<NodeConfigDataManager> > vRequests;
        for(MUINT32 i = 1 ; i< gFrameDataTableSet.size() ; i++)
        {
            vRequests.push_back(buildNodeConfigDataManager(gFrameDataTableSet[i]));
        }

        pBuilder->setRequestTypes(pNodeConfigDataManager, vRequests);
    }

    mpImageStreamManager = pImageStreamManager;

    mpPipeline = pBuilder->create(pMetaStreamManager, pImageStreamManager, pNodeConfigDataManager);
    if(mpPipeline == NULL){
        MY_LOGE("pBuilder->create PostProcPipeline failed");
        return UNKNOWN_ERROR;
    }
    return OK;
}
/******************************************************************************
 * create p2 prv pipeline
 ******************************************************************************/
sp<NodeConfigDataManager>
ThirdPartyPostProcessor::
buildNodeConfigDataManager(
    FrameDataTableSet& data
)
{
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(data.metadata);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(data.image);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mSensorId,
                                            (MUINT32)mSensorIdMain2,
                                            data.node
                                        };
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create(data.name.c_str(), &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }
    sp<NSCam::StereoBasicParameters> userParams = new NSCam::StereoBasicParameters();
    //userParams->set(NSCam::StereoBasicParameters::KEY_STEREO_CAM_MODULE_TYPE, mCurrentSensorModuleType);
    //userParams->set(NSCam::StereoBasicParameters::KEY_STEREO_CAM_FEATURE_TYPE, mCurrentStereoMode);
    sp<PipelineContext> pipelineContext = nullptr;
    pNodeConfigDataManager->configNode(
                                pMetaStreamManager,
                                pImageStreamManager,
                                userParams,
                                pipelineContext);
    pMetaStreamManager = nullptr;
    pImageStreamManager = nullptr;
    return pNodeConfigDataManager;
}