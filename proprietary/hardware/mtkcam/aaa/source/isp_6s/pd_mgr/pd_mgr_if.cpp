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
#define LOG_TAG "pdmgrv5_IF"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <private/aaa_hal_private.h>

#include "pd_mgr_if.h"
#include "pd_mgr.h"

using namespace NS3Av3;
static  IPDMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IPDMgr& IPDMgr::getInstance()
{
    return  singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::config(MINT32 sensorDev, MINT32 sensorOpenIndex, MUINT32 sensorMode, MUINT32 afNVRamIdx)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).config(sensorOpenIndex, sensorMode, afNVRamIdx);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).config(sensorOpenIndex, sensorMode, afNVRamIdx);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).config(sensorOpenIndex, sensorMode, afNVRamIdx);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).config(sensorOpenIndex, sensorMode, afNVRamIdx);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).config(sensorOpenIndex, sensorMode, afNVRamIdx);

    return PDMgr::getInstance(ESensorDev_Main).config(sensorOpenIndex, sensorMode, afNVRamIdx);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::stop(MINT32 sensorDev)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).stop();
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).stop();
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).stop();
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).stop();
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).stop();

    return PDMgr::getInstance(ESensorDev_Main).stop();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IPDMgr::CamPwrOnState(MINT32 sensorDev)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).CamPwrOnState();
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).CamPwrOnState();
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).CamPwrOnState();
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).CamPwrOnState();
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).CamPwrOnState();

    return PDMgr::getInstance(ESensorDev_Main).CamPwrOnState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IPDMgr::CamPwrOffState(MINT32 sensorDev)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).CamPwrOffState();
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).CamPwrOffState();
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).CamPwrOffState();
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).CamPwrOffState();
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).CamPwrOffState();

    return PDMgr::getInstance(ESensorDev_Main).CamPwrOffState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::doSWPDE(MINT32 sensorDev, MVOID *iHalMetaData, MVOID *iImgbuf)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).doSWPDE(iHalMetaData, iImgbuf);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).doSWPDE(iHalMetaData, iImgbuf);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).doSWPDE(iHalMetaData, iImgbuf);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).doSWPDE(iHalMetaData, iImgbuf);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).doSWPDE(iHalMetaData, iImgbuf);

    return PDMgr::getInstance(ESensorDev_Main).doSWPDE(iHalMetaData, iImgbuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::postToPDTask(MINT32 sensorDev, StatisticBufInfo *pSttData, mcuMotorInfo *pLensInfo)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).postToPDTask(pSttData, pLensInfo);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).postToPDTask(pSttData, pLensInfo);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).postToPDTask(pSttData, pLensInfo);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).postToPDTask(pSttData, pLensInfo);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).postToPDTask(pSttData, pLensInfo);

    return PDMgr::getInstance(ESensorDev_Main).postToPDTask(pSttData, pLensInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDTaskResult(MINT32 sensorDev, PD_CALCULATION_OUTPUT **ptrOutputRes)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).getPDTaskResult(ptrOutputRes);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).getPDTaskResult(ptrOutputRes);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).getPDTaskResult(ptrOutputRes);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).getPDTaskResult(ptrOutputRes);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).getPDTaskResult(ptrOutputRes);

    return PDMgr::getInstance(ESensorDev_Main).getPDTaskResult(ptrOutputRes);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::GetVersionOfPdafLibrary(MINT32 sensorDev, SPDLibVersion_t &tOutSWVer)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).GetVersionOfPdafLibrary(tOutSWVer);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).GetVersionOfPdafLibrary(tOutSWVer);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).GetVersionOfPdafLibrary(tOutSWVer);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).GetVersionOfPdafLibrary(tOutSWVer);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).GetVersionOfPdafLibrary(tOutSWVer);

    return PDMgr::getInstance(ESensorDev_Main).GetVersionOfPdafLibrary(tOutSWVer);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::GetDebugInfo( MINT32 sensorDev, AF_DEBUG_INFO_T &sOutDbgInfo)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).GetDebugInfo(sOutDbgInfo);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).GetDebugInfo(sOutDbgInfo);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).GetDebugInfo(sOutDbgInfo);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).GetDebugInfo(sOutDbgInfo);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).GetDebugInfo(sOutDbgInfo);

    return PDMgr::getInstance(ESensorDev_Main).GetDebugInfo(sOutDbgInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::UpdatePDParam(MINT32 sensorDev, MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos, MBOOL bForceCalculation)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);

    return PDMgr::getInstance(ESensorDev_Main).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDOHWCfg(MINT32 sensorDev, ISP_PDO_CFG_T *pCfg)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).getPDOHWCfg(pCfg);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).getPDOHWCfg(pCfg);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).getPDOHWCfg(pCfg);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).getPDOHWCfg(pCfg);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).getPDOHWCfg(pCfg);

    return PDMgr::getInstance(ESensorDev_Main).getPDOHWCfg(pCfg);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setNVRAMIndex(MINT32 sensorDev, MUINT32 afNVRamIdx)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).setNVRAMIndex(afNVRamIdx);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).setNVRAMIndex(afNVRamIdx);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).setNVRAMIndex(afNVRamIdx);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).setNVRAMIndex(afNVRamIdx);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).setNVRAMIndex(afNVRamIdx);

    return PDMgr::getInstance(ESensorDev_Main).setNVRAMIndex(afNVRamIdx);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDInfoForSttCtrl(MINT32 sensorDev, MINT32 sensorIdx, MINT32 sensorMode, MUINT32 &oPDOSizeW, MUINT32 &oPDOSizeH, FEATURE_PDAF_STATUS &oPDAFStatus)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);

    return PDMgr::getInstance(ESensorDev_Main).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setPBNen(MINT32 sensorDev, MBOOL bEnable)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).setPBNen(bEnable);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).setPBNen(bEnable);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).setPBNen(bEnable);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).setPBNen(bEnable);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).setPBNen(bEnable);

    return PDMgr::getInstance(ESensorDev_Main).setPBNen(bEnable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setAETargetMode(MINT32 sensorDev, eAETargetMODE eAETargetMode)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).setAETargetMode(eAETargetMode);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).setAETargetMode(eAETargetMode);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).setAETargetMode(eAETargetMode);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).setAETargetMode(eAETargetMode);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).setAETargetMode(eAETargetMode);

    return PDMgr::getInstance(ESensorDev_Main).setAETargetMode(eAETargetMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::isModuleEnable(MINT32 sensorDev, MBOOL &oModuleEn)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).isModuleEnable(oModuleEn);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).isModuleEnable(oModuleEn);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).isModuleEnable(oModuleEn);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).isModuleEnable(oModuleEn);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).isModuleEnable(oModuleEn);

    return PDMgr::getInstance(ESensorDev_Main).isModuleEnable(oModuleEn);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setRequest(MINT32 sensorDev, MINT32 iRequestNum)
{
    if (sensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).setRequest(iRequestNum);
    if (sensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).setRequest(iRequestNum);
    if (sensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).setRequest(iRequestNum);
    if (sensorDev & ESensorDev_SubSecond)
        return PDMgr::getInstance(ESensorDev_SubSecond).setRequest(iRequestNum);
    if (sensorDev & ESensorDev_MainThird)
        return PDMgr::getInstance(ESensorDev_MainThird).setRequest(iRequestNum);

    return PDMgr::getInstance(ESensorDev_Main).setRequest(iRequestNum);

}


