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

#include "gtest/gtest.h"
#include "StreamingFeaturePipeUsage.h"
#include <mtkcam3/feature/eis/eis_ext.h>
#include <camera_custom_eis.h>

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_EIS22) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    EIS_MODE_ENABLE_EIS_22(hint.mEISInfo.mode);
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MTRUE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MTRUE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MTRUE,  EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0, pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(5, pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(3, pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_EIS25) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    EIS_MODE_ENABLE_EIS_25(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_GYRO(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_QUEUE(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_IMAGE(hint.mEISInfo.mode);
    hint.mEISInfo.queueSize = 25;
    hint.mEISInfo.startFrame = 20;
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MTRUE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MTRUE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode)));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  pipeUsage.supportEIS_Q());

    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0,  pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(5,  pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(25, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(3,  pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_EIS30_rgwt) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    EIS_MODE_ENABLE_EIS_30(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_GYRO(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_QUEUE(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_IMAGE(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_DEJELLO(hint.mEISInfo.mode);
    hint.mEISInfo.queueSize = 25;
    hint.mEISInfo.startFrame = 20;
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MTRUE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MTRUE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MTRUE,  pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  pipeUsage.supportEIS_Q());

    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0,  pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(5,  pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(25, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(3,  pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_EIS30_gwt) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    EIS_MODE_ENABLE_EIS_30(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_GYRO(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_QUEUE(hint.mEISInfo.mode);
    EIS_MODE_ENABLE_EIS_DEJELLO(hint.mEISInfo.mode);
    hint.mEISInfo.queueSize = 25;
    hint.mEISInfo.startFrame = 20;
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MTRUE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MTRUE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE,  pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());


    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MTRUE,  pipeUsage.supportEIS_Q());
    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0,  pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(5,  pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(25, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(3,  pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_NO_EIS) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MFALSE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE,  pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(3,  pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(0,  pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_FULL_AND_NO_EIS_NO_3DNR) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_FULL, size);
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE,  pipeUsage.supportEISNode());
    EXPECT_EQ(MFALSE,  pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE,  pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MFALSE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0,  pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(0,  pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0,  pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_P2A_PASS_THROUGH) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH, size);
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE, pipeUsage.supportEISNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MFALSE, pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MFALSE, pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0, pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_P2A_PASS_THROUGH_TIME_SHARING) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH_TIME_SHARING, size);
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE, pipeUsage.supportEISNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MTRUE,  pipeUsage.supportTimeSharing());
    EXPECT_EQ(MFALSE, pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MFALSE, pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(0, pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, USAGE_P2A_FEATURE) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_P2A_FEATURE, size);
    hint.m3DNRMode = NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE, pipeUsage.supportEISNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportWarpNode());
    EXPECT_EQ(MFALSE, pipeUsage.supportRSCNode());

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    EXPECT_EQ(MFALSE, pipeUsage.supportTimeSharing());
    EXPECT_EQ(MTRUE,  pipeUsage.supportP2AFeature());
    EXPECT_EQ(MFALSE, pipeUsage.supportBypassP2A());

    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_22_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_25_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, EIS_MODE_IS_EIS_30_ENABLED(hint.mEISInfo.mode));
    EXPECT_EQ(MFALSE, pipeUsage.supportEIS_Q());

    EXPECT_EQ(MTRUE,  pipeUsage.support3DNR());

    EXPECT_EQ(MFALSE, pipeUsage.supportVendor());
    EXPECT_EQ(MFALSE, pipeUsage.supportMultiSensor());

    EXPECT_NE(0, pipeUsage.getNumP2ATuning());
    EXPECT_EQ(3, pipeUsage.getNumP2ABuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumExtraWarpInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumWarpOutBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorInBuffer());
    EXPECT_EQ(0, pipeUsage.getNumVendorOutBuffer());
}

TEST(StreamingFeaturePipeUsage, MSize4K2K) {
    MSize size(1920, 1080);
    MUINT32 sensorIdx = 0;
    IStreamingFeaturePipe::UsageHint hint(IStreamingFeaturePipe::USAGE_P2A_FEATURE, size);
    StreamingFeaturePipeUsage pipeUsage(hint, sensorIdx);

    EXPECT_EQ(MFALSE, pipeUsage.support4K2K());

    MSize size4K2K(5120, 4060);
    IStreamingFeaturePipe::UsageHint _hint4K2K(IStreamingFeaturePipe::USAGE_P2A_FEATURE, size4K2K);
    StreamingFeaturePipeUsage hint4K2K(_hint4K2K, sensorIdx);

    EXPECT_EQ(MTRUE, hint4K2K.support4K2K());
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
