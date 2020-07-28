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

#include <Hal3AFlowCtrl.h>
#include <Hal3AFlowCtrl_SMVR.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <debug/DebugUtil.h>
#include <vector>
#include <array>
#include <private/aaa_hal_private.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/hw/HwTransform.h>

#include "private/PDTblGen.h"

using namespace NS3Av3;
using namespace NSCamHW;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MY_INST NS3Av3::INST_T<Hal3AFlowCtrl_SMVR>
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
Hal3AFlowCtrl_SMVR::
getInstance(MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount)
{
    if(i4SensorOpenIndex >= SENSOR_IDX_MAX || i4SensorOpenIndex < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorOpenIndex);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3AFlowCtrl_SMVR>(i4SensorOpenIndex);
    } );
    (rSingleton.instance)->init(i4SensorOpenIndex, i4SubsampleCount);

    return rSingleton.instance.get();
}

MVOID
Hal3AFlowCtrl_SMVR::
destroyInstance()
{
    uninit();
}

Hal3AFlowCtrl_SMVR::
Hal3AFlowCtrl_SMVR(MINT32 i4SensorIdx)
    : Hal3AFlowCtrl(i4SensorIdx)////fix resultsize
    , m_pBufferSensorThread(NULL)
    , m_i4SubsampleCount(1)
{}

Hal3AFlowCtrl_SMVR::
~Hal3AFlowCtrl_SMVR()
{}

