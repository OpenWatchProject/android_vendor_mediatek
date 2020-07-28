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
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <debug/DebugUtil.h>
#include <vector>
#include <array>
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

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

#include <aaa_common_custom.h>

#include "private/PDTblGen.h"

#define AAO_READOUT_COUNT 1


using namespace NS3Av3;
using namespace NSCamHW;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MY_INST NS3Av3::INST_T<Hal3AFlowCtrl>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

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
        CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_FLOW, fmt, ##arg); \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if ( (cond) ){ CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_FLOW, __VA_ARGS__); } \
    }while(0)

#define MY_LOGW(fmt, arg...) \
    do { \
        CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK_FLOW, fmt, ##arg); \
    }while(0)

#define MY_LOGE(fmt, arg...) \
    do { \
        CAM_ULOGE(Utils::ULog::MOD_3A_FRAMEWORK_FLOW, fmt, ##arg); \
    }while(0)

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

    MY_LOGD_IF(fgLog, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
              input.p.x, input.p.y, input.s.w, input.s.h,
              output.p.x, output.p.y, output.s.w, output.s.h);
    MY_LOGD_IF(fgLog, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
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
getInstance(MINT32 i4SensorOpenIndex)
{
    if(i4SensorOpenIndex >= SENSOR_IDX_MAX || i4SensorOpenIndex < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorOpenIndex);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3AFlowCtrl>(i4SensorOpenIndex);
    } );
    (rSingleton.instance)->init(i4SensorOpenIndex);

    return rSingleton.instance.get();
}

MVOID
Hal3AFlowCtrl::
destroyInstance()
{
    uninit();
}

Hal3AFlowCtrl::
Hal3AFlowCtrl(MINT32 i4SensorIdx)
    : Hal3AIf()
    , m_fgLogEn(MFALSE)
    , m_fgDebugLogWEn(MFALSE)
    , m_i4SensorDev(0)
    , m_i4SensorOpenIdx(i4SensorIdx)
    , m_u4FrmIdStat(0)
    , m_u4FrmIdFreeze(0)
    , m_u4FlashOnIdx(-1)
    , m_fgFlashOn(0)
    , m_fgInternalFlashOn(0)
    , m_u4MainFlashOnIdx(-1)
    , m_p3AWrap(NULL)
    , m_pThread(NULL)
    , m_pEventIrq(NULL)
    , m_p3ASttCtrl(NULL)
    , m_pCbSet(NULL)
    , m_fgPreStop(MFALSE)
    , m_bCallBack(MFALSE)
    , m_bRRZDump(0)
    , m_u4PreMagicReq(0)
    , m_ThreadStt()
    , m_SttBufQEnable(0)
    , m_pResultPoolObj(NULL)
    , m_i4SensorMode(0)
    , m_bAAOIsReady(0)
    , m_u4AAOReadOutCount(0)
{
    m_i4SensorDev = mapSensorIdxToDev(i4SensorIdx);
    MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, i4SensorIdx);
}

Hal3AFlowCtrl::
~Hal3AFlowCtrl()
{}

