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
#define LOG_TAG "aaa_hal_if"

#include <cutils/properties.h>
#include <mtkcam/featureio/aaa_hal_if.h>

//#include <mtkcam/core/featureio/pipe/aaa/ResultBufMgr/ResultBufMgr.h>
//#include "aaa_hal_template.h"

using namespace NS3A;

#define GET_PROP(prop, dft, val)\
{\
   char value[PROPERTY_VALUE_MAX] = {'\0'};\
   property_get(prop, value, (dft));\
   (val) = atoi(value);\
}

#if 0
/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const u4SensorOpenIndex)
{
    //static Hal3AIf singleton;
    //return &singleton;
    return Hal3ATemplate::createInstance(i4SensorDevId);
}

#else

#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <state_mgr/aaa_state.h>
#include <state_mgr/aaa_state_mgr.h>
#include "aaa_hal_yuv.h"
//
/*NeedUpdate*///#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>

using namespace NS3A;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const i4SensorOpenIndex)
{
#ifdef USING_MTK_LDVT
    static Hal3AIf singleton;
    return &singleton;
#endif


    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList) return NULL;

    MINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);
    MINT32 eSensorType = pHalSensorList->queryType(i4SensorOpenIndex); // sensor hal defined

    if (eSensorType == NSSensorType::eRAW)
        return Hal3A::createInstance(i4SensorDevId, i4SensorOpenIndex);
    else if (eSensorType == NSSensorType::eYUV)
        return Hal3AYuv::createInstance(i4SensorDevId, i4SensorOpenIndex);
    else
    {
        static Hal3AIf singleton;
        return &singleton;
    }

#if 0
    SensorHal* pSensorHal = SensorHal::createInstance();
    if ( !pSensorHal )
    {
        return NULL;
    }
    //
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(i4SensorDevId),
                            SENSOR_CMD_SET_SENSOR_DEV,
                            0,
                            0,
                            0
                           );
    //
    pSensorHal->init();
    //
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(i4SensorDevId),
                             SENSOR_CMD_GET_SENSOR_TYPE,
                             reinterpret_cast<int>(&eSensorType),
                             0,
                             0
                            );
    //
    if  ( pSensorHal )
    {
        pSensorHal->uninit();
        pSensorHal->destroyInstance();
        pSensorHal = NULL;
    }

    if (eSensorType == SENSOR_TYPE_RAW) {
#ifdef USING_MTK_LDVT
    static Hal3AIf singleton;
    return &singleton;
#else
        return Hal3A::createInstance(i4SensorDevId);
#endif
    }
    else if (eSensorType == SENSOR_TYPE_YUV)
    {
#ifdef USING_MTK_LDVT
         static Hal3AIf singleton;
         return &singleton;
#else
        return Hal3AYuv::createInstance(i4SensorDevId);
#endif
    }
    else {
#if 1
    static Hal3AIf singleton;
    return &singleton;
#else
    return NULL;//Hal3AYuv::getInstance();
#endif
    }
#endif
}

/*******************************************************************************
*
********************************************************************************/

CallBackSet::
CallBackSet()
    : m_Mutex()
{
    GET_PROP("vendor.debug.aaa_cb.log", "0", m_i4LogEn);
    Mutex::Autolock autoLock(m_Mutex);
    m_CallBacks.clear();
}

CallBackSet::
~CallBackSet()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

void
CallBackSet::
doNotifyCb (
   int32_t _msgType,
   int32_t _ext1,
   int32_t _ext2,
   int32_t _ext3,
   MINTPTR _ext4
)
{
    Mutex::Autolock autoLock(m_Mutex);

    if (m_i4LogEn) MY_LOG("[%s] _msgType(%d) _ext1(0x%08x) _ext2(0x%08x) _ext3(0x%08x) _ext4(0x%08x)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3, _ext3);
    List<wp<I3ACallBack>>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        sp<I3ACallBack> pCb = (*it).promote();
        if (pCb != NULL)
        {
            if (m_i4LogEn) MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, pCb.get());
            pCb->doNotifyCb(_msgType, _ext1, _ext2, _ext3, _ext4);
        }
    }
}

