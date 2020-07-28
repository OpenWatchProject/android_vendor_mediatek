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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/******************************************************************************
*
*******************************************************************************/
//
// NOTE:
// 1. this files is only used/include by DualCameraUtility.h
//
/******************************************************************************
 *
******************************************************************************/
namespace
{
    static constexpr char* const gDualCameraUtilityTag = "MtkCam/DualCameraUtility";

    template<typename TProp, MINT32 defaultValue>
    class PropertyGetter
    {
    public:
        //
        inline MINT32 getValue() const
        {
            return mValue;
        }
        //
        inline PropertyGetter()
        : mValue(defaultValue)
        {
            mValue = property_get_int32(TProp::NAME, mValue);

            __android_log_print(ANDROID_LOG_DEBUG, gDualCameraUtilityTag, "[%s] get property value, %s: %d", __FUNCTION__, TProp::NAME, mValue);
        }
        //
    private:
        MINT32 mValue;
    };
}
/******************************************************************************
 *
******************************************************************************/
namespace NSCam
{
/******************************************************************************
 *
 ******************************************************************************/
template<typename TProp, int defaultValue>
MINT32
DualCameraUtility::
getProperty()
{
    static const PropertyGetter<TProp, defaultValue> getter;
    return getter.getValue();
}
//
template<typename T>
MVOID
DualCameraUtility::
setMetadata(IMetadata::IEntry& entry, const T& value)
{
    entry.push_back(value, Type2Type<T>());
}
//
template<typename T>
MVOID
DualCameraUtility::
setMetadataAndUpdate(IMetadata& metadata, const IMetadata::Tag_t& tag, const T& value)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(value, Type2Type<T>());
    metadata.update(tag, entry);
}
//
template<typename T>
MBOOL
DualCameraUtility::
tryGetMetadata(const IMetadata& metadata, const IMetadata::Tag_t& tag, T& out)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry = metadata.entryFor(tag);
    if(!entry.isEmpty())
    {
        out = entry.itemAt(0, Type2Type<T>());
        ret = MTRUE;
    }
    return ret;
}
//
MVOID
DualCameraUtility::
updateMetadata(IMetadata& metadata, IMetadata::IEntry& entry)
{
    metadata.update(entry.tag(), entry);
}
//
template<typename T1, typename T2>
MBOOL
DualCameraUtility::
isKeyExisting(const std::map<T1, T2>& map, const T1& key)
{
    auto iter = map.find(key);
    if(iter == map.end())
    {
        return MFALSE;
    }
    return MTRUE;
}
//
/******************************************************************************
 *
 ******************************************************************************/
} //NSCam
/******************************************************************************
 *
******************************************************************************/