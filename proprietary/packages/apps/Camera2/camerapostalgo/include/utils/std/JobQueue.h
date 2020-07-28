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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_CAMERA_UTILS_STD_JOBQUEUE_H_
#define _MTK_CAMERA_UTILS_STD_JOBQUEUE_H_

// STL
#include <deque>
#include <mutex>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <atomic>
#include <functional>
#include <algorithm>
#include <memory>

// C
#include <cstring> // strlen
#include <cstdio> // sprintf
#include <cstdint>
#include <cassert>

// AOSP
#include <log/log.h>
#include <android/log.h>

// Linux
#include <pthread.h>    // ::pthread_self
#include <sys/prctl.h>  // ::prctl


// systrace will be enabled for userdebug / eng load
#if defined(MTKCAM_LOG_LEVEL_DEFAULT) && (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
#define _MTK_CAMERA_UTILS_STD_JOBQUEUE_TRACE_       1
#else
#define _MTK_CAMERA_UTILS_STD_JOBQUEUE_TRACE_       0
#endif

#ifndef _MTK_CAMERA_UTILS_STD_JOBQUEUE_DEBUG_
// debug message is default off
#define _MTK_CAMERA_UTILS_STD_JOBQUEUE_DEBUG_       0
#endif


#if _MTK_CAMERA_UTILS_STD_JOBQUEUE_TRACE_
// {{{
#include    <utils/std/Trace.h>
#define     JOBQUEUE_TRACE_NAME(name)               CAM_TRACE_NAME(name)
#define     JOBQUEUE_TRACE_CALL()                   CAM_TRACE_CALL()
#define     JOBQUEUE_TRACE_BEGIN(name)              CAM_TRACE_BEGIN(name)
#define     JOBQUEUE_TRACE_END()                    CAM_TRACE_END()
#define     JOBQUEUE_TRACE_FMT_BEGIN(fmt, arg...)   CAM_TRACE_FMT_BEGIN(fmt, ##arg)
#else
#define     JOBQUEUE_TRACE_NAME(name)
#define     JOBQUEUE_TRACE_CALL()
#define     JOBQUEUE_TRACE_BEGIN(name)
#define     JOBQUEUE_TRACE_END()
#define     JOBQUEUE_TRACE_FMT_BEGIN(fmt, arg...)
// }}}
#endif


#if _MTK_CAMERA_UTILS_STD_JOBQUEUE_DEBUG_
// {{{
#include    <memory>
#include    <functional>
#define     JOBQUEUE_DEBUG(fmt, arg...)             ALOGD("JobQueue: " fmt, ##arg)
#define     JOBQUEUE_ERROR(fmt, arg...)             ALOGE("JobQueue: " fmt, ##arg)
#define     JOBQUEUE_DEBUG_SCOPE(name)              auto __jobQueueLogger__ = __job_queue_create_scope_logger(name)
#define     JOBQUEUE_DEBUG_CALL()                   JOBQUEUE_DEBUG_SCOPE(__FUNCTION__)
inline std::shared_ptr<char> __job_queue_create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    JOBQUEUE_DEBUG("%s [+] ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ JOBQUEUE_DEBUG("%s [-]", p); });
}
#else
#define     JOBQUEUE_DEBUG(fmt, arg...)
#define     JOBQUEUE_ERROR(fmt, arg...)
#define     JOBQUEUE_DEBUG_SCOPE(name)
#define     JOBQUEUE_DEBUG_CALL()
// }}}
#endif


