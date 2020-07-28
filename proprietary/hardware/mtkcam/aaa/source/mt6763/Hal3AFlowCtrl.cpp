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
#define LOG_TAG "Hal3AFlowCtrl"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <Hal3AFlowCtrl.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <debug/DebugUtil.h>
#include <vector>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <SttBufQ.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/hw/HwTransform.h>

using namespace NS3Av3;
using namespace android;
using namespace NSCamHW;

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

template <MINT32 sensorDev>
class Hal3AFlowCtrlDev : public Hal3AFlowCtrl
{
public:
    static Hal3AFlowCtrlDev* getInstance()
    {
        static Hal3AFlowCtrlDev<sensorDev> singleton;
        return &singleton;
    }
    Hal3AFlowCtrlDev()
        : Hal3AFlowCtrl()
    {
    }

private:

};

inline static
CameraArea_T _transformArea2Active(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    MBOOL fgLog = ::property_get_int32("vendor.debug.3a.transformArea2Active", 0);
    CameraArea_T rOut;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixToActive(i4SensorMode, mat))
        MY_ERR("Get hw matrix failed");
    if(fgLog)
        mat.dump(__FUNCTION__);

    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;

    CAM_LOGW_IF(fgLog, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
              input.p.x, input.p.y, input.s.w, input.s.h,
              output.p.x, output.p.y, output.s.w, output.s.h);
    CAM_LOGW_IF(fgLog, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
              rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
              rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    return rOut;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// instancing (no user count protection, only referenced by Hal3AAdpater which
// controls the life cycle from init to uninit)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AIf*
Hal3AFlowCtrl::
createInstance(MINT32 i4SensorOpenIndex)
{
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return NULL;

    MINT32 i4SensorDev = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);

    switch (i4SensorDev)
    {
    case SENSOR_DEV_MAIN:
        Hal3AFlowCtrlDev<SENSOR_DEV_MAIN>::getInstance()->init(i4SensorDev, i4SensorOpenIndex);
        return Hal3AFlowCtrlDev<SENSOR_DEV_MAIN>::getInstance();
    break;
    case SENSOR_DEV_SUB:
        Hal3AFlowCtrlDev<SENSOR_DEV_SUB>::getInstance()->init(i4SensorDev, i4SensorOpenIndex);
        return Hal3AFlowCtrlDev<SENSOR_DEV_SUB>::getInstance();
    break;
    case SENSOR_DEV_MAIN_2:
        Hal3AFlowCtrlDev<SENSOR_DEV_MAIN_2>::getInstance()->init(i4SensorDev, i4SensorOpenIndex);
        return Hal3AFlowCtrlDev<SENSOR_DEV_MAIN_2>::getInstance();
    break;
    case SENSOR_DEV_SUB_2:
        Hal3AFlowCtrlDev<SENSOR_DEV_SUB_2>::getInstance()->init(i4SensorDev, i4SensorOpenIndex);
        return Hal3AFlowCtrlDev<SENSOR_DEV_SUB_2>::getInstance();
    break;
    default:
        CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDev);
        AEE_ASSERT_3A_HAL("Unsupport sensor device.");
        return MNULL;
    }
}

MVOID
Hal3AFlowCtrl::
destroyInstance()
{
    uninit();
}

Hal3AFlowCtrl::
Hal3AFlowCtrl()
    : Hal3AIf()
    , m_fgLogEn(MFALSE)
    , m_fgDebugLogWEn(MFALSE)
    , m_i4SensorDev(0)
    , m_i4SensorOpenIdx(0)
    , m_u4FrmIdStat(0)
    , m_u4FrmIdFreeze(0)
    , m_u4FlashOnIdx(-1)
    , m_u4MainFlashOnIdx(-1)
    , m_fgFlashOn(0)
    , m_fgInternalFlashOn(0)
    , m_p3AWrap(NULL)
    , m_pThread(NULL)
    , m_pEventIrq(NULL)
    , m_p3ASttCtrl(NULL)
    , m_pCbSet(NULL)
    , m_fgPreStop(MFALSE)
    , m_bCallBack(MFALSE)
    , m_bRRZDump(0)
    , m_u4PreMagicReq(0)
    , m_bTgIntAEEn(0)
    , m_fTgIntAERatio(0.0)
    , m_ThreadStt()
    , m_SttBufQEnable(0)
    , m_i4SensorMode(0)
{}

Hal3AFlowCtrl::
~Hal3AFlowCtrl()
{}

