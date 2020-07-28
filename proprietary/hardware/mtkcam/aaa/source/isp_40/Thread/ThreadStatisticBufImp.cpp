/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#define LOG_TAG "ThreadStatistic"

#include <mtkcam/def/PriorityDefs.h>
#include <cutils/properties.h>
#include <utils/threads.h>
#include <sys/prctl.h>
#include <IThread.h>
#include <pthread.h>
#include <semaphore.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Log.h>
#include <debug/DebugUtil.h>

namespace NS3Av3
{
using namespace android;
using namespace NSCam::Utils;

class ThreadStatisticBufImp : public ThreadStatisticBuf
{
public:
    virtual MBOOL               destroyInstance();
    virtual MBOOL               postCmd(void* pArg);
    virtual MBOOL               waitFinished();
    virtual MVOID                pause();
    virtual MVOID                resume();

                                ThreadStatisticBufImp(MINT32 i4SensorDev, char* pName, std::vector<IBufMgr*> rBufMgrList);
    virtual                     ~ThreadStatisticBufImp();
private:
    static MVOID*               onThreadLoop(MVOID*);
    MVOID                       changeThreadSetting();

    MINT32                      m_i4SensorDev;
    MBOOL                       m_fgLogEn;
    MBOOL                       m_fgTerminate;
    MBOOL                       m_fgPause;
    pthread_t                   m_rThread;
    sem_t                       m_SemFinish;
    sem_t                       m_SemPause;
    char                        m_pName[256];
    Mutex                       m_ModuleMtx;
    std::vector<IBufMgr*>             m_pBufMgrList;
};

ThreadStatisticBuf*
ThreadStatisticBuf::
createInstance(MINT32 i4SensorDev, char* pName, std::vector<IBufMgr*> rBufMgrList)
{
    ThreadStatisticBufImp* pObj = new ThreadStatisticBufImp(i4SensorDev, pName,rBufMgrList);
    return pObj;
}

MBOOL
ThreadStatisticBufImp::
destroyInstance()
{
    waitFinished();
    Mutex::Autolock autoLock(m_ModuleMtx);
    CAM_LOGD_IF(m_fgLogEn,"[%s] +\n", __FUNCTION__);

    CAM_LOGD_IF(m_fgLogEn,"[%s]sem_wait : m_SemFinish \n", __FUNCTION__);
    ::sem_wait(&m_SemFinish);
    CAM_LOGD_IF(m_fgLogEn,"[%s]pthread_join : m_rThread \n", __FUNCTION__);
    ::pthread_join(m_rThread, NULL);
    m_pBufMgrList.clear();

    CAM_LOGD_IF(m_fgLogEn,"[%s] -\n", __FUNCTION__);
    delete this;
    return MTRUE;
}

MBOOL
ThreadStatisticBufImp::
postCmd(void* /*pArg*/)
{
    return MTRUE;
}

MVOID
ThreadStatisticBufImp::
pause()
{
    if(!m_fgPause)
        m_fgPause = MTRUE;
}

MVOID
ThreadStatisticBufImp::
resume()
{
    if(m_fgPause)
    {
        MINT32 i4SemValue = 0;
        ::sem_getvalue(&m_SemPause, &i4SemValue);
        CAM_LOGD("[%s] m_SemPause(%d)", __FUNCTION__, i4SemValue);
        m_fgPause = MFALSE;
        ::sem_post(&m_SemPause);
    }
}


MBOOL
ThreadStatisticBufImp::
waitFinished()
{
    Mutex::Autolock autoLock(m_ModuleMtx);
    CAM_LOGD_IF(m_fgLogEn,"[%s] +\n", __FUNCTION__);
    if(!m_fgTerminate){
        m_fgTerminate = 1;
    }
    resume();
    CAM_LOGD_IF(m_fgLogEn,"[%s] -\n", __FUNCTION__);
    return MTRUE;
}

ThreadStatisticBufImp::
ThreadStatisticBufImp(MINT32 i4SensorDev, char* pName, std::vector<IBufMgr*> rBufMgrList)
    : m_i4SensorDev(i4SensorDev)
    , m_fgLogEn(0)
    , m_fgTerminate(0)
    , m_fgPause(0)
    , m_rThread()
    , m_SemFinish()
    , m_SemPause()
    , m_ModuleMtx()
{
    strncpy(m_pName, pName, 255);
    m_pName[255] = '\0';
    m_pBufMgrList = rBufMgrList;
    // init something
    ::sem_init(&m_SemFinish, 0, 0);
    ::sem_init(&m_SemPause, 0, 0);
    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.thread_stt.enable", value, "0");
    m_fgLogEn = atoi(value);
    CAM_LOGD_IF(m_fgLogEn,"[%s] m_pName: %s\n", __FUNCTION__,m_pName);
}

ThreadStatisticBufImp::
~ThreadStatisticBufImp()
{
    CAM_LOGD_IF(m_fgLogEn,"[%s]\n", __FUNCTION__);
}

MVOID
ThreadStatisticBufImp::
changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, m_pName, 0, 0, 0);

    // (2) set policy/priority
    int const expect_policy    = SCHED_OTHER;
    int const expect_priority  = NICE_CAMERA_STT;

    int policy = 0, priority = 0;
    setThreadPriority(expect_policy, expect_priority);
    getThreadPriority(policy, priority);

    CAM_LOGD(
        "[%s] name(%s), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)", __FUNCTION__
        , m_pName, ::gettid()
        , expect_policy, policy
        , expect_priority, priority
    );
}

MVOID*
ThreadStatisticBufImp::
onThreadLoop(MVOID* pArg)
{
    ThreadStatisticBufImp* _this = reinterpret_cast<ThreadStatisticBufImp*>(pArg);
    _this->changeThreadSetting();

    MBOOL bDequeFail = MFALSE;
    MINT32 result = MTRUE;
    while (!_this->m_fgTerminate)
    {
        // sleep 5ms if deque fail, than deque again.
        if(bDequeFail)
            usleep(5000);
        CAM_LOGD_IF(_this->m_fgLogEn, "[%s] %s : dequeueHwBuf + \n", __FUNCTION__, _this->m_pName);
        for (MINT32 i = 0; i < (MINT32)_this->m_pBufMgrList.size(); i++)
        {
            CAM_LOGD_IF(_this->m_fgLogEn, "[%s] %s : i(%d) result(%d) + \n", __FUNCTION__, _this->m_pName, i, result);
            if(_this->m_pBufMgrList[i] != NULL)
            {
                result = _this->m_pBufMgrList[i]->dequeueHwBuf();
                CAM_LOGD_IF(_this->m_fgLogEn, "[%s] %s : i(%d) result(%d) - \n", __FUNCTION__, _this->m_pName, i, result);
            }
            else
                CAM_LOGD_IF(_this->m_fgLogEn, " _this->m_pBufMgrList[i] is NULL - \n", __FUNCTION__);
            if(result == -1)
                _this->m_fgTerminate = MTRUE;
            else
                bDequeFail = (result == 0) ? MTRUE : MFALSE;

            if(_this->m_fgTerminate)
                break;
        }
        CAM_LOGD_IF(_this->m_fgLogEn, "[%s] %s : dequeueHwBuf - \n", __FUNCTION__, _this->m_pName);

        if(_this->m_fgPause)
        {
            CAM_LOGD("Pause +");
            ::sem_wait(&_this->m_SemPause);
            CAM_LOGD("Pause -");
        }
    }
    CAM_LOGD("[%s]sem_post : m_SemFinish \n", __FUNCTION__);
    ::sem_post(&_this->m_SemFinish);
    return NULL;
}
};