void
CallBackSet::
doDataCb (
   int32_t  _msgType,
   void*    _data,
   uint32_t _size
)
{
    Mutex::Autolock autoLock(m_Mutex);

    if (m_i4LogEn) MY_LOG("[%s] _msgType(%d) _data(0x%08x) _size(0x%08x)", __FUNCTION__, _msgType, _data, _size);

    List<wp<I3ACallBack>>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        sp<I3ACallBack> pCb = (*it).promote();
        if (pCb != NULL)
        {
            if (m_i4LogEn) MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, pCb.get());
            pCb->doDataCb(_msgType, _data, _size);
        }
    }
}

MINT32
CallBackSet::
addCallback(I3ACallBack* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;

    if (cb == NULL)
    {
        MY_ERR("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    List<wp<I3ACallBack>>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == it->unsafe_get())
            break;
    }

    if (i4Cnt == (MINT32) m_CallBacks.size())
    {
        // not exist, add
        m_CallBacks.push_back(cb);
        return m_CallBacks.size();
    }
    else
    {
        // already exists
        return -m_CallBacks.size();
    }
}

MINT32
CallBackSet::
removeCallback(I3ACallBack* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;
    MINT32 i4Size = m_CallBacks.size();

    List<wp<I3ACallBack>>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == it->unsafe_get())
        {
            m_CallBacks.erase(it);
            // successfully removed
            return m_CallBacks.size();
        }
    }

    // cannot be found
    return -i4Size;
}


///----------------------------------------------------------------------

//                                                         AF   CALLBACK

///----------------------------------------------------------------------


AFCallBackSet::
AFCallBackSet()
    : m_Mutex()
    , m_MutexNotify(NULL)
    , m_MutexData(NULL)
    , m_NotifySem()
    , m_DataSem()
    , threadNotifyCb()
    , threadDataCb()
    , NotifyStatus(NULL)
    , DataStatus(NULL)
{
    MY_LOG("[%s]+ Constructor =====", __FUNCTION__);
    GET_PROP("vendor.debug.aaa_cb_af.log", "0", m_i4LogEn);
    m_CallBacks.clear();
    //sem_init(&m_NotifySem, 0, 0);
    //sem_init(&m_DataSem, 0, 0);
    pthread_create(&threadNotifyCb, NULL, AFCallBackSet::NotifyCbloop, this);
    pthread_create(&threadDataCb, NULL, AFCallBackSet::DataCbloop, this);
    MY_LOG("[%s]- Constructor =====", __FUNCTION__);
}

AFCallBackSet::
~AFCallBackSet()
{
    MY_LOG("[%s]+ destroy ======", __FUNCTION__);
    m_CallBacks.clear();
    MY_LOG("[%s]- destroy ======", __FUNCTION__);
}

void
AFCallBackSet::
doNotifyCb (
   int32_t _msgType,
   int32_t _ext1,
   int32_t _ext2,
   int32_t _ext3,
   MINTPTR _ext4
)
{
    Mutex::Autolock autoLock( (*m_MutexNotify) );
    if( NotifyStatus && (*NotifyStatus) == ALIVE)
    {
        NotifyQ.push(Notify_Param(_msgType, _ext1, _ext2, _ext3, _ext4));

        MY_LOG("[%s] push done _msgType(%d) _ext1(%d) _ext2(%d) _ext3(%d) _ext4(%d)  NotifyStatus(%d) ======", __FUNCTION__, _msgType, _ext1, _ext2, _ext3, _ext3, *NotifyStatus);
    }

    MINT32 LocalValue = 0;
    sem_getvalue(&(*m_NotifySem), &LocalValue);
    MY_LOG("[%s] sem_post LocalValue(%d) ======", __FUNCTION__, LocalValue);
    sem_post( &(*m_NotifySem) );
    sem_getvalue(&(*m_NotifySem), &LocalValue);
    MY_LOG("[%s] sem_post LocalValue(%d) done======", __FUNCTION__, LocalValue);
}

