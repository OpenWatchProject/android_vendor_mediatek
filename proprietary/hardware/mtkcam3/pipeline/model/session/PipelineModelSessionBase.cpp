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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "mtkcam-PipelineModelSession"
//
#include "PipelineModelSessionBase.h"
//
#include <impl/AppRequestParser.h>
//
#include "MyUtils.h"
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DEFAULT_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelSessionBase::
PipelineModelSessionBase(
    std::string const&& sessionName,
    CtorParams const& rCtorParams
)
    : mSessionName(sessionName)
    , mStaticInfo(rCtorParams.staticInfo)
    , mDebugInfo(rCtorParams.debugInfo)
    , mPipelineModelCallback(rCtorParams.pPipelineModelCallback)
    , mPipelineSettingPolicy(rCtorParams.pPipelineSettingPolicy)
    , mpSessionLifeHolder(ISessionLifeHolder::create(
                    mStaticInfo.pUserConfiguration->pParsedAppConfiguration->configTimestamp))
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
submitRequest(
    std::vector<std::shared_ptr<UserRequestParams>>const& requests,
    uint32_t& numRequestProcessed
) -> int
{
    numRequestProcessed = 0;

    std::vector<std::shared_ptr<ParsedAppRequest>> reqs;
    reqs.reserve(requests.size());

    //Convert: UserRequestParams -> ParsedAppRequest
    for (size_t i = 0; i < requests.size(); i++) {
        auto& request = requests[i];
        {
            // The user has to set the App image buffer provider at configuration stage
            // while submitting a request without enough output App image stream buffers.
            bool isBadRequest =
                        (mStaticInfo.pUserConfiguration2->pImageStreamBufferProvider == nullptr)
                     && (request->vOImageStreams.size() > request->vOImageBuffers.size())
                        ;
            if (CC_UNLIKELY( isBadRequest )) {
                MY_LOGE("[requestNo:%u] No provider configured && #OImageStreams:%zu > #OImageBuffers:%zu",
                    request->requestNo, request->vOImageStreams.size(), request->vOImageBuffers.size());
                return -EINVAL;
            }
        }

        auto r = std::make_shared<ParsedAppRequest>();
        RETURN_ERROR_IF_NULLPTR( r, -ENODEV, "i:%zu Fail to make_shared<ParsedAppRequest>", i );

        RETURN_ERROR_IF_NOT_OK(
            parseAppRequest(ParseAppRequest{
                .to = r.get(), .from = request.get(),
                .pUserConfiguration = mStaticInfo.pUserConfiguration.get(),}),
            "parseAppRequest fail on requestNo:%u - %zu/%zu requests parsed sucessfully",
            request->requestNo, i, requests.size() );

        // Dump the metadata request if it's not repeating
        if (CC_UNLIKELY( ! r->pParsedAppMetaControl->repeating )) {
            MY_LOGI("<%s> requestNo:%d %s", getSessionName().c_str(), r->requestNo, toString(*r->pParsedAppMetaControl).c_str());
        }

        reqs.emplace_back(r);
    }

    //Submit ParsedAppRequest one by one
    for (size_t i = 0; i < reqs.size(); i++, numRequestProcessed++) {
        RETURN_ERROR_IF_NOT_OK( submitOneRequest(reqs[i]),
            "submitOneRequest fail on requestNo:%u - %u/%zu requests submitted sucessfully",
            reqs[i]->requestNo, numRequestProcessed, reqs.size() );
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
beginFlush() -> int
{
    auto pPipelineContext = getCurrentPipelineContext();
    RETURN_ERROR_IF_NULLPTR( pPipelineContext, OK, "No current pipeline context" );
    RETURN_ERROR_IF_NOT_OK( pPipelineContext->beginFlush(), "PipelineContext::beginFlush()" );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
endFlush() -> void
{
    auto pPipelineContext = getCurrentPipelineContext();
    if (CC_LIKELY( pPipelineContext != nullptr )) {
        pPipelineContext->endFlush();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    auto pPipelineContext = getCurrentPipelineContext();
    printer.printFormatLine("\n== Current PipelineContext (%p) ==", pPipelineContext.get());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
determineTimestampSOF(
    StreamId_T const streamId,
    android::Vector<android::sp<IMetaStreamBuffer>>const& vMetaStreamBuffer
) -> int64_t
{
    int64_t timestampSOF = 0;
    for (size_t i = 0; i < vMetaStreamBuffer.size(); i++) {
        auto const& pStreamBuffer = vMetaStreamBuffer[i];
        if  ( pStreamBuffer != nullptr
           && pStreamBuffer->getStreamInfo() != nullptr
           && streamId == pStreamBuffer->getStreamInfo()->getStreamId() ) {
            auto pMetadata = pStreamBuffer->tryReadLock(LOG_TAG);
            IMetadata::getEntry(pMetadata, MTK_P1NODE_FRAME_START_TIMESTAMP, timestampSOF);
            pStreamBuffer->unlock(LOG_TAG, pMetadata);
        }
    }
    return timestampSOF;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
updateFrameTimestamp(
    MUINT32 const requestNo,
    MINTPTR const userId,
    Result const& result,
    int64_t timestampStartOfFrame
) -> void
{
    #if 0
    MY_LOGD_IF( 1, "requestNo:%d timestamp(SOF):%" PRId64 " user:%#" PRIxPTR " bFrameEnd:%d App(size:%zu; left%zu) Hal(size:%zu; left%zu)",
                requestNo, timestampStartOfFrame, userId, result.bFrameEnd,
                result.vAppOutMeta.size(), result.nAppOutMetaLeft,
                result.vHalOutMeta.size(), result.nHalOutMetaLeft );
    #endif

    if (result.bFrameEnd) {
        return;
    }

    sp<IPipelineModelCallback> pCallback;
    pCallback = mPipelineModelCallback.promote();
    if ( CC_UNLIKELY(! pCallback.get()) ) {
        MY_LOGE("Have not set callback to session");
        return;
    }

    {
        UserOnFrameUpdated params;
        params.requestNo = requestNo;
        params.userId = userId;
        params.nOutMetaLeft = result.nAppOutMetaLeft;
        params.timestampStartOfFrame = timestampStartOfFrame;
        for ( size_t i=0; i < result.vAppOutMeta.size(); ++i ) {
            params.vOutMeta.push_back(result.vAppOutMeta[i]);
        }

        for ( size_t i=0; i < result.vPhysicalOutMeta.size(); ++i) {
            params.vPhysicalOutMeta.add(
                        result.vPhysicalOutMeta.keyAt(i),
                        result.vPhysicalOutMeta.valueAt(i));
        }

        if(result.nAppOutMetaLeft == 0 &&
           result.nPhysicalID >= 0)
        {
            params.activePhysicalId = result.nPhysicalID;
        }

        pCallback->onFrameUpdated(params);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
onImageBufferReleased(ImageBufferReleased&& arg) -> void
{
    MY_LOGF_IF(arg.frame == nullptr, "Bad IPipelineFrame");
    MY_LOGF_IF(arg.frame->getGroupFrameType()!=NSPipelineContext::GroupFrameType::MAIN,
        "[requestNo:%u frameNo:%u] Not main frame", arg.frame->getRequestNo(), arg.frame->getFrameNo());

    {
        auto pCallback = mPipelineModelCallback.promote();
        if (CC_UNLIKELY( pCallback == nullptr )) {
            MY_LOGE("Fail to promote IPipelineModelCallback");
            return;
        }

        {
            std::vector<UserOnImageBufferReleased::Result> results;
            results.reserve(arg.buffers.size());
            for (auto const& b : arg.buffers) {
                results.push_back({
                        .streamId = b->getStreamInfo()->getStreamId(),
                        .status = b->getStatus(),
                    });
            }

            CAM_TRACE_CALL();
            pCallback->onImageBufferReleased(
                   UserOnImageBufferReleased{
                    .requestNo = arg.frame->getRequestNo(),
                    .results = std::move(results),
                });
        }
    }

}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelSessionBase::
onPipelineFrameDestroy(PipelineFrameDestroy&& arg) -> void
{

    switch ( arg.groupFrameType )
    {
    case NSPipelineContext::GroupFrameType::MAIN:{

        auto pCallback = mPipelineModelCallback.promote();
        if (CC_UNLIKELY( pCallback == nullptr )) {
            MY_LOGE("Fail to promote IPipelineModelCallback");
            return;
        }

        CAM_TRACE_CALL();
        pCallback->onRequestCompleted(
               UserOnRequestCompleted{
                .requestNo = arg.requestNo,
            });

        }break;
    default:
        break;
    }

}