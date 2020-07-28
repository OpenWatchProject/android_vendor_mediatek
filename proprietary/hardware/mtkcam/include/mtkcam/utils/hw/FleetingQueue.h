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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_FLEETINGQUEUE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_FLEETINGQUEUE_H_


#ifndef LOG_TAG
#define LOG_TAG         "MtkCam/HwUtils/FleetingQueue"
#endif
#define FLEETINGQUEUE   "FleetingQueue"


#include <utils/RefBase.h> // android::RefBase

// MTKCAM
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock
#include <mtkcam/def/common.h> //MBOOL
#include <cutils/compiler.h> //CC_LIKELY, CC_UNLIKELY

// STL
#include <unordered_set>
#include <vector>
#include <map>
#include <string>
#include <cassert> // assert


// enable additional debug info
// #define __DEBUG

#define MY_LOGV(fmt, arg...)        CAM_ULOGV(NSCam::Utils::ULog::MOD_UTILITY, "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGD(NSCam::Utils::ULog::MOD_UTILITY, "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGI(NSCam::Utils::ULog::MOD_UTILITY, "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGW(NSCam::Utils::ULog::MOD_UTILITY, "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGE(NSCam::Utils::ULog::MOD_UTILITY, "[%s] " fmt, __FUNCTION__, ##arg)

#ifdef __DEBUG
#include <memory>
#define FLEETINGQUEUE_FUNCTION_SCOPE \
auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    MY_LOGD("[%s] + ", f); \
    return std::shared_ptr<const char>(f, [](char const* p){MY_LOGD("[%s] -", p);}); \
}(__FUNCTION__)
#else
#define FLEETINGQUEUE_FUNCTION_SCOPE  do{}while(0)
#endif


namespace NSCam {
template <class DATATYPE, size_t QUEUESIZE, size_t BACKUPSIZE>
class FleetingQueue : public android::RefBase
{
protected:
    class QIndex
    {
    public:
        size_t get() { return mCurIdx; };

        QIndex getHead() { return QIndex( (mCurIdx + int(mQuerySize/mMaxSize + 1)*mMaxSize - mQuerySize) % mMaxSize, mQuerySize, mMaxSize-mQuerySize); };

        void move() { if (CC_UNLIKELY(++mCurIdx == mMaxSize)) mCurIdx = 0; };


    public:
        QIndex() = delete;
        QIndex(size_t idx, size_t queue_size, size_t backup_size)
            : mCurIdx(idx)
            , mQuerySize(queue_size)
            , mMaxSize(queue_size+backup_size)
        {
            if (CC_UNLIKELY(mMaxSize == 0))
                MY_LOGE("%s: queue_size + backup_size can not be zero.", __FUNCTION__);
            else
                idx %= mMaxSize;
        };

    private:
        size_t  mCurIdx;
        size_t  mQuerySize;
        size_t  mMaxSize;
    };

    class QMeta : public android::RefBase
    {
    public:
        MBOOL registerUserId(char const* userId)
        {
            if (CC_LIKELY(userId)) {
                vUserId.insert(std::string(userId));
                return MTRUE;
            }

            MY_LOGE("%s: userId can not be NULL.", __FUNCTION__);
            return MFALSE;
        };

        MBOOL unregisterUserId(char const* userId)
        {
            if (CC_LIKELY(userId)) {
                auto it = vUserId.find(std::string(userId));
                if (CC_LIKELY(it != vUserId.end()) ) {
                    vUserId.erase(it);
                    return MTRUE;
                }

                MY_LOGE("%s: userId(%s) can not be found in FleetingQueue.", __FUNCTION__, userId);
                return MFALSE;
            }
            MY_LOGE("%s: userId can not be NULL.", __FUNCTION__);
            return MFALSE;
        };

        size_t userCount()
        {
            return vUserId.size();
        };

        void dumpInfo()
        {
            MY_LOGD("%s: - %zu usage of pData(%p), isDirty(%d):", __FUNCTION__, userCount(), pData, isDirty);

            std::string usage;
            for (auto it = vUserId.begin(); it != vUserId.end(); it++)
                usage += "[" + *it + "] ";

            if (!usage.empty())
                MY_LOGD("%s:   %s", __FUNCTION__, usage.c_str());
        };

    public:
        QMeta()
            : timestamp(-1)
            , pData(nullptr)
            , isDirty(MTRUE)
        {
            vUserId.clear();
        }

        QMeta(DATATYPE* ptr)
            : timestamp(-1)
            , pData(ptr)
            , isDirty(MTRUE)
        {
            vUserId.clear();
        }


        ~QMeta() {
            if (pData)
                delete(pData);
        }

    public:
        int64_t                                 timestamp;
        DATATYPE*                               pData;
        MBOOL                                   isDirty;