void
AFCallBackSet::
doDataCb (
   int32_t  _msgType,
   void*    _data,
   uint32_t _size
)
{
    Mutex::Autolock autoLock( (*m_MutexData) );
    if( DataStatus && (*DataStatus) == ALIVE && _data != NULL && _size != 0)
    {
        DataQ.push(Data_Param(_msgType, _data, _size));

        MY_LOG("[%s] push done _msgType(%d) _data(%p) _size(%d)  DataStatus(%d) ======", __FUNCTION__, _msgType, (void*)_data, _size, *DataStatus);
    }

    MINT32 LocalValue = 0;
    sem_getvalue(&(*m_DataSem), &LocalValue);
    MY_LOG("[%s] sem_post LocalValue(%d) ======", __FUNCTION__, LocalValue);
    sem_post( &(*m_DataSem) );
    sem_getvalue(&(*m_DataSem), &LocalValue);
    MY_LOG("[%s] sem_post LocalValue(%d) done======", __FUNCTION__, LocalValue);
}

MINT32
AFCallBackSet::
addCallback(I3ACallBack* cb)
{
    if (m_i4LogEn) MY_LOG("addCallback being called =====");
    Mutex::Autolock autoLock(m_Mutex);

    if (cb == NULL)
    {
        MY_ERR("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    for (List<wp<I3ACallBack>>::iterator it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        if (cb == it->unsafe_get())  return -m_CallBacks.size();
    }

    m_CallBacks.push_back(cb);
    if (m_i4LogEn) MY_LOG("[%s] PushBack CallBack %p  ======", __FUNCTION__, (void*)cb);

    return m_CallBacks.size();
}

MINT32
AFCallBackSet::
removeCallback(I3ACallBack* cb)
{
    if (m_i4LogEn) MY_LOG("removeCallback being called =====");
    Mutex::Autolock autoLock(m_Mutex);

    List<wp<I3ACallBack>>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        if (cb == it->unsafe_get())
        {
            if (m_i4LogEn) MY_LOG("[%s] Remove CallBack %p  ======", __FUNCTION__, (void*)cb);
            m_CallBacks.erase(it);
            // successfully removed
            return m_CallBacks.size();
        }
    }

    // cannot be found
    return m_CallBacks.size();
}

List<wp<I3ACallBack>>
AFCallBackSet::
getCallback()
{
    Mutex::Autolock autoLock(m_Mutex);
    return m_CallBacks;
}

void
AFCallBackSet::
uninit()
{
    if(m_MutexNotify == NULL && m_MutexData == NULL)
    {
        queue<Notify_Param> NotifyQEmpty;
        queue<Data_Param> DataQEmpty;
        ::swap( NotifyQ, NotifyQEmpty );
        ::swap( DataQ, DataQEmpty );
        MY_LOG("[%s] Object is NULL. m_MutexNotify(%p) , m_MutexData(%p)", __FUNCTION__, m_MutexNotify, m_MutexData);
        return;
    }
    MY_LOG("[%s]+ lock start ======", __FUNCTION__);
    (*m_MutexNotify).lock();
    (*m_MutexData).lock();

    MY_LOG("[%s]++ Clear queue and update statue ======", __FUNCTION__);

    queue<Notify_Param> NotifyQEmpty;
    queue<Data_Param> DataQEmpty;
    ::swap( NotifyQ, NotifyQEmpty );
    ::swap( DataQ, DataQEmpty );

    if(NotifyStatus)
        *NotifyStatus = END;
    if(DataStatus)
        *DataStatus = END;

    MINT32 LocalValue = 0;
    sem_getvalue(&(*m_NotifySem), &LocalValue);
    MY_LOG("[%s] sem_post m_NotifySem LocalValue(%d) ======", __FUNCTION__, LocalValue);
    sem_post(&(*m_NotifySem));
    sem_getvalue(&(*m_NotifySem), &LocalValue);
    MY_LOG("[%s] sem_post m_NotifySem LocalValue(%d) done======", __FUNCTION__, LocalValue);

    sem_getvalue(&(*m_DataSem), &LocalValue);
    MY_LOG("[%s] sem_post m_DataSem LocalValue(%d) ======", __FUNCTION__, LocalValue);
    sem_post(&(*m_DataSem));
    sem_getvalue(&(*m_DataSem), &LocalValue);
    MY_LOG("[%s] sem_post m_DataSem LocalValue(%d) done======", __FUNCTION__, LocalValue);


    //pthread_join(threadNotifyCb, NULL);
    //pthread_join(threadDataCb, NULL);
    MY_LOG("[%s]--  (NotifyQSize DataQSize)(%d %d), (NotifyQEmpty DataQEmpty)(%d %d)======", __FUNCTION__, NotifyQ.size(), DataQ.size(), NotifyQ.empty(), DataQ.empty());

    (*m_MutexData).unlock();
    (*m_MutexNotify).unlock();
    MY_LOG("[%s]- unlock end ======", __FUNCTION__);
    return ;
}

