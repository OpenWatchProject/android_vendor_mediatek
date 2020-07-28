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
#define LOG_TAG "aaa_state_af"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_state.h>
#include <aaa_state_mgr.h>
#include <aaa_common_custom.h>
#include <aaa_hal_if.h>
#include <aaa_hal_sttCtrl.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>

#include <af_param.h>
#include <awb_param.h>
#include <aaa/flash_param.h>

#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>

#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_mgr/flash_mgr.h>
#include <flash_feature.h>
#include <flicker/flicker_hal_base.h>
#include <sensor_mgr/aaa_sensor_buf_mgr.h>

#include <awb_tuning_custom.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <flash_tuning_custom.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <IHal3AResultBufInfo.h>

using namespace NS3Av3;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateAF::
StateAF(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateAF", sensorDevId, pStateMgr)
{
   sem_init(&m_pStateMgr->mSemAF, 0, 1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_AFStart>)
{
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendIntent]<eIntent_AFStart>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_AFEnd>)
{
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendIntent]<eIntent_AFEnd>");

#if CAM3_FLASH_FEATURE_EN
    CAM_LOGD_IF(m_i4EnableLog, "isAFLampOn=%d, getFlashMode=%d, bLampAlreadyOnBeforeSingleAF=%d\n"
        , FlashMgr::getInstance().isAFLampOn(m_SensorDevId)
        , FlashMgr::getInstance().getFlashMode(m_SensorDevId)
        , m_pStateMgr->mAFStateCntSet.bLampAlreadyOnBeforeSingleAF);

    //this logic condition is referred to (copied from then modified)
    //sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo)
    if((FlashMgr::getInstance().isAFLampOn(m_SensorDevId)==1)
        && (FlashMgr::getInstance().getFlashMode(m_SensorDevId)!= LIB3A_FLASH_MODE_FORCE_TORCH)
        && (!m_pStateMgr->mAFStateCntSet.bLampAlreadyOnBeforeSingleAF))
        m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp=1;
    else
        m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp=0;

    CAM_LOGD_IF(m_i4EnableLog, "AF_bNeedToTurnOffLamp=%d\n", m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp);

    if (m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp) FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 0);
#endif

    // State transition: eState_AF --> mePrevState
    if(m_pStateMgr->getStateStatus().eNextState!=eState_Invalid)
    {
        m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().eNextState);
        m_pStateMgr->setNextState(eState_Invalid);
    }
    else // eNextState==eState_Invalid
    {
        m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().ePrevState);
    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_Uninit>)
{
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendIntent]<eIntent_Uninit>");
    // State transition: eState_AF --> eState_Uninit
    m_pStateMgr->transitState(eState_AF, eState_Uninit);

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MRESULT err = S_3A_OK;

    //update frame count
    m_pStateMgr->updateFrameCount();
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendIntent]<eIntent_VsyncUpdate> line=%d, frameCnt=%d, EAFState=%d"
        , __LINE__
        , m_pStateMgr->getFrameCount()
        , static_cast<int>(m_pStateMgr->getAFState()));

    StatisticBufInfo* rBufInfo;
    StatisticBufInfo* rPSOBufInfo;

    // Dequeue AAO buffer from SW buffer
    IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO);
    if(pAAOBufMgr == NULL) return S_3A_OK;

    rBufInfo = pAAOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL) return S_3A_OK;

    // Dequeue PSO buffer from SW buffer
    IBufMgr* pPSOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_PSO);
    if(pPSOBufMgr == NULL) return S_3A_OK;

    rPSOBufInfo = pPSOBufMgr->dequeueSwBuf();
    if(rPSOBufInfo == NULL) return S_3A_OK;

    m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    CAM_LOGD_IF(m_i4EnableLog, "[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());

    if (m_pStateMgr->getAFState() == eAFState_PreAF)
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_PreAF>(), rBufInfo, rPSOBufInfo);

    else if (m_pStateMgr->getAFState() == eAFState_AF)
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_AF>(), rBufInfo, rPSOBufInfo);

    if (m_pStateMgr->getAFState() == eAFState_PostAF)
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_PostAF>(), rBufInfo, rPSOBufInfo);

    if (m_pStateMgr->getAFState() == eAFState_Num) //at the end of AF flow, transitState & CallbackNotify
    {
        if(m_pStateMgr->getIsAFTrigInPrecapState())
        {
#if CAM3_FLASH_FEATURE_EN
            if(FlashMgr::getInstance().isAFLampOn(m_SensorDevId) && !m_pStateMgr->getIsFlashOpened())
                FlashMgr::getInstance().setCapPara(m_SensorDevId);
#endif

#if CAM3_AF_FEATURE_EN
            IAfMgr::getInstance().WaitTriggerAF(m_SensorDevId,MFALSE);
#endif
            m_pStateMgr->setIsAFTrigInPrecapState(MFALSE);
        }

        m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
        m_pStateMgr->mAFStateCntSet.bIsFocused = MFALSE;
        m_pStateMgr->mAFStateCntSet.bIsFocusFinish = MFALSE;
        if(m_pStateMgr->getStateStatus().eNextState!=eState_Invalid)
        {
            m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().eNextState);
            m_pStateMgr->setNextState(eState_Invalid);
        }
        else
            m_pStateMgr->transitState(eState_AF, m_pStateMgr->getStateStatus().ePrevState);

