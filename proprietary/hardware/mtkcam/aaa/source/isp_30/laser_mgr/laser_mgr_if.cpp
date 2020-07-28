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
#define LOG_TAG "laser_mgr_if"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>
#include <cutils/properties.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>

#include <kd_camera_feature.h>
#include <private/aaa_hal_private.h>

#include <af_feature.h>
class NvramDrvBase;
using namespace android;


#include "laser_mgr_if.h"
#include <mtkcam/aaa/drv/laser_drv.h>


using namespace NS3Av3;
static  ILaserMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ILaserMgr& ILaserMgr::getInstance()
{
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT ILaserMgr::init(MINT32 i4SensorDev)
{
    CAM_LOGD("ILaserMgr::init dev %d", i4SensorDev);

    if (i4SensorDev & 0x01)
        return (MRESULT)(LaserDrv::getInstance().init());
    else
        return (MRESULT)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT ILaserMgr::uninit(MINT32 i4SensorDev)
{
    CAM_LOGD("ILaserMgr::uninit dev %d", i4SensorDev);

    if (i4SensorDev & 0x01)
        return (MRESULT)(LaserDrv::getInstance().uninit());
    else
        return (MRESULT)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::checkHwSetting(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().checkHwSetting();
    else
        return (int)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void ILaserMgr::setLaserGoldenTable(MINT32 i4SensorDev, unsigned int *TableValue, int LaserTableNum, int LaserMaxDistance)
{
    if (i4SensorDev & 0x01)
        LaserDrv::getInstance().setLaserGoldenTable(TableValue, LaserTableNum, LaserMaxDistance);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void ILaserMgr::setLensCalibrationData(MINT32 i4SensorDev, int LensDAC_10cm, int LensDAC_50cm)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().setLensCalibrationData(LensDAC_10cm, LensDAC_50cm);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void ILaserMgr::setLaserCalibrationData(MINT32 i4SensorDev, unsigned int OffsetData, unsigned int XTalkData)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().setLaserCalibrationData(OffsetData, XTalkData);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::getLaserCurDist(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserCurDist();
    else
        return (int)NULL;
}
int ILaserMgr::getLaserCurDac(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserCurDac();
    else
        return (int)NULL;
}
int ILaserMgr::getLaserCurStatus(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserCurStatus();
    else
        return (int)NULL;
}
int ILaserMgr::getLaserCurDMAX(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserCurDMAX();
    else
        return (int)NULL;
}
int ILaserMgr::predictAFStartPosDac(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().predictAFStartPosDac();
    else
        return (int)NULL;
}
int ILaserMgr::predictAFStartPosDist(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().predictAFStartPosDist();
    else
        return (int)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::getLaserOffsetCalib(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserOffsetCalib();
    else
        return (int)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::setLaserOffsetCalib(MINT32 i4SensorDev, int Value)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().setLaserOffsetCalib(Value);
    else
        return (int)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::getLaserXTalkCalib(MINT32 i4SensorDev)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().getLaserXTalkCalib();
    else
        return (int)NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ILaserMgr::setLaserXTalkCalib(MINT32 i4SensorDev, int Value)
{
    if (i4SensorDev & 0x01)
        return LaserDrv::getInstance().setLaserXTalkCalib(Value);
    else
        return (int)NULL;
}





