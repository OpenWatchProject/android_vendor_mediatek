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
#define LOG_TAG "FeatureParam"
#include <utils/Log.h>
#include <utils/Errors.h>

#include <binder/Value.h>
#include <binder/Map.h>

#include <campostalgo/FeatureParam.h>

using namespace ::com::mediatek::campostalgo;
using android::binder::Value;
using android::binder::Map;



FeatureParam::FeatureParam() {
     appendHeader();
}

FeatureParam::~FeatureParam() {

}

FeatureParam::FeatureParam(const FeatureParam & param) {
    mDataInParcel.appendFrom(&(param.mDataInParcel),0,param.mDataInParcel.dataSize());
    mDataOutParcel.appendFrom(&(param.mDataOutParcel),0,param.mDataOutParcel.dataSize());
    mKeyToPosMap = param.mKeyToPosMap;
}

int32_t FeatureParam::getInt(std::string key) const {
    checkType(key, VAL_INTEGER);
    int32_t data = mDataInParcel.readInt32();
    return data;
}

void FeatureParam::getMap(std::string key,std::map<std::string, int32_t> &out) const {
    checkType(key, VAL_MAP);
    Map tmp;
    mDataInParcel.readMap(&tmp);

    std::map<std::string, Value>::iterator iter;
    for(iter = tmp.begin(); iter != tmp.end(); iter++) {
        int32_t val;
        iter->second.getInt(&val);
        out.insert(std::make_pair(iter->first, val));
    }
}

void FeatureParam::getIntArray(std::string key, int* out, int32_t length) const {
     checkType(key, VAL_INTARRAY);
     int size = mDataInParcel.readInt32();

     for(int i = 0; i < size; i++) {
         int value = mDataInParcel.readInt32();
         out[i] = value;
     }

 }

 int FeatureParam::getArrayLength(std::string key) const {
    checkType(key, VAL_INTARRAY);
    int size = mDataInParcel.readInt32();
    return size;
 }

 FeatureParam::ParamType FeatureParam::getType(std::string key) const{
     std::unordered_map<std::string, size_t>::const_iterator got = mKeyToPosMap.find(key);
     mDataInParcel.setDataPosition(got->second);
     size_t metadataType = mDataInParcel.readInt32();
     return ParamType(metadataType);

 }

  bool FeatureParam::appendInt32(std::string key, int32_t val) {
    int begin = mDataOutParcel.dataPosition();
    bool ok = true;

    // 4 int32s: size, key, type, value.
    ok = ok && mDataOutParcel.writeInt32(-1) == OK;
    ok = ok && mDataOutParcel.writeString16(android::String16(key.c_str())) == OK;
    ok = ok && mDataOutParcel.writeInt32(VAL_INTEGER) == OK;
    ok = ok && mDataOutParcel.writeInt32(val) == OK;
    if (!ok) {
        mDataOutParcel.setDataPosition(begin);
    } else {
        int end = mDataOutParcel.dataPosition();
        //set data Size
        mDataOutParcel.setDataPosition(begin);
        mDataOutParcel.writeInt32(end - begin);
        mDataOutParcel.setDataPosition(end);
    }
    return ok;
 }

 void FeatureParam::appendMap(std::string key,std::map<std::string, int32_t> in) {

    int begin = mDataOutParcel.dataPosition();
    bool ok = true;

    std::map<std::string, Value> tmpMap;
    std::map<std::string, int32_t>::iterator iter;
    for(iter = in.begin(); iter != in.end(); iter++) {
        tmpMap.insert(std::make_pair(iter->first, Value(iter->second)));
    }

      // 4 int32s: size, key, type, value.
    ok = ok && mDataOutParcel.writeInt32(-1) == OK;
    ok = ok && mDataOutParcel.writeString16(android::String16(key.c_str())) == OK;
    ok = ok && mDataOutParcel.writeInt32(VAL_MAP) == OK;
    ok = ok && mDataOutParcel.writeMap(tmpMap) == OK;
    if (!ok) {
        mDataOutParcel.setDataPosition(begin);
    } else {
        int end = mDataOutParcel.dataPosition();
        //set data Size
        mDataOutParcel.setDataPosition(begin);
        mDataOutParcel.writeInt32(end - begin);
        mDataOutParcel.setDataPosition(end);
    }
   // return ok;
}

void FeatureParam::appendIntArray(std::string key, int32_t *in, int32_t length) {
    int begin = mDataOutParcel.dataPosition();
    bool ok = true;

     // 4 int32s: size, key, type, value.
    ok = ok && mDataOutParcel.writeInt32(-1) == OK;
    ok = ok && mDataOutParcel.writeString16(android::String16(key.c_str())) == OK;
    ok = ok && mDataOutParcel.writeInt32(VAL_INTARRAY) == OK;
    ok = ok && mDataOutParcel.writeInt32Array(length, in) == OK;

    if (!ok) {
        mDataOutParcel.setDataPosition(begin);
    } else {
        int end = mDataOutParcel.dataPosition();
        //set data Size
        mDataOutParcel.setDataPosition(begin);
        mDataOutParcel.writeInt32(end - begin);
        mDataOutParcel.setDataPosition(end);
    }
}