MRESULT
Hal3AFlowCtrl::
init(MINT32 i4SensorDev, MINT32 i4SensorOpenIndex) //can be called only once by RAW or YUV, no user count
{
    MY_LOGD("[%s] +", __FUNCTION__);

    m_fgLogEn = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( m_fgLogEn == 0 ) {
        m_fgLogEn = ::property_get_int32("vendor.debug.camera.log.hal3a", 0);
    }
    m_u4FrmIdFreeze = ::property_get_int32("vendor.debug.3a.freeze", 0);
    m_bRRZDump = 0;
    m_fgDebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    //MRESULT ret = S_3A_OK;  /* fix build warning */
    //MBOOL bRet = MTRUE;  /* fix build warning */

    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return 0;

#if (CAM3_3ATESTLVL <= CAM3_3AUT)
    I3AWrapper::E_TYPE_T eType = I3AWrapper::E_TYPE_DFT;
#else
    MINT32 eSensorType = pHalSensorList->queryType(i4SensorOpenIndex);
    I3AWrapper::E_TYPE_T eType = I3AWrapper::E_TYPE_RAW;
    switch (eSensorType)
    {
    case NSSensorType::eRAW:
        eType = I3AWrapper::E_TYPE_RAW;
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
        m_p3AWrap = I3AWrapper::createInstance(eType, i4SensorOpenIndex);
        if (m_p3AWrap)
        {
            MY_LOGD_IF(m_fgLogEn, "[%s] m_p3AWrapper(%p) created OK", __FUNCTION__, m_p3AWrap);
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

    m_SttBufQEnable = property_get_int32("vendor.debug.camera.SttBufQ.enable", 0);
    if (m_SttBufQEnable) {
        ISttBufQ::createSingleton(m_i4SensorDev, LOG_TAG);
    }

    // create statistic control
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if (m_p3ASttCtrl == NULL  && eType == I3AWrapper::E_TYPE_RAW)
    {
        m_p3ASttCtrl = Hal3ASttCtrl::createInstance(i4SensorDev, i4SensorOpenIndex);
        if (m_p3ASttCtrl)
        {
            MY_LOGD_IF(m_fgLogEn, "[%s] m_p3ASttCtrl(%p) created OK", __FUNCTION__, m_p3ASttCtrl);
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
            MY_LOGD_IF(m_fgLogEn, "[%s] m_pThread(%p) created OK", __FUNCTION__, m_pThread);
        }
        else
        {
            CAM_LOGE("m_pThread created fail!");
            AEE_ASSERT_3A_HAL("m_pThread created fail!");
        }
    }

    // init ResultBuf
    MY_LOGD("[%s] Init ResultBuf(12)", __FUNCTION__);
    IHal3AResultBuf::getInstance(m_i4SensorDev)->init(12);

    MY_LOGD("[%s] -", __FUNCTION__);
    return S_3A_OK;
}

MRESULT
Hal3AFlowCtrl::
uninit() //can be called only once by RAW or YUV, no user count
{
    MY_LOGD("[%s] +", __FUNCTION__);

    //MRESULT ret = S_3A_OK;  /* fix build warning */
    //MBOOL bRet = MTRUE;  /* fix build warning */

    if (m_pThread)
    {
        m_pThread->destroyInstance();
        m_pThread = NULL;
    }

#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if (m_p3ASttCtrl)
    {
        m_p3ASttCtrl->destroyInstance();
        m_p3ASttCtrl = NULL;
    }
#endif

    if (m_SttBufQEnable) {
        ISttBufQ::destroySingleton(m_i4SensorDev, LOG_TAG);
    }

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3AFlowCtrl::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
    {
        m_p3ASttCtrl->setSensorDevInfo(m_i4SensorDev, m_i4SensorOpenIdx);
        m_p3ASttCtrl->getPdInfoForSttCtrl();
    }
    m_rConfigInfo = rConfigInfo;
    MVOID* ThreadSttret;
    ::pthread_create(&m_ThreadStt, NULL, Hal3AFlowCtrl::ThreadConfigSttpipe, this);
#endif
    MBOOL bRetConfig = m_p3AWrap->config(rConfigInfo);
    MBOOL bRetCCUCB = m_p3AWrap->send3ACtrl(E3ACtrl_SetCCUCB, (MINTPTR)this, NULL);
    MBOOL bRetTgInt = m_p3AWrap->send3ACtrl(E3ACtrl_EnableTgInt, reinterpret_cast<MINTPTR>(&m_bTgIntAEEn), reinterpret_cast<MINTPTR>(&m_fTgIntAERatio));
    CAM_LOGD_IF(m_fgLogEn,"[%s()] m_bTgIntAEEn:%d, TG_INT_AE_RATIO:%f", __FUNCTION__, m_bTgIntAEEn, m_fTgIntAERatio);

    ::pthread_join(m_ThreadStt, &ThreadSttret);
    if (ThreadSttret)    CAM_LOGD("Thread Config Sttpipe create failed");

    return (bRetConfig & bRetCCUCB & bRetTgInt);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Stop Stt
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
stopStt()
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    if(m_p3AWrap)
        m_p3AWrap->notifyPreStop();
    if(m_p3ASttCtrl)
    m_p3ASttCtrl->preStopStt();
    resume();
    m_fgPreStop = MTRUE;
    m_pThread->enableListenUpdate(MFALSE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pause
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
pause()
{
    CAM_LOGD("[%s] pause +", __FUNCTION__);
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->pause();
#endif
    if(m_p3AWrap)
        m_p3AWrap->pause();
    if(m_pThread)
        m_pThread->pause();
    CAM_LOGD("[%s] pause -", __FUNCTION__);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Resume
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
resume(MINT32 MagicNum)
{
    CAM_LOGD("[%s] resume +", __FUNCTION__);
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->resume();
#endif
    if(m_p3AWrap)
        m_p3AWrap->resume(MagicNum);
    if(m_pThread)
        m_pThread->resume();
    CAM_LOGD("[%s] resume -", __FUNCTION__);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// flush VSirq
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
flushVSirq()
{
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->abortDeque();
#endif
    MY_LOGD("[%s] abortDeque done", __FUNCTION__);
    if(m_pEventIrq)
        m_pEventIrq->flush();
    MY_LOGD("[%s] flushVSirq done", __FUNCTION__);
}


// request/result & callback flows
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3AFlowCtrl::
sendCommand(ECmd_T const eCmd, MUINTPTR const i4Arg)
{
    return m_pThread->sendCommand(eCmd, i4Arg);
}

MBOOL
Hal3AFlowCtrl::
doUpdateCmd(const ParamIspProfile_T* pParam)
{
    MINT32 i4MagicNum = pParam->i4MagicNum;
    if (m_pThread->isEnd() || m_fgPreStop)
    {
        MY_LOGD("[%s] Stop updating #(%d)", __FUNCTION__, i4MagicNum);
        return MTRUE;
    }
    //call notifyResult4TG API
    MY_LOGD_IF(m_fgLogEn, "[%s] notifyResult4TG Previous FrmId #(%d) New FrmId #(%d) bTgIntAEEn(%d)",__FUNCTION__, m_rResult.i4FrmId, i4MagicNum, m_bTgIntAEEn);
    if(m_rResult.i4FrmId != 0 && m_bTgIntAEEn)
        notifyResult4TG(m_rResult.i4FrmId);

    AAA_TRACE_D("3A_Update #(%d)", i4MagicNum);
    MBOOL fgRet = postCommand(ECmd_Update, pParam);
    AAA_TRACE_END_D;
    if (!fgRet)
    {
        CAM_LOGE("ECmd_Update failed");
        return MFALSE;
    }

    if(m_fgPreStop)
    {
        MY_LOGD("[%s] Stop wait vsirq #(%d)", __FUNCTION__, i4MagicNum);
        return MTRUE;
    }
    IEventIrq::Duration duration;
    MY_LOGD_IF(m_fgLogEn, "[%s] start waitVSirq.", __FUNCTION__);
    AAA_TRACE_D("3A_WaitVSync #(%d)", i4MagicNum);
    AAA_TRACE_G(WaitVSync);
    m_pEventIrq->wait(duration);
    AAA_TRACE_END_G;
    AAA_TRACE_END_D;

    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
doUpdateCmdDummy()
{
    resetParams();
    setParams(m_rParam);
    setAfParams(m_rAFParam);

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    rRequestSet.vNumberSet.push_back(0);
    NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 0, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
    if (m_bCallBack != MTRUE)
        _3AProf.iValidateOpt = NS3Av3::ParamIspProfile_T::EParamValidate_None;
    return sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
}

MBOOL
Hal3AFlowCtrl::
postCommand(ECmd_T const eCmd, const ParamIspProfile_T* pParam)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] enter(%d)", __FUNCTION__, eCmd);
    MUINT32 u4MagicNum = 0;
    MUINT32 u4MagicNumCur = 0;
    MBOOL bMainFlash = MFALSE;
    MBOOL bPre = MFALSE;
    ParamIspProfile_T rParamIspProf = *pParam;
    m_rCapParam.u4CapType = E_CAPTURE_NORMAL;
    if (eCmd == ECmd_CameraPreviewStart)
    {
        IHal3AResultBuf::getInstance(m_i4SensorDev)->uninit();
        m_rParam.i8ExposureTime = 0;
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
        MY_LOGD("m_rParam.u4AeMode(%d), m_rParam.i8ExposureTime(%ld)", (MINT32)m_rParam.u4AeMode, (long)m_rParam.i8ExposureTime);
        if (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
        {
            u4MagicNum = rParamIspProf.i4MagicNum;
            u4MagicNumCur = rParamIspProf.i4MagicNumCur;
            bMainFlash = m_p3AWrap->chkMainFlashOnCond();
            bPre = m_p3AWrap->chkPreFlashOnCond();
            rParamIspProf.bMainFlash = bMainFlash;
            MY_LOGD_IF(m_fgLogEn, "[%s] (bMainFlash,bPre):(%d,%d)", __FUNCTION__, bMainFlash,bPre);

            if (bMainFlash && (MINT32)m_u4FlashOnIdx==-1)
            {
                m_p3AWrap->setFlashLightOnOff(1, 1);
                m_u4MainFlashOnIdx = u4MagicNum;
                m_fgInternalFlashOn = MFALSE;
            }
            if (bPre)
            {
                m_p3AWrap->setFlashLightOnOff(1, 0);
                m_fgInternalFlashOn = MFALSE;
            }
            if ((bMainFlash||bPre) && u4MagicNum != 0)
            {
                m_u4FlashOnIdx = u4MagicNum;
                m_fgFlashOn = 1;
            }

            m_p3AWrap->postCommand(eCmd, &rParamIspProf);

            if (m_p3AWrap->chkCapFlash())
            {
                m_rCapParam.u4CapType = E_CAPTURE_HIGH_QUALITY_CAPTURE;
            }
            MY_LOGD_IF(m_fgLogEn, "[%s][HQC] inform p1 capture type (%d)", __FUNCTION__, m_rCapParam.u4CapType);

            u4MagicNumCur = m_p3AWrap->queryMagicNumber();
            on3AProcFinish(u4MagicNum, u4MagicNumCur);
        }else {
            if (m_rParam.u4AeMode == 0 && m_rParam.i8ExposureTime >= 400000000)
            {
                m_p3AWrap->postCommand(eCmd, &rParamIspProf);
            }
        }
        IspTuningBufCtrl::getInstance(m_i4SensorDev)->clearP1Buffer();
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
        m_fgPreStop = MFALSE;
    }

    return MTRUE;
}

MVOID
Hal3AFlowCtrl::
on3AProcSet(MBOOL bCallBackFlag)
{
    m_bCallBack = bCallBackFlag;
    MY_LOGD_IF(m_fgLogEn,"[%s] Vysnc CB, bCallBackFlag(%d)",__FUNCTION__, bCallBackFlag);
    if (m_pCbSet && bCallBackFlag) {
        AAA_TRACE_G(doNotifyCbSet);
        AAA_TRACE_NL(doNotifyCbSet);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_VSYNC_DONE,
            0,     // magic number
            0,        // SOF idx
            0);
        AAA_TRACE_END_NL;
        AAA_TRACE_END_G;
    }
}

MVOID
Hal3AFlowCtrl::
on3AProcFinish(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur)
{
    RequestSet_T RequestCbSet;

    AAA_TRACE_D("updateResult#(%d,%d)", u4MagicNum, u4MagicNumCur);
    updateResult(u4MagicNum, u4MagicNumCur);
    AAA_TRACE_END_D;

    MINT32 i4AeShutDelayFrame;
    MINT32 i4AeISPGainDelayFrame;

    m_u4PreMagicReq = u4MagicNum;

    send3ACtrl(E3ACtrl_GetSensorDelayInfo, reinterpret_cast<MINTPTR>(&i4AeShutDelayFrame), reinterpret_cast<MINTPTR>(&i4AeISPGainDelayFrame));

    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d), i4AeShutDelayFrame(%d), i4AeISPGainDelayFrame(%d)", __FUNCTION__, u4MagicNum, i4AeShutDelayFrame, i4AeISPGainDelayFrame);

    //Shutter delay is 3 when sensor type is YUV. 3A will call back PreMagicum is (u4MagicNum-1)
    MINT32 i4ShutterDelay = (i4AeISPGainDelayFrame - i4AeShutDelayFrame);
    if(i4ShutterDelay < 2)
        i4ShutterDelay = 2;
    MUINT32 u4PreMagicNum = u4MagicNum - ( i4ShutterDelay - 2);

    if(u4PreMagicNum != u4MagicNum)
    {
        Result_T rPrerResult;
        MINT32 i4Ret = IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(u4PreMagicNum, rPrerResult);

        if (-1 == i4Ret)
        {
            MY_LOGD("Fail get Result(%d)", u4PreMagicNum);
        }
        m_rCapParam.i8ExposureTime = rPrerResult.i8SensorExposureTime;
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AF_STATE, rPrerResult.u1AfState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_STATE, rPrerResult.u1AeState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FLASH_STATE, rPrerResult.u1FlashState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_MODE, rPrerResult.u1AeMode);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_3A_AE_BV_TRIGGER, rPrerResult.fgAeBvTrigger);
        if( rPrerResult.vecFocusAreaPos.size()!=0)
        {
            MINT32  szW = rPrerResult.i4FocusAreaSz[0];
            MINT32  szH = rPrerResult.i4FocusAreaSz[1];
            const MINT32 *ptrPos = rPrerResult.vecFocusAreaPos.array();
            MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
            android::Vector<MINT32> vecCvtPos = rPrerResult.vecFocusAreaPos;
            MINT32 *ptrCvtPos = vecCvtPos.editArray();
            szW = szW/2;
            szH = szH/2;
            for( MUINT32 i=0; i<rPrerResult.vecFocusAreaPos.size(); i+=2)
            {
                CameraArea_T pos;
                MINT32 X = *ptrPos++;
                MINT32 Y = *ptrPos++;

                pos.i4Left   = X - szW;
                pos.i4Top    = Y - szH;
                pos.i4Right  = X + szW;
                pos.i4Bottom = Y + szH;
                pos.i4Weight = 0;

                pos = _transformArea2Active(m_i4SensorOpenIdx, m_i4SensorMode, pos);
                cvtX = (pos.i4Left + pos.i4Right )/2;
                cvtY = (pos.i4Top  + pos.i4Bottom)/2;
                *ptrCvtPos++ = cvtX;
                *ptrCvtPos++ = cvtY;

                cvtW = pos.i4Right  -  pos.i4Left;
                cvtH = pos.i4Bottom -  pos.i4Top;
            }
            if( m_fgLogEn)
            {
                for( MUINT32 i=0; i<vecCvtPos.size(); i++)
                {
                    MY_LOGD_IF(m_fgLogEn, "Pos %d(%d)", rPrerResult.vecFocusAreaPos[i], vecCvtPos[i]);
                }

                for( MUINT32 i=0; i<rPrerResult.vecFocusAreaRes.size(); i++)
                {
                    MY_LOGD_IF(m_fgLogEn, "Res (%d)", rPrerResult.vecFocusAreaRes[i]);
                }
                MY_LOGD_IF(m_fgLogEn, "W %d(%d), H %d(%d)", rPrerResult.i4FocusAreaSz[0], cvtW, rPrerResult.i4FocusAreaSz[1], cvtH);
            }
            MSize areaCvtSZ(cvtW, cvtH);
            UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FOCUS_AREA_SIZE, areaCvtSZ);
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_POSITION, vecCvtPos.array(), vecCvtPos.size());
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_RESULT, rPrerResult.vecFocusAreaRes.array(), rPrerResult.vecFocusAreaRes.size());
        }
        RequestCbSet.vNumberSet.clear();
        RequestCbSet.vNumberSet.push_back(((MINT32)(u4PreMagicNum)));
    }
    else
    {
        m_rCapParam.i8ExposureTime = m_rResult.i8SensorExposureTime;
        AAA_TRACE_L(UpdateSingleEntry);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AF_STATE, m_rResult.u1AfState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_STATE, m_rResult.u1AeState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FLASH_STATE, m_rResult.u1FlashState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_MODE, m_rResult.u1AeMode);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_3A_AE_BV_TRIGGER, m_rResult.fgAeBvTrigger);

        if( m_rResult.vecFocusAreaPos.size()!=0)
        {
            MINT32  szW = m_rResult.i4FocusAreaSz[0];
            MINT32  szH = m_rResult.i4FocusAreaSz[1];
            const MINT32 *ptrPos = m_rResult.vecFocusAreaPos.array();
            MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
            android::Vector<MINT32> vecCvtPos = m_rResult.vecFocusAreaPos;
            MINT32 *ptrCvtPos = vecCvtPos.editArray();
            szW = szW/2;
            szH = szH/2;
            for( MUINT32 i=0; i<m_rResult.vecFocusAreaPos.size(); i+=2)
            {
                CameraArea_T pos;
                MINT32 X = *ptrPos++;
                MINT32 Y = *ptrPos++;

                pos.i4Left   = X - szW;
                pos.i4Top    = Y - szH;
                pos.i4Right  = X + szW;
                pos.i4Bottom = Y + szH;
                pos.i4Weight = 0;

                pos = _transformArea2Active(m_i4SensorOpenIdx, m_i4SensorMode, pos);
                cvtX = (pos.i4Left + pos.i4Right )/2;
                cvtY = (pos.i4Top  + pos.i4Bottom)/2;
                *ptrCvtPos++ = cvtX;
                *ptrCvtPos++ = cvtY;

                cvtW = pos.i4Right  -  pos.i4Left;
                cvtH = pos.i4Bottom -  pos.i4Top;
            }
            if( m_fgLogEn)
            {
                for( MUINT32 i=0; i<vecCvtPos.size(); i++)
                {
                    MY_LOGD_IF(m_fgLogEn, "Pos %d(%d)", m_rResult.vecFocusAreaPos[i], vecCvtPos[i]);
                }

                for( MUINT32 i=0; i<m_rResult.vecFocusAreaRes.size(); i++)
                {
                    MY_LOGD_IF(m_fgLogEn, "Res (%d)", m_rResult.vecFocusAreaRes[i]);
                }
                MY_LOGD_IF(m_fgLogEn, "W %d(%d), H %d(%d)", m_rResult.i4FocusAreaSz[0], cvtW, m_rResult.i4FocusAreaSz[1], cvtH);
            }
            MSize areaCvtSZ(cvtW, cvtH);
            UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FOCUS_AREA_SIZE, areaCvtSZ);
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_POSITION, vecCvtPos.array(), vecCvtPos.size());
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_RESULT, m_rResult.vecFocusAreaRes.array(), m_rResult.vecFocusAreaRes.size());
        }
        AAA_TRACE_END_L;

        RequestCbSet.vNumberSet.clear();
        RequestCbSet.vNumberSet.push_back(((MINT32)(u4MagicNum)));
    }

    {
        m_bRRZDump = ::property_get_int32("vendor.debug.rrz.dump", 0);
        if (m_bRRZDump == 1)
            m_u4FrmIdFreeze = u4MagicNum - 1;

        MBOOL bDump = ::property_get_int32("vendor.debug.tuning.dump_capture", 0);
        if ((m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) && bDump)
        {
            m_u4FrmIdFreeze = u4MagicNum;
            char filename[512];
            sprintf(filename, "/sdcard/debug/p1dbg_dump_capture-%04d.bin", m_rResult.i4FrmId);
            FILE* fp = fopen(filename, "wb");
            if (fp)
            {
                NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = *reinterpret_cast<NSIspExifDebug::IspExifDebugInfo_T*>(m_rResult.vecDbgIspInfo.editArray());
                ::fwrite(rDbgIspInfo.P1RegInfo.regData, sizeof(rDbgIspInfo.P1RegInfo.regData), 1, fp);
            }
            if (fp)
                fclose(fp);
        }
    }

    // hold the callback for stopping p1 register update if we want to dump p1 registers for debug.
    while ((m_u4FrmIdFreeze != 0) && (u4MagicNum > m_u4FrmIdFreeze))
    {
        usleep(10000);
        MY_LOGD("[%s] debug only: hold cb for dump p1 reg#(%d>%d)", __FUNCTION__, u4MagicNum, m_u4FrmIdFreeze);
    }

    if (m_pCbSet)
    {
        MINT32 i4CurId = m_p3AWrap->getCurrentHwId();
        MY_LOGD_IF(m_fgLogEn, "[%s] #(%d,%d), SOF(0x%x), m_rCapParam.i8ExposureTime(%ld), m_rCapParam.capType(%d), RequestCbSet.vNumberSet[0](%d), m_rResult.u1AfState(%d), m_rResult.u1AeState(%d), m_rResult.u1FlashState(%d), m_rResult.u4AeMode(%d), m_rResult.fgAeBvTrigger(%d)"
                    , __FUNCTION__, (MINT32)u4MagicNum, (MINT32)u4MagicNumCur, i4CurId, (long)m_rCapParam.i8ExposureTime, m_rCapParam.u4CapType, RequestCbSet.vNumberSet[0], m_rResult.u1AfState, m_rResult.u1AeState, m_rResult.u1FlashState, m_rResult.u1AeMode, m_rResult.fgAeBvTrigger);
        AAA_TRACE_D("3A_CB #(%d), SOF(%d)", u4MagicNum, i4CurId);
        AAA_TRACE_G(doNotifyCbFinish);
        AAA_TRACE_NL(doNotifyCbFinish);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_3APROC_FINISH,
            reinterpret_cast<MINTPTR> (&RequestCbSet),     // magic number
            i4CurId,        // SOF idx
            reinterpret_cast<MINTPTR> (&m_rCapParam));
        AAA_TRACE_END_NL;
        AAA_TRACE_END_G;

