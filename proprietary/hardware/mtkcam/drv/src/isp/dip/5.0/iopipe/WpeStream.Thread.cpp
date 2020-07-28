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
#define LOG_TAG "WpeThread"

#include "WpeStream.h"

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

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(WpeThread);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (WpeThread_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (WpeThread_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (WpeThread_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (WpeThread_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (WpeThread_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (WpeThread_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSWpe;

//pthread_mutex_t WpeMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
//nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.
pthread_t                                   WpeStream::mThread;
list<NSCam::NSIoPipe::NSWpe::ECmd>          WpeStream::mCmdList;
MINT32                                      WpeStream::mTotalCmdNum;
sem_t                                       WpeStream::mSemWpeThread;
Mutex                                       WpeStream::mWpeCmdLock;
Condition                                   WpeStream::mCmdListCond;
sem_t                                       WpeStream::mWPESemEQDone;


/******************************************************************************
 *

******************************************************************************/
MVOID
WpeStream::createThread()
{
    resetDequeVariables();
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *

******************************************************************************/
MVOID
WpeStream::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and
    clearCommands();
    ECmd cmd=ECmd_UNINIT;
    addCommand(cmd);
}


MINT32 WpeStream::getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/******************************************************************************
 *

******************************************************************************/
MVOID
WpeStream::
addCommand(ECmd const &cmd)
{
    Mutex::Autolock autoLock(mWpeCmdLock);

    //[1] add specific command to cmd list
    mCmdList.push_back(cmd);
    LOG_DBG("mCmdList, size(%d) + cmd(%d)", mCmdList.size(), cmd);
    mTotalCmdNum++;
    LOG_DBG("mTotalCmdNum_1(%d)",mTotalCmdNum);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mCmdListCond.broadcast();
}


/******************************************************************************
 *

******************************************************************************/
MBOOL
WpeStream::
getCommand(
    ECmd &cmd)
{
    Mutex::Autolock autoLock(mWpeCmdLock);
    //[1] check if there is command in cmd list
    LOG_DBG("TEST mTotalCmdNum(%d)",mTotalCmdNum);
    if (mTotalCmdNum <= 0)
    {   //no more cmd in cq1/cq2/cq3 dequeueCmd list, block wait for user add cmd
        LOG_DBG("no more cmd, block wait");
        mCmdListCond.wait(mWpeCmdLock);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mCmdList.size()>0)
    {
        LOG_DBG("mCmdList curSize(%d), todo(%d)", mCmdList.size(), *mCmdList.begin());
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
WpeStream::
clearCommands()
{
    Mutex::Autolock autoLock(mWpeCmdLock);
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
//#define Wpe_INT_WAIT_TIMEOUT_MS (3000)
#define WPE_INT_WAIT_TIMEOUT_MS (10000) //Early Poting is slow.

/******************************************************************************
 *
 ******************************************************************************/
 
MBOOL
WpeStream::
eraseNotify(
	EWPEBufferListTag bufferListTag,
	MINT32 burstIdx,
	MINT32 dupIdx,
	NSImageio::NSIspio::EPIPE_P2BUFQUECmd cmd,
	MUINT32 callerID)
{
    LOG_DBG("+,tag(%d),framenum_dupidx(%d/%d)",bufferListTag,burstIdx,dupIdx);
    MBOOL ret=MTRUE;

    if(bufferListTag==EWPEBufferListTag_Package)
    {  //erase frame package from list 
        this->getWPELock(EWPELockEnum_wDQFramePackList);
		this->getWPELock(EWPELockEnum_wDQFrameUnitList);
		
        LOG_DBG("PKLsize(%d)",this->mLWPEDQFramePackList.size());
		LOG_DBG("PULsize(%d)",this->mLWPEwaitDQFrameUnitList.size());
		
        vector<WPEFramePackage>::iterator it = this->mLWPEDQFramePackList.begin();
        this->mLWPEDQFramePackList.erase(it);

		LOG_DBG("updateFPList(%d)",this->mLWPEDQFramePackList.size());
        for (vector<WPEFramePackage>::iterator t2 = this->mLWPEDQFramePackList.begin(); t2 !=this->mLWPEDQFramePackList.end(); t2++)
        {
            if((*t2).idxofwaitDQFrmPackList>0)
            {(*t2).idxofwaitDQFrmPackList -= 1;}
        }
		
        
        LOG_DBG("updateFUList(%d)",this->mLWPEwaitDQFrameUnitList.size());
        for (list<WPEFrameUnit>::iterator t3 = this->mLWPEwaitDQFrameUnitList.begin(); t3 != this->mLWPEwaitDQFrameUnitList.end(); t3++)
        {
            if((*t3).idxofwaitDQFrmPackList>0)
            {(*t3).idxofwaitDQFrmPackList -= 1;}
        }
		this->releaseWPELock(EWPELockEnum_wDQFrameUnitList);
        this->releaseWPELock(EWPELockEnum_wDQFramePackList);
        //release user
        this->getEQUserLock();
        this->m_wpeEQUserNum--;
		this->releaseEQUserLock();
		LOG_DBG("Release EnqueueBufList Resource");
        ::sem_post(&mWPESemEQDone);	

    }
    else
    {   //erase frame unit from list
        this->getWPELock(EWPELockEnum_wDQFrameUnitList);
        list<WPEFrameUnit>::iterator it = this->mLWPEwaitDQFrameUnitList.begin();
        this->mLWPEwaitDQFrameUnitList.erase(it);     
        this->releaseWPELock(EWPELockEnum_wDQFrameUnitList);
    }

    LOG_DBG("-");
    return ret;
}



MBOOL
WpeStream::
dequeueJudgement()
{
    LOG_INF("+");

	MINT32 deqret;
    bool ret=true;
	bool dequeFramePackDone= false;
    WPEFrameUnit frameunit;
	MINT32 sizeofwaitDequeUnit = 0;
    MINT32 curWaitedDequeBufNodeNum=0;
	MINT32	wpeDupIdx;	   //wpe duplicate index(pinpon here)
	MINT32  wpeBurstIdx;   //wpe burst index (for multi-frames of one EQ request)
	MUINT32 callerID;
	MUINT32 DupCmdIdx;
    MINT32  index; //element index in waitDQFramePackageList
	ISP_WPE_MODULE_IDX hwModule;		   //0: WPE A; 1: WPE B
	MINT32 mdp_err = -1;


	this->getWPELock(EWPELockEnum_wDQFrameUnitList);
	sizeofwaitDequeUnit=(MINT32)(this->mLWPEwaitDQFrameUnitList.size());
	LOG_DBG("dequeueJudgement, sizeofwaitDequeUnit(%d)",sizeofwaitDequeUnit);
	list<WPEFrameUnit>::iterator it = this->mLWPEwaitDQFrameUnitList.begin();
	frameunit = *it;
	this->releaseWPELock(EWPELockEnum_wDQFrameUnitList);

	wpeDupIdx = frameunit.wpeDupIdx;
	wpeBurstIdx = frameunit.wpeBurstIdx;
	callerID = frameunit.callerID;
	hwModule = frameunit.wpeModeIdx;
	DupCmdIdx = frameunit.wpeDupCmdIdx;
	index = frameunit.idxofwaitDQFrmPackList;

	LOG_INF("dequeueJudgement frameunit callerID(0x%x) wpeDupIdx(%d), wpeBurstIdx(%d), DupCmdIdx(%d), WARP_Engine_ID(%d)",
		                          frameunit.callerID, frameunit.wpeDupIdx, frameunit.wpeBurstIdx, frameunit.wpeDupCmdIdx, frameunit.wpeModeIdx);

	mdp_err = this->mwarpengine[hwModule]->dequeue(wpeDupIdx, wpeBurstIdx, DupCmdIdx);
	LOG_DBG("dequeueJudgement, hwModule(%d), mdp_err(%d)", hwModule, mdp_err);

	if(mdp_err == 0)
	{
		int frm;
		//1. judge dequeued buffer number
		index = frameunit.idxofwaitDQFrmPackList;
		LOG_DBG("dequeueJudgement idx(%d)", index);
		this->getWPELock(EWPELockEnum_wDQFramePackList);
		LOG_DBG("dequeueJudgement getWPELock(EWPELockEnum_wDQFramePackList)");

		frm = this->mLWPEDQFramePackList[index].dequedNum;

		if(this->mLWPEDQFramePackList[index].rParams.mvFrameParams[wpeBurstIdx].mvOut.size()>0)
		{
			LOG_DBG("WPE(0x%x)_FmvOut(0x%x),idx(%d)",frameunit.callerID, this->mLWPEDQFramePackList[index].rParams.mvFrameParams[wpeBurstIdx].mvOut[0].mBuffer->getBufPA(0),index);
		}
		else
		{
			LOG_ERR("no output dma, index(%d)",index);
		}
		LOG_DBG("dequeueJudgement mLWPEDQFramePackList[%d]: dequedNum(%d)", index, this->mLWPEDQFramePackList[index].dequedNum);


		this->mLWPEDQFramePackList[index].dequedNum ++;;	 //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)

		LOG_INF("Aftter DQ mLWPEDQFramePackList[%d]:dequedNum(%d), frameNum(%d)", index, this->mLWPEDQFramePackList[index].dequedNum, this->mLWPEDQFramePackList[index].frameNum);
		if(this->mLWPEDQFramePackList[index].dequedNum == this->mLWPEDQFramePackList[index].frameNum)
		{
					dequeFramePackDone = true;
					this->mLWPEDQFramePackList[index].rParams.mDequeSuccess = MTRUE;
		}
		LOG_DBG("dequeueJudgement dequeFramePackDone(%d)",dequeFramePackDone);

		//2. buffer control
		if(!dequeFramePackDone)
		{
			this->releaseWPELock(EWPELockEnum_wDQFramePackList);
			eraseNotify(EWPEBufferListTag_Unit,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
			LOG_INF("+,[Dequeue FramePack is NOT Done]tag(%d),WPE Caller(0x%x), WARP_Engine_ID(%d),framenum_dupidx(%d/%d)",\
						 EWPEBufferListTag_Unit,frameunit.callerID, hwModule,frameunit.wpeBurstIdx,frameunit.wpeDupIdx);
		}
		else
		{
			if(this->mLWPEDQFramePackList[index].rParams.mpfnCallback == NULL)
			{

				this->releaseWPELock(EWPELockEnum_wDQFramePackList);
				LOG_ERR("Not Support Blcking De-Q, WPE Caller(0x%x), ",frameunit.callerID);

				//
				eraseNotify(EWPEBufferListTag_Unit,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
				eraseNotify(EWPEBufferListTag_Package,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
		
				LOG_INF("+,tag(%d),WPE Caller(0x%x),(%d),framenum_dupidx(%d/%d),\n\
							 +,tag(%d),WPE Caller(0x%x),,framenum_dupidx(%d/%d)",\
						EWPEBufferListTag_Unit,frameunit.callerID,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,\
						EWPEBufferListTag_Package,frameunit.callerID,frameunit.wpeBurstIdx,frameunit.wpeDupIdx);
			}
			else
			{
				//callback
				MUINT32 a=0,b=0;
				LOG_DBG("idx(%d)",index);
				WPEFramePackage framepack=this->mLWPEDQFramePackList[index];
				this->releaseWPELock(EWPELockEnum_wDQFramePackList); 
				LOG_DBG("WPE deque done callback (0x%x)=",framepack.rParams.mpfnCallback);
				a=getUs();
				framepack.rParams.mpfnCallback(framepack.rParams);
				b=getUs();
		
				
				eraseNotify(EWPEBufferListTag_Unit,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
				eraseNotify(EWPEBufferListTag_Package,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
				LOG_INF("+,tag(%d),WPE Caller(0x%x),(%d),framenum_dupidx(%d/%d),\n\
							 +,tag(%d),WPE Caller(0x%x),,framenum_dupidx(%d/%d),\n\
							 -,cb(%d us)",\
						EWPEBufferListTag_Unit,frameunit.callerID,frameunit.wpeBurstIdx,frameunit.wpeDupIdx,\
						EWPEBufferListTag_Package,frameunit.callerID,frameunit.wpeBurstIdx,frameunit.wpeDupIdx, b-a);
			
			}
		}
		
	}
	else
		{
			LOG_ERR("WPE Caller(0x%x) framenum_dupidx(%d/%d) WPE dequeMdpFrameEnd fail",frameunit.callerID,frameunit.wpeBurstIdx,frameunit.wpeDupIdx);
			//AEE???
			ret = false ;
		}

	return ret;
}


MVOID*
WpeStream::
onThreadLoop(
    MVOID *arg)
{
    WPERequest WPEReqVal;
    MUINT32 time1=0,time2=0,i;
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"WpeThread",0,0,0);
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
    WpeStream *_this = reinterpret_cast<WpeStream*>(arg);
    ECmd cmd;
	list<WPERequest>::iterator iterWpeReq;
    WPEBufParamNode param;
	
	bool ret=true;



    while(_this->getCommand(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        switch(cmd)
        {
            case ECmd_WPE_ENQUE:
				ret=_this->dequeueJudgement();		
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
    ::sem_post(&mSemWpeThread);
    LOG_INF("-");
    return NULL;
}




/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
getEQUserLock()
{
    this->mMutex_EQUser.lock();
}

/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
releaseEQUserLock()
{
    this->mMutex_EQUser.unlock();
}


/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
getEQLock()
{
    this->mMutex_EQDone.lock();
}

/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
releaseEQLock()
{
    this->mMutex_EQDone.unlock();
}


/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
getWPELock(
    EWPELockEnum lockType)
{
	LOG_DBG("getLock");

    switch(lockType)
    {
        case EWPELockEnum_wDQFrameUnitList:
			this->mMutex_WPEwaitDQFrameUnitList.lock();
            break;
        case EWPELockEnum_wDQFramePackList:
            this->mMutex_WPEwaitDQFramePackList.lock();
            break;
        default:
            break;
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
WpeStream::
releaseWPELock(
    EWPELockEnum lockType)
{
    switch(lockType)
    {
        case EWPELockEnum_wDQFrameUnitList:
            this->mMutex_WPEwaitDQFrameUnitList.unlock();
            break;
        case EWPELockEnum_wDQFramePackList:
            this->mMutex_WPEwaitDQFramePackList.unlock();
            break;
        default:
            break;
    }
	LOG_DBG("releaseLock");
}

/******************************************************************************
 *

******************************************************************************/
MBOOL
WpeStream::resetDequeVariables()
{
    //clear and free mCmdList
    list<ECmd>().swap(mCmdList);
    mTotalCmdNum=0;
    return true;
}