namespace NSCam {

// JobQueue provides three ways to handle your jobs. Default is using FIFO (First
// In First Out) mechanism to execute your jobs, which means the first added job
// will be executed first. And the other way is sorting by priority. Each of job
// has a priority (default is 0), by JQPriority_GreaterFIFO, the higher priority
// will be executed much earlier, if the priority is the same, following the FIFO
// principle. The third one is JQPriority_Greater, as like JQPriority_GreaterFIFO,
// but if the priority is the same, the last pushed element will be executed much
// earlier.
enum JQPriority
{
    JQPriority_FIFO = 0,
    JQPriority_GreaterFIFO,
    JQPriority_Greater,
};

class JobQueueStatic {
public:
    // A helper to lock a mutex w/o throwing exception
    //  @param lk           An unique_lock constructed with std::defer_lock.
    //  @return             If locked, return true, otherwise false.
    static bool lock(std::unique_lock<std::mutex>& lk) noexcept;
};

// JobQueue is a mechanism to start a thread to execute jobs
// sequentially. A thread will be created while creating JobQueue instance,
// and be joined while destroying instance. Caller has no need to sync
// thread, just destroy the instance. And, Caller can easily add a job to
// JobQueue and get a JobHandle which is retrieved from a
// std::packaged_task. It means caller can invoke JobHandle::get() to get
// the result of the job, unlike lazy evaluation, the job will be execute
// in the JobQueue's thread. Notice that, caller cannot destroy JobQueue
// instance in it's own thread, or an exception will occur.
//
// * JobQueue is thread-safe and reentrant.
// * JobHandle is NOT thread-safe but reentrant.
#define __JQ_TMP_CLT_DCLT__  template <class F, JQPriority P>
#define __JQ_CLS_DCLT__      JobQueue<F, P>
template< class F, JQPriority PRIORITY = JQPriority_FIFO >
class JobQueue
{
public:
    // JobQueue's state
    enum State
    {
        State_Stopped = 0,
        State_Stopping,
        State_Running,
        //
        State_Size
    };


public:
    typedef typename std::function<F>::result_type R;
public:
    ////////////////////////////////////////
    // JobHandle
    ////////////////////////////////////////
    class JobHandle {
    // constructors
    public:
        explicit JobHandle(int id)  { _id = id; }
        JobHandle(JobHandle&& o)    { *this = std::move(o); }
        // forbidden copy constructor
        JobHandle(const JobHandle&) = delete;

    // interface
    public:
        // Get job ID, caller can use this ID to remove job from the JobQueue,
        // but notice that, you MUST invoke JobHandle::abandonSharedState first,
        // or exception will be thrown.
        //  @return                 An unique job ID.
        inline int  getId() const       { return _id; }

        // Wait and get the result.
        //  @return                 The result of your job.
        //  @note                   An expection will be thrown if JobHandle is
        //                          invalid.
        inline R    get()   const       { return _future.get(); }

        // Check if the JobHandle is valid (with a shared state) or not.
        //  @return                 JobHandle is valid or note.
        inline bool valid() const       { return _future.valid(); }

        // Remove or says abandon share state of this JobHandle.
        inline void abandonSharedState(){ _future = std::future<R>(); }

    // operator
    public:
        JobHandle& operator = (JobHandle&& o);
        JobHandle& operator = (std::future<R>&& f);
        // forbidden copy assignment
        JobHandle& operator = (const JobHandle&) = delete;

    private:
        int                     _id;
        std::shared_future<R>   _future;
    };


public:
    ////////////////////////////////////////
    // JobTask
    ////////////////////////////////////////
    class JobTask {
    public:
        int                   _id;
        int                   _priority;
        std::packaged_task<F> _task;
        //
        JobTask() : _id(0), _priority(0) {}
        JobTask(int id) : _id(id), _priority(0) {}
        JobTask(int id, std::function<R()>&& f)
            : _id(id), _priority(0), _task(std::move(f)) {}

    };


// constructors
public:
    JobQueue(const char* caller);
    virtual ~JobQueue();


public:

    // Add a job to JobQueue.
    //  @param job          A function object. If your function object is necessary
    //                      to have some variables, you have to use std::bind.
    //  @param priority     The priority of the job, if PRIORITY = JQPriority_FIFO,
    //                      this argument takes no effect.
    //  @return             A JobHandle to caller who wants to get the result of
    //                      the job after it finished.
    //  @complexity         Depends on PRIORITY:
    //                      Constant if it's JQPriority_FIFO
    //                      log(N) if it's JQPriority_Greater or JQPriority_GreaterFIFO
    JobHandle               addJob(
                                std::function<R()> job,
                                int priority = 0
                            );

