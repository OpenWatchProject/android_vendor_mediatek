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
* @file HwEventIrq.cpp
* @brief Declarations of Hw IRQ Event waiting interface
*/
#define LOG_TAG "HwIRQ3A"

#include "IEventIrq.h"

#include <mutex>
#include <string>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning.h>
#include <private/IopipeUtils.h>
#include <assert.h>

namespace NS3Av3
{
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

/******************************************************************************
 *  Default EventIrq
 ******************************************************************************/
class HwEventIrq : public IEventIrq
{
public:
    /**
     * @brief Create instance of IEventIrq
     * @param [in] rCfg config of sensor, tg, and event for listening.
     * @param [in] strUser username
     */
    static HwEventIrq*          createInstance(const IEventIrq::ConfigParam& rCfg, const char* strUser);

    /**
     * @brief Destroy instance of IEventIrq
     * @param [in] strUser username
     */
    virtual MVOID               destroyInstance(const char* strUser);

    /**
     * @brief Register to listen the event
     * @return
     * - MINT32 0 for success
     */
    virtual MINT32              mark();

    /**
     * @brief Wait for the event
     * @param [out] rDuration
     * @return
     * - 0 for blocking wait case
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              wait(Duration& rDuration);

    /**
     * @brief Query for the event
     * @param [out] rDuration
     * @return
     * - 0 for indicating the event not yet happened
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              query(Duration& rDuration);

    /**
     * @brief flush hw signal
     * @return
     */
    virtual MVOID               flush();
    virtual MINT32              waitDequeue(BUF_PORTS port);

protected:
    HwEventIrq();
    virtual ~HwEventIrq(){}

    MVOID                       init(const IEventIrq::ConfigParam& rCfg, const char* strUser);
    MVOID                       uninit(const char* strUser);

    MINT32                      m_i4User;
    mutable std::mutex          m_Lock;

    IEventIrq::ConfigParam      m_rCfgParam;
    std::string                 m_strName;

    INormalPipe*                m_pNormalPipe;
    MINT32                      m_i4UserKey;
    EPipeSignal                 m_eSignal;
    MBOOL                       m_bFlush;
    MBOOL                       m_bWaiting;
    MINT32                      m_i4SensorDev;
};

template<MUINT32 eSensorDev, HwEventIrq::E_Event_Type eType>
class HwEventIrqType : public HwEventIrq
{
public:
    static HwEventIrqType* getInstance()
    {
        static HwEventIrqType<eSensorDev, eType> singleton;
        return &singleton;
    }
};

HwEventIrq::
HwEventIrq()
    : m_i4User(0)
    , m_Lock()
    , m_pNormalPipe(NULL)
    , m_i4UserKey(0)
    , m_eSignal(EPipeSignal_NONE)
    , m_bFlush(MFALSE)
    , m_bWaiting(MFALSE)
    , m_i4SensorDev(0)
{
    CAM_LOGD("[%s]", __FUNCTION__);
}

HwEventIrq*
HwEventIrq::
createInstance(const IEventIrq::ConfigParam& rCfg, const char* strUser)
{
    HwEventIrq* pHwEventIrq = NULL;
    pHwEventIrq = new HwEventIrq;

    if (pHwEventIrq)
    {
        pHwEventIrq->init(rCfg, strUser);
        return pHwEventIrq;
    }

    CAM_LOGE("dev(%d)", rCfg.i4SensorDev);
    static HwEventIrq _rHwEventIrq;
    _rHwEventIrq.init(rCfg, strUser);
    return &_rHwEventIrq;
}

MVOID
HwEventIrq::
destroyInstance(const char* strUser)
{
    uninit(strUser);
    delete this;
}

MVOID
HwEventIrq::
init(const IEventIrq::ConfigParam& rCfg, const char* strUser)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_i4User > 0)
    {
        CAM_LOGD_IF(1, "[%s] m_i4User(%d), m_strName(%s), strUser(%s)", __FUNCTION__, m_i4User, m_strName.c_str(), strUser);
        m_i4User ++;
        return;
    }

    m_rCfgParam = rCfg;
    m_strName = strUser;

    m_pNormalPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(rCfg.i4SensorIndex, strUser);
    m_i4UserKey = m_pNormalPipe->attach(strUser);
    switch (rCfg.eEventType)
    {
    default:
    case IEventIrq::E_Event_Vsync:
        m_eSignal = EPipeSignal_SOF;
        break;
    case IEventIrq::E_Event_Af:
        m_eSignal = EPipeSignal_AFDONE;
        break;
    case IEventIrq::E_Event_P1_Done:
        m_eSignal = EPipeSignal_EOF;
        break;
    }
    m_i4User ++;

    m_i4SensorDev = m_rCfgParam.i4SensorDev;
    CAM_LOGD("[%s](%s) this(%p) m_pNormalPipe(%p), userKey(%d), cfg(%d, %d, %d, %d)", __FUNCTION__, strUser, this, m_pNormalPipe, m_i4UserKey,
        m_rCfgParam.i4SensorDev, m_rCfgParam.i4SensorIndex, m_rCfgParam.eEventType, m_rCfgParam.u4TimeoutMs);
}

MVOID
HwEventIrq::
uninit(const char* strUser)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_i4User <= 0)
    {
        return;
    }

    m_i4User --;

    if (m_i4User == 0)
    {
        // uninit
        CAM_LOGD("[%s] m_strName(%s), strUser(%s)", __FUNCTION__, m_strName.c_str(), strUser);
        if (m_pNormalPipe)
        {
            m_pNormalPipe->destroyInstance(m_strName.c_str());
            m_pNormalPipe = MNULL;
        }
    }
    else
    {
        // do nothing
    }
}

