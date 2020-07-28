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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_FEATURE_DUAL_FEATURE_COMMON_H_
#define _MTK_CAMERA_FEATURE_DUAL_FEATURE_COMMON_H_

#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

#include <chrono>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam
{
namespace v3
{
namespace DualFeature
{
class process_frame : virtual public android::RefBase
{
public:
    process_frame() = delete;
    process_frame(
            android::sp<NSCam::v3::IPipelineFrame> const& pFrame,
            MINT32 openId,
            MUINT32 iRequestId,
            MUINT32 nodeId,
            MINT32 logLevel);
    virtual ~process_frame();
public:
    MVOID uninit(MBOOL vaild, const char* nodeName);
    MVOID dumpInfo();
    android::sp<NSCam::v3::IPipelineFrame> getFrame();
private:
    MVOID unlockAllStream(const char* nodeName);
    MVOID markStreamStatus(MBOOL vaild);
private:
    android::sp<NSCam::v3::IPipelineFrame> mpFrame = nullptr;
public:
    MINT32 mOpenId = -1;
    MUINT32 mRequestId = 0;
    MUINT32 mNodeId = 0;
    MINT32 mLogLevel = 0;
    // input metadata buffer
    android::DefaultKeyedVector<StreamId_T, IMetadata*>
                                mvInMetaBufferContainer;
    android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamBuffer> >
                                mvInMetaStreamBuffer;
    // output metadata buffer
    android::DefaultKeyedVector<StreamId_T, IMetadata*>
                                mvOutMetaBufferContainer;
    android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamBuffer> >
                                mvOutMetaStreamBuffer;
    // input image buffer
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageBuffer> >
                                mvInImgBufContainer;
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamBuffer> >
                                mvInImgStreamBuffer;
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageBuffer> >
                                mvOutImgBufContainer;
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamBuffer> >
                                mvOutImgStreamBuffer;
    // store time to get processing time
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
};
};
};
};
#endif // _MTK_CAMERA_FEATURE_DUAL_FEATURE_COMMON_H_