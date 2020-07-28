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
* @file I3AWrapper.h
* @brief Declarations of 3A Wrapper Interface Class
*/

#ifndef _I3A_WRAPPER_H_
#define _I3A_WRAPPER_H_

//-------------------------------
#include <aaa_types.h>
#include <dbg_aaa_param.h>
#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <aaa_result.h>
//-------------------------------
namespace NS3Av3
{
class I3AWrapper
{
public:
    typedef enum
    {
        E_TYPE_DFT = 0,
        E_TYPE_RAW,
        E_TYPE_YUV,
        E_TYPE_SMVR
    } E_TYPE_T;

    static I3AWrapper*  getInstance(E_TYPE_T eType, MINT32 const i4SensorOpenIndex, MINT32 i4SubsampleCount = 1);
    virtual MVOID       destroyInstance() = 0;
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo) = 0;
    virtual MBOOL       start() = 0;
    virtual MBOOL       stop() = 0;
    virtual MVOID       pause() = 0;
    virtual MVOID       resume(MINT32 MagicNum = 0) = 0;
    virtual MVOID       setSensorMode(MINT32 i4SensorMode) = 0;
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rCamInfo, void* pRegBuf, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe) = 0;
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario) = 0;
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam) = 0;
    virtual MBOOL       autoFocus() = 0;
    virtual MBOOL       cancelAutoFocus() = 0;
    virtual MVOID       setFDEnable(MBOOL fgEnable) = 0;
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces) = 0;
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT) = 0;
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum) = 0;
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/, MINT32 i4P1DoneSttNum = -1) = 0;
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/) = 0;
    virtual MBOOL       isNeedTurnOnPreFlash() const = 0;
    virtual MBOOL       chkMainFlashOnCond() const = 0;
    virtual MBOOL       chkPreFlashOnCond() const = 0;
    virtual MBOOL       isStrobeBVTrigger() const = 0;
    virtual MBOOL       chkCapFlash() const = 0;
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleIndex = 0) const = 0;
    virtual MINT32      getCurrentHwId() const = 0;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0) = 0;
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) = 0;
    virtual MINT32      queryMagicNumber() const = 0;
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight) = 0;
    virtual MINT32      attachCb(I3ACallBack* cb) = 0;
    virtual MINT32      detachCb(I3ACallBack* cb) = 0;
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight) = 0;
    virtual MBOOL       notifyPwrOn() = 0;
    virtual MBOOL       notifyPwrOff() = 0;
    virtual MBOOL       notifyP1PwrOn() = 0;
    virtual MBOOL       notifyP1PwrOff() = 0;
    virtual MBOOL       dumpP1Params(MINT32 i4MagicNum) = 0;
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    virtual MBOOL       preset(Param_T const &rNewParam) = 0;
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId) = 0;
    virtual MBOOL       notify4CCU(MUINT32 u4PreFrmId, ISP_NVRAM_OBC_T const &rOBCResult) = 0;
    virtual MVOID       notifyPreStop() = 0;
    virtual MBOOL       set3AISPInfo() {return MTRUE;}; // Use in isp 3.0 only
protected:  //    Ctor/Dtor.
                        I3AWrapper(){}
    virtual             ~I3AWrapper(){}

                        I3AWrapper(const I3AWrapper&);
                        I3AWrapper& operator=(const I3AWrapper&);
    //virtual MUINT32     Scenario4AEAWB(Param_T const &rParam);
    //virtual MUINT32     Scenario4AF(Param_T const &rParam);

};
};
#endif //_I3A_WRAPPER_H_