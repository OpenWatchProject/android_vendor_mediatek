#ifndef __DP_THREAD_ANDROID_H__
#define __DP_THREAD_ANDROID_H__

#include "DpDataType.h"
#include "DpList.h"
#include "DpMutex.h"
#include "DpLogger.h"
#include "DpTimer.h"

#include <sys/resource.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

/*
 * From linux kernel 3.10
 *
 * Priority of a process goes from 0..MAX_PRIO-1, valid RT
 * priority is 0..MAX_RT_PRIO-1, and SCHED_NORMAL/SCHED_BATCH
 * tasks are in the range MAX_RT_PRIO..MAX_PRIO-1. Priority
 * values are inverted: lower p->prio value means higher priority.
 *
 * #define MAX_RT_PRIO             100
 * #define MAX_PRIO                (MAX_RT_PRIO + 40)
 * #define DEFAULT_PRIO            (MAX_RT_PRIO + 20)
 *
 * Convert user-nice values [ -20 ... 0 ... 19 ]
 * to static priority [ MAX_RT_PRIO..MAX_PRIO-1 ],
 * and back.
 *
 * #define NICE_TO_PRIO(nice)      (MAX_RT_PRIO + (nice) + 20)
 * #define PRIO_TO_NICE(prio)      ((prio) - MAX_RT_PRIO - 20)
 *
 * static inline int normal_prio(struct task_struct *p)
 * {
 *         int prio;
 *
 *         if (task_has_rt_policy(p))
 *                 prio = MAX_RT_PRIO-1 - p->rt_priority;
 *         else
 *                 prio = p->static_prio;
 *         return prio;
 * }
 *
 * SYSCALL_DEFINE2(sched_getparam, pid_t, pid, struct sched_param __user *, param)
 * { ...
 *         lp.sched_priority = p->rt_priority;
 *   ... }
 *
 * void set_user_nice(struct task_struct *p, long nice)
 * { ...
 *         p->static_prio = NICE_TO_PRIO(nice);
 *   ... }
 */
/*
 * By the definition above, here we define cmdq priority as
 * (higher value means higher priority):
 *
 * #define RT_TO_CMDQ_PRIO(prio)   (MAX_PRIO - (MAX_RT_PRIO-1 - (prio)))
 * #define NICE_TO_CMDQ_PRIO(nice) (MAX_PRIO - NICE_TO_PRIO(nice))
 *
 *        | RT |nice |normal|MDP/cmdq
 *        |    |     | prio |  prio
 * -------|----|-----|------|---------
 * higher | 99 |     |    0 |   140   // 41 + RT
 *        |  . |     |    . |     .
 *   |    |  : |     |    : |     :
 *   |    |  1 |     |   98 |    42
 *   |    |  x |     |      |
 *   |    |    | -20 |  100 |    40   // 20 - nice
 *   |    |    |   . |    . |     .
 *   v    |    |   : |    : |     :
 *        |    |  19 |  139 |     1
 * lower  |    |   x |      |     0   // unset
 *
 * And we use the following princeples:
 * 1. For single thread, use the thread priority.
 * 2. For multi thread,
 *    2.1 if caller is real-time, use the worker priority,
 *            which should be min(hint, caller priority + 1);
 *    2.2 if caller is normal, use the caller priority.
 */

#define RT_TO_CMDQ_PRIO(prio)   (41 + (prio))
#define NICE_TO_CMDQ_PRIO(nice) (20 - (nice))

#define IS_RT_POLICY(cmdq_prio)         ((cmdq_prio) > 40)
#define CMDQ_PRIO_TO_POLICY(cmdq_prio)  (IS_RT_POLICY(cmdq_prio) ? SCHED_RR : SCHED_NORMAL)
#define CMDQ_PRIO_TO_RT(cmdq_prio)      (IS_RT_POLICY(cmdq_prio) ? ((cmdq_prio) - 41) : 0)
#define CMDQ_PRIO_TO_NICE(cmdq_prio)    (20 - (cmdq_prio))


