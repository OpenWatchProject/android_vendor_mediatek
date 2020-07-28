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
    return PDMgr::getInstance(sensorDev).config(sensorOpenIndex, sensorMode, afNVRamIdx);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::preStop(MINT32 sensorDev)
{
    return PDMgr::getInstance(sensorDev).preStop();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::stop(MINT32 sensorDev)
{
    return PDMgr::getInstance(sensorDev).stop();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IPDMgr::CamPwrOnState(MINT32 sensorDev)
{
    return PDMgr::getInstance(sensorDev).CamPwrOnState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IPDMgr::CamPwrOffState(MINT32 sensorDev)
{
    return PDMgr::getInstance(sensorDev).CamPwrOffState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::doSWPDE(MINT32 sensorDev, MVOID *iHalMetaData, MVOID *iImgbuf)
{
    return PDMgr::getInstance(sensorDev).doSWPDE(iHalMetaData, iImgbuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::postToPDTask(MINT32 sensorDev, StatisticBufInfo *pSttData, mcuMotorInfo *pLensInfo)
{
    return PDMgr::getInstance(sensorDev).postToPDTask(pSttData, pLensInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDTaskResult(MINT32 sensorDev, PD_CALCULATION_OUTPUT **ptrOutputRes)
{
    return PDMgr::getInstance(sensorDev).getPDTaskResult(ptrOutputRes);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::GetVersionOfPdafLibrary(MINT32 sensorDev, SPDLibVersion_t &tOutSWVer)
{
    return PDMgr::getInstance(sensorDev).GetVersionOfPdafLibrary(tOutSWVer);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::GetDebugInfo( MINT32 sensorDev, AF_DEBUG_INFO_T &sOutDbgInfo)
{
    return PDMgr::getInstance(sensorDev).GetDebugInfo(sOutDbgInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::UpdatePDParam(MINT32 sensorDev, MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos, MBOOL bForceCalculation)
{
    return PDMgr::getInstance(sensorDev).UpdatePDParam(u4FrmNum, i4InputPDAreaNum, tInputPDArea, i4MinLensPos, i4MaxLensPos, bForceCalculation);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setNVRAMIndex(MINT32 sensorDev, MUINT32 afNVRamIdx)
{
    return PDMgr::getInstance(sensorDev).setNVRAMIndex(afNVRamIdx);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDInfoForSttCtrl(MINT32 sensorDev, MINT32 sensorIdx, MINT32 sensorMode, MUINT32 &oPDOSizeW, MUINT32 &oPDOSizeH, FEATURE_PDAF_STATUS &oPDAFStatus)
{
    return PDMgr::getInstance(sensorDev).getPDInfoForSttCtrl(sensorIdx, sensorMode, oPDOSizeW, oPDOSizeH, oPDAFStatus);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setPBNen(MINT32 sensorDev, MBOOL bEnable)
{
    return PDMgr::getInstance(sensorDev).setPBNen(bEnable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setAETargetMode(MINT32 sensorDev, eAETargetMODE eAETargetMode)
{
    return PDMgr::getInstance(sensorDev).setAETargetMode(eAETargetMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::isModuleEnable(MINT32 sensorDev, MBOOL &oModuleEn)
{
    return PDMgr::getInstance(sensorDev).isModuleEnable(oModuleEn);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setRequest(MINT32 sensorDev, MINT32 iRequestNum)
{
    return PDMgr::getInstance(sensorDev).setRequest(iRequestNum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::imgoIsEnqueued(MINT32 sensorDev, StatisticBufInfo *pSttData)
{
    return PDMgr::getInstance(sensorDev).imgoIsEnqueued(pSttData);
}
