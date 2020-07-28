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
#include <binder/Parcel.h>
#include <utils/Errors.h>

#include <campostalgo/StreamInfo.h>

#include <log/Log.h>

using namespace ::com::mediatek::campostalgo;

StreamInfo::StreamInfo(int32_t format, int32_t width, int32_t height,
        int32_t orientation) :
        mWidth(width), mHeight(height), mFormat(format), mOrientation(
                orientation) {

}

StreamInfo::StreamInfo() :
        mWidth(-1), mHeight(-1), mFormat(-1), mOrientation(
                -1) {

}

int32_t StreamInfo::getFormat() {
    return mFormat;
}
int32_t StreamInfo::getWidth() {
    return mWidth;
}
int32_t StreamInfo::getHeight() {
    return mHeight;
}
int32_t StreamInfo::getOrientation() {
    return mOrientation;
}

void StreamInfo::setFormat(int32_t format) {
    mFormat = format;
}
;
void StreamInfo::setWidth(int32_t width) {
    mWidth = width;
}
;
void StreamInfo::setHeight(int32_t height) {
    mHeight = height;
}
;
void StreamInfo::setOrientation(int32_t orientation) {
    mOrientation = orientation;
}
;

StreamInfo::~StreamInfo() {
//
}

status_t StreamInfo::writeToParcel(Parcel* parcel) const {
    ///TODO: fix here!
/*    if (parcel == nullptr) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }
    parcel->writeInt32(mFormat);
    parcel->writeInt32(mWidth);
    parcel->writeInt32(mHeight);
    parcel->writeInt32(mOrientation);*/

    return android::OK;
}

status_t StreamInfo::readFromParcel(const Parcel* parcel) {
    if (parcel == nullptr) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    status_t res = android::OK;
    res = parcel->readInt32(&mFormat);
    if (res != android::OK) {
        ALOGE("%s: failed to read format type: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    res = parcel->readInt32(&mWidth);
    if (res != android::OK) {
        ALOGE("%s: failed to read width - %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    res = parcel->readInt32(&mHeight);
    if (res != android::OK) {
        ALOGE("%s: failed to read height -  %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    res = parcel->readInt32(&mOrientation);
    if (res != android::OK) {
        ALOGE("%s: failed to read orientation -  %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    return android::OK;
}