    // Add a job which is contained in a shared_ptr to JobQueue.
    // This API is for caller who is really care about the lifetime in job.
    //  @param job          A shared_ptr containes a function object.
    //  @param priority     The priority of the job, if PRIORITY = JQPriority_FIFO,
    //                      this argument takes no effect.
    //  @return             A JobHandle to caller who wants to get the result of
    //                      the job after it finished.
    //  @complexity         Depends on PRIORITY:
    //                      Constant if it's JQPriority_FIFO
    //                      log(N) if it's JQPriority_Greater or JQPriority_GreaterFIFO
    JobHandle               addJob(
                                std::shared_ptr< std::function<R()> > job,
                                int priority = 0
                            );

    // Remove a job from JobQueue if it's still pending.
    //  @note               You MUST abandon the shared state of your JobHandle first,
    //                      or an exception will be thrown.
    //  @sa                 JobQueue<>::JobHandle::abandonSharedState
    //  @complexity         At most N applications of predicate.
    bool                    removeJob(int id);

    // Add an exit token as the last job to this JobQueue. This method returns
    // immediately, if caller wants to wait until all job has been finished,
    // caller needs to invoke JobQueue::wait.
    //  @complexity         Constant.
    void                    requestExit();

    // Add an exit token as the first job to this JobQueue. This method returns
    // immediately, if caller wants to wait until all job has been finished,
    // caller needs to invoke JobQueue::wait. Besides, the jobs after the exit
    // token, caller has responsibility to release their share state, or an
    // exception will be thrown.
    //  @note               Make sure all pending jobs' shared state has been
    //                      released, or an exception will be thrown.
    //  @complexity         Constant.
    void                    aggressExit();

    // Flush all pending request.
    //  @note               Make sure all pending jobs' shared state has been
    //                      released, or an exception would be thrown.
    //  @complexity         Constant.
    void                    flush();

    // Wait until JobQueue exited.
    //  @return             Wait successfully, returns true.
    //  @note               JobQueue won't end unless you invoke
    //                      JobQueue<>::requestExit or JobQueue<>::aggressExit.
    //  @complexity         Constant.
    bool                    wait() const;

    // Get the pending job size.
    //  @return             Size of pending job.
    //  @complexity         Constant.
    size_t                  getPendingSize() const;

    // Get JobQueue state.
    //  @return             The state of the JobQueue.
    //  @complexity         Constant.
    State                   getState() const;


private:
    // Get the current state.
    //  @return             The current state.
    //  @complexity         Constant.
    State                   getStateLocked() const;

    // Change the state by the FSM (Finite-State-Machine).
    //  @complexity         Constant.
    void                    transStateLocked(State s) const;

    // To generate a job ID.
    //  @return             An unique job ID.
    //  @note               This function is thread-safe.
    //  @complexity         Constant.
    int                     generateJobId() const;


//
// template methods
//
private:
    typedef std::shared_ptr<JobTask>        JobT;
    typedef std::deque< JobT >              QueT;

    // template specilization
    template < JQPriority P, typename DUMMY = void>
    struct templateOp {
        static void         enqueJobLocked(JobT, QueT&) {}
        static void         requestExitLocked(QueT&) {}
        static void         aggressExitLocked(QueT&) {}
        static JobT         dequeJobLocked(QueT&) { return JobT(); }
    };

    //
    // JQPriority_FIFO
    template <typename DUMMY>
    struct templateOp<JQPriority_FIFO, DUMMY> {
        // We push the element in the beginning of _queue, and pop from the
        // end of _queue.
        static void         enqueJobLocked(JobT job, QueT& _queue)
        { _queue.emplace_front(job); }

        static void         requestExitLocked(QueT& _queue)
        { _queue.emplace_front(nullptr); }

        static void         aggressExitLocked(QueT& _queue)
        { _queue.emplace_back(nullptr); }

        static JobT         dequeJobLocked(QueT& _queue)
        {
            auto j = std::move(_queue.back());
            _queue.pop_back();
            return j;
        }
    };

