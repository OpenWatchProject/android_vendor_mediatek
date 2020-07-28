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
#define LOG_TAG "af_mgr_if"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
//#include <faces.h>
#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <isp_reg.h>
#include <isp_drv.h>
//
#include <cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/aaa/drv/mcu_drv.h>


#include <af_feature.h>
class NvramDrvBase;
using namespace android;

#include <af_algo_if.h>
//#include <CamDefs.h>
#include "af_mgr_if.h"
#include "af_mgr.h"


using namespace NS3Av3;
static  IAfMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAfMgr& IAfMgr::getInstance()
{
    return  singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::init(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 isInitMCU)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).init(i4SensorIdx, isInitMCU);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).init(i4SensorIdx, isInitMCU);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).init(i4SensorIdx, isInitMCU);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).init(i4SensorIdx, isInitMCU);

    return AfMgr::getInstance(ESensorDev_Main).init(i4SensorIdx, isInitMCU);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::uninit(MINT32 i4SensorDev, MINT32 isInitMCU)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).uninit(isInitMCU);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).uninit(isInitMCU);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).uninit(isInitMCU);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).uninit(isInitMCU);

    return AfMgr::getInstance(ESensorDev_Main).uninit(isInitMCU);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetPauseAF(MINT32 i4SensorDev, MBOOL bIsPause)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SetPauseAF(bIsPause);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SetPauseAF(bIsPause);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SetPauseAF(bIsPause);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SetPauseAF(bIsPause);

    return AfMgr::getInstance(ESensorDev_Main).SetPauseAF(bIsPause);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::triggerAF(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).triggerAF(AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).triggerAF(AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).triggerAF(AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).triggerAF(AF_CMD_CALLER);

    return AfMgr::getInstance(ESensorDev_Main).triggerAF(AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::WaitTriggerAF(MINT32 i4SensorDev, MBOOL bWait)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).WaitTriggerAF(bWait);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).WaitTriggerAF(bWait);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).WaitTriggerAF(bWait);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).WaitTriggerAF(bWait);

    return AfMgr::getInstance(ESensorDev_Main).WaitTriggerAF(bWait);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFMode(MINT32 i4SensorDev,MINT32 a_eAFMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAFMode(a_eAFMode, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAFMode(a_eAFMode, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAFMode(a_eAFMode, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setAFMode(a_eAFMode, AF_CMD_CALLER);

    return AfMgr::getInstance(ESensorDev_Main).setAFMode(a_eAFMode, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT  IAfMgr::getFocusAreaResult(MINT32 i4SensorDev, android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);

    return AfMgr::getInstance(ESensorDev_Main).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getFocusArea( MINT32 i4SensorDev, android::Vector<MINT32> &vecOut)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocusArea(vecOut);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocusArea(vecOut);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocusArea(vecOut);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getFocusArea(vecOut);

    return AfMgr::getInstance(ESensorDev_Main).getFocusArea(vecOut);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NS3A::EAfState_T IAfMgr::getAFState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFState();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFState();

    return AfMgr::getInstance(ESensorDev_Main).getAFState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFArea(MINT32 i4SensorDev, CameraFocusArea_T a_sAFArea)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAFArea( a_sAFArea);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAFArea( a_sAFArea);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAFArea( a_sAFArea);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setAFArea( a_sAFArea);

    return AfMgr::getInstance(ESensorDev_Main).setAFArea( a_sAFArea);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 a_eCamScenarioMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setCamScenarioMode(a_eCamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setCamScenarioMode(a_eCamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setCamScenarioMode(a_eCamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setCamScenarioMode(a_eCamScenarioMode);

    return AfMgr::getInstance(ESensorDev_Main).setCamScenarioMode(a_eCamScenarioMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetCropRegionInfo(MINT32 i4SensorDev,MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);

    return AfMgr::getInstance(ESensorDev_Main).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setFDInfo(a_sFaces);

    return AfMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces);
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sObtinfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setOTInfo(a_sObtinfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setOTInfo(a_sObtinfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setOTInfo(a_sObtinfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setOTInfo(a_sObtinfo);

    return AfMgr::getInstance(ESensorDev_Main).setOTInfo(a_sObtinfo);
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFMaxAreaNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFMaxAreaNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFMaxAreaNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFMaxAreaNum();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFMaxAreaNum();

    return AfMgr::getInstance(ESensorDev_Main).getAFMaxAreaNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isAFSupport(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isAFSupport();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isAFSupport();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isAFSupport();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).isAFSupport();

    return AfMgr::getInstance(ESensorDev_Main).isAFSupport();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isAFEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isAFEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isAFEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isAFEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).isAFEnable();

    return AfMgr::getInstance(ESensorDev_Main).isAFEnable();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMaxLensPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getMaxLensPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getMaxLensPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getMaxLensPos();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getMaxLensPos();

    return AfMgr::getInstance(ESensorDev_Main).getMaxLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMinLensPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getMinLensPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getMinLensPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getMinLensPos();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getMinLensPos();

    return AfMgr::getInstance(ESensorDev_Main).getMinLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFBestPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFBestPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFBestPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFBestPos();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFBestPos();

    return AfMgr::getInstance(ESensorDev_Main).getAFBestPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFPos();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFPos();

    return AfMgr::getInstance(ESensorDev_Main).getAFPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFStable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFStable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFStable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFStable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFStable();

    return AfMgr::getInstance(ESensorDev_Main).getAFStable();
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableOffset(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableOffset();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableOffset();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableOffset();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFTableOffset();

    return AfMgr::getInstance(ESensorDev_Main).getAFTableOffset();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableMacroIdx(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableMacroIdx();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableMacroIdx();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableMacroIdx();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFTableMacroIdx();

    return AfMgr::getInstance(ESensorDev_Main).getAFTableMacroIdx();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableIdxNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableIdxNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableIdxNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableIdxNum();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFTableIdxNum();

    return AfMgr::getInstance(ESensorDev_Main).getAFTableIdxNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IAfMgr::getAFTable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFTable();

    return AfMgr::getInstance(ESensorDev_Main).getAFTable();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAfMgr::MoveLensTo(MINT32 i4SensorDev, MINT32 &i4TargetPos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).MoveLensTo(i4TargetPos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).MoveLensTo(i4TargetPos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).MoveLensTo(i4TargetPos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).MoveLensTo(i4TargetPos, AF_CMD_CALLER);

    return AfMgr::getInstance(ESensorDev_Main).MoveLensTo(i4TargetPos, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setMFPos(MINT32 i4SensorDev, MINT32 a_i4Pos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setMFPos(a_i4Pos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setMFPos(a_i4Pos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setMFPos(a_i4Pos, AF_CMD_CALLER);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setMFPos(a_i4Pos, AF_CMD_CALLER);

    return AfMgr::getInstance(ESensorDev_Main).setMFPos(a_i4Pos, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFullScanstep(MINT32 i4SensorDev, MINT32 a_i4Step)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setFullScanstep(a_i4Step);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setFullScanstep(a_i4Step);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setFullScanstep(a_i4Step);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setFullScanstep(a_i4Step);

    return AfMgr::getInstance(ESensorDev_Main).setFullScanstep(a_i4Step);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T IAfMgr::getFLKStat(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFLKStat();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFLKStat();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFLKStat();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getFLKStat();

    return AfMgr::getInstance(ESensorDev_Main).getFLKStat();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID IAfMgr::setAE2AFInfo(MINT32 i4SensorDev,AE2AFInfo_T rAEInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAE2AFInfo(rAEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAE2AFInfo(rAEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAE2AFInfo(rAEInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setAE2AFInfo(rAEInfo);

    return AfMgr::getInstance(ESensorDev_Main).setAE2AFInfo(rAEInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAdptCompInfo(MINT32 i4SensorDev, const AdptCompTimeData_T &AdptCompTime)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAdptCompInfo(AdptCompTime);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAdptCompInfo(AdptCompTime);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAdptCompInfo(AdptCompTime);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setAdptCompInfo(AdptCompTime);

    return AfMgr::getInstance(ESensorDev_Main).setAdptCompInfo(AdptCompTime);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setIspSensorInfo2AF(MINT32 i4SensorDev, ISP_SENSOR_INFO_T ispSensorInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setIspSensorInfo2AF(ispSensorInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setIspSensorInfo2AF(ispSensorInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setIspSensorInfo2AF(ispSensorInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setIspSensorInfo2AF(ispSensorInfo);

    return AfMgr::getInstance(ESensorDev_Main).setIspSensorInfo2AF(ispSensorInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetAETargetMode(MINT32 i4SensorDev, MINT32 eAETargetMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SetAETargetMode((eAETargetMODE)eAETargetMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SetAETargetMode((eAETargetMODE)eAETargetMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SetAETargetMode((eAETargetMODE)eAETargetMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SetAETargetMode((eAETargetMODE)eAETargetMode);

    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::getAFRefWin(MINT32 i4SensorDev, CameraArea_T &rWinSize)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFRefWin(rWinSize);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFRefWin(rWinSize);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFRefWin(rWinSize);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAFRefWin(rWinSize);

    return AfMgr::getInstance(ESensorDev_Main).getAFRefWin(rWinSize);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::doAF(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).doAF(pAFStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).doAF(pAFStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).doAF(pAFStatBuf);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).doAF(pAFStatBuf);

    return AfMgr::getInstance(ESensorDev_Main).doAF(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::passAFBuffer(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).passAFBuffer(pAFStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).passAFBuffer(pAFStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).passAFBuffer(pAFStatBuf);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).passAFBuffer(pAFStatBuf);

    return AfMgr::getInstance(ESensorDev_Main).passAFBuffer(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::passPDBuffer(MINT32 i4SensorDev, MVOID *ptrInPDData)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).passPDBuffer(ptrInPDData);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).passPDBuffer(ptrInPDData);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).passPDBuffer(ptrInPDData);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).passPDBuffer(ptrInPDData);

    return AfMgr::getInstance(ESensorDev_Main).passPDBuffer(ptrInPDData);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::doPDBuffer(MINT32 i4SensorDev, MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride, MUINT32 i4FrmNum)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).doPDBuffer(buffer, w,h, stride, i4FrmNum);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).doPDBuffer(buffer, w,h, stride, i4FrmNum);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).doPDBuffer(buffer, w,h, stride, i4FrmNum);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).doPDBuffer(buffer, w,h, stride, i4FrmNum);

    return AfMgr::getInstance(ESensorDev_Main).doPDBuffer(buffer, w,h, stride, i4FrmNum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::PDPureRawInterval(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).PDPureRawInterval();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).PDPureRawInterval();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).PDPureRawInterval();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).PDPureRawInterval();

    return AfMgr::getInstance(ESensorDev_Main).PDPureRawInterval();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::enablePBIN(MINT32 i4SensorDev, MBOOL bEnable)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).enablePBIN(bEnable);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).enablePBIN(bEnable);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).enablePBIN(bEnable);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).enablePBIN(bEnable);

    return AfMgr::getInstance(ESensorDev_Main).enablePBIN(bEnable);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setPdSeparateMode(MINT32 i4SensorDev, MUINT8 i4SMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setPdSeparateMode(i4SMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setPdSeparateMode(i4SMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setPdSeparateMode(i4SMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setPdSeparateMode(i4SMode);

    return AfMgr::getInstance(ESensorDev_Main).setPdSeparateMode(i4SMode);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getPdInfoForSttCtrl(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 i4SensorMode,
        MUINT32 &u4PDOSizeW, MUINT32 &u4PDOSizeH, FEATURE_PDAF_STATUS &PDAFStatus)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);

    return AfMgr::getInstance(ESensorDev_Main).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocusFinish(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isFocusFinish();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isFocusFinish();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isFocusFinish();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).isFocusFinish();

    return AfMgr::getInstance(ESensorDev_Main).isFocusFinish();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocused(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isFocused();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isFocused();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isFocused();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).isFocused();

    return AfMgr::getInstance(ESensorDev_Main).isFocused();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isLockAE(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isLockAE();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isLockAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isLockAE();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).isLockAE();

    return AfMgr::getInstance(ESensorDev_Main).isLockAE();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getDebugInfo(MINT32 i4SensorDev, AF_DEBUG_INFO_T &rAFDebugInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getDebugInfo(rAFDebugInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAFDebugInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAFDebugInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getDebugInfo(rAFDebugInfo);

    return AfMgr::getInstance(ESensorDev_Main).getDebugInfo(rAFDebugInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setBestShotConfig(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setBestShotConfig();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setBestShotConfig();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setBestShotConfig();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setBestShotConfig();

    return AfMgr::getInstance(ESensorDev_Main).setBestShotConfig();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::calBestShotValue(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).calBestShotValue(pAFStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).calBestShotValue(pAFStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).calBestShotValue(pAFStatBuf);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).calBestShotValue(pAFStatBuf);

    return AfMgr::getInstance(ESensorDev_Main).calBestShotValue(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAfMgr::getBestShotValue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getBestShotValue();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getBestShotValue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getBestShotValue();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getBestShotValue();

    return AfMgr::getInstance(ESensorDev_Main).getBestShotValue();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setSGGPGN(MINT32 i4SensorDev, MINT32 i4SGG_PGAIN)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setSGGPGN(i4SGG_PGAIN);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setSGGPGN(i4SGG_PGAIN);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setSGGPGN(i4SGG_PGAIN);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).setSGGPGN(i4SGG_PGAIN);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::autoFocus(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).autoFocus();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).autoFocus();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).autoFocus();
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).autoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::cancelAutoFocus(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).cancelAutoFocus();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).cancelAutoFocus();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).cancelAutoFocus();
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).cancelAutoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::TimeOutHandle(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).TimeOutHandle();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).TimeOutHandle();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).TimeOutHandle();
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).TimeOutHandle();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MUINT32 i4BINInfo_SzW, MUINT32 i4BINInfo_SzH)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AfMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode, i4BINInfo_SzW, i4BINInfo_SzH);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AfMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode, i4BINInfo_SzW, i4BINInfo_SzH);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AfMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode, i4BINInfo_SzW, i4BINInfo_SzH);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_main2 = AfMgr::getInstance(ESensorDev_SubSecond).setSensorMode(i4NewSensorMode, i4BINInfo_SzW, i4BINInfo_SzH);

    return (ret_main | ret_sub | ret_main2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::SetCurFrmNum(MINT32 i4SensorDev, MUINT32 u4FrmNum, MUINT32 u4FrmNumCur)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).SetCurFrmNum(u4FrmNum, u4FrmNumCur);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).SetCurFrmNum(u4FrmNum, u4FrmNumCur);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).SetCurFrmNum(u4FrmNum, u4FrmNumCur);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).SetCurFrmNum(u4FrmNum, u4FrmNumCur);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getDAFtbl(MINT32 i4SensorDev, MVOID ** ptbl)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getDAFtbl(ptbl);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getDAFtbl(ptbl);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getDAFtbl(ptbl);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getDAFtbl(ptbl);

    return AfMgr::getInstance(ESensorDev_Main).getDAFtbl(ptbl);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::setMultiZoneEnable( MINT32 i4SensorDev, MUINT8 bEn)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setMultiZoneEnable( bEn);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setMultiZoneEnable( bEn);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setMultiZoneEnable( bEn);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).setMultiZoneEnable( bEn);

    return AfMgr::getInstance(ESensorDev_Main).setMultiZoneEnable( bEn);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::updateSensorListenerParams(MINT32 i4SensorDev, MINT32 *i4SensorInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).updateSensorListenerParams(i4SensorInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).updateSensorListenerParams(i4SensorInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAfMgr::sendAFCtrl(MINT32 i4SensorDev, MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).sendAFCtrl(eAFCtrl, iArg1, iArg2);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).sendAFCtrl(eAFCtrl, iArg1, iArg2);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).sendAFCtrl(eAFCtrl, iArg1, iArg2);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).sendAFCtrl(eAFCtrl, iArg1, iArg2);
    return AfMgr::getInstance(ESensorDev_Main).sendAFCtrl(eAFCtrl, iArg1, iArg2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Camera 3.0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAperture(MINT32 i4SensorDev, MFLOAT lens_aperture)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setAperture(lens_aperture);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setAperture(lens_aperture);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setAperture(lens_aperture);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).setAperture(lens_aperture);
}

MFLOAT IAfMgr::getAperture(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAperture();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAperture();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAperture();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getAperture();

    return AfMgr::getInstance(ESensorDev_Main).getAperture();
}
MVOID IAfMgr::setFocalLength(MINT32 i4SensorDev, MFLOAT lens_focalLength)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setFocalLength(lens_focalLength);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setFocalLength(lens_focalLength);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setFocalLength(lens_focalLength);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).setFocalLength(lens_focalLength);

}

MFLOAT IAfMgr::getFocalLength(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocalLength();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocalLength();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocalLength();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getFocalLength();

    return AfMgr::getInstance(ESensorDev_Main).getFocalLength();
}

MVOID IAfMgr::setFocusDistance(MINT32 i4SensorDev, MFLOAT lens_focusDistance)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setFocusDistance(lens_focusDistance);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setFocusDistance(lens_focusDistance);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setFocusDistance(lens_focusDistance);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).setFocusDistance(lens_focusDistance);
}

