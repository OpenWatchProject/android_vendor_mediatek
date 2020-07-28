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

#include <campostalgo/FeaturePipeConfig.h>

using namespace ::com::mediatek::campostalgo;

#define LOG_TAG "FeaturePipeConfig"

FeaturePipeConfig::FeaturePipeConfig() :
        mStreamType(StreamType::UNKNOWN) {

}
FeaturePipeConfig::FeaturePipeConfig(StreamType type) :
        mStreamType(type) {

}

FeaturePipeConfig& FeaturePipeConfig::operator=(
        FeaturePipeConfig const & src) {
    this->mStreamType = src.mStreamType;
    this->mStreamAlgos = src.mStreamAlgos;
    return *this;
}

void FeaturePipeConfig::setStreamType(StreamType& streamType) {
    mStreamType = (int32_t)streamType;
}

StreamType FeaturePipeConfig::getStreamType() {
    return static_cast<StreamType>(mStreamType);
}

FeaturePipeConfig::~FeaturePipeConfig() {

}

void FeaturePipeConfig::setStreamAlgos(AlgoType* types, size_t len) {
    if (types == nullptr) {
        ALOGE("%s: invalid stream types", __FUNCTION__);
        return;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(len); i++) {
        mStreamAlgos.push_back(types[i]);
    }
}

std::vector<AlgoType>& FeaturePipeConfig::getStreamAlgos() {
    return mStreamAlgos;
}

status_t FeaturePipeConfig::writeToParcel(Parcel* parcel) const {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }
    parcel->writeInt32(mStreamType);
    int32_t size = static_cast<int32_t>(mStreamAlgos.size());
    parcel->writeInt32(size);
    for (int type : mStreamAlgos) {
        parcel->writeInt32(type);
    }

    return OK;
}

status_t FeaturePipeConfig::readFromParcel(const Parcel* parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }
    status_t res = android::OK;
    res = parcel->readInt32(&mStreamType);
    if (res != android::OK) {
        ALOGE("%s: Failed to read stream type to parcel: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    int size = parcel->readInt32();
    int32_t type = StreamType::UNKNOWN;
    for (int i = 0; i < size; i++) {
        res = parcel->readInt32(&type);
        if (res != android::OK) {
            ALOGE("%s: Failed to read algo type %d to parcel: %s(%d)",
                    __FUNCTION__, i, strerror(-res), res);
            return res;
        }
        mStreamAlgos.push_back((AlgoType)type);
    }
    return OK;
}
