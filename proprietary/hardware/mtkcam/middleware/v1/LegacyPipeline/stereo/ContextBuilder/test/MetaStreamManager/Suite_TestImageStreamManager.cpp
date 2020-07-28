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

#include <gtest/gtest.h>
#include <iostream>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ContextBuilder.h>
//
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
//
#include "My_utility.h"
//
using namespace NSCam;
using namespace v3;
using namespace android;
using namespace NSCam::v3::Utils;
using namespace NSMtkStreamId;
//
// For image stream
static const struct ImageStreamManager::image_stream_info_pool_setting gImageStreamSetting_pool[] =
{
    // Hal
    // DepthMapNode - input
    {"Hal:Image:Pass1_FullRaw", STREAM_ID_PASS1_FULLSIZE, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(3000, 2000), 0, 0, eStreamType_IMG_HAL_POOL, NULL}, // temp run
    {"Hal:Image:Pass1_ResizeRaw", STREAM_ID_PASS1_RESIZE, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(1500, 1000), 0, 0, eStreamType_IMG_HAL_POOL, NULL}, // temp run
    {"Hal:Image:Pass2_ResizeRaw", STREAM_ID_PASS1_RESIZE_MAIN2, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(1500, 1000), 0, 0, eStreamType_IMG_HAL_POOL, NULL}, // temp run
    // DepthMapNode - output
    // APP
    {"App:Meta:DepthMap", STREAM_ID_DEPTHMAPNODE_DEPTHMAPYUV, eSTREAMTYPE_IMAGE_OUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(240, 135), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    // Hal
    {"Hal:Image:MY_S", STREAM_ID_DEPTHMAPNODE_MY_SYUV, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(240, 135), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:DMW", STREAM_ID_DEPTHMAPNODE_DMWYUV, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN, 
    eImgFmt_YV12, MSize(240, 135), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:MainImage", STREAM_ID_DEPTHMAPNODE_MAINIMAGEYUV, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(1920, 1080), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:MainImage_Cap", STREAM_ID_DEPTHMAPNODE_MAINIMAGE_CAPYUV, eSTREAMTYPE_IMAGE_INOUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(3072, 1728), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:JpsMain", STREAM_ID_DEPTHMAPNODE_JPSMAIN1YUV, eSTREAMTYPE_IMAGE_INOUT, 1, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(2176, 1152), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:JpsMain2", STREAM_ID_DEPTHMAPNODE_JPSMAIN2YUV, eSTREAMTYPE_IMAGE_INOUT, 1, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(2176, 1152), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    {"Hal:Image:FD", STREAM_ID_DEPTHMAPNODE_FDYUV, eSTREAMTYPE_IMAGE_OUT, 5, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(640, 480), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    // Bokeh - output
    {"Hal:Image:BokehResult", STREAM_ID_BOKEHNODE_RESULTYUV, eSTREAMTYPE_IMAGE_INOUT, 1, 1, 
    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE, 
    eImgFmt_YV12, MSize(3072, 1728), 0, 0, eStreamType_IMG_HAL_POOL, NULL},
    //End
    {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
};
//
TEST(PipelineContext, ImageStreamManager_ValueCheck)
{
    MY_LOGD("CKH: +");
    sp<ImageStreamManager> pImageStreamManager = ImageStreamManager::create(gImageStreamSetting_pool);
    pImageStreamManager->dump();
    MUINT32 size = pImageStreamManager->getSize();
    
    sp<IImageStreamInfo> tempImage;
    for(MUINT32 i=0;i<size;++i)
    {
        tempImage = pImageStreamManager->getStreamInfoByStreamId(gImageStreamSetting_pool[i].streamId);
        EXPECT_STREQ(gImageStreamSetting_pool[i].streamName, tempImage->getStreamName());
        EXPECT_EQ(gImageStreamSetting_pool[i].streamId, tempImage->getStreamId());
        EXPECT_EQ(gImageStreamSetting_pool[i].streamType, tempImage->getStreamType());
        EXPECT_EQ(gImageStreamSetting_pool[i].maxBufNum, tempImage->getMaxBufNum());
        EXPECT_EQ(gImageStreamSetting_pool[i].usageForAllocator, tempImage->getUsageForAllocator());
        EXPECT_EQ(gImageStreamSetting_pool[i].imgFormat, tempImage->getImgFormat());
        EXPECT_EQ(gImageStreamSetting_pool[i].bufStreamType, pImageStreamManager->getPoolTypeByStreamId(tempImage->getStreamId()));
    }
    pImageStreamManager->destroy();
    pImageStreamManager = NULL;
    MY_LOGD("CKH: release pImageStreamManager");
    Wait(20);
    MY_LOGD("CKH: -");
}
//