MRESULT
Hal3AFlowCtrl::
init(MINT32 i4SensorOpenIndex) //can be called only once by RAW or YUV, no user count
{
    MY_LOGD("[%s] + sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, i4SensorOpenIndex);

    m_fgLogEn = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( m_fgLogEn == 0 ) {
        m_fgLogEn = ::property_get_int32("vendor.debug.camera.log.hal3a", 0);
    }
    m_u4FrmIdFreeze = ::property_get_int32("vendor.debug.3a.freeze", 0);
    m_bRRZDump = 0;
    m_fgDebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

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

    m_i4SensorOpenIdx = i4SensorOpenIndex;

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

    // ResultPool - Get result pool object
    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        CAM_LOGE("ResultPool getInstance fail");

    // create 3A wrapper
    if (m_p3AWrap == NULL)
    {
        m_p3AWrap = I3AWrapper::getInstance(eType, i4SensorOpenIndex);
        if (m_p3AWrap)
        {
            MY_LOGD_IF(m_fgLogEn, "[%s] m_p3AWrapper(%p) created OK", __FUNCTION__, m_p3AWrap);
        }
        else
        {
            MY_LOGE("m_p3AWrapper created fail!");
            AEE_ASSERT_3A_HAL("m_p3AWrapper created fail!");
        }
    }

    m_SttBufQEnable = property_get_int32("vendor.debug.camera.SttBufQ.enable", 0);
    if (m_SttBufQEnable) {
        ISttBufQ::createSingleton(m_i4SensorDev, LOG_TAG);
    }

    // create statistic control
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if (m_p3ASttCtrl == NULL  && eType == I3AWrapper::E_TYPE_RAW)
    {
        m_p3ASttCtrl = Hal3ASttCtrl::createInstance(m_i4SensorDev, i4SensorOpenIndex);
        if (m_p3ASttCtrl)
        {
            MY_LOGD_IF(m_fgLogEn, "[%s] m_p3ASttCtrl(%p) created OK", __FUNCTION__, m_p3ASttCtrl);
        }
        else
        {
            MY_LOGE("m_p3ASttCtrl created fail!");
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
            MY_LOGE("m_pThread created fail!");
            AEE_ASSERT_3A_HAL("m_pThread created fail!");
        }
    }

    // ResultPool - Get result pool object
    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        MY_LOGE("ResultPool getInstance fail");

    MY_LOGD("[%s] - eType(%d), sensor(%d), sensorIdx(%d)", __FUNCTION__, eType, m_i4SensorDev, i4SensorOpenIndex);
    return S_3A_OK;
}

MRESULT
Hal3AFlowCtrl::
uninit() //can be called only once by RAW or YUV, no user count
{
    MY_LOGD("[%s] + sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

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
        MY_LOGE("User did not detach callbacks!");
    }

    MY_LOGD("[%s] - sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3AFlowCtrl::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD_IF(m_fgLogEn, "[%s]+ sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);

    // Must start PDTblGen module before getPdInfoForSttCtrl is executed.
    ::IPDTblGen::getInstance()->start(m_i4SensorDev, m_i4SensorOpenIdx);

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

    ::pthread_join(m_ThreadStt, &ThreadSttret);
    if (ThreadSttret)    MY_LOGD("Thread Config Sttpipe create failed");

    MY_LOGD_IF(m_fgLogEn, "[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
    return (bRetConfig);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Stop Stt
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
stopStt()
{
    MY_LOGD_IF(m_fgLogEn, "[%s]+ sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
    if(m_p3AWrap)
        m_p3AWrap->notifyPreStop();
    if(m_p3ASttCtrl)
    m_p3ASttCtrl->preStopStt();
    resume();
    m_fgPreStop = MTRUE;
    m_pThread->enableListenUpdate(MFALSE);
    MY_LOGD_IF(m_fgLogEn, "[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pause
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
pause()
{
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->pause();
#endif
    if(m_p3AWrap)
        m_p3AWrap->pause();
    if(m_pThread)
        m_pThread->pause();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Resume
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
resume(MINT32 MagicNum)
{
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
        m_p3ASttCtrl->resume();
#endif
    if(m_p3AWrap)
        m_p3AWrap->resume(MagicNum);
    if(m_pThread)
        m_pThread->resume();
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
    MY_LOGD("[%s]sensorDev(%d), sensorIdx(%d) abortDeque done", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
    if(m_pEventIrq)
        m_pEventIrq->flush();
    MY_LOGD("[%s]sensorDev(%d), sensorIdx(%d) abortDeque done", __FUNCTION__, m_i4SensorDev, m_i4SensorOpenIdx);
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

    AAA_TRACE_D("3A_Update #(%d)", i4MagicNum);
    MBOOL fgRet = postCommand(ECmd_Update, pParam);
    AAA_TRACE_END_D;

    if (!fgRet)
    {
        MY_LOGE("ECmd_Update failed");
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
    AAA_TRACE_DRV(WaitVSync);
    m_pEventIrq->wait(duration);
    AAA_TRACE_END_DRV;
    AAA_TRACE_END_D;

    if(cust_getIsSpecialLongExpOn())
    {
        MY_LOGD("[%s] start waitVSirq. (Special Long Exp)", __FUNCTION__);
        m_pEventIrq->wait(duration);
    }

    if (m_u4AAOReadOutCount != AAO_READOUT_COUNT)
    {
        m_u4AAOReadOutCount++;
        m_bAAOIsReady = MFALSE;
    }
    if (m_u4AAOReadOutCount == AAO_READOUT_COUNT)
    {
        m_bAAOIsReady = MTRUE;
    }

    MY_LOGD_IF(m_fgLogEn, "[%s] m_u4AAOReadOutCount(%u) m_bAAOIsReady(%d).", __FUNCTION__, m_u4AAOReadOutCount, m_bAAOIsReady);

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
    MINT32 i4MagicNum = 0;
    MINT32 i4MagicNumCur = 0;
    MBOOL bMainFlash = MFALSE;
    MBOOL bPre = MFALSE;
    ParamIspProfile_T rParamIspProf = *pParam;
    m_rCapParam.u4CapType = E_CAPTURE_NORMAL;
    if (eCmd == ECmd_CameraPreviewStart)
    {
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
        MY_LOGD("m_rParam.u4AeMode(%d), m_rParam.i8ExposureTime(%ld)", (MINT32)m_rParam.u4AeMode, m_rParam.i8ExposureTime);
        if (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
        {
            i4MagicNum = rParamIspProf.i4MagicNum;
            i4MagicNumCur = rParamIspProf.i4MagicNumCur;
            bMainFlash = m_p3AWrap->chkMainFlashOnCond();
            bPre = m_p3AWrap->chkPreFlashOnCond();
            rParamIspProf.bMainFlash = bMainFlash;
            MY_LOGD_IF(m_fgLogEn, "[%s] (bMainFlash,bPre):(%d,%d)", __FUNCTION__, bMainFlash,bPre);

            if ((bMainFlash||bPre) && i4MagicNum != 0)
            {
                m_u4FlashOnIdx = i4MagicNum;
                m_fgFlashOn = 1;
            }
            if (bMainFlash)
            {
                m_p3AWrap->setFlashLightOnOff(1, 1);
                m_u4MainFlashOnIdx = i4MagicNum;
                m_fgInternalFlashOn = MFALSE;
            }
            if (bPre) {
                m_p3AWrap->setFlashLightOnOff(1, 0);
                m_fgInternalFlashOn = MFALSE;
            }
            m_p3AWrap->postCommand(eCmd, &rParamIspProf);

            if (m_p3AWrap->chkCapFlash())
            {
                m_rCapParam.u4CapType = E_CAPTURE_HIGH_QUALITY_CAPTURE;
            }
            MY_LOGD_IF(m_fgLogEn, "[%s][HQC] inform p1 capture type (%d)", __FUNCTION__, m_rCapParam.u4CapType);

            i4MagicNumCur = m_p3AWrap->queryMagicNumber();

            MY_LOGD_IF(m_fgLogEn, "[%s] (Req, Stt) = (#%d, #%d)", __FUNCTION__, i4MagicNum, i4MagicNumCur);

            // ResultPool - must update u4MagicNumCur to resultPool i4StatiticMagic
            RESULT_CFG_T rResultCfg = m_pResultPoolObj->getResultCFG(i4MagicNum);
            rResultCfg.i4StatisticMagic = i4MagicNumCur;
            MBOOL ret = m_pResultPoolObj->updateResultCFG(rResultCfg); //update ReqResult i4StatiticMagic of CFG
            if(!ret)
                MY_LOGE("[%s] update MagicNumCur to resultPool i4StatiticMagic fail", __FUNCTION__);
            // i4StatiticMagic for high quality
            AAA_TRACE_HAL(on3AProcFinish);
            on3AProcFinish(i4MagicNum, i4MagicNumCur);
            AAA_TRACE_END_HAL;
        }
        else
        {
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
        // ResultPool - preview start to clear ResultPool
        m_pResultPoolObj->clearAllResultPool();
        m_u4FlashOnIdx = -1;
        m_u4AAOReadOutCount = 0;
        m_bAAOIsReady = MFALSE;
        MY_LOGD("[%s] reset bMainFlash m_u4AAOReadOutCount(%u) m_bAAOIsReady(%d)", __FUNCTION__, m_u4AAOReadOutCount, m_bAAOIsReady);

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
        AAA_TRACE_MW(doNotifyCbSet);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_VSYNC_DONE,
            0,     // magic number
            0,        // SOF idx
            0);
        AAA_TRACE_END_MW;
    }
}

MVOID
Hal3AFlowCtrl::
on3AProcFinish(MINT32 i4MagicNum, MINT32 i4MagicNumCur)
{
    RequestSet_T RequestCbSet;

    AAA_TRACE_D("updateResult#(%d,%d)", i4MagicNum, i4MagicNumCur);
    updateResult(i4MagicNum, i4MagicNumCur);
    AAA_TRACE_END_D;

    MINT32 i4AeShutDelayFrame;
    MINT32 i4AeISPGainDelayFrame;

    m_u4PreMagicReq = i4MagicNum;

    send3ACtrl(E3ACtrl_GetSensorDelayInfo, reinterpret_cast<MINTPTR>(&i4AeShutDelayFrame), reinterpret_cast<MINTPTR>(&i4AeISPGainDelayFrame));

    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d), i4AeShutDelayFrame(%d), i4AeISPGainDelayFrame(%d)", __FUNCTION__, i4MagicNum, i4AeShutDelayFrame, i4AeISPGainDelayFrame);

    //Shutter delay is 3 when sensor type is YUV. 3A will call back PreMagicum is (u4MagicNum-1)
    MINT32 i4PreMagicNum = i4MagicNum - ( (i4AeISPGainDelayFrame - i4AeShutDelayFrame) - 2);

    if(i4PreMagicNum != i4MagicNum)
    {
        updateCapParam(i4PreMagicNum);
        RequestCbSet.vNumberSet.clear();
        RequestCbSet.vNumberSet.push_back(i4PreMagicNum);
    }
    else
    {
        updateCapParam(i4MagicNum);
        RequestCbSet.vNumberSet.clear();
        RequestCbSet.vNumberSet.push_back(i4MagicNum);
    }

    if (m_pCbSet)
    {
        MINT32 i4CurId = m_p3AWrap->getCurrentHwId();
        MY_LOGD_IF(m_fgLogEn, "[%s] #(%d,%d), SOF(0x%x), m_rCapParam.i8ExposureTime(%lld), m_rCapParam.capType(%d), RequestCbSet.vNumberSet[0](%d)"
            , __FUNCTION__, (MINT32)i4MagicNum, i4MagicNumCur, i4CurId, (long long)m_rCapParam.i8ExposureTime, m_rCapParam.u4CapType, RequestCbSet.vNumberSet[0]);

        AAA_TRACE_D("3A_CB #(%d), SOF(%d)", i4MagicNum, i4CurId);
        AAA_TRACE_MW(doNotifyCbFinish);
        m_pCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_3APROC_FINISH,
            reinterpret_cast<MINTPTR> (&RequestCbSet),     // magic number
            i4CurId,        // SOF idx
            reinterpret_cast<MINTPTR> (&m_rCapParam));
        AAA_TRACE_END_MW;
        AAA_TRACE_END_D;
    }
}

MVOID
Hal3AFlowCtrl::
updateResult(MINT32 i4MagicNum, MINT32 i4MagicNumCur)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] Req(#%d)", __FUNCTION__, i4MagicNum);

    std::lock_guard<std::mutex> autoLock(m_rResultMtx);
    MBOOL bRet = MFALSE;
    const HALResultToMeta_T *pHALResult = NULL;
    AAA_TRACE_D("getCurrResult");
    // ResultPool - Get MGR result to update resultPool with i4MagicNum
    bRet = m_p3AWrap->getCurrResult(i4MagicNum);
    if(!bRet)
        MY_LOGE("[%s] getCurrResult fail!", __FUNCTION__);
    AAA_TRACE_END_D;


    AAA_TRACE_HAL(setAFEGain2AF);
    m_p3AWrap->setIspSensorInfo2AF(i4MagicNum);
    AAA_TRACE_END_HAL;

    // reset flashlight status after capture-end querying debug info

    AAA_TRACE_HAL(send3ACtrl);
    pHALResult = (HALResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicNum, E_HAL_RESULTTOMETA, __FUNCTION__);
    if(pHALResult)
    {
        if (pHALResult->fgKeep == MTRUE)
            m_p3AWrap->send3ACtrl(E3ACtrl_SetIsFlashOnCapture, MFALSE, NULL);
    }
    else
        MY_LOGE("[%s] pHALResult is NULL", __FUNCTION__);
    AAA_TRACE_END_HAL;


    m_rResultCond.notify_all();
    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNumCur(%d) HQC", __FUNCTION__, i4MagicNumCur);
}

MVOID
Hal3AFlowCtrl::
updateCapParam(MINT32 i4MagicNum)
{
    AllResult_T      *pAllResult = NULL;
    AEResultToMeta_T *pAEResult = NULL;
    AFResultToMeta_T *pAFResult = NULL;
    EARLY_CALL_BACK  *pEarlyCB = NULL;

    // get AE result
    pAEResult = (AEResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicNum, E_AE_RESULTTOMETA, __FUNCTION__);
    if(pAEResult)
        m_rCapParam.i8ExposureTime = pAEResult->u8P1Exposuretime_ns;
    else
        MY_LOGW("[%s] ReqMagic(#%d) pAEResult is NULL %p", __FUNCTION__, i4MagicNum, pAEResult);

    // get AF result
    pAllResult = m_pResultPoolObj->getAllResult(i4MagicNum);
    pAFResult = (AFResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicNum, E_AF_RESULTTOMETA, __FUNCTION__);
    if(!pAFResult)
    {
        MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
        MINT32 i4UseMagic = 0;
        m_pResultPoolObj->getHistory(rHistoryReqMagic);
        for(MINT32 i = 0; i < HistorySize; i++)
        {
            if(i4MagicNum != rHistoryReqMagic[i] && i4MagicNum != 0)
                i4UseMagic = rHistoryReqMagic[i];
        }
        MY_LOGD_IF(m_fgLogEn, "[%s] ReqMagic(#%d, #%d) History(#%d, #%d, #%d)", __FUNCTION__, i4MagicNum, i4UseMagic, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);
        pAFResult = (AFResultToMeta_T*)m_pResultPoolObj->getResult(i4UseMagic, E_AF_RESULTTOMETA, __FUNCTION__);
        pAllResult = m_pResultPoolObj->getAllResult(i4UseMagic);
    }

    // ResultPool - get Pre Early CallBack, AFState is Pre AFState
    pEarlyCB = m_pResultPoolObj->getEarlyCB(i4MagicNum);

    // Nelson Coverity : Print rEarlyCB->u1AfState after check if it's available
    if (pEarlyCB) {
        // MW need to AFState
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AF_STATE, pEarlyCB->u1AfState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_STATE, pEarlyCB->u1AeState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FLASH_STATE, pEarlyCB->u1FlashState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_MODE, pEarlyCB->u1AeMode);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_3A_AE_BV_TRIGGER, pEarlyCB->fgAeBvTrigger);
#if CAM3_STEREO_FEATURE_EN
        MINT32 StereoData[2];
        StereoData[0] = pEarlyCB->i4MasterIdx;
        StereoData[1] = pEarlyCB->i4SlaveIdx;
        UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_STEREO_SYNC2A_MASTER_SLAVE, StereoData, 2);
#endif
        MY_LOGD_IF(m_fgLogEn, "[%s] u1AfState(%d), u1AeState(%d), u1FlashState(%d), u4AeMode(%d), fgAeBvTrigger(%d), MasterIdx(%d) SlaveIdx(%d)", __FUNCTION__, pEarlyCB->u1AfState, pEarlyCB->u1AeState, pEarlyCB->u1FlashState, pEarlyCB->u1AeMode, pEarlyCB->fgAeBvTrigger, pEarlyCB->i4MasterIdx, pEarlyCB->i4SlaveIdx);
    }
    else
        MY_LOGW("[%s] ReqMagic(#%d) EarlyCB(%p)", __FUNCTION__, i4MagicNum, pEarlyCB);

    if(pAllResult && pAFResult)
    {
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
        if( pAllResult->vecFocusAreaPos.size()!=0)
        {
            MINT32  szW = 0;
            MINT32  szH = 0;
            if(pAFResult)
            {
                szW = pAFResult->i4FocusAreaSz[0];
                szH = pAFResult->i4FocusAreaSz[1];
            }
            const MINT32 *ptrPos = &(pAllResult->vecFocusAreaPos[0]);
            MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
            std::vector<MINT32> vecCvtPos = pAllResult->vecFocusAreaPos;
            MINT32 *ptrCvtPos = &vecCvtPos[0];
            szW = szW/2;
            szH = szH/2;
            for( MUINT32 i=0; i<pAllResult->vecFocusAreaPos.size(); i+=2)
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
                    MY_LOGD_IF(m_fgLogEn, "Pos %d(%d)", pAllResult->vecFocusAreaPos[i], vecCvtPos[i]);
                }

                for( MUINT32 i=0; i<pAllResult->vecFocusAreaRes.size(); i++)
                {
                    MY_LOGD_IF(m_fgLogEn, "Res (%d)", pAllResult->vecFocusAreaRes[i]);
                }
                if(pAFResult)
                    MY_LOGD_IF(m_fgLogEn, "W %d(%d), H %d(%d)", pAFResult->i4FocusAreaSz[0], cvtW, pAFResult->i4FocusAreaSz[1], cvtH);
            }
            MSize areaCvtSZ(cvtW, cvtH);
            UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FOCUS_AREA_SIZE, areaCvtSZ);
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_POSITION, &vecCvtPos[0], vecCvtPos.size());
            UPDATE_ENTRY_ARRAY(m_rCapParam.metadata, MTK_FOCUS_AREA_RESULT, &(pAllResult->vecFocusAreaRes[0]), pAllResult->vecFocusAreaRes.size());
        }
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_3A_AF_LENS_POSITION, pAFResult->u2LensPosition);
    }
    else
        MY_LOGD_IF(m_fgLogEn, "[%s] ReqMagic(#%d) pAllResult(%p) pAFResult(%p)", __FUNCTION__, i4MagicNum, pAllResult, pAFResult);
}

