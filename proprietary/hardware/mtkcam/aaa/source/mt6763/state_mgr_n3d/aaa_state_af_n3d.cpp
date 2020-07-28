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
#define LOG_TAG "aaa_state_af_n3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_state_n3d.h>
#include <aaa_state_mgr.h>
#include <aaa_common_custom.h>
#include <aaa_scheduling_custom.h>
#include <aaa_hal_if.h>
#include <aaa_hal_sttCtrl.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>

#include <af_param.h>
#include <awb_param.h>
#include <flash_param.h>

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
#include <IHal3AResultBufInfo.h>

#include <ISync3A.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAFN3d
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateAFN3d::
StateAFN3d(MINT32 sensorDevId, StateMgr* pStateMgr)
    : StateAF(sensorDevId, pStateMgr)
{
	 CAM_LOGD("[%s] sensorDevId(%d)", __FUNCTION__, sensorDevId);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAFN3d::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
  ::sem_wait(&m_pStateMgr->mSemAF);
    MRESULT err = S_3A_OK;

    //update frame count
    m_pStateMgr->updateFrameCount();
    CAM_LOGD("[StateAF::sendIntent]<eIntent_VsyncUpdate> line=%d, frameCnt=%d, EAFState=%d"
        , __LINE__
        , m_pStateMgr->getFrameCount()
        , static_cast<int>(m_pStateMgr->getAFState()));
    StatisticBufInfo* rBufInfo;
    StatisticBufInfo* rPSOBufInfo;

    // Dequeue AAO buffer from SW buffer
    IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO);
    if(pAAOBufMgr == NULL) return S_3A_OK;

    rBufInfo = pAAOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL)
    {
        ::sem_post(&m_pStateMgr->mSemAF);
        return S_3A_OK;
    }

    // Dequeue PSO buffer from SW buffer
    IBufMgr* pPSOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_PSO);
    if(pPSOBufMgr == NULL) return S_3A_OK;

    rPSOBufInfo = pPSOBufMgr->dequeueSwBuf();
    if(rPSOBufInfo == NULL)
        return S_3A_OK;

    m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    CAM_LOGD_IF(m_i4EnableLog, "[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());

    if (m_pStateMgr->getAFState() == eAFState_PreAF)
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_PreAF>(), rBufInfo, rPSOBufInfo);

    if (m_pStateMgr->getAFState() == eAFState_AF)
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_AF>(), rBufInfo);

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

        if (ISync3AMgr::getInstance()->isActive() && ISync3AMgr::getInstance()->getSync3A()->isSyncEnable())
        {
            CAM_LOGD("[%s] Sync 2A: Sensor(%d), AF End", __FUNCTION__, m_SensorDevId);
            ISync3AMgr::getInstance()->setAFState(ISync3AMgr::E_SYNC3AMGR_AF_STATE_IDLE);
        }
#if CAM3_FLASH_FEATURE_EN
        FlashMgr::getInstance().notifyAfExit(m_SensorDevId);
#endif
    }
    ::sem_post(&m_pStateMgr->mSemAF);

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
StateAFN3d::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    CAM_LOGD("[StateAF::sendIntent]<eIntent_PrecaptureStart>");

    m_pStateMgr->setNextState(eState_Precapture);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PreAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
StateAFN3d::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PreAF>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
#define AFLAMP_PREPARE_FRAME 2

    MRESULT err = S_3A_OK;

    // Update frame count
    m_pStateMgr->mAFStateCntSet.PreAFFrmCnt++;
    CAM_LOGD("[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_PreAF) PreAFFrmCnt=%d"
        , m_pStateMgr->mAFStateCntSet.PreAFFrmCnt);

    if(!CUST_ONE_SHOT_AE_BEFORE_TAF())
    {
        // change to next state directly
        CAM_LOGD("IsDoAEInPreAF is MFALSE, triggerAF, proceedAFState()");
        IAfMgr::getInstance().triggerAF(m_SensorDevId);
        m_pStateMgr->proceedAFState();
        return  S_3A_OK;
    }

    ISync3A* pSync3A = ISync3AMgr::getInstance()->getSync3A();
    MINT32 i4SyncFrmCount = pSync3A->getFrameCount();
    MINT32 i4ActiveAeItem = pSync3A->getAeSchedule();
    MINT32 i4SyncOpt = 0;
    i4SyncOpt |= ((i4ActiveAeItem & E_AE_AE_CALC) ? ISync3A::E_SYNC3A_DO_AE : 0);
    i4SyncOpt |= ISync3A::E_SYNC3A_DO_AWB;
    i4SyncOpt |= ISync3AMgr::getInstance()->getFrmSyncOpt();

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
                                   , i4ActiveAeItem, pBuf->mMagicNumber, 1);

        // workaround for iVHDR
        MUINT32 u4AFSGG1Gain;
        IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
#if CAM3_AF_FEATURE_EN
        IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);