//#warning "FIXME"
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
Hal3AFlowCtrl::
updateResult(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    // pass1 result
    Result_T rResultNew;
    AAA_TRACE_L(getCurrResult);
    AAA_TRACE_NS(getCurrResult);
    m_p3AWrap->getCurrResult(u4MagicNum, rResultNew);
    AAA_TRACE_END_NS;
    AAA_TRACE_END_L;

    AAA_TRACE_L(updateRepeatQ);
    IHal3AResultBuf::getInstance(m_i4SensorDev)->updateRepeatQueue(u4MagicNum);
    AAA_TRACE_END_L;

    AAA_TRACE_L(updateResultBuf);
    if (!rResultNew.fgKeep)
    {
        m_rResult = rResultNew;
        IHal3AResultBuf::getInstance(m_i4SensorDev)->updateResult(u4MagicNum, rResultNew);
    }
    else
    {
        MY_LOGD_IF(m_fgLogEn, "[%s] #(%d) keep result", __FUNCTION__, u4MagicNum);
        m_rResult.i4FrmId = u4MagicNum;
        IHal3AResultBuf::getInstance(m_i4SensorDev)->updateResult(u4MagicNum, m_rResult);
    }
    AAA_TRACE_END_L;

    AAA_TRACE_L(setAFEGain2AF);
    m_p3AWrap->setIspSensorInfo2AF((MINT32)u4MagicNum, m_rResult);
    AAA_TRACE_END_L;

    // reset flashlight status after capture-end querying debug info
    AAA_TRACE_L(send3ACtrl);
    if (rResultNew.fgKeep == MTRUE)
        m_p3AWrap->send3ACtrl(E3ACtrl_SetIsFlashOnCapture, MFALSE, NULL);
    AAA_TRACE_END_L;

    // result for high quality
    Mutex::Autolock autoLock(m_rResultMtx);
    rResultNew.i4FrmId = u4MagicNumCur;

    AAA_TRACE_L(updateResultBufCur);
    IHal3AResultBuf::getInstance(m_i4SensorDev)->updateResultCur(u4MagicNumCur, rResultNew);
    AAA_TRACE_END_L;

    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNumCur(%d) HQC", __FUNCTION__, u4MagicNumCur);
    m_rResultCond.broadcast();
}

