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
#define LOG_TAG "Hal3AFlowCtrl_SMVR"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <Hal3AFlowCtrl_SMVR.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <debug/DebugUtil.h>
#include <vector>

#include <mtkcam/drv/IHalSensor.h>

using namespace NS3Av3;
using namespace android;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define MY_LOGD(fmt, arg...) \
    do { \
        if (m_fgDebugLogWEn) { \
            CAM_LOGW(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if (m_fgDebugLogWEn) { \
            CAM_LOGW_IF(cond, __VA_ARGS__); \
        } else { \
            if ( (cond) ) { CAM_LOGD(__VA_ARGS__); } \
        } \
    }while(0)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// instancing (no user count protection, only referenced by Hal3AAdpater which
// controls the life cycle from init to uninit)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AIf*
Hal3AFlowCtrl_SMVR::
createInstance(MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return NULL;

    MINT32 i4SensorDev = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);

    switch (i4SensorDev)
    {
    case SENSOR_DEV_MAIN:
        Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_MAIN>::getInstance()->init(i4SensorDev, i4SensorOpenIndex, i4SubsampleCount);
        return Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_MAIN>::getInstance();
    break;
    case SENSOR_DEV_SUB:
        Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_SUB>::getInstance()->init(i4SensorDev, i4SensorOpenIndex, i4SubsampleCount);
        return Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_SUB>::getInstance();
    break;
    case SENSOR_DEV_MAIN_2:
        Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_MAIN_2>::getInstance()->init(i4SensorDev, i4SensorOpenIndex, i4SubsampleCount);
        return Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_MAIN_2>::getInstance();
    break;
    case SENSOR_DEV_SUB_2:
        Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_SUB_2>::getInstance()->init(i4SensorDev, i4SensorOpenIndex, i4SubsampleCount);
        return Hal3AFlowCtrl_SMVR_Dev<SENSOR_DEV_SUB_2>::getInstance();
    break;
    default:
        CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDev);
        AEE_ASSERT_3A_HAL("Unsupport sensor device.");
        return MNULL;
    }
}

MVOID
Hal3AFlowCtrl_SMVR::
destroyInstance()
{
    uninit();
}

Hal3AFlowCtrl_SMVR::
Hal3AFlowCtrl_SMVR()
    : Hal3AFlowCtrl()////fix resultsize
    , m_pBufferSensorThread(NULL)
    , m_i4SubsampleCount(1)
{}

Hal3AFlowCtrl_SMVR::
~Hal3AFlowCtrl_SMVR()
{}

