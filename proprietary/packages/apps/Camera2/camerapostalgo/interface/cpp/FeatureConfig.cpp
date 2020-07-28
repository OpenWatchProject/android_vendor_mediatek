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
// #define LOG_NDEBUG 0
#define LOG_TAG "FeatureConfig"

#include <utils/String16.h>
#include <log/Log.h>
#include <campostalgo/FeatureConfig.h>

using namespace ::com::mediatek::campostalgo;

FeatureConfig::FeatureConfig() :
        mFeaturePipeConfig(nullptr) {

}

void FeatureConfig::setFeaturePipeConfig(sp<FeaturePipeConfig> config) {
    mFeaturePipeConfig = config;
}

void FeatureConfig::setSurfaces(Vector<sp<android::Surface>>& vSurfaces) {
    mAppSurfaces = vSurfaces;
}

void FeatureConfig::setStreamInfo(Vector<sp<StreamInfo>>& vStreamInfo) {
    mStreamInfos = vStreamInfo;
}

void FeatureConfig::addSurface(sp<android::Surface>& surface) {
    mAppSurfaces.push_back(surface);
}

void FeatureConfig::addStreamInfo(sp<StreamInfo>& streamInfo) {
    mStreamInfos.push_back(streamInfo);
}

const Vector<sp<StreamInfo>> & FeatureConfig::getStreamInfos() const {
    return mStreamInfos;
}

const Vector<sp<android::Surface>>& FeatureConfig::getSurfaceList() const {
    return mAppSurfaces;
}

const sp<FeatureParam> FeatureConfig::getInterfaceParams() const {
    return mInterfaceParams;
}

FeatureConfig::~FeatureConfig() {

}

const sp<FeaturePipeConfig>& FeatureConfig::getFeaturePipeConfig() const {
    return mFeaturePipeConfig;
}

status_t FeatureConfig::writeToParcel(Parcel* parcel) const {
    ///TODO: fix here!
/*
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    status_t err = OK;

    int32_t size = static_cast<int32_t>(mAppSurfaces.size());

    // write app surface
    parcel->writeInt32(size);

    for (int32_t i = 0; i < size; ++i) {
        // not sure if readParcelableArray does this, hard to tell from source
        parcel->writeString16(android::String16("android.view.Surface"));

        // Surface.writeToParcel
        android::view::Surface surfaceShim;
        surfaceShim.name = android::String16("unknown_name");
        surfaceShim.graphicBufferProducer =
                mAppSurfaces[i]->getIGraphicBufferProducer();
        if ((err = surfaceShim.writeToParcel(parcel)) != OK) {
            ALOGE(
                    "%s: Failed to write output target Surface %d to parcel: %s (%d)",
                    __FUNCTION__, i, strerror(-err), err);
            return err;
        }
    }

    //write StreamInfos
*/

    return OK;
}

status_t FeatureConfig::readFromParcel(const Parcel* parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    status_t err = OK;
    int32_t size = 0;
    size_t len = 0;
    mAppSurfaces.clear();
    mStreamInfos.clear();
    mFeaturePipeConfig = new FeaturePipeConfig;

    //1. read pipe config
    const char16_t* className = parcel->readString16Inplace(&len);
    if ((err = mFeaturePipeConfig->readFromParcel(parcel)) != OK) {
        ALOGE("%s: Failed to read FeaturePipeConfig from parcel: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        return err;
    }
    //2. read App Surfaces
    if ((err = parcel->readInt32(&size)) != OK) {
        ALOGE("%s: Failed to read surface list size from parcel: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        return err;
    }

    for (int32_t i = 0; i < size; ++i) {
        const char16_t* className = parcel->readString16Inplace(&len);

        if (className == NULL) {
            continue;
        }

        // Surface.writeToParcel
        android::view::Surface surfaceShim;
        if ((err = surfaceShim.readFromParcel(parcel)) != OK) {
            ALOGE(
                    "%s: Failed to read output target Surface %d from parcel: %s (%d)",
                    __FUNCTION__, i, strerror(-err), err);
            return err;
        }

        sp<android::Surface> surface;
        if (surfaceShim.graphicBufferProducer != NULL) {
            surface = new android::Surface(surfaceShim.graphicBufferProducer);
        }

        mAppSurfaces.push_back(surface);
    }
    //3. read Stream Infos
    if ((err = parcel->readInt32(&size)) != OK) {
        ALOGE("%s: Failed to read stream infos size from parcel: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        return err;
    }
    for (int32_t i = 0; i < size; i++) {
        const char16_t* className = parcel->readString16Inplace(&len);
        if (className == NULL) {
            continue;
        }
        sp<StreamInfo> streamInfo = new StreamInfo;
        if ((err = streamInfo->readFromParcel(parcel)) != OK) {
            ALOGE("%s: Failed to read stream infos %d from parcel: %s (%d)",
                    __FUNCTION__, i, strerror(-err), err);
            return err;
        }
        mStreamInfos.push_back(streamInfo);
    }

    //4. interface feature params
    mInterfaceParams = new FeatureParam;
    const char16_t* FPClassName = parcel->readString16Inplace(&len);
    if (FPClassName == nullptr || len <= 0) {
        ALOGE("Failed to read FeatureParam class name");
    } else if ((err = mInterfaceParams->readFromParcel(parcel)) != OK) {
        ALOGE("%s: Failed to read FeatureParam from parcel: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        return err;
    }

    return OK;
}