MINT32
Hal3AFlowCtrl::
getResult(MUINT32 i4FrmId, Result_T& rResult)
{
    MINT32 i4Ret =  IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(i4FrmId, rResult);
    if (-1 == i4Ret)
    {
        MY_LOGD("Fail get Result(%d)", i4FrmId);
        //m_p3AWrap->getCurrResult(i4FrmId, rResult);
    }

    if (((MINT32)m_u4MainFlashOnIdx != -1) &&
        ((i4FrmId == m_u4MainFlashOnIdx - 2) ||
         (i4FrmId == m_u4MainFlashOnIdx - 1)))
    {
        rResult.rCamInfo.fgBlackMute = MTRUE;
    }
    return i4Ret;
}

MINT32
Hal3AFlowCtrl::
getResultCur(MUINT32 i4FrmId, Result_T& rResult)
{
    Mutex::Autolock autoLock(m_rResultMtx);

    MINT32 i4ResultWaitCnt = 3;
    MINT32 i4Ret =  IHal3AResultBuf::getInstance(m_i4SensorDev)->getResultCur(i4FrmId, rResult);

    if (-1 == i4Ret)
    {
        while (i4ResultWaitCnt)
        {
           MY_LOGD_IF(m_fgLogEn, "[%s] wait result #(%d) i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
           m_rResultCond.waitRelative(m_rResultMtx, (long long int)500000000);
           MY_LOGD_IF(m_fgLogEn, "[%s] wait result done #(%d), i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
           i4ResultWaitCnt--;
           i4Ret = IHal3AResultBuf::getInstance(m_i4SensorDev)->getResultCur(i4FrmId, rResult);
           if (i4Ret != -1)
               break;
        }
#if 1
        if (-1 == i4Ret)
        {
            CAM_LOGE("Fail get Result(%d), try getting last", i4FrmId);
            i4Ret = IHal3AResultBuf::getInstance(m_i4SensorDev)->getLastCur(rResult);
            if (-1 == i4Ret)
            {
                CAM_LOGE("Fail get last");
                m_p3AWrap->getCurrResult(i4FrmId, rResult);
            }
            else
            {
                MY_LOGD("[%s] (%d, %d)", __FUNCTION__, i4FrmId, rResult.i4FrmId);
            }
        }
#else
        i4Ret = m_rResultBufCur.getLast(rResult);
        if (-1 == i4Ret)
        {
            CAM_LOGE("Fail get last");
            m_p3AWrap->getCurrResult(i4FrmId, rResult);
        }
        else
        {
            MY_LOGD("[%s] (%d, %d)", __FUNCTION__, i4FrmId, rResult.i4FrmId);
        }
#endif
        // std exif should be use capture start
        Result_T rResultAtStart;
        if (-1 != IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(i4FrmId, rResultAtStart))
        {
            rResult.vecExifInfo = rResultAtStart.vecExifInfo;
            rResult.rCamInfo.rFlashInfo.isFlash = rResultAtStart.rCamInfo.rFlashInfo.isFlash;

            MBOOL isStartEmpty = rResultAtStart.vecDbg3AInfo.empty();
            MBOOL isCurEmpty = rResult.vecDbg3AInfo.empty();
            if(!isStartEmpty && !isCurEmpty)
            {
                // get capture start AE setting to update EXIF info
                AAA_DEBUG_INFO1_T& rDbg3AInfoStart = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rResultAtStart.vecDbg3AInfo.editArray());
                AAA_DEBUG_INFO1_T& rDbg3AInfoCur = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rResult.vecDbg3AInfo.editArray());

                MUINT32 u4CapFlare = 0;
                MUINT32 u4CapFlareGain = 0;
                MUINT32 u4CapFlareIdx = 0;
                MUINT32 u4CapFlareGainIdx = 0;
                for(MINT32 i = 0; i < AE_DEBUG_TAG_SIZE; i++)
                {
                    // get Pass2 Flare value
                    if(rDbg3AInfoCur.rAEDebugInfo.Tag[i].u4FieldID == AE_TAG_CAP_FLARE)
                        u4CapFlare = rDbg3AInfoCur.rAEDebugInfo.Tag[i].u4FieldValue;
                    if(rDbg3AInfoCur.rAEDebugInfo.Tag[i].u4FieldID == AE_TAG_CAP_FLARE_GAIN)
                        u4CapFlareGain = rDbg3AInfoCur.rAEDebugInfo.Tag[i].u4FieldValue;
                    // get Pass1 Flare index
                    if(rDbg3AInfoStart.rAEDebugInfo.Tag[i].u4FieldID == AE_TAG_CAP_FLARE)
                        u4CapFlareIdx = i;
                    if(rDbg3AInfoStart.rAEDebugInfo.Tag[i].u4FieldID == AE_TAG_CAP_FLARE_GAIN)
                        u4CapFlareGainIdx = i;
                    if(u4CapFlare != 0 && u4CapFlareGain != 0 && u4CapFlareIdx != 0 && u4CapFlareGainIdx != 0)
                        break;
                }
                // update AE setting
                rDbg3AInfoCur.rAEDebugInfo = rDbg3AInfoStart.rAEDebugInfo;
                if(u4CapFlare != 0 && u4CapFlareGain != 0)
                {
                    rDbg3AInfoCur.rAEDebugInfo.Tag[u4CapFlareIdx].u4FieldValue = u4CapFlare;
                    rDbg3AInfoCur.rAEDebugInfo.Tag[u4CapFlareGainIdx].u4FieldValue = u4CapFlareGain;
                }
            }
            else
            {
                CAM_LOGE("isStartEmpty(%d) isCurEmpty(%d)", isStartEmpty, isCurEmpty);
            }

            MBOOL isIspStartEmpty = rResultAtStart.vecDbgIspInfo.empty();
            MBOOL isIspCurEmpty = rResult.vecDbgIspInfo.empty();

            if(!isIspStartEmpty && !isIspCurEmpty)
            {
                // get capture start AE setting to update EXIF info
                AAA_DEBUG_INFO2_T& rDbgISPInfoStart = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rResultAtStart.vecDbgIspInfo.editArray());
                AAA_DEBUG_INFO2_T& rDbgISPInfoCur = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rResult.vecDbgIspInfo.editArray());
                rDbgISPInfoCur.rISPDebugInfo = rDbgISPInfoStart.rISPDebugInfo;
            } else
            {
                CAM_LOGE("isIspStartEmpty(%d) isIspCurEmpty(%d)", isIspStartEmpty, isIspCurEmpty);
            }
            rResult.rCamInfo.rLCS_Info.rOutSetting = rResultAtStart.rCamInfo.rLCS_Info.rOutSetting;
            rResult.rCamInfo.bBypassLCE = rResultAtStart.rCamInfo.bBypassLCE;
        }
        else
        {
            CAM_LOGE("Fail get pResultAtStart #(%d)", i4FrmId);
        }
    }
    else
    {
        MY_LOGD_IF(m_fgLogEn, "[%s] got result #(%d)", __FUNCTION__, i4FrmId);
    }
    return i4Ret;
}

