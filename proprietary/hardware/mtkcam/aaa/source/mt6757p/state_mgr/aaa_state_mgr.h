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
/**
* @file aaa_state_mgr.h
* @brief Declarations of 3A state machine
*/

#ifndef _AAA_STATE_MGR_H_
#define _AAA_STATE_MGR_H_

#include <semaphore.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_state.h>
#include <IThreadRaw.h>
#include <aaa_hal_if.h>
#include <utils/List.h>


using namespace android;

namespace NS3Av3
{

//class IState; //forward declaration

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  State Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State Manager. It handles 3A intents to current state.
 */
class StateMgr
{
    friend class Hal3ARaw;
public:
    StateMgr();
    StateMgr(MINT32 sensorDevId);

  ~StateMgr();

    MRESULT sendCmd(ECmd_T eCmd);

    MINT32 attachCb(I3ACallBack* cb);
    MINT32 detachCb(I3ACallBack* cb);
    VOID doNotifyCb(MINT32 _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3);

    // for using AE thread to updateSensorByI2C
    MVOID setThreadRaw(IThreadRaw* pThreadRaw);
    MVOID postToAESenThread();
    MVOID postToAFTrigger();
    MBOOL isCapQueueEmpty();
    MBOOL queryCapQueue(MINT32 maginNum);
    MVOID updateCapQueue(MINT32 maginNum);
    MVOID switchToN3D();
    MVOID switchToNormal();

    //================================================================
    //========================== 3A State Machine ==========================
    //================================================================
    inline MVOID updateFrameCount()
    {
      if (++m_i4FrameCount >= 0x7FFFFFFF) { m_i4FrameCount = 0; }
    }

    MVOID resetFrameCount();

    inline MINT32 getFrameCount()
    {
        return m_i4FrameCount;
    }

  struct StateStatus
  {
    EState_T eCurrState;
    EState_T eNextState;
    EState_T ePrevState;
    StateStatus()
      : eCurrState(eState_Uninit)
      , eNextState(eState_Invalid)
      , ePrevState(eState_Invalid)
    {}
  };
    inline StateStatus getStateStatus() { return mStateStatus; }
    inline MVOID setNextState(EState_T eNext) { mStateStatus.eNextState = eNext; }

  MRESULT transitState(EState_T const eCurrState, EState_T const eNewState);


    //================================================================
    //========================== AF Sub State =============================
    //================================================================
    struct AFStateCntSet //centralize all AF state flag and counter here
    {
        MINT32 PreAFFrmCnt;
        MINT32 AFFrmCnt;
        MINT32 PostAFFrmCnt;
        MINT32 PreAF_bNeedToTurnOnLamp;
        MINT32 AF_bNeedToTurnOffLamp;
        MINT32 bLampAlreadyOnBeforeSingleAF;
        MINT32 bNeedToDoPrecapAF;
        MBOOL  bIsRecording;
        MBOOL  bIsFocused;
        MBOOL  bIsFocusFinish;
        MINT32 AFTimeOutFrmCnt;

        inline MVOID resetAll()
        {
            PreAFFrmCnt = AFFrmCnt = AFTimeOutFrmCnt = PostAFFrmCnt = 0;
            bNeedToDoPrecapAF = bLampAlreadyOnBeforeSingleAF = PreAF_bNeedToTurnOnLamp = AF_bNeedToTurnOffLamp = 0;
            bIsRecording = bIsFocused = bIsFocusFinish = MFALSE;
        }
        AFStateCntSet()
        {
            resetAll();
        }
    };

    /**
      * @brief proceed AF state by one stage
      */
    inline MVOID proceedAFState()
    {
        if (m_eAFState < IState::eAFState_Num)
        {
            m_eAFState = static_cast<IState::EAFState_T>(static_cast<int>(m_eAFState)+1);
        }
    }
    /**
      * @brief reset AF state to the beginning state
      */
    inline MVOID resetAFState()
    {
        m_eAFState = static_cast<IState::EAFState_T>(0);
    }
    /**
      * @brief get current AF state
      * @return current AF state; please refer to IState::EAFState_T
      */
    inline IState::EAFState_T getAFState()
    {
        return m_eAFState;
    }


    //================================================================
    //========================== Precapture Sub State ========================
    //================================================================
    /**
      * @brief proceed Precap state by one stage
      */
    inline MVOID proceedPrecapState()
    {
        if (m_ePrecapState < IState::ePrecap_Num)
        {
            m_ePrecapState = static_cast<IState::EPrecapState_T>(static_cast<int>(m_ePrecapState)+1);
        }
    }
    /**
      * @brief reset Precap state to the beginning state
      */
    inline MVOID resetPrecapState()
    {
        m_ePrecapState = static_cast<IState::EPrecapState_T>(0);
    }
    /**
      * @brief get current Precap state
      * @return current Precap state; please refer to IState::EPrecapState_T
      */
    inline IState::EPrecapState_T getPrecapState()
    {
        return m_ePrecapState;
    }

