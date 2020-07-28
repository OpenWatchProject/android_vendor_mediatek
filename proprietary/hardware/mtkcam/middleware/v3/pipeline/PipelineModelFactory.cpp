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

#define LOG_TAG "MtkCam/HwPipeline"
//
#include "MyUtils.h"
#include "PipelineModelFactory.h"
#include "PipelineModel_Default.h"
#include "PipelineModel_Hdr.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/

typedef ConfigurePipeline::Params              ConfigParamsT;
typedef ConfigurePipeline::ImageStreamsT       ImageStreamsT;
typedef ConfigurePipeline::MetaStreamsT        MetaStreamsT;
typedef ConfigurePipeline::MinFrameDurationT   MinFrameDurationT;
typedef ConfigurePipeline::StallFrameDurationT StallFrameDurationT;

#ifdef MTKCAM_HAVE_HDR
#define ENABLE_HDR          (1)
#else
#define ENABLE_HDR          (0)
#endif

/******************************************************************************
 *
 ******************************************************************************/
static
android::sp<IPipelineModel>
configurePipelineModel_Default(ConfigParamsT& rCfgParams)
{
    typedef PipelineModel_Default PipelineT;

    PipeConfigParams appStreams;

    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId, rCfgParams.pAppCallback);
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a DEFAULT pipeline model");
        return NULL;
    }
    //
    appStreams.mOperation_mode = rCfgParams.operation_mode;
    //
    MetaStreamsT const&  vMetaStreams  = rCfgParams.vMetaStreams;
    ImageStreamsT const& vImageStreams = rCfgParams.vImageStreams;
    MinFrameDurationT const& vMinFrameDuration = rCfgParams.vMinFrameDuration;
    StallFrameDurationT const& vStallFrameDuration = rCfgParams.vStallFrameDuration;
    appStreams.pMeta_Control = vMetaStreams[0];
    //
    MY_LOGD("WillDBG6 vImageStreams size : %d", vImageStreams.size());
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
            case eImgFmt_RAW16:
                appStreams.pImage_Raw = pStreamInfo;
                appStreams.mImage_Raw_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                appStreams.mImage_Jpeg_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                Duration duration;
                duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                //
                MY_LOGD("WillDBG6 format : %X, type : %d", pStreamInfo->getImgFormat(), pStreamInfo->getStreamType());
                if(pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN) {
                    appStreams.pImage_Yuv_In = pStreamInfo;
                    appStreams.mImage_Yuv_In_Duration = duration;
                }
                else if(pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT) {
                    appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                    appStreams.vImage_Yuv_Duration.push_back(duration);
                }
                break;
                //
            case eImgFmt_CAMERA_OPAQUE:
                if(pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN) {
                    appStreams.pImage_Opaque_In = pStreamInfo;
                    appStreams.mImage_Opaque_In_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                }
                else if(pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT) {
                    appStreams.pImage_Opaque_Out = pStreamInfo;
                    appStreams.mImage_Opaque_Out_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                }
                break;
                //
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
    //
#if 1
    MERROR err = pPipelineModel->configure(appStreams, rCfgParams.pPipelineModel);
#else
    // Must need to release resource
    rCfgParams.pPipelineModel = NULL;
    MERROR err = pPipelineModel->configure(appStreams);
#endif
    //
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    //
    return pPipelineModel;
}

#if ENABLE_HDR
/******************************************************************************
 *
 ******************************************************************************/
static
android::sp<IPipelineModel>
configurePipelineModel_Hdr(ConfigParamsT& rCfgParams)
{
#if 0
    typedef PipelineModel_Hdr PipelineT;

    PipelineT::PipeConfigParams appStreams;

    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId, rCfgParams.pAppCallback);
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a HDR pipeline model");
        return NULL;
    }
    //
    appStreams.mOperation_mode = rCfgParams.operation_mode;
    //
    MetaStreamsT const&  vMetaStreams  = rCfgParams.vMetaStreams;
    ImageStreamsT const& vImageStreams = rCfgParams.vImageStreams;
    MinFrameDurationT const& vMinFrameDuration = rCfgParams.vMinFrameDuration;
    StallFrameDurationT const& vStallFrameDuration = rCfgParams.vStallFrameDuration;
    appStreams.pMeta_Control = vMetaStreams[0];
    //
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
                appStreams.pImage_Raw = pStreamInfo;
                appStreams.mImage_Raw_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                appStreams.mImage_Jpeg_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                PipelineT::Duration duration;
                duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                appStreams.vImage_Yuv_Duration.push_back(duration);
                break;
                //
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
#if 1
    MERROR err = pPipelineModel->configure(appStreams, rCfgParams.pPipelineModel);
#else
    // Must need to release resource
    rCfgParams.pPipelineModel = NULL;
    MERROR err = pPipelineModel->configure(appStreams);
#endif
    //
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    //
    return pPipelineModel;
#endif
    return NULL;
}
#endif


#if 0
/******************************************************************************
 *
 ******************************************************************************/
