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
#define LOG_TAG "EGNStream"

#include "EgnStream.h"
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#if 1
//thread
#include <utils/threads.h>
#include <mtkcam/def/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>

#endif
/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(EgnStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (EgnStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (EgnStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (EgnStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (EgnStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (EgnStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (EgnStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSEgn;

template <class T>
EngineDrv<T>* EgnStream<T>::m_pEngineDrv = NULL;

#ifdef DUAL_ENGINE
template <class T2>
EngineDrv<T2>* EgnStream<T2>::m_pEngineDrv2 = NULL;

template <class T2>
list<EGNRequest<T2>>  EgnStream<T2>::m_QueueEGNReqList2; //EGN Req List
#endif

template <class T>
list<EGNRequest<T>>   EgnStream<T>::m_QueueEGNReqList; //EGN Req List
template <class T>
Mutex               EgnStream<T>::mModuleMtx;
template <class T>
MINT32              EgnStream<T>::mInitCount;     //Record the user count

#if 1
template <typename T>
pthread_t           EgnStream<T>::mThread;

template <typename T>
list<ECmd>          EgnStream<T>::mCmdList;

template <typename T>
list<SUB_ENGINE_ID> EgnStream<T>::mEgnList;

template <typename T>
MINT32              EgnStream<T>::mTotalCmdNum;

template <typename T>
sem_t               EgnStream<T>::mSemEgnThread;

template <typename T>
Mutex               EgnStream<T>::mEgnCmdLock;

template <typename T>
Condition           EgnStream<T>::mCmdListCond;
#endif

//Debug Utility

/******************************************************************************
 *
 ******************************************************************************/
static bool DrvMkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
        {
            LOG_ERR("fail to mkdir [%s]: %d[%s]", path, errno, ::strerror(errno));
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        LOG_ERR("!S_ISDIR");
        return  false;
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
static MBOOL DrvMakePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    char*copypath = strdup(path);
    char*pp = copypath;
    char*sp;
    if (pp != NULL) {
        while ( ret && 0 != (sp = strchr(pp, '/')) )
        {
            if (sp != pp)
            {
                *sp = '\0';
                ret = DrvMkdir(copypath, mode);
                *sp = '/';
            }
            pp = sp + 1;
        }
        if (ret) {
            ret = DrvMkdir(path, mode);
        }
        free(copypath);
    }
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
#ifdef FACTORY  
template <class T>
IEgnStream<T>*
NSCam::NSIoPipe::NSEgn::getEgnStreamImp(
    char const* szCallerName)
{
    static EgnStream<T> singleton(szCallerName);
    return &singleton;
}
#else

template <class T>
IEgnStream<T>*
IEgnStream<T>::createInstance(
    char const* szCallerName)
{
    EgnStream<T>* pstream = new EgnStream<T>(szCallerName);
    return pstream;;
}

#endif

/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MVOID
EgnStream<T>::
destroyInstance(
    char const* szCallerName)
{
    (void) szCallerName;
    FUNCTION_LOG_START;


    FUNCTION_LOG_END;
#ifndef FACTORY  
    delete this;
#endif
}
/*******************************************************************************
*
********************************************************************************/
template <class T>
EgnStream<T>::
EgnStream(char const* szCallerName)
{
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        strncpy((char*)m_UserName,(char const*)szCallerName, strlen(szCallerName));
	m_UserName[strlen(szCallerName)] = '\0';
    }
    else if (szCallerName != NULL)
    {
        LOG_ERR("CallerName(%s), CallerNameLen(%zd), mEgnInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
    }
    m_EgnStreamInit = MFALSE;
    m_iEgnEnqueReq[eENGINE_MAX]= {0};
    m_bSaveReqToFile = MFALSE;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
    DBG_LOG_CONFIG(drv, EgnStream);

}

/*******************************************************************************
*
********************************************************************************/
template <class T>
EgnStream<T>::~EgnStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    char szBuf[MAX_USER_NAME_SIZE];
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int n;
    int32_t old_cnt;
    //[1] increase user count and record user
    old_cnt = android_atomic_inc(&mInitCount);
    LOG_INF("mEgnInitCnt(%d)",mInitCount);

    property_get("vendor.cam.egnsavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mSemEgnThread, 0, 0);
	m_pEngineDrv = EngineDrv<T>::createInstance();

        //create dequeue thread
        createThread();
        m_QueueEGNReqList.clear();
    }

    //sprintf(m_UserName,"egnStream_%d",mInitCount);
    if (MFALSE == m_EgnStreamInit)
    {
        sprintf(szBuf,"_egn_%d",mInitCount);
        strncat(m_UserName, szBuf, strlen(szBuf));

        m_pEngineDrv->init(m_UserName);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        m_iEgnEnqueReq[eENGINE_MAX] = {0};
        m_EgnStreamInit = MTRUE;
    }



    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    int32_t old_cnt;

    //[1] decrease user count and record user
    old_cnt = android_atomic_dec(&mInitCount);
    LOG_INF("mEgnUnitCnt(%d)",mInitCount);

    if (MTRUE == m_EgnStreamInit)
    {
        m_pEngineDrv->uninit(m_UserName);
        strncpy(m_UserName,"",MAX_USER_NAME_SIZE);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_EgnStreamInit = MFALSE;
        m_iEgnEnqueReq[eENGINE_MAX] = {0};
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {
        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemEgnThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
saveToFile(char const* filepath, EGNBufInfo* buffer)
{
    MBOOL ret = MFALSE;
    int fd = -1;

    //
    LOG_DBG("save to %s", filepath);
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if  ( fd < 0 )
    {
        LOG_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        MUINT8* pBuf = (MUINT8*)buffer->u4BufVA;
        size_t  size = buffer->u4BufSize;
        size_t  written = 0;
        int nw = 0, cnt = 0;
        while ( written < size )
        {
            nw = ::write(fd, pBuf+written, size-written);
            if  (nw < 0)
            {
                LOG_ERR(
                    "fail to write %s, write-count:%d, written-bytes:%zd : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        LOG_DBG("write %zd bytes to %s", size, filepath);
    }
    //
    ret = MTRUE;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    return  ret;

}

/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
EGNenque(EGNParams<T> const& rEgnParams)
{
    DRV_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_EGNPREFIX "/sdcard/egndump_egn"

#define DUMP_EGNBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        EGNBufInfo* buffer = (EGNBufInfo*)pbuf;                \
        EgnStream* pStream = (EgnStream*)stream;               \
        char filename[256];                                    \
        sprintf(filename, "%s%s%s_%d_%d.%s",                   \
                dumppath,                                      \
                #type,                                         \
                dir,                                           \
                buffer->u4BufSize,                             \
                cnt,                                           \
                fileExt                                        \
               );                                              \
        pStream->saveToFile(filename, buffer);                 \
    }while(0)


    char dumppath[256];
    if (MTRUE == m_bSaveReqToFile)
    {
        sprintf( dumppath, "%s/", DUMP_EGNPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("EGN makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }

    bool ret = true;

    if (static_cast<SUB_ENGINE_ID>(rEgnParams.mpEngineID) >= eENGINE_MAX){
        LOG_ERR("Invalid engine ID used:%d, enque failed.", rEgnParams.mpEngineID);
        return false;
    }

    ECmd cmd;
    if(m_pEngineDrv->queryCmd(rEgnParams.mpEngineID, cmd) != 0) {
        LOG_ERR("Invalid engine(%d) cmd(%d) NOT found, enque failed.", rEgnParams.mpEngineID, cmd);
        return false;
    }
    LOG_DBG("Engine(%d) used Cmd(%d)\n", rEgnParams.mpEngineID, cmd);

    EGNRequest<T> egnreq;
    T egnconfig;
    typename vector<T>::const_iterator iter;

    egnreq.m_Num = rEgnParams.mEGNConfigVec.size(); //Request Number.
    egnreq.m_pEgnStream = (void*) this;
    egnreq.m_EGNParams.mpfnCallback = rEgnParams.mpfnCallback;
    egnreq.m_EGNParams.mpCookie = rEgnParams.mpCookie;
    egnreq.m_EGNParams.mpEngineID = rEgnParams.mpEngineID;

    iter = rEgnParams.mEGNConfigVec.begin();

    for (;iter<rEgnParams.mEGNConfigVec.end();iter++) {
        egnconfig = (*iter);
        if (MTRUE == m_bSaveReqToFile)
        {
#if 0
            if (MTRUE == egnconfig.Egn_Ctrl_0.Egn_Enable)
            {
                DUMP_EGNBuffer(this, &egnconfig.Egn_Ctrl_0.Egn_Imgi, DMA_EGN_IMGI , "egn0" , m_iEgnEnqueReq , "dat");
                DUMP_EGNBuffer(this, &egnconfig.Egn_Ctrl_0.Egn_Dpi, DMA_EGN_DPI , "egn0" , m_iEgnEnqueReq , "dat");
                DUMP_EGNBuffer(this, &egnconfig.Egn_Ctrl_0.Egn_Tbli, DMA_EGN_TBLI , "egn0" , m_iEgnEnqueReq , "dat");
                DUMP_EGNBuffer(this, &egnconfig.Egn_Ctrl_0.Egn_Dpo, DMA_EGN_DPO , "egn0" , m_iEgnEnqueReq , "dat");
            }
#endif
        }
        m_iEgnEnqueReq[rEgnParams.mpEngineID]++;


        egnreq.m_EGNParams.mEGNConfigVec.push_back(egnconfig);
    }

    //Enque EGN Request!!
    m_QueueEGNReqList.push_back(egnreq);

#if 0
    addCommand(ECmd_EGN_ENQUE);
#else
    addCommand(cmd, (SUB_ENGINE_ID) rEgnParams.mpEngineID);
#endif
    m_pEngineDrv->enque(egnreq.m_EGNParams.mEGNConfigVec);

    FUNCTION_LOG_END;
#undef DUMP_EGNBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
EGNdeque(EGNParams<T>& rEgnParams, MINT64 i8TimeoutNs)
{
    (void) rEgnParams;
    DRV_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("EGN no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    /*LOG_DBG("EGN i8TimeoutNs(%ld)", i8TimeoutNs);*/

    mEgnDequeCond[rEgnParams.mpEngineID].wait(mDequeMtx);      //wait for egn thread

    FUNCTION_LOG_END;
    return ret;

}

#if 1
/******************************************************************************
 *

******************************************************************************/
template <class T>
MVOID
EgnStream<T>::createThread()
{
    resetDequeVariables();
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *

******************************************************************************/
template <class T>
MVOID
EgnStream<T>::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and
    MUINT32 id;
    clearCommands();
    ECmd cmd=ECmd_UNINIT;

    for(id = 0; id < eENGINE_MAX; id++)
        addCommand(cmd, (SUB_ENGINE_ID) id);
}

template <class T>
MINT32 EgnStream<T>::getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/******************************************************************************
 *

******************************************************************************/
template <class T>
MVOID
EgnStream<T>::
addCommand(ECmd const &cmd, SUB_ENGINE_ID const &eid)
{
    Mutex::Autolock autoLock(mEgnCmdLock);

    //[1] add specific command to cmd list
    mCmdList.push_back(cmd);
    mEgnList.push_back(eid);
    LOG_DBG("mCmdList, size(%d) + cmd(%d)", mCmdList.size(), cmd);
    mTotalCmdNum++;
    LOG_DBG("mTotalCmdNum_1(%d)",mTotalCmdNum);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mCmdListCond.broadcast();
}

/******************************************************************************
 *

******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::
getCommand(
    ECmd &cmd, SUB_ENGINE_ID &eid)
{
    Mutex::Autolock autoLock(mEgnCmdLock);
    //[1] check if there is command in cmd list
    LOG_DBG("TEST mTotalCmdNum(%d)",mTotalCmdNum);
    if (mTotalCmdNum <= 0)
    {   //no more cmd in cq1/cq2/cq3 dequeueCmd list, block wait for user add cmd
        LOG_DBG("no more cmd, block wait");
        mCmdListCond.wait(mEgnCmdLock);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mCmdList.size()>0)
    {
        LOG_DBG("mCmdList curSize(%d), todo(%d)", mCmdList.size(), *mCmdList.begin());
        cmd = *mCmdList.begin();
        mCmdList.erase(mCmdList.begin());
        mTotalCmdNum--;
        eid = *mEgnList.begin();
        mEgnList.erase(mEgnList.begin());

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
template <class T>
MVOID
EgnStream<T>::
clearCommands()
{
    Mutex::Autolock autoLock(mEgnCmdLock);
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
//#define EGN_INT_WAIT_TIMEOUT_MS (3000)
#define EGN_INT_WAIT_TIMEOUT_MS (10000) //Early Poting is slow.
template <class T>
MVOID*
EgnStream<T>::
onThreadLoop(
    MVOID *arg)
{
    EGNRequest<T> EGNReqVal;
    MUINT32 time1=0,time2=0,i;
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"EgnThread",0,0,0);
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
    EgnStream<T> *_this = reinterpret_cast<EgnStream<T>*>(arg);
    ECmd cmd;
    SUB_ENGINE_ID eid;
    //list<MAP_PORT_INFO>::iterator iterMapPort;
    //for(iterMapPort = mlMapPort[bufQueIdx].begin(); iterMapPort != mlMapPort[bufQueIdx].end(); iterMapPort++)

    while(_this->getCommand(cmd, eid))
    {
        LOG_DBG("cmd(%d)",cmd);
        printf("cmd(%d)",cmd);
        switch(cmd)
        {
            unsigned int engine_int;

            case ECmd_ENQUE:
                printf("EGN Get ECmd_EGN_ENQUE\n");
#ifdef DUAL_ENGINE
#else
                EGNReqVal = _this->m_QueueEGNReqList.front();
#endif
                engine_int = _this->m_pEngineDrv->queryInterrupt(EGNReqVal.m_EGNParams.mpEngineID);
                if (engine_int == 0)
                {
                    LOG_ERR("Interrupt NOT found, ERR(%d)", engine_int);
                }

                if (MTRUE == _this->m_pEngineDrv->waitFrameDone(engine_int, EGN_INT_WAIT_TIMEOUT_MS))
                {
                    //Check EGN deque
                    vector<T> EgnConfigVec;
                    _this->m_pEngineDrv->deque(EgnConfigVec);
                    typename list<EGNRequest<T>>::iterator iterEgnReq;
                    printf("_this->m_QueueEGNReqList.size():%d,!!", _this->m_QueueEGNReqList.size());
                    if (_this->m_QueueEGNReqList.size()>0)
                    {
                        //Protect the Global variable (critical section)
                        _this->getLock();
                        iterEgnReq = _this->m_QueueEGNReqList.begin();
                        EGNReqVal = (*iterEgnReq);
                        _this->m_QueueEGNReqList.pop_front();
                        _this->releaseLock();
                        //check enque and dequq size is equal or not ? the size must be equal
                        if (EgnConfigVec.size() == EGNReqVal.m_EGNParams.mEGNConfigVec.size())
                        {
                            if(EGNReqVal.m_EGNParams.mpfnCallback == NULL)
                            {
                                LOG_DBG("EGN deque no callback");
                                printf("EGN deque no callback");
                                EgnStream<T>* pEgnStream = (EgnStream<T>*)EGNReqVal.m_pEgnStream;
                                printf("wake EGN Main Thread!!");
                                pEgnStream->mEgnDequeCond[eid].broadcast(); //wake the wait thread.
                            }
                            else
                            {
                                LOG_DBG("EGN deque callback");
                                printf("EGN deque has callback");
                                time1=_this->getUs();
                                DRV_TRACE_BEGIN("egn callback");
                                EGNReqVal.m_EGNParams.mpfnCallback(EGNReqVal.m_EGNParams);
                                DRV_TRACE_END();
                                time2=_this->getUs();
                                LOG_DBG("EGN callbackT(%d us)",time2-time1);
                            }
                        }
                        else
                        {
                            LOG_ERR("EGN Error !!, Deque size(%d) is not equal to Enque size(%d)",EgnConfigVec.size(), EGNReqVal.m_EGNParams.mEGNConfigVec.size());
                        }
                    }
                    else
                    {
                        printf("error!!, receive the interrupt but the _this->m_QueueEGNReqList.size() is zero!!");
                        LOG_ERR("EGN Error !!, m_QueueEGNReqList size(%d) is zero!!", _this->m_QueueEGNReqList.size());
                    }

                }
                break;
            case ECmd_ENQUE_FEEDBACK:
                printf("ECmd_EGN_ENQUE_FEEBACK\n");
#ifdef DUAL_ENGINE
#else
                EGNReqVal = _this->m_QueueEGNReqList.front();
#endif
                engine_int = _this->m_pEngineDrv->queryInterrupt(EGNReqVal.m_EGNParams.mpEngineID);
                if (MTRUE == _this->m_pEngineDrv->waitFrameDone(engine_int, EGN_INT_WAIT_TIMEOUT_MS))
                {
                    //Check EGN deque
                    vector<T> EgnConfigVec;
                    _this->m_pEngineDrv->deque(EgnConfigVec);
                    typename list<EGNRequest<T>>::iterator iterEgnReq;
                    printf("_this->m_QueueEGNReqList.size():%d,!!", _this->m_QueueEGNReqList.size());
                    if (_this->m_QueueEGNReqList.size()>0)
                    {
                        //Protect the Global variable (critical section)
                        _this->getLock();
                        iterEgnReq = _this->m_QueueEGNReqList.begin();
                        EGNReqVal = (*iterEgnReq);
                        _this->m_QueueEGNReqList.pop_front();
                        _this->releaseLock();

                        typename vector<T>::iterator enq = EGNReqVal.m_EGNParams.mEGNConfigVec.begin();
                        typename vector<T>::iterator deq = EgnConfigVec.begin();

                        //check enque and dequq size is equal or not ? the size must be equal
                        if (EgnConfigVec.size() == EGNReqVal.m_EGNParams.mEGNConfigVec.size())
                        {
                            if(EGNReqVal.m_EGNParams.mpfnCallback == NULL)
                            {
                                LOG_DBG("EGN deque no callback");
                                EgnStream<T>* pEgnStream = (EgnStream<T>*)EGNReqVal.m_pEgnStream;
#ifdef USED
                                pEgnStream->m_Params.mEGNConfigVec.clear();
                                for(; deq != EgnConfigVec.end(); enq++, deq++)
                                {
                                    (*enq).feedback = (*deq).feedback;
                                     /*Still need to store the statistics for no-callback*/
                                    pEgnStream->m_Params.mEGNConfigVec.push_back((*enq));
                                }
#endif
                                pEgnStream->mEgnDequeCond[eid].broadcast(); //wake the wait thread.
                            }
                            else
                            {
                                LOG_DBG("EGN deque callback");

                                for(; deq != EgnConfigVec.end(); enq++, deq++)
                                {
                                    (*enq).feedback = (*deq).feedback;
                                }

                                time1=_this->getUs();
                                DRV_TRACE_BEGIN("egn callback");
                                EGNReqVal.m_EGNParams.mpfnCallback(EGNReqVal.m_EGNParams);
                                DRV_TRACE_END();
                                time2=_this->getUs();
                                LOG_DBG("EGN callbackT(%d us)",time2-time1);
                            }
                        }
                        else
                        {
                            LOG_ERR("EGN Error !!, Deque size(%d) is not equal to Enque size(%d)",EgnConfigVec.size(), EGNReqVal.m_EGNParams.mEGNConfigVec.size());
                        }
                    }
                    else
                    {
                        printf("error!!, receive the interrupt but the _this->m_QueueEGNReqList.size() is zero!!");
                        LOG_ERR("EGN Error !!, m_QueueEGNReqList size(%d) is zero!!", _this->m_QueueEGNReqList.size());
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
    ::sem_post(&(_this->mSemEgnThread));
    LOG_INF("-");
    return NULL;
}




/******************************************************************************
*
******************************************************************************/
template <class T>
MVOID
EgnStream<T>::
getLock()
{
    this->mModuleMtx.lock();
}

/******************************************************************************
*
******************************************************************************/
template <class T>
MVOID
EgnStream<T>::
releaseLock()
{
    this->mModuleMtx.unlock();
}



/******************************************************************************
 *

******************************************************************************/
template <class T>
MBOOL
EgnStream<T>::resetDequeVariables()
{
    //clear and free mCmdList
    list<ECmd>().swap(mCmdList);
    mTotalCmdNum=0;
    return true;
}

#endif
#ifdef FACTORY
template IEgnStream<OCCConfig>* NSCam::NSIoPipe::NSEgn::getEgnStreamImp(char const* szCallerName);
template IEgnStream<OWMFEConfig>* NSCam::NSIoPipe::NSEgn::getEgnStreamImp(char const* szCallerName);
#else
template IEgnStream<OCCConfig>* IEgnStream<OCCConfig>::createInstance(char const* szCallerName);
template IEgnStream<OWMFEConfig>* IEgnStream<OWMFEConfig>::createInstance(char const* szCallerName);
#endif
