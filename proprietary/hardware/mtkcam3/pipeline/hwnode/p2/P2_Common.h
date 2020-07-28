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

#ifndef _MTKCAM_HWNODE_P2_COMMON_H_
#define _MTKCAM_HWNODE_P2_COMMON_H_

#include "P2_Header.h"
#include "P2_Logger.h"
using namespace android;

namespace P2
{

#define INVALID_P1_ISO_VAL (-99999)

template<typename T>
MBOOL tryGet(const IMetadata &meta, MUINT32 tag, T &val)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry = meta.entryFor(tag);
    if( !entry.isEmpty() )
    {
        val = entry.itemAt(0, Type2Type<T>());
        ret = MTRUE;
    }
    return ret;
}

template<typename T>
MBOOL tryGet(const IMetadata *meta, MUINT32 tag, T &val)
{
    return (meta != NULL) ? tryGet<T>(*meta, tag, val) : MFALSE;
}

template<typename T>
MBOOL trySet(IMetadata &meta, MUINT32 tag, const T &val)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    ret = (meta.update(tag, entry) == OK);
    return ret;
}

template<typename T>
MBOOL trySet(IMetadata *meta, MUINT32 tag, const T &val)
{
    return (meta != NULL) ? trySet<T>(*meta, tag, val) : MFALSE;
}

template<typename T>
MVOID getVector(const IMetadata &meta, MUINT32 tag, std::vector<T> &vector)
{
    IMetadata::IEntry entry = meta.entryFor(tag);
    MUINT32 count = entry.count();
    vector.resize(count);
    for( MUINT32 i = 0; i < count; ++i )
    {
        vector[i] = entry.itemAt(i, Type2Type<T>());
    }
}

template<typename T>
MVOID getVector(const IMetadata *meta, MUINT32 tag, std::vector<T> &vector)
{
    if( meta != NULL )
    {
        getVector<T>(*meta, tag, vector);
    }
}

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_COMMON_H_