MFLOAT IAfMgr::getFocusDistance(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocusDistance();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocusDistance();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocusDistance();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getFocusDistance();

    return AfMgr::getInstance(ESensorDev_Main).getFocusDistance();
}

MVOID IAfMgr::setOpticalStabilizationMode(MINT32 i4SensorDev, MINT32 ois_OnOff)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setOpticalStabilizationMode(ois_OnOff);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setOpticalStabilizationMode(ois_OnOff);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setOpticalStabilizationMode(ois_OnOff);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).setOpticalStabilizationMode(ois_OnOff);
}

MINT32 IAfMgr::getOpticalStabilizationMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getOpticalStabilizationMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getOpticalStabilizationMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getOpticalStabilizationMode();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getOpticalStabilizationMode();

    return AfMgr::getInstance(ESensorDev_Main).getOpticalStabilizationMode();
}

MVOID IAfMgr::getFocusRange(MINT32 i4SensorDev, MFLOAT *vnear, MFLOAT *vfar)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).getFocusRange(vnear,vfar);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).getFocusRange(vnear,vfar);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).getFocusRange(vnear,vfar);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).getFocusRange(vnear,vfar);
}
MINT32 IAfMgr::getLensState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getLensState();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getLensState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getLensState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).getLensState();

    return AfMgr::getInstance(ESensorDev_Main).getLensState();
}