#endif

        // AWB
        MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
        IAwbMgr::getInstance().doAFAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepLSCBuf)));
        //IAwbMgr::getInstance().doAFAWB(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));

        // Sync AE/AWB
        if (pSync3A->isSyncEnable())
        {
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) i4SyncOpt(%d) =============", __FUNCTION__, m_SensorDevId, i4SyncOpt);
            MINT32 i4Sync = pSync3A->sync(m_SensorDevId, i4SyncOpt, pBuf);
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);
        }
        // Get PSO info
        IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, pBuf->mMagicNumber, rAAOInfo);
        // Get PSO buf info
        rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
        // AE
        IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &rAAOInfo);
        IAeMgr::getInstance().doAFAE(m_SensorDevId, pBuf->mTimeStamp, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf)),
            E_AE_AE_APPLY, pBuf->mMagicNumber, 1);
        if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
        {
        #if USE_AE_THD
            m_pStateMgr->postToAESenThread();
        #else
            IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
        #endif
        }

        if(IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MTRUE) {
            m_pStateMgr->postToAFTrigger();
            m_pStateMgr->proceedAFState();
            CAM_LOGD("eAFState_PreAF, proceedAFState()");
        }
    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_AF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAFN3d::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo)
{
#define AFLAMP_OFF_PREPARE_FRAME 2
#define AF_TIME_OUT_FRAME_COUNT 30


    CAM_LOGD("[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_AF), AFFrmCnt=%d"
        , m_pStateMgr->mAFStateCntSet.AFFrmCnt);
    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);
    m_pStateMgr->mAFStateCntSet.AFTimeOutFrmCnt++;
    MBOOL bIsTimeOut = (m_pStateMgr->mAFStateCntSet.AFTimeOutFrmCnt >= AF_TIME_OUT_FRAME_COUNT);
    if (bIsTimeOut)    IAfMgr::getInstance().TimeOutHandle(m_SensorDevId);

    ISync3A* pSync3A = ISync3AMgr::getInstance()->getSync3A();

    if (!IAfMgr::getInstance().isFocusFinish(m_SensorDevId) && !bIsTimeOut)
    {
        if((FlashMgr::getInstance().isAFLampOn(m_SensorDevId)==1)
            && (FlashMgr::getInstance().getFlashMode(m_SensorDevId)!= LIB3A_FLASH_MODE_FORCE_TORCH)
            && (!m_pStateMgr->mAFStateCntSet.bLampAlreadyOnBeforeSingleAF))
            m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp=1;
        else
            m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp=0;

        if (pSync3A->isSyncEnable())
        {
            MINT32 i4SyncOpt = ISync3A::E_SYNC3A_BYP_AE;
            i4SyncOpt |= ISync3AMgr::getInstance()->getFrmSyncOpt();
            // 2A sync: independent AE/AWB
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) =============", __FUNCTION__, m_SensorDevId);
            MINT32 i4Sync = pSync3A->sync(m_SensorDevId, i4SyncOpt, pBuf);
            // apply
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);
        }

        return S_3A_OK;
    }
    //now, isFocusFinish() == MTRUE
    m_pStateMgr->mAFStateCntSet.AFFrmCnt++;
    CAM_LOGD("isFocusFinish() == MTRUE, AFFrmCnt=%d, AF_bNeedToTurnOffLamp=%d"
        , m_pStateMgr->mAFStateCntSet.AFFrmCnt
        , m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp);

    if ((m_pStateMgr->mAFStateCntSet.AF_bNeedToTurnOffLamp == 0) ||
        (m_pStateMgr->mAFStateCntSet.AFFrmCnt >= (1+AFLAMP_OFF_PREPARE_FRAME)))
    {
        if (pSync3A->isSyncEnable())
        {
            MINT32 i4SyncOpt = ISync3A::E_SYNC3A_BYP_AE;
            i4SyncOpt |= ISync3AMgr::getInstance()->getFrmSyncOpt();
            // 2A sync: independent AE/AWB
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) =============", __FUNCTION__, m_SensorDevId);
            MINT32 i4Sync = pSync3A->sync(m_SensorDevId, i4SyncOpt, pBuf);
            // apply
            CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);
        }

        m_pStateMgr->proceedAFState();
        CAM_LOGD("eAFState_AF, proceedAFState()");
        return  S_3A_OK;
    }
    //now, AF_isAFLampOn == 1 AND AFFrmCnt < 1+AFLAMP_OFF_PREPARE_FRAME
    //which means we need to do/continue our AF Lamp-off flow

    if (m_pStateMgr->mAFStateCntSet.AFFrmCnt == 1) IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MFALSE);

    IAeMgr::getInstance().setRestore(m_SensorDevId, m_pStateMgr->mAFStateCntSet.AFFrmCnt/*-1*/); //-1 --> +0: is to advance by 1 frame //-1 is to align starting from 0

    if ((m_pStateMgr->mAFStateCntSet.AFFrmCnt == 1+1/*2*/) && //+2 --> +1: is to advance by 1 frame
        (FlashMgr::getInstance().getFlashMode(m_SensorDevId)!= LIB3A_FLASH_MODE_FORCE_TORCH)       )
    {
#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
        CAM_LOGD("af sync support");
        usleep(33000);
#else
        CAM_LOGD("af sync NOT support");
#endif
        FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 0);
        m_pStateMgr->updatePreCapFlashOn(MFALSE);

        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_OFF);
        IAeMgr::getInstance().setStrobeMode(m_SensorDevId, MFALSE);
    }

    MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    IAwbMgr::getInstance().doAFAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepLSCBuf)));
    //IAwbMgr::getInstance().doAFAWB(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PostAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAFN3d::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
    MRESULT err = S_3A_OK;

    CAM_LOGD("[StateAF::sendAFIntent](eIntent_VsyncUpdate,eAFState_PostAF)");

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
    IAwbMgr::getInstance().doAFAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepLSCBuf)));
    //IAwbMgr::getInstance().doAFAWB(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepAWBBuf)));

    if(IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MTRUE)
    {
        m_pStateMgr->proceedAFState();
        CAM_LOGD("eAFState_PostAF, proceedAFState()");
        return S_3A_OK;
    }


    return  S_3A_OK;
}

