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
#define LOG_TAG "RscThread"

#include "RscStream.h"
//#include <mtkcam/iopipe/PostProc/IHalRscPipe.h>


//thread
#include <utils/threads.h>
#include <mtkcam/def/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>
//#include <vector>
//#include <list>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(RscThread);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (RscThread_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (RscThread_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (RscThread_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (RscThread_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (RscThread_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (RscThread_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSRsc;

//pthread_mutex_t RscMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
//nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.
pthread_t           RscStream::mThread;
list<ECmd>          RscStream::mCmdList;
MINT32              RscStream::mTotalCmdNum;
sem_t               RscStream::mSemRscThread;
Mutex               RscStream::mRscCmdLock;
Condition           RscStream::mCmdListCond;


/******************************************************************************
 *

******************************************************************************/
MVOID
RscStream::createThread()
{
    resetDequeVariables();
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *

******************************************************************************/
MVOID
RscStream::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and
    clearCommands();
    ECmd cmd=ECmd_UNINIT;
    addCommand(cmd);
}


MINT32 RscStream::getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/******************************************************************************
 *

******************************************************************************/
MVOID
RscStream::
addCommand(ECmd const &cmd)
{
    Mutex::Autolock autoLock(mRscCmdLock);

    //[1] add specific command to cmd list
    mCmdList.push_back(cmd);
    LOG_DBG("mCmdList, size(%zd) + cmd(%d)", mCmdList.size(), cmd);
    mTotalCmdNum++;
    LOG_DBG("mTotalCmdNum_1(%d)",mTotalCmdNum);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mCmdListCond.broadcast();
}


/******************************************************************************
 *

******************************************************************************/
MBOOL
RscStream::
getCommand(
    ECmd &cmd)
{
    Mutex::Autolock autoLock(mRscCmdLock);
    //[1] check if there is command in cmd list
    LOG_DBG("TEST mTotalCmdNum(%d)",mTotalCmdNum);
    if (mTotalCmdNum <= 0)
    {   //no more cmd in cq1/cq2/cq3 dequeueCmd list, block wait for user add cmd
        LOG_DBG("no more cmd, block wait");
        mCmdListCond.wait(mRscCmdLock);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mCmdList.size()>0)
    {
        LOG_DBG("mCmdList curSize(%zd), todo(%d)", mCmdList.size(), *mCmdList.begin());
        cmd = *mCmdList.begin();
        mCmdList.erase(mCmdList.begin());
        mTotalCmdNum--;
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ECmd_UNINIT)
        {
            return MFALSE;
        }
        else
        {
            return MTRUE;
        }
    }
    else
    {
        cmd=ECmd_UNKNOWN;
        return MTRUE;
    }
}

/******************************************************************************
 *

******************************************************************************/
MVOID
RscStream::
clearCommands()
{
    Mutex::Autolock autoLock(mRscCmdLock);
    //[1] erase all the command in cmd list
    for (list<ECmd>::iterator it = mCmdList.begin(); it != mCmdList.end();it++)
    {
        it = mCmdList.erase(it);
    }
    mTotalCmdNum=0;
}


/******************************************************************************
 *
******************************************************************************/
#define PR_SET_NAME 15
//#define RSC_INT_WAIT_TIMEOUT_MS (3000)
#define RSC_INT_WAIT_TIMEOUT_MS (10000) //Early Poting is slow.

