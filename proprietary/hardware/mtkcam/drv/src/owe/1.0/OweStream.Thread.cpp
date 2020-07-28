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
#define LOG_TAG "OweThread"

#include "OweStream.h"
//#include <mtkcam/iopipe/PostProc/IHalOwePipe.h>


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

#include <engine_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(OweThread);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (OweThread_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (OweThread_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (OweThread_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (OweThread_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (OweThread_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (OweThread_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSOwe;

//pthread_mutex_t OweMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
//nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.
pthread_t           OweStream::mThread;
list<ECmd>          OweStream::mCmdList;
MINT32              OweStream::mTotalCmdNum;
sem_t               OweStream::mSemOweThread;
Mutex               OweStream::mOweCmdLock;
Condition           OweStream::mCmdListCond;


/******************************************************************************
 *

******************************************************************************/
MVOID
OweStream::createThread()
{
    resetDequeVariables();
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *

******************************************************************************/
MVOID
OweStream::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and
    clearCommands();
    ECmd cmd=ECmd_UNINIT;
    addCommand(cmd);
}


MINT32 OweStream::getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/******************************************************************************
 *

******************************************************************************/
MVOID
OweStream::
addCommand(ECmd const &cmd)
{
    Mutex::Autolock autoLock(mOweCmdLock);

    //[1] add specific command to cmd list
    mCmdList.push_back(cmd);
    LOG_DBG("mCmdList, size(%zu) + cmd(%d)", mCmdList.size(), cmd);
    mTotalCmdNum++;
    LOG_DBG("mTotalCmdNum_1(%d)",mTotalCmdNum);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mCmdListCond.broadcast();
}