MRESULT
Hal3AFlowCtrl_SMVR::
init(MINT32 i4SensorDev, MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount) //can be called only once by RAW or YUV, no user count
{
    // init ResultBuf
    IHal3AResultBuf::getInstance(i4SensorDev)->init(i4SubsampleCount * 12);

    m_i4SubsampleCount = i4SubsampleCount;

    MY_LOGD("[%s] +", __FUNCTION__);

    char cLogLevel[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    m_fgLogEn = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( m_fgLogEn == 0 ) {
        m_fgLogEn = ::property_get_int32("vendor.debug.camera.log.hal3a", 0);
    }
    m_fgDebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return NULL;

#if (CAM3_3ATESTLVL <= CAM3_3AUT)
    I3AWrapper::E_TYPE_T eType = I3AWrapper::E_TYPE_DFT;
#else
    MINT32 eSensorType = pHalSensorList->queryType(i4SensorOpenIndex);
    I3AWrapper::E_TYPE_T eType = I3AWrapper::E_TYPE_SMVR;
    switch (eSensorType)
    {
    case NSSensorType::eRAW:
        eType = I3AWrapper::E_TYPE_SMVR;
        break;
    case NSSensorType::eYUV:
        eType = I3AWrapper::E_TYPE_YUV;
        break;
    default:
        eType = I3AWrapper::E_TYPE_DFT;
        break;
    }
#endif


    m_i4SensorDev = i4SensorDev;
    m_i4SensorOpenIdx = i4SensorOpenIndex;

    MY_LOGD("[%s] eType(%d), sensor(%d), sensorIdx(%d)", __FUNCTION__, eType, i4SensorDev, i4SensorOpenIndex);

    // create 3A wrapper
    if (m_p3AWrap == NULL)
    {
        m_p3AWrap = I3AWrapper::createInstance(eType, i4SensorOpenIndex, i4SubsampleCount);
        if (m_p3AWrap)
        {
            MY_LOGD("[%s] m_p3AWrapper(%p) eType(%d) created OK", __FUNCTION__, m_p3AWrap, eType);
        }
        else
        {
            CAM_LOGE("m_p3AWrapper created fail!");
            AEE_ASSERT_3A_HAL("m_p3AWrapper created fail!");
        }
    }

    // create Vsync event
    //IEventIrq::ConfigParam IrqConfig(i4SensorDev, i4SensorOpenIndex, 5000, IEventIrq::E_Event_Vsync);
    //m_pEventIrq = IEventIrq::createInstance(IrqConfig, "VSIrq");

    // create statistic control
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if (m_p3ASttCtrl == NULL && eType == I3AWrapper::E_TYPE_SMVR)
    {
        m_p3ASttCtrl = Hal3ASttCtrl::createInstance(i4SensorDev, i4SensorOpenIndex);
        if (m_p3ASttCtrl)
        {
            MY_LOGD("[%s] m_p3ASttCtrl(%p) created OK", __FUNCTION__, m_p3ASttCtrl);
        }
        else
        {
            CAM_LOGE("m_p3ASttCtrl created fail!");
            AEE_ASSERT_3A_HAL("m_p3ASttCtrl created fail!");
        }
    }
#endif

    // create AA thread
    if (m_pThread == NULL)
    {
        m_pThread = IThread3A::createInstance(this);
        if (m_pThread)
        {
            MY_LOGD("[%s] m_pThread(%p) created OK", __FUNCTION__, m_pThread);
        }
        else
        {
            CAM_LOGE("m_pThread created fail!");
            AEE_ASSERT_3A_HAL("m_pThread created fail!");
        }
    }
    // create Sensor Buffer mode thread

  /*
    if (m_pBufferSensorThread == NULL)
    {
        m_pBufferSensorThread = ThreadSensorBufferMode::createInstance(m_i4SensorDev, m_i4SensorOpenIdx);
        if (m_pBufferSensorThread)
        {
            MY_LOGD("[%s] m_pBufferSensorThread(%p) created OK", __FUNCTION__, m_pBufferSensorThread);
        }
        else
        {
            CAM_LOGE("m_pBufferSensorThread created fail!");
            AEE_ASSERT_3A_HAL("m_pBufferSensorThread created fail!");
        }
    }
*/
    MY_LOGD("[%s] -", __FUNCTION__);
    return S_3A_OK;
}

MRESULT
Hal3AFlowCtrl_SMVR::
uninit() //can be called only once by RAW or YUV, no user count
{
    MY_LOGD("[%s] +", __FUNCTION__);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    if (m_pThread)
    {
        m_pThread->destroyInstance();
        m_pThread = NULL;
    }
/*
    if (m_pBufferSensorThread != NULL)
    {
        m_pBufferSensorThread->destroyInstance();
        m_pBufferSensorThread = NULL;
    }
*/
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if (m_p3ASttCtrl)
    {
        m_p3ASttCtrl->destroyInstance();
        m_p3ASttCtrl = NULL;
    }
#endif

    if (m_p3AWrap)
    {
        m_p3AWrap->destroyInstance();
        m_p3AWrap = NULL;
    }

    if (m_pCbSet)
    {
        m_pCbSet = NULL;
        CAM_LOGE("User did not detach callbacks!");
    }

    IHal3AResultBuf::getInstance(m_i4SensorDev)->uninit();

    MY_LOGD("[%s] -", __FUNCTION__);
    return S_3A_OK;
}

MINT32
Hal3AFlowCtrl_SMVR::
config()
{
    MY_LOGD("[%s]", __FUNCTION__);
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->initStt(m_i4SensorDev, m_i4SensorOpenIdx, m_i4SubsampleCount);
#endif
    return m_p3AWrap->config();
}


MBOOL
Hal3AFlowCtrl_SMVR::
doUpdateCmdDummy()
{
    resetParams();
    setParams(m_rParam);
    setAfParams(m_rAFParam);

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4InitCount = 0; i4InitCount < m_i4SubsampleCount; i4InitCount++) //avoid dummy request
        rRequestSet.vNumberSet.push_back(0);
    NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 0, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
    if (m_bCallBack != MTRUE)
        _3AProf.iValidateOpt = NS3Av3::ParamIspProfile_T::EParamValidate_None;
    return sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
}