#if CAM3_FLASH_FEATURE_EN
        FlashMgr::getInstance().notifyAfExit(m_SensorDevId);
#endif
    }

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendIntent]<eIntent_PrecaptureStart>");

    // State transition: eState_AF --> eState_Precapture
    //m_pStateMgr->transitState(eState_AF, eState_Precapture);
    m_pStateMgr->setNextState(eState_Precapture);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PreAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PreAF>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
#define AFLAMP_PREPARE_FRAME 2

    MRESULT err = S_3A_OK;

    // Update frame count
    m_pStateMgr->mAFStateCntSet.PreAFFrmCnt++;
    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_PreAF) PreAFFrmCnt=%d"
        , m_pStateMgr->mAFStateCntSet.PreAFFrmCnt);

    if(!CUST_ONE_SHOT_AE_BEFORE_TAF())
    {
        // change to next state directly
        CAM_LOGD_IF(m_i4EnableLog, "IsDoAEInPreAF is MFALSE, triggerAF, proceedAFState()");
        IAfMgr::getInstance().triggerAF(m_SensorDevId);
        m_pStateMgr->proceedAFState();
        return  S_3A_OK;
    }

    // do AE/AWB before AF start
    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);

#if CAM3_FLASH_FEATURE_EN
    if(m_pStateMgr->mAFStateCntSet.PreAFFrmCnt==1)
        m_pStateMgr->mAFStateCntSet.bLampAlreadyOnBeforeSingleAF = FlashMgr::getInstance().isAFLampOn(m_SensorDevId);

    if((m_pStateMgr->mAFStateCntSet.PreAFFrmCnt==1) &&
       (!m_pStateMgr->mAFStateCntSet.bLampAlreadyOnBeforeSingleAF))
    {
        CAM_LOGD_IF(m_i4EnableLog, "Check and set AF Lamp On/Off");
        MBOOL bIsrecording = m_pStateMgr->mAFStateCntSet.bIsRecording;
        if (bIsrecording != MTRUE)
        {
            m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp = cust_isNeedAFLamp(
                      FlashMgr::getInstance().getFlashMode(m_SensorDevId),
                      FlashMgr::getInstance().getAfLampMode(m_SensorDevId),
                      IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));
            CAM_LOGD_IF(m_i4EnableLog, "eAFState_PreAF-cust_isNeedAFLamp ononff:%d flashM:%d AfLampM:%d triger:%d ",
                      m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp,
                      FlashMgr::getInstance().getFlashMode(m_SensorDevId),
                      FlashMgr::getInstance().getAfLampMode(m_SensorDevId),
                      IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));
        }
        else
        {
            CAM_LOGD_IF(m_i4EnableLog, "eAFState_PreAF-cust_isNeedAFLamp ononff:%d bIsrecording:%d ",
                                  m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp,
                                  m_pStateMgr->mAFStateCntSet.bIsRecording);
        }

        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId,
            (m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp) ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);
        IAeMgr::getInstance().setStrobeMode(m_SensorDevId,
            (m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp) ? MTRUE : MFALSE);
        if(m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp==1)
        {
            CAM_LOGD_IF(m_i4EnableLog, "eAFState_PreAF-isAFLampOn=1");
            IAeMgr::getInstance().doBackAEInfo(m_SensorDevId);
            IAwbMgr::getInstance().backup(m_SensorDevId);
            FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 1);
        }
    }