static
android::sp<IPipelineModel>
configurePipelineModel_Feature(ConfigParamsT& rCfgParams)
{
    typedef PipelineModel_Feature PipelineT;

    PipelineT::PipeConfigParams appStreams;

    rCfgParams.pPipelineModel = NULL;
    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId);;
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a pipeline model");
        return NULL;
    }
    //
    MetaStreamsT const&  vMetaStreams  = rCfgParams.vMetaStreams;
    ImageStreamsT const& vImageStreams = rCfgParams.vImageStreams;
    MinFrameDurationT const& vMinFrameDuration = rCfgParams.vMinFrameDuration;
    StallFrameDurationT const& vStallFrameDuration = rCfgParams.vStallFrameDuration;
    appStreams.pMeta_Control = vMetaStreams[0];
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
            case eImgFmt_RAW16:
                appStreams.pImage_Raw = pStreamInfo;
                appStreams.mImage_Raw_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                appStreams.mImage_Jpeg_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                PipelineT::Duration duration;
                duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                appStreams.vImage_Yuv_Duration.push_back(duration);
                break;
                //
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
    //
    err |= pPipelineModel->configure(appStreams);
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    //
    return pPipelineModel;
}

/******************************************************************************
 *
 ******************************************************************************/
static
android::sp<IPipelineModel>
configurePipelineModel_Stereo(ConfigParamsT& rCfgParams)
{
    typedef PipelineModel_Stereo PipelineT;

    PipelineT::PipeConfigParams appStreams;

    rCfgParams.pPipelineModel = NULL;
    sp<PipelineT> pPipelineModel = PipelineT::create(rCfgParams.openId);;
    if  ( pPipelineModel == 0 ) {
        MY_LOGE("Fail to new a pipeline model");
        return NULL;
    }
    //
    MetaStreamsT const&  vMetaStreams  = rCfgParams.vMetaStreams;
    ImageStreamsT const& vImageStreams = rCfgParams.vImageStreams;
    MinFrameDurationT const& vMinFrameDuration = rCfgParams.vMinFrameDuration;
    StallFrameDurationT const& vStallFrameDuration = rCfgParams.vStallFrameDuration;
    appStreams.pMeta_Control = vMetaStreams[0];
    for (size_t i = 0; i < vImageStreams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vImageStreams[i].get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
            case eImgFmt_BAYER14:
            case eImgFmt_RAW16:
                appStreams.pImage_Raw = pStreamInfo;
                appStreams.mImage_Raw_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_BLOB:
                appStreams.pImage_Jpeg_Stall = pStreamInfo;
                appStreams.mImage_Jpeg_Duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                appStreams.vImage_Yuv_NonStall.push_back(pStreamInfo);
                PipelineT::Duration duration;
                duration.set(vMinFrameDuration[i], vStallFrameDuration[i]);
                appStreams.vImage_Yuv_Duration.push_back(duration);
                break;
                //
            default:
                MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                break;
            }
        }
    }
    //
    err |= pPipelineModel->configure(appStreams);
    if  ( OK != err ) {
        MY_LOGE("pPipelineModel->configure err:%d", err);
        pPipelineModel = NULL;
        return NULL;
    }
    //
    return pPipelineModel;
}
#endif


/******************************************************************************
 *
 ******************************************************************************/
MVOID
dump(ConfigParamsT& rParam)
{
    MetaStreamsT const&  vMetaStreams  = rParam.vMetaStreams;
    ImageStreamsT const& vImageStreams = rParam.vImageStreams;
    //
    MY_LOGD(" openId(%d) PipelineModel(%p) scene(%d)",
            rParam.openId, rParam.pPipelineModel.get(),
            rParam.pipelineScene);
    //
    for ( size_t i=0; i<vImageStreams.size(); i++ ) {
        sp<IImageStreamInfo> pStreamInfo = vImageStreams.valueAt(i);
        MY_LOGD("ImageStream(%zu/%zu) stream %#" PRIx64 "(%s), format:0x%x size %dx%d",
                i, vImageStreams.size(),
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(),
                pStreamInfo->getImgFormat(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                );
    }
    for ( size_t i=0; i<vMetaStreams.size(); i++ ) {
        sp<IMetaStreamInfo> pStreamInfo = vMetaStreams.valueAt(i);
        MY_LOGD("ImageStream(%zu/%zu) stream %#" PRIx64 "(%s)",
                i, vMetaStreams.size(),
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName()
                );
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ConfigurePipeline::
operator()(ConfigParamsT& rParams)
{
    MY_LOGD_IF(1, "enter %d", rParams.pipelineScene);
    if(rParams.pipelineScene == ePIPELINEMODEL_DEFAULT) {
        rParams.pPipelineModel = configurePipelineModel_Default(rParams);
    }
    else if(rParams.pipelineScene == ePIPELINEMODEL_HDR) {
#if 0//ENABLE_HDR
        rParams.pPipelineModel = configurePipelineModel_Hdr(rParams);
#else
        rParams.pPipelineModel = configurePipelineModel_Default(rParams);
#endif
    }
#if 0
    else if(rParams.pipelineScene == ePIPELINEMODEL_FEATURE) {
        rParams.pPipelineModel = configurePipelineModel_Feature(rParams);
    }
    else if(rParams.pipelineScene == ePIPELINEMODEL_STEREO){
        rParams.pPipelineModel = configurePipelineModel_Stereo(rParams);
    }
#endif
    //
    if  ( rParams.pPipelineModel == 0 ) {
        MY_LOGE("failed");
        dump(rParams);
        return -ENODEV;
    }
    return OK;
}