    private:
        std::unordered_multiset<std::string>    vUserId;
    };


public:
    DATATYPE* getLatestData(void)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        if (CC_UNLIKELY( mLatestMeta == nullptr || mLatestMeta->isDirty == MTRUE ))
            return nullptr;

        return mLatestMeta->pData;
    }

    std::vector< DATATYPE* > getInfo(char const* userId)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        std::vector< DATATYPE* > ret;

        for (QIndex idx = mQueueIndex.getHead(); idx.get() != mQueueIndex.get(); idx.move()) {
            android::sp<QMeta> meta = mQueueMeta[idx.get()];
            if (CC_UNLIKELY( meta->isDirty == MTRUE ))
                continue;

            if (CC_LIKELY( meta->pData )) {
                if (meta->registerUserId(userId))
                    ret.push_back(meta->pData);
            }
            else {
                MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                meta->dumpInfo();
                return std::vector< DATATYPE* >();
            }
        }

        return ret;
    }

    std::vector< DATATYPE* > getInfo(char const* userId, const int64_t& ts_start, const int64_t& ts_end)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        std::vector< DATATYPE* > ret;

        for (QIndex idx = mQueueIndex.getHead(); idx.get() != mQueueIndex.get(); idx.move()) {
            android::sp<QMeta> meta = mQueueMeta[idx.get()];
            if (CC_UNLIKELY( meta->isDirty == MTRUE ))
                continue;

            if (CC_LIKELY( meta->pData )) {
                if (meta->timestamp >= ts_start && meta->timestamp <= ts_end) {
                    if (meta->registerUserId(userId))
                        ret.push_back(meta->pData);
                }
            }
            else {
                MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                meta->dumpInfo();
                return std::vector< DATATYPE* >();
            }
        }

        return ret;
    }

    std::vector< DATATYPE* > getInfo(char const* userId, const std::vector<int64_t>& vecTss)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        std::vector< DATATYPE* > ret;

        std::map<int64_t, android::sp<QMeta>> queryMap;
        queryMap.clear();

        // build queryMap
        for (QIndex idx = mQueueIndex.getHead(); idx.get() != mQueueIndex.get(); idx.move()) {
            android::sp<QMeta> meta = mQueueMeta[idx.get()];
            if (CC_UNLIKELY( meta->isDirty == MTRUE ))
                continue;

            if (CC_LIKELY( meta->pData )) {
                queryMap[meta->timestamp] = meta;
            }
            else {
                MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                meta->dumpInfo();
                return std::vector< DATATYPE* >();
            }
        }

        // return found chunk in queryMap or nullptr
        for (size_t i = 0 ; i < vecTss.size() ; i++) {
            android::sp<QMeta> meta = queryMap[vecTss[i]];

            //return nullptr if key is not found
            ret.push_back(nullptr);
            if (CC_LIKELY( meta.get() )) {
                if (CC_LIKELY( meta->pData)) {
                    if (CC_LIKELY( meta->timestamp == vecTss[i] )) {
                        if (meta->registerUserId(userId))
                            ret.back() = meta->pData;
                    }
                    else {
                        MY_LOGW("%s: Queue pData is non-sync", __FUNCTION__);
                        meta->dumpInfo();
                    }
                }
                else {
                    MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                    meta->dumpInfo();
                }
            }
        }

        return ret;
    }

    MBOOL returnInfo(char const* userId, const std::vector< DATATYPE* >& vecInfos)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);
        std::map<DATATYPE*, android::sp<QMeta>> queryMap;
        queryMap.clear();

        // build queryMap
        for (size_t i = 0; i < mQueueMeta.size(); i++) {
            android::sp<QMeta> meta = mQueueMeta[i];
            if (CC_LIKELY( meta->pData )) {
                queryMap[meta->pData] = meta;
            }
            else {
                MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                meta->dumpInfo();
                return MFALSE;
            }
        }

        for (size_t i = 0 ; i < vecInfos.size() ; i++) {
            if (CC_LIKELY( vecInfos[i] )) {
                android::sp<QMeta> meta = queryMap[vecInfos[i]];

                if (CC_LIKELY( meta.get() )) {
                    if (CC_LIKELY( meta->pData )) {
                        meta->unregisterUserId(userId);
                    } else {
                        MY_LOGW("%s: Queue pData is NULL", __FUNCTION__);
                        meta->dumpInfo();
                        return MFALSE;
                    }
                }
            }
        }
        return MTRUE;
    }

    DATATYPE* editInfo(char const* userId, int64_t timestamp)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        if (mIsEditing == MTRUE) {
            MY_LOGW("%s: User(%s) is not allow write to FleetingQueue because it already under writing", __FUNCTION__, std::string(userId).c_str());
            return nullptr;
        }

        android::sp<QMeta> meta = mQueueMeta[mQueueIndex.get()];
        if (CC_LIKELY( meta->pData )) {
            if (meta->userCount() == 0) {
                if (meta->registerUserId(userId)) {
                    meta->timestamp = timestamp;
                    mIsEditing = MTRUE;
                    // set the same timestamp to dirty
                    for (size_t i = 0; i < mQueueMeta.size(); i++) {
                        if (CC_LIKELY( mQueueMeta[i]->timestamp != timestamp ))
                            continue;
                        mQueueMeta[i]->isDirty = MTRUE;
                    }
                    return meta->pData;
                }
            }
            else {
                MY_LOGE("%s: FleetingQueue is full", __FUNCTION__);
                meta->dumpInfo();
            }
        }
        else {
            MY_LOGW("%s: Queue entry is NULL", __FUNCTION__);
            meta->dumpInfo();
        }

        return nullptr;
    }

    MBOOL publishInfo(char const* userId, DATATYPE* info)
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        if (mIsEditing == MFALSE) {
            MY_LOGW("%s:  User(%s) is not in write state", __FUNCTION__, std::string(userId).c_str());
            return MFALSE;
        }

        android::sp<QMeta> meta = mQueueMeta[mQueueIndex.get()];
        if (CC_LIKELY( info )) {
            if (info == meta->pData) {
                if (meta->unregisterUserId(userId)) {
                    mIsEditing = MFALSE;
                    meta->isDirty = MFALSE;
                    mQueueIndex.move();
                    mLatestMeta = meta;
                    return MTRUE;
                }
                else {
                    MY_LOGE("%s: Someone else is editing FleetingQueue?", __FUNCTION__);
                    meta->dumpInfo();
                }
            }
            else {
                MY_LOGW("%s: Published pData is not under writing", __FUNCTION__);
                meta->dumpInfo();
            }
        }
        else {
            MY_LOGW("%s: Publish pData is NULL", __FUNCTION__);
        }

        return MFALSE;
    }

    void dumpInfo()
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        MY_LOGD("%s: FleetingQueue mQueueIndex = %zu", __FUNCTION__, mQueueIndex.get());
        for (size_t i = 0 ; i < mQueueMeta.size() ; i++) {
            android::sp<QMeta> meta = mQueueMeta[i];
            if (CC_LIKELY( meta->pData )) {
                MY_LOGD("%s: FleetingQueue[%zu]->timestamp is %" PRIi64, __FUNCTION__, i, meta->timestamp);
                meta->dumpInfo();
            }
            else {
                MY_LOGD("%s: FleetingQueue[%zu] is null", __FUNCTION__, i);
            }
        }
    }

    void clear()
    {
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);
        for (size_t i = 0 ; i < mQueueMeta.size() ; i++)
            mQueueMeta[i]->isDirty = MTRUE;
        mLatestMeta = nullptr;
    }


