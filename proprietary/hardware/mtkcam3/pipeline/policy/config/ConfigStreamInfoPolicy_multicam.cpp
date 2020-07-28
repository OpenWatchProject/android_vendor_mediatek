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

#define LOG_TAG "mtkcam-ConfigStreamInfoPolicy_Multicam"

#include <mtkcam3/pipeline/policy/IConfigStreamInfoPolicy.h>

#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"
#include "P1StreamInfoBuilder.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

/******************************************************************************
 *
 ******************************************************************************/
static
sp<IImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(idx, planes, height, stride)                                 \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            planes[idx] = { _height * _stride, _stride };                        \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
            bufPlanes.count = 3;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w);
            addBufPlane(1, bufPlanes.planes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(2, bufPlanes.planes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            bufPlanes.count = 2;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w);
            addBufPlane(1, bufPlanes.planes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w << 1);
            break;
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
#undef  addBufPlane

    android::sp<IImageStreamInfo> pStreamInfo =
                ImageStreamInfoBuilder()
                    .setStreamName(streamName)
                    .setStreamId(streamId)
                    .setStreamType(streamType)
                    .setMaxBufNum(maxBufNum)
                    .setMinInitBufNum(minInitBufNum)
                    .setUsageForAllocator(usageForAllocator)
                    .setAllocImgFormat(imgFormat)
                    .setAllocBufPlanes(bufPlanes)
                    .setImgSize(imgSize)
                    .setImgFormat(imgFormat)
                    .setBufPlanes(bufPlanes)
                    .setBufCount(1)
                    .setStartOffset(0)
                    .setBufStep(0)
                    .build();

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
static
MVOID
evaluatePreviewSize(
    PipelineUserConfiguration const* pPipelineUserConfiguration,
    MSize &rSize
)
{
    sp<IImageStreamInfo> pStreamInfo;
    int consumer_usage = 0;
    int allocate_usage = 0;
    int prevwidth      = 0;
    int prevheight     = 0;

    for( const auto& n : pPipelineUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_Proc )
    {
        if  ( (pStreamInfo = n.second) != 0 ) {
            consumer_usage = pStreamInfo->getUsageForConsumer();
            allocate_usage = pStreamInfo->getUsageForAllocator();
            MY_LOGD("consumer : %X, allocate : %X", consumer_usage, allocate_usage);
            if(consumer_usage & GRALLOC_USAGE_HW_TEXTURE) {
                prevwidth = pStreamInfo->getImgSize().w;
                prevheight = pStreamInfo->getImgSize().h;
                break;
            }
            if(consumer_usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) {
                continue;
            }
            prevwidth = pStreamInfo->getImgSize().w;
            prevheight = pStreamInfo->getImgSize().h;
        }
    }
    if(prevwidth == 0 || prevheight == 0)
    {
        MY_LOGI("Cannot find preview stream, set fd yuv size = 640x480");
        rSize.w = 640;
        rSize.h = 480;
        return ;
    }
    if (prevheight <= prevwidth)
        rSize.h = prevheight * rSize.w / prevwidth;
    else
        rSize.w = prevwidth * rSize.h / prevheight;

    MY_LOGD("evaluate preview size : %dx%d", prevwidth, prevheight);
    MY_LOGD("FD buffer size : %dx%d", rSize.w, rSize.h);
}

/**
 * multi cam version
 */
FunctionType_Configuration_StreamInfo_NonP1 makePolicy_Configuration_StreamInfo_NonP1_Multicam()
{
    return [](Configuration_StreamInfo_NonP1_Params const& params) -> int {
        auto pOut = params.pOut;
        auto pPipelineNodesNeed = params.pPipelineNodesNeed;
        auto pCaptureFeatureSetting = params.pCaptureFeatureSetting;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        auto const& pParsedAppImageStreamInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;
        auto const& vPhysicCameras = pPipelineUserConfiguration->vPhysicCameras;

        pOut->pAppMeta_Control = pPipelineUserConfiguration->vMetaStreams.begin()->second;

        if (pPipelineNodesNeed->needP2StreamNode)
        {
            pOut->pAppMeta_DynamicP2StreamNode =
                    new MetaStreamInfo(
                        "App:Meta:DynamicP2",
                        eSTREAMID_META_APP_DYNAMIC_02,
                        eSTREAMTYPE_META_OUT,
                        10, 1
                        );
            pOut->pHalMeta_DynamicP2StreamNode =
                    new MetaStreamInfo(
                        "Hal:Meta:P2:Dynamic",
                        eSTREAMID_META_PIPE_DYNAMIC_02,
                        eSTREAMTYPE_META_INOUT,
                        10, 1
                        );
            auto getPhysicalStreamId_P2StreamDynamic = [](int32_t idx) {
                int64_t streamId = eSTREAMID_END_OF_PIPE;
                switch(idx)
                {
                    case 0:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_MAIN1;
                        break;
                    case 1:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_MAIN2;
                        break;
                    case 2:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_MAIN3;
                        break;
                }
                return streamId;
            };
            // create main1, main2 dynamic meta stream info
            for(size_t i = 0; i < vPhysicCameras.size(); i++)
            {
                int32_t idx = -1;
                for(size_t j=0;j<pPipelineStaticInfo->sensorId.size();j++)
                {
                    if(vPhysicCameras[i] == pPipelineStaticInfo->sensorId[j])
                    {
                        idx = j;
                        break;
                    }
                }
                if(idx != -1)
                {
                    std::string name = "App:Meta:DynamicP2_Cam"+std::to_string(idx);
                    pOut->vAppMeta_DynamicP2StreamNode_Physical.emplace(
                        vPhysicCameras[i],
                        new MetaStreamInfo(
                            name.c_str(),
                            getPhysicalStreamId_P2StreamDynamic(idx),
                            eSTREAMTYPE_META_OUT,
                            10, 1)
                        );
                }
                else
                {
                    MY_LOGA("not support, please check flow");
                }
            }
        }
        if (pPipelineNodesNeed->needP2CaptureNode)
        {
            pOut->pAppMeta_DynamicP2CaptureNode =
                    new MetaStreamInfo(
                        "App:Meta:DynamicP2Cap",
                        eSTREAMID_META_APP_DYNAMIC_02_CAP,
                        eSTREAMTYPE_META_OUT,
                        10, 1
                        );
            pOut->pHalMeta_DynamicP2CaptureNode =
                    new MetaStreamInfo(
                        "Hal:Meta:P2C:Dynamic",
                        eSTREAMID_META_PIPE_DYNAMIC_02_CAP,
                        eSTREAMTYPE_META_INOUT,
                        10, 1
                        );
            auto getPhysicalStreamId_P2CaptureDynamic = [](int32_t idx) {
                int64_t streamId = eSTREAMID_END_OF_PIPE;
                switch(idx)
                {
                    case 0:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN1;
                        break;
                    case 1:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN2;
                        break;
                    case 2:
                        streamId = eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN3;
                        break;
                }
                return streamId;
            };
            // create main1, main2 dynamic meta stream info
            for(size_t i = 0; i < vPhysicCameras.size(); i++)
            {
                int32_t idx = -1;
                for(size_t j=0;j<pPipelineStaticInfo->sensorId.size();j++)
                {
                    if(vPhysicCameras[i] == pPipelineStaticInfo->sensorId[j])
                    {
                        idx = j;
                        break;
                    }
                }
                if(idx != -1)
                {
                    std::string name = "App:Meta:DynamicP2Cap_Cam"+std::to_string(idx);
                    pOut->vAppMeta_DynamicP2CaptureNode_Physical.emplace(
                        vPhysicCameras[i],
                        new MetaStreamInfo(
                            name.c_str(),
                            getPhysicalStreamId_P2CaptureDynamic(idx),
                            eSTREAMTYPE_META_OUT,
                            10, 1)
                        );
                }
                else
                {
                    MY_LOGA("not support, please check flow");
                }
            }
        }
        if (pPipelineNodesNeed->needFDNode)
        {
            pOut->pAppMeta_DynamicFD =
                    new MetaStreamInfo(
                        "App:Meta:FD",
                        eSTREAMID_META_APP_DYNAMIC_FD,
                        eSTREAMTYPE_META_OUT,
                        10, 1
                        );
            // FD YUV
            //MSize const size(640, 480); //FIXME: hard-code here?
            MSize size(640, 640);
            // evaluate preview size
            evaluatePreviewSize(pPipelineUserConfiguration, size);

            MY_LOGD("evaluate FD buffer size : %dx%d", size.w, size.h);

            MINT const format = eImgFmt_YUY2;//eImgFmt_YV12;
            MUINT const usage = 0;

            pOut->pHalImage_FD_YUV =
            createImageStreamInfo(
                    "Hal:Image:FD",
                    eSTREAMID_IMAGE_FD,
                    eSTREAMTYPE_IMAGE_INOUT,
                    5, 1,
                    usage, format, size
                );
        }
        if (pPipelineNodesNeed->needJpegNode && pParsedAppImageStreamInfo->pAppImage_Jpeg != nullptr)
        {
            pOut->pAppMeta_DynamicJpeg =
                    new MetaStreamInfo(
                        "App:Meta:Jpeg",
                        eSTREAMID_META_APP_DYNAMIC_JPEG,
                        eSTREAMTYPE_META_OUT,
                        10, 1
                        );

            uint32_t const maxJpegNum = ( pCaptureFeatureSetting )?
                                        pCaptureFeatureSetting->maxAppJpegStreamNum : 1;
            // Jpeg YUV
            {
                MSize const& size = pParsedAppImageStreamInfo->pAppImage_Jpeg->getImgSize();
                MINT const format = eImgFmt_YUY2;
                MUINT const usage = 0;
                pOut->pHalImage_Jpeg_YUV =
                        createImageStreamInfo(
                                "Hal:Image:YuvJpeg",
                                eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,
                                eSTREAMTYPE_IMAGE_INOUT,
                                maxJpegNum, 0,
                                usage, format, size
                            );
            }
            // Thumbnail YUV
            {
                MSize const size(-1L, -1L); //unknown now
                MINT const format = eImgFmt_YUY2;
                MUINT const usage = 0;
                pOut->pHalImage_Thumbnail_YUV =
                    createImageStreamInfo(
                            "Hal:Image:YuvThumbnail",
                            eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,
                            eSTREAMTYPE_IMAGE_INOUT,
                            maxJpegNum, 0,
                            usage, format, size
                        );
            }
        }

        if (pPipelineNodesNeed->needPDENode)
        {
            pOut->pHalMeta_DynamicPDE =
                new MetaStreamInfo(
                        "Hal:Meta:PDE:Dynamic",
                        eSTREAMID_META_PIPE_DYNAMIC_PDE,
                        eSTREAMTYPE_META_INOUT,
                        10, 1
                        );
        }

        if (pPipelineNodesNeed->needRaw16Node)
        {
            if(pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() > 0)
            {
                for(auto &elm : pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical)
                {
                    int index = pPipelineStaticInfo->getIndexFromSensorId(elm.first);
                    auto name = (index == 0) ? "App:Meta:Raw16:Dynamic_Main1" : "App:Meta:Raw16:Dynamic_Main2";
                    auto streamID = (index == 0) ? eSTREAMID_META_APP_DYNAMIC_RAW16_MAIN1 : eSTREAMID_META_APP_DYNAMIC_RAW16_MAIN2;
                    pOut->vAppMeta_DynamicRAW16_Physical.emplace(elm.first,
                                                                 new MetaStreamInfo(
                                                                     name,
                                                                     streamID,
                                                                     eSTREAMTYPE_META_INOUT,
                                                                     10, 1
                                                                 ));
                }
            }
            else
            {
                pOut->pAppMeta_DynamicRAW16 =
                    new MetaStreamInfo(
                            "App:Meta:Raw16:Dynamic",
                            eSTREAMID_META_APP_DYNAMIC_RAW16,
                            eSTREAMTYPE_META_INOUT,
                            10, 1
                            );
            }
        }

        return OK;
    };
}

};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

