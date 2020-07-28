/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#define LOG_TAG "MtkCam/LPipelineMFC"

#include "LegacyPipelineMFC.h"
#include "MyUtils.h"

#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/HDRNode.h>
#include <mtkcam/pipeline/hwnode/MfllNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#define FUNCTION_LOG_START      CAM_LOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        CAM_LOGD("[%s] - X. ret: %d.", __FUNCTION__, ret)
#define FUNCTION_LOG_END_MUM    CAM_LOGD("[%s] - X.", __FUNCTION__)

/**
 *  For BSP, MFNR is not built-in feature
 */
#if MTKCAM_HAVE_MFB_SUPPORT
typedef MfllNode    MfllNode_T;
#else
typedef MFCNode     MfllNode_T;
#endif

// ---------------------------------------------------------------------------

using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineContext;

using namespace NS3Av3;

// ---------------------------------------------------------------------------

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

// ---------------------------------------------------------------------------

struct LegacyPipelineUtil
{
    static inline MVOID addStream(
            LegacyPipelineMFC * const lp, const sp<IMetaStreamInfo>& info)
    {
        if (info.get()) lp->setMetaStreamInfo(info);
    }

    static inline MVOID addStream(
            LegacyPipelineMFC * const lp, const sp<IImageStreamInfo>& info)
    {
        if (info.get()) lp->setImageStreamInfo(info);
    }
};

static MVOID dumpConfiguration(const sp<Configuration>& config)
{
    if (config == NULL) return;

#define DUMP_STREAMINFO(name, msg)               \
    do {                                         \
        if (name.get())                          \
        msg.appendFormat("\n(%#" PRIx64 ") %s", \
                name->getStreamId(),             \
                name->getStreamName());          \
    } while(0)

    String8 msg;
    msg.append("\ninput");
    DUMP_STREAMINFO(config->controlMeta_App, msg);
    DUMP_STREAMINFO(config->controlMeta_Hal, msg);

    msg.append("\noutput (metadata)");
    DUMP_STREAMINFO(config->resultMeta_P1_App, msg);
    DUMP_STREAMINFO(config->resultMeta_P1_Hal, msg);
    DUMP_STREAMINFO(config->resultMeta_MFC_App, msg);
    DUMP_STREAMINFO(config->resultMeta_MFC_Hal, msg);
    DUMP_STREAMINFO(config->resultMeta_Jpeg_App, msg);

    msg.append("\noutput (image)");
    DUMP_STREAMINFO(config->imageInfo_imgoRaw, msg);
    DUMP_STREAMINFO(config->imageInfo_rrzoRaw, msg);
    DUMP_STREAMINFO(config->imageInfo_yuv00, msg);
    DUMP_STREAMINFO(config->imageInfo_yuvJpeg, msg);
    DUMP_STREAMINFO(config->imageInfo_yuvThumbnail, msg);
    DUMP_STREAMINFO(config->imageInfo_jpeg, msg);

    MY_LOGD("%s", msg.string());
#undef DUMP_STREAMINFO
}

static sp<HalMetaStreamBuffer> createMetaStreamBuffer(
        android::sp<IMetaStreamInfo>& streamInfo,
        const IMetadata& metadata,
        const MBOOL repeating __attribute__((unused)))
{
    HalMetaStreamBuffer* pStreamBuffer =
        HalMetaStreamBuffer::Allocator(streamInfo.get())(metadata);

    return pStreamBuffer;
}

// ---------------------------------------------------------------------------

LegacyPipelineMFC::LegacyPipelineMFC()
{
    mTimestamp = TimeTool::getReadableTime();
}

LegacyPipelineMFC::~LegacyPipelineMFC()
{
}

MVOID LegacyPipelineMFC::setRequestBuilder(
        sp<RequestBuilder> const /*pRequestBuilder*/)
{
    MY_LOGE("Unsupport setRequestBuilder without keyedvector input");
}

MERROR LegacyPipelineMFC::waitUntilDrainedAndFlush()
{
    CAM_TRACE_CALL();

    if (mPipelineContext == NULL)
    {
        MY_LOGW("get pipeline context failed");
        return UNKNOWN_ERROR;
    }

    MERROR err = OK;

    // P1
    if (mConfiguration->configFlag & eNodeConfigP1)
        err |= drainAndFlushNode<P1Node>(eNODEID_P1Node);

    // HDR
    if (mConfiguration->configFlag & eNodeConfigHdr)
        err |= drainAndFlushNode<HDRNode>(eNODEID_HdrNode);

    // Mfll
    if (mConfiguration->configFlag & eNodeConfigMfll)
        err |= drainAndFlushNode<MfllNode_T>(eNODEID_MfllNode);

    // Jpeg
    if (mConfiguration->configFlag & eNodeConfigJpeg)
        err |= drainAndFlushNode<JpegNode>(eNODEID_JpegNode);

    return (err == OK) ? OK : INVALID_OPERATION;
}