public:
    FleetingQueue()
        : mQueueIndex(0, QUEUESIZE, BACKUPSIZE)
        , mIsEditing(MFALSE)
        , mLatestMeta(nullptr)
    {
        FLEETINGQUEUE_FUNCTION_SCOPE;
        std::lock_guard<NSCam::SpinLock> __l(mQueuelLock);

        if (CC_UNLIKELY(QUEUESIZE == 0 || BACKUPSIZE == 0)) {
            MY_LOGE("%s: Both QUEUESIZE and BACKUPSIZE can not be zero", __FUNCTION__);
            assert(0);
        }

        mQueueMeta.resize(QUEUESIZE + BACKUPSIZE, nullptr);

        //allocate time limit infos
        for (size_t i = 0 ; i < mQueueMeta.size() ; i++)
            mQueueMeta[i] = new QMeta( new DATATYPE );
    }

    virtual ~FleetingQueue()
    {
        FLEETINGQUEUE_FUNCTION_SCOPE;

        for (size_t i = 0 ; i < mQueueMeta.size() ; i++) {
            if (CC_UNLIKELY( mQueueMeta[i]->userCount() != 0 )) {
                MY_LOGE("%s: FleetingQueue[%zu] is in-used", __FUNCTION__, i);
                mQueueMeta[i]->dumpInfo();
            }
        }
        mQueueMeta.clear();
        mLatestMeta = nullptr;
    }

private:
    // AttributesMeta;
    std::vector< android::sp<QMeta> >       mQueueMeta;
    android::sp<QMeta>                      mLatestMeta;
    NSCam::SpinLock                         mQueuelLock;
    QIndex                                  mQueueIndex;
    MBOOL                                   mIsEditing;
};
}; /* namespace NSCam */

#endif//_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_FLEETINGQUEUE_H_
