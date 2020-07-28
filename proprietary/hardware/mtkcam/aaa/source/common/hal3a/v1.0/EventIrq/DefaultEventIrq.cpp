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

/**
* @file DefaultEventIrq.cpp
* @brief Declarations of Fake IRQ Event waiting interface
*/
#define LOG_TAG "EventIRQ3A"

#include "DefaultEventIrq.h"


namespace NS3Av3
{
using namespace android;

DefaultEventIrq::
DefaultEventIrq()
    : m_i4User(0)
    , m_i4EventCnt(-1)
    , m_i4TimeStampMark(0)
    , m_i4TimeStampEvent(0)
    , m_Lock()
    , m_EventMtx()
    , m_fgThreadOn(MFALSE)
{
    CAM_LOGD("[%s]", __FUNCTION__);
}

DefaultEventIrq*
DefaultEventIrq::
createInstance(const IEventIrq::ConfigParam& rCfg, const char* strUser)
{
    DefaultEventIrq* pHwEventIrq = NULL;
    pHwEventIrq = new DefaultEventIrq;

    if (pHwEventIrq)
    {
        pHwEventIrq->init(rCfg, strUser);
        return pHwEventIrq;
    }
    return pHwEventIrq;
}

MVOID
DefaultEventIrq::
destroyInstance(const char* strUser)
{
    uninit(strUser);
}

MVOID
DefaultEventIrq::
init(const IEventIrq::ConfigParam& rCfg, const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {
        CAM_LOGD_IF(1, "[%s] m_i4User(%d), m_strName(%s), strUser(%s)", __FUNCTION__, m_i4User, m_strName.c_str(), strUser);
        m_i4User ++;
        return;
    }

    // init
    m_rCfgParam = rCfg;
    m_strName = strUser;
    m_i4EventCnt = 0;
    m_i4TimeStampMark = 0;
    m_i4TimeStampEvent = 0;
    m_fgThreadOn = MTRUE;
    // create thread
    ::sem_init(&m_SemEvent, 0, 0);
    ::pthread_create(&m_Thread, NULL, threadLoop, this);

    m_i4User ++;

    CAM_LOGD("[%s] cfg(%d, %d)", __FUNCTION__, m_rCfgParam.i4SensorDev, m_rCfgParam.eEventType);
}

MVOID
DefaultEventIrq::
uninit(const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User <= 0)
    {
        return;
    }

    m_i4User --;

    if (m_i4User == 0)
    {
        // uninit
        CAM_LOGD("[%s] m_strName(%s), strUser(%s)", __FUNCTION__, m_strName.c_str(), strUser);
        m_fgThreadOn = MFALSE;
        // destroy thread
        ::pthread_join(m_Thread, NULL);
        ::sem_destroy(&m_SemEvent);
    }
    else
    {
        // do nothing
    }
}

MINT32
DefaultEventIrq::
mark()
{
    Mutex::Autolock lock(m_EventMtx);

    m_i4TimeStampMark = getUsTime();
    m_i4TimeStampEvent = m_i4TimeStampMark;
    m_i4EventCnt = 0;
    CAM_LOGD("[%s] mark(%d)", __FUNCTION__, m_i4TimeStampMark);
    return 0;
}

MINT32
DefaultEventIrq::
query(IEventIrq::Duration& rDuration)
{
    Mutex::Autolock lock(m_EventMtx);

    MINT32 cur, dif0, dif1;
    cur = getUsTime();
    CAM_LOGD("[%s] cur(%d)", __FUNCTION__, cur);
    dif0 = cur - m_i4TimeStampMark;
    dif1 = cur - m_i4TimeStampEvent;
    rDuration.i4Duration0 = dif0;
    rDuration.i4Duration1 = dif1;
    return m_i4EventCnt;
}

MINT32
DefaultEventIrq::
wait(IEventIrq::Duration& rDuration)
{
    Mutex::Autolock lock(m_EventMtx);
    MINT64 cur, dif0, dif1;
    if (m_i4EventCnt <= 0)
    {
        CAM_LOGD("1st time wait");
        return -1;
    }
    MINT32 i4EventCnt = m_i4EventCnt;
    cur = getUsTime();
    CAM_LOGD("[%s] cur0(%ld)", __FUNCTION__, (long)cur);
    dif0 = cur - m_i4TimeStampMark;
    m_EventCond.wait(m_EventMtx);
    cur = getUsTime();
    CAM_LOGD("[%s] cur1(%ld)", __FUNCTION__, (long)cur);
    dif1 = cur - m_i4TimeStampEvent;
    rDuration.i4Duration0 = dif0;
    rDuration.i4Duration1 = dif1;
    return i4EventCnt;
}

MINT32
DefaultEventIrq::
setTgInfo(MINT32 /*i4TgInfo*/)
{
    return 0;
}

MVOID*
DefaultEventIrq::
threadLoop(MVOID* arg)
{
    DefaultEventIrq* _this = reinterpret_cast<DefaultEventIrq*>(arg);

    while (_this->m_fgThreadOn)
    {
        ::usleep(33333);

        Mutex::Autolock lock(_this->m_EventMtx);
        _this->m_i4TimeStampEvent = _this->getUsTime();
        CAM_LOGD("[%s] event(%d)", __FUNCTION__, _this->m_i4TimeStampEvent);
        if (_this->m_i4EventCnt >= 0)
        {
            _this->m_i4EventCnt++;
            _this->m_EventCond.broadcast();
            //if (_this->m_i4EventCnt == 1)
            //    ::sem_post(&_this->m_SemEvent);
        }
    }
    return NULL;
}

#if 0
IEventIrq*
IEventIrq::
createInstance(const ConfigParam& rCfg, const char* strUser)
{
    return DefaultEventIrq::createInstance(rCfg, strUser);
}
#endif
};

