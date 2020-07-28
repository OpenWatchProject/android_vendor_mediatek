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
#ifndef __AAA_UTILS_H__
#define __AAA_UTILS_H__

#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template<typename T>
inline MINT32 UPDATE_ENTRY_SINGLE(NSCam::IMetadata& metadata, MINT32 entry_tag, T value)
{
    NSCam::IMetadata::IEntry entry(entry_tag);
    entry.push_back(value, NSCam::Type2Type< T >());
    return metadata.update(entry_tag, entry);
}

template<typename T>
inline MINT32 UPDATE_ENTRY_ARRAY(NSCam::IMetadata& metadata, MINT32 entry_tag, const T* array, MUINT32 size)
{
    NSCam::IMetadata::IEntry entry(entry_tag);
    for (MUINT32 i = size; i != 0; i--)
    {
        entry.push_back(*array++, NSCam::Type2Type< T >());
    }
    return metadata.update(entry_tag, entry);
}

template<typename T>
inline MINT32 UPDATE_MEMORY(NSCam::IMetadata& metadata, MINT32 entry_tag, const T& data)
{
    NSCam::IMetadata::Memory rTmp;
    rTmp.resize(sizeof(T));
    ::memcpy(rTmp.editArray(), &data, sizeof(T));
    return UPDATE_ENTRY_SINGLE(metadata, entry_tag, rTmp);
}

template<typename T>
inline MBOOL QUERY_ENTRY_SINGLE(const NSCam::IMetadata& metadata, MINT32 entry_tag, T& value)
{
    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if ( (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG) && (!entry.isEmpty()) )
    {
        value = entry.itemAt(0, NSCam::Type2Type< T >());
        return MTRUE;
    }
    return MFALSE;
}

template<typename T>
inline MBOOL QUERY_ENTRY_SINGLE_BY_IDX(const NSCam::IMetadata& metadata, MINT32 entry_tag, T& value, MINT32 idx)
{
    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG && entry.count()>idx)
    {
        value = entry.itemAt(idx, NSCam::Type2Type< T >());
        return MTRUE;
    }
    return MFALSE;
}
template<typename T>
inline MBOOL GET_ENTRY_SINGLE_IN_ARRAY(const NSCam::IMetadata& metadata, MINT32 entry_tag, T tag)
{
	MUINT32 cnt = 0;

    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
	if( (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG) && (!entry.isEmpty()) )
	{
		cnt = entry.count();
        for (MUINT32 i = 0; i < cnt; i++)
	    {
	        if (tag == entry.itemAt(i, NSCam::Type2Type< T >()))
		    {
	            return MTRUE;
	        }
        }
    }
    return MFALSE;
}

template<typename T>
inline MBOOL GET_ENTRY_ARRAY(const NSCam::IMetadata& metadata, MINT32 entry_tag, T* array, MUINT32 size)
{
    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if ( (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG) && (entry.count() == size) )
    {
        for (MUINT32 i = 0; i < size; i++)
        {
            *array++ = entry.itemAt(i, NSCam::Type2Type< T >());
        }
        return MTRUE;
    }
    return MFALSE;
}
};
#endif //__AAA_UTILS_H__