    //
    // JQPriority_GreaterFIFO
    template <typename DUMMY>
    struct templateOp<JQPriority_GreaterFIFO, DUMMY>
    : public templateOp<JQPriority_FIFO, DUMMY>
    {
        static void         enqueJobLocked(JobT job, QueT& _queue)
        {
            typedef typename QueT::iterator RandomItr_T;

            auto cmp = [](const JobT& a, const JobT& b)
            {
                return a->_priority < b->_priority;
            };

            // complexity is O(log(N))
            auto lower = std::lower_bound
            <
                RandomItr_T,
                JobT,
                decltype(cmp)
            >
            (_queue.begin(), _queue.end(), job, cmp);

            // insert after the lower bound
            _queue.emplace( lower, job );
        }
    };

    //
    // JQPriority_Greater
    template <typename DUMMY>
    struct templateOp<JQPriority_Greater, DUMMY>
    : public templateOp<JQPriority_FIFO, DUMMY>
    {
        static void         enqueJobLocked(JobT job, QueT& _queue)
        {
            typedef typename QueT::iterator RandomItr_T;

            auto cmp = [](const JobT& a, const JobT& b)
            {
                return a->_priority < b->_priority;
            };

            // complexity is O(log(N))
            auto up = std::upper_bound
            <
                RandomItr_T,
                JobT,
                decltype(cmp)
            >
            (_queue.begin(), _queue.end(), job, cmp);

            // always insert after the upper_bound
            _queue.emplace( up, job );
        }
    };


private:
    const char*                                 _callerName;
    std::deque< std::shared_ptr<JobTask> >      _queue;
    std::condition_variable                     _queueCond;
    mutable std::mutex                          _queueLock;
    std::atomic<long>                           _threadId;
    mutable State                               _state;
    std::future<void>                           _thread;
    mutable std::atomic<int>                    _jobIdCnt; // use for count job ID
};


//
// Implementations
//
__JQ_TMP_CLT_DCLT__
__JQ_CLS_DCLT__::JobQueue(const char* caller)
    : _callerName(caller)
    , _threadId(0)
    , _state(State_Running)
    , _jobIdCnt(0)
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    _thread = std::async(std::launch::async, [this]()
    {
        JOBQUEUE_TRACE_FMT_BEGIN("JobQueue_%s", _callerName);
        JOBQUEUE_DEBUG_SCOPE("JobQueueThread");
        _threadId.store(::pthread_self());

#if _MTK_CAMERA_UTILS_STD_JOBQUEUE_TRACE_
        do {
            size_t len = ::strlen(_callerName);
            if (__builtin_expect( len > 32, false )) {
                JOBQUEUE_ERROR("The length of caller name(%s) must <= 32.",
                    _callerName);
                assert(0);
            }
            char threadName[256] = {0};
            ::sprintf(threadName, "JQ_%s", _callerName);
            ::prctl(PR_SET_NAME, threadName, 0, 0, 0);
        } while(0);
#endif

        while (true) {
            std::shared_ptr<JobTask> job;
            //
            {
                std::unique_lock<decltype(_queueLock)> l(_queueLock);
                if (_queue.empty()) {
                    JOBQUEUE_TRACE_NAME("waitJob");
                    JOBQUEUE_DEBUG_SCOPE("waitJob");
                    //
                    _queueCond.wait(l);
                    continue;
                }
                else {
                    // take a job and remove it from queue
                    JOBQUEUE_TRACE_NAME("dequeJob");
                    JOBQUEUE_DEBUG_SCOPE("dequeJob");
                    //
                    job = templateOp<P>::dequeJobLocked(_queue);
                    // if the command is exit, mark as stopped
                    if (__builtin_expect( job.get() == nullptr, false )) {
                        transStateLocked(State_Stopped); // stopped
                    }
                }
            }
            //
            if ( job.get() != nullptr && job->_task.valid() ) {
                JOBQUEUE_TRACE_NAME("doJob");
                JOBQUEUE_DEBUG_SCOPE("doJob");
                job->_task();
            }
            else {
                JOBQUEUE_DEBUG("exit");
                break;
            }
        } // while

        {
            std::lock_guard<decltype(_queueLock)> l(_queueLock);
            transStateLocked(State_Stopped); // stop again
        }

        JOBQUEUE_TRACE_END();
    });
}


