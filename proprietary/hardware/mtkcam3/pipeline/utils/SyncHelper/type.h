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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_TYPE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_TYPE_H_

#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>

#include <mtkcam3/pipeline/utils/SyncHelper/ISyncHelper.h>

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
enum SyncDataUpdaterType {
    E_None,
    E_Timestamp             = 1<<0,
    E_MasterCam             = 1<<1,
    E_Active                = 1<<2,
    E_PhysicalMetadata      = 1<<3,
};
/******************************************************************************
 *
 ******************************************************************************/
struct SyncData {
    int mFrameNo = -1;
    int mRequestNo = -1;
    //
    std::vector<int> mSyncTarget;
    size_t miSyncTartgetSize = 0;
    std::vector<int> mMWMasterSlaveList;
    //
    std::unordered_map<int, SyncResultInputParams*> mvInputParams;
    std::unordered_map<int, SyncResultOutputParams*> mvOutputParams;
};
/******************************************************************************
 *
 ******************************************************************************/
class SyncElement {
public:
    SyncElement() = default;
    ~SyncElement() {}
public:
    std::condition_variable mCV;
    std::mutex mConditionLock;
    std::mutex mLock;
    //
    SyncData mSyncData;
    //
    bool needWait = true;
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

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_TYPE_H_
