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
#define LOG_TAG "awb_mgr_if"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>

#include <isp_tuning.h>
#include "awb_mgr_if.h"
#include "awb_mgr.h"

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbMgr&
IAwbMgr::
getInstance()
{
    static  IAwbMgr singleton;
    return  singleton;
}

MBOOL
IAwbMgr::
start(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).start();   
}

MBOOL
IAwbMgr::
stop(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).stop();
}


MBOOL
IAwbMgr::
init(MINT32 i4SensorDev, MINT32 i4SensorIdx, IAWBInitPara* initPara)
{
    return AwbMgr::getInstance(i4SensorDev).init(i4SensorIdx, initPara);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
uninit(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
isAWBEnable(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).isAWBEnable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBMode(MINT32 i4SensorDev, MINT32 i4NewAWBMode)
{
    return AwbMgr::getInstance(i4SensorDev).setAWBMode(i4NewAWBMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MINT32 i4BinWidth, MINT32 i4BinHeight, MINT32 i4QbinWidth, MINT32 i4QbinHeight)
{
    return AwbMgr::getInstance(i4SensorDev).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBMode(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBMode();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setStrobeMode(MINT32 i4SensorDev, MINT32 i4NewStrobeMode)
{
    return AwbMgr::getInstance(i4SensorDev).setStrobeMode(i4NewStrobeMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getStrobeMode(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).getStrobeMode();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setFlashAWBData(MINT32 i4SensorDev, FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData)
{
    return AwbMgr::getInstance(i4SensorDev).setFlashAWBData(rFlashAwbData);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBLock(MINT32 i4SensorDev, MBOOL bAWBLock)
{
    return AwbMgr::getInstance(i4SensorDev).setAWBLock(bAWBLock);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
enableAWB(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).enableAWB();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
disableAWB(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).disableAWB();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBStatCropRegion(MINT32 i4SensorDev,
                     MINT32 i4SensorMode,
                     MINT32 i4CropOffsetX,
                     MINT32 i4CropOffsetY,
                     MINT32 i4CropRegionWidth,
                     MINT32 i4CropRegionHeight)
{
    return AwbMgr::getInstance(i4SensorDev).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPvAWB(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MUINT32 u4ExposureTime, MBOOL bApplyToHW)
{
    return AwbMgr::getInstance(i4SensorDev).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doAFAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    return AwbMgr::getInstance(i4SensorDev).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPreCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    return AwbMgr::getInstance(i4SensorDev).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    return AwbMgr::getInstance(i4SensorDev).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doCapFlashAWBDisplay(MINT32 i4SensorDev, MBOOL bApplyToHW)
{
    return AwbMgr::getInstance(i4SensorDev).doCapFlashAWBDisplay(bApplyToHW);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if CAM3_STEREO_FEATURE_EN
MBOOL
IAwbMgr::
applyAWB(MINT32 i4SensorDev, AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput)
{
    return AwbMgr::getInstance(i4SensorDev).applyAWB(rAWBSyncOutput);
}
#endif
#if CAM3_STEREO_FEATURE_EN
MBOOL
IAwbMgr::setAWBSyncInfo(MINT32 i4SensorDev, AWB_SYNC_INPUT_N3D_T i4ZoomRatio)
{
    return AwbMgr::getInstance(i4SensorDev).setAWBSyncInfo(i4ZoomRatio);
}
#endif

MBOOL
IAwbMgr::
CalSencondSensorDefaultGain(MINT32 i4SensorDev,AWB_GAIN_T &gain, MINT32 cct)
{
    return AwbMgr::getInstance(i4SensorDev).CalSencondSensorDefaultGain(gain,cct);
}


MBOOL
IAwbMgr::
getAWBUnitGain(MINT32 i4SensorDev,AWB_CALIBRATION_DATA_T &a_rAWBCalData)

{
    return AwbMgr::getInstance(i4SensorDev).getAWBUnitGain(a_rAWBCalData);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getDebugInfo(MINT32 i4SensorDev, AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData)
{
    return AwbMgr::getInstance(i4SensorDev).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBCCT(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBCCT();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getASDInfo(MINT32 i4SensorDev, AWB_ASD_INFO_T &a_rAWBASDInfo)
{
    return AwbMgr::getInstance(i4SensorDev).getASDInfo(a_rAWBASDInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getAWBOutput(MINT32 i4SensorDev, AWB_OUTPUT_T &a_rAWBOutput)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBOutput(a_rAWBOutput);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getAWBGain(MINT32 i4SensorDev, AWB_GAIN_T &rAwbGain, MINT32& i4ScaleUnit) const
{
    return AwbMgr::getInstance(i4SensorDev).getAWBGain(rAwbGain, i4ScaleUnit);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
getAWBParentStat(MINT32 i4SensorDev, AWB_PARENT_BLK_STAT_T *a_rAWBParentState, MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY )
{
    return AwbMgr::getInstance(i4SensorDev).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IAwbMgr::
setAFLV(MINT32 i4SensorDev, MINT32 i4AFLV)
{
    return AwbMgr::getInstance(i4SensorDev).setAFLV(i4AFLV);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAFLV(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).getAFLV();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    return AwbMgr::getInstance(i4SensorDev).setTGInfo(i4TGInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setAWBNvramIdx(MINT32 const i4SensorDev, MINT32 const i4AWBNvramIdx)
{
    return AwbMgr::getInstance(i4SensorDev).setAWBNvramIdx(i4AWBNvramIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setStrobeNvramIdx(MINT32 const i4SensorDev, MINT32 const i4StrobeNvramIdx)
{
    return AwbMgr::getInstance(i4SensorDev).setStrobeNvramIdx(i4StrobeNvramIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setFlashCaliNvramIdx(MINT32 const i4SensorDev, MINT32 const i4FlashCaliNvramIdx)
{
    return AwbMgr::getInstance(i4SensorDev).setFlashCaliNvramIdx(i4FlashCaliNvramIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAwbMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 u4CamScenarioMode)
{
    return  AwbMgr::getInstance(i4SensorDev).setCamScenarioMode(u4CamScenarioMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAwbMgr::setNVRAMIndex(MINT32 i4SensorDev, MUINT32 a_eNVRAMIndex)
{
    return AwbMgr::getInstance(i4SensorDev).setNVRAMIndex(a_eNVRAMIndex);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::getColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT& fGain_R, MFLOAT& fGain_G, MFLOAT& fGain_B)
{
    return AwbMgr::getInstance(i4SensorDev).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
}

MBOOL
IAwbMgr::setIsMono(MINT32 const i4SensorDev, int bMono, MUINT32 i4SensorType)
{
    return AwbMgr::getInstance(i4SensorDev).setIsMono(bMono, i4SensorType);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT fGain_R, MFLOAT fGain_G, MFLOAT fGain_B)
{
    return AwbMgr::getInstance(i4SensorDev).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::getAWBState(MINT32 const i4SensorDev, mtk_camera_metadata_enum_android_control_awb_state_t& eAWBState)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBState(eAWBState);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Face detection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAwbMgr::setFDenable(MINT32 i4SensorDev, MBOOL bFDenable)
{
    return AwbMgr::getInstance(i4SensorDev).setFDenable(bFDenable);
}

MRESULT IAwbMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    return AwbMgr::getInstance(i4SensorDev).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
}

MRESULT IAwbMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return AwbMgr::getInstance(i4SensorDev).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
}

MRESULT IAwbMgr::getFocusArea(MINT32 i4SensorDev, android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    return AwbMgr::getInstance(i4SensorDev).getFocusArea(vecOut, i4tgwidth, i4tgheight);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::backup(MINT32 const i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).backup();
}
MBOOL IAwbMgr::restore(MINT32 const i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).restore();
}

MBOOL IAwbMgr::SetAETargetMode(MINT32 const i4SensorDev, int mode) //for hdr
{
    return AwbMgr::getInstance(i4SensorDev).SetAETargetMode(mode);
}

MBOOL
IAwbMgr::setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode)
{
    return AwbMgr::getInstance(i4SensorDev).setColorCorrectionMode(i4ColorCorrectionMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBEnable(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBEnable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBDisable(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBDisable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain, MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBSetAWBGain(a_pAWBGain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 u4CamScenarioMode)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBApplyK71NVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBApplyK71NVRAMParam(a_pAWBNVRAM);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, CAM_SCENARIO_T eIdx, MVOID *a_pAWBNVRAM)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBSaveNVRAMParam();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBMode(MINT32 i4SensorDev, MINT32 a_AWBMode)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBSetAWBMode(a_AWBMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBMode(MINT32 i4SensorDev, MINT32 *a_pAWBMode, MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetLightProb(MINT32 i4SensorDev, MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBBypassCalibration(MINT32 i4SensorDev, MBOOL bBypassCalibration)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPAWBBypassCalibration(bBypassCalibration);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM, MUINT32 *a_pOutLen)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPFlashAWBSaveNVRAMParam();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashCalibrationSaveNVRAMParam(MINT32 i4SensorDev)
{
    return AwbMgr::getInstance(i4SensorDev).CCTOPFlashCalibrationSaveNVRAMParam();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::setAAOMode(MINT32 i4SensorDev, MUINT32 u4AWBAAOmode)
{
    return AwbMgr::getInstance(i4SensorDev).setAAOMode(u4AWBAAOmode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::getAAOConfig(MINT32 i4SensorDev, AWB_AAO_CONFIG_Param_T &rAWBConfig)
{
    return AwbMgr::getInstance(i4SensorDev).getAAOConfig(rAWBConfig);
}
MBOOL IAwbMgr::setMWBColorTemperature(MINT32 i4SensorDev, int colorTemperature)
{
    return AwbMgr::getInstance(i4SensorDev).setMWBColorTemperature(colorTemperature);
}
MBOOL IAwbMgr::getSupportMWBColorTemperature(MINT32 i4SensorDev,MUINT32 &max, MUINT32 &min)
{
    return AwbMgr::getInstance(i4SensorDev).getSuppotMWBColorTemperature(max, min);
}

MBOOL IAwbMgr::getAWBColorTemperature(MINT32 i4SensorDev, MUINT32 &colorTemperature)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBColorTemperature(colorTemperature);
}
MBOOL IAwbMgr::SetTorchMode(MINT32 i4SensorDev, MBOOL is_torch_now)
{
    return AwbMgr::getInstance(i4SensorDev).SetTorchMode(is_torch_now);
}

MBOOL IAwbMgr::SetMainFlashInfo(MINT32 i4SensorDev, MBOOL is_main_flash_on)
{
    return AwbMgr::getInstance(i4SensorDev).SetMainFlashInfo(is_main_flash_on);
}

MBOOL IAwbMgr::CallBackAwb(MINT32 i4SensorDev, MVOID* pIn, MVOID* pOut)
{
    return AwbMgr::getInstance(i4SensorDev).CallBackAwb(pIn, pOut);
}

MBOOL IAwbMgr::getAWBCalibrationStatus(MINT32 i4SensorDev, calibration_status_t& i8AwbStatus)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBCalibrationStatus(i8AwbStatus);
}
MBOOL IAwbMgr::getAWBSourceRGB(MINT32 i4SensorDev, MUINT16& u2AwbSource1R, MUINT16& u2AwbSource1Gr, MUINT16& u2AwbSource1Gb, MUINT16& u2AwbSource1B)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBSourceRGB(u2AwbSource1R,u2AwbSource1Gr,u2AwbSource1Gb,u2AwbSource1B);
}
MBOOL IAwbMgr::getAWBSourceGoldenRGB(MINT32 i4SensorDev, MUINT16& u2AwbSource1GoldenR, MUINT16& u2AwbSource1GoldenGr, MUINT16& u2AwbSource1GoldenGb, MUINT16& u2AwbSource1GoldenB)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBSourceGoldenRGB(u2AwbSource1GoldenR,u2AwbSource1GoldenGr,u2AwbSource1GoldenGb,u2AwbSource1GoldenB);
}

MBOOL IAwbMgr::getAWBSourceRatio(MINT32 i4SensorDev, MFLOAT& fAwbSource1RG, MFLOAT& fAwbSource1BG, MFLOAT& fAwbSource1GRGB)
{
    return AwbMgr::getInstance(i4SensorDev).getAWBSourceRatio(fAwbSource1RG,fAwbSource1BG,fAwbSource1GRGB);
}

MBOOL IAwbMgr::setMaxFPS(MINT32 i4SensorDev, MINT32 i4MaxFPS)
{
    return AwbMgr::getInstance(i4SensorDev).setMaxFPS(i4MaxFPS);
}