MVOID
Hal3AFlowCtrl::
notifyP1Done(MUINT32 /*u4MagicNum*/, MVOID* /*pvArg*/)
{
    MY_LOGD_IF(m_fgLogEn, "EMPTY HERE, that's totally fine");
    /*MBOOL bMainFlash = m_p3AWrap->chkMainFlashOnCond();
    MBOOL bPre = m_p3AWrap->chkPreFlashOnCond();
    MBOOL bMagicNum = ((MINT32)u4MagicNum >= (MINT32)m_u4FlashOnIdx);

    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d), m_u4FlashOnIdx(%d), bMagicNum(%d), bMainFlash(%d), bPre(%d), m_fgFlashOn(%d)", __FUNCTION__, u4MagicNum, m_u4FlashOnIdx, bMagicNum, bMainFlash, bPre, m_fgFlashOn);
    if ( bMagicNum && !bMainFlash && !bPre && m_fgFlashOn)
    {
        m_p3AWrap->setFlashLightOnOff(0, 1); // don't care main or pre
        m_u4FlashOnIdx = -1;
        m_fgFlashOn = 0;
    }

    if (bMagicNum && m_u4MainFlashOnIdx != -1)
    {
        m_u4MainFlashOnIdx = -1;
    }

    MBOOL isNeedTurnOnPreFlash = m_p3AWrap->isNeedTurnOnPreFlash();
    MY_LOGD_IF(m_fgLogEn,"[%s] isNeedTurnOnPreFlash(%d)", __FUNCTION__, isNeedTurnOnPreFlash);
    if(m_fgInternalFlashOn != isNeedTurnOnPreFlash)
    {
        m_p3AWrap->setPreFlashOnOff(isNeedTurnOnPreFlash);
        m_fgInternalFlashOn = isNeedTurnOnPreFlash;
        MY_LOG("[%s] setPreFlashOnOff(%d)", __FUNCTION__, m_fgInternalFlashOn);
    }

    if ((m_u4FrmIdFreeze != 0) && (u4MagicNum == m_u4FrmIdFreeze))
    {
        MY_LOGD("[%s]+ one-shot dump @%d", __FUNCTION__, u4MagicNum);
        m_p3AWrap->dumpP1Params(u4MagicNum);
        ::property_set("vendor.debug.rrz.dump", 0);
        m_u4FrmIdFreeze = 0;
        MY_LOGD("[%s]- one-shot dump @%d", __FUNCTION__, u4MagicNum);
    }*/
    return ;
}

