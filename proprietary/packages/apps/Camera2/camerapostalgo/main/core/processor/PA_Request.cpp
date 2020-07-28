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

#include <processor/PA_Request.h>
#include <mtk/mtk_feature_type.h>

using ::NSCam::NSPipelinePlugin::eFeatureIndexMtk;

namespace NSPA {

PA_Request::PA_Request(int32_t stream_type, uint32_t request_id,
        sp<android::BufferItemConsumer> consumer, const sp<android::Surface> sf,
        uint32_t jpegOrientation) :
        mConsumer(consumer), mStreamType(stream_type), mHalStream(sf), mJpegOrientation(
                jpegOrientation), mRequestNo(request_id), mFrameCount(1), mFrameIndex(
                0), mCombinedFeatureSet(eFeatureIndexMtk::NO_FEATURE_NORMAL) {

}


PA_Request::~PA_Request() {
    mImg.clear();
    mConsumer=nullptr;
    ALOGV("PA_Request destroyed!");
}

uint32_t PA_Request::getRequestNo() {
    return mRequestNo;
}

int64_t PA_Request::getCombinedFeatureSet() {
    return mCombinedFeatureSet;
}

void PA_Request::setCombinedFeatureSet(int64_t value) {
    mCombinedFeatureSet = value;
}

uint32_t PA_Request::getFrameCount() {
    return mFrameCount;
}

void PA_Request::setFrameCount(uint32_t request_count) {
    mFrameCount = request_count;
}

uint32_t PA_Request::getFrameIndex() {
    return mFrameIndex;
}

void PA_Request::setFrameIndex(uint32_t idx) {
    mFrameIndex = idx;
}

sp<android::Surface> PA_Request::getHalStream() {
    return mHalStream;
}

MBOOL PA_Request::addImg(ID_IMG id, const sp<ImageBufferInfo>& infoPtr) {
    if (id <= NO_IMG || id >= ID_IMG_INVALID) {
        return MFALSE;
    }
    auto it = mImg.find(id);
    if (it == mImg.end()) {
        mImg[id] = infoPtr;
        return MTRUE;
    }
    ALOGW("Image info %d already existed!", id);
    return MFALSE;
}

MBOOL PA_Request::isValidImg(ID_IMG id) const
{
    if (id <= NO_IMG|| id >= ID_IMG_INVALID) {
        return MFALSE;
    }
    auto it = mImg.find(id);
    if (it != mImg.end()) {
        return MTRUE;
    } else {
        return MFALSE;
    }
}

sp<IImageBuffer> PA_Request::getImg(ID_IMG id) {
    auto it = mImg.find(id);
    return (it != mImg.end()) ? it->second->mImg : sp<IImageBuffer>();
}

sp<ImageBufferInfo> PA_Request::getImgInfo(ID_IMG id) {
    auto it = mImg.find(id);
    return (it != mImg.end()) ? it->second : sp<ImageBufferInfo>();
}

MVOID PA_Request::releaseImageWithLock(ID_IMG id) {
    ALOGV("PA_Request releaseImageWithLock %d, 0x%p!", id, mImg[id].get());
    Mutex::Autolock _l(mLock);
//    if ( id == OUT_YUV )
//    {
//        mImgOutArray.clear();
//    }
//    else
    if (id == IN_FULL || id == IN_FULL_YUV)
    {
        mImg[id] = NULL;
        mImg.erase(id);
    }

}

MBOOL PA_Request::isValidMeta(ID_META id) const {
    if (id <= NO_META || id >= ID_META_MAX) {
        return MFALSE;
    }
    auto it = mMeta.find(id);
    if (it == mMeta.end()) {
        return MFALSE;
    }
    return MTRUE;
}

MBOOL PA_Request::addMetadata(ID_META id, const sp<MetadataPack>& metadataPtr) {
    if (id <= NO_META || id >= ID_META_MAX) {
        return MFALSE;
    }
    auto it = mMeta.find(id);
    if (it == mMeta.end()) {
        mMeta[id] = metadataPtr;
        return MTRUE;
    }
    ALOGW("meta info %d already existed!", id);
    return MFALSE;
}

sp<MetadataPack> PA_Request::getMetadataPack(ID_META id) const {
    auto it = mMeta.find(id);
    return (it != mMeta.end()) ? it->second : sp<MetadataPack>();
}

MVOID PA_Request::releaseMetaWithLock(ID_META id) {
    TRACE_S_FUNC_ENTER(mLog);
    ALOGV("PA_Request releaseMetaWithLock %d, 0x%p!", id, mMeta[id].get());
    Mutex::Autolock _l(mLock);
    if (id == IN_PAS) {
        mMeta[id] = NULL;
        mMeta.erase(id);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

int32_t PA_Request::getStreamType() {
    return mStreamType;
}

const sp<android::BufferItemConsumer>& PA_Request::getHalConsumer() const {
    return mConsumer;
}

MVOID PA_Request::setIsNeedWorkingBuffer(MBOOL isNeed) {
    ALOGV("setIsNeedWorkingBuffer isNeed  = %d", isNeed);
    mIsNeedWorkingBuffer = isNeed;
}

MBOOL PA_Request::isNeedWorkingBuffer() {
    return mIsNeedWorkingBuffer;
}

} /* namespace NSPA */
