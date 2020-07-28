/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "MtkCam/CameraSettingMgr_Imp"

#include "CameraSettingMgr_Imp.h"

#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "PipelineUtility.h"

using namespace NSCam;
using namespace NSCamHW;
using namespace android;

#undef __func__
#define __func__ __FUNCTION__

#define DEFAULT_LOG_LEVEL (2)

class my_scoped_tracer
{
/******************************************************************************
 *
 ******************************************************************************/
public:
    my_scoped_tracer(MINT32 openId, const char* functionName, MINT32 log, MINT32 logLevel)
    : mOpenId(openId)
    , mFunctionName(functionName)
    , mLogEn(log)
    , mLevel(logLevel)
    {
        MY_LOGI_IF(mLogEn <= mLevel, "id:%d[%s] +", mOpenId, mFunctionName);
    }
    ~my_scoped_tracer()
    {
        MY_LOGI_IF(mLogEn <= mLevel, "id:%d[%s] -", mOpenId, mFunctionName);
    }
private:
    const MINT32        mOpenId;
    const char* const   mFunctionName;
    const MINT32        mLogEn;
    const MINT32        mLevel;
};
#define MY_SCOPED_TRACER(Log) my_scoped_tracer ___scoped_tracer(mDeviceIdx, __FUNCTION__, Log, mLogLevel);

#define FUNC_ASSERT(exp, msg) \
    do{ if(!(exp)) { MY_LOGE("%s", msg); return INVALID_OPERATION; } } while(0)
/*******************************************************************************
* CameraSettingMgr_Imp  cons/destructors
********************************************************************************/
CameraSettingMgr_Imp::CameraSettingMgr_Imp(const MUINT32 DevIdx)
    : mDeviceIdx(DevIdx)
{
    mPrevFDEn = 0;
    mLogLevel = ::property_get_int32("debug.camera.log.camsetting", DEFAULT_LOG_LEVEL);
}

CameraSettingMgr_Imp::~CameraSettingMgr_Imp()
{
}

MINT32
CameraSettingMgr_Imp::generateCamSetting(PipeConfigParams const& rConfigParams, std::shared_ptr<MyProcessedParams> pParams)
{
    MY_SCOPED_TRACER(mLogLevel);
    pParams->update(rConfigParams, false);
    MERROR err;
    err =  pParams->querySensorStatics();
    err |= pParams->preprocess();
    err |= pParams->decideSensor();
    err |= pParams->decideP1();
    return err;
}

MERROR
CameraSettingMgr_Imp::evluateRequest( parsedAppRequest const& request,
                                      std::shared_ptr<MyProcessedParams> pParams,
                                      std::vector<evaluateRequestResult> &vReqResult
                                    )
{
    MY_SCOPED_TRACER(2);
    IMetadata* pAppMetaControl = request.pRequest->vIMetaBuffers[0]->tryReadLock(__FUNCTION__);
    evaluateRequestResult MainResult;
    vReqResult.clear();
    if( ! pAppMetaControl ) {
        MY_LOGE("cannot get control meta");
        return UNKNOWN_ERROR;
    }
    evaluateRequestLocked_InitRequestSetting(request, pParams, MainResult);
    // for Jpeg node
    if (MainResult.nodeInfos.needJpegNode)
    {
        createStreamInfoLocked_Jpeg_YUV(pAppMetaControl, pParams, MainResult.pHalImage_Jpeg_YUV);
        if( MainResult.pHalImage_Jpeg_YUV.get() )
        {
            MainResult.vUpdatedImageInfos.add(MainResult.pHalImage_Jpeg_YUV->getStreamId(), MainResult.pHalImage_Jpeg_YUV);
        }
        createStreamInfoLocked_Thumbnail_YUV(pAppMetaControl, pParams, MainResult.pHalImage_Thumbnail_YUV);
        if( MainResult.pHalImage_Thumbnail_YUV.get() )
        {
            MainResult.vUpdatedImageInfos.add(MainResult.pHalImage_Thumbnail_YUV->getStreamId(), MainResult.pHalImage_Thumbnail_YUV);
        }
    }
    evaluateRequestLocked_Jpeg(request, pParams, MainResult);
    evaluateRequestLocked_FD(request, pParams, MainResult);
    evaluateRequestLocked_Raw16(request, pParams, MainResult);
    evaluateRequestLocked_Streaming(request, pParams, MainResult);
    evaluateRequestLocked_Capture(request, pParams, MainResult);
    evaluateRequestLocked_Pass1(request, pParams, MainResult);
    
    vReqResult.push_back(MainResult);
    request.pRequest->vIMetaBuffers[0]->unlock(__FUNCTION__, pAppMetaControl);
    return OK;
}

