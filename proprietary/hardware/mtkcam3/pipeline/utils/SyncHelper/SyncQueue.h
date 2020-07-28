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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNC_QUEUE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNC_QUEUE_H_

#include <chrono>
#include <unordered_map>
#include <memory>

#include "type.h"

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
class ISyncQueueCB {
public:
    enum class Type {
        SyncDone,
        Drop,
    };
public:
    ISyncQueueCB() = default;
    virtual ~ISyncQueueCB() {}
public:
    virtual bool onEvent(
                        ISyncQueueCB::Type type,
                        SyncDataUpdaterType const& updater_type,
                        SyncData &data) = 0;
};
/******************************************************************************
 *
 ******************************************************************************/
class SyncQueue {
public:
    SyncQueue(int type);
    ~SyncQueue() {};
public:
    bool addUser(int camId);
    bool removeUser(int camId);
    bool setCallback(std::weak_ptr<ISyncQueueCB> pCb);
    bool enque(
                string callerName,
                int const& frameno,
                SyncResultInputParams const& input,
                SyncResultOutputParams &output,
                std::vector<int> &dropFrameNoList);
    bool dropItem(
                int const& frameno);
    bool removeItem(
                int const& frameno);
    bool clear();
    SyncDataUpdaterType
         getSyncDataUpdateType() const;
    bool getFrameNoList(
                std::vector<int> &list);
    std::shared_ptr<SyncElement> getSyncElement(
                int const& frameno);
    bool genSyncData(
                SyncData &syncdata,
                SyncResultInputParams const& input,
                SyncResultOutputParams &output);
    bool addDropFrameInfo(
                int const frameno,
                int const camId,
                std::vector<int> &DropCameraList);
private:
    // check sync target is exist and need drop frame check.
    bool checkSyncQueue(
                SyncResultInputParams const& input,
                std::vector<int> &dropFrameNoList);
    bool sequenceCheck(
                int const& frameno,
                int const& camId,
                std::vector<int> &dropFrameNoList);
    std::vector<int>
         getSyncTartgetList(
                SyncResultInputParams const& input);
    std::vector<int>
         getMWMasterSlaveList(
                SyncResultInputParams const& input);
    bool isSyncTargetExist(SyncResultInputParams const& input);
    bool enqueDataUpdate(
                int const& frameno,
                SyncResultInputParams const& input,
                SyncResultOutputParams &output,
                std::shared_ptr<SyncElement>& syncElement);
    bool clearDropQueue(
        int const& frameno,
        int const& camId);
    bool pruneDropQueue();
private:
    int mType = SyncDataUpdaterType::E_None;
    std::mutex mLock;
    // <frame number, SyncElement>
    unordered_map<int, std::shared_ptr<SyncElement> > mvQueue;
    // <frame number, sensorid > list the frame will drop and its sensor ids
    unordered_map<int, std::vector<int> > mDropQueue;
    //
    std::weak_ptr<ISyncQueueCB> mpCb;
    //
    std::mutex mUserListLock;
    std::vector<int> mvUserList;
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

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_SYNC_QUEUE_H_
