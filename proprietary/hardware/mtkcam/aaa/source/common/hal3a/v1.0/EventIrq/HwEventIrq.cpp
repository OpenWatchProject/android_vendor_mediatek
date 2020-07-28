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

#include <utils/Mutex.h>
#include <string>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning.h>
#include <private/IopipeUtils.h>
#include <assert.h>

#if (CAM3_3A_ISP_30_EN)
#include <isp_drv.h>
#endif

namespace NS3Av3
{
using namespace android;
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
    MVOID                       createDrv();
    MVOID                       flushDrv();
    MBOOL                       waitDrv();
    MVOID                       uninitDrv();

    MINT32                      m_i4User;
    mutable Mutex               m_Lock;

    IEventIrq::ConfigParam      m_rCfgParam;
    std::string                 m_strName;
    MINT32                      m_i4UserKey;
    EPipeSignal                 m_eSignal;
    MBOOL                       m_bFlush;
    MBOOL                       m_bWaiting;
    MINT32                      m_i4SensorDev;

#if (!CAM3_3A_ISP_30_EN)
    INormalPipe*                m_pSttPipe;
#else
    IspDrv*                     m_pIspDrv;
    ISP_DRV_WAIT_IRQ_STRUCT     m_rWaitIrq;
#endif
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
    , m_i4UserKey(0)
    , m_eSignal(EPipeSignal_NONE)
    , m_bFlush(MFALSE)
    , m_bWaiting(MFALSE)
    , m_i4SensorDev(0)
#if (!CAM3_3A_ISP_30_EN)
    , m_pSttPipe(NULL)
#else
    , m_pIspDrv(NULL)
#endif
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
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {
        CAM_LOGD_IF(1, "[%s] m_i4User(%d), m_strName(%s), strUser(%s)", __FUNCTION__, m_i4User, m_strName.c_str(), strUser);
        m_i4User ++;
        return;
    }

    m_rCfgParam = rCfg;
    m_strName = strUser;

    createDrv();

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
    CAM_LOGD("[%s](%s) this(%p), userKey(%d), cfg(%d, %d, %d, %d)", __FUNCTION__, strUser, this, m_i4UserKey,
        m_rCfgParam.i4SensorDev, m_rCfgParam.i4SensorIndex, m_rCfgParam.eEventType, m_rCfgParam.u4TimeoutMs);
}

MVOID
HwEventIrq::
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
        uninitDrv();
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
    while(m_bWaiting && count < 3000) // wait 10ms * 1000 = 30s
    {
        CAM_LOGD("[%s] count(%d)", __FUNCTION__, count);
        flushDrv();
        usleep(50); // 0.05ms, avoid wait m_pSttPipe done, but m_bWaiting still is true
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

    if(!waitDrv())
    {
        CAM_LOGE("Error");
        m_bWaiting = MFALSE;
        return -1;
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

MVOID
HwEventIrq::
createDrv()
{
#if (!CAM3_3A_ISP_30_EN)
////////////////////////////////////
//-------- IP-Base driver --------//
////////////////////////////////////
    m_pSttPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_rCfgParam.i4SensorIndex, m_strName.c_str());
    m_i4UserKey = m_pSttPipe->attach(m_strName.c_str());
    CAM_LOGD("[%s] use IP-Base driver, m_pSttPipe(%p)", __FUNCTION__, m_pSttPipe);
#else
////////////////////////////////////
//--------Legacy 3.0 driver-------//
////////////////////////////////////
    m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv)
    {
        CAM_LOGE("[%s] IspDrv::createInstance() fail", __FUNCTION__);
    }
    else
    {
        if (!m_pIspDrv->init(LOG_TAG))
        {
            m_pIspDrv->destroyInstance();
            CAM_LOGE("[%s] pIspDrv->init() fail", __FUNCTION__);
        }
    }
    CAM_LOGD("[%s] use Legacy 3.0 driver, m_pIspDrv(%p)", __FUNCTION__, m_pIspDrv);

    m_i4UserKey = m_pIspDrv->registerIrq(m_strName.c_str());

    ISP_DRV_IRQ_TYPE_ENUM   type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    MUINT32                 status = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;

    if(m_rCfgParam.eEventType == E_Event_Vsync || m_rCfgParam.eEventType == E_Event_Vsync_Sensor)
    {
        CAM_LOGD("[%s] E_Event_Vsync", __FUNCTION__);
        switch(m_rCfgParam.i4TgInfo)
        {
            case CAM_TG_1:
                type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
                status = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
                break;
            case CAM_TG_2:
                type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                status = CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST;
                break;
            case CAM_SV_1:
                type = ISP_DRV_IRQ_TYPE_INT_CAMSV;
                status = CAMSV_INT_STATUS_VS1_ST;
                break;
            default:
                CAM_LOGE("[%s] Unsupport TG, use default, type(%d), status(%d)", __FUNCTION__, type, status);
                break;
        }
    }
    else if(m_rCfgParam.eEventType == E_Event_P1_Done)
    {
        switch(m_rCfgParam.i4TgInfo)
        {
            case CAM_TG_1:
                type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
                status = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
                break;
            case CAM_TG_2:
                type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                status = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
                break;
        }
    }
    else /*AF*/
    {
        type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        status = CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
    }
    CAM_LOGD("[%s] type(%d), status(%d)", __FUNCTION__, type, status);

    m_rWaitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
    m_rWaitIrq.UserInfo.Type    = type;
    m_rWaitIrq.UserInfo.Status  = status;
    m_rWaitIrq.UserInfo.UserKey = m_i4UserKey;
    m_rWaitIrq.Timeout          = m_rCfgParam.u4TimeoutMs;
    m_rWaitIrq.bDumpReg         = 0;
    m_rWaitIrq.UserNumber       = 0;
#endif
}

MVOID
HwEventIrq::
flushDrv()
{
#if (!CAM3_3A_ISP_30_EN)
////////////////////////////////////
//-------- IP-Base driver --------//
////////////////////////////////////
    CAM_LOGD("[%s] %s, m_eSignal(%d), m_i4UserKey(%d)", __FUNCTION__, m_strName.c_str(), m_eSignal, m_i4UserKey);
    if(m_pSttPipe != NULL)
        m_pSttPipe->signal(m_eSignal,m_i4UserKey);
#else
////////////////////////////////////
//--------Legacy 3.0 driver-------//
////////////////////////////////////
    CAM_LOGD("[%s] %s, Type(%d), Status(%d), UserKey(%d)", __FUNCTION__, m_strName.c_str(), m_rWaitIrq.UserInfo.Type, m_rWaitIrq.UserInfo.Status, m_rWaitIrq.UserInfo.UserKey);
    ISP_DRV_WAIT_IRQ_STRUCT rWaitIrq = m_rWaitIrq;
    if(m_pIspDrv != NULL)
        m_pIspDrv->flushIrq(rWaitIrq);
#endif
}

MBOOL
HwEventIrq::
waitDrv()
{
#if (!CAM3_3A_ISP_30_EN)
////////////////////////////////////
//-------- IP-Base driver --------//
////////////////////////////////////
    return m_pSttPipe->wait(m_eSignal, EPipeSignal_ClearWait, m_i4UserKey, m_rCfgParam.u4TimeoutMs);
#else
////////////////////////////////////
//--------Legacy 3.0 driver-------//
////////////////////////////////////
    ISP_DRV_WAIT_IRQ_STRUCT rWaitIrq = m_rWaitIrq;
    return m_pIspDrv->waitIrq(&rWaitIrq);
#endif
}

MVOID
HwEventIrq::
uninitDrv()
{
#if (!CAM3_3A_ISP_30_EN)
////////////////////////////////////
//-------- IP-Base driver --------//
////////////////////////////////////
    if (m_pSttPipe)
    {
        m_pSttPipe->destroyInstance(m_strName.c_str());
        m_pSttPipe = MNULL;
    }
#else
////////////////////////////////////
//--------Legacy 3.0 driver-------//
////////////////////////////////////
    if (m_pIspDrv)
    {
        if (m_pIspDrv->uninit(LOG_TAG))
        {
            m_pIspDrv->destroyInstance();
        }
        else
        {
            CAM_LOGE("[%s] m_pIspDrv->uninit() fail", __FUNCTION__);
        }
    }
    m_pIspDrv = MNULL;
#endif
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
