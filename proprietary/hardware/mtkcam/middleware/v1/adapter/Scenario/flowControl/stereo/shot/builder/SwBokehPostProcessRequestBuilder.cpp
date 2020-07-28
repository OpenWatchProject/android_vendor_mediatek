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

#define LOG_TAG "SwBokehPostProcessRequestBuilder"

#include "SwBokehPostProcessRequestBuilder.h"
// mtkcam
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
// size related
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
// metadata tag
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
//
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include "../../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
SwBokehPostProcessRequestBuilder::
~SwBokehPostProcessRequestBuilder()
{
    MY_LOGD("dctor(%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehPostProcessRequestBuilder::
init(
    PostProcRequestInitSetting &setting
)
{
    MY_LOGD("+");
    status_t ret = UNKNOWN_ERROR;
    miRequestNo         = setting.iRequestNo;
    mShotParam          = setting.shotParam;
    mJpegParam          = setting.jpegParam;
    mpILegacyPipeline   = setting.pILegacyPipeline;
    mAppMetadata        = setting.appMetadata;
    mHalMetadata        = setting.halMetadata;
    miTargetPipelineId  = setting.iTargetPipelineId;
    mpCallbackImageBufferManager  = new CallbackImageBufferManager(miTargetPipelineId, setting.pImageStreamManager);
    mpCallbackBufferHandler       = new BufferCallbackHandler(miTargetPipelineId);
    mpCallbackBufferHandler->setImageCallback(setting.pImageCallback);

    auto createImageStreamInfo = [](
                char const* streamName,
                StreamId_T streamId,
                MUINT32 streamType,
                size_t maxBufNum,
                size_t minInitBufNum,
                MUINT usageForAllocator,
                MINT imgFormat,
                MSize const& imgSize,
                MUINT32 transform)
    {
        IImageStreamInfo::BufPlanes_t bufPlanes;
    #define addBufPlane(planes, height, stride)                                      \
            do{                                                                      \
                size_t _height = (size_t)(height);                                   \
                size_t _stride = (size_t)(stride);                                   \
                IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
                planes.push_back(bufPlane);                                          \
            }while(0)
        switch( imgFormat ) {
            case eImgFmt_YV12:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                break;
            case eImgFmt_NV21:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
                break;
            case eImgFmt_RAW16:
            case eImgFmt_YUY2:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
                break;
            case eImgFmt_Y8:
            case eImgFmt_STA_BYTE:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                break;
            case eImgFmt_RGBA8888:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
                break;
            case eImgFmt_BLOB:
                /*
                add 328448 for image size
                standard exif: 1280 bytes
                4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
                max thumbnail size: 64K bytes
                */
                addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 15 / 10) + 328448); //328448 = 64K+1280+65408*4
                break;
            default:
                MY_LOGE("format not support yet %p", imgFormat);
                break;
        }
    #undef  addBufPlane

        sp<ImageStreamInfo>
            pStreamInfo = new ImageStreamInfo(
                    streamName,
                    streamId,
                    streamType,
                    maxBufNum, minInitBufNum,
                    usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                    );

        if( pStreamInfo == NULL ) {
            MY_LOGE("create ImageStream failed, %s, %#" PRIxPTR ,
                    streamName, streamId);
        }

        return pStreamInfo;
    };
    // sw bokeh provider
    sp<IImageStreamInfo> pAppExtraDataStreamInfo = nullptr;
    sp<IImageStreamInfo> bokehResultStreamInfo = nullptr;
    sp<IImageStreamInfo> bokehCleanImageStreamInfo = nullptr;
    sp<IImageStreamInfo> thumbImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pDepthMapWrapper = nullptr;
    // jpeg - bokeh
    sp<IImageStreamInfo> pSwBokehJpegStreamInfo = nullptr;
    // jpeg - clean image
    sp<IImageStreamInfo> pCleanJpegStreamInfo = nullptr;
    MSize depthMapWrapper = setting.depthMapWrapper;
    MUINT32 extraDataSize = setting.extraDataSize;
    MSize finalPictureSize = MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);
    MSize finalThumbnailSize = MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    if(mShotParam.mu4Transform & eTransform_ROT_90)
    {
        finalPictureSize   = MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth);
    }
    MY_LOGD("depWrap:%dx%d extra:%d", depthMapWrapper.w, depthMapWrapper.h,extraDataSize);
    MY_LOGD("final capture size: picture:%dx%d", finalPictureSize.w, finalPictureSize.h);
    // pAppExtraDataStreamInfo
    {
        pAppExtraDataStreamInfo = createImageStreamInfo(
                                        (std::string("App:Image:ExtraData")+std::to_string(miRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        MSize(extraDataSize, 1),
                                        0);
    }
    // pDepthMapWrapper
    {
        pDepthMapWrapper = createImageStreamInfo(
                                        (std::string("App:Image:DepthWrapper")+std::to_string(miRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        depthMapWrapper,
                                        0);
    }
    // thumbnail
    {
        thumbImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:Bokeh_Thumb")+std::to_string(miRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_YV12,
                                              finalThumbnailSize,
                                              0);
    }
    // bokehResult
    {
        bokehResultStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:BokehResult")+std::to_string(miRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              mShotParam.mu4Transform);
    }

    // bokehClean
    {
        bokehCleanImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:CleanImage")+std::to_string(miRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              mShotParam.mu4Transform);
    }
    // jpgBokeh
    {
        pSwBokehJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_Bokeh")+std::to_string(miRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_Bokeh,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // jpgClean
    {
        pCleanJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_Clean")+std::to_string(miRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // update dst stream
    auto addDstStreams = [this](MINT32 streamId, MBOOL bNeedProvider)
    {
        mvDstStreams.push_back(
            BufferSet{
                .streamId       = streamId,
                .criticalBuffer = bNeedProvider,
            }
        );
    };
    // metadata
    addDstStreams(eSTREAMID_META_HAL_DYNAMIC_BOKEH, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_BOKEH, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_BOKEH_JPG, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_JPEG, MFALSE);
    // image
    addDstStreams(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_DBG, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_JPG_Bokeh, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg, MTRUE);

    // allocate buffer, update ImageStreamManager with new Pool & Provider
    ret = mpCallbackImageBufferManager->allocBuffer(pAppExtraDataStreamInfo,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pDepthMapWrapper,      mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pSwBokehJpegStreamInfo,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pCleanJpegStreamInfo,  mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(bokehResultStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(bokehCleanImageStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(thumbImageStreamInfo);

    mpCallbackImageBufferManager->setTimestamp(miRequestNo, setting.timestamp);

lbExit:
    MY_LOGD("-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehPostProcessRequestBuilder::
doPostProc()
{
    status_t ret = UNKNOWN_ERROR;
    MY_LOGD("+");
    // check init value first.
    // if init value not set, return error.
    std::vector< std::shared_ptr<BufferData> > vBufferDataList;
    if(mpCallbackBufferHandler == nullptr)
    {
        MY_LOGW("BufferCallbackHandler is uninit.");
        goto lbExit;
    }

    // send to legacy pipeline
    {
        sp<ILegacyPipeline> pPipeline = mpILegacyPipeline.promote();

        if(pPipeline!= nullptr)
        {
            if(mvDstStreams.size() == 0)
            {
                MY_LOGW("vDstStreams is empty");
            }

            pPipeline->submitRequest(miRequestNo, mAppMetadata, mHalMetadata, mvDstStreams);
        }
    }
    ret = OK;
lbExit:
    MY_LOGD("-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehPostProcessRequestBuilder::
destroy()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
SwBokehPostProcessRequestBuilder::
onLastStrongRef(
    const void*
)
{
    MY_LOGD("+");
    MY_LOGD("-");
}