MBOOL
Hal3AFlowCtrl_SMVR::
postCommand(ECmd_T const eCmd, const ParamIspProfile_T* pParam)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] enter(%d)", __FUNCTION__, eCmd);
    MUINT32 u4MagicNum = 0;
    MUINT32 u4MagicNumCur = 0;
    MBOOL bMainFlash = MFALSE;
    MBOOL bPre = MFALSE;
    ParamIspProfile_T rParamIspProf = *pParam;
    m_rCapParam.u4CapType = E_CAPTURE_NORMAL;
    RequestSet_T rRequestSet;

    MBOOL fgUpdate = (eCmd == ECmd_Update) && (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None);
    if (eCmd == ECmd_CameraPreviewStart)
    {
        IHal3AResultBuf::getInstance(m_i4SensorDev)->uninit();
        if (m_pEventIrq == NULL)
        {
            IEventIrq::ConfigParam IrqConfig(m_i4SensorDev, m_i4SensorOpenIdx, 5000000, IEventIrq::E_Event_Vsync);
            m_pEventIrq = IEventIrq::createInstance(IrqConfig, "VSIrq");
        }
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->startStt();
#endif
        m_p3AWrap->start();
    }

    if (eCmd == ECmd_Update)
    {
        MY_LOGD("m_rParam.u4AeMode(%d), m_rParam.i8ExposureTime(%lld)",m_rParam.u4AeMode, m_rParam.i8ExposureTime);
        if (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
        {
            u4MagicNum = rParamIspProf.i4MagicNum;
            u4MagicNumCur = rParamIspProf.i4MagicNumCur;
            rRequestSet = rParamIspProf.rRequestSet;

            bMainFlash = m_p3AWrap->chkMainFlashOnCond();
            bPre = m_p3AWrap->chkPreFlashOnCond();
            rParamIspProf.bMainFlash = bMainFlash;
                MY_LOGD_IF(m_fgLogEn, "[%s] (bMainFlash,bPre):(%d,%d)", __FUNCTION__, bMainFlash,bPre);

            if ((bMainFlash||bPre) && u4MagicNum != 0)
                m_u4FlashOnIdx = u4MagicNum;
            if (bMainFlash)
            {
                m_p3AWrap->setFlashLightOnOff(1, 1);
                m_u4MainFlashOnIdx = u4MagicNum;
            }
            if (bPre) m_p3AWrap->setFlashLightOnOff(1, 0);

            m_p3AWrap->postCommand(eCmd, &rParamIspProf);

            if (m_p3AWrap->chkCapFlash())
            {
                m_rCapParam.u4CapType = E_CAPTURE_HIGH_QUALITY_CAPTURE;
            }
            MY_LOGD_IF(m_fgLogEn, "[%s][HQC] inform p1 capture type (%d)", __FUNCTION__, m_rCapParam.u4CapType);

            u4MagicNumCur = m_p3AWrap->queryMagicNumber();
            on3AProcFinish(rRequestSet, u4MagicNumCur);
        }else {
            if (m_rParam.u4AeMode == 0 && m_rParam.i8ExposureTime >= 400000000)
            {
                m_p3AWrap->postCommand(eCmd, &rParamIspProf);
            }
        }
    }
    else
    {
        m_p3AWrap->postCommand(eCmd, &rParamIspProf);
    }

    if (eCmd == ECmd_CameraPreviewEnd)
    {
        MY_LOGD("[%s] reset bMainFlash", __FUNCTION__);
        m_u4FlashOnIdx = -1;

#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
        if(m_p3ASttCtrl)
            m_p3ASttCtrl->stopStt();
#endif

        m_p3AWrap->stop();

#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
        if(m_p3ASttCtrl)
        m_p3ASttCtrl->uninitStt();
#endif

        if (m_pEventIrq)
        {
            m_pEventIrq->destroyInstance("VSIrq");
            m_pEventIrq = NULL;
        }
    }

    return MTRUE;
}

MVOID
Hal3AFlowCtrl_SMVR::
on3AProcFinish(const RequestSet_T rRequestSet, MUINT32 u4MagicNumCur)
{
    RequestSet_T RequestCbSet;
    RequestCbSet = rRequestSet;
    std::vector<MINT32> rNumberSet = RequestCbSet.vNumberSet;
    std::vector<MINT32>::iterator it;
    MUINT32 u4MagicNum = 0;
    MINT32 i4SubsampleIndex = 0;
    for (it = rNumberSet.begin(); it != rNumberSet.end(); it++)
    {
        u4MagicNum = (MUINT32)(*it);
        AAA_TRACE_D("updateResult#(%d,%d)", u4MagicNum, u4MagicNumCur);
        updateResult(u4MagicNum, i4SubsampleIndex);
        AAA_TRACE_END_D;
        i4SubsampleIndex++;
    }
    m_rCapParam.i8ExposureTime = m_rResult.i8SensorExposureTime;
    u4MagicNum = rNumberSet[0];
    if (m_pCbSet)
    {
        MINT32 i4CurId = m_p3AWrap->getCurrentHwId();

        MY_LOGD_IF(m_fgLogEn, "[%s] #(%d,%d), SOF(0x%x), m_rResult.i8SensorExposureTime(%lld), m_rCapParam.capType(%d)",
                   __FUNCTION__, u4MagicNum, u4MagicNumCur, i4CurId, m_rCapParam.i8ExposureTime,m_rCapParam.u4CapType);
        AAA_TRACE_D("3A_CB #(%d), SOF(%d)", u4MagicNum, i4CurId);
        AAA_TRACE_G(doNotifyCbFinish);
        AAA_TRACE_NL(doNotifyCbFinish);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_3APROC_FINISH,
            reinterpret_cast<MINTPTR> (&RequestCbSet),     // magic number
            i4CurId,        // SOF idx
            reinterpret_cast<MINTPTR> (&m_rCapParam));
        AAA_TRACE_END_G;
        AAA_TRACE_END_NL;

#warning "FIXME"
/*
        // for flash state
        if (!(m_rParam.u4HalFlag & HAL_FLG_DUMMY))
        {
            m_pCbSet->doNotifyCb(
                I3ACallBack::eID_NOTIFY_CURR_RESULT,
                u4MagicNumCur,
                MTK_FLASH_STATE,
                m_rResult.u1FlashState);
        }
*/
        AAA_TRACE_END_D;
    }
}

MVOID
Hal3AFlowCtrl_SMVR::
updateResult(MUINT32 u4MagicNum, MINT32 i4SubsampleIndex)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    // pass1 result
    MINT32 i4Ret = m_p3AWrap->getCurrResult(u4MagicNum, m_rResult, i4SubsampleIndex);
    if (i4Ret == E_Repeat_OFF)
    {
        MY_LOGD_IF(m_fgLogEn, "[%s] updateRepeatQueue(%d)", __FUNCTION__, u4MagicNum);
        IHal3AResultBuf::getInstance(m_i4SensorDev)->updateRepeatQueue(u4MagicNum);
    }
    IHal3AResultBuf::getInstance(m_i4SensorDev)->updateResult(u4MagicNum, m_rResult);
    // result for high quality
    {
        ISP_SENSOR_INFO_T rIspSensorInfo;
        rIspSensorInfo.i4FrameId = u4MagicNum;
        rIspSensorInfo.u4AfeGain = m_rResult.rCamInfo.rAEInfo.u4AfeGain;
        rIspSensorInfo.u4IspGain = m_rResult.rCamInfo.rAEInfo.u4IspGain;
        rIspSensorInfo.bHLREnable = MFALSE;
        m_p3AWrap->setIspSensorInfo2AF(rIspSensorInfo);
    }
    #if 0
    Mutex::Autolock autoLock(m_rResultMtx);

    m_rResultBufCur.updateResult(m_u4FrmIdStat, m_rResult);
    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d) HQC", __FUNCTION__, u4MagicNum);
    m_rResultCond.broadcast();
    #endif
}
