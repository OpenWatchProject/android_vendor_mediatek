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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPER_H_

#include <mutex>
//#include "ISyncHelperBase.h"
#include "mtkcam/pipeline/utils/SyncHelper/ISyncHelper.h"
#include "SyncHelperBase.h"

using namespace std;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {
namespace Imp {

/******************************************************************************
 *SyncHelper is used to convert IMetadata for SyncHelpBase
 ******************************************************************************/
class SyncHelper
      : public virtual ISyncHelper,
      public SyncHelperBase
{
private:
    template <typename T>
    inline bool
    tryGetMetadata(
        IMetadata const* pMetadata,
        MUINT32 const tag,
        T & rVal
    )
    {
        if (pMetadata == NULL) {
            return false;
        }
        //
        IMetadata::IEntry entry = pMetadata->entryFor(tag);
        if(!entry.isEmpty()) {
            rVal = entry.itemAt(0, Type2Type<T>());
            return true;
        }
        //
        return false;
    }

    template <typename T>
    inline void
    trySetMetadata(
        IMetadata* pMetadata,
        MUINT32 const tag,
        T const& val
    )
    {
       if( pMetadata == NULL ) {
            /*MY_LOGW("pMetadata == NULL");*/
            return;
        }
        //
        IMetadata::IEntry entry(tag);
        entry.push_back(val, Type2Type<T>());
        pMetadata->update(tag, entry);
   }

template <typename T>
inline uint
tryGetMetadataCount(
    IMetadata const* pMetadata,
    MUINT32 const tag
)
{
    uint count = 0;
    if (pMetadata == NULL) {
        return 0;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        count = entry.count();
    }
    //
    return count;
}

template <typename T>
inline bool
tryGetMetadataByIndex(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    MUINT const index,
    T & rVal
)
{
    if (pMetadata == NULL) {
        return false;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(index, Type2Type<T>());
        return true;
    }
    //
    return false;
}


public:

    using SyncHelperBase::start;
    using SyncHelperBase::stop;
    using SyncHelperBase::init;
    using SyncHelperBase::uninit;
    using SyncHelperBase::syncEnqHW;
    using SyncHelperBase::syncResultCheck;

    status_t syncEnqHW(int CamId,  IMetadata* HalControl);
    bool syncResultCheck(int CamId, IMetadata* HalControl, IMetadata* HalDynamic);


protected:
    //mutable Mutex  mLock;


};
/******************************************************************************
 *
 ******************************************************************************/
}
}
}
}
/******************************************************************************
 *
 ******************************************************************************/

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_UTILS_H_
