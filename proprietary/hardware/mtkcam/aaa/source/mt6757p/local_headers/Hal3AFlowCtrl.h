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
* @file Hal3AFlowCtrl.h
* @brief Declarations of 3A Hal Flow Control Class (public inherited from Hal3AIf)
*/

#ifndef _AAA_HAL_FLOWCTRL_H_
#define _AAA_HAL_FLOWCTRL_H_

//-------------------------------
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_timer.h>
#include "aaa_hal_if.h"
#include <ResultBufMgr.h>
#include <IEventIrq.h>
#include <IThread3A.h>
#include <I3AWrapper.h>
#include <dbg_aaa_param.h>
#include <aaa_hal_sttCtrl.h>
#include <aaa_utils.h>
#include <isp_tuning_buf.h>
#include <IHal3AResultBuf.h>
#include <mtkcam/aaa/IHal3A.h>
//-------------------------------
using namespace NSCam;
using namespace android;

namespace NS3Av3
{
class Hal3AFlowCtrl : public Hal3AIf, public IHal3ACb
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3AIf*     createInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID       destroyInstance();
    virtual MBOOL       sendCommand(ECmd_T const eCmd, MUINTPTR const i4Arg = 0);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario = MTRUE);
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    virtual MINT32      getResult(MUINT32 u4FrmId, Result_T& rResult);
    virtual MINT32      getResultCur(MUINT32 u4FrmId, Result_T& rResult);
    virtual MBOOL       autoFocus();
    virtual MBOOL       cancelAutoFocus();
    virtual MBOOL       setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    virtual MINT32      getDelayFrame(EQueryType_T const eQueryType) const;
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual MBOOL       setIspPass2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2);
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MUINT32     getSensorDev() const {return m_i4SensorDev;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MVOID       setFDEnable(MBOOL fgEnable);
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MVOID       notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0);

    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);

    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       checkCapFlash() const;

    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MBOOL       doUpdateCmd(const ParamIspProfile_T* pParam);
    virtual MINT32      config();
    virtual MVOID       stopStt();
    virtual MVOID       pause();
    virtual MVOID       resume();
    virtual MVOID       flushVSirq();
    virtual MBOOL       queryRepeatQueue(MINT32 maginNum);
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId);
    virtual void        doNotifyCb(MINT32  _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                        Hal3AFlowCtrl();
    virtual             ~Hal3AFlowCtrl();

                        Hal3AFlowCtrl(const Hal3AFlowCtrl&);
                        Hal3AFlowCtrl& operator=(const Hal3AFlowCtrl&);

    virtual MRESULT     init(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);
    virtual MRESULT     uninit();
    virtual MVOID       on3AProcSet(MBOOL bCBFlag);
    virtual MVOID       resetParams();
    virtual MBOOL       doUpdateCmdDummy();
    virtual MVOID       on3AProcFinish(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur);
    virtual MVOID       updateResult(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur);

    MUINT32             m_fgLogEn;
    MUINT32             m_fgDebugLogWEn;
    MINT32              m_i4SensorDev;
    MINT32              m_i4SensorOpenIdx;
    MUINT32             m_u4FrmIdStat;
    MUINT32             m_u4FrmIdFreeze;
    CapParam_T          m_rCapParam;
    MUINT32             m_u4FlashOnIdx;
    MUINT32             m_u4MainFlashOnIdx;
    MUINT32             m_fgFlashOn;
    I3AWrapper*         m_p3AWrap;
    IThread3A*          m_pThread;
    IEventIrq*          m_pEventIrq;
    Hal3ASttCtrl*       m_p3ASttCtrl;
    I3ACallBack*        m_pCbSet;
    Result_T            m_rResult;
    Mutex               m_rResultMtx;
    Condition           m_rResultCond;
    Param_T             m_rParam;
    AF_Param_T          m_rAFParam;
    MBOOL               m_fgPreStopStt;
    MBOOL               m_fgFlushVsIrq;
    MBOOL               m_bCallBack;
    MUINT32             m_bRRZDump;
    MUINT32             m_u4PreMagicReq;
    MBOOL               m_bTgIntAEEn;
    MFLOAT              m_fTgIntAERatio;
};

}; // namespace NS3Av3

#endif //_AAA_HAL_FLOWCTRL_H_
