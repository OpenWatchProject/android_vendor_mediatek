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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef MAIN_CORE_INCLUDE_STREAMMGR_IMAGESTREAMINFO_H_
#define MAIN_CORE_INCLUDE_STREAMMGR_IMAGESTREAMINFO_H_

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <campostalgo/pipe/PipeInfo.h>
#include <gui/Surface.h>

using namespace ::com::mediatek::campostalgo;
using android::sp;

namespace NSPA {

class ImageStreamInfo: public android::RefBase {
    friend class StreamManager;
    friend class FeatureProcessor;
    friend class PipelineConfigure;
public:
    ImageStreamInfo() = delete;
    ImageStreamInfo(char const* streamName, int32_t streamType,
            int32_t streamId, int32_t width, int32_t height,
            sp<android::Surface> sb, uint32_t transform) :
            mStreamName(streamName), mStreamType(streamType), mStreamId(
                    streamId), mWidth(width), mHeight(height), mStream(sb), mTransform(
                    transform) {

    }


    /**
     * A stream name.
     *
     * @remark This should be fixed and unchangable.
     */
    char const* getStreamName() const;

    /**
     * A unique stream ID.
     *
     * @remark This should be fixed and unchangable.
     */
    int32_t getStreamId() const;

    /**
     * A stream type of StreamType.
     *
     * @remark This should be fixed and unchangable.
     */
    int32_t getStreamType() const;

    /**
     * The maximum number of buffers which may be used at the same time.
     *
     * @remark This should be fixed and unchangable.
     */
    size_t getMaxBufNum() const;

    /**
     * Set the maximum number of buffers which may be used at the same time.
     *
     * @param[in] count: the max. number of buffers.
     */
    void setMaxBufNum(size_t count);

    /*    *
     * The minimum number of buffers which is suggested to allocate initially.
     *
     * @remark This should be fixed and unchangable.

     size_t getMinInitBufNum() const;*/

    uint32_t getTransform() const;

public:
    ////                    Operations.

    /**
     * dump to a string for debug.
     */
    android::String8 toString() const;
private:
    std::string mStreamName { "unknown" };
    int32_t mStreamType { StreamType::UNKNOWN };
    int32_t mStreamId { -1 };
    int32_t mWidth { 0 };
    int32_t mHeight { 0 };
    sp<android::Surface> mStream { nullptr };
    uint32_t mTransform { 0 };
    uint32_t mDataspace { 0 };
    int32_t mAllocFormat { 0 };
    size_t mMaxBufNum { 0 };
    size_t mMinInitBufNum { 0 };

};

} /* namespace NSPA */

#endif /* MAIN_CORE_INCLUDE_STREAMMGR_IMAGESTREAMINFO_H_ */
