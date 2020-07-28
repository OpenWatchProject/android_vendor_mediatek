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
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
//
#include "My_utility.h"
//
using namespace NSCam;
using namespace v3;
using namespace android;
using namespace NSCam::v3::Utils;
using namespace NSMtkStreamId;
//
// For metadata
//
static const struct MetaStreamManager::metadata_info_setting gMetadataSetting[] =
{
    // Hal
    {"App:Meta:Control", STREAM_ID_METADATA_CONTROL_APP, eSTREAMTYPE_META_IN, 0, 0, eStreamType_META_APP},
    {"Hal:Meta:Control", STREAM_ID_METADATA_CONTROL_HAL, eSTREAMTYPE_META_IN, 0, 0, eStreamType_META_HAL},
    // DepthMapNode - input
    {"Hal:Meta:P1:Dynamic", STREAM_ID_METADATA_RESULT_P1_HAL, eSTREAMTYPE_META_INOUT, 10, 1, eStreamType_META_HAL},
    {"Hal:Meta:P1_main2:Dynamic", STREAM_ID_METADATA_CONTROL_HAL_MAIN2, eSTREAMTYPE_META_INOUT, 10, 1, eStreamType_META_HAL},
    // DepthMapNode - ouput
    {"Hal:Meta:DynamicDepth", STREAM_ID_METADATA_RESULT_DEPTH_HAL, eSTREAMTYPE_META_INOUT, 10, 1, eStreamType_META_HAL},
    // BokehNode
    {"Hal:Meta:DynamicBokeh", STREAM_ID_METADATA_RESULT_BOKEH_HAL, eSTREAMTYPE_META_INOUT, 10, 1, eStreamType_META_HAL},
    // App
    // DepthMapNode - Output
    {"App:Meta:DepthMap", STREAM_ID_DEPTHMAPNODE_DEPTHMAPYUV, eSTREAMTYPE_META_OUT, 10, 1, eStreamType_META_APP},
    // BokehNode - Ouput
    {"App:Meta:Bokeh", STREAM_ID_METADATA_RESULT_BOKEH_APP, eSTREAMTYPE_META_OUT, 10, 1, eStreamType_META_APP},
    // FD
    {"App:Meta:DynamicP1", STREAM_ID_DEPTHMAPNODE_FDYUV, eSTREAMTYPE_META_OUT, 10, 1, eStreamType_META_APP},
    // End
    {"", 0, 0, 0, 0},
};
//
TEST(PipelineContext, MetaStreamManager_ValueCheck)
{
    MY_LOGD("CKH: +");
    sp<MetaStreamManager> pMetaStreamManager = MetaStreamManager::create(gMetadataSetting);
    pMetaStreamManager->dump();
    MUINT32 size = pMetaStreamManager->getSize();
    
    sp<IMetaStreamInfo> tempMeta;
    for(MUINT32 i=0;i<size;++i)
    {
        tempMeta = pMetaStreamManager->getStreamInfoByStreamId(gMetadataSetting[i].streamId);
        EXPECT_STREQ(gMetadataSetting[i].name, tempMeta->getStreamName());
        EXPECT_EQ(gMetadataSetting[i].streamId, tempMeta->getStreamId());
        EXPECT_EQ(gMetadataSetting[i].streamType, tempMeta->getStreamType());
        EXPECT_EQ(gMetadataSetting[i].maxBufNum, tempMeta->getMaxBufNum());
        EXPECT_EQ(gMetadataSetting[i].minInitButNum, tempMeta->getMinInitBufNum());
        EXPECT_EQ(gMetadataSetting[i].streamPoolType, pMetaStreamManager->getPoolTypeByStreamId(tempMeta->getStreamId()));
    }
    pMetaStreamManager->destroy();
    pMetaStreamManager = NULL;
    MY_LOGD("CKH: release pMetaStreamManager");
    Wait(20);
    MY_LOGD("CKH: -");
}
//