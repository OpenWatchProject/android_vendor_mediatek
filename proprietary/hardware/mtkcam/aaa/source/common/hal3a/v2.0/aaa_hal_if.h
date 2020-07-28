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

#ifndef _AAA_HAL_IF_H_
#define _AAA_HAL_IF_H_

#include <mtkcam/def/common.h>
#include <private/aaa_hal_private.h>
#include "aaa_result.h"

using namespace NSCam;
using namespace NS3Av3;

//class IBaseCamExif;

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class I3ACallBack
{
public:

    virtual             ~I3ACallBack() {}

public:

    virtual void        doNotifyCb (
                           MINT32  _msgType,
                           MINTPTR _ext1,
                           MINTPTR _ext2,
                           MINTPTR _ext3
                        ) = 0;

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        ) = 0;
public:

    enum ECallBack_T
    {
        eID_NOTIFY_AF_FOCUSED,
        eID_NOTIFY_AF_MOVING,
        eID_DATA_AF_FOCUSED,
        eID_NOTIFY_3APROC_FINISH,
        eID_NOTIFY_VSYNC_DONE,
        eID_NOTIFY_READY2CAP,
        eID_NOTIFY_CURR_RESULT,
        eID_NOTIFY_AE_RT_PARAMS,
        eID_NOTIFY_HDRD_RESULT,
        eID_NOTIFY_LCS_ISP_PARAMS,
        eID_NOTIFY_AF_FSC_INFO,
        eID_MSGTYPE_NUM
    };

    enum E3APROC_FINISH_BIT
    {
        e3AProcOK           = 0,
        e3APvInitReady    = 1,
        e3AProcNum
    };
};

class Hal3AIf
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AIf() {}
    virtual ~Hal3AIf() {}

private: // disable copy constructor and copy assignment operator
    Hal3AIf(const Hal3AIf&);
    Hal3AIf& operator=(const Hal3AIf&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum E_FlowControl_Type
    {
       E_FlowControl_Type_NORMAL,
       E_FlowControl_Type_SMVR
    };
    //
    static Hal3AIf*     getInstance(MINT32 const i4SensorOpenIndex, E_FlowControl_Type type, MINT32 i4SubsampleCount = 1);
    virtual MVOID       destroyInstance() = 0;
    virtual MBOOL       sendCommand(ECmd_T const eCmd, MUINTPTR const i4Arg = 0) = 0;
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario = MTRUE) = 0;
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam) = 0;
    virtual MINT32      getResult(MINT32 i4FrmId) = 0;
    virtual MINT32      getResultCur(MINT32 i4FrmId) = 0;
    virtual MBOOL       autoFocus() = 0;
    virtual MBOOL       cancelAutoFocus() = 0;
    virtual MBOOL       setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height) = 0;
    virtual MINT32      getDelayFrame(EQueryType_T const eQueryType) const = 0;
    virtual MINT32      attachCb(I3ACallBack* cb) = 0;
    virtual MINT32      detachCb(I3ACallBack* cb) = 0;
    //virtual MBOOL       setIspPass2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2) = 0;
    virtual MVOID       setSensorMode(MINT32 i4SensorMode) = 0;
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight) = 0;
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight) = 0;
    virtual MUINT32     getSensorDev() const = 0;
    virtual MVOID       setFDEnable(MBOOL fgEnable) = 0;
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces) = 0;
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT) = 0;
    virtual MVOID       notifyP1Done(MINT32 i4MagicNum, MVOID* pvArg = 0) = 0;

    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) = 0;

    virtual MBOOL       notifyPwrOn() = 0;
    virtual MBOOL       notifyPwrOff() = 0;
    virtual MBOOL       notifyP1PwrOn() = 0;
    virtual MBOOL       notifyP1PwrOff() = 0;
    virtual MBOOL       checkCapFlash() const = 0;

    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0) = 0;
    virtual MBOOL       doUpdateCmd(const ParamIspProfile_T* pParam) = 0;
    virtual MBOOL       doUpdateCmdDummy() = 0;
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo) = 0;
    virtual MVOID       on3AProcSet(MBOOL bCallBackFlag) = 0;
    virtual MVOID       stopStt() = 0;
    virtual MVOID       pause() = 0;
    virtual MVOID       resume(MINT32 MagicNum = 0) = 0;
    virtual MVOID       flushVSirq() = 0;
    virtual MBOOL       queryRepeatQueue(MINT32 maginNum) = 0;
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2) = 0;
    //virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type) = 0;
    virtual MBOOL       preset(Param_T const &rNewParam) = 0;
    virtual MBOOL       queryAaoIsReady() = 0;
};
}; // namespace NS3Av3

#endif

