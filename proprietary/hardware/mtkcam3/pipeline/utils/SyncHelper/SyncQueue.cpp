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

#define LOG_TAG "MtkCam/SyncQueue"
//
#include <assert.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include "SyncQueue.h"

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

using namespace NSCam::v3::Utils::Imp;
/******************************************************************************
 *
 ******************************************************************************/
SyncQueue::
SyncQueue(
    int type
) : mType(type)
{
    MY_LOGD("2020/01/14 - create type (%d)", mType);
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
addUser(
    int camId
)
{
    std::lock_guard<std::mutex> lk(mLock);
    auto iter = std::find(
                            mvUserList.begin(),
                            mvUserList.end(),
                            camId);
    if(iter != mvUserList.end()) {
        MY_LOGE("Please check flow, id(%d) already exist", camId);
    }
    else {
        MY_LOGI("user(%d)", camId);
        mvUserList.push_back(camId);
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
removeUser(
    int camId
)
{
    std::lock_guard<std::mutex> lk(mLock);
    auto iter = std::find(
                            mvUserList.begin(),
                            mvUserList.end(),
                            camId);
    if(iter != mvUserList.end()) {
        MY_LOGI("user(%d)", camId);
        mvUserList.erase(iter);
    }
    clear();
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
setCallback(
    std::weak_ptr<ISyncQueueCB> pCb
)
{
    if(auto p = pCb.lock()) {
        mpCb = pCb;
    }
    else {
        MY_LOGE("set callback fail");
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
sequenceCheck(
    int const& frameno,
    int const& camId,
    std::vector<int> &dropFrameNoList
)
{
    bool hasFrametoDrop = false;
    //For Drop request comes first
    auto item = mDropQueue.find(frameno);
    if (item != mDropQueue.end()) {
        //Check has certain frameNo
        if(item->first == frameno) {
            std::vector <int> dropCameraList = item->second;
            for(int i = 0;i < dropCameraList.size();i++)
            {
                //Check camid if in drop list
                if(dropCameraList[i]==camId)
                {
                    MY_LOGD("cam[%d] frame(%d) is equal to DROP cam[%d] target(%d), remove",\
                                    camId,
                                    frameno,
                                    dropCameraList[i],
                                    item->first);
                    dropFrameNoList.push_back(frameno);
                    hasFrametoDrop = true;
                }
            }
        }
    }
    //Drop specific frame
    if(hasFrametoDrop){
        hasFrametoDrop = clearDropQueue(frameno,camId);
    }
    //Check if MvQueue has already has drop target
    auto vitem = mvQueue.find(frameno);
    if (vitem != mvQueue.end()) {
        if(vitem->first == frameno&&hasFrametoDrop) {
            MY_LOGD("target(%d) is equal to dropping frame(%d), remove",
                                    vitem->first,
                                    frameno);
        }
    }
    return (dropFrameNoList.size() == 0);
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
clearDropQueue(
    int const& frameno,
    int const& camId
)
{
    auto iter = mDropQueue.find(frameno);
    if(iter != mDropQueue.end()) {
        auto iter_cam = std::find(iter->second.begin(), iter->second.end(), camId);
        if(iter_cam != iter->second.end()){
            iter->second.erase(iter_cam);
        }
        if(iter->second.size()==0){
            MY_LOGD("All cam's frameno: %d has delete from DropQueue",frameno);
            mDropQueue.erase(iter);
            return true;
        }
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
pruneDropQueue(
)
{
    std::vector<int> dropFrames;
        for(auto&& item:mDropQueue) {
        if(item.second.size() >0 && item.first > 0) {
            dropFrames.push_back(item.first);
        }
    }
    int targetFrame = -1;
    if (dropFrames.size() != 0)
    {
        std::sort(dropFrames.begin(),dropFrames.end());
        targetFrame = dropFrames.front();
    }
    else
    {
        MY_LOGW("There is no dropframe to drop");
        return false;
    }
    if (targetFrame < 0)
    {
        MY_LOGW("Invalid target frame: %d Do not delete", targetFrame);
        return false;
    }
    MY_LOGD("target Prune frame: %d in DropQueue",targetFrame);
    auto iter = mDropQueue.find(targetFrame);
    if(iter != mDropQueue.end()) {
        mDropQueue.erase(iter);
        return true;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
addDropFrameInfo(
    int const frameno,
    int const camId,
    std::vector<int> &DropCameraList
)
{
    if (frameno < 0 || camId < 0)
    {
        MY_LOGE("Invalid frame (%d) or CamId", frameno, camId);
        return false;
    }
    // =====================================================================
    mLock.lock();
    // =====================================================================
    const int mdropQueueThreshold = 20;
    if (mDropQueue.size() > mdropQueueThreshold)
    {
        pruneDropQueue();
    }
    auto iter = mDropQueue.find(frameno);
    if(iter != mDropQueue.end())
    {
        MY_LOGD("already has same frame %d Cam[%d] in DropQueue, pass append",frameno, camId);
        clearDropQueue(frameno,camId);
        mLock.unlock();
        return false;
    }

    mDropQueue.insert({frameno, DropCameraList});
    //Check if there is drop target already in MvQueue
    if(dropItem(frameno))
        MY_LOGD("Remove frame(%d) from Queue because it should be drop", frameno);
    else
        MY_LOGD("Canot Remove frame(%d) from Queue because it's not in Queue", frameno);
    // =====================================================================
    // =====================================================================
    mLock.unlock();
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
enque(
    string callerName,
    int const& frameno,
    SyncResultInputParams const& input,
    SyncResultOutputParams &output,
    std::vector<int> &dropFrameNoList
)
{
    // =====================================================================
    mLock.lock();
    // =====================================================================
    // check still is user or not.
    checkSyncQueue(input, dropFrameNoList);
    bool isNeedDropSelf = false;
    for(auto& item:dropFrameNoList) {
        if(item == frameno) {
            // no needs to do sync.
            std::shared_ptr<SyncElement> pSyncElement = nullptr;
            enqueDataUpdate(item, input, output, pSyncElement);
            isNeedDropSelf = true;
        }
        dropItem(item);
    }
    if(isNeedDropSelf) {
        // =====================================================================
        mLock.unlock();
        // =====================================================================
        return false;
    }
    // check exist in mvQueue or not.
    std::shared_ptr<SyncElement> pSyncElement = nullptr;
    enqueDataUpdate(frameno, input, output, pSyncElement);
    if(pSyncElement != nullptr) {
        auto &data = pSyncElement->mSyncData;
        auto iter_data = std::find(
                            data.mSyncTarget.begin(),
                            data.mSyncTarget.end(),
                            input.CamId);
        // camid is exist in sync target.
        if(iter_data != data.mSyncTarget.end()) {
            std::unique_lock<std::mutex> condition_lk(pSyncElement->mConditionLock);
            data.miSyncTartgetSize++;
            if(data.mSyncTarget.size() == data.miSyncTartgetSize) {
                // wait all done.
                if(auto p = mpCb.lock()) {
                    MY_LOGD("[%s][f%d] CamId = %d notify", callerName.c_str(), frameno, input.CamId);
                    p->onEvent(
                            ISyncQueueCB::Type::SyncDone,
                            (SyncDataUpdaterType)mType,
                            data);
                }
                pSyncElement->mCV.notify_all();
                mLock.unlock();
                MY_LOGD("[%s][f%d] CamId = %d unlock", callerName.c_str(), frameno, input.CamId);
            }
             else {
                if(pSyncElement->needWait) {
                    // before wait, it has to unlock this mutex. Move Here to protect needWait
                    MY_LOGD("[%s][f%d] CamId = %d wait(%d)+", callerName.c_str(), frameno, input.CamId,pSyncElement->needWait);
                    mLock.unlock();
                    pSyncElement->mCV.wait(condition_lk);
                    MY_LOGD("[%s][f%d] CamId = %d wait-", callerName.c_str(), frameno, input.CamId);
                }
                else {
                    mLock.unlock();
                    MY_LOGD("[%s][f%d] no need wait CamId = %d", callerName.c_str(), frameno, input.CamId);
                }
            }
        }
    }
    else {
        // =====================================================================
        mLock.unlock();
        // =====================================================================
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
dropItem(
    int const& frameno
)
{
    auto pSyncElement = getSyncElement(frameno);
    if(!!pSyncElement) {
        std::unique_lock<std::mutex> condition_lk(pSyncElement->mConditionLock);
        if(auto p = mpCb.lock()) {
            auto &data = pSyncElement->mSyncData;
            MY_LOGD("[f%d] notify", frameno);
            p->onEvent(
                    ISyncQueueCB::Type::Drop,
                    (SyncDataUpdaterType)mType,
                    data);
        }
        pSyncElement->needWait = false;
        pSyncElement->mCV.notify_all();
        MY_LOGD("[f%d] unlock", frameno);
    }
    auto iter = mvQueue.find(frameno);
    if(iter != mvQueue.end()) {
        mvQueue.erase(iter);
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
removeItem(
    int const& frameno
)
{
    bool ret = false;
    std::lock_guard<std::mutex> lk(mLock);
    auto iter = mvQueue.find(frameno);
    if(iter != mvQueue.end()) {
        ret = true;
        mvQueue.erase(iter);
    }
    else {
        ret = false;
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
clear()
{
    for(auto&& item:mvQueue) {
        if(item.second != nullptr) {
            auto pSyncElement = getSyncElement(item.second->mSyncData.mFrameNo);
            if(!!pSyncElement) {
                std::unique_lock<std::mutex> condition_lk(pSyncElement->mConditionLock);
                pSyncElement->needWait = false;
                if(auto p = mpCb.lock()) {
                    MY_LOGI("[f%d] notify", item.second->mSyncData.mFrameNo);
                    p->onEvent(
                        ISyncQueueCB::Type::SyncDone,
                        (SyncDataUpdaterType)mType,
                        item.second->mSyncData);
                }
            }
        }
        item.second->mCV.notify_all();
        MY_LOGD("[r%d:f%d] unlock",
                        item.second->mSyncData.mRequestNo,
                        item.second->mSyncData.mFrameNo);
    }
    mvQueue.clear();
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
SyncDataUpdaterType
SyncQueue::
getSyncDataUpdateType() const
{
    return (SyncDataUpdaterType)mType;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
getFrameNoList(
    std::vector<int> &list
)
{
    std::lock_guard<std::mutex> lk(mLock);
    for(auto&& item:mvQueue) {
        list.push_back(item.first);
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
checkSyncQueue(
    SyncResultInputParams const& input,
    std::vector<int> &dropFrameNoList
)
{
    bool ret = true;
    if(!isSyncTargetExist(input)) {
        dropFrameNoList.push_back(input.frameNum);
        ret = false;
    }
    if(!sequenceCheck(input.frameNum, input.CamId, dropFrameNoList))
    {
        ret = false;
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
std::vector<int>
SyncQueue::
getSyncTartgetList(
    SyncResultInputParams const& input
)
{
    std::vector<int> ret;
    // add itself
    ret.push_back(input.CamId);
    //
    IMetadata::IEntry entry = input.HalControl->entryFor(MTK_FRAMESYNC_ID);
    if(!entry.isEmpty()) {
        for (MUINT i=0; i<entry.count(); i++) {
            ret.push_back(entry.itemAt(i, Type2Type<MINT32>()));
        }
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
std::vector<int>
SyncQueue::
getMWMasterSlaveList(
    SyncResultInputParams const& input
)
{
    std::vector<int> ret;
    // set MW master slave id
    IMetadata::IEntry entry = input.HalControl->entryFor(MTK_STEREO_SYNC2A_MASTER_SLAVE);
    if(!entry.isEmpty()) {
        for (MUINT i=0; i<entry.count(); i++) {
            ret.push_back(entry.itemAt(i, Type2Type<MINT32>()));
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
isSyncTargetExist(
    SyncResultInputParams const& input
)
{
    auto syncTargetList = getSyncTartgetList(input);
    bool ret = true;
    for(auto&& id:syncTargetList) {
        auto iter = std::find(
                                mvUserList.begin(),
                                mvUserList.end(),
                                id);
        // anyone not exist in user list, it means this request no need to sync.
        if(iter == mvUserList.end()) {
            ret &= false;
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
enqueDataUpdate(
    int const& frameno,
    SyncResultInputParams const& input,
    SyncResultOutputParams &output,
    std::shared_ptr<SyncElement>& syncElement
)
{
    // if frame number not exist in mvQueue, add new element.
    // otherwise, update sync data element.
    auto iter = mvQueue.find(frameno);
    if(iter != mvQueue.end() && iter->second != nullptr) {
        auto input_iter =
                iter->second->mSyncData.mvInputParams.find(input.CamId);
        if(input_iter != iter->second->mSyncData.mvInputParams.end()) {
            MY_LOGA("(input) Duplicate insert! frameno(%d) input cam(%d)",
                        frameno, input.CamId);
        }
        else {
            SyncResultInputParams *p = const_cast<SyncResultInputParams*>(&input);
            iter->second->mSyncData.mvInputParams.insert({input.CamId, p});
        }
        auto output_iter =
                iter->second->mSyncData.mvOutputParams.find(input.CamId);
        if(output_iter != iter->second->mSyncData.mvOutputParams.end()) {
            MY_LOGA("(output) Duplicate insert! frameno(%d) input cam(%d)",
                        frameno, input.CamId);
        }
        else {
            SyncResultOutputParams *p = &output;
            iter->second->mSyncData.mvOutputParams.insert({input.CamId, p});
        }
        syncElement = iter->second;
    }
    else {
        // need create new.
        syncElement = std::make_shared<SyncElement>();
        assert(!syncElement);
        genSyncData(syncElement->mSyncData, input, output);
        mvQueue.insert({syncElement->mSyncData.mFrameNo, syncElement});
        MY_LOGA_IF(mvQueue.size() > 10, "please check flow");
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<SyncElement>
SyncQueue::
getSyncElement(
    int const& frameno
)
{
    std::shared_ptr<SyncElement> pSyncElement;
    {
        auto iter = mvQueue.find(frameno);
        if(iter != mvQueue.end()) {
            pSyncElement = iter->second;
        }
        else {
            MY_LOGE("cannot get sync element F%d",frameno);
            return nullptr;
        }
    }
    return pSyncElement;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SyncQueue::
genSyncData(
    SyncData &syncdata,
    SyncResultInputParams const& input,
    SyncResultOutputParams &output
)
{
    auto syncTargetList = getSyncTartgetList(input);
    //
    syncdata.mFrameNo = input.frameNum;
    syncdata.mRequestNo = input.requestNum;
    syncdata.mSyncTarget = syncTargetList;
    syncdata.mMWMasterSlaveList = getMWMasterSlaveList(input);
    SyncResultInputParams *p_in = const_cast<SyncResultInputParams*>(&input);
    syncdata.mvInputParams.insert({input.CamId, p_in});
    SyncResultOutputParams *p_out = &output;
    syncdata.mvOutputParams.insert({input.CamId, p_out});
    return true;
}