__JQ_TMP_CLT_DCLT__
__JQ_CLS_DCLT__::~JobQueue()
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    auto callerThreadId = ::pthread_self();
    bool isNeedDetachRelease =
        (_threadId.load() != 0) && (callerThreadId == _threadId.load());

    // if release JobQueue istance in it's own thread, it may happen deadlocl
    if (__builtin_expect( isNeedDetachRelease, false )) {
        ALOGE("NSCam::JobQueue: fatal: cannot delete JobQueue instance in its "\
              "own thread. JobQueue's caller=%s, tid=%ld, caller's tid=%ld",
              _callerName, _threadId.load(), callerThreadId);
        assert(0);
    }

    {
        // request to exit and wait
        std::unique_lock<std::mutex> lk(_queueLock, std::defer_lock); // defer lock, avoid exception
        auto __result = JobQueueStatic::lock(lk); // invoke a noexcept lock helper
        if (__builtin_expect( !__result, false )) {
            ALOGE("lock mutex exception");
        }

        templateOp<P>::requestExitLocked(_queue);
        _queueCond.notify_one();
    }

    wait();
}


__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::JobHandle
__JQ_CLS_DCLT__::addJob(
        std::function<R()>  job,
        int                 priority /* = 0 */
        )
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    JobHandle h( generateJobId() ); // a job ID will be generated here

    // if job is invalid
    if (__builtin_expect( !(job), false )) {
        JOBQUEUE_ERROR("%s: the job is invalid", _callerName);
        return h;
    }

    // create a JobTask
    auto j = std::make_shared<JobTask>( JobTask( h.getId(), std::move(job) ));
    job = nullptr;
    j->_priority = priority;

    // critical section
    {
        std::lock_guard<decltype(_queueLock)> l1(_queueLock);

        // if state has become State_Stopped, do not add into JobQueue
        if (__builtin_expect( getStateLocked() != State_Running, false )) {
            JOBQUEUE_DEBUG("%s: discard this job due to stopped JobQueue", _callerName);
            return h;
        }

        // retrieve JobHandle
        h = j->_task.get_future();

        // add to queue
        templateOp<P>::enqueJobLocked(std::move(j), _queue);
        _queueCond.notify_one();
    }

    return h;
}


__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::JobHandle
__JQ_CLS_DCLT__::addJob(
        std::shared_ptr< std::function<R()> >   job,
        int                                     priority /* = 0 */
        )
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    JobHandle h( generateJobId() );

    // if job is nullptr or invalid, return invalid JobHandle.
    if (__builtin_expect( job.get() == nullptr, false )) {
        JOBQUEUE_ERROR("%s: the job is nullptr", _callerName);
        return h;
    }

    // if job is invalid, return invalid JobHandle.
    if (__builtin_expect( !(*job), false )) {
        JOBQUEUE_ERROR("%s: the job is nullptr", _callerName);
        return h;
    }

    // create a JobTask
    auto j = std::make_shared<JobTask>( JobTask( h.getId(), std::move(*job) ));
    job = nullptr;
    j->_priority = priority;

    // critical section
    {
        std::lock_guard<decltype(_queueLock)> l1(_queueLock);
        // if state has become State_Stopping, State_Stopped, do not add into JobQueue
        if (__builtin_expect( getStateLocked() != State_Running, false )) {
            JOBQUEUE_DEBUG("%s: discard this job due to stopping or stopped JobQueue",
                    _callerName);
            return h;
        }
        // retrieve JobHandle
        h = j->_task.get_future();
        // add to queue
        templateOp<P>::enqueJobLocked(std::move(j), _queue);
        _queueCond.notify_one();
    }

    return h;
}