MVOID
Hal3AFlowCtrl::
SttP1Done(MUINT32 u4MagicNum, MVOID* /*pvArg*/)
{
    MBOOL bMainFlash = m_p3AWrap->chkMainFlashOnCond();
    MBOOL bPre = m_p3AWrap->chkPreFlashOnCond();
    MBOOL bMagicNum = ((MINT32)u4MagicNum >= (MINT32)m_u4FlashOnIdx);

    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d), m_u4FlashOnIdx(%d), bMagicNum(%d), bMainFlash(%d), bPre(%d), m_fgFlashOn(%d)", __FUNCTION__, u4MagicNum, m_u4FlashOnIdx, bMagicNum, bMainFlash, bPre, m_fgFlashOn);
    if ( bMagicNum && !bMainFlash && !bPre && m_fgFlashOn)
    {
        m_p3AWrap->setFlashLightOnOff(0, 1); // don't care main or pre
        m_u4FlashOnIdx = -1;
        m_fgFlashOn = 0;
    }

    if (bMagicNum && (MINT32)m_u4MainFlashOnIdx != -1)
    {
        m_u4MainFlashOnIdx = -1;
    }

    MBOOL isNeedTurnOnPreFlash = m_p3AWrap->isNeedTurnOnPreFlash();
    MY_LOGD_IF(m_fgLogEn,"[%s] isNeedTurnOnPreFlash(%d)", __FUNCTION__, isNeedTurnOnPreFlash);
    if(m_fgInternalFlashOn != isNeedTurnOnPreFlash)
    {
        m_p3AWrap->setPreFlashOnOff(isNeedTurnOnPreFlash);
        m_fgInternalFlashOn = isNeedTurnOnPreFlash;
        MY_LOG("[%s] setPreFlashOnOff(%d)", __FUNCTION__, m_fgInternalFlashOn);
    }

    if ((m_u4FrmIdFreeze != 0) && (u4MagicNum == m_u4FrmIdFreeze))
    {
        MY_LOGD("[%s]+ one-shot dump @%d", __FUNCTION__, u4MagicNum);
        m_p3AWrap->dumpP1Params(u4MagicNum);
        ::property_set("vendor.debug.rrz.dump", 0);
        m_u4FrmIdFreeze = 0;
        MY_LOGD("[%s]- one-shot dump @%d", __FUNCTION__, u4MagicNum);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3AFlowCtrl::
attachCb(I3ACallBack* cb)
{
    MY_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->attachCb(cb);
    m_p3AWrap->attachCb(cb);
    return 0;
}

MINT32
Hal3AFlowCtrl::
detachCb(I3ACallBack* cb)
{
    MY_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = NULL;
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->detachCb(cb);
    m_p3AWrap->detachCb(cb);
    return 0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// passing to wrapper functions directly
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MBOOL
Hal3AFlowCtrl::isMeetMainFlashOnCondition()
{
    return
        (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))
        ||
        ( ((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isStrobeBVTrigger()))
           && (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) );
}

MBOOL
Hal3AFlowCtrl::isMeetPreFlashOnCondition()
{
    return
        (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH));
}