MINT32
HwEventIrq::
mark()
{
    #if 0
    if (!m_pIspDrv->markIrq(m_rWaitIrq))
    {
        CAM_LOGE("Error");
        return -1;
    }
    else
    {
        CAM_LOGD("[%s] %s", __FUNCTION__, m_strName.c_str());
        return 0;
    }
    #else
        return -1;
    #endif
}

MINT32
HwEventIrq::
query(Duration& /*rDuration*/)
{
#if 0
    ISP_WAIT_IRQ_ST rWaitIrq = m_rWaitIrq;

    if (!m_pIspDrv->queryirqtimeinfo(&rWaitIrq))
    {
        CAM_LOGE("Error");
        return -1;
    }
    else
    {
        rDuration.i4Duration0 = rWaitIrq.TimeInfo.tmark2read_sec*1000000 + rWaitIrq.TimeInfo.tmark2read_usec;
        rDuration.i4Duration1 = rWaitIrq.TimeInfo.tevent2read_sec*1000000 + rWaitIrq.TimeInfo.tevent2read_usec;
        CAM_LOGD("[%s] %s: T0(%d), T1(%d), EventCnt(%d)", __FUNCTION__, m_strName.c_str(), rDuration.i4Duration0, rDuration.i4Duration1, rWaitIrq.TimeInfo.passedbySigcnt);
        return rWaitIrq.TimeInfo.passedbySigcnt;
    }
#else
    return -1;
#endif
}

MVOID
HwEventIrq::
flush()
{
    m_bFlush = MTRUE;
    MUINT32 count = 0;
    while(m_bWaiting && count < 3000)   //2604431 wait 10ms*3000 = 30s
    {
        CAM_LOGD("[%s] %s, m_eSignal(%d), m_i4UserKey(%d), count(%d)", __FUNCTION__, m_strName.c_str(), m_eSignal, m_i4UserKey, count);
        if(m_pNormalPipe != NULL)
            m_pNormalPipe->signal(m_eSignal,m_i4UserKey);
        usleep(50); // 0.05ms, avoid wait m_pNormalPipe done, but m_bWaiting still is true
        if(m_bWaiting)
            usleep(10000); //10ms
        count++;
    }
   assert(m_bWaiting == MFALSE);
}


MINT32
HwEventIrq::
wait(Duration& /*rDuration*/)
{
    m_bWaiting = MTRUE;
    if(m_bFlush)
    {
        CAM_LOGD("[%s] %s flush", __FUNCTION__, m_strName.c_str());
        return 0;
    }

    MUINT32 u4RetryCount = 2;
    while(m_bWaiting)
    {
        if (!m_pNormalPipe->wait(m_eSignal, EPipeSignal_ClearWait, m_i4UserKey, m_rCfgParam.u4TimeoutMs))
        {
            MUINT32 u4FrameStatus = 0;
            m_pNormalPipe->sendCommand(ENPipeCmd_GET_CUR_FRM_STATUS, (MINTPTR)&u4FrameStatus, 0, 0);
            if(u4FrameStatus == _drop_frame_status && u4RetryCount > 0)
            {
                u4RetryCount--;
                CAM_LOGW("[%s] %s This drop frame status. Retry wait Count(%d)", __FUNCTION__, m_strName.c_str(), u4RetryCount);
            } else
            {
                CAM_LOGE("Error");
                m_bWaiting = MFALSE;
                return -1;
            }
        }
        else
        {
            #if 0
            rDuration.i4Duration0 = rWaitIrq.TimeInfo.tmark2read_sec*1000000 + rWaitIrq.TimeInfo.tmark2read_usec;
            rDuration.i4Duration1 = rWaitIrq.TimeInfo.tevent2read_sec*1000000 + rWaitIrq.TimeInfo.tevent2read_usec;
            CAM_LOGD("[%s] %s: T0(%d), T1(%d), EventCnt(%d)", __FUNCTION__, m_strName.c_str(), rDuration.i4Duration0, rDuration.i4Duration1, rWaitIrq.TimeInfo.passedbySigcnt);
            return rWaitIrq.TimeInfo.passedbySigcnt;
            #else
            CAM_LOGD("[%s] %s", __FUNCTION__, m_strName.c_str());
            m_bWaiting = MFALSE;
            return 0;
            #endif
        }
    }
    return 0;
}

MINT32
HwEventIrq::
waitDequeue(BUF_PORTS port)
{
    if(m_bFlush)
    {
        CAM_LOGD("[%s] flush", __FUNCTION__);
        return 0;
    }

    IBufMgr* pBufMgr = Hal3ASttCtrl::getInstance(m_i4SensorDev)->getBufMgr(port);
    if(pBufMgr)
        pBufMgr->waitDequeue();
    else
        CAM_LOGE("[%s] pBufMgr NULL", __FUNCTION__);

    CAM_LOGD("[%s] done", __FUNCTION__);
    return 0;
}

#if 1 //(CAM3_3ATESTLVL > CAM3_3AUT)
IEventIrq*
IEventIrq::
createInstance(const ConfigParam& rCfg, const char* strUser)
{
    return HwEventIrq::createInstance(rCfg, strUser);
}
#endif

};
