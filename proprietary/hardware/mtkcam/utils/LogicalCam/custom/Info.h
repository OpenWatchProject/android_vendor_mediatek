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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_HELPER_CUSTOM_INFO_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_HELPER_CUSTOM_INFO_H_
//
#include <utils/String8.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSLogicalDeviceMetadataHelper {


/******************************************************************************
 *
 ******************************************************************************/
void
showCustInfo();


/******************************************************************************
 *  Info
 ******************************************************************************/
class Info
{
protected:  ////    Data Members.
    int32_t         mDeviceId = -1;
    bool            mBackSide = true;
    char const*     mLogicalDeviceName;

public:     ////    Instantiation.
    Info()
        : mDeviceId(0)
        , mBackSide(true)
        , mLogicalDeviceName(nullptr)
    {}
    //
    Info(
        int32_t const    uDeviceId,
        bool const       bBackSide,
        char const*      szLogicalDeviceName
    )
        : mDeviceId(uDeviceId)
        , mBackSide(bBackSide)
        , mLogicalDeviceName(szLogicalDeviceName)
    {
    }

public:     ////    Accessors.
    // for multi-cam logical device case, specific device id is meanless.
    // just check facing
    int32_t         getDeviceId()      const   { return mDeviceId; }
    bool            isBackSide()       const   { return mBackSide; }
    char const*     getSensorDrvName() const   { return mLogicalDeviceName; }
};


/******************************************************************************
 *  Static Metadata
 ******************************************************************************/
#define STATIC_METADATA_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    status_t \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata, \
        Info const&     rInfo \
    ) \
    {

#define STATIC_METADATA_END() \
        return  OK; \
    }

#define STATIC_METADATA2_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    status_t \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata\
    ) \
    {

#define STATIC_METADATA_END() \
        return  OK; \
    }


#define PREFIX_FUNCTION_STATIC_METADATA "constructCustStaticMetadata"

/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSLogicalDeviceMetadataHelper
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_HELPER_CUSTOM_INFO_H_