MBOOL
Hal3AFlowCtrl::isStrobeBVTrigger()
{
    CAM_LOGE("[%s] it should be overwritten", __FUNCTION__);
    return MFALSE;
}

MBOOL Hal3AFlowCtrl::setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/)
{
    CAM_LOGE("[%s] it should be overwritten", __FUNCTION__);
    return MTRUE;
}
#endif

MVOID
Hal3AFlowCtrl::
setSensorMode(MINT32 i4SensorMode)
{
    m_i4SensorMode = i4SensorMode;
    if(m_p3ASttCtrl)
    m_p3ASttCtrl->setSensorMode(i4SensorMode);
    m_p3AWrap->setSensorMode(i4SensorMode);
}


MVOID
Hal3AFlowCtrl::
resetParams()
{
    m_rParam.i4MagicNum = 0;
    m_rParam.i4MagicNumCur = 0;
    m_rParam.u1PrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    if (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD)
    {
        MY_LOGD("[%s] CAPTURE_INTENT protection during Video record mode",__FUNCTION__);
    }
    else
        m_rParam.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    //---------------------------------------------------------------
    m_rAFParam.u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    m_rAFParam.u1PrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    m_rAFParam.i4MagicNum = 0;


    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
}

MBOOL
Hal3AFlowCtrl::
setParams(Param_T const &rNewParam, MBOOL bUpdateScenario)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    m_rParam = rNewParam;
    return m_p3AWrap->setParams(rNewParam, bUpdateScenario);
}

MBOOL
Hal3AFlowCtrl::
setAfParams(AF_Param_T const &rNewParam)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    m_rAFParam = rNewParam;
    return m_p3AWrap->setAfParams(rNewParam);
}

VOID
Hal3AFlowCtrl::
queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    return m_p3AWrap->queryTgSize(i4TgWidth,i4TgHeight);
}

VOID
Hal3AFlowCtrl::
queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    return m_p3AWrap->queryHbinSize(i4HbinWidth,i4HbinHeight);
}



MBOOL
Hal3AFlowCtrl::
autoFocus()
{
    return m_p3AWrap->autoFocus();
#if 0
    MY_LOGD_IF(m_fgLogEn, "[%s()]\n", __FUNCTION__);

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        //ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFStart));
    }
    //IAfMgr::getInstance().autoFocus(m_i4SensorDev);
}
    return MTRUE;