void*
AFCallBackSet::
NotifyCbloop(void* arg)
{
    MY_LOG("[%s]+  ======", __FUNCTION__);
    pthread_detach(pthread_self());
    AFCallBackSet *_this = static_cast<AFCallBackSet*>(arg);

    MINT32 LocalValue = 0;

    MINT32 LocalNotifyStatus = ALIVE;
    Mutex LocalMutexNotify;
    sem_t LocalNotifySem;

    sem_init(&LocalNotifySem, 0, 0);

    _this->NotifyStatus = &LocalNotifyStatus;
    _this->m_NotifySem = &LocalNotifySem;
    _this->m_MutexNotify = &LocalMutexNotify;

    while(LocalNotifyStatus != END){

        sem_getvalue(&LocalNotifySem, &LocalValue);
        MY_LOG("[%s] sem_wait LocalNotifySem before LocalValue(%d) NotifyStatus(%d) NotifyQEmpty(%d) NotifyQSize(%d)======", __FUNCTION__, LocalValue, LocalNotifyStatus, _this->NotifyQ.empty(), _this->NotifyQ.size());
        sem_wait(&(LocalNotifySem));
        sem_getvalue(&LocalNotifySem, &LocalValue);
        MY_LOG("[%s] sem_wait LocalNotifySem after LocalValue(%d) NotifyStatus(%d) NotifyQEmpty(%d) NotifyQSize(%d)======", __FUNCTION__, LocalValue, LocalNotifyStatus, _this->NotifyQ.empty(), _this->NotifyQ.size());

        {
            MY_LOG("[%s] LocalMutexNotify1 before ======", __FUNCTION__);
            Mutex::Autolock autoLock(LocalMutexNotify);
            if (1) MY_LOG("[%s] First NotifyStatus(%d) NotifyQEmpty(%d) NotifyQSize(%d) LocalMutexNotify1 after ======", __FUNCTION__, LocalNotifyStatus, _this->NotifyQ.empty(), _this->NotifyQ.size());
            if (LocalNotifyStatus == END)
            {
                MY_LOG("[%s] First Break NotifyStatus(%d) ======", __FUNCTION__, LocalNotifyStatus);
                break;
            }
        }

        while(!(_this->NotifyQ.empty())){
            Notify_Param cur;

            {
                MY_LOG("[%s] LocalMutexNotify2 before ======", __FUNCTION__);
                Mutex::Autolock autoLock(LocalMutexNotify);
                if (1) MY_LOG("[%s] Second NotifyStatus(%d) NotifyQEmpty(%d) NotifyQSize(%d) LocalMutexNotify2 after======", __FUNCTION__, LocalNotifyStatus, _this->NotifyQ.empty(), _this->NotifyQ.size());
                if (LocalNotifyStatus == END)
                {
                    MY_LOG("[%s] Second Break NotifyStatus(%d) ======", __FUNCTION__, LocalNotifyStatus);
                    break;
                }
                cur = _this->NotifyQ.front();
                _this->NotifyQ.pop();
            }

            List<wp<I3ACallBack>> cbList = _this->getCallback();

           for (List<wp<I3ACallBack>>::iterator it = cbList.begin(); it != cbList.end(); it++)
           {
               sp<I3ACallBack> pCb = (*it).promote();
               if (pCb != NULL)
               {
                   MY_LOG("[%s] doNotifyCb before cb(%p)  notifymsg %d, ext1 %d, ext2 %d, ext3 %d, ext4 %d  ======", __FUNCTION__, pCb.get(), cur.notifymsgType, cur.ext1, cur.ext2, cur.ext3, cur.ext4);
                   pCb->doNotifyCb(cur.notifymsgType, cur.ext1, cur.ext2, cur.ext3, cur.ext4);
                   MY_LOG("[%s] doNotifyCb done cb(%p)  notifymsg %d, ext1 %d, ext2 %d, ext3 %d, ext4 %d  ======", __FUNCTION__, pCb.get(), cur.notifymsgType, cur.ext1, cur.ext2, cur.ext3, cur.ext4);
               }
           }
        }
        if (LocalNotifyStatus == END)
        {
            MY_LOG("[%s] Break NotifyStatus(%d) final ======", __FUNCTION__, LocalNotifyStatus);
            break;
        }
    }

    MY_LOG("[%s]-  ======", __FUNCTION__);
    ::pthread_exit((void*)0);
}