/******************************************************************************
 *

******************************************************************************/
MBOOL
OweStream::
getCommand(
    ECmd &cmd)
{
    Mutex::Autolock autoLock(mOweCmdLock);
    //[1] check if there is command in cmd list
    LOG_DBG("TEST mTotalCmdNum(%d)",mTotalCmdNum);
    if (mTotalCmdNum <= 0)
    {   //no more cmd in cq1/cq2/cq3 dequeueCmd list, block wait for user add cmd
        LOG_DBG("no more cmd, block wait");
        mCmdListCond.wait(mOweCmdLock);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mCmdList.size()>0)
    {
        LOG_DBG("mCmdList curSize(%zu), todo(%d)", mCmdList.size(), *mCmdList.begin());
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
OweStream::
clearCommands()
{
    Mutex::Autolock autoLock(mOweCmdLock);
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
//#define OWE_INT_WAIT_TIMEOUT_MS (3000)
#define OWE_INT_WAIT_TIMEOUT_MS (10000) //Early Poting is slow.

MVOID*
OweStream::
onThreadLoop(
    MVOID *arg)
{
    OCCRequest OCCReqVal;
    WMFERequest WMFEReqVal;
    MUINT32 time1=0,time2=0,i;
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"OweThread",0,0,0);
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
    OweStream *_this = reinterpret_cast<OweStream*>(arg);
    ECmd cmd;

    //list<MAP_PORT_INFO>::iterator iterMapPort;
    //for(iterMapPort = mlMapPort[bufQueIdx].begin(); iterMapPort != mlMapPort[bufQueIdx].end(); iterMapPort++)

    while(_this->getCommand(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        printf("cmd(%d)",cmd);
        switch(cmd)
        {
            case ECmd_OCC_ENQUE:
                printf("OWE Get ECmd_OCC_ENQUE\n");
                if (MTRUE == _this->m_pOweDrv->waitOWEFrameDone(OWE_OCC_INT_ST, OWE_INT_WAIT_TIMEOUT_MS))
                {
                    //Check OCC deque
                    vector<OCCConfig> OccConfigVec;
                    _this->m_pOweDrv->dequeOCC(OccConfigVec);
                    printf("_this->m_QueueOCCReqList.size():%zu,!!", m_QueueOCCReqList.size());
                    if (m_QueueOCCReqList.size()>0)
                    {
                        //Protect the Global variable (critical section)
                        _this->getLock();
                        list<OCCRequest>::iterator it =  m_QueueOCCReqList.begin();
                        OCCReqVal = (*it);
                        vector<OCCConfig>::iterator iterEnqueReq = OCCReqVal.m_OCCParams.mOCCConfigVec.begin();
                        vector<OCCConfig>::iterator iterDequeReq = OccConfigVec.begin();
                        m_QueueOCCReqList.pop_front();
                        _this->releaseLock();
                        //check enque and dequq size is equal or not ? the size must be equal
                        if (OccConfigVec.size() == OCCReqVal.m_OCCParams.mOCCConfigVec.size())
                        {
                            //Update the OCC Statistical Value
                            if(OCCReqVal.m_OCCParams.mpfnCallback == NULL)
                            {
                                printf("WMFE deque no callback");
                                LOG_DBG("WMFE deque no callback");
                                OweStream* pOweStream = (OweStream*)OCCReqVal.m_pOweStream;
                                pOweStream->m_OccParams.mOCCConfigVec.clear();
                                for(; iterEnqueReq != OCCReqVal.m_OCCParams.mOCCConfigVec.end(); iterEnqueReq++, iterDequeReq++)
                                {
#ifdef FEEDBACK 
                                    (*iterEnqueReq).Occ_Vert_Sv = (*iterDequeReq).Occ_Vert_Sv;
                                    (*iterEnqueReq).Occ_Horz_Sv = (*iterDequeReq).Occ_Horz_Sv;
#endif
                                    pOweStream->m_OccParams.mOCCConfigVec.push_back((*iterEnqueReq));
                                }
                                printf("wake OCC Main Thread!!");
                                pOweStream->mOccDequeCond.broadcast(); //wake the wait thread.
                            }
                            else
                            {
                                printf("OCC deque callback");
                                LOG_DBG("OCC deque callback");
                                for(; iterEnqueReq != OCCReqVal.m_OCCParams.mOCCConfigVec.end(); iterEnqueReq++, iterDequeReq++)
                                {
#ifdef FEEDBACK
                                    (*iterEnqueReq).Occ_Vert_Sv = (*iterDequeReq).Occ_Vert_Sv;
                                    (*iterEnqueReq).Occ_Horz_Sv = (*iterDequeReq).Occ_Horz_Sv;
#endif
                                }

                                time1=_this->getUs();
                                CAM_TRACE_BEGIN("occ callback");
                                OCCReqVal.m_OCCParams.mpfnCallback(OCCReqVal.m_OCCParams);
                                CAM_TRACE_END();

                                time2=_this->getUs();

                                LOG_DBG("OCC callbackT(%d us)",time2-time1);
                            }
                        }
                        else
                        {
                            LOG_ERR("OCC Error !!, Deque size(%zu) is not equal to Enque size(%zu)",OccConfigVec.size(), OCCReqVal.m_OCCParams.mOCCConfigVec.size());
                        }
                    }
                    else
                    {
                        printf("error!!, receive the interrupt but the _this->m_QueueOCCReqList.size() is zero!!");
                        LOG_ERR("OCC Error !!, m_QueueOCCReqList size(%zu) is zero!!", m_QueueOCCReqList.size());
                    }
                }
                break;

            case ECmd_WMFE_ENQUE:
                printf("OWE Get ECmd_WMFE_ENQUE\n");
                if (MTRUE == _this->m_pOweDrv->waitOWEFrameDone(OWE_WMFE_INT_ST, OWE_INT_WAIT_TIMEOUT_MS))
                {
                    //Check WMFE deque
                    vector<OWMFEConfig> WmfeConfigVec;
                    _this->m_pOweDrv->dequeWMFE(WmfeConfigVec);
                    list<WMFERequest>::iterator iterWmfeReq;
                    printf("_this->m_QueueWMFEReqList.size():%zu,!!", m_QueueWMFEReqList.size());
                    if (m_QueueWMFEReqList.size()>0)
                    {
                        //Protect the Global variable (critical section)
                        _this->getLock();
                        iterWmfeReq = m_QueueWMFEReqList.begin();
                        WMFEReqVal = (*iterWmfeReq);

                        m_QueueWMFEReqList.pop_front();
                        _this->releaseLock();
                        //check enque and dequq size is equal or not ? the size must be equal
                        if (WmfeConfigVec.size() == WMFEReqVal.m_WMFEParams.mWMFEConfigVec.size())
                        {
                            if(WMFEReqVal.m_WMFEParams.mpfnCallback == NULL)
                            {
                                LOG_DBG("WMFE deque no callback");
                                printf("WMFE deque no callback");
                                OweStream* pOweStream = (OweStream*)WMFEReqVal.m_pOweStream;
                                printf("wake WMFE Main Thread!!");
                                pOweStream->mWmfeDequeCond.broadcast(); //wake the wait thread.
                            }
                            else
                            {
                                LOG_DBG("WMFE deque callback");
                                printf("WMFE deque has callback");
                                time1=_this->getUs();
                                CAM_TRACE_BEGIN("wmfe callback");
                                WMFEReqVal.m_WMFEParams.mpfnCallback(WMFEReqVal.m_WMFEParams);
                                CAM_TRACE_END();
                                time2=_this->getUs();
                                LOG_DBG("WMFE callbackT(%d us)",time2-time1);
                            }
                        }
                        else
                        {
                            LOG_ERR("WMFE Error !!, Deque size(%zu) is not equal to Enque size(%zu)",WmfeConfigVec.size(), WMFEReqVal.m_WMFEParams.mWMFEConfigVec.size());
                        }
                    }
                    else
                    {
                        printf("error!!, receive the interrupt but the _this->m_QueueWMFEReqList.size() is zero!!");
                        LOG_ERR("WMFE Error !!, m_QueueWMFEReqList size(%zu) is zero!!", m_QueueWMFEReqList.size());
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
    ::sem_post(&(mSemOweThread));
    LOG_INF("-");
    return NULL;
}




/******************************************************************************
*
******************************************************************************/
MVOID
OweStream::
getLock()
{
    this->mModuleMtx.lock();
}

/******************************************************************************
*
******************************************************************************/
MVOID
OweStream::
releaseLock()
{
    this->mModuleMtx.unlock();
}



/******************************************************************************
 *

******************************************************************************/
MBOOL
OweStream::resetDequeVariables()
{
    //clear and free mCmdList
    list<ECmd>().swap(mCmdList);
    mTotalCmdNum=0;
    return true;
}