class DpSingleThread
{
public:
    enum PRIORITY_ENUM // RT priority
    {
        PRIORITY_HIGH   = 95,
        PRIORITY_MEDIUM = 85,
        PRIORITY_LOW    = 50
    };

    DpSingleThread()
        : m_schedPolicy(0),
          m_priority(0)
    {
    }

    virtual ~DpSingleThread()
    {
    }

    int32_t priority()
    {
        int32_t            priority;
        int32_t            schedPolicy;
        struct sched_param threadParam;

        pthread_getschedparam(pthread_self(), &schedPolicy, &threadParam);

        if ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy))
        {
            priority = RT_TO_CMDQ_PRIO(threadParam.sched_priority);
            DPLOGI("DpSingleThread: real-time, prio %#x, cmdq %d\n", threadParam.sched_priority, priority);
        }
        else
        {
            int32_t rc = getpriority(PRIO_PROCESS, 0);

            priority = NICE_TO_CMDQ_PRIO(rc);
            DPLOGI("DpSingleThread: normal, nice %d, cmdq %d\n", rc, priority);
        }

        return priority;
    }

    DP_STATUS_ENUM run(PRIORITY_ENUM)
    {
        //boostPriority();
        return this->threadLoop();
    }

    DP_STATUS_ENUM exit()
    {
        //restorePriority();
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM join()
    {
        //restorePriority();
        return DP_STATUS_RETURN_SUCCESS;
    }

private:
    int32_t m_schedPolicy;
    int32_t m_priority;

    virtual DP_STATUS_ENUM threadLoop() = 0;

    void boostPriority()
    {
        struct sched_param threadParam;

        pthread_getschedparam(pthread_self(), &m_schedPolicy, &threadParam);

        if ((SCHED_RR == m_schedPolicy) || (SCHED_FIFO == m_schedPolicy))
        {
            m_priority = RT_TO_CMDQ_PRIO(threadParam.sched_priority);
            DPLOGI("DpSingleThread: real-time, prio %#x, cmdq %d\n", threadParam.sched_priority, m_priority);

        #ifdef SUPPORT_SETSCHEDPARAM
            threadParam.sched_priority = 92;

            if (pthread_setschedparam(pthread_self(), m_schedPolicy, &threadParam))
            {
                DPLOGW("DpSingleThread: boost schedule fail; real-time, prio %#x\n", CMDQ_PRIO_TO_RT(m_priority));
            }
        #endif // SUPPORT_SETSCHEDPARAM
        }
        else
        {
            int32_t rc = getpriority(PRIO_PROCESS, 0);

            m_priority = NICE_TO_CMDQ_PRIO(rc);
            DPLOGI("DpSingleThread: normal, nice %d, cmdq %d\n", rc, m_priority);
        }
    }

    void restorePriority()
    {
    #ifdef SUPPORT_SETSCHEDPARAM
        int32_t            schedPolicy;
        struct sched_param threadParam;

        threadParam.sched_priority = CMDQ_PRIO_TO_RT(m_priority);

        if (pthread_setschedparam(pthread_self(), m_schedPolicy, &threadParam))
        {
            pthread_getschedparam(pthread_self(), &schedPolicy, &threadParam);
            DPLOGW("DpSingleThread: restore schedule fail: %s, prio %#x\n",
                ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", threadParam.sched_priority);
        }
    #endif // SUPPORT_SETSCHEDPARAM
    }
};


class DpJobWorker
{
private:
    enum STATE_ENUM
    {
        STATE_IDLE,
        STATE_BUSY,
        STATE_WAIT,
        STATE_STOP
    };

public:
    typedef DP_STATUS_ENUM (*WorkerJob)(void *pArg);

    DpJobWorker()
        : m_workerIndex(0)
    {
    }

    ~DpJobWorker()
    {
        WorkerList::iterator iterator;

        AutoMutex lock(m_workerLock);

        DPLOGI("DpJobWorker: destroy all workers in pool begin\n");

        for(iterator = m_workerList.begin(); iterator != m_workerList.end(); iterator++)
        {
            pthread_mutex_lock(&iterator->threadLock);
            iterator->workerState = STATE_STOP;
            pthread_cond_signal(&iterator->inIdleCond);
            pthread_cond_signal(&iterator->threadCond);
            pthread_mutex_unlock(&iterator->threadLock);

            pthread_join(iterator->workerThread, NULL);
            pthread_mutex_destroy(&iterator->threadLock);
            pthread_cond_destroy(&iterator->threadCond);
            pthread_cond_destroy(&iterator->inIdleCond);

            delete &(*iterator);
        }

        DPLOGI("DpJobWorker: destroy all workers in pool end\n");
    }

    DP_STATUS_ENUM runThread(int32_t   rt_priority,
                             WorkerJob currentJob,
                             void      *pArgument,
                             void      **pToken)
    {
        DpTimeValue begin;
        DpTimeValue end;
        int32_t     diff;

        DP_TIMER_GET_CURRENT_TIME(begin);

        acquireWorker(rt_priority,
                      currentJob,
                      pArgument,
                      (WorkerItem**)pToken);

        DPLOGI("DpJobWorker: got the worker (%p)\n", *pToken);

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff);
        if (diff > 10)
        {
            DPLOGW("DpJobWorker: run thread %d ms\n", diff);
        }
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM exitThread(void *token)
    {
        WorkerItem *pWorker;
        DpTimeValue begin;
        DpTimeValue end;
        int32_t     diff;

        DP_TIMER_GET_CURRENT_TIME(begin);

        DPLOGI("DpJobWorker: exit the specified worker (%p) begin\n", token);

        pWorker = (WorkerItem*)token;

        pthread_mutex_lock(&pWorker->threadLock);
        if (STATE_BUSY == pWorker->workerState)
        {
            DPLOGI("DpJobWorker: the worker (%p) state is not in idle state (%d)\n", token, pWorker->workerState);
            pthread_cond_wait(&pWorker->inIdleCond, &pWorker->threadLock);
        }

        if (pWorker->workerState != STATE_STOP)
        {
            pWorker->workerState = STATE_WAIT;
        }

        pthread_mutex_unlock(&pWorker->threadLock);

        DPLOGI("DpJobWorker: exit the specified worker (%p) end\n", token);

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff);
        if (diff > 10)
        {
            DPLOGW("DpJobWorker: exit thread %d ms\n", diff);
        }
        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM joinThread(void           *token,
                              DP_STATUS_ENUM *pStatus)
    {
        WorkerItem *pWorker;
        DpTimeValue begin;
        DpTimeValue end;
        int32_t     diff;

        DP_TIMER_GET_CURRENT_TIME(begin);

        DPLOGI("DpJobWorker: join the specified worker (%p) begin\n", token);

        pWorker = (WorkerItem*)token;

        pthread_mutex_lock(&pWorker->threadLock);
        if (STATE_BUSY == pWorker->workerState)
        {
            DPLOGI("DpJobWorker: the worker (%p) state is not in idle state (%d)\n", token, pWorker->workerState);
            pthread_cond_wait(&pWorker->inIdleCond, &pWorker->threadLock);
        }

        if (pWorker->workerState != STATE_STOP)
        {
            pWorker->workerState = STATE_WAIT;
        }

        *pStatus = pWorker->lastStatus;

        pthread_mutex_unlock(&pWorker->threadLock);

        DPLOGI("DpJobWorker: join the specified worker (%p) end\n", token);

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff);
        if (diff > 10)
        {
            DPLOGW("DpJobWorker: join thread %d ms\n", diff);
        }
        return DP_STATUS_RETURN_SUCCESS;
    }

    int32_t getPriority(void *token)
    {
        WorkerItem *pWorker;
        int32_t    priority;

        pWorker = (WorkerItem*)token;

        pthread_mutex_lock(&pWorker->threadLock);
        priority = pWorker->priority;
        pthread_mutex_unlock(&pWorker->threadLock);

        return priority;
    }

private:
    typedef struct WorkerItem
    {
        DP_STATUS_ENUM  lastStatus;
        STATE_ENUM      workerState;
        pthread_t       workerThread;
        WorkerJob       currentJob;
        void            *pArgument;
        pthread_cond_t  threadCond;
        pthread_cond_t  inIdleCond;
        pthread_mutex_t threadLock;
        int32_t         priority;
        char            workerName[16];
    } WorkerItem;

    typedef ListPtr<WorkerItem> WorkerList;

    WorkerList m_workerList;
    DpMutex    m_workerLock;
    int32_t    m_workerIndex;

    static void* workerLoop(void *pInput)
    {
        DP_STATUS_ENUM status;
        WorkerItem     *pWorker;
        STATE_ENUM     state;
        WorkerJob      pJob;
        void           *pArg;

        assert(0 != pInput);

        pWorker = (WorkerItem*)pInput;

        pthread_mutex_lock(&pWorker->threadLock);
        state = pWorker->workerState;
        pJob  = pWorker->currentJob;
        pArg  = pWorker->pArgument;
        pthread_mutex_unlock(&pWorker->threadLock);

        DPLOGI("DpJobWorker: worker %p enters worker loop\n", pWorker);
        do
        {
            if (0 != pJob)
            {
                DPLOGI("DpJobWorker: worker %p process job begin\n", pWorker);

                ::prctl(PR_SET_NAME, pWorker->workerName, 0, 0, 0);

                DPLOGI("DpJobWorker: process job: %p, argument: %p\n", pJob, pArg);

                // Process the job
                status = pJob(pArg);

                pthread_mutex_lock(&pWorker->threadLock);

                if (pWorker->workerState != STATE_STOP)
                {
                    pWorker->workerState = STATE_IDLE;
                }
                pWorker->lastStatus  = status;
                pWorker->currentJob  = 0;
                pWorker->pArgument   = 0;

                DPLOGI("DpJobWorker: worker %p process job end, state %d\n", pWorker, pWorker->lastStatus);

                pthread_cond_signal(&pWorker->inIdleCond);
                pthread_mutex_unlock(&pWorker->threadLock);
            }

            pthread_mutex_lock(&pWorker->threadLock);

            if (pWorker->workerState != STATE_BUSY)
            {
                DPLOGI("DpJobWorker: worker %p wait new job\n", pWorker);
                pthread_cond_wait(&pWorker->threadCond, &pWorker->threadLock);
            }

            state = pWorker->workerState;
            pJob  = pWorker->currentJob;
            pArg  = pWorker->pArgument;

            pthread_mutex_unlock(&pWorker->threadLock);
        } while (STATE_STOP != state);

        return NULL;
    }

    void getCurrentPriority(int32_t rt_priority, WorkerItem *pWorker, struct sched_param &threadParam)
    {
        int32_t     schedPolicy;
        DpTimeValue begin;
        DpTimeValue end;
        int32_t     diff;

        DP_TIMER_GET_CURRENT_TIME(begin);

        pthread_getschedparam(pthread_self(), &schedPolicy, &threadParam);

        // DpThread priority should be heigher than user context
        if ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy))
        {
            // For RR case, higher value means higher priority
            if (threadParam.sched_priority < 99)
            {
                threadParam.sched_priority = MIN(rt_priority, threadParam.sched_priority + 1);
            }

            pWorker->priority = RT_TO_CMDQ_PRIO(threadParam.sched_priority);
            DPLOGI("DpJobWorker: real-time, prio %#x, cmdq %d\n", threadParam.sched_priority, pWorker->priority);
        }
        else
        {
            // For non-RR case, lower value means higher priority
            int32_t rc = getpriority(PRIO_PROCESS, 0);

            // Prevent blocking real high priority jobs
            threadParam.sched_priority = 75;

            // Adjustment to make low priority really low
            pWorker->priority = MIN(rt_priority - 49, NICE_TO_CMDQ_PRIO(rc));
            DPLOGI("DpJobWorker: normal, nice %d, cmdq %d\n", rc, pWorker->priority);
        }

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff);
        if (diff > 10)
        {
            DPLOGW("DpJobWorker: get priority (%s) %d ms\n",
                ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", diff);
        }
    }

    void adjustPriority(int32_t rt_priority, WorkerItem *pWorker)
    {
    #ifdef SUPPORT_SETSCHEDPARAM
        int32_t            schedPolicy;
    #endif // SUPPORT_SETSCHEDPARAM
        struct sched_param threadParam;

        getCurrentPriority(rt_priority, pWorker, threadParam);

    #ifdef SUPPORT_SETSCHEDPARAM
        if (pthread_setschedparam(pWorker->workerThread, SCHED_RR, &threadParam))
        {
            pthread_getschedparam(pWorker->workerThread, &schedPolicy, &threadParam);
            DPLOGI("DpJobWorker: adjust schedule fail: %s, prio %#x\n",
                ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", threadParam.sched_priority);
        }
        else
        {
            DPLOGI("DpJobWorker: adjust schedule: real-time, prio %#x\n", threadParam.sched_priority);
        }
    #endif // SUPPORT_SETSCHEDPARAM
    }

    void createThread(int32_t rt_priority, WorkerItem *pWorker)
    {
    #ifdef SUPPORT_SETSCHEDPARAM
        pthread_attr_t     attribute;
    #endif // SUPPORT_SETSCHEDPARAM
        int32_t            schedPolicy;
        struct sched_param threadParam;
        DpTimeValue begin;
        DpTimeValue end;
        int32_t     diff;

        schedPolicy = SCHED_RR;
        getCurrentPriority(rt_priority, pWorker, threadParam);

        memset(pWorker->workerName, 0x0, sizeof(char) * 16);
        sprintf(pWorker->workerName, "MDP-%d", m_workerIndex);
        m_workerIndex ++;

    #ifdef SUPPORT_SETSCHEDPARAM
        pthread_attr_init(&attribute);
        pthread_attr_setschedpolicy(&attribute, schedPolicy);
        pthread_attr_setschedparam(&attribute, &threadParam);

        DP_TIMER_GET_CURRENT_TIME(begin);

        if (pthread_create(&pWorker->workerThread, &attribute, workerLoop, pWorker))
        {
            pthread_create(&pWorker->workerThread, NULL, workerLoop, pWorker);

            pthread_getschedparam(pWorker->workerThread, &schedPolicy, &threadParam);
            DPLOGW("DpJobWorker: create real-time thread fail: %s, prio %#x\n",
                ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", threadParam.sched_priority);
        }
        else
        {
            DPLOGI("DpJobWorker: create thread: real-time, prio %#x\n", threadParam.sched_priority);
        }
    #else
        DP_TIMER_GET_CURRENT_TIME(begin);

        pthread_create(&pWorker->workerThread, NULL, workerLoop, pWorker);

        pthread_getschedparam(pWorker->workerThread, &schedPolicy, &threadParam);
        DPLOGI("DpJobWorker: create thread: %s, prio %#x\n",
            ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", threadParam.sched_priority);
    #endif // SUPPORT_SETSCHEDPARAM

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff);
        if (diff > 10)
        {
            DPLOGW("DpJobWorker: create thread (%s) %d ms\n",
                ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", diff);
        }

    #ifdef SUPPORT_SETSCHEDPARAM
        pthread_attr_destroy(&attribute);
    #endif // SUPPORT_SETSCHEDPARAM
    }

    void acquireWorker(int32_t   rt_priority,
                       WorkerJob currentJob,
                       void      *pArgument,
                       WorkerItem **ppWorker)
    {
        WorkerList::iterator iterator;
        WorkerItem           *pCurNode;

        AutoMutex lock(m_workerLock);

        for(iterator = m_workerList.begin(); iterator != m_workerList.end(); iterator++)
        {
            pthread_mutex_lock(&iterator->threadLock);
            if (STATE_WAIT == iterator->workerState)
            {
                DPLOGI("DpJobWorker: set worker (%p) state to busy state\n", &(*iterator));

                adjustPriority(rt_priority, &(*iterator));

                *ppWorker = &(*iterator);
                iterator->currentJob  = currentJob;
                iterator->pArgument   = pArgument;
                iterator->workerState = STATE_BUSY;
                pthread_cond_signal(&iterator->threadCond);
                pthread_mutex_unlock(&iterator->threadLock);

                DPLOGI("DpJobWorker: current job: %p, argument: %p\n", currentJob, pArgument);
                return;
            }
            pthread_mutex_unlock(&iterator->threadLock);
        }

        pCurNode = new WorkerItem();
        DPLOGD("DpJobWorker: create new worker %p\n", pCurNode);
        assert(0 != pCurNode);

        *ppWorker = pCurNode;
        if (0 != pCurNode)
        {
            pthread_cond_init(&pCurNode->inIdleCond, NULL);
            pthread_cond_init(&pCurNode->threadCond, NULL);
            pthread_mutex_init(&pCurNode->threadLock, NULL);

            DPLOGI("DpJobWorker: current job: %p, argument: %p\n", currentJob, pArgument);

            // Setup thread information
            pCurNode->workerState = STATE_BUSY;
            pCurNode->currentJob  = currentJob;
            pCurNode->pArgument   = pArgument;

            createThread(rt_priority, pCurNode);

            m_workerList.push_back(pCurNode);
        }
    }
};


class DpMultiThread
{
public:
    enum PRIORITY_ENUM // RT priority
    {
        PRIORITY_HIGH   = 95,
        PRIORITY_MEDIUM = 85,
        PRIORITY_LOW    = 50
    };

    DpMultiThread()
        : m_running(false),
          m_token(NULL)
    {
    }

    virtual ~DpMultiThread()
    {
    }

    int32_t priority()
    {
        AutoMutex lock(m_mutex);

        return s_worker.getPriority(m_token);
    }

    DP_STATUS_ENUM run(PRIORITY_ENUM hint = PRIORITY_HIGH)
    {
        DP_STATUS_ENUM status;

        AutoMutex lock(m_mutex);

        if (true == m_running)
        {
            return DP_STATUS_INVALID_STATE;
        }

        status = s_worker.runThread(hint,
                                    _threadLoop,
                                    this,
                                    &m_token);

        m_running = true;

        return status;
    }

    DP_STATUS_ENUM exit()
    {
        void           *token;

        {
            AutoMutex lock(m_mutex);

            if (false == m_running)
            {
                return DP_STATUS_INVALID_STATE;
            }
            token = m_token;
        }

        s_worker.exitThread(token);

        {
            AutoMutex lock(m_mutex);
            m_running = false;
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM join()
    {
        void           *token;
        DP_STATUS_ENUM status;

        {
            AutoMutex lock(m_mutex);

            if (false == m_running)
            {
                return DP_STATUS_INVALID_STATE;
            }
            token = m_token;
        }

        s_worker.joinThread(token,
                            &status);

        {
            AutoMutex lock(m_mutex);
            m_running = false;
        }

        return status;
    }

private:
    static DpJobWorker s_worker;

    bool               m_running;
    DpMutex            m_mutex;
    void               *m_token;

    virtual DP_STATUS_ENUM threadLoop() = 0;

    static DP_STATUS_ENUM _threadLoop(void *pToken)
    {
        DP_STATUS_ENUM status;

        status = ((DpMultiThread*)pToken)->threadLoop();
        DPLOGI("DpMultiThread: finished loop with status %d\n", status);

        return status;
    }
};

#endif // __DP_THREAD_ANDROID_H__
