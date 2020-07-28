/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_SYNCMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_SYNCMANAGER_H_


#include <vector>

#include <utils/RefBase.h>
#include <mtkcam/feature/DualCam/IFrame3AControl.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/DualCam/ISyncManager.h>
#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/utils/std/Log.h>

#include <utils/Thread.h>

using namespace android;
using namespace std;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

/******************************************************************************
 *
 ******************************************************************************/

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF

#define MY_LOGV(id, fmt, arg...)        CAM_LOGV("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)        CAM_LOGD("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)        CAM_LOGI("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)        CAM_LOGW("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)        CAM_LOGE("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)        CAM_LOGA("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)        CAM_LOGF("[%d]id:%d[%s] " fmt, mInstanceRefCount, id, __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)



/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam
{


template <typename T>
inline MVOID updateEntry(IMetadata* pMetadata, MUINT32 const tag, T const& val)
{
    if (pMetadata == NULL)
    {
        //MY_LOGW("pMetadata is NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL getMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T& rVal)
{
    if (pMetadata == NULL)
    {
        //MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
    {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
class SyncManagerReqItem : public android::RefBase
{
    public:
        enum STATE
        {
            UNPROCESS,
            PROCESSED_AND_WAIT_CALLBACK,
            FINAL_CALLBACK,
            FINAL_CALLBACK_WITH_DROP
        };

    public:
        MINT32                      mOpenId;
        MUINT32                     mRequestNo;
        FeaturePipeParam            mFeaturePipeParam;
        void*                       mCb;

        enum STATE                  mState = UNPROCESS;
        FeaturePipeParam::MSG_TYPE  mCallbackType = FeaturePipeParam::MSG_FRAME_DONE;


    public:
        SyncManagerReqItem(
            MINT32  const               openId,
            MUINT32 const               requestNo,
            FeaturePipeParam            param,
            void*                       cb) {
            mOpenId    = openId;
            mRequestNo = requestNo;
            mFeaturePipeParam = param;
            mCb = cb;
        }
};

class SyncManagerReqList : public android::RefBase
{
    public:
        static MINT32 totalRefCount;
        MINT32        mInstanceRefCount;

    private:
        std::vector<sp<SyncManagerReqItem>> mList;

    public:
        SyncManagerReqList(): mInstanceRefCount(totalRefCount) {
            MY_LOGD(-1, "new %p", this);
            SyncManagerReqList::totalRefCount++;
        };

        void add(
            MINT32  const               openId,
            MUINT32 const               requestNo,
            FeaturePipeParam            param,
            void*                       cb) {

            sp<SyncManagerReqItem> req =
                new SyncManagerReqItem(openId, requestNo, param, cb);

            mList.push_back(req);
        }

        void add(sp<SyncManagerReqItem> item) {
            mList.push_back(item);
        }

        void rm(MINT32 idx) {
            mList.erase(mList.begin() + idx);
        }

        void doClear() {
            mList.clear();
        }

        MUINT getListItemCount() {
            return mList.size();
        }

        MBOOL isEmpty() {
            return (mList.size() == 0);
        }

        sp<SyncManagerReqItem> getItem(MUINT idx) {
            if (idx >= getListItemCount()) {
                return NULL;
            }
            return mList[idx];
        }

};


#define SYNC_MANAGER_MIN_ABS_VALUE     1      // 1 ms

#define SYNC_MANAGER_SYNC_FAIL                (-99)
#define SYNC_MANAGER_SYNC_OK                  0
#define SYNC_MANAGER_SYNC_FAIL_A              1
#define SYNC_MANAGER_SYNC_FAIL_B              (-1)
#define SYNC_MANAGER_SYNC_FAIL_A_NO_TIMESTAMP 1000
#define SYNC_MANAGER_SYNC_FAIL_B_NO_TIMESTAMP 1001

// key to FeaturePipeParam.mVarMap
#define SYNC_SYNCMGR            "SyncManager"
#define SYNC_REQITEM            "SyncManagerReqItem"
#define SYNC_REQITEM_SLAVE      "SyncManagerReqItemSlave"
#define SYNC_CALLBACK           "SyncManagerCallback"

class SyncManager : public Thread, public ISyncManager
{
    public:
        static MINT32                   totalRefCount;
        MINT32                          mInstanceRefCount;
        static wp<SyncManager>          mThiz;
        static android::sp<SyncManager> createInstance(
            MINT32 const i4OpenId
        );
        static android::sp<SyncManager> getInstance(
            MINT32 const i4OpenId
        );

    protected:
        MBOOL                           mFlush;

        MINT32                          mFirstOpenId = -1;

        // ************************************************************************
        // these three shall be one-one mapping with it's index and the adding order
        vector<MINT32>                  mOpenIds;
        vector<sp<SyncManagerReqList>>  mSyncManagerReqLists;
        vector<sp<SyncManagerReqList>>  mCallBackLists;  // in-order callback lists
        // ************************************************************************

        // HW Frame 3A control
        sp<IFrame3AControl>             mIFrame3AControl;

        // Policy control
        sp<IDualCamPolicy>              mIDualCamPolicy;
        sp<ISyncManagerCallback>        mICallback;

        MINT32                          mPreviewID = 0;
        MINT32                          mPreviewNewID = 0;


    protected:
        Mutex                           mRequsetLock;
        Mutex                           mSettingLock;

    public:
        SyncManager(MINT32 const i4OpenId);
        ~SyncManager();
        char const*             getName();

        MBOOL                   isFirstId(MINT32 id);

        MINT32                  getIndexOfOpenIds(MINT32 id);

        MINT32                  getOpenedIdCount();

        MBOOL                   isOpenedId(MINT32 id);

        MVOID                   addOpenID(MINT32 id);

        status_t                queue(
            MINT32  const               openId,
            MUINT32 const               requestNo,
            FeaturePipeParam            param,
            void*                       cb);

        status_t                flushAndRemoveOpenId(MINT32 openId);
        MBOOL                   flushOnly(MINT32 openId);

        void                    onCallback(FeaturePipeParam::MSG_TYPE type,
                                           sp<SyncManagerReqItem> frame,
                                           FeaturePipeParam& org_callback_param);

        void                    processFrameCallback(MINT32 const openIndex, MBOOL isForceFlush);


        //// debug
        status_t                dump(MINT32 openId);



        //// ISyncManager interface *************************************************
    public:
        MINT32 updateSetting(
            MINT32 const i4OpenId,
            IMetadata* appMetadata,
            IMetadata* halMetadata,
            SyncManagerParams &syncParams
        );
        MVOID lock(void* arg1);
        MVOID unlock(void* arg1);
        MVOID registerMgrCb(sp<ISyncManagerCallback> cb);
        MVOID setEvent(MUINT32 key, void * arg);


        //// Thread interface *************************************************
    protected:
        // Derived class must implement threadLoop(). The thread starts its life
        // here. There are two ways of using the Thread object:
        // 1) loop: if threadLoop() returns true, it will be called again if
        //          requestExit() wasn't called.
        // 2) once: if threadLoop() returns false, the thread will exit upon return.
        virtual bool    threadLoop();

        // Do not call Thread::requestExitAndWait() without first calling requestExit().
        // Thread::requestExitAndWait() is not virtual, and the implementation doesn't do enough.
        virtual void    requestExit();

        // FIXME merge API and implementation with AudioTrackThread
        void            pause();        // suspend thread from execution at next loop boundary
        void            resume();       // allow thread to execute, if not requested to exit

        Mutex           mMyLock;        // Thread::mLock is private
        Condition       mMyCond;        // Thread::mThreadExitedCondition is private
        MBOOL           mIsPaused;        // whether thread is currently paused
        MBOOL           mIsExit = 0;      // whether thread is currently paused
        MBOOL           mInFlushing;

        //virtual void            onFirstRef();
        //virtual void            onLastStrongRef(const void* id);
        //virtual bool            onIncStrongAttempted(uint32_t flags, const void* id);
        //virtual void            onLastWeakRef(const void* id);




        //// internal *********************************************************
    private:
        MBOOL        isDropFrame(sp<SyncManagerReqItem> frame, MINT32 doDrop);
        MINT32       isSyncFrame(MINT64 a, MINT64 b);
        MINT32       isSyncFrame(IMetadata* a, IMetadata* b);
        IMetadata*   getIMetadata(/*Vector<sp<MetaHandle>> metaVector*/);
        void         markEnqueFrame(sp<SyncManagerReqItem> frame, sp<SyncManagerReqItem> slave,
                                    MBOOL isCombine);
        void         markDropFrame(sp<SyncManagerReqItem> frame);

        status_t     processFrameDrop();
        status_t     processFrameSync(MINT32 const openId);
        status_t     processFrameCallback();
};

} // namespace NSCam

#endif // _MTK_HARDWARE_MTKCAM_SYNCMANAGER_H_