MINT32
Hal3AFlowCtrl::
getResult(MINT32 i4FrmId)
{
    return MFALSE;
}

MINT32
Hal3AFlowCtrl::
getResultCur(MINT32 i4FrmId)
{
    std::unique_lock<std::mutex> autoLock(m_rResultMtx);
    MINT32 i4ResultWaitCnt = 3;

    MINT32 i4Ret = 1;

    // ResultPool - get Current All Result with SttMagic
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResultCur(i4FrmId);

    if(NULL == pAllResult)
    {
        // ResultPool - Wait to get Current All Result with SttMagic
        while (i4ResultWaitCnt)
        {
           MY_LOGD("[%s] wait result #(%d) i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
           m_rResultCond.wait_for(autoLock, std::chrono::nanoseconds(500000000));
           MY_LOGD("[%s] wait result done #(%d), i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
           i4ResultWaitCnt--;

           pAllResult = m_pResultPoolObj->getAllResultCur(i4FrmId);

           if (pAllResult != NULL)
               break;
        }

        // ResultPool - CaptureStart ReqMagic is 2 3 4 5.., always update Result. If fail to get Current All Result, get last current All Result with ReqMagic 4.
        if(NULL == pAllResult)
        {
            // ResultPool - Get History
            MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
            m_pResultPoolObj->getHistory(rHistoryReqMagic);

            MY_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d), Fail to get ResultCur with ReqMagic(%d), try getting ResultLastCur(%d)", __FUNCTION__,
                        rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2], i4FrmId, rHistoryReqMagic[2]);
            i4Ret = -1;

            pAllResult = m_pResultPoolObj->getAllResultLastCur(rHistoryReqMagic[2]);//get req/stt:4/1

            // ResultPool - If fail to get last current All Result with last ReqMagic 4, get All Result with ReqMagic 2.
            if(pAllResult == NULL)
            {
                MY_LOGW("Fail to get ResultLastCur, then get Result with ReqMagic(%d)", i4FrmId);
                pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);//get req/stt:2/xx
                i4Ret = -2;
                if(pAllResult)
                    MY_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);
                else
                    MY_LOGE("[%s] Ret(%d) pAllResult is NULL", __FUNCTION__, i4Ret);
            }
            else
                MY_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);
        }

        // ResultPool - std exif should be use capture start
        AllResult_T *pAllResultAtStart = m_pResultPoolObj->getAllResult(i4FrmId);

        if(NULL != pAllResultAtStart && pAllResult != NULL)
        {
            // protect vector before use vector
            std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);

            MY_LOGD("[%s] (Req, ResultAtStartReq, ResultReq, Stt) = (#%d, #%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResultAtStart->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);
            pAllResult->vecExifInfo = pAllResultAtStart->vecExifInfo;

            MBOOL isStartEmpty = pAllResultAtStart->vecDbg3AInfo.empty();
            MBOOL isCurEmpty = pAllResult->vecDbg3AInfo.empty();
            if(!isStartEmpty && !isCurEmpty)
            {
                // get capture start AE setting to update EXIF info
                AAA_DEBUG_INFO1_T& rDbg3AInfoStart = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResultAtStart->vecDbg3AInfo.editArray());
                AAA_DEBUG_INFO1_T& rDbg3AInfoCur = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());

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
                // update AF info
                if(pAllResult->vecDbg3AInfo.size() && pAllResult->vecDbg3AInfo.size())
                    ::memcpy(&rDbg3AInfoCur.rAFDebugInfo, &rDbg3AInfoStart.rAFDebugInfo, sizeof(AF_DEBUG_INFO_T));
            } else
            {
                MY_LOGE("isStartEmpty(%d) isCurEmpty(%d)", isStartEmpty, isCurEmpty);
            }
        }
        else
        {
            MY_LOGE("Fail get pResultAtStart (#%d) pAllResultAtStart/pAllResult:%p/%p", i4FrmId, pAllResultAtStart, pAllResult);
        }
    }
    else
    {
        MY_LOGD("[%s] got result (#%d)", __FUNCTION__, i4FrmId);
    }

    // copy capture start AF result to Capture end.
    AFResultToMeta_T* pAFResult = (AFResultToMeta_T*)m_pResultPoolObj->getResult(i4FrmId,E_AF_RESULTTOMETA,__FUNCTION__);
    if(pAFResult && pAllResult)
    {
        const HALResultToMeta_T* pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read()) );
        MY_LOGD("[%s] start AFRestult update to current AFRestult(#%d)->(#%d), capture start AFState(%d)", __FUNCTION__, i4FrmId, pHALResult->i4FrmId, pAFResult->u1AfState);
        m_pResultPoolObj->updateResult(LOG_TAG, pHALResult->i4FrmId, E_AF_RESULTTOMETA, pAFResult);
    }
    else
    {
        CAM_LOGE("Update AFResult fail pAFResult/pAllResult:%p/%p", pAFResult, pAllResult);
    }

    // copy capture start Flash result to Capture end.
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)m_pResultPoolObj->getResult(i4FrmId,E_FLASH_RESULTTOMETA,__FUNCTION__);
    if(pFLASHResult && pAllResult)
    {
        const HALResultToMeta_T* pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read()) );
        MY_LOGD_IF(m_fgLogEn, "[%s] start FLASHResult update to current FLASHRestult(#%d)->(#%d), capture start FLASH State(%d)", __FUNCTION__, i4FrmId, pHALResult->i4FrmId, pFLASHResult->u1FlashState);
        m_pResultPoolObj->updateResult(LOG_TAG, pHALResult->i4FrmId, E_FLASH_RESULTTOMETA, pFLASHResult);
    }
    else
    {
        CAM_LOGE("Update FLASHResult fail pFLASHResult/pAllResult:%p/%p", pFLASHResult, pAllResult);
    }
    return i4Ret;
}

