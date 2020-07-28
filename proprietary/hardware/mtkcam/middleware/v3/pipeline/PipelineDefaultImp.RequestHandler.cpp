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

#define LOG_TAG "MtkCam/HwPipeline/RequestHandler"
//
#include "PipelineDefaultImp.h"
#include "PipelineUtility.h"
//
// #include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineContext;

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
parse(AppRequest* pRequest, parsedAppRequest &ParsedRequest)
{
    struct categorize_img_stream
    {
        typedef KeyedVector< StreamId_T, sp<IImageStreamBuffer> >       IImageSBMapT;
        typedef KeyedVector< StreamId_T, sp<IImageStreamInfo> >         IImageInfoMapT;
        MERROR      operator()(
                IImageSBMapT& map,
                IImageInfoMapT* pMapRaw,
                IImageInfoMapT* pMapOpaque,
                IImageInfoMapT* pMapYuv,
                IImageInfoMapT* pMapJpeg
                )
        {
            for(size_t i = 0; i < map.size(); i++ )
            {
                sp<IImageStreamBuffer> buf = map.valueAt(i);
                if ( IImageStreamInfo const* pStreamInfo = buf->getStreamInfo() )
                {
                    IImageInfoMapT* pTargetMap = NULL;
                    switch( pStreamInfo->getImgFormat() )
                    {
                        //case eImgFmt_BAYER10: //TODO: not supported yet
                        //case eImgFmt_BAYER12:
                        //case eImgFmt_BAYER14:
                        case eImgFmt_RAW16:
                            pTargetMap = pMapRaw;
                            break;
                        case eImgFmt_CAMERA_OPAQUE:
                            pTargetMap = pMapOpaque;
                            break;
                            //
                        case eImgFmt_BLOB:
                            pTargetMap = pMapJpeg;
                            break;
                            //
                        case eImgFmt_YV12:
                        case eImgFmt_NV21:
                        case eImgFmt_YUY2:
                        case eImgFmt_Y8:
                        case eImgFmt_Y16:
                            pTargetMap = pMapYuv;
                            break;
                            //
                        default:
                            MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                            break;
                    }
                    if( pTargetMap == NULL ) {
                        MY_LOGE("cannot get target map");
                        return UNKNOWN_ERROR;
                    }
                    //
                    pTargetMap->add(
                            pStreamInfo->getStreamId(),
                            const_cast<IImageStreamInfo*>(pStreamInfo)
                            );
                }
            }
            return OK;
        }
    };
    //
    CHECK_ERROR(
            categorize_img_stream() (
                pRequest->vIImageBuffers,
                &ParsedRequest.vIImageInfos_Raw, &ParsedRequest.vIImageInfos_Opaque,
                &ParsedRequest.vIImageInfos_Yuv, NULL
                )
            );
    CHECK_ERROR(
            categorize_img_stream() (
                pRequest->vOImageBuffers,
                &ParsedRequest.vOImageInfos_Raw, &ParsedRequest.vOImageInfos_Opaque,
                &ParsedRequest.vOImageInfos_Yuv, &ParsedRequest.vOImageInfos_Jpeg
                )
            );
    // TBD
    ParsedRequest.pRequest = pRequest;
    ParsedRequest.requestNo = pRequest->requestNo;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
submitRequestLocked(AppRequest& request)
{
    parsedAppRequest aParsedRequest;
    CHECK_ERROR( parse(&request, aParsedRequest) );
    //
    MY_LOGD_IF( mCommonInfo->mLogLevel>=2, "requestNo(%d) Output:raw(%zu),opaque(%zu),yuv(%zu),jpeg(%zu); Input:raw(%zu),opaque(%zu),yuv(%zu)",
            request.requestNo,
            aParsedRequest.vOImageInfos_Raw.size(), aParsedRequest.vOImageInfos_Opaque.size(),
            aParsedRequest.vOImageInfos_Yuv.size(), aParsedRequest.vOImageInfos_Jpeg.size(),
            aParsedRequest.vIImageInfos_Raw.size(), aParsedRequest.vIImageInfos_Opaque.size(),
            aParsedRequest.vIImageInfos_Yuv.size() );
    //
    // RWLock::AutoRLock _l(mRWLock);
    //
    std::vector<evaluateRequestResult>       vevaluateResult;
    //CHECK_ERROR( mConfigHandler->reconfigPipelineLocked(aParsedRequest) );
    //
    CHECK_ERROR( evaluateRequestLocked(aParsedRequest, vevaluateResult) );
    //
    CHECK_ERROR( refineRequestMetaStreamBuffersLocked(aParsedRequest, vevaluateResult[0]) );
    //
    //
    // main frame
    {
        sp<IPipelineFrame> pFrame = buildPipelineFrameLocked(request.requestNo, vevaluateResult[0]);
        if( ! pFrame.get() )
            return UNKNOWN_ERROR;

        CHECK_ERROR( mCommonInfo->mpPipelineContext->queue(pFrame) );
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
#define FUNC_ASSERT(exp, msg) \
    do{ if(!(exp)) { MY_LOGE("%s", msg); return INVALID_OPERATION; } } while(0)

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
evaluateRequestLocked(parsedAppRequest const& request, std::vector<evaluateRequestResult> &vresult)
{
    CAM_TRACE_NAME(__FUNCTION__);

    //
    FUNC_ASSERT( request.vIImageInfos_Raw.size() == 0, "[TODO] not supported yet!" );
    //FUNC_ASSERT( request.vIImageInfos_Yuv.size() == 0, "[TODO] not supported yet!" );
    FUNC_ASSERT( request.vOImageInfos_Raw.size() <= 1, "[TODO] not supported yet!" );
    FUNC_ASSERT( request.vOImageInfos_Jpeg.size() <= 1, "[TODO] not supported yet!" );
    //
    //result.isTSflow = mParams->mPipelineConfigParams.mbUseP2CapNode && isTimeSharingForJpegSource(request);
    //
    //
    CHECK_ERROR( mCommonInfo->mCameraSetting->evluateRequest(request, mParams, vresult) );
    CHECK_ERROR( evaluateRequestLocked_updateStreamBuffers(request, vresult[0]) );
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
evaluateRequestLocked_updateStreamBuffers(
    parsedAppRequest const& request,
    evaluateRequestResult& result
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    // app image
    {
        result.vAppImageBuffers.setCapacity(
                request.pRequest->vIImageBuffers.size() +
                request.pRequest->vOImageBuffers.size()
                );
        for( size_t i = 0; i < request.pRequest->vIImageBuffers.size(); i++ )
            result.vAppImageBuffers.add(
                    request.pRequest->vIImageBuffers.keyAt(i),
                    request.pRequest->vIImageBuffers.valueAt(i)
                    );
        for( size_t i = 0; i < request.pRequest->vOImageBuffers.size(); i++ )
            result.vAppImageBuffers.add(
                    request.pRequest->vOImageBuffers.keyAt(i),
                    request.pRequest->vOImageBuffers.valueAt(i)
                    );
    }
    // hal image
    {
        result.vHalImageBuffers.clear();
    }
    // app meta
    {
        result.vAppMetaBuffers.setCapacity(request.pRequest->vIMetaBuffers.size());
        for( size_t i = 0; i < request.pRequest->vIMetaBuffers.size(); i++ )
        {
            result.vAppMetaBuffers.add(
                    request.pRequest->vIMetaBuffers.keyAt(i),
                    request.pRequest->vIMetaBuffers.valueAt(i)
                    );
            if ( mParams->mStreamSet.mpAppMeta_Control.get() &&
                     mParams->mStreamSet.mpAppMeta_Control->getStreamId() ==
                     result.vAppMetaBuffers[i]->getStreamInfo()->getStreamId() )
            {
                result.isRepeating = result.vAppMetaBuffers[i]->isRepeating();
            }
        }
    }
    // hal meta
    {
        result.vHalMetaBuffers.setCapacity(1);
        sp<HalMetaStreamBuffer> pBuffer =
            HalMetaStreamBufferAllocatorT(mParams->mStreamSet.mpHalMeta_Control[0].get())();
        result.vHalMetaBuffers.add(mParams->mStreamSet.mpHalMeta_Control[0]->getStreamId(), pBuffer);
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
evaluateRequestLocked_updateHalBuffers(
    parsedAppRequest const& request,
    evaluateRequestResult& result
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    // hal image
    {
        //result.vHalImageBuffers.clear();
    }
    //
    return OK;
}


#undef FUNC_ASSERT

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::RequestHandler::
refineRequestMetaStreamBuffersLocked(parsedAppRequest const& request, evaluateRequestResult& result)
{
    CAM_TRACE_NAME(__FUNCTION__);
    //
    if( ! mParams->mStreamSet.mpHalMeta_Control[0].get() ) {
        MY_LOGE("should config hal control meta");
        return UNKNOWN_ERROR;
    }
    //
    {
        sp<IMetaStreamBuffer> pBuf = result.vHalMetaBuffers.valueFor(mParams->mStreamSet.mpHalMeta_Control[0]->getStreamId());
        if( pBuf.get() )
        {
            IMetadata* pMetadata = pBuf->tryWriteLock(LOG_TAG);

            // update sensor size
            {
                IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
                entry.push_back(mParams->mHwParams[0]->mSensorSize, Type2Type< MSize >());
                pMetadata->update(entry.tag(), entry);
            }

            if ( (mParams->mStreamSet.mpAppImage_Jpeg.get() &&
                    0 <= result.vAppImageBuffers.indexOfKey(mParams->mStreamSet.mpAppImage_Jpeg->getStreamId())) ||
                 (mParams->mStreamSet.mpAppImage_Opaque_Out.get() &&
                    0 <= result.vAppImageBuffers.indexOfKey(mParams->mStreamSet.mpAppImage_Opaque_Out->getStreamId())) )
            {
                MY_LOGD_IF(1, "set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
                IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                entry.push_back(1, Type2Type<MUINT8>());
                pMetadata->update(entry.tag(), entry);
            }
            // set "the largest frame duration of streams" as "minimum frame duration"
            {
                MINT64 iMinFrmDuration = 0;
                for ( size_t i=0; i<result.vAppImageBuffers.size(); i++ ) {
                    StreamId_T const streamId = result.vAppImageBuffers.keyAt(i);
                    if( mParams->mStreamSet.mvStreamDurations.indexOfKey(streamId) < 0 ) {
                        MY_LOGE("Request App stream %#" PRIx64 "have not configured yet", streamId);
                        continue;
                    }
                    iMinFrmDuration = ( mParams->mStreamSet.mvStreamDurations.valueFor(streamId) > iMinFrmDuration)?
                        mParams->mStreamSet.mvStreamDurations.valueFor(streamId) : iMinFrmDuration;
                }
                MY_LOGD_IF( mCommonInfo->mLogLevel>=2, "The min frame duration is %" PRId64, iMinFrmDuration);
                IMetadata::IEntry entry(MTK_P1NODE_MIN_FRM_DURATION);
                entry.push_back(iMinFrmDuration, Type2Type<MINT64>());
                pMetadata->update(entry.tag(), entry);
            }
            //
            {
                MUINT8 bRepeating = (MUINT8) result.isRepeating;
                IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
                entry.push_back(bRepeating, Type2Type< MUINT8 >());
                pMetadata->update(entry.tag(), entry);
                MY_LOGD_IF( mCommonInfo->mLogLevel>=2, "Control AppMetadata is repeating(%d)", bRepeating);
            }
        #if 0
            // SMVR decide pass2 direct-link venc for performance
            if ( mParams.mOperation_mode && mParams.mDefaultBusrstNum>1 )
            {
                android::sp<IMetadataProvider const>
                    pMetadataProvider = NSMetadataProviderManager::valueFor(mOpenId);
                IMetadata::IEntry const& entry = pMetadataProvider->getMtkStaticCharacteristics()
                                                 .entryFor(MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS);
                if  ( entry.isEmpty() ) {
                    MY_LOGW("no static MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS");
                }
                else {
                    // [width, height, fps_min, fps_max, batch_size]
                    for ( size_t i=0; i<entry.count(); i+=5 )
                    {
                        MINT32 fps = entry.itemAt(i+3, Type2Type<MINT32>());
                        MSize  vdoSize = MSize( entry.itemAt(i  , Type2Type<MINT32>()),
                                                entry.itemAt(i+1, Type2Type<MINT32>()) );
                        if ( vdoSize == mParams.mVideoSize )
                        {
                            //update hal seting
                            if ( fps>=240 )
                            {
                                MY_LOGD("%dx%d@%d -> direct link", vdoSize.w, vdoSize.h, fps);
                                {
                                    IMetadata::IEntry entry(MTK_P2NODE_HIGH_SPEED_VDO_FPS);
                                    entry.push_back( fps, Type2Type< MINT32 >());
                                    pMetadata->update(entry.tag(), entry);
                                }
                                {
                                    IMetadata::IEntry entry(MTK_P2NODE_HIGH_SPEED_VDO_SIZE);
                                    entry.push_back( vdoSize, Type2Type< MSize >());
                                    pMetadata->update(entry.tag(), entry);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        #endif
        #if 0
            //update feature relative metadata, including vHDR
            if( mCommonInfo->mpAdvSettingMgr != NULL)
            {
                IMetadata* pAppMetaControl = request.pRequest->vIMetaBuffers[0]->tryWriteLock(__FUNCTION__);
                if( ! pAppMetaControl ) {
                    MY_LOGE("cannot get control meta");
                    return UNKNOWN_ERROR;
                }

                NSCam::AdvCamSettingMgr::PipelineParam pipelineParam;
                pipelineParam.mResizedRawSize = mParams->mHwParams.mResizedrawSize;
                pipelineParam.mSensorMode = mParams->mHwParams.mSensorMode;
                pipelineParam.mSensorSize = mParams->mHwParams.mSensorSize;
                pipelineParam.mVideoSize = mParams->mPipelineConfigParams.mVideoSize;
                pipelineParam.mMaxStreamSize = mParams->mPipelineConfigParams.mMaxStreamSize;
                pipelineParam.currentAdvSetting = mParams->mStreamingPipeParams.mpAdvSetting;
                NSCam::AdvCamSettingMgr::AdvCamInputParam advCamInput{
                    mParams->mPipelineConfigParams.mbHasRecording,
                    mParams->mPipelineConfigParams.mVideoSize,
                    mParams->mPipelineConfigParams.mb4KRecording,
                    mParams->mStreamingPipeParams.mpAdvSetting,
                    mParams->mConfigParams.mOperation_mode
                };

                NSCam::AdvCamSettingMgr::RequestParam reqParam;
                reqParam.mHasEncodeBuf = result.hasEncOut;
                reqParam.mIsRepeatingReq = result.isRepeating;

                mCommonInfo->mpAdvSettingMgr->updateRequestMeta(pMetadata, pAppMetaControl, pipelineParam, reqParam, advCamInput);
                if( pAppMetaControl )
                    request.pRequest->vIMetaBuffers[0]->unlock(
                        __FUNCTION__, pAppMetaControl
                    );
            }
        #endif
            //
            pBuf->unlock(LOG_TAG, pMetadata);
        }
        else
        {
            MY_LOGE("cannot get hal control meta sb.");
            return UNKNOWN_ERROR;
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineFrame>
PipelineDefaultImp::RequestHandler::
buildPipelineFrameLocked(
    MUINT32                 requestNo,
    evaluateRequestResult&  evaluateResult
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    //
    RequestBuilder builder;
    builder.setRootNode( evaluateResult.roots );
    builder.setNodeEdges( evaluateResult.edges );
    //
    for( size_t i = 0; i < evaluateResult.vUpdatedImageInfos.size(); i++ )
    {
        builder.replaceStreamInfo(
                evaluateResult.vUpdatedImageInfos.keyAt(i),
                evaluateResult.vUpdatedImageInfos.valueAt(i)
                );
    }
    //
#define try_setIOMap(_nodeId_)                                                        \
    do {                                                                              \
        ssize_t idx_image = evaluateResult.nodeIOMapImage.indexOfKey(_nodeId_);       \
        ssize_t idx_meta  = evaluateResult.nodeIOMapMeta.indexOfKey(_nodeId_);        \
        builder.setIOMap(                                                             \
                _nodeId_,                                                             \
                (0 <= idx_image ) ?                                                   \
                evaluateResult.nodeIOMapImage.valueAt(idx_image) : IOMapSet::empty(), \
                (0 <= idx_meta ) ?                                                    \
                evaluateResult.nodeIOMapMeta.valueAt(idx_meta) : IOMapSet::empty()    \
                );                                                                    \
    } while(0)
    //
    try_setIOMap(eNODEID_P1Node);
    try_setIOMap(eNODEID_P2Node);
    try_setIOMap(eNODEID_P2Node_VSS);
    try_setIOMap(eNODEID_RAW16Out);
    try_setIOMap(eNODEID_FDNode);
    try_setIOMap(eNODEID_JpegNode);
    //
#undef try_setIOMap
    //
#define setStreamBuffers(_sb_type_, _type_, _vStreamBuffer_, _builder_)    \
    do {                                                                   \
        for (size_t i = 0; i < _vStreamBuffer_.size(); i++ )               \
        {                                                                  \
            StreamId_T streamId                = _vStreamBuffer_.keyAt(i); \
            sp<_sb_type_> buffer = _vStreamBuffer_.valueAt(i);             \
            _builder_.set##_type_##StreamBuffer(streamId, buffer);         \
        }                                                                  \
    } while(0)
    //
    setStreamBuffers(IImageStreamBuffer  , Image, evaluateResult.vAppImageBuffers, builder);
    setStreamBuffers(HalImageStreamBuffer, Image, evaluateResult.vHalImageBuffers, builder);
    setStreamBuffers(IMetaStreamBuffer   , Meta , evaluateResult.vAppMetaBuffers , builder);
    setStreamBuffers(HalMetaStreamBuffer , Meta , evaluateResult.vHalMetaBuffers , builder);
#undef setStreamBuffers
    //
    sp<IPipelineFrame> pFrame = builder
        .updateFrameCallback(this)
        .build(requestNo, mCommonInfo->mpPipelineContext);
    //
    return pFrame;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineDefaultImp::RequestHandler::
isTimeSharingForJpegSource(
    parsedAppRequest const& request
) const
{
#if 0
    IMetadata* pAppMetaControl = request.pRequest->vIMetaBuffers[0]->tryReadLock(__FUNCTION__);
    if( ! pAppMetaControl ) {
        MY_LOGW("cannot get control meta");
        return MFALSE;
    }
    MBOOL bTS = MFALSE;
    // vss
    if ( mParams->mPipelineConfigParams.mbHasRecording && request.vOImageInfos_Jpeg.size()>0 )
        bTS = MTRUE;
    // reserved for c-shot(vendortag) / reprocessing in the future.
    if ( request.vOImageInfos_Jpeg.size()>0 )
    {
        if ( request.vIImageInfos_Opaque.size() || request.vIImageInfos_Yuv.size() )
        {
            MY_LOGW("reprocessing not use time-sharing flow");
            // bTS = MTRUE;
        }
        // bTS = MTRUE;
    }
    // others ...
    //
    if( pAppMetaControl )
        request.pRequest->vIMetaBuffers[0]->unlock(
                __FUNCTION__, pAppMetaControl
            );
    //
#endif
    return false;
}


/*******************************************************************************
 *
 ********************************************************************************/
PipelineDefaultImp::RequestHandler::
RequestHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    std::shared_ptr<MyProcessedParams> pParams,
    wp<IPipelineModelMgr::IAppCallback> pAppCallback,
    sp<PipelineDefaultImp::ConfigHandler> pConfigHandler
)
    : mpAppCallback(pAppCallback)
    , mCommonInfo(pCommonInfo)
    , mParams(pParams)
    , mPrevFDEn(MFALSE)
    , mConfigHandler(pConfigHandler)
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
 *
 ********************************************************************************/
PipelineDefaultImp::RequestHandler::
~RequestHandler()
{
    FUNC_START;
    FUNC_END;
}


/******************************************************************************
 *  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
 ******************************************************************************/
MVOID
PipelineDefaultImp::RequestHandler::
updateFrame(
    MUINT32 const frameNo,
    MINTPTR const userId,
    Result const& result
)
{
    if ( result.bFrameEnd ) return;

    MY_LOGD_IF( mCommonInfo->mLogLevel>=2, "frameNo %d, user %#" PRIxPTR ", AppLeft %zu, appMeta %zu, HalLeft %zu, halMeta %zu",
                frameNo, userId,
                result.nAppOutMetaLeft, result.vAppOutMeta.size(),
                result.nHalOutMetaLeft, result.vHalOutMeta.size()
                );
    sp<IPipelineModelMgr::IAppCallback> pAppCallback;
    pAppCallback = mpAppCallback.promote();
    if ( ! pAppCallback.get() ) {
        MY_LOGE("Have not set callback to device");
        FUNC_END;
        return;
    }
    pAppCallback->updateFrame(frameNo, userId, result.nAppOutMetaLeft, result.vAppOutMeta);
}
