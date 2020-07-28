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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _MTK_CAMERA_STL_UTILITY_H_
#define _MTK_CAMERA_STL_UTILITY_H_

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

namespace NSCam {


// ----------------------------------------------------------------------------
// SpinLock implementation
// ----------------------------------------------------------------------------
class SpinLock
{
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void lock()
    {
        while(locked.test_and_set(std::memory_order_acquire)) {
            // std::this_thread::yield();
        }
    }

    void unlock()
    {
        locked.clear(std::memory_order_release);
    }
}; // Spin lock


// ----------------------------------------------------------------------------
// Multi-Thread Safe Queue
// ----------------------------------------------------------------------------
// A thread-safe queue implementation. Notice that, the deque method will return
// immediately. If the queue is empty, thread_safe_queue will invoke default
// constructor to allocate a default variable. The default constructor can be
// assigned by the constructor of thread_safe_queue.
template <class T>
class MtQueue {
public:
    MtQueue(std::function<T(void)> d = []()->T{ return T(); })
    {
        mConstructor = d;
    }

    ~MtQueue()
    {
    }

private:
    mutable std::mutex      mMtx;
    std::condition_variable mCv;
    std::deque<T>           mQueue;
    std::function<T(void)>  mConstructor;

public:
    // add an element into the queue
    // @param val       the element that call by reference
    void enque(const T &val)
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        mQueue.push_back(val);
        mCv.notify_one();
    }

    // deque an element from the front of queue immediately
    // @param bUseMoveConstructor   if use std::move to invoke move constructor
    //                              of class T, or twice copy constructor will
    //                              be invoked
    // @return                      an element at the front of queue
    // @note                        if the queue is empty, return one from
    //                              default_constructor
    T deque(bool bUseMoveConstructor = false)
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        if (mQueue.size() <= 0) {
            return mConstructor();
        }

        if (bUseMoveConstructor) {
            T val = std::move(mQueue.front());
            mQueue.pop_front();
            return std::move(val);
        }
        else {
            T val = mQueue.front();
            mQueue.pop_front();
            return val;
        }
    }

    // deque an element from the front of queue, if no available element, this
    // method will block the thread and wait until element available
    // @param bUseMoveConstructor   if use std::move to invoke move constructor
    //                              of class T, or twice copy constructor will
    //                              be invoked
    // @return                      an element at the front of queue
    T dequeWait(bool bUseMoveConstructor = false)
    {
        std::unique_lock<std::mutex> ___l(mMtx);
        if (mQueue.size() <= 0) {
            mCv.wait(___l);
        }

        if (bUseMoveConstructor) {
            T val = std::move(mQueue.front());
            mQueue.pop_front();
            return std::move(val);
        }
        else {
            T val = mQueue.front();
            mQueue.pop_front();
            return val;
        }
    }

    // access the i-th element in the queue
    // @param i     the index of element
    // @return      reference to the element
    // @sa          at
    T& operator[](size_t i)
    {
        return this->at(i);
    }

    // access the i-th element in the queue
    // @param i     the index of element
    // @return      reference to the element
    T& at(size_t i)
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        return mQueue[i];
    }

    // returns the size of queue
    // @return size of queue, element counting
    size_t size() const
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        return mQueue.size();
    }

    // clears all element in the queue
    void clear()
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        mQueue.clear();
    }

    // gives a function for thread-safe iterative working with an additional
    // argument
    // @param worker    a function to work
    // @param arg       an additional argument for worker
    // @note            a operation mutex will be automatically locked and unlocked
    //                  during worker is working. do not invoke getMutex().lock()
    //                  or makes dead lock
    void iterativeDo(std::function<void(T &val, void *arg)> worker, void *arg = NULL)
    {
        std::lock_guard<std::mutex> ___l(mMtx);
        for (size_t i = 0; i < mQueue.size(); i++) {
            worker(arg);
        }
    }

// Advanced Methods: Caller has responsibility to make raw operation correct
public:
    // get the reference of operation mutex
    // @return  the reference of the mutex
    inline std::mutex& getMutex()
    {
        return mMtx;
    }

    // get the reference of queue
    // @return  the reference of the queue
    inline std::deque<T>& getQueue()
    {
        return mQueue;
    }
};

// ----------------------------------------------------------------------------
// Scope Worker
// ----------------------------------------------------------------------------
// Give a callback function and a void* argument, Scope Worker will invoke it
// while destroying
class ScopeWorker {
public:
    ScopeWorker(std::function<void(void *arg)> future_worker, void *arg = NULL)
    {
        mWorker = future_worker;
        mArg = arg;
    }
    ~ScopeWorker();

private:
    void *mArg;
    std::function<void(void* arg)> mWorker;
}; // }}}


// ----------------------------------------------------------------------------
// A thread-safe state manage implementation
// ----------------------------------------------------------------------------
template <class T>
class StateManager {
public:
    StateManager(std::function<T(void)> default_constructor = []()->T { return T(); })
    {
        mState = default_constructor();
    }

    ~StateManager()
    {
    }

private:
    std::mutex              mMutex;
    T                       mState;

public:
    // get the current state
    // @return  the current state that StateManager is holding
    // @note    this method is thread-safe
    // @sa      getStateNolock
    T getState()
    {
        std::lock_guard<std::mutex> ___l(mMutex);
        return mState;
    }

    // update the current state to the new one
    // @param s     the state to be updated
    // @note        this method is thread-safe
    // @sa          updateStateNolock
    void updateState(const T &s)
    {
        std::lock_guard<std::mutex> ___l(mMutex);
        mState = s;
    }

    // do a job with the same thread-safe operation
    // @tparam RTYPE    retrun type of this method
    // @param  worker   a function to work on, which will receive two arguments:
    //                    1. state, call by reference of current state (T&)
    //                       caller can update state by this value
    //                    2. customized argument (void*)
    // @param  arg      an customized argument for worker
    // @return          retruns what worker returns
    template <typename RTYPE = void>
    RTYPE doWork(
            std::function<RTYPE(T &state, void *arg)> work,
            void *arg = NULL)
    {
        std::lock_guard<std::mutex> ___l(mMutex);
        return work(mState, arg);
    }
}; // }}}




}; // namespace NSCam
#endif // _MTK_CAMERA_STL_UTILITY_H_