#endif
}

MBOOL
Hal3AFlowCtrl::
cancelAutoFocus()
{
    return m_p3AWrap->cancelAutoFocus();
#if 0
    MY_LOGD_IF(m_fgLogEn, "[%s()]\n", __FUNCTION__);

    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        //ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFEnd));
    }
    //IAfMgr::getInstance().cancelAutoFocus(m_i4SensorDev);
    return MTRUE;
#endif
}

MVOID
Hal3AFlowCtrl::
setFDEnable(MBOOL fgEnable)
{
    m_p3AWrap->setFDEnable(fgEnable);
}

MBOOL
Hal3AFlowCtrl::
setFDInfo(MVOID* prFaces, MVOID* prAFFaces)
{
    return m_p3AWrap->setFDInfo(prFaces, prAFFaces);
}

MBOOL
Hal3AFlowCtrl::
setOTInfo(MVOID* prOT, MVOID* prAFOT)
{
    return m_p3AWrap->setOTInfo(prOT, prAFOT);
}


MBOOL
Hal3AFlowCtrl::
setZoom(MUINT32 /*u4ZoomRatio_x100*/, MUINT32 /*u4XOffset*/, MUINT32 /*u4YOffset*/, MUINT32 /*u4Width*/, MUINT32 /*u4Height*/)
{
    return MTRUE;
}

MINT32
Hal3AFlowCtrl::
getDelayFrame(EQueryType_T const /*eQueryType*/) const
{
    return 0;
}

MBOOL
Hal3AFlowCtrl::
setIspPass2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2)
{
    return m_p3AWrap->generateP2(flowType, rIspInfo, pRegBuf, pResultP2);
}

MBOOL
Hal3AFlowCtrl::
notifyPwrOn()
{
    //IAfMgr::getInstance().CamPwrOnState(m_i4SensorDev);
    m_p3AWrap->notifyPwrOn();
    MY_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
notifyPwrOff()
{
    //IAfMgr::getInstance().CamPwrOffState(m_i4SensorDev);
    m_u4FlashOnIdx = -1;
    m_fgFlashOn = 0;
    m_p3AWrap->setFlashLightOnOff(0, 0);
    m_p3AWrap->notifyPwrOff();
    MY_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
notifyP1PwrOn()
{
    MY_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    return m_p3AWrap->notifyP1PwrOn();
}

MBOOL
Hal3AFlowCtrl::
notifyP1PwrOff()
{
    MY_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
    return m_p3AWrap->notifyP1PwrOff();
}


MBOOL
Hal3AFlowCtrl::
checkCapFlash() const
{
    return m_p3AWrap->chkCapFlash();
}

MINT32
Hal3AFlowCtrl::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2)
{
    return m_p3AWrap->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
}

MBOOL
Hal3AFlowCtrl::
queryRepeatQueue(MINT32 maginNum)
{
    return IHal3AResultBuf::getInstance(m_i4SensorDev)->queryRepeatQueue(maginNum);
}

MBOOL
Hal3AFlowCtrl::
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
   m_p3AWrap->setP2Params(rNewP2Param, pResultP2);
   return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
   m_p3AWrap->getP2Result(rNewP2Param, pResultP2);
   return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
notifyResult4TG(MINT32 i4PreFrmId)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] Previous FrmId(%d) +", __FUNCTION__, i4PreFrmId);

    Result_T *rPreResult = NULL;//Previous Result

    IHal3AResultBuf::getInstance(m_i4SensorDev)->lockBuf();
    rPreResult = IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(i4PreFrmId);

    if(rPreResult == NULL)
    {
        MY_LOGE("[%s] Get previous result fail", __FUNCTION__);
        IHal3AResultBuf::getInstance(m_i4SensorDev)->unlockBuf();
        return MFALSE;
    }
    else
    {
        MY_LOGD_IF(m_fgLogEn, "[%s] rPreResult Addr(%p)", __FUNCTION__, rPreResult);
        m_p3AWrap->notifyResult4TG(i4PreFrmId, rPreResult);
    }

    IHal3AResultBuf::getInstance(m_i4SensorDev)->unlockBuf();

    MY_LOGD_IF(m_fgLogEn, "[%s] Previous FrmId(%d) -", __FUNCTION__, i4PreFrmId);
    return MTRUE;
}

void
Hal3AFlowCtrl::
doNotifyCb(MINT32 /*_msgType*/, MINTPTR /*_ext1*/, MINTPTR /*_ext2*/, MINTPTR /*_ext3*/)
{
    MUINT32 u4PreFrmId = m_u4PreMagicReq;

    MY_LOGD_IF(m_fgLogEn, "[%s] CCU Previous FrmId(%d) +", __FUNCTION__, u4PreFrmId);

    if(u4PreFrmId != 0)
    {
        Result_T *rPreResult = NULL;//Previous Result

        IHal3AResultBuf::getInstance(m_i4SensorDev)->lockBuf();
        rPreResult = IHal3AResultBuf::getInstance(m_i4SensorDev)->getResult(u4PreFrmId);

        if(rPreResult == NULL)
        {
            MY_LOGE("[%s] CCU Get previous result fail", __FUNCTION__);
        }
        else
        {
            MY_LOGD_IF(m_fgLogEn, "[%s] rPreResult Addr(%p)", __FUNCTION__, rPreResult);
            m_p3AWrap->notify4CCU(u4PreFrmId, rPreResult);
        }

        IHal3AResultBuf::getInstance(m_i4SensorDev)->unlockBuf();
    }

    MY_LOGD_IF(m_fgLogEn, "[%s] CCU Previous FrmId(%d) -", __FUNCTION__, u4PreFrmId);

}

MVOID*
Hal3AFlowCtrl::
ThreadConfigSttpipe(MVOID* arg)
{
    ::prctl(PR_SET_NAME, "ThreadConfigSttpipe", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = NICE_CAMERA_CONFIG_STTPIPE;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    Hal3AFlowCtrl *_this = static_cast<Hal3AFlowCtrl*>(arg);


    AAA_TRACE_D("Config STT Pipe");
    if(_this->m_p3ASttCtrl)
        _this->m_p3ASttCtrl->initStt(_this->m_i4SensorDev, _this->m_i4SensorOpenIdx, _this->m_rConfigInfo);
    AAA_TRACE_END_D;


    ::pthread_exit((MVOID*)0);
}