MERROR LegacyPipelineMFC::submitSetting(
        MINT32    const requestNo,
        IMetadata& appMeta,
        IMetadata& halMeta,
        ResultSet* /*pResultSet*/)
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    if ((mvRequestBuilder.isEmpty()) || (mPipelineContext == NULL))
    {
        MY_LOGE_IF(mvRequestBuilder.isEmpty(), "request builder is not set yet");
        MY_LOGE_IF(mPipelineContext == NULL, "pipeline contest is not set yet");
        return NO_INIT;
    }

    // get timstamp (Epoch time) as an unique pipeline key
    if (!trySetMetadata<MINT32>(halMeta, MTK_PIPELINE_UNIQUE_KEY, mTimestamp))
        MY_LOGE("set unique key failed");

    // build and get pipeline frame
    sp<IPipelineFrame> frame = createRequest(requestNo, appMeta, halMeta);

    // queue pipeline frame into pipeline context
    if (frame.get())
    {
        if (OK != mPipelineContext->queue(frame))
        {
            MY_LOGE("queue pipeline frame failed");
        }
    }
    else
    {
        MY_LOGE("build request(%d) failed", requestNo);
    }

    // clear frame
    frame.clear();

    FUNCTION_LOG_END_MUM;

    return OK;
}

MERROR LegacyPipelineMFC::submitRequest(
        MINT32    const requestNo __attribute__((__unused__)),
        IMetadata& appMeta __attribute__((__unused__)),
        IMetadata& halMeta __attribute__((__unused__)),
        Vector<BufferSet> vDstStreams __attribute__((__unused__)),
        ResultSet* pResultSet __attribute__((__unused__)))
{
    CAM_TRACE_CALL();

    MY_LOGE("implementation needed");

    // TODO: implementation needed
    return INVALID_OPERATION;
}

MVOID LegacyPipelineMFC::onLastStrongRef(
        const void* id __attribute__((__unused__)))
{
    CAM_TRACE_CALL();

    // flush all currently in-flight captures and all buffers in the pipeline
    if (mPipelineContext.get())
    {
        mPipelineContext->flush();
        mPipelineContext->waitUntilDrained();
        mPipelineContext.clear();
    }
    if (mResultProcessor.get())
        mResultProcessor->flush();
    // remove reference
    mConfiguration.clear();

    // empty vectors
    mvRequestBuilder.clear();

    const size_t count = mvStreamBufferProvider.size();
    for (size_t i = 0; i < count; i++)
    {
        if (mvStreamBufferProvider.editValueAt(i).get())
            mvStreamBufferProvider.editValueAt(i)->flush();
    }
    mvStreamBufferProvider.clear();

    mvMetaStreamInfo.clear();
    mvImageStreamInfo.clear();
}

MVOID LegacyPipelineMFC::setConfiguration(const sp<Configuration>& config)
{
    if (config == NULL)
    {
        MY_LOGE("configuration is null");
        return;
    }

    mConfiguration = config;

    // input
    LegacyPipelineUtil::addStream(this, config->controlMeta_App);
    LegacyPipelineUtil::addStream(this, config->controlMeta_Hal);

    // output (metadata)
    LegacyPipelineUtil::addStream(this, config->resultMeta_P1_App);
    LegacyPipelineUtil::addStream(this, config->resultMeta_P1_App);
    LegacyPipelineUtil::addStream(this, config->resultMeta_P1_Hal);
    LegacyPipelineUtil::addStream(this, config->resultMeta_MFC_App);
    LegacyPipelineUtil::addStream(this, config->resultMeta_MFC_Hal);
    LegacyPipelineUtil::addStream(this, config->resultMeta_Jpeg_App);

    // output (image)
    LegacyPipelineUtil::addStream(this, config->imageInfo_imgoRaw);
    LegacyPipelineUtil::addStream(this, config->imageInfo_rrzoRaw);
    LegacyPipelineUtil::addStream(this, config->imageInfo_yuv00);
    LegacyPipelineUtil::addStream(this, config->imageInfo_yuvJpeg);
    LegacyPipelineUtil::addStream(this, config->imageInfo_yuvThumbnail);
    LegacyPipelineUtil::addStream(this, config->imageInfo_jpeg);
}