    inline MUINT32 queryMagicNumber()
    {
        return m_u4MagicNumber;
    }

    inline MVOID updateMagicNumber(MUINT32 u4MagicNumber)
    {
        m_u4MagicNumber = u4MagicNumber;
    }

    inline MUINT32 queryMagicNumberRequest()
    {
         return m_u4MagicNumberRequest;
    }

    inline MVOID updateMagicNumberRequest(MUINT32 u4MagicNumberRequest)
    {
         m_u4MagicNumberRequest = u4MagicNumberRequest;
    }
    inline MVOID setUpdateStatus(MBOOL bUpdateFlag)
    {
         m_bUpdateFlag = bUpdateFlag;
    }

    inline MBOOL queryUpdateStatus()
    {
        return m_bUpdateFlag;
    }

    inline MVOID setEnableShortExposure(MBOOL bEnableShortExposure)
    {
        m_bEnableShortExposure = bEnableShortExposure;
    }

    inline MBOOL getEnableShortExposure()
    {
        return m_bEnableShortExposure;
    }

    inline MINT32 updateCheckCount()
    {
        return (++m_i4CheckCount) % 10;
    }

    inline MVOID resetCheckCount()
    {
        m_i4CheckCount = 0;
    }

    inline MVOID setIsFlashOpened(MBOOL bIsFlashOpened)
    {
        m_bIsFlashOpened = bIsFlashOpened;
    }

    inline MBOOL getIsFlashOpened()
    {
        return m_bIsFlashOpened;
    }

    inline MINT32 getRestoreCount()
    {
        return m_i4RestoreCount;
    }

    inline MINT32 updateRestoreCount()
    {
        return ++m_i4RestoreCount;
    }

    inline MVOID resetRestoreCount()
    {
        m_i4RestoreCount = 0;
    }

    inline MVOID setIsAFTrigInPrecapState(MBOOL bAFTrigInPrecapState)
    {
        m_bAFTrigInPrecapState = bAFTrigInPrecapState;
    }

    inline MVOID setAfMode(MUINT32 u4AfMode)
    {
        m_u4AfMode = u4AfMode;
    }

    inline MUINT32 getAfMode()
    {
        return m_u4AfMode;
    }


    inline MBOOL getIsAFTrigInPrecapState()
    {
        return m_bAFTrigInPrecapState;
    }

    inline MVOID setStartCapture(MBOOL bStartCapture)
    {
        m_bStartCapture = bStartCapture;
    }
    inline MBOOL getStartCapture()
    {
        return m_bStartCapture;
    }
protected:
    IState* mpIState[eState_Number];
    IState* mpCurrentState;
    mutable Mutex m_Lock;
    mutable Mutex m_StateLock;
    IState::EAFState_T       m_eAFState; // its usage/methods are constrained
    IState::EPrecapState_T   m_ePrecapState; // its usage/methods are constrained
    StateStatus              mStateStatus;
    I3ACallBack*             m_pCbSet;
    MUINT32                  m_u4MagicNumber;
    MUINT32                  m_u4MagicNumberRequest;
    MINT32                   m_i4FrameCount;
    MINT32                   mSensorDevId;
    List<MINT32>             m_capQueue;
    MBOOL                    m_bUpdateFlag;
    MBOOL                    m_bEnableShortExposure;
    MBOOL                    m_bIsFlashOpened;
    MINT32                   m_i4CheckCount;
    MINT32                   m_i4RestoreCount;
    MBOOL                    m_bAFTrigInPrecapState;
    MBOOL                    m_bStartCapture;
    MUINT32                  m_u4AfMode;
    MBOOL                    m_bIsNoramlCam;
public:
  //add state flags here
  EState_T             mePrevStateForAe; /*NeedUpdate*/
  sem_t                mSemAF;
    IThreadRaw*          m_pThreadRaw;
    AFStateCntSet        mAFStateCntSet;
    MINT32               m_bAFStartWait;
    MUINT8               m_uAEBlockAreaY[120*90];
    MBOOL                mbIsRecording;
    MBOOL                m_bIsAFLastStateFinished;
};

#if CAM3_STEREO_FEATURE_EN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  State Manager N3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State Manager for N3D. It handles 3A intents to current state.
 */
class StateMgrN3d : public StateMgr
{
public:
    StateMgrN3d(MINT32 sensorDevId);

};
#endif

}

#endif //_AAA_STATE_MGR_H_