MVOID
Hal3AFlowCtrl::
notifyP1Done(MINT32 i4MagicNum, MVOID* /*pvArg*/)
{
    MBOOL bMainFlash = m_p3AWrap->chkMainFlashOnCond();
    MBOOL bPre = m_p3AWrap->chkPreFlashOnCond();
    MBOOL bMagicNum = (i4MagicNum >= (MINT32)m_u4FlashOnIdx);

    MY_LOGD_IF(m_fgLogEn, "[%s] i4MagicNum(%d), m_u4FlashOnIdx(%d), bMagicNum(%d), bMainFlash(%d), bPre(%d), m_fgFlashOn(%d)", __FUNCTION__, i4MagicNum, m_u4FlashOnIdx, bMagicNum, bMainFlash, bPre, m_fgFlashOn);
    if ( bMagicNum && !bMainFlash && !bPre && m_fgFlashOn)
    {
        m_p3AWrap->setFlashLightOnOff(0, 1, i4MagicNum); // don't care main or pre
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
        m_fgInternalFlashOn = isNeedTurnOnPreFlash;
        MY_LOG("[%s] setPreFlashOnOff(%d)", __FUNCTION__, m_fgInternalFlashOn);
        m_p3AWrap->setPreFlashOnOff(isNeedTurnOnPreFlash);
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
    m_p3AWrap->attachCb(cb);
    return 0;
}

MINT32
Hal3AFlowCtrl::
detachCb(I3ACallBack* cb)
{
    MY_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = NULL;
    m_p3AWrap->detachCb(cb);
    return 0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// passing to wrapper functions directly
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

/*
MBOOL
Hal3AFlowCtrl::
setIspPass2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2)
{
    return m_p3AWrap->generateP2(flowType, rIspInfo, pRegBuf, pResultP2);
}
*/

MBOOL
Hal3AFlowCtrl::
notifyPwrOn()
{
    //IAfMgr::getInstance().CamPwrOnState(m_i4SensorDev);
    m_p3AWrap->notifyPwrOn();
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
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
    MY_LOGD_IF(m_fgLogEn, "[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
notifyP1PwrOn()
{
    MY_LOGD_IF(m_fgLogEn, "[%s] notifyP1PwrOn", __FUNCTION__);
    return m_p3AWrap->notifyP1PwrOn();
}

MBOOL
Hal3AFlowCtrl::
notifyP1PwrOff()
{
    MY_LOGD_IF(m_fgLogEn, "[%s] notifyP1PwrOff", __FUNCTION__);
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
queryRepeatQueue(MINT32 /*i4MagicNum*/)
{
    return 0;
}

MBOOL
Hal3AFlowCtrl::
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
   return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
   return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type)
{
    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
preset(Param_T const &rNewParam)
{
    m_p3AWrap->preset(rNewParam);
    return MTRUE;
}

MBOOL
Hal3AFlowCtrl::
queryAaoIsReady()
{
    return m_bAAOIsReady;
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