MVOID LegacyPipelineMFC::setImageStreamInfo(
    const sp<IImageStreamInfo>& pImageStreamInfo)
{
    if (pImageStreamInfo == NULL)
    {
        MY_LOGE("ImageStreamInfo is NULL");
        return;
    }

    mvImageStreamInfo.add(pImageStreamInfo->getStreamId(), pImageStreamInfo);
}

MVOID LegacyPipelineMFC::setMetaStreamInfo(
    const sp<IMetaStreamInfo>& pMetaStreamInfo)
{
    if (pMetaStreamInfo ==  NULL)
    {
        MY_LOGE("MetaStreamInfo is NULL");
        return;
    }

    mvMetaStreamInfo.add(pMetaStreamInfo->getStreamId(), pMetaStreamInfo);
}

MVOID LegacyPipelineMFC::setStreamBufferProvider(
    const sp<StreamBufferProvider>& pStreamBufferProvider)
{
    if (pStreamBufferProvider == NULL)
    {
        MY_LOGE("StreamBufferProvider is NULL");
        return;
    }

    mvStreamBufferProvider.add(
            pStreamBufferProvider->queryImageStreamInfo()->getStreamId(),
            pStreamBufferProvider);
}

sp<IPipelineFrame> LegacyPipelineMFC::createRequest(
        const MINT32 requestNo,
        const IMetadata& appMeta,
        const IMetadata& halMeta)
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    if (requestNo < 0)
    {
        MY_LOGE("invalid request number(%d)", requestNo);
        return NULL;
    }

    // NOTE: the request number is used to judge whether to
    // create a main request (0) or a subrequest (> 0)
    MBOOL isMainRequest = (requestNo == 0) ? MTRUE : MFALSE;

    MY_LOGD("create %srequest", isMainRequest ? "main " : "sub");

    // build request and return the corresponding pipeline frame
    sp<RequestBuilder> requestBuilder = mvRequestBuilder[isMainRequest ? 0 : 1];

    // create meta stream buffers and set them into request builder
    {
        sp<IMetaStreamBuffer> appMetaStreamBuffer;
        sp<HalMetaStreamBuffer> halMetaStreamBuffer;

        appMetaStreamBuffer =
            createMetaStreamBuffer(mConfiguration->controlMeta_App, appMeta, false);

        halMetaStreamBuffer =
            createMetaStreamBuffer(mConfiguration->controlMeta_Hal, halMeta, false);

        // set meta stream buffer
        requestBuilder->setMetaStreamBuffer(
                mConfiguration->controlMeta_App->getStreamId(),
                appMetaStreamBuffer);

        requestBuilder->setMetaStreamBuffer(
                mConfiguration->controlMeta_Hal->getStreamId(),
                halMetaStreamBuffer);
    }

    dumpConfiguration(mConfiguration);

    FUNCTION_LOG_END_MUM;

    return requestBuilder->build(requestNo, mPipelineContext);
}

template <typename TNode>
MERROR LegacyPipelineMFC::drainAndFlushNode(const NodeId_T nodeId)
{
    CAM_TRACE_CALL();

    sp< NodeActor< TNode > > nodeActor;

    // query node actor
    MERROR err = mPipelineContext->queryNodeActor(nodeId, nodeActor);
    if ((err != OK) || (nodeActor == NULL))
    {
        MY_LOGW("query node(%#" PRIxPTR ") failed(%s)", nodeId, strerror(-err));
        return NAME_NOT_FOUND;
    }

    // drain node
    err = mPipelineContext->waitUntilNodeDrained(nodeId);
    if (err != OK)
    {
        MY_LOGW("drain node(%#" PRIxPTR ") failed(%s)", nodeId, strerror(-err));
        return err;
    }

    // flush pipeline node
    IPipelineNode* node = nodeActor->getNode();
    if (node == NULL)
    {
        MY_LOGW("get node(%#" PRIxPTR ") failed(%s)", nodeId, strerror(-err));
        return UNKNOWN_ERROR;
    }

    if (node->flush() != OK)
    {
        MY_LOGW("flush node(%#" PRIxPTR ") failed(%s)", nodeId, strerror(-err));
        return err;
    }

    return OK;
}
