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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPERBASE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPERBASE_H_

#include <vector>
#include <semaphore.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>

#include "mtkcam/pipeline/utils/SyncHelper/ISyncHelperBase.h"

using namespace std;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {
namespace Imp {

/******************************************************************************
 *
 ******************************************************************************/
class SyncContext {

private:
    std::vector<int>  mSyncCam;

public:
    SyncContext()
    :mStatus(status_Inited),
    mResultTimeStamp(0)
    {
         sem_init(&mSyncSem, 0, 0);
         sem_init(&mResultSem, 0, 0);
    }

    ~SyncContext()
    {
        sem_destroy(&mSyncSem);
        sem_destroy(&mResultSem);
        mStatus = status_Uninit;
    }
    sem_t             mSyncSem;
    sem_t             mResultSem;
    syncStatus        mStatus;
    int64_t           mResultTimeStamp;
};

class SyncHelperBase
    : public virtual ISyncHelperBase
{
private:
    KeyedVector<int, shared_ptr<SyncContext>> mContextMap;

    std::vector<int>  mSyncQueue;
    std::vector<int>  mResultQueue;
    mutable Mutex     mSyncQLock;
    mutable Mutex     mResultQLock;
    std::atomic<int>  mUserCounter;

public:
    SyncHelperBase();
    ~SyncHelperBase();

    status_t start(int CamId);
    status_t stop(int CamId);
    status_t init(int CamId);
    status_t uninit(int CamId);
    status_t syncEnqHW(SyncParam &sParam);
    status_t syncResultCheck(SyncParam &sParam);


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
