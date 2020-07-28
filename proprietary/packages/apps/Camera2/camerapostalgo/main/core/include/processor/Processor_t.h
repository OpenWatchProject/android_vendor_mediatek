/*
 * Processor_t.h
 *
 *  Created on: Jun 18, 2019
 *      Author: mtk54498
 */

#ifndef MAIN_CORE_INCLUDE_PROCESSOR_T_H_
#define MAIN_CORE_INCLUDE_PROCESSOR_T_H_


#include <queue>
#include <string>
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include "header_base.h"

namespace NSPA
{

template <typename Init_T, typename Config_T, typename Enque_T>
class Processor
{
public:
    Processor(const std::string &name);
    Processor(const std::string &name, MINT32 policy, MINT32 priority);
    virtual ~Processor();
    const char* getName() const;
    MBOOL setEnable(MBOOL enable);
    MBOOL isEnabled() const;
    MBOOL setNeedThread(MBOOL isThreadNeed);
    MVOID setIdleWaitMS(MUINT32 msec);
    MBOOL init(const Init_T &param);
    MVOID uninit();
    MBOOL config(const Config_T &param);
    MBOOL enque(const Enque_T &param);
    MVOID flush();
    MVOID notifyFlush();
    MVOID waitFlush();
    std::string getStatus();

protected:
    virtual MBOOL onInit(const Init_T &param) = 0;
    virtual MVOID onUninit() = 0;
    virtual MVOID onThreadStart() = 0;
    virtual MVOID onThreadStop() = 0;
    virtual MBOOL onConfig(const Config_T &param) = 0;
    virtual MBOOL onEnque(const Enque_T &param) = 0;
    virtual MVOID onNotifyFlush() = 0;
    virtual MVOID onWaitFlush() = 0;
    virtual MVOID onIdle() {} // It will NOT be called if mNeedThread is false.
    virtual std::string onGetStatus() {return std::string();}

private:
    class ProcessThread : virtual public android::Thread
    {
    public:
        ProcessThread(Processor *parent, MBOOL needThread);
        virtual ~ProcessThread();
        android::status_t readyToRun();
        bool threadLoop();

        MVOID enque(const Enque_T &param);
        MVOID flush();
        MVOID stop();

    private:
        enum WaitResult { WAIT_OK, WAIT_ERROR, WAIT_IDLE, WAIT_EXIT };
        WaitResult waitEnqueParam(Enque_T &param);

    private:
        Processor *mParent;
        const std::string mName;
        nsecs_t mIdleWaitTime;
        android::Mutex mMutex;
        android::Condition mCondition;
        MBOOL mStop;
        MBOOL mIdle;
        MBOOL mNeedThread;
        std::queue<Enque_T> mQueue;
    };

protected:
    const std::string mName;
private:
    android::Mutex mThreadMutex;
    android::Mutex mProcessorMutex;
    android::sp<ProcessThread> mThread;
    MINT32 mThreadPolicy;
    MINT32 mThreadPriority;
    MBOOL mEnable;
    MUINT32 mIdleWaitMS;
    MBOOL mNeedThread;

};

} // namespace P2


#endif /* MAIN_CORE_INCLUDE_PROCESSOR_T_H_ */
