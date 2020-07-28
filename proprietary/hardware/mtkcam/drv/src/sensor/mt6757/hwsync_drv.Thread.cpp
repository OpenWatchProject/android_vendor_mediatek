/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "Drv/HWsync.TD"

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
//thread
#include <utils/threads.h>
#include <semaphore.h>
#include <mtkcam/def/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <utils/ThreadDefs.h>
#include <sys/prctl.h>
// For property_get().
#include <cutils/properties.h>
#include "hwsync_drv_imp.h"
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG

//#include "../../core/drv/inc/drv_log.h"    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include "drv_log.h"
DECLARE_DBG_LOG_VARIABLE(hwsyncDrvTD);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (hwsyncDrvTD_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

extern sem_t gsemHWSyncLv;
extern pthread_mutex_t mStateMutex;
extern pthread_mutex_t mEnterIdleMutex;
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::createThread()
{
    pthread_create(&mThreadMain1, NULL, onThreadLoopMain1, this);
    pthread_create(&mThreadMain2, NULL, onThreadLoopMain2, this);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and add uninit
    clearExecCmdsMain1();
    clearExecCmdsMain2();
    ExecCmd cmd=ExecCmd_LEAVE;
    addExecCmdMain1(cmd);
    addExecCmdMain2(cmd);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::
addExecCmdMain1(
    ExecCmd const &cmd)
{
    //LOG_INF("+");
    Mutex::Autolock autoLock(mThreadMtxMain1);
    //[1] add specific command to cmd list
    //LOG_DBG("mCmdList, size(%d) + cmd(%d)", mExecCmdListMain1.size(), cmd);
    mExecCmdListMain1.push_back(cmd);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mExecCmdListCondMain1.broadcast();
}

MVOID
HWSyncDrvImp::
addExecCmdMain2(
    ExecCmd const &cmd)
{
    //LOG_INF("+");
    Mutex::Autolock autoLock(mThreadMtxMain2);
    //[1] add specific command to cmd list
    //LOG_DBG("mCmdList, size(%d) + cmd(%d)", mExecCmdListMain2.size(), cmd);
    mExecCmdListMain2.push_back(cmd);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mExecCmdListCondMain2.broadcast();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::
clearExecCmdsMain1()
{
    Mutex::Autolock autoLock(mThreadMtxMain1);
    //[1] erase all the command in cmd list
    for (list<ExecCmd>::iterator it = mExecCmdListMain1.begin(); it != mExecCmdListMain1.end();)
    {
        it = mExecCmdListMain1.erase(it);
        it++;
    }
}

MVOID
HWSyncDrvImp::
clearExecCmdsMain2()
{
    Mutex::Autolock autoLock(mThreadMtxMain2);
    //[1] erase all the command in cmd list
    for (list<ExecCmd>::iterator it = mExecCmdListMain2.begin(); it != mExecCmdListMain2.end();)
    {
        it = mExecCmdListMain2.erase(it);
        it++;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HWSyncDrvImp::
get1stCmdMain1(
    ExecCmd &cmd)
{
    Mutex::Autolock autoLock(mThreadMtxMain1);
    //[1] check if there is command in cmd list
    if (mExecCmdListMain1.size() <= 0)
    {
        //LOG_INF("no more cmd, wait here");
        mExecCmdListCondMain1.wait(mThreadMtxMain1);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mExecCmdListMain1.size()>0)
    {
        //LOG_INF("get 1st cmd(%d)",*mExecCmdListMain1.begin());
        cmd = *mExecCmdListMain1.begin();
        mExecCmdListMain1.erase(mExecCmdListMain1.begin());
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ExecCmd_LEAVE)
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
        cmd=ExecCmd_UNKNOWN;
        return MTRUE;
    }
}

MBOOL
HWSyncDrvImp::
get1stCmdMain2(
    ExecCmd &cmd)
{
    Mutex::Autolock autoLock(mThreadMtxMain2);
    //[1] check if there is command in cmd list
    if (mExecCmdListMain2.size() <= 0)
    {
        //LOG_INF("no more cmd, wait here");
        mExecCmdListCondMain2.wait(mThreadMtxMain2);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mExecCmdListMain2.size()>0)
    {
        //LOG_INF("get 1st cmd(%d)",*mExecCmdListMain2.begin());
        cmd = *mExecCmdListMain2.begin();
        mExecCmdListMain2.erase(mExecCmdListMain2.begin());
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ExecCmd_LEAVE)
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
        cmd=ExecCmd_UNKNOWN;
        return MTRUE;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
#define PR_SET_NAME 15
MVOID* HWSyncDrvImp::onThreadLoopMain1(MVOID *arg)
{
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"HWSyncDrvThreadMain1",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_PASS1;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do sync if there is enqueue cmd in cmd list
    HWSyncDrvImp *_this = reinterpret_cast<HWSyncDrvImp*>(arg);
    ExecCmd cmd;
    ExecCmd nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN1;

    while(_this->get1stCmdMain1(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        switch(cmd)
        {
            case ExecCmd_SET_SYNC_FRMTIME_MAIN1:
                if(_this->setSyncFrmTimeMain1() < 0){
                    LOG_ERR("Set sync frame time failed!");
                }
                break;
            case ExecCmd_UNKNOWN:
            default:
                LOG_ERR("fail cmd_%d\n",cmd);
                break;
        }
        cmd=ExecCmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearExecCmdsMain1();
    //send leave signal
    ::sem_post(&(_this->mSemHWSyncLvMain1));
    LOG_INF("-");
    return NULL;
}

MVOID* HWSyncDrvImp::onThreadLoopMain2(MVOID *arg)
{
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"HWSyncDrvThreadMain2",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_PASS1;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do sync if there is enqueue cmd in cmd list
    HWSyncDrvImp *_this = reinterpret_cast<HWSyncDrvImp*>(arg);
    ExecCmd cmd;
    ExecCmd nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN2;

    while(_this->get1stCmdMain2(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        switch(cmd)
        {
            case ExecCmd_SET_SYNC_FRMTIME_MAIN2:
                if(_this->setSyncFrmTimeMain2() < 0){
                    LOG_ERR("Set sync frame time failed!");
                }
                break;
            case ExecCmd_UNKNOWN:
            default:
                LOG_ERR("fail cmd_%d\n",cmd);
                break;
        }
        cmd=ExecCmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearExecCmdsMain2();
    //send leave signal
    ::sem_post(&(_this->mSemHWSyncLvMain2));
    LOG_INF("-\n");
    return NULL;
}