__JQ_TMP_CLT_DCLT__
bool __JQ_CLS_DCLT__::removeJob(int id)
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    std::shared_ptr<JobTask> job;
    {
        auto compareId = [id](const std::shared_ptr<JobTask>& j)
        {
            // check if it's nullptr first
            if (__builtin_expect( j.get() == nullptr, false )) {
                return false;
            }
            return (j->_id == id);
        };

        std::lock_guard<decltype(_queueLock)> l1(_queueLock);
        //
        auto itr = std::find_if(_queue.begin(), _queue.end(), compareId);
        // not found
        if (__builtin_expect( itr == _queue.end(), false )) {
            return false;
        }

        // found, but the element is nullptr
        if (__builtin_expect( (*itr).get() == nullptr, false )) {
            return false;
        }
        else {
            job = *itr;
            _queue.erase(itr);
        }
    }

    job = nullptr;

    return true;
}


__JQ_TMP_CLT_DCLT__
void __JQ_CLS_DCLT__::requestExit()
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    std::lock_guard<decltype(_queueLock)> l1(_queueLock);
    transStateLocked(State_Stopping);

    templateOp<P>::requestExitLocked(_queue);
    _queueCond.notify_one();
}


__JQ_TMP_CLT_DCLT__
void __JQ_CLS_DCLT__::aggressExit()
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    std::lock_guard<decltype(_queueLock)> l1(_queueLock);
    transStateLocked(State_Stopping);
    templateOp<P>::aggressExitLocked(_queue);
    _queueCond.notify_one();
}


__JQ_TMP_CLT_DCLT__
void __JQ_CLS_DCLT__::flush()
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    _queueLock.lock();      // critical section+++

    // do not clear all job in the critical section
    auto pendingJobs = std::move(_queue); // move all elements

    // if state is State_Stopping, add an exit token back
    if (getStateLocked() == State_Stopping)
        templateOp<P>::requestExitLocked(_queue);

    _queueCond.notify_one();
    _queueLock.unlock();    // critical section---

    pendingJobs.clear();
}


__JQ_TMP_CLT_DCLT__
bool __JQ_CLS_DCLT__::wait() const
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    if (__builtin_expect( _thread.valid(), true )) {
        _thread.wait();
        return true;
    }
    return false;
}


__JQ_TMP_CLT_DCLT__
size_t __JQ_CLS_DCLT__::getPendingSize() const
{
    JOBQUEUE_TRACE_CALL();
    JOBQUEUE_DEBUG_CALL();

    size_t s = 0;
    {
        std::lock_guard<decltype(_queueLock)> l1(_queueLock);
        s = _queue.size();
    }

    return s;
}


__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::State
__JQ_CLS_DCLT__::getState() const
{
    std::lock_guard<decltype(_queueLock)> l1(_queueLock);
    return getStateLocked();
}


__JQ_TMP_CLT_DCLT__
void __JQ_CLS_DCLT__::transStateLocked(State s) const
{
    // stateMachine[current][toBe]
    const State stateMachine[State_Size][State_Size] =
    {                     /* Stopped */     /* Stopping */       /* Running */   // <-- ToBe
        /* current */
        /* Stopped */   { State_Stopped,    State_Stopped,      State_Stopped   }, // final state
        /* Stopping */  { State_Stopped,    State_Stopping,     State_Stopping  },
        /* Running */   { State_Stopped,    State_Stopping,     State_Running   }
    };

    // transState
    _state = stateMachine[_state][s];
}


__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::State
__JQ_CLS_DCLT__::getStateLocked() const
{
    return _state;
}


__JQ_TMP_CLT_DCLT__
int __JQ_CLS_DCLT__::generateJobId() const
{
    return ++_jobIdCnt; // overflow is ok, just makes it unique.
}


///////////////////////////////////////////////////////////////////////////////
// JobHandle
///////////////////////////////////////////////////////////////////////////////
__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::JobHandle&
__JQ_CLS_DCLT__::JobHandle::operator = (JobHandle&& o)
{
    if (__builtin_expect( this == &o , false)) {
        return *this;
    }
    _future = std::move(o._future);
    _id = o._id;
    o._id = 0;
    return *this;
}


__JQ_TMP_CLT_DCLT__
typename __JQ_CLS_DCLT__::JobHandle&
__JQ_CLS_DCLT__::JobHandle::operator = (std::future<R>&& f)
{
    _future = std::move(f);
    return *this;
}


}; // namespace NSCam;
#endif // _MTK_CAMERA_UTILS_STD_JOBQUEUE_H_
