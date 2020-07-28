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

#ifndef MAIN_CORE_PROCESSOR_PA_REQUEST_H_
#define MAIN_CORE_PROCESSOR_PA_REQUEST_H_

#include <utils/Vector.h>
#include <ui/GraphicBuffer.h>

#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/BufferItemConsumer.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <gui/BufferItemConsumer.h>
#include <campostalgo/FeatureConfig.h>
#include <utils/imgbuf/IImageBuffer.h>
#include <utils/metadata/IMetadata.h>
#include "header_base.h"
#include "PA_Param.h"
#include <utils/def/BuiltinTypes.h>
#include <utils/metadata/mtk_metadata_types.h>

using android::Vector;
using android::GraphicBuffer;
using android::Mutex;

using namespace NSCam;

using ::com::mediatek::campostalgo::NSFeaturePipe::IMetadata;

namespace NSPA {
class ImageBufferInfo;
class MetadataPack;

class PA_Request : virtual public android::RefBase {
public:
    PA_Request(int32_t stream_type, uint32_t request_id,
            sp<android::BufferItemConsumer> consumer, const sp<android::Surface> sf, uint32_t jpegOrientation = 0);
    virtual ~PA_Request();

public:
    uint32_t getRequestNo();
    int32_t getStreamType();
    sp<android::Surface> getHalStream();

    /// based on FID_XXX
    int64_t getCombinedFeatureSet();
    void setCombinedFeatureSet(int64_t value);
    uint32_t getFrameCount();
    void setFrameCount(uint32_t request_count);
    uint32_t getFrameIndex();
    void setFrameIndex(uint32_t idx);
    android::BufferItem getBufferItem() {return mBI;};

    MBOOL isValidImg(ID_IMG id) const;
    sp<IImageBuffer> getImg(ID_IMG id);
    sp<ImageBufferInfo> getImgInfo(ID_IMG id);
    const sp<android::BufferItemConsumer>& getHalConsumer() const;
    MBOOL addImg(ID_IMG id, const sp<ImageBufferInfo>& info);
    MVOID releaseImageWithLock(ID_IMG id);

    MBOOL isValidMeta(ID_META id) const;
    sp<MetadataPack> getMetadataPack(ID_META id) const;
    MBOOL addMetadata(ID_META id, const sp<MetadataPack>& metadataPtr);
    MVOID releaseMetaWithLock(ID_META id);
    MVOID setIsNeedWorkingBuffer(MBOOL isNeed);
    MBOOL isNeedWorkingBuffer();

protected:
    std::unordered_map<ID_IMG, sp<ImageBufferInfo>> mImg;
    std::unordered_map<ID_META, sp<MetadataPack>> mMeta;
    std::unordered_map<uint32_t,bool> mMultiFrameWorkingBufferStatus;
    sp<android::BufferItemConsumer> mConsumer = nullptr;
    int32_t mStreamType;
    sp<android::Surface> mHalStream;
    uint32_t mJpegOrientation { 0 };
    uint32_t mRequestNo;
    uint32_t mFrameCount = 1;
    uint32_t mFrameIndex = 0;
    int64_t mCombinedFeatureSet;
    mutable Mutex mLock;
    android::BufferItem mBI;
    MBOOL mIsNeedWorkingBuffer = MTRUE;

    friend class HalBufferListener;
    friend class CaptureRequestCallback;
    friend class StreamManager;
    friend class ResultHandler;
    friend class FeatureProcessor;

};

class ImageBufferInfo: public virtual RefBase {
public:
    ImageBufferInfo(const sp<android::Surface>& sf, const sp<GraphicBuffer> gbf,
            const sp<IImageBuffer>& img, int32_t jpegOrientation = 0, int fencefd = -1) :
            mSurface(sf), mGbf(gbf), mImg(img), mJpegOrientation(jpegOrientation),fenceFd(fencefd) {
        LOG_ALWAYS_FATAL_IF(img == nullptr, "ImageBufferInfo error!");
    }
/*    virtual ~ImageBufferInfo() {
        mImg = nullptr;
        mSurface = nullptr;
    }*/
    sp<android::Surface> mSurface;
    sp<GraphicBuffer> mGbf;
    sp<IImageBuffer> mImg;
    int32_t mJpegOrientation;
    int fenceFd;

};

class MetadataPack: public virtual RefBase {
public:

    MetadataPack() {
    }

    MetadataPack(const MetadataPack& meta) :
            mMetadata(meta.mMetadata) {
    }

    MetadataPack(const IMetadata& meta) :
            mMetadata(meta) {
    }

    android::status_t update(IMetadata::Tag_t tag, IMetadata::IEntry const& entry) {
        if (mMetadata.entryFor(tag).count()== 0) {
            return mMetadata.update(tag, entry);
        } else {
            IMetadata::IEntry ie = mMetadata.takeEntryFor(tag);
            MINT32 type = entry.type();

            switch(type) {
#define CASE_TYPE(_type_)                                         \
            case TYPE_##_type_ : {                                    \
            auto tt = Type2Type<_type_>(); \
            ie.replaceItemAt(0, entry.itemAt(0, tt), tt); \
                break; \
            }

            CASE_TYPE(MUINT8);
            CASE_TYPE(MINT32);
            CASE_TYPE(MFLOAT);
            CASE_TYPE(MINT64);
            CASE_TYPE(MDOUBLE);
            CASE_TYPE(MRational);
            CASE_TYPE(MPoint);
            CASE_TYPE(MSize);
            CASE_TYPE(MRect);
            CASE_TYPE(IMetadata);
//            CASE_TYPE(Memory);
#undef CASE_TYPE
            case TYPE_Memory:
                auto tt = Type2Type<IMetadata::Memory>();
                ie.replaceItemAt(0, entry.itemAt(0, tt), tt);
                break;
            }
            return mMetadata.update(tag, ie);
        }
    }

    IMetadata::IEntry entryFor(IMetadata::Tag_t tag) const {
        return mMetadata.entryFor(tag);
    }

    MetadataPack& operator=(const IMetadata& meta) {
        mMetadata = meta;
        return *this;
    }

    MetadataPack& operator+=(MetadataPack const& other){
        this->mMetadata+=other.mMetadata;
        return *this;
    }

    virtual ~MetadataPack() {
        mMetadata.clear();
    }

    IMetadata* getMetaPtr() {
        return &mMetadata;
    }

protected:
    IMetadata mMetadata;
};
} /* namespace NSPA */

#endif /* MAIN_CORE_PROCESSOR_PA_REQUEST_H_ */
