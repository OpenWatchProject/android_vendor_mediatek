/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASEVEDNOR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASEVEDNOR_H_

// MTKCAM
#include <mtkcam/pipeline/extension/IVendorManager.h>

// STL
#include <vector>
#include <deque>
#include <mutex>
#include <future>

#include "utils/WorkPool.h"
#include "utils/BaseController.h"
#include "utils/ControllerContainer.h"
//
using namespace android;
#define OPENID_DONT_CARE -1

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace plugin {

class BaseVendor
    : public virtual IVendor
{
public:     ////
    enum WorkDirection
    {
        eDIRECTION_IN,
        eDIRECTION_OUT,
        eDIRECTION_IN_OUT,
        eDIRECTION_MAX_NUM,
    };

public:     ////
                                    BaseVendor(MINT32  openId, MINT64  mode, String8 name);

public:     //// IVendor interface.
    virtual char const*             getName() { return mName.string(); }
    virtual MINT32                  getOpenId() const { return mOpenId; }
    virtual MINT64                  getVendorMode() const { return mMode; }

    virtual status_t                acquireWorkingBuffer(
                                        MINT32             openId,
                                        const MetaItem&    setting,
                                        /*output*/
                                        BufferItem&        buf
                                    );
    virtual status_t                releaseWorkingBuffer(
                                        MINT32             openId,
                                        const MetaItem&    setting,
                                        BufferItem&        buf
                                    );

public:     //// Operation.
            status_t                prepareWorkingBuffer(
                                        MINT32 rDirection,
                                        sp<IImageStreamInfo> pInfo,
                                        MBOOL continuous
                                    );
            status_t                setPool(
                                        MINT32 rDirection,
                                        sp<WorkPool> pPool
                                    );
            status_t                prepareOneWithoutPool(
                                        MINT32 rDirection,
                                        sp<IImageStreamInfo> pInfo,
                                        MBOOL continuous,
                                        MINT32 openId = OPENID_DONT_CARE
                                    );
            void                    clearOneTimeBuffer() { Mutex::Autolock _l(mLock); mHeapList.clear(); }



protected:
    // To drop a frame queued from IVendor::queue (in a detached thread).
    //  @param openId               open id.
    //  @param cb                   Weak pointer of IDataCallback from IVendor::queue.
    //  @param bufParam             Buffer parameter from IVendor::queue.
    //  @param metaParam            Metadata parameter from IVendor::queue.
    //  @param bAsync               A RequestFrame cannot be dropped with the same
    //                              thread of IVendor::queue. To set bAsync to true
    //                              to drop frame in a detached thread.
    virtual void                    drop(
                                        MINT32  const               openId,
                                        wp<IVendor::IDataCallback>  cb,
                                        const BufferParam&          bufParam,
                                        const MetaParam&            metaParam,
                                        bool                        bAsync = true
                                    );

//
// Attributes
//
protected:     ////                 Data Members.
    MINT32                          mOpenId;
    MINT64                          mMode;
    android::String8                mName;

protected:
    mutable Mutex                   mLock;
    sp<WorkPool>                    mpInPool;
    sp<WorkPool>                    mpOutPool;

protected:
    struct OneTimeHeap
    {
        MINT32 direction;
        sp<IImageBufferHeap> heap;
        sp<IImageStreamInfo> info;
        MINT32 openId;

        // Constructor
        OneTimeHeap() :
            direction(0), openId(OPENID_DONT_CARE)
        {
        }
    };
    List< OneTimeHeap >             mHeapList;


protected:
    // BaseVendor provides a thread slots mechanism. Assume caller set the thread
    // slots size to N, it limits that there're N threads are running. This function
    // can only be invoked once, in other words, the size of thread slots is supposed
    // to be initialized while creating derived iVendor object.
    //  @param maxSize          The size of thread slots.
    //  @return                 Set ok returns ture.
    //  @note                   This thread is thread-safe.
    virtual bool setThreadShotsSize(size_t maxSize);

    // Get the size of thread slots.
    //  @return                 The size of thread slots.
    //  @note                   This method is thread-safe.
    virtual size_t getThreadShotSize() const;

    // Wait until all thread done.
    //  @note                   This method may block caller's thread.
    //                          This method is thread-safe.
    virtual void syncAllThread();

    // To run a task asynchronously. While alive thread number reaches to the size
    // of thread slots, it will wait until the earliest added job(thread) finished
    // it's task. This method blocks caller thread.
    //  @param task             A std::function object contains a task to
    //                          be run asynchronously. This function object
    //                          doesn't provide any argument and return result,
    //                          caller could use lambda function with captured
    //                          parameters as the function object.
    //  @note                   This method is thread-safe.
    virtual void asyncRun(std::function<void()>&& task);


protected:
    // Thread slots implementation.
    struct FutureSlot
    {
        typedef std::mutex OP_MUTEX_T;
        size_t                              _size;
        mutable OP_MUTEX_T                  _mx;
        std::atomic<size_t>                 _index;
        std::vector< std::future<void> >    _future_slots;
        mutable std::deque< std::mutex >    _future_slots_mx; // std::mutex is non-movable
        // Constructor
        FutureSlot();
        // Methods
        bool init(size_t size); // init FutureSlot with "size" slots.
        void async_run(std::function<void()>&& func); // asynchronously run a job
        void clear(); // wait all job done and clear slots
    } mFutureSlot;

}; // class BaseVendor
} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASEVEDNOR_H_