int FeatureParam::getCount() const{
    return mKeyToPosMap.size();
}

void FeatureParam::getParamKeys(std::vector<std::string> & keys) const {
    keys.reserve(mKeyToPosMap.size());
    for(auto kv : mKeyToPosMap) {
        keys.push_back(kv.first);
    }
}

status_t FeatureParam::writeToParcel(Parcel* parcel) const {
    const size_t end = mDataOutParcel.dataPosition();
    const size_t length = end - mOutBegin;

    size_t start = parcel->dataPosition();

    parcel->appendFrom(&mDataOutParcel,0,length);

    parcel->setDataPosition(start);
    int32_t size = static_cast<int32_t>(length);
    parcel->writeInt32(size);
    parcel->setDataPosition(start + end);
    return OK;
}

status_t FeatureParam::readFromParcel(const Parcel* parcel) {
    if (parcel == nullptr) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return android::BAD_VALUE;
    }

    size_t kInt32Size = 4;
    size_t kMetaHeaderSize = 2 * kInt32Size; //  size + marker
    size_t kRecordHeaderSize = 3 * kInt32Size; // size + id + type
    size_t kMetaMarker = 0x4d455441;  // 'M' 'E' 'T' 'A'
    size_t length = parcel->dataAvail();

    if(parcel->dataAvail() < kMetaHeaderSize) {
         ALOGE("%s: Not enough data %zu", __FUNCTION__, parcel->dataAvail());
         return android::BAD_VALUE;
    }

    int pin = parcel->dataPosition();
    size_t size = parcel->readInt32();

    // The extra kInt32Size below is to account for the int32 'size' just read.
    if (parcel->dataAvail() + kInt32Size < size || size < kMetaHeaderSize) {
        ALOGE("%s: Bad size %zu, avail %zu, position %d ", __FUNCTION__, size, parcel->dataAvail(),pin);
        parcel->setDataPosition(pin);
        return android::BAD_VALUE;
    }
    int pos = parcel->dataPosition();
    // Checks if the 'M' 'E' 'T' 'A' marker is present.
    size_t kShouldBeMetaMarker = parcel->readInt32();

    if (kShouldBeMetaMarker != kMetaMarker ) {
        ALOGE("%s: Marker missing %zu", __FUNCTION__,kShouldBeMetaMarker);
        return android::BAD_VALUE;
    }
    pos = parcel->dataPosition();

    //Scan the records to collect metadata ids and offsets.
    if (scanAllRecords(parcel, pin, size - kMetaHeaderSize) != android::OK) {
        ALOGE("%s: scan all record error", __FUNCTION__);
        parcel->setDataPosition(pin);
        return android::BAD_VALUE;
    }
    mDataInParcel.appendFrom(parcel, pin, length);
    return OK;
}

// Write the header. The java layer will look for the marker.
bool FeatureParam::appendHeader() {
    bool ok = true;
    // Placeholder for the length of the metadata
     ok = ok && mDataOutParcel.writeInt32(-1) == OK;
     ok = ok && mDataOutParcel.writeInt32(kMetaMarker) == OK;
     return ok;
}

bool FeatureParam::checkKey(int key) {
     return OK;
}

void FeatureParam::checkType(const std::string key, const int expectedType) const {
     std::unordered_map<std::string, size_t>::const_iterator got = mKeyToPosMap.find(key);
     mDataInParcel.setDataPosition(got->second);
     size_t metadataType = mDataInParcel.readInt32();
}

status_t FeatureParam::scanAllRecords(const Parcel *parcel, const size_t offset, size_t bytesLeft) {
    size_t recCount = 0;
    bool error = android::OK;

    while (bytesLeft > kRecordHeaderSize) {
        size_t start = parcel->dataPosition();
        // Check the size.
        size_t size = parcel->readInt32();

        // Check the metadata key.
        android::String16 metadataId = parcel->readString16();
        android::String8 metadata8 = android::String8(metadataId);

        std::string key = metadata8.string();
        if (mKeyToPosMap.count(key)){
            ALOGE("%s: Duplicate metadata ID found: %s", __FUNCTION__, metadata8.string());
            error = android::BAD_VALUE;
            break;
        }
        std::pair<std::string, size_t> position(key, parcel->dataPosition() - offset);
        mKeyToPosMap.insert(position);

        size_t metadataType = parcel->readInt32();

        // Skip to the next one.
        parcel->setDataPosition(start + size);

        bytesLeft -= size;
        ++recCount;
    }
    if (0 != bytesLeft || error) {
        ALOGE("%s: Ran out of data or error on record %zu", __FUNCTION__, recCount);
        mKeyToPosMap.clear();
        return android::BAD_VALUE;
    }
    return android::OK;
}
