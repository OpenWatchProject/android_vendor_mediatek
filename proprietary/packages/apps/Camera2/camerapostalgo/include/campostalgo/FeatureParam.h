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

#ifndef INCLUDE_CAMPOSTALGO_FEATUREPARAM_H_
#define INCLUDE_CAMPOSTALGO_FEATUREPARAM_H_
#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <utils/RefBase.h>
#include <unordered_map>

using android::OK;
using android::status_t;
using android::Parcel;
using android::Parcelable;
using android::RefBase;

namespace com {
namespace mediatek {
namespace campostalgo {
class FeatureParam: virtual public RefBase, public Parcelable {
public:

    FeatureParam();
    FeatureParam(const FeatureParam & param);
    FeatureParam& operator=(const FeatureParam& param) = delete;
    virtual ~FeatureParam();
    virtual status_t writeToParcel(Parcel* parcel) const override;
    virtual status_t readFromParcel(const Parcel* parcel) override;

    int getInt(std::string key) const;
    void getMap(std::string key,std::map<std::string, int32_t> &out) const;
    void getIntArray(std::string key, int *out, int32_t length) const;

    bool appendInt32(std::string key, int32_t);
    void appendMap(std::string key,std::map<std::string, int32_t> out);
    void appendIntArray(std::string key, int32_t *in, int32_t length);

protected:
    enum ParamType {
        VAL_NULL = -1,
        VAL_STRING = 0,
        VAL_INTEGER = 1,
        VAL_MAP = 2,
        VAL_BUNDLE = 3,
        VAL_PARCELABLE = 4,
        VAL_SHORT = 5,
        VAL_LONG = 6,
        VAL_DOUBLE = 8,
        VAL_BOOLEAN = 9,
        VAL_BYTEARRAY = 13,
        VAL_STRINGARRAY = 14,
        VAL_IBINDER = 15,
        VAL_INTARRAY = 18,
        VAL_LONGARRAY = 19,
        VAL_BYTE = 20,
        VAL_SERIALIZABLE = 21,
        VAL_BOOLEANARRAY = 23,
        VAL_PERSISTABLEBUNDLE = 25,
        VAL_DOUBLEARRAY = 28,
    };

    int getCount() const;
    void getParamKeys(std::vector<std::string> & keys) const;
    int getArrayLength(std::string key) const;
    ParamType getType(std::string key) const;
    friend class ParamConvert;

private:
    typedef int32_t Type;
    // Append the size and 'META' marker.
    bool appendHeader();
    // Once all the records have been added, call this to update the
    // lenght field in the header.
    void updateLength();
    // Checks the key is valid and not already present.
    bool checkKey(Type key);

    status_t scanAllRecords(const Parcel *parcel, const size_t offset, size_t bytesLeft);
    void checkType(const std::string key, const int expectedType) const;

    Parcel mDataInParcel;
    Parcel mDataOutParcel;
    int32_t mInBegin =0;
    int32_t mOutBegin =0;
    std::unordered_map<std::string, size_t> mKeyToPosMap;

    const size_t kRecordHeaderSize = 3 * sizeof(int32_t);
    const size_t kMetaMarker = 0x4d455441;  // 'M' 'E' 'T' 'A'

};
}
}
}

#endif /* INCLUDE_CAMPOSTALGO_FEATUREPARAM_H_ */