MRESULT
Hal3AFlowCtrl_SMVR::
init(MINT32 i4SensorOpenIndex, MINT32 i4SubsampleCount) //can be called only once by RAW or YUV, no user count
{
    // ResultPool - Get result pool object
    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        CAM_LOGE("ResultPool getInstance fail");

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
    if (!pHalSensorList) return 0;

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

    m_i4SensorOpenIdx = i4SensorOpenIndex;

    MY_LOGD("[%s] eType(%d), sensor(%d), sensorIdx(%d)", __FUNCTION__, eType, m_i4SensorDev, i4SensorOpenIndex);

    // create 3A wrapper
    if (m_p3AWrap == NULL)
    {
        m_p3AWrap = I3AWrapper::getInstance(eType, i4SensorOpenIndex, i4SubsampleCount);
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
        m_p3ASttCtrl = Hal3ASttCtrl::createInstance(m_i4SensorDev, i4SensorOpenIndex);
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

    MY_LOGD("[%s] -", __FUNCTION__);
    return S_3A_OK;
}

MINT32
Hal3AFlowCtrl_SMVR::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD("[%s] +", __FUNCTION__);
    // Must start PDTblGen module before getPdInfoForSttCtrl is executed.
    ::IPDTblGen::getInstance()->start(m_i4SensorDev, m_i4SensorOpenIdx);
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
    if(m_p3ASttCtrl)
    {
        m_p3ASttCtrl->setSensorDevInfo(m_i4SensorDev, m_i4SensorOpenIdx);
        m_p3ASttCtrl->getPdInfoForSttCtrl();
        m_p3ASttCtrl->initStt(m_i4SensorDev, m_i4SensorOpenIdx, rConfigInfo);
    }
#endif
    return m_p3AWrap->config(rConfigInfo);
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
    MINT32 i4MagicNum = 0;
    MINT32 i4MagicNumCur = 0;
    MBOOL bMainFlash = MFALSE;
    MBOOL bPre = MFALSE;
    ParamIspProfile_T rParamIspProf = *pParam;
    m_rCapParam.u4CapType = E_CAPTURE_NORMAL;
    RequestSet_T rRequestSet;

    MBOOL fgUpdate = (eCmd == ECmd_Update) && (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None);
    if (eCmd == ECmd_CameraPreviewStart)
    {
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
        MY_LOGD("m_rParam.u4AeMode(%d), m_rParam.i8ExposureTime(%lld)", m_rParam.u4AeMode, m_rParam.i8ExposureTime);
        if (rParamIspProf.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
        {
            i4MagicNum = rParamIspProf.i4MagicNum;
            i4MagicNumCur = rParamIspProf.i4MagicNumCur;
            rRequestSet = rParamIspProf.rRequestSet;

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
                m_fgInternalFlashOn = MFALSE;       // Align FlowCtrl
            }
            if (bPre) {
                m_p3AWrap->setFlashLightOnOff(1, 0);
                m_fgInternalFlashOn = MFALSE;       // Align FlowCtrl
            }

            m_p3AWrap->postCommand(eCmd, &rParamIspProf);

            if (m_p3AWrap->chkCapFlash())
            {
                m_rCapParam.u4CapType = E_CAPTURE_HIGH_QUALITY_CAPTURE;
            }
            MY_LOGD_IF(m_fgLogEn, "[%s][HQC] inform p1 capture type (%d)", __FUNCTION__, m_rCapParam.u4CapType);

            i4MagicNumCur = m_p3AWrap->queryMagicNumber();

            MY_LOGD_IF(m_fgLogEn, "[%s] (MagicNum, MagicNumCur) = (%d, %d)", __FUNCTION__, i4MagicNum, i4MagicNumCur);

            // ResultPool - must update u4MagicNumCur to resultPool i4StatiticMagic
            std::vector<MINT32> rNumberSet = rRequestSet.vNumberSet;
            std::vector<MINT32>::iterator it;
            for (it = rNumberSet.begin(); it != rNumberSet.end(); it++)
            {
                MINT32 i4MagicNum = (*it);
                MY_LOGD_IF(m_fgLogEn, "[%s] MagicNum:%d", __FUNCTION__,i4MagicNum);
                RESULT_CFG_T rResultCfg = m_pResultPoolObj->getResultCFG(i4MagicNum);
                rResultCfg.i4StatisticMagic = i4MagicNumCur;
                MBOOL ret = m_pResultPoolObj->updateResultCFG(rResultCfg); //update ReqResult i4StatiticMagic of CFG
                if(!ret)
                    MY_LOGE("[%s] updateResultCFG fail", __FUNCTION__);
            }

            // i4StatiticMagic for high quality
            on3AProcFinish(rRequestSet, i4MagicNumCur);
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
        // ResultPool - preview start to clear ResultPool
        m_pResultPoolObj->clearAllResultPool();
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
Hal3AFlowCtrl_SMVR::
on3AProcFinish(const RequestSet_T rRequestSet, MINT32 i4MagicNumCur)
{
    RequestSet_T RequestCbSet;
    RequestCbSet = rRequestSet;
    std::vector<MINT32> rNumberSet = RequestCbSet.vNumberSet;
    std::vector<MINT32>::iterator it;
    MINT32 i4MagicNum = 0;
    MINT32 i4SubsampleIndex = 0;
    {
        i4MagicNum = rNumberSet[0];
        AAA_TRACE_D("updateResult#(%d,%d)", i4MagicNum, i4MagicNumCur);
        updateResult(i4MagicNum, i4SubsampleIndex);
        AAA_TRACE_END_D;
    }

    // 3A HAL updateResult with first magic, then convert 4 times.
    // If use last magic to get Result, will print error log, because pool does not validate
    // EX:13(updateResult validate-1) 14(non- updateResult validate-0) 15(non- updateResult validate-0) 16(non- updateResult validate-0)
    //for (it = rNumberSet.begin(); it != rNumberSet.end(); it++)
    //    i4MagicNum = *it;

    MY_LOGD_IF(m_fgLogEn, "[%s] get MagicNum:%d", __FUNCTION__, i4MagicNum);

    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4MagicNum);// get result
    AEResultToMeta_T *pAEResult = (AEResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicNum, E_AE_RESULTTOMETA, __FUNCTION__);
    AFResultToMeta_T *pAFResult = (AFResultToMeta_T*)m_pResultPoolObj->getResult(i4MagicNum, E_AF_RESULTTOMETA, __FUNCTION__);
    // ResultPool - get Pre Early CallBack, AFState is Pre-Pre AFState
    EARLY_CALL_BACK *rEarlyCB = m_pResultPoolObj->getEarlyCB(i4MagicNum);

    if(pAllResult && pAEResult && pAFResult && rEarlyCB)
    {
        m_rCapParam.i8ExposureTime = pAEResult->i8SensorExposureTime;
        // MW need to AFState
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AF_STATE, rEarlyCB->u1AfState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_STATE, rEarlyCB->u1AeState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_FLASH_STATE, rEarlyCB->u1FlashState);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_CONTROL_AE_MODE, rEarlyCB->u1AeMode);
        UPDATE_ENTRY_SINGLE(m_rCapParam.metadata, MTK_3A_AE_BV_TRIGGER, rEarlyCB->fgAeBvTrigger);

        if( pAllResult->vecFocusAreaPos.size()!=0)
        {
            MINT32  szW = pAFResult->i4FocusAreaSz[0];
            MINT32  szH = pAFResult->i4FocusAreaSz[1];
            const MINT32 *ptrPos = &(pAllResult->vecFocusAreaPos[0]);
            MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
            std::vector<MINT32> vecCvtPos = pAllResult->vecFocusAreaPos;
            MINT32 *ptrCvtPos = &(vecCvtPos[0]);
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
        MY_LOGE("[%s] pAllResult/pAEResult/pAFResult/rEarlyCB is NULL", __FUNCTION__);

    if (m_pCbSet)
    {
        MINT32 i4CurId = m_p3AWrap->getCurrentHwId();

        MY_LOGD_IF(m_fgLogEn, "[%s] #(%d,%d), SOF(0x%x), m_rResult.i8SensorExposureTime(%lld), m_rCapParam.capType(%d)",
                   __FUNCTION__, i4MagicNum, i4MagicNumCur, i4CurId, m_rCapParam.i8ExposureTime, m_rCapParam.u4CapType);

        if (rEarlyCB) {
            MY_LOGD_IF(m_fgLogEn, "[%s] u1AfState(%d), u1AeState(%d), u1FlashState(%d), u4AeMode(%d), fgAeBvTrigger(%d)", __FUNCTION__, rEarlyCB->u1AfState, rEarlyCB->u1AeState, rEarlyCB->u1FlashState, rEarlyCB->u1AeMode, rEarlyCB->fgAeBvTrigger);
        }

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
Hal3AFlowCtrl_SMVR::
updateResult(MINT32 i4MagicNum, MINT32 i4SubsampleIndex)
{
    MY_LOGD_IF(m_fgLogEn, "[%s] i4MagicNum(%d)", __FUNCTION__, i4MagicNum);

    // ResultPool - Get MGR result to update resultPool with i4MagicNum
    // pass1 result
    MINT32 i4Ret = m_p3AWrap->getCurrResult(i4MagicNum, i4SubsampleIndex);
    // result for high quality
    m_p3AWrap->setIspSensorInfo2AF(i4MagicNum);

    #if 0
    Mutex::Autolock autoLock(m_rResultMtx);

    m_rResultBufCur.updateResult(m_u4FrmIdStat, m_rResult);
    MY_LOGD_IF(m_fgLogEn, "[%s] u4MagicNum(%d) HQC", __FUNCTION__, u4MagicNum);
    m_rResultCond.broadcast();
    #endif
}