#endif

    // if lamp is off, or lamp-on is ready
    if ((m_pStateMgr->mAFStateCntSet.PreAF_bNeedToTurnOnLamp == 0) ||
        (m_pStateMgr->mAFStateCntSet.PreAFFrmCnt >= (1+AFLAMP_PREPARE_FRAME)))
    {

        // Get PSO info
        AAO_PROC_INFO_T rAAOInfo;
        IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, pBuf->mMagicNumber, rAAOInfo);
        // Get PSO buf info
        rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
        // AE
        IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &rAAOInfo);
        IAeMgr::getInstance().doAFAE(m_SensorDevId, pBuf->mTimeStamp
                                   , reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf))
                                   , 0, pBuf->mMagicNumber, 0);
        if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
        {
#if USE_AE_THD
            m_pStateMgr->postToAESenThread();
#else
            IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
#endif
        }

        NSIspTuningv3::IspTuningMgr::GMA_AE_DYNAMIC_INFO dynamicInfo;
        dynamicInfo.bStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
        NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC, (MINTPTR)&dynamicInfo, 0);

        // workaround for iVHDR
        MUINT32 u4AFSGG1Gain;
        IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);
#endif
        MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
        IAwbMgr::getInstance().doAFAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));
        //IAwbMgr::getInstance().doAFAWB(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));

        if(IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MTRUE) {
            if(!m_pStateMgr->getIsAFTrigInPrecapState())
            {
                MUINT32 u4AfMode = m_pStateMgr->getAfMode();
                if( u4AfMode == MTK_CONTROL_AF_MODE_AUTO || u4AfMode == MTK_CONTROL_AF_MODE_MACRO)
                {
                    m_pStateMgr->m_bIsAFLastStateFinished = MTRUE;
                }
                m_pStateMgr->postToAFTrigger();
            }
            else
            {
                MBOOL isFocused = MTRUE;
                MBOOL isFlashOn = MFALSE;
                MBOOL isFocusFinish = MTRUE;
#if CAM3_AF_FEATURE_EN
                isFocused = m_pStateMgr->mAFStateCntSet.bIsFocused;
                isFocusFinish = m_pStateMgr->mAFStateCntSet.bIsFocusFinish;
#endif

#if CAM3_FLASH_FEATURE_EN
                isFlashOn = FlashMgr::getInstance().isAFLampOn(m_SensorDevId);
#endif
                CAM_LOGD("[%s] isFocused(%d), isFlashOn(%d), isFocusFinish(%d)", __FUNCTION__, isFocused, isFlashOn, isFocusFinish);
                if(isFlashOn == MTRUE)
                {
                    CAM_LOGD("[%s] Flash On, wait AF done", __FUNCTION__);
                    IAfMgr::getInstance().cancelAutoFocus(m_SensorDevId);
                    if(!IAfMgr::getInstance().isFocusFinish(m_SensorDevId))
                    {
                        MY_LOG("[%s] Flash On, wait AF isFocusFinish", __FUNCTION__);
                        return S_3A_OK;
                    }
                    IAfMgr::getInstance().WaitTriggerAF(m_SensorDevId, MTRUE);
                    IAfMgr::getInstance().autoFocus(m_SensorDevId);
                    m_pStateMgr->m_bIsAFLastStateFinished = MTRUE;
                    m_pStateMgr->postToAFTrigger();
                } else if(!isFocusFinish)
                {
                    CAM_LOGD("[%s] Not Focus Finish, wait AF done", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
                    IAfMgr::getInstance().WaitTriggerAF(m_SensorDevId, MFALSE);
#endif
                } else if(!isFocused)
                {
                    CAM_LOGD("[%s] Focus Finish, but AF isn't Focused", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
                    IAfMgr::getInstance().WaitTriggerAF(m_SensorDevId, MFALSE);
#endif
                    //m_pStateMgr->postToAFTrigger();
                } else
                {
                    CAM_LOGD("[%s] Focus Finish, and AF is Focused", __FUNCTION__);
#if CAM3_AF_FEATURE_EN
                    IAfMgr::getInstance().WaitTriggerAF(m_SensorDevId, MFALSE);
#endif
                }
            }

            m_pStateMgr->proceedAFState();
            CAM_LOGD_IF(m_i4EnableLog, "eAFState_PreAF, proceedAFState()");
        }
    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_AF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
#define AFLAMP_OFF_PREPARE_FRAME 2
#define AF_TIME_OUT_FRAME_COUNT 30

    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);
    // Get PSO info
    AAO_PROC_INFO_T rAAOInfo;
    IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, pBuf->mMagicNumber, rAAOInfo);

    rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
    // AE
    IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &rAAOInfo);
    IAeMgr::getInstance().doAFAEAAO(m_SensorDevId,reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf)),pBuf->mMagicNumber);

    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_AF), AFFrmCnt=%d"
        , m_pStateMgr->mAFStateCntSet.AFFrmCnt);
    MBOOL bAFLastState, bAFCurrentState, bIsFocusFinished;
    MBOOL bIsTimeOut = MFALSE;
    bAFLastState = m_pStateMgr->m_bIsAFLastStateFinished;
    bAFCurrentState = IAfMgr::getInstance().isFocusFinish(m_SensorDevId);
    bIsFocusFinished = !bAFLastState && bAFCurrentState;

    m_pStateMgr->mAFStateCntSet.AFTimeOutFrmCnt++;
    MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode(m_SensorDevId);
    if(i4OperMode == EOperMode_Normal && m_pStateMgr->mAFStateCntSet.AFTimeOutFrmCnt >= AF_TIME_OUT_FRAME_COUNT)
    {
        CAM_LOGD("[%s] AF Time out", __FUNCTION__);
        IAfMgr::getInstance().TimeOutHandle(m_SensorDevId);
        bIsTimeOut = MTRUE;
    }
    CAM_LOGD("[%s] bAFLastState(%d), bAFCurrentState(%d), bIsFocusFinished(%d), AFTimeOutFrmCnt(%d)",__FUNCTION__, bAFLastState, bAFCurrentState, bIsFocusFinished, m_pStateMgr->mAFStateCntSet.AFTimeOutFrmCnt);
    if (!bIsFocusFinished && !bIsTimeOut)
    {
        m_pStateMgr->m_bIsAFLastStateFinished = bAFCurrentState;
        return S_3A_OK;
    }
    //now, isFocusFinish() == MTRUE

    m_pStateMgr->m_bIsAFLastStateFinished = MTRUE;

    m_pStateMgr->proceedAFState();
    CAM_LOGD_IF(m_i4EnableLog, "eAFState_AF, proceedAFState()");
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PostAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
    MRESULT err = S_3A_OK;

    CAM_LOGD_IF(m_i4EnableLog, "[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_PostAF)");

    if(CUST_ONE_SHOT_AE_BEFORE_TAF())
    {
        m_pStateMgr->proceedAFState();
        return S_3A_OK;
    }
    // now, IsDoAEInPreAF == MFALSE
    // do AE/AWB after AF done
    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);

        // AE
        /*NeedUpdate*///CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    // Get PSO info
    AAO_PROC_INFO_T rAAOInfo;
    IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, pBuf->mMagicNumber, rAAOInfo);
    // Get PSO buf info
    rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
    // AE
    IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &rAAOInfo);
    IAeMgr::getInstance().doAFAE(m_SensorDevId, pBuf->mTimeStamp
                               , reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf))
                               , 0, pBuf->mMagicNumber, 0);
    if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
    {
    #if USE_AE_THD
        //m_pHal3A->mbPostAESenThd = MTRUE;
        m_pStateMgr->postToAESenThread();
    #else
        IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
    #endif
    }

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);    // Profiling Start.

    MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    IAwbMgr::getInstance().doAFAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));
    //IAwbMgr::getInstance().doAFAWB(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));

    if(IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MTRUE)
    {
        m_pStateMgr->proceedAFState();
        CAM_LOGD_IF(m_i4EnableLog, "eAFState_PostAF, proceedAFState()");
        return S_3A_OK;
    }


    return  S_3A_OK;
}