void*
AFCallBackSet::
DataCbloop(void* arg)
{
    MY_LOG("[%s]+  ======", __FUNCTION__);
    pthread_detach(pthread_self());
    AFCallBackSet *_this = static_cast<AFCallBackSet*>(arg);

    MINT32 LocalValue = 0;

    MINT32 LocalDataStatus = ALIVE;
    Mutex LocalMutexData;
    sem_t LocalDataSem;

    sem_init(&LocalDataSem, 0, 0);

    _this->DataStatus = &LocalDataStatus;
    _this->m_DataSem = &LocalDataSem;
    _this->m_MutexData = &LocalMutexData;

    while(LocalDataStatus != END){

        sem_getvalue(&LocalDataSem, &LocalValue);
        MY_LOG("[%s] LocalataSem before LocalValue(%d) DataStatus(%d) DataQEmpty(%d) DataQSize(%d) ======", __FUNCTION__, LocalValue, LocalDataStatus, _this->DataQ.empty(), _this->DataQ.size());
        sem_wait(&(LocalDataSem));
        sem_getvalue(&LocalDataSem, &LocalValue);
        MY_LOG("[%s] LocalataSem after LocalValue(%d) DataStatus(%d) DataQEmpty(%d) DataQSize(%d) ======", __FUNCTION__, LocalValue, LocalDataStatus, _this->DataQ.empty(), _this->DataQ.size());

        {
            MY_LOG("[%s] LocalMutexData1 before ======", __FUNCTION__);
            Mutex::Autolock autoLock(LocalMutexData);
            if (1) MY_LOG("[%s] First DataStatus(%d) DataQEmpty(%d) DataQSize(%d) LocalMutexData1 after ======", __FUNCTION__, LocalDataStatus, _this->DataQ.empty(), _this->DataQ.size());
            if (LocalDataStatus == END)
            {
                MY_LOG("[%s] First Break DataStatus(%d) ======", __FUNCTION__, LocalDataStatus);
                break;
            }
        }

        while(!(_this->DataQ.empty())){
            Data_Param cur;

            {
                MY_LOG("[%s] LocalMutexData2 before ======", __FUNCTION__);
                Mutex::Autolock autoLock(LocalMutexData);
                if (1) MY_LOG("[%s] Second DataStatus(%d) DataQEmpty(%d) DataQSize(%d) LocalMutexData2 after ======", __FUNCTION__, LocalDataStatus, _this->DataQ.empty(), _this->DataQ.size());
                if (LocalDataStatus == END)
                {
                    MY_LOG("[%s] Second Break DataStatus(%d) ======", __FUNCTION__, LocalDataStatus);
                    break;
                }
                cur = _this->DataQ.front();
                _this->DataQ.pop();
            }

            List<wp<I3ACallBack>> cbList = _this->getCallback();

            for (List<wp<I3ACallBack>>::iterator it = cbList.begin(); it != cbList.end(); it++)
            {
                sp<I3ACallBack> pCb = (*it).promote();
                if (pCb != NULL)
                {
                    MY_LOG("[%s] doDataCb before cb(%p)  datamsg %d, data %p, size %d  ======", __FUNCTION__, pCb.get(), cur.datamsgType, cur.datadata, cur.datasize);
                    pCb->doDataCb(cur.datamsgType, cur.datadata, cur.datasize);
                    MY_LOG("[%s] doDataCb done cb(%p)  datamsg %d, data %p, size %d  ======", __FUNCTION__, pCb.get(), cur.datamsgType, cur.datadata, cur.datasize);
                }
            }
        }
        if (LocalDataStatus == END)
        {
            MY_LOG("[%s] Break DataStatus(%d) final ======", __FUNCTION__, LocalDataStatus);
            break;
        }
    }

    MY_LOG("[%s]-  ======", __FUNCTION__);
    ::pthread_exit((void*)0);
}



#endif
