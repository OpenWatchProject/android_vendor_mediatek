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

#include <campostalgo/Stream.h>

using namespace ::com::mediatek::campostalgo;

Stream::Stream() :
        mSurface(nullptr), mStreamType(StreamType_t::UNKNOWN) {

}
Stream::Stream(const Stream& stream) {
    mSurface = stream.mSurface;
    mStreamType = stream.mStreamType;
}

Stream::Stream(sp<android::Surface> surface, StreamType type) :
        mSurface(surface), mStreamType(type) {

}

const sp<android::Surface> Stream::getSurface() const {
    return mSurface;
}
void Stream::setStreamType(StreamType stype) {
    mStreamType = stype;
}

StreamType Stream::getStreamType() const {
    return mStreamType;
}
;

Stream::~Stream() {
    mSurface = nullptr;
}

status_t Stream::writeToParcel(Parcel* parcel) const {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    if (mSurface.get() == nullptr) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    parcel->writeUint32(mStreamType);
    {
        status_t err = OK;
        // not sure if readParcelableArray does this, hard to tell from source
        parcel->writeString16(android::String16("android.view.Surface"));

        // Surface.writeToParcel
        android::view::Surface surfaceShim;
        surfaceShim.name = android::String16("unknown_name");
        surfaceShim.graphicBufferProducer =
                mSurface->getIGraphicBufferProducer();
        if ((err = surfaceShim.writeToParcel(parcel)) != OK) {
            ALOGE("%s: Failed to write Surface to parcel: %s (%d)",
                    __FUNCTION__, strerror(-err), err);
            return err;
        }
    }
    return android::OK;
}

status_t Stream::readFromParcel(const Parcel* parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }
    status_t err = OK;
    uint32_t type = StreamType::UNKNOWN;
    if ((err = parcel->readUint32(&type)) != OK) {
        ALOGE("%s: Failed to read surface type from parcel", __FUNCTION__);
        return err;
    }
    mStreamType = static_cast<StreamType>(type);
    size_t len;
    const char16_t* className = parcel->readString16Inplace(&len);
    ALOGI("%s: Read surface class = %s", __FUNCTION__,
            className != NULL ? android::String8(className).string() : "<null>");

    if (className == NULL) {
        return android::UNEXPECTED_NULL;
    }

    // Surface.readFromParcel
    android::view::Surface surfaceShim;
    if ((err = surfaceShim.readFromParcel(parcel)) != OK) {
        ALOGE("%s: Failed to read output target Surface from parcel: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        return err;
    }

    if (surfaceShim.graphicBufferProducer != NULL) {
        mSurface = new android::Surface(surfaceShim.graphicBufferProducer);
    }

    return android::OK;
}
