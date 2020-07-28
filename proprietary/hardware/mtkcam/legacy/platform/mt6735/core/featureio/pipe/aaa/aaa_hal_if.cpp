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
    , m_MutexNotify()
    , m_MutexData()
    , m_NotifySem()
    , m_DataSem()
    , threadNotifyCb()
    , threadDataCb()
{
    MY_LOG("[%s]+ Constructor =====", __FUNCTION__);
    GET_PROP("vendor.debug.aaa_cb_af.log", "0", m_i4LogEn);
    m_CallBacks.clear();
    sem_init(&m_NotifySem, 0, 0);
    sem_init(&m_DataSem, 0, 0);
    MY_LOG("[%s]- Constructor =====", __FUNCTION__);
}

AFCallBackSet::
~AFCallBackSet()
{}

void
AFCallBackSet::
init()
{
    MY_LOG("[%s]+  ======", __FUNCTION__);

    MINT32 handle;
    handle = pthread_create(&threadNotifyCb, NULL, AFCallBackSet::NotifyCbloop, this);
    if (handle) CAM_LOGD("[%s] NotifyCb thread create error = %d", __FUNCTION__, handle);

    handle = pthread_create(&threadDataCb, NULL, AFCallBackSet::DataCbloop, this);
    if (handle) CAM_LOGD("[%s] DataCb thread create error = %d", __FUNCTION__, handle);

    NotifyStatus = ALIVE;
    DataStatus = ALIVE;

    MY_LOG("[%s]-  ======", __FUNCTION__);

    return ;
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
    Mutex::Autolock autoLock(m_MutexNotify);

    if (m_i4LogEn) MY_LOG("[%s] _msgType(%d) _ext1(%d) _ext2(%d) _ext3(%d) _ext4(%d)  ======", __FUNCTION__, _msgType, _ext1, _ext2, _ext3, _ext3);

    NotifyQ.push_back(Notify_Param(_msgType, _ext1, _ext2, _ext3, _ext4));
    sem_post(&m_NotifySem);
}

void
AFCallBackSet::
doDataCb (
   int32_t  _msgType,
   void*    _data,
   uint32_t _size
)
{
    Mutex::Autolock autoLock(m_MutexData);
    if (m_i4LogEn) MY_LOG("[%s] _msgType(%d) _data(%p) _size(%d)  ======", __FUNCTION__, _msgType, (void*)_data, _size);

    DataQ.push_back(Data_Param(_msgType, _data, _size));

    sem_post(&m_DataSem);
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
    MY_LOG("[%s]+  ======", __FUNCTION__);
    NotifyStatus = END;
    DataStatus = END;
    sem_post(&m_NotifySem);
    sem_post(&m_DataSem);

    pthread_join(threadNotifyCb, NULL);
    pthread_join(threadDataCb, NULL);
    m_CallBacks.clear();

    MY_LOG("[%s]-  ======", __FUNCTION__);
    return ;
}

void*
AFCallBackSet::
NotifyCbloop(void* arg)
{
    MY_LOG("[%s]+  ======", __FUNCTION__);
    AFCallBackSet *_this = static_cast<AFCallBackSet*>(arg);

    while(1){

        sem_wait(&(_this->m_NotifySem));

        if (_this->NotifyQ.empty() && _this->NotifyStatus == END)  break;

        while(!(_this->NotifyQ.empty())){
            Notify_Param cur;

            {
                Mutex::Autolock autoLock(_this->m_MutexNotify);
                cur = *(_this->NotifyQ.begin());
                _this->NotifyQ.erase(_this->NotifyQ.begin());
            }

            List<wp<I3ACallBack>> cbList = _this->getCallback();

           for (List<wp<I3ACallBack>>::iterator it = cbList.begin(); it != cbList.end(); it++)
            {
                sp<I3ACallBack> pCb = (*it).promote();
                if (pCb != NULL)
                {
                    if (_this->m_i4LogEn) MY_LOG("[%s] cb(%p)  notifymsg %d, ext1 %d, ext2 %d, ext3 %d, ext4 %d  ======", __FUNCTION__, pCb.get(), cur.notifymsgType, cur.ext1, cur.ext2, cur.ext3, cur.ext4);
                    pCb->doNotifyCb(cur.notifymsgType, cur.ext1, cur.ext2, cur.ext3, cur.ext4);
                }
            }
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
    AFCallBackSet *_this = static_cast<AFCallBackSet*>(arg);

    while(1){

        sem_wait(&(_this->m_DataSem));

        if (_this->DataQ.empty() && _this->DataStatus == END)  break;

        while(!(_this->DataQ.empty())){
            Data_Param cur;

            {
                Mutex::Autolock autoLock(_this->m_MutexData);
                cur = *(_this->DataQ.begin());
                _this->DataQ.erase(_this->DataQ.begin());
            }

            List<wp<I3ACallBack>> cbList = _this->getCallback();

            for (List<wp<I3ACallBack>>::iterator it = cbList.begin(); it != cbList.end(); it++)
            {
                sp<I3ACallBack> pCb = (*it).promote();
                if (pCb != NULL)
                {
                    if (_this->m_i4LogEn) MY_LOG("[%s] cb(%p)  datamsg %d, data %p, size %d  ======", __FUNCTION__, pCb.get(), cur.datamsgType, cur.datadata, cur.datasize);
                    pCb->doDataCb(cur.datamsgType, cur.datadata, cur.datasize);
                }
            }
        }
    }

    MY_LOG("[%s]-  ======", __FUNCTION__);
    ::pthread_exit((void*)0);
}

#endif
