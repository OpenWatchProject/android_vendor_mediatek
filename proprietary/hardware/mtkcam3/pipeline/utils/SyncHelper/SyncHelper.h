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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPER_H_

#include <utils/String8.h>
#include <mtkcam3/pipeline/utils/SyncHelper/ISyncHelper.h>
#include <chrono>

#include "SyncQueue.h"
#include "ISyncDataUpdater.h"

using namespace std;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {
namespace Imp {
class SyncQueue_ResultCheck_Cb;
/******************************************************************************
 *
 ******************************************************************************/
class Timer
{
public:
    Timer();
    ~Timer();
public:
    std::chrono::duration<double> getTimeDiff();
private:
    std::chrono::time_point<std::chrono::system_clock> pre_time;
};
/******************************************************************************
 *
 ******************************************************************************/
class SyncHelper
      : public virtual ISyncHelper
{
public:
    SyncHelper(int32_t openId);
    virtual ~SyncHelper();
public:
    status_t start(int CamId) override;
    status_t stop(int CamId) override;
    status_t init(int CamId) override;
    status_t uninit(int CamId) override;
    status_t flush(int CamId) override;
    status_t syncEnqHW(
                        SyncResultInputParams const& input) override;
    status_t notifyDrop(
                    SyncResultInputParams const& input) override;
    bool syncResultCheck(
                        SyncResultInputParams const& input,
                        SyncResultOutputParams &output) override;
    bool hasOtherUser(int CamId) override;
public:
    bool updateData(
                        SyncDataUpdaterType const& updater_type,
                        SyncData &data);
private:
    bool isFlush();
public:
    std::unordered_map<SyncDataUpdaterType, std::unique_ptr<ISyncDataUpdater>>
                                mvDataUpdater;
private:
    std::mutex mLock;
    bool bFlush = false;
    std::unique_ptr<SyncQueue>  mSyncQueue_EnqHw;
    std::unique_ptr<SyncQueue>  mSyncQueue_ResultCheck;
    std::shared_ptr<SyncQueue_ResultCheck_Cb>
                                mSyncQueueCb_ResultCheck = nullptr;
    Timer mResultTimer;
    std::mutex mUserListLock;
    std::vector<int> mvUserList;
    const int32_t mOpenId;
};
/******************************************************************************
 *
 ******************************************************************************/
class SyncQueue_ResultCheck_Cb : public virtual ISyncQueueCB
{
public:
    SyncQueue_ResultCheck_Cb(SyncHelper* p) : mpSyncHelper(p)
    {}
    virtual ~SyncQueue_ResultCheck_Cb() {}
public:
    bool onEvent(
                ISyncQueueCB::Type type,
                SyncDataUpdaterType const& updater_type,
                SyncData &data) override;
private:
    SyncHelper* mpSyncHelper = nullptr;
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

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNCHELPER_SYNCHELPER_H_