// TODO : implement ZSD flow after basic flow
#define isZSDCap (0)
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_InitRequestSetting(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);
    //
    NodeEdgeSet& aEdges = ReqResult.edges;
    NodeSet& aRoot      = ReqResult.roots;
    aRoot.add(eNODEID_P1Node);
    if ( request.vIImageInfos_Yuv.size() )
    {
        FUNC_ASSERT(
                pParams->mStreamSet.mpAppImage_Yuv_In.get(),
                "wrong yuv in config");
        //
        ReqResult.reprocInfos.useYuvIn = true;
    }
    //
    if ( request.vIImageInfos_Opaque.size() )
    {
        FUNC_ASSERT(
                pParams->mStreamSet.mpAppImage_Opaque_In.get(),
                "wrong opaque in config");
        ReqResult.reprocInfos.useOpaqueIn = true;
    }
    //
    if ( request.vOImageInfos_Opaque.size() )
    {
        FUNC_ASSERT(
                pParams->mStreamSet.mpAppImage_Opaque_Out.get(),
                "wrong opaque out config");
        ReqResult.reprocInfos.useOpaqueOut = true;
    }
    //
    if ( isZSDCap )
    {
        ReqResult.isZsdCap = true;
    }
    //
    if( request.vOImageInfos_Jpeg.size() )
    {
        ReqResult.nodeInfos.needJpegNode = true;
        aEdges.addEdge(eNODEID_P2Node_VSS, eNODEID_JpegNode);
    }
    //
    if( ReqResult.nodeInfos.needJpegNode || ReqResult.reprocInfos.useYuvIn || ReqResult.reprocInfos.useOpaqueIn || ReqResult.isZsdCap )
    {
        ReqResult.nodeInfos.needCaptureNode = true;
        aEdges.addEdge(eNODEID_P1Node, eNODEID_P2Node_VSS);
        if ( ReqResult.reprocInfos.useYuvIn || ReqResult.reprocInfos.useOpaqueIn || ReqResult.isZsdCap )
        {
            ReqResult.nodeInfos.onlyCaptureNode = true;
        }
    }
    //
    if ( request.vOImageInfos_Raw.size() )
    {
        ReqResult.nodeInfos.needRaw16Node = true;
        aEdges.addEdge(eNODEID_P1Node, eNODEID_RAW16Out);
    }
    //
    if (request.vOImageInfos_Yuv.size() && !ReqResult.nodeInfos.onlyCaptureNode )
    {
        ReqResult.nodeInfos.needStreamNode = true;
        aEdges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
    }
    //
    if (isFdEnable(request, pParams, ReqResult) && ReqResult.nodeInfos.needStreamNode )
    {
        ReqResult.nodeInfos.needFDNode = true;
        aEdges.addEdge(eNODEID_P2Node, eNODEID_FDNode);
    }
    //
    if ( pParams->mPipelineConfigParams.mbHasLcso[0] && pParams->mStreamSet.mpHalImage_P1_Lcso[0].get())
    {
        ReqResult.nodeInfos.vhasLcso.push_back(true);
    }
    else
    {
        ReqResult.nodeInfos.vhasLcso.push_back(false);
    }
    //
    if ( pParams->mPipelineConfigParams.mbHasRsso[0] && pParams->mStreamSet.mpHalImage_P1_Rsso[0].get())
    {
        ReqResult.nodeInfos.vhasRsso.push_back(true);
    }
    else
    {
        ReqResult.nodeInfos.vhasRsso.push_back(false);
    }
    //
    return OK;
}
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_Jpeg(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);

    if ( !ReqResult.nodeInfos.needJpegNode || ReqResult.pHalImage_Jpeg_YUV == NULL)
    {
        MY_LOGD("No need Jpeg node");
        return OK;
    }
    IOMap JpegMap;
    JpegMap.addIn(ReqResult.pHalImage_Jpeg_YUV->getStreamId());
    if (ReqResult.pHalImage_Thumbnail_YUV != NULL)
    {
        JpegMap.addIn(ReqResult.pHalImage_Thumbnail_YUV->getStreamId());
    }
    JpegMap.addOut(pParams->mStreamSet.mpAppImage_Jpeg->getStreamId());
    //
    ReqResult.nodeIOMapImage.add(eNODEID_JpegNode, IOMapSet().add(JpegMap));
    ReqResult.nodeIOMapMeta.add(eNODEID_JpegNode, 
                                    IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpAppMeta_Control->getStreamId())
                                    .addIn(pParams->mStreamSet.mpHalMeta_DynamicP2Capture->getStreamId())
                                    .addOut(pParams->mStreamSet.mpAppMeta_DynamicJpeg->getStreamId()))
                        );
    //
    return OK;
}
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_FD(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);

    if ( !ReqResult.nodeInfos.needFDNode )
    {
        MY_LOGD("No need FD node");
        return OK;
    }
    IOMap FDMap;
    FDMap.addIn(pParams->mStreamSet.mpHalImage_FD_YUV->getStreamId());
    //
    ReqResult.nodeIOMapImage.add(eNODEID_FDNode, IOMapSet().add(FDMap));
    ReqResult.nodeIOMapMeta.add(eNODEID_FDNode, 
                                    IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpAppMeta_Control->getStreamId())
                                    //.addIn(pParams->mStreamSet.mpAppMeta_DynamicP2->getStreamId()) TBD
                                    .addOut(pParams->mStreamSet.mpAppMeta_DynamicFD->getStreamId()))
                        );
    //
    return OK;
}
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_Raw16(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);
    //
    if ( !ReqResult.nodeInfos.needRaw16Node )
    {
        MY_LOGD("No need Raw16 node");
        return OK;
    }
    IImageStreamInfo const* pStreamInfo = request.vOImageInfos_Raw[0].get();
    if ( pParams->mStreamSet.mpAppImage_RAW16.get() && 
         pParams->mStreamSet.mpAppImage_RAW16->getStreamId() == pStreamInfo->getStreamId() )
    {
        // Raw16: full-size raw -> raw16
        //
        FUNC_ASSERT(
                pParams->mPipelineConfigParams.mbUseRaw16Node &&
                pParams->mStreamSet.mpHalImage_P1_Raw[0].get() && pParams->mStreamSet.mpAppImage_RAW16.get(),
                "not properly configured");
        //
        ReqResult.nodeIOMapImage.add(eNODEID_RAW16Out,
                                     IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpHalImage_P1_Raw[0]->getStreamId())
                                    .addOut(pParams->mStreamSet.mpAppImage_RAW16->getStreamId())));
        MY_LOGD("evaluateRequestLocked add RAW16");
    }
    else
    {
        MY_LOGE("not supported raw output stream %#" PRIx64 ,
                pStreamInfo->getStreamId());
        return INVALID_OPERATION;
    }
    return OK;
}
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_Streaming(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);
    //
    ReqResult.vPrvSettingResult.clear();
    if ( !ReqResult.nodeInfos.needStreamNode )
    {
        MY_LOGD("Has input buffer or ZSD(%d)", ReqResult.isZsdCap);
        return OK;
    }
    evaluateRequestResult::SettingResult Result;
    IOMap ImgoMap;
    IOMap RrzoMap;
    sp<IImageStreamInfo> ImgoInfo;
    sp<IImageStreamInfo> RrzoInfo;
    //
    if ( ReqResult.isOpaqueReprocOut() )
    {
        ImgoInfo = pParams->mStreamSet.mpAppImage_Opaque_Out;
    }
    else
    {
        ImgoInfo = pParams->mStreamSet.mpHalImage_P1_Raw[0];
    }
    RrzoInfo = pParams->mStreamSet.mpHalImage_P1_ResizerRaw[0];
    for( size_t i = 0; i < request.vOImageInfos_Yuv.size(); i++ )
    {
        sp<IImageStreamInfo> pInfo = request.vOImageInfos_Yuv.valueAt(i);
        StreamId_T const streamId = pInfo->getStreamId();
        if( 0 <= pParams->mStreamSet.mvYuvStreams_Fullraw.indexOf(streamId) )
        {
            ImgoMap.addOut(streamId);
        }
        else if( 0 <= pParams->mStreamSet.mvYuvStreams_Resizedraw.indexOf(streamId) )
        {
            RrzoMap.addOut(streamId);
        }
        else
        {
            MY_LOGE("cannot find propery raw for stream %s(%#" PRIx64 ")",
                    pInfo->getStreamName(),streamId);
            return UNKNOWN_ERROR;
        }
    }
    if (ReqResult.nodeInfos.needFDNode)
    {
        RrzoMap.addOut(pParams->mStreamSet.mpHalImage_FD_YUV->getStreamId());
    }
    if (ImgoMap.sizeOut() > 0)
    {
        MY_LOGD("has full yuv ouput");
        ImgoMap.addIn(ImgoInfo->getStreamId());
        Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Imgo;
        if (ReqResult.nodeInfos.vhasLcso[0])
        {
            ImgoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Lcso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Lcso;
        }
        if (ReqResult.nodeInfos.vhasRsso[0])
        {
            ImgoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Rsso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Rsso;
        }
        Result.P2IO.add(ImgoMap);
    }
    if (RrzoMap.sizeOut() > 0)
    {
        MY_LOGD("has rrzo yuv ouput");
        RrzoMap.addIn(RrzoInfo->getStreamId());
        Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Rrzo;
        if (ReqResult.nodeInfos.vhasLcso[0])
        {
            RrzoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Lcso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Lcso;
        }
        if (ReqResult.nodeInfos.vhasRsso[0])
        {
            RrzoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Rsso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Rsso;
        }
        Result.P2IO.add(RrzoMap);
    }
    ReqResult.vPrvSettingResult.push_back(Result);
    //

    if (ReqResult.vPrvSettingResult.size() > 0)
    {
        // TODO : need process multi frame?
        ReqResult.nodeIOMapImage.add(eNODEID_P2Node, ReqResult.vPrvSettingResult[0].P2IO);
        ReqResult.nodeIOMapMeta.add(eNODEID_P2Node, 
                                    IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpAppMeta_Control->getStreamId())
                                    .addIn(pParams->mStreamSet.mpHalMeta_DynamicP1[0]->getStreamId())
                                    .addIn(pParams->mStreamSet.mpAppMeta_DynamicP1[0]->getStreamId())
                                    .addOut(pParams->mStreamSet.mpHalMeta_DynamicP2->getStreamId())
                                    .addOut(pParams->mStreamSet.mpAppMeta_DynamicP2->getStreamId()))
                        );
    }
    return OK;
}
//
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_Capture(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);
    //
    ReqResult.vCapSettingResult.clear();
    if ( !ReqResult.nodeInfos.needCaptureNode )
    {
        MY_LOGD("no need capture node");
        return OK;
    }
    evaluateRequestResult::SettingResult Result;
    IOMap ImgoMap;
    IOMap RrzoMap;
    sp<IImageStreamInfo> ImgoInfo;
    sp<IImageStreamInfo> RrzoInfo;
    bool IsReproc = false;
    //
    if ( ReqResult.isOpaqueReprocOut() )
    {
        ImgoInfo = pParams->mStreamSet.mpAppImage_Opaque_Out;
    }
    else if (ReqResult.isOpaqueReprocIn())
    {
        ImgoInfo = pParams->mStreamSet.mpAppImage_Opaque_In;
        IsReproc = true;
    }
    else if (ReqResult.isYuvReprocIn())
    {
        ImgoInfo = pParams->mStreamSet.mpAppImage_Yuv_In;
        IsReproc = true;
    }
    else
    {
        ImgoInfo = pParams->mStreamSet.mpHalImage_P1_Raw[0];
    }
    RrzoInfo = pParams->mStreamSet.mpHalImage_P1_ResizerRaw[0];
    if (ReqResult.nodeInfos.onlyCaptureNode)
    {
        for( size_t i = 0; i < request.vOImageInfos_Yuv.size(); i++ )
        {
            sp<IImageStreamInfo> pInfo = request.vOImageInfos_Yuv.valueAt(i);
            StreamId_T const streamId = pInfo->getStreamId();
            if( 0 <= pParams->mStreamSet.mvYuvStreams_Fullraw.indexOf(streamId) || IsReproc)
            {
                ImgoMap.addOut(streamId);
            }
            else if( 0 <= pParams->mStreamSet.mvYuvStreams_Resizedraw.indexOf(streamId) )
            {
                RrzoMap.addOut(streamId);
            }
            else
            {
                MY_LOGE("cannot find propery raw for stream %s(%#" PRIx64 ")",
                        pInfo->getStreamName(),streamId);
                return UNKNOWN_ERROR;
            }
        }
    }
    if (ReqResult.nodeInfos.needJpegNode)
    {
        if (ImgoInfo != NULL)
        {
            ImgoMap.addOut(ReqResult.pHalImage_Jpeg_YUV->getStreamId());
            if (ReqResult.pHalImage_Thumbnail_YUV != NULL)
            {
                ImgoMap.addOut(ReqResult.pHalImage_Thumbnail_YUV->getStreamId());
            }
        }
        else
        {

            RrzoMap.addOut(ReqResult.pHalImage_Jpeg_YUV->getStreamId());
            if (ReqResult.pHalImage_Thumbnail_YUV != NULL)
            {
                RrzoMap.addOut(ReqResult.pHalImage_Thumbnail_YUV->getStreamId());
            }
        }
    }
    if (ImgoMap.sizeOut() > 0)
    {
        MY_LOGD("has full yuv ouput");
        ImgoMap.addIn(ImgoInfo->getStreamId());
        Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Imgo;
        if (ReqResult.nodeInfos.vhasLcso[0] && !IsReproc)
        {
            ImgoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Lcso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Lcso;
        }
        Result.P2IO.add(ImgoMap);
    }
    if (RrzoMap.sizeOut() > 0)
    {
        MY_LOGD("has rrzo yuv ouput");
        RrzoMap.addIn(RrzoInfo->getStreamId());
        Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Rrzo;
        if (ReqResult.nodeInfos.vhasLcso[0] && !IsReproc)
        {
            RrzoMap.addIn(pParams->mStreamSet.mpHalImage_P1_Lcso[0]->getStreamId());
            Result.P1RawMap |= evaluateRequestResult::eP1RawType_Main1_Lcso;
        }
        Result.P2IO.add(RrzoMap);
    }
    ReqResult.vCapSettingResult.push_back(Result);
    //

    if (ReqResult.vCapSettingResult.size() > 0)
    {
        // TODO : need process multi frame?
        ReqResult.nodeIOMapImage.add(eNODEID_P2Node_VSS, ReqResult.vCapSettingResult[0].P2IO);
        ReqResult.nodeIOMapMeta.add(eNODEID_P2Node_VSS, 
                                    IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpAppMeta_Control->getStreamId())
                                    .addIn(pParams->mStreamSet.mpHalMeta_DynamicP1[0]->getStreamId())
                                    .addIn(pParams->mStreamSet.mpAppMeta_DynamicP1[0]->getStreamId())
                                    .addOut(pParams->mStreamSet.mpHalMeta_DynamicP2Capture->getStreamId())
                                    .addOut(pParams->mStreamSet.mpAppMeta_DynamicP2Capture->getStreamId()))
                        );
    }
    return OK;
}
//
#define max(a,b)  ((a) < (b) ? (b) : (a))
#define min(a,b)  ((a) < (b) ? (a) : (b))
MERROR
CameraSettingMgr_Imp::
evaluateRequestLocked_Pass1(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult &ReqResult
)
{
    MY_SCOPED_TRACER(2);
    CAM_TRACE_NAME(__FUNCTION__);
    //
    int needP1RawTypes = 0;
    IOMap P1Map;
    if (ReqResult.vPrvSettingResult.size() > 0)
    {
        needP1RawTypes |= ReqResult.vPrvSettingResult[0].P1RawMap;
    }
    if (ReqResult.vCapSettingResult.size() > 0)
    {
        needP1RawTypes |= ReqResult.vCapSettingResult[0].P1RawMap;
    }
    // set p1 image iomap
    if (ReqResult.isOpaqueReprocIn())
    {
        P1Map.addIn(pParams->mStreamSet.mpAppImage_Opaque_In->getStreamId());
    }
    else if (ReqResult.isYuvReprocIn())
    {
        P1Map.addIn(pParams->mStreamSet.mpAppImage_Yuv_In->getStreamId());
    }
    else
    {
        if ( ReqResult.isOpaqueReprocOut() )
        {
            P1Map.addOut(pParams->mStreamSet.mpAppImage_Opaque_Out->getStreamId());
        }
        else if (needP1RawTypes & evaluateRequestResult::eP1RawType_Main1_Imgo)
        {
            P1Map.addOut(pParams->mStreamSet.mpHalImage_P1_Raw[0]->getStreamId());
        }
        if (needP1RawTypes & evaluateRequestResult::eP1RawType_Main1_Rrzo)
        {
            P1Map.addOut(pParams->mStreamSet.mpHalImage_P1_ResizerRaw[0]->getStreamId());
        }
        if (needP1RawTypes & evaluateRequestResult::eP1RawType_Main1_Lcso)
        {
            P1Map.addOut(pParams->mStreamSet.mpHalImage_P1_Lcso[0]->getStreamId());
        }
        if (needP1RawTypes & evaluateRequestResult::eP1RawType_Main1_Rsso)
        {
            P1Map.addOut(pParams->mStreamSet.mpHalImage_P1_Rsso[0]->getStreamId());
        }
    }
    ReqResult.nodeIOMapImage.add(eNODEID_P1Node, IOMapSet().add(P1Map));
    ReqResult.nodeIOMapMeta.add(eNODEID_P1Node, 
                                    IOMapSet().add(IOMap()
                                    .addIn(pParams->mStreamSet.mpAppMeta_Control->getStreamId())
                                    .addIn(pParams->mStreamSet.mpHalMeta_Control[0]->getStreamId())
                                    .addOut(pParams->mStreamSet.mpAppMeta_DynamicP1[0]->getStreamId())
                                    .addOut(pParams->mStreamSet.mpHalMeta_DynamicP1[0]->getStreamId()))
                        );
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CameraSettingMgr_Imp::
createStreamInfoLocked_Jpeg_YUV(
    IMetadata const* pMetadata,
    std::shared_ptr<MyProcessedParams> pParams,
    android::sp<IImageStreamInfo>& rpStreamInfo
)
{
    MY_SCOPED_TRACER(2);
    if ( pParams->mStreamSet.mpHalImage_Jpeg_YUV == 0 ) {
        MY_LOGW("No config stream: Jpeg_YUV");
        return NO_INIT;
    }
    rpStreamInfo = pParams->mStreamSet.mpHalImage_Jpeg_YUV;
    if (!pParams->mSupportJpegRot)
    {
        return OK;
    }
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return NAME_NOT_FOUND;
    }
    //
    MINT32 const jpegOrientation = entryJpegOrientation.itemAt(0, Type2Type<MINT32>());
    MUINT32      jpegTransform   = 0;
    if ( 0==jpegOrientation )
        jpegTransform = 0;
    else if ( 90==jpegOrientation )
        jpegTransform = eTransform_ROT_90;
    else if ( 180==jpegOrientation )
        jpegTransform = eTransform_ROT_180;
    else if ( 270==jpegOrientation )
        jpegTransform = eTransform_ROT_270;
    else
         MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    //
    MUINT32 const imgTransform   = pParams->mStreamSet.mpHalImage_Jpeg_YUV->getTransform();
    MY_LOGD_IF( 1, "Jpeg orientation from metadata:%d transform current(%d) & previous(%d)",
                jpegOrientation, jpegTransform, imgTransform);
    if ( imgTransform == jpegTransform ) {
        return OK;
    }
    MSize size;
    if ( jpegTransform&eTransform_ROT_90 ) { // pillarbox
        size.w = min(pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().w, pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().h);
        size.h = max(pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().w, pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().h);
    } else { // letterbox
        size.w = max(pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().w, pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().h);
        size.h = min(pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().w, pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize().h);
    }
    MINT const format = pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgFormat();
    MUINT const usage = pParams->mStreamSet.mpHalImage_Jpeg_YUV->getUsageForAllocator();
    sp<IImageStreamInfo>
        pStreamInfo = createImageStreamInfo(
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getStreamName(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getStreamId(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getStreamType(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getMaxBufNum(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getMinInitBufNum(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getUsageForAllocator(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgFormat(),
            size, jpegTransform
        );
    if( pStreamInfo == NULL ) {
        MY_LOGE(
            "fail to new ImageStreamInfo: %s %#" PRIx64,
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getStreamName(),
            pParams->mStreamSet.mpHalImage_Jpeg_YUV->getStreamId()
        );
        return NO_MEMORY;
    }
    rpStreamInfo = pStreamInfo;
    MY_LOGD_IF(
        1,
        "streamId:%#" PRIx64 " %s %p (%p) yuvsize:%dx%d jpegOrientation:%d",
        rpStreamInfo->getStreamId(),
        rpStreamInfo->getStreamName(),
        rpStreamInfo.get(),
        pParams->mStreamSet.mpHalImage_Jpeg_YUV.get(),
        rpStreamInfo->getImgSize().w, rpStreamInfo->getImgSize().h, jpegOrientation
    );
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CameraSettingMgr_Imp::
createStreamInfoLocked_Thumbnail_YUV(
    IMetadata const* pMetadata,
    std::shared_ptr<MyProcessedParams> pParams,
    android::sp<IImageStreamInfo>& rpStreamInfo
)
{
    MY_SCOPED_TRACER(2);
    if  ( pParams->mStreamSet.mpHalImage_Thumbnail_YUV == 0 ) {
        MY_LOGW("No config stream: Thumbnail_YUV");
        return NO_INIT;
    }
    //
    rpStreamInfo = NULL;
    IMetadata::IEntry const& entryThumbnailSize = pMetadata->entryFor(MTK_JPEG_THUMBNAIL_SIZE);
    if  ( entryThumbnailSize.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_THUMBNAIL_SIZE");
        return NAME_NOT_FOUND;
    }
    MSize const& thumbnailSize = entryThumbnailSize.itemAt(0, Type2Type<MSize>());
    if  ( ! thumbnailSize ) {
        MY_LOGW("Bad thumbnail size: %dx%d", thumbnailSize.w, thumbnailSize.h);
        return NOT_ENOUGH_DATA;
    }
    MY_LOGD_IF( 1, "thumbnail size from metadata: %dx%d", thumbnailSize.w, thumbnailSize.h);
    //
    //
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return NAME_NOT_FOUND;
    }
    //
    MSize const yuvthumbnailsize = calcThumbnailYuvSize(
                                        pParams->mStreamSet.mpHalImage_Jpeg_YUV->getImgSize(),
                                        thumbnailSize
                                        );
    //
    MINT32  jpegOrientation = 0;
    MUINT32 jpegTransform   = 0;
    MSize   thunmbSize      = yuvthumbnailsize; // default thumbnail size
    //
    MINT const format = pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getImgFormat();
    IImageStreamInfo::BufPlanes_t bufPlanes;
    switch (format)
    {
    case eImgFmt_YUY2:{
        IImageStreamInfo::BufPlane bufPlane;
        bufPlane.rowStrideInBytes = (yuvthumbnailsize.w << 1);
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * yuvthumbnailsize.h;
        bufPlanes.push_back(bufPlane);
        }break;
    default:
        MY_LOGE("not supported format: %#x", format);
        break;
    }
    //
    rpStreamInfo = createImageStreamInfo(
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getStreamName(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getStreamId(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getStreamType(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getMaxBufNum(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getMinInitBufNum(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getUsageForAllocator(),
        format,
        thunmbSize,
        jpegTransform
    );
    if  ( rpStreamInfo == 0 ) {
        MY_LOGE(
            "fail to new ImageStreamInfo: %s %#" PRIx64,
            pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getStreamName(),
            pParams->mStreamSet.mpHalImage_Thumbnail_YUV->getStreamId()
        );
        return NO_MEMORY;
    }
    //
    MY_LOGD_IF(
        1,
        "streamId:%#" PRIx64 " %s %p %p yuvthumbnailsize:%dx%d jpegOrientation:%d",
        rpStreamInfo->getStreamId(),
        rpStreamInfo->getStreamName(),
        rpStreamInfo.get(),
        pParams->mStreamSet.mpHalImage_Thumbnail_YUV.get(),
        thunmbSize.w, thunmbSize.h, jpegOrientation
    );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
CameraSettingMgr_Imp::
calcThumbnailYuvSize(
    MSize const rPicSize,
    MSize const rThumbnailsize
)
{
    MY_SCOPED_TRACER(2);
#define align2(x) (((x) + 1) & (~0x1))
    MSize size;
    MUINT32 const val0 = rPicSize.w * rThumbnailsize.h;
    MUINT32 const val1 = rPicSize.h * rThumbnailsize.w;
    if( val0 > val1 ) {
        size.w = align2(val0/rPicSize.h);
        size.h = rThumbnailsize.h;
    }
    else if( val0 < val1 ) {
        size.w = rThumbnailsize.w;
        size.h = align2(val1/rPicSize.w);
    }
    else {
        size = rThumbnailsize;
    }
#undef align2
    MY_LOGD_IF(1, "thumb %dx%d, pic %dx%d -> yuv for thumb %dx%d",
            rThumbnailsize.w, rThumbnailsize.h,
            rPicSize.w, rPicSize.h,
            size.w, size.h
            );
    return size;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CameraSettingMgr_Imp::
isFdEnable(
    parsedAppRequest const& request,
    std::shared_ptr<MyProcessedParams> pParams,
    evaluateRequestResult& result
)
{
    MY_SCOPED_TRACER(2);
    MBOOL bSupported = MFALSE;
    IMetadata* pAppMetaControl = request.pRequest->vIMetaBuffers[0]->tryReadLock(__FUNCTION__);
    if( ! pAppMetaControl ) {
        MY_LOGE("cannot get control meta");
        return bSupported;
    }
    //
    if( pParams->mPipelineConfigParams.mbUseFDNode )
    {
        bSupported = isFdEnable(pAppMetaControl);
    }
    //
    if( pAppMetaControl )
        request.pRequest->vIMetaBuffers[0]->unlock(__FUNCTION__, pAppMetaControl);
    //
    return bSupported;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CameraSettingMgr_Imp::
isFdEnable(
    IMetadata const* pMetadata
)
{
    MY_SCOPED_TRACER(2);
    //  If Face detection is not OFF or scene mode is face priority,
    //  add App:Meta:FD_result stream to Output App Meta Streams.
    //return 0;

    IMetadata::IEntry const& entryFdMode = pMetadata->entryFor(MTK_STATISTICS_FACE_DETECT_MODE);
    IMetadata::IEntry const& entryfaceScene = pMetadata->entryFor(MTK_CONTROL_SCENE_MODE);
    IMetadata::IEntry const& entryGdMode = pMetadata->entryFor(MTK_FACE_FEATURE_GESTURE_MODE);
    IMetadata::IEntry const& entrySdMode = pMetadata->entryFor(MTK_FACE_FEATURE_SMILE_DETECT_MODE);
    IMetadata::IEntry const& entryAsdMode = pMetadata->entryFor(MTK_FACE_FEATURE_ASD_MODE);

    MBOOL FDMetaEn, FDEnable;
    //
    FDMetaEn =   //(0 != mDebugFdMode) ||
             ( !entryFdMode.isEmpty() && MTK_STATISTICS_FACE_DETECT_MODE_OFF != entryFdMode.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryfaceScene.isEmpty() && MTK_CONTROL_SCENE_MODE_FACE_PRIORITY == entryfaceScene.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryGdMode.isEmpty() && MTK_FACE_FEATURE_GESTURE_MODE_OFF != entryGdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entrySdMode.isEmpty() && MTK_FACE_FEATURE_SMILE_DETECT_MODE_OFF != entrySdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entryAsdMode.isEmpty() && MTK_FACE_FEATURE_ASD_MODE_OFF != entryAsdMode.itemAt(0, Type2Type<MINT32>()));
    FDEnable = mPrevFDEn || FDMetaEn;
    mPrevFDEn = FDMetaEn;
    return FDEnable;

}


#if 0
sp<AdvCamSetting>
AdvCamSettingMgr_Imp::regenAdvSetting(IMetadata* appMetadata, const AdvCamInputParam &inputParam, const UserParam &userParam)
{
    if(appMetadata == NULL){
        ACSM_ERR("app metadata is NULL!, regen AdvCamSetting FAILED!");
        return NULL;
    }

    MBOOL useAdvP2 = (inputParam.lastAdvCamSetting != NULL)
                                ? inputParam.lastAdvCamSetting->useAdvP2
                                : MFALSE;

    MBOOL isTkApp = (inputParam.lastAdvCamSetting != NULL)
                                ? inputParam.lastAdvCamSetting->isTkApp
                                : MFALSE;

    return convertMetaToSetting(isTkApp, useAdvP2 , *appMetadata, inputParam, userParam);;
}

sp<AdvCamSetting>
AdvCamSettingMgr_Imp::convertMetaToSetting(       MBOOL             isTkApp,
                                                  MBOOL             useAdvP2,
                                            const IMetadata         &configMeta,
                                            const AdvCamInputParam  &inputPara,
                                            const UserParam         &userParam)
{
    sp<AdvCamSetting> outSet = new AdvCamSetting();
    outSet->useAdvP2 = useAdvP2;
    outSet->isTkApp = isTkApp;

    // parse metadata to adv camera setting
    // ===== HDR =====
    HDRMode hdrMode = getHDRMode(&configMeta);
    outSet->vhdrMode = getVHDRMode(hdrMode,NULL);

    // ===== 3DNR ======
    outSet->nr3dMode = get3DNRMode(configMeta, inputPara, userParam);
    outSet->needLMV = (outSet->nr3dMode &
        (NR3D::E3DNR_MODE_MASK_UI_SUPPORT | NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) != 0;

    // ===== EIS ======
    calEisSettings(&configMeta, inputPara, outSet->vhdrMode, outSet->isTkApp, outSet->useAdvP2,
                    outSet->eisMode, outSet->eisFactor, outSet->eisExtraBufNum);
    outSet->useTSQ = mEnableTSQ && isAdvEisEnabled(outSet);

    ACSM_LOG("adv Cam Setting, hdr(%hhu), vhdr(%d), 3dnr(0x%x), needLMV(%d), \
eis(0x%x/%u/%u), isRec/isTkApp(%d/%d), is4K(%d), videoSize(%dx%d), useAdvP2(%d)",
            hdrMode, outSet->vhdrMode, outSet->nr3dMode, outSet->needLMV,
            outSet->eisMode, outSet->eisFactor, outSet->eisExtraBufNum,
            inputPara.isRecordPipe, outSet->isTkApp,  inputPara.is4K2K, inputPara.videoSize.w, inputPara.videoSize.h,
            outSet->useAdvP2);

    return outSet;
}

/*******************************************************************************
* App doesn't set config param before pipeline config. We need to generate default config from template
********************************************************************************/
MBOOL AdvCamSettingMgr_Imp::genDefaultConfig(const AdvCamInputParam& inputParam,
                                                          IMetadata&        outMeta)
{
    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(mSensorIdx);
    if( obj == NULL)
    {
        ACSM_ERR("Default template request not built yet !! Can not generate default config.");
        return MFALSE;
    }

    MUINT32 reqType = CAMERA3_TEMPLATE_PREVIEW;
    if(inputParam.operationMode == CAMERA3_STREAM_CONFIGURATION_NORMAL_MODE
        || inputParam.operationMode >= CAMERA3_VENDOR_STREAM_CONFIGURATION_MODE_START)
    {
        if(inputParam.isRecordPipe)
            reqType = CAMERA3_TEMPLATE_VIDEO_RECORD;
    }
    else
    {
        ACSM_WRN("Default Config not support stream operation mode (%d)", inputParam.operationMode);
        return MFALSE;
    }

    outMeta = obj->getMtkData(reqType);

    // Customize config meta by vendor's pipe operation mode
    customConfigMeta(inputParam, outMeta);


    ACSM_LOG("generate Default config done. Template/OperationMode(%u,%u)", reqType, inputParam.operationMode);

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::needReconfigPipeline(IMetadata* appMetadata , const PipelineParam &pipelineParam,
                                           const AdvCamInputParam &inputParam)
{
    MBOOL needReconfig = MFALSE;
    MBOOL reconfig = MFALSE;
    sp<AdvCamSetting> curSet = pipelineParam.currentAdvSetting;

    // Check need reconfig
    reconfig |= needReconfigByHDR(appMetadata, curSet);
    reconfig |= needReconfigByEIS(appMetadata, inputParam, curSet);


    return reconfig;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateRequestMeta(
                        IMetadata*  halMetadata,
                        IMetadata*  appMetadata,
                  const PipelineParam &pipelineParam,
                  const RequestParam  &reqParam,
                  const AdvCamInputParam &inputParam
)
{
    if(!appMetadata)
    {
       ACSM_ERR("app metadata is NULL!");
       return MFALSE;
    }

    // Need update common meta first
    updateStreamingState(appMetadata, reqParam, inputParam, halMetadata);

    // Prepare query Feature Streaming ISP Profile
    HDRMode hdrMode = getHDRMode(appMetadata);

    // update HDR mode to 3A
    IMetadata::setEntry<MUINT8>(halMetadata, MTK_3A_HDR_MODE, (MUINT8)toLiteral(hdrMode));

    // EIS
    updateEISRequestMeta(appMetadata, pipelineParam, halMetadata);

    // update ISP Profile
    updateIspProfile(hdrMode, halMetadata, appMetadata, pipelineParam);

    // ======== App Metadata Update ========
    if(!updateCropRegion(halMetadata, appMetadata, pipelineParam))
        return MFALSE;


    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateStreamingState(
                               const IMetadata*          appMetadata,
                               const RequestParam&       reqParam,
                               const AdvCamInputParam&   inputParam,
                                     IMetadata*          halMetadata)
{
    MINT32 recordState = -1;

    // 1. Decide App Mode
    if (IMetadata::getEntry<MINT32>(appMetadata, MTK_STREAMING_FEATURE_RECORD_STATE, recordState) )
    {   // App has set recordState Tag
        if(recordState == MTK_STREAMING_FEATURE_RECORD_STATE_PREVIEW)
        {
            if(reqParam.mHasEncodeBuf)
                mAppMode = MTK_FEATUREPIPE_VIDEO_STOP;
            else
                mAppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
        }
        else
        {
            ACSM_WRN("Unknown or Not Supported app recordState(%d)", recordState);
        }
    }
    else
    {   // App has NOT set recordState Tag
        // (slow motion has no repeating request)
        if( reqParam.mIsRepeatingReq
            || inputParam.operationMode == CAMERA3_STREAM_CONFIGURATION_CONSTRAINED_HIGH_SPEED_MODE)
        {
            if(reqParam.mHasEncodeBuf)
                mAppMode = MTK_FEATUREPIPE_VIDEO_RECORD;
            else if (inputParam.isRecordPipe)
                mAppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
            else
                mAppMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
        }
    }

    IMetadata::setEntry<MINT32>(halMetadata, MTK_FEATUREPIPE_APP_MODE, mAppMode);

    MBOOL isRecStaChange = (mLastInfo.recordState != recordState);
    MBOOL isAppModeChange = (mLastInfo.appMode != mAppMode);

    if(isAppModeChange)
    {
        if( (mLastInfo.appMode == MTK_FEATUREPIPE_VIDEO_PREVIEW || mLastInfo.appMode == MTK_FEATUREPIPE_PHOTO_PREVIEW)
                && mAppMode == MTK_FEATUREPIPE_VIDEO_STOP)
            ACSM_WRN("Error state machine change : App mode video/photo preview -> video stop");
        else if(mLastInfo.appMode == MTK_FEATUREPIPE_VIDEO_STOP && mAppMode == MTK_FEATUREPIPE_VIDEO_RECORD)
            ACSM_WRN("Error state machine change : App mode video stop -> video record");
    }

    mLastInfo.recordState = recordState;
    mLastInfo.appMode = mAppMode;

    MY_LOGD_IF((mLogLevel || isRecStaChange || isAppModeChange),
            "rec/appChange(%d/%d), recState/appMode(%d/%d),isRep/hasEnc(%d,%d)",
            isRecStaChange, isAppModeChange ,recordState, mAppMode, reqParam.mIsRepeatingReq, reqParam.mHasEncodeBuf);
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateIspProfile( HDRMode     hdrMode,
                                        IMetadata*  halMetadata,
                                        IMetadata*  appMetadata,
                                  const PipelineParam &pipelineParam)
{
    MUINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
    MINT32 fMask = ProfileParam::FMASK_NONE;
    if ((hdrMode == HDRMode::AUTO) || (hdrMode == HDRMode::VIDEO_AUTO))
        fMask |= ProfileParam::FMASK_AUTO_HDR_ON;
    vhdrMode = getVHDRMode(hdrMode, pipelineParam.currentAdvSetting);

    if( isEisEnabled(pipelineParam.currentAdvSetting) )
    {
        fMask |= ProfileParam::FMASK_EIS_ON;
    }

    //
    MUINT8 profile = 0;

    ProfileParam profileParam(
        pipelineParam.mResizedRawSize,
        vhdrMode,
        pipelineParam.mSensorMode,
        ProfileParam::FLAG_NONE, // TODO set flag by isZSDPureRawStreaming or not
        fMask
    );
    //for streaming ISP profile
    if (FeatureProfileHelper::getStreamingProf(profile, profileParam))
    {
        IMetadata::setEntry<MUINT8>(halMetadata, MTK_3A_ISP_PROFILE, profile);
    }
    //for VSS/normal capture ISP profile
    if (FeatureProfileHelper::getShotProf(profile, profileParam))
    {
        IMetadata::setEntry<MUINT8>(halMetadata, MTK_VHDR_IMGO_3A_ISP_PROFILE, profile);
    }

    if (mLogLevel >= 1)
    {
        // Print debug info
        MUINT8 RhdrMode = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_3A_HDR_MODE, RhdrMode)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_3A_HDR_MODE Get!");
        }
        MUINT8 Rprofile = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_3A_ISP_PROFILE, Rprofile)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_3A_ISP_PROFILE Get!");
        }
        MUINT8 RIMGOprofile = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_VHDR_IMGO_3A_ISP_PROFILE, RIMGOprofile)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_VHDR_IMGO_3A_ISP_PROFILE:%d",RIMGOprofile);
        }
        MINT32 RrequestID = 0;
        if(IMetadata::getEntry<MINT32>(appMetadata, MTK_REQUEST_ID, RrequestID)){
            MY_LOGD_IF((mLogLevel >= 2), "get APP:MTK_REQUEST_ID:%d",RrequestID);
        }

        ACSM_LOG("Get MTK_3A_HDR_MODE:%d MTK_3A_ISP_PROFILE:%d, \
                MTK_VHDR_IMGO_3A_ISP_PROFILE:%d, MTK_REQUEST_ID:%d",
                RhdrMode, Rprofile, RIMGOprofile, RrequestID);
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateCropRegion(IMetadata*  halMetadata, IMetadata*  appMetadata,const PipelineParam &pipelineParam)
{
    MRect cropActive;
    if(! IMetadata::getEntry<MRect>(appMetadata, MTK_SCALER_CROP_REGION, cropActive) )
    {
       ACSM_ERR("Can not get MTK_SCALER_CROP_REGION !");
       return MFALSE;
    }

    if(pipelineParam.mSensorSize.h <= 0 || pipelineParam.mMaxStreamSize.h <= 0){
        ACSM_ERR("Size height <= 0. sensorSize.h(%d), maxStreamSize.h(%d)",
                    pipelineParam.mSensorSize.h, pipelineParam.mMaxStreamSize.h);
        return MFALSE;
    }
    MUINT32 vhdrMode = (pipelineParam.currentAdvSetting != NULL)
                                ? pipelineParam.currentAdvSetting->vhdrMode
                                : SENSOR_VHDR_MODE_NONE;
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define THRESHOLD 0.2
    float aspRatioDiff = abs((float)pipelineParam.mSensorSize.w / (float)pipelineParam.mSensorSize.h ,
                     (float)pipelineParam.mMaxStreamSize.w / (float)pipelineParam.mMaxStreamSize.h);
    // TODO currently only vHDR need to run this flow
    if (aspRatioDiff > THRESHOLD && vhdrMode != SENSOR_VHDR_MODE_NONE){
        // --- Update Crop Region ----
        MRect sensorCrop, sensorCropAlign, activeCropAlign;
        HwMatrix matSensor2Active;
        HwMatrix matActive2SensorRatioAlign;
        HwTransHelper hwTransHelper(mSensorIdx);
        if (!hwTransHelper.getMatrixToActive(pipelineParam.mSensorMode, matSensor2Active) ||
            !hwTransHelper.getMatrixFromActiveRatioAlign(pipelineParam.mSensorMode, matActive2SensorRatioAlign) ) {
            ACSM_ERR("get matrix fail");
            return MFALSE;
        }

        // Transfer active crop to sensor center_align crop, then transfer to active axis back.
        matActive2SensorRatioAlign.transform(cropActive, sensorCropAlign);
        matSensor2Active.transform(sensorCropAlign, activeCropAlign);

        // Calculate P1 sensor crop region (match sensor aspect ratio) according to sensorCropAlign.
        HwTransHelper::cropAlignRatioInverse(sensorCropAlign, pipelineParam.mSensorSize, sensorCrop);

        if( IMetadata::setEntry(appMetadata, MTK_SCALER_CROP_REGION, activeCropAlign) != OK)
        {
            ACSM_ERR("reset MTK_SCALER_CROP_REGION failed!");
            return MFALSE;
        }

        if( IMetadata::setEntry(halMetadata, MTK_P1NODE_SENSOR_CROP_REGION, sensorCrop) != OK)
        {
            ACSM_ERR("set MTK_P1NODE_SENSOR_CROP_REGION failed!");
            return MFALSE;
        }

        MY_LOGD_IF(mLogLevel, "aspRatioDiff(%f), maxStream(%d,%d), sensor(%d,%d),\
activeCrop(%d,%d,%dx%d),sensorCrop(%d,%d,%dx%d), activeCropAlign(%d,%d,%dx%d),\
sesnorCropAlgin(%d,%d,%dx%d),use Active2SensorFOVAlign",aspRatioDiff,
                pipelineParam.mMaxStreamSize.w, pipelineParam.mMaxStreamSize.h,
                pipelineParam.mSensorSize.w, pipelineParam.mSensorSize.h,
                cropActive.p.x, cropActive.p.y, cropActive.s.w, cropActive.s.h,
                sensorCrop.p.x, sensorCrop.p.y, sensorCrop.s.w, sensorCrop.s.h,
                activeCropAlign.p.x, activeCropAlign.p.y, activeCropAlign.s.w, activeCropAlign.s.h,
                sensorCropAlign.p.x, sensorCropAlign.p.y, sensorCropAlign.s.w, sensorCropAlign.s.h);
    }else{

        MY_LOGD_IF(mLogLevel, "aspRatioDiff(%f), maxStream(%d,%d), sensor(%d,%d),\
activeCrop(%d,%d,%dx%d), remain app crop region",aspRatioDiff,
                pipelineParam.mMaxStreamSize.w, pipelineParam.mMaxStreamSize.h,
                pipelineParam.mSensorSize.w, pipelineParam.mSensorSize.h,
                cropActive.p.x, cropActive.p.y, cropActive.s.w, cropActive.s.h);
    }
#undef THRESHOLD
#undef abs
    return MTRUE;
}
#endif

