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

#define LOG_TAG "MtkCam/SyncHelper"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include "SyncHelper.h"
#include "TimestampSyncDataUpdater.h"
#include "MasterCamSyncDataUpdater.h"
#include "ActiveSyncDataUpdater.h"
#include "PhysicalMetaSyncDataUpdater.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P1_SYNCHELPER);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#include <utils/String8.h>
using namespace NSCam::v3::Utils::Imp;
/******************************************************************************
 *
 ******************************************************************************/
Timer::
Timer()
{
    pre_time = std::chrono::system_clock::now();
}
/******************************************************************************
 *
 ******************************************************************************/
Timer::
~Timer()
{
}
/******************************************************************************
 *
 ******************************************************************************/
std::chrono::duration<double>
Timer::
getTimeDiff()
{
    auto cur_Time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = cur_Time - pre_time;
    pre_time = cur_Time;
    return diff;
}
/******************************************************************************
 *
 ******************************************************************************/
android::sp<ISyncHelper>
ISyncHelper::
createInstance(int32_t openId)
{
    return new SyncHelper(openId);
}
/******************************************************************************
 *
 ******************************************************************************/
SyncHelper::
SyncHelper(int32_t openId)
: mOpenId(openId)
{
    mSyncQueue_EnqHw = make_unique<SyncQueue>(
                                    SyncDataUpdaterType::E_None);
    assert(!mSyncQueue_EnqHw);
    mSyncQueue_ResultCheck = make_unique<SyncQueue>(
                                    (SyncDataUpdaterType::E_Timestamp|
                                    SyncDataUpdaterType::E_MasterCam|
                                    SyncDataUpdaterType::E_Active|
                                    SyncDataUpdaterType::E_PhysicalMetadata));
    assert(!mSyncQueue_ResultCheck);
    mSyncQueueCb_ResultCheck = std::make_shared<SyncQueue_ResultCheck_Cb>(this);
    assert(!mSyncQueueCb_ResultCheck);
    mSyncQueue_ResultCheck->setCallback(mSyncQueueCb_ResultCheck);
    auto timestampSyncDataUpdater = make_unique<TimestampSyncDataUpdater>();
    assert(!timestampSyncDataUpdater);
    mvDataUpdater.emplace(
                    SyncDataUpdaterType::E_Timestamp,
                    std::move(timestampSyncDataUpdater));
    auto masterCamSyncDataUpdater = make_unique<MasterCamSyncDataUpdater>();
    assert(!masterCamSyncDataUpdater);
    mvDataUpdater.emplace(
                    SyncDataUpdaterType::E_MasterCam,
                    std::move(masterCamSyncDataUpdater));
    auto activeSyncDataUpdater = make_unique<ActiveSyncDataUpdater>();
    assert(!activeSyncDataUpdater);
    mvDataUpdater.emplace(
                    SyncDataUpdaterType::E_Active,
                    std::move(activeSyncDataUpdater));
    auto physicalMetaSyncDataUpdater = make_unique<PhysicalMetaSyncDataUpdater>(mOpenId);
    assert(!physicalMetaSyncDataUpdater);
    mvDataUpdater.emplace(
                    SyncDataUpdaterType::E_PhysicalMetadata,
                    std::move(physicalMetaSyncDataUpdater));
    mvUserList.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
SyncHelper::
~SyncHelper()
{
    MY_LOGD("release");
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
start(
    int CamId
)
{
    std::lock_guard<std::mutex> lk(mUserListLock);
    MY_LOGI("add sync instance sensor: %d", CamId);
    // reset flag.
    bFlush = false;
    auto iter = std::find(
                        mvUserList.begin(),
                        mvUserList.end(),
                        CamId);
    if(iter != mvUserList.end())
    {
        MY_LOGW("instance id(%d) already exist in mvUserList.", CamId);
    }
    else
    {
        mvUserList.push_back(CamId);
    }
    if(mSyncQueue_EnqHw != nullptr) {
        mSyncQueue_EnqHw->addUser(CamId);
    }
    if(mSyncQueue_ResultCheck != nullptr) {
        mSyncQueue_ResultCheck->addUser(CamId);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
stop(
    int CamId
)
{
    std::lock_guard<std::mutex> lk(mUserListLock);
    MY_LOGI("remove sync instance sensor: %d", CamId);
    if(mSyncQueue_ResultCheck != nullptr) {
        mSyncQueue_ResultCheck->removeUser(CamId);
    }
    if(mSyncQueue_EnqHw!=nullptr) {
        mSyncQueue_EnqHw->removeUser(CamId);
    }
    auto iter = std::find(
                        mvUserList.begin(),
                        mvUserList.end(),
                        CamId);
    if(iter != mvUserList.end())
    {
        mvUserList.erase(iter);
    }
    else
    {
        MY_LOGW("instance id(%d) not exist in mvUserList.", CamId);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
init(
    int CamId __unused
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
uninit(
    int CamId __unused
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
flush(
    int CamId
)
{
    MY_LOGI("flush (%d)", CamId);
    std::lock_guard<std::mutex> data_lk(mLock);
    bFlush = true;
    {
        std::lock_guard<std::mutex> lk(mUserListLock);
        MY_LOGI("remove sync instance sensor: %d", CamId);
        if(mSyncQueue_ResultCheck != nullptr) {
            mSyncQueue_ResultCheck->removeUser(CamId);
        }
        if(mSyncQueue_EnqHw!=nullptr) {
            mSyncQueue_EnqHw->removeUser(CamId);
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
syncEnqHW(
    SyncResultInputParams const& input
)
{
    if(isFlush()) {
        return true;
    }
    if(!mSyncQueue_EnqHw) {
        MY_LOGE("mSyncQueue_EnqHw is nullptr");
        return BAD_VALUE;
    }
    if(!input.HalControl) {
        MY_LOGE("input.HalControl is nullptr");
        return BAD_VALUE;
    }
    // if not contain MTK_FRAMESYNC_ID, return directly.
    IMetadata::IEntry entry = input.HalControl->entryFor(MTK_FRAMESYNC_ID);
    if(entry.isEmpty()) {
        return OK;
    }
    // check sync type need enq hw sync
    MINT32 syncType = 0;
    if(IMetadata::getEntry<MINT32>(input.HalControl, MTK_FRAMESYNC_TYPE, syncType))
    {
        if(!(syncType & ENQ_HW))
        {
            //MY_LOGD("[r%d] no need ignore", input.frameNum);
            return OK;
        }
    }
    // check need sync
    std::vector<int> dropFrameList;
    if(!isFlush()) {
        SyncResultOutputParams output; // empty
        auto result = mSyncQueue_EnqHw->enque(
                                "syncEnqHW",
                                input.frameNum,
                                input,
                                output,
                                dropFrameList);
        if(!result) {
            goto lbExit;
        }
        mSyncQueue_EnqHw->removeItem(input.frameNum);
    }
lbExit:
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SyncHelper::
notifyDrop(
    SyncResultInputParams const& input
)
{
    std::lock_guard<std::mutex> data_lk(mLock);
    // if not contain MTK_FRAMESYNC_ID, return directly.
    IMetadata::IEntry entry = input.HalControl->entryFor(MTK_FRAMESYNC_ID);
    if(entry.isEmpty()) {
        MY_LOGD("There is no MTK_FRAMESYNC_ID when P1 call dropframe");
        return OK;
    }
    else{
        //add the other streaming camera id into List
        std::vector<int> cameraDropList;
        android::String8 camstr;
        for (MUINT32 i = 0; i < entry.count(); i++){
            MINT32 camID = entry.itemAt(i, Type2Type<MINT32>());
            if(camID != input.CamId ){
                cameraDropList.push_back(camID);
                camstr += android::String8::format("%d,",camID);
            }
        }
        auto result = mSyncQueue_ResultCheck->addDropFrameInfo(input.frameNum, input.CamId, cameraDropList);
        if(result){
            MY_LOGD("Frame:%d Update cameraDropList[%s] to DropQueue ",input.frameNum, camstr.string());
        }
        else{
            MY_LOGD("Drop but NOT Update cameraDropList to DropQueue");
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncHelper::
syncResultCheck(
    SyncResultInputParams const& input,
    SyncResultOutputParams &output
)
{
    if(!mSyncQueue_ResultCheck) {
        MY_LOGE("mSyncQueue_ResultCheck is nullptr");
        return BAD_VALUE;
    }
    if(!input.HalControl) {
        MY_LOGE("input.HalControl is nullptr");
        return BAD_VALUE;
    }
    MY_LOGI("[f%d] +", input.frameNum);
    // check is single target
    IMetadata::IEntry entry = input.HalControl->entryFor(MTK_FRAMESYNC_ID);
    if(isFlush() || entry.isEmpty()) {
        // flush occured, generate sync data and update data.
        SyncData syncData;
        mSyncQueue_ResultCheck->genSyncData(syncData, input, output);
        auto syncQueueType = mSyncQueue_ResultCheck->getSyncDataUpdateType();
        updateData(syncQueueType, syncData);
        return true;
    }
    if(!output.HalDynamic || !output.AppDynamic) {
        MY_LOGE("output.HalDynamic(%p) or output.AppDynamic(%p) is nullptr",
                output.HalDynamic, output.AppDynamic);
        return BAD_VALUE;
    }
    // check sync type need enq hw sync
    MINT32 syncType = 0;
    if(IMetadata::getEntry<MINT32>(input.HalControl, MTK_FRAMESYNC_TYPE, syncType))
    {
        if(!(syncType & HW_RESULT_CHECK))
        {
            //MY_LOGD("[r%d] no need ignore", input.frameNum);
            return OK;
        }
    }
    std::vector<int> dropFrameList;
    if(!isFlush())
    {
        auto result = mSyncQueue_ResultCheck->enque(
                                "syncResultCheck",
                                input.frameNum,
                                input,
                                output,
                                dropFrameList);
        if(!result) {
            goto lbExit;
        }
        mSyncQueue_ResultCheck->removeItem(input.frameNum);
lbExit:
        if(output.bMasterCam)
            MY_LOGD("m_id(%d) [f%d] sync check diff = %lf", input.CamId, input.frameNum, mResultTimer.getTimeDiff().count());
    }
    else {
        SyncData syncData;
        mSyncQueue_ResultCheck->genSyncData(syncData, input, output);
        auto syncQueueType = mSyncQueue_ResultCheck->getSyncDataUpdateType();
        updateData(syncQueueType, syncData);
        if(output.bMasterCam)
            MY_LOGD("[d] m_id(%d) [f%d] sync check diff = %lf", input.CamId, input.frameNum, mResultTimer.getTimeDiff().count());
    }
    // check MTK_FRAMESYNC_RESULT value, if value is MTK_FRAMESYNC_RESULT_FAIL_DROP
    // return false. (return false, p1node will drop this frame.)
    MINT64 framesync_result = MTK_FRAMESYNC_RESULT_FAIL_CONTINUE;
    IMetadata::getEntry<MINT64>(output.HalDynamic, MTK_FRAMESYNC_RESULT, framesync_result);
    if(MTK_FRAMESYNC_RESULT_FAIL_DROP == framesync_result) {
        return false;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncHelper::
hasOtherUser(
    int CamId
)
{
    std::lock_guard<std::mutex> lk(mUserListLock);
    auto iter = std::find(
                        mvUserList.begin(),
                        mvUserList.end(),
                        CamId);
    bool ret = false;
    if(iter != mvUserList.end()) {
        // If decreasing self count and count great than 0, it should return
        // true;
        auto count = mvUserList.size() - 1;
        if(count > 0) {
            ret = true;
        }
        else {
            ret = false;
        }
    }
    else {
        if(mvUserList.size() > 0) {
            ret = true;
        }
        else {
            ret = false;
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncHelper::
updateData(
    SyncDataUpdaterType const& updater_type,
    SyncData &data
)
{
    // timestamp
    if(updater_type & SyncDataUpdaterType::E_Timestamp) {
        auto iter = mvDataUpdater.find(SyncDataUpdaterType::E_Timestamp);
        if(iter != mvDataUpdater.end()) {
            iter->second->update(data);
        }
    }
    // master cam
    if(updater_type & SyncDataUpdaterType::E_MasterCam) {
        auto iter = mvDataUpdater.find(SyncDataUpdaterType::E_MasterCam);
        if(iter != mvDataUpdater.end()) {
            iter->second->update(data);
        }
    }
    // active check
    if(updater_type & SyncDataUpdaterType::E_Active) {
        auto iter = mvDataUpdater.find(SyncDataUpdaterType::E_Active);
        if(iter != mvDataUpdater.end()) {
            iter->second->update(data);
        }
    }
    // Update  physical metadata
    if(updater_type & SyncDataUpdaterType::E_PhysicalMetadata) {
        auto iter = mvDataUpdater.find(SyncDataUpdaterType::E_PhysicalMetadata);
        if(iter != mvDataUpdater.end()) {
            iter->second->update(data);
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncHelper::
isFlush()
{
    std::lock_guard<std::mutex> data_lk(mLock);
    return bFlush;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue_ResultCheck_Cb::
onEvent(
    ISyncQueueCB::Type type,
    SyncDataUpdaterType const& updater_type,
    SyncData &data
)
{
    MY_LOGD("[r%d:f%d] type(%d) process type(%d)"
                , data.mRequestNo, data.mFrameNo, type, updater_type);
    mpSyncHelper->updateData(updater_type, data);
    return true;
}