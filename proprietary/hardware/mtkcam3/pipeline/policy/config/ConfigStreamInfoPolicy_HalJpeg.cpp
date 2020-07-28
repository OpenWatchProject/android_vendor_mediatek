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

#define LOG_TAG "mtkcam-ConfigStreamInfoPolicy-HalJpeg"

#include <mtkcam3/pipeline/policy/IConfigStreamInfoPolicy.h>
//
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
//
#include "MyUtils.h"


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
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
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
        case eImgFmt_JPEG:
            //A height of 1, and width equal to their size in bytes.
            //The size must be large enough. (w x h x 2 bytes/pixel, by default).
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes , 1, (imgSize.w << 1)*imgSize.h);
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


// HAL Jpeg version - decorator design pattern
FunctionType_Configuration_StreamInfo_NonP1
makePolicy_Configuration_StreamInfo_NonP1_HalJpeg(FunctionType_Configuration_StreamInfo_NonP1 f)
{
    auto p = [](
        FunctionType_Configuration_StreamInfo_NonP1 fDecoratedPolicy,
        Configuration_StreamInfo_NonP1_Params const& params
    ) -> int {
        auto pOut = params.pOut;
        auto pPipelineNodesNeed = params.pPipelineNodesNeed;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        auto const& pParsedAppImageStreamInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;

        auto err = fDecoratedPolicy(params);

        // Need HAL Jpeg stream info if needJpegNode==true && no App Jpeg
        if ( pPipelineNodesNeed->needJpegNode
          && pParsedAppImageStreamInfo->pAppImage_Jpeg == nullptr )
        {
            MY_LOGI("Configuring stream info: Hal:Image:Jpeg");

            /**
             * The size should be the max yuv size (i.e. pParsedAppImageStreamInfo->vAppImage_Output_Proc)
             * Here we just use pParsedAppImageStreamInfo->maxImageSize for the simplicity.
             */
            MSize const& size = pParsedAppImageStreamInfo->maxImageSize;
            MINT const format = eImgFmt_JPEG;
            MUINT const usage = 0;
            pOut->pHalImage_Jpeg =
                    createImageStreamInfo(
                            "Hal:Image:Jpeg",
                            eSTREAMID_IMAGE_PIPE_JPEG,
                            eSTREAMTYPE_IMAGE_OUT,
                            1, 0,
                            usage, format, size
                        );

        }

        return err;
    };

    MY_LOGA_IF(f==nullptr, "decoraated policy == nullptr");
    return std::bind(p, f, std::placeholders::_1);
}

};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