MVOID*
RscStream::
onThreadLoop(
    MVOID *arg)
{
    RSCRequest RSCReqVal;
    MUINT32 time1=0,time2=0;
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"RscThread",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_PASS2;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if(policy == SCHED_OTHER)
    {   //  Note: "priority" is nice-value priority.
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);
    }
    else
    {   //  Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do dequeue buf if there is enqueue cmd in cmd list
    RscStream *_this = reinterpret_cast<RscStream*>(arg);
    ECmd cmd;

    //list<MAP_PORT_INFO>::iterator iterMapPort;
    //for(iterMapPort = mlMapPort[bufQueIdx].begin(); iterMapPort != mlMapPort[bufQueIdx].end(); iterMapPort++)

    while(_this->getCommand(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        printf("cmd(%d)",cmd);
        switch(cmd)
        {
            case ECmd_RSC_ENQUE:
                printf("RSC Get ECmd_RSC_ENQUE\n");
                if (MTRUE == _this->m_pRscDrv->waitRSCFrameDone(RSC_INT_ST, RSC_INT_WAIT_TIMEOUT_MS))
                {
                    //Check RSC deque
                    vector<RSCConfig> RscConfigVec;
                    _this->m_pRscDrv->dequeRSC(RscConfigVec);
                    list<RSCRequest>::iterator iterRscReq;
                    printf("_this->m_QueueRSCReqList.size():%zd,!!", m_QueueRSCReqList.size());
                    if (m_QueueRSCReqList.size()>0)
                    {
                        //Protect the Global variable (critical section)
                        _this->getLock();
                        iterRscReq = _this->m_QueueRSCReqList.begin();
                        RSCReqVal = (*iterRscReq);

                        _this->m_QueueRSCReqList.pop_front();
                        _this->releaseLock();

                        vector<RSCConfig>::iterator enq = RSCReqVal.m_RSCParams.mRSCConfigVec.begin();
                        vector<RSCConfig>::iterator deq = RscConfigVec.begin();

                        //check enque and dequq size is equal or not ? the size must be equal
                        if (RscConfigVec.size() == RSCReqVal.m_RSCParams.mRSCConfigVec.size())
                        {
                            if(RSCReqVal.m_RSCParams.mpfnCallback == NULL)
                            {
                                LOG_DBG("RSC deque no callback");
                                printf("RSC deque no callback");
                                RscStream* pRscStream = (RscStream*)RSCReqVal.m_pRscStream;
                                printf("wake RSC Main Thread!!");
                                pRscStream->mRscDequeCond.broadcast(); //wake the wait thread.
                            }
                            else
                            {
                                LOG_DBG("RSC deque callback");
                                printf("RSC deque has callback");

                                for(;deq != RscConfigVec.end(); enq++, deq++)
                                    (*enq).feedback = (*deq).feedback;

                                time1=_this->getUs();
                                DRV_TRACE_BEGIN("rsc callback");
                                RSCReqVal.m_RSCParams.mpfnCallback(RSCReqVal.m_RSCParams);
                                DRV_TRACE_END();
                                time2=_this->getUs();
                                LOG_DBG("RSC callbackT(%d us)",time2-time1);
                            }
                        }
                        else
                        {
                            LOG_ERR("RSC Error !!, Deque size(%zd) is not equal to Enque size(%zd)",RscConfigVec.size(), RSCReqVal.m_RSCParams.mRSCConfigVec.size());
                        }
                    }
                    else
                    {
                        printf("error!!, receive the interrupt but the _this->m_QueueRSCReqList.size() is zero!!");
                        LOG_ERR("RSC Error !!, m_QueueRSCReqList size(%zd) is zero!!", m_QueueRSCReqList.size());
                    }

                }
                break;
            case ECmd_UNINIT:
                goto EXIT;
                break;
            default:
                break;
        }
        cmd=ECmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearCommands();
    ::sem_post(&mSemRscThread);
    LOG_INF("-");
    return NULL;
}




/******************************************************************************
*
******************************************************************************/
MVOID
RscStream::
getLock()
{
    this->mModuleMtx.lock();
}

/******************************************************************************
*
******************************************************************************/
MVOID
RscStream::
releaseLock()
{
    this->mModuleMtx.unlock();
}



/******************************************************************************
 *

******************************************************************************/
MBOOL
RscStream::resetDequeVariables()
{
    //clear and free mCmdList
    list<ECmd>().swap(mCmdList);
    mTotalCmdNum=0;
    return true;
}


