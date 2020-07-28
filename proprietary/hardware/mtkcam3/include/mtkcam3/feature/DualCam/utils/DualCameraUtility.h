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

#ifndef _MTK_HARDWARE_MTKCAM_DUALCAM_DUALCAMERAUTILITY
#define _MTK_HARDWARE_MTKCAM_DUALCAM_DUALCAMERAUTILITY

// Standard C header file
#include <map>

// Android system/core header file
#include <android/log.h>
#include <cutils/properties.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/IMetadata.h>

// Module header file

// Local header file

/******************************************************************************
*
*******************************************************************************/
#define PROPERTY_TYPE(TYPE, PROPNAME)               \
class TYPE final                                    \
{                                                   \
public:                                             \
    TYPE() = delete;                                \
public:                                             \
    static constexpr char * const NAME = PROPNAME;  \
}                                                   \
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam
{
/******************************************************************************
 *
 ******************************************************************************/
class DualCameraUtility final
{
public:
    DualCameraUtility() = delete;

public:
    template<typename TProp, MINT32 defaultValue = 0>
    static inline MINT32 getProperty();

public:
    template<typename T>
    static inline MVOID setMetadata(IMetadata::IEntry& entry, const T& value);
    //
    template<typename T>
    static inline MVOID setMetadataAndUpdate(IMetadata& metadata, const IMetadata::Tag_t& tag, const T& value);
    //
    template<typename T>
    static inline MBOOL tryGetMetadata(const IMetadata& metadata, const IMetadata::Tag_t& tag, T& out);
    //
    template<typename T1, typename T2>
    static inline MBOOL isKeyExisting(const std::map<T1, T2>& map, const T1& key);

 public:
    //
    static inline MVOID updateMetadata(IMetadata& metadata, IMetadata::IEntry& entry);
};
/******************************************************************************
 *
 ******************************************************************************/
}// NSCam

#include "DualCameraUtility_Imp.h"

#endif