MBOOL IAfMgr::Stop(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).Stop();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).Stop();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).Stop();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).Stop();

    return AfMgr::getInstance(ESensorDev_Main).Stop();
}


MBOOL IAfMgr::Start(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).Start();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).Start();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).Start();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).Start();

    return AfMgr::getInstance(ESensorDev_Main).Start();
}

MBOOL IAfMgr::CamPwrOnState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CamPwrOnState();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CamPwrOnState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CamPwrOnState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CamPwrOnState();

    return AfMgr::getInstance(ESensorDev_Main).CamPwrOnState();
}

MBOOL IAfMgr::CamPwrOffState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CamPwrOffState();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CamPwrOffState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CamPwrOffState();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CamPwrOffState();

    return AfMgr::getInstance(ESensorDev_Main).CamPwrOffState();
}

MBOOL IAfMgr::AFThreadStart(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).AFThreadStart();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).AFThreadStart();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).AFThreadStart();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).AFThreadStart();

    return AfMgr::getInstance(ESensorDev_Main).AFThreadStart();
}

MBOOL IAfMgr::VsyncUpdate(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).VsyncUpdate();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).VsyncUpdate();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).VsyncUpdate();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).VsyncUpdate();

    return AfMgr::getInstance(ESensorDev_Main).VsyncUpdate();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameinit(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameinit(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTMCUNameinit(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTMCUNameinit(i4SensorIdx);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTMCUNameinit(i4SensorIdx);

    return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameinit(i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameuninit(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameuninit();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTMCUNameuninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTMCUNameuninit();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTMCUNameuninit();

    return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameuninit();
}
MINT32 IAfMgr::CCTOPAFOpeartion(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFOpeartion();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFOpeartion();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFOpeartion();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFOpeartion();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFOpeartion();
}
MINT32 IAfMgr::CCTOPMFOpeartion(MINT32 i4SensorDev,MINT32 a_i4MFpos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPMFOpeartion(a_i4MFpos);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPMFOpeartion(a_i4MFpos);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPMFOpeartion(a_i4MFpos);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPMFOpeartion(a_i4MFpos);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPMFOpeartion(a_i4MFpos);
}
MINT32 IAfMgr::CCTOPAFGetAFInfo(MINT32 i4SensorDev,MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetBestPos(MINT32 i4SensorDev,MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFCaliOperation(MINT32 i4SensorDev,MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFSetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetFocusRange(a_pFocusRange);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFSetFocusRange(a_pFocusRange);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFSetFocusRange(a_pFocusRange);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFSetFocusRange(a_pFocusRange);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetFocusRange(a_pFocusRange);
}
MINT32 IAfMgr::CCTOPCheckAutoFocusDone(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPCheckAutoFocusDone();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPCheckAutoFocusDone();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPCheckAutoFocusDone();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPCheckAutoFocusDone();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPCheckAutoFocusDone();
}
MINT32 IAfMgr::CCTOPAFGetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFApplyNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);
}
MINT32 IAfMgr::CCTOPAFSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFSaveNVRAMParam();


    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSaveNVRAMParam();
}
MINT32 IAfMgr::CCTOPAFGetFV(MINT32 i4SensorDev,MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFEnable();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFEnable();
}
MINT32 IAfMgr::CCTOPAFDisable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFDisable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFDisable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFDisable();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFDisable();
}
MINT32 IAfMgr::CCTOPAFGetEnableInfo(MINT32 i4SensorDev,MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
}
MRESULT IAfMgr::CCTOPAFSetAfArea(MINT32 i4SensorDev,MUINT32 a_iPercent)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetAfArea(a_iPercent);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFSetAfArea(a_iPercent);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFSetAfArea(a_iPercent);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).CCTOPAFSetAfArea(a_iPercent);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetAfArea(a_iPercent);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    AF Sync
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::SyncAFReadDatabase(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SyncAFReadDatabase();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SyncAFReadDatabase();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SyncAFReadDatabase();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SyncAFReadDatabase();

    return AfMgr::getInstance(ESensorDev_Main).SyncAFReadDatabase();
}
MVOID IAfMgr::SyncAFWriteDatabase(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SyncAFWriteDatabase();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SyncAFWriteDatabase();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SyncAFWriteDatabase();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SyncAFWriteDatabase();

    return AfMgr::getInstance(ESensorDev_Main).SyncAFWriteDatabase();
}
MVOID IAfMgr::SyncAFSetMode(MINT32 i4SensorDev, MINT32 a_i4SyncMode)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).SyncAFSetMode(a_i4SyncMode);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).SyncAFSetMode(a_i4SyncMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).SyncAFSetMode(a_i4SyncMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).SyncAFSetMode(a_i4SyncMode);
}
MVOID IAfMgr::SyncAFGetMotorRange(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).SyncAFGetMotorRange(sCamInfo);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).SyncAFGetMotorRange(sCamInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).SyncAFGetMotorRange(sCamInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).SyncAFGetMotorRange(sCamInfo);
}
MINT32 IAfMgr::SyncAFGetInfo(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SyncAFGetInfo(sCamInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SyncAFGetInfo(sCamInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SyncAFGetInfo(sCamInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SyncAFGetInfo(sCamInfo);

    return AfMgr::getInstance(ESensorDev_Main).SyncAFGetInfo(sCamInfo);
}
MVOID IAfMgr::SyncAFSetInfo(MINT32 i4SensorDev, MINT32 a_i4Pos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SyncAFSetInfo(a_i4Pos);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SyncAFSetInfo(a_i4Pos);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SyncAFSetInfo(a_i4Pos);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AfMgr::getInstance(ESensorDev_SubSecond).SyncAFSetInfo(a_i4Pos);

    return AfMgr::getInstance(ESensorDev_Main).SyncAFSetInfo(a_i4Pos);
}
MVOID IAfMgr::SyncAFGetCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).SyncAFGetCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).SyncAFGetCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).SyncAFGetCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).SyncAFGetCalibPos(sCamInfo);
}
MVOID IAfMgr::SyncAFCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).SyncAFCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).SyncAFCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).SyncAFCalibPos(sCamInfo);
    if (i4SensorDev & ESensorDev_SubSecond)
        AfMgr::getInstance(ESensorDev_SubSecond).SyncAFCalibPos(sCamInfo);
}
