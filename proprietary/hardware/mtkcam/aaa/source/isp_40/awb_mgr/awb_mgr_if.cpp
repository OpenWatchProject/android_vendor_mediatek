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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam, IAWBInitPara* initPara)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).cameraPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).cameraPreviewInit(i4SensorIdx, rParam, initPara);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
camcorderPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam, IAWBInitPara* initPara)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).camcorderPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).camcorderPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).camcorderPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).camcorderPreviewInit(i4SensorIdx, rParam, initPara);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).camcorderPreviewInit(i4SensorIdx, rParam, initPara);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraCaptureInit(MINT32 i4SensorDev, IAWBInitPara* initPara)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraCaptureInit(initPara);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraCaptureInit(initPara);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraCaptureInit(initPara);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).cameraCaptureInit(initPara);
	if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).cameraCaptureInit(initPara);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraPreviewReinit(MINT32 i4SensorDev, IAWBInitPara* initPara)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraPreviewReinit(initPara);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraPreviewReinit(initPara);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraPreviewReinit(initPara);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).cameraPreviewReinit(initPara);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).cameraPreviewReinit(initPara);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}


MBOOL
IAwbMgr::
start(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).start();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).start();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).start();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).start();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).start();

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

MBOOL
IAwbMgr::
stop(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).stop();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).stop();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).stop();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).stop();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).stop();

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}


MBOOL
IAwbMgr::
init(MINT32 i4SensorDev, MINT32 i4SensorIdx, IAWBInitPara* initPara)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).init(i4SensorIdx, initPara);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).init(i4SensorIdx, initPara);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).init(i4SensorIdx, initPara);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).init(i4SensorIdx, initPara);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).init(i4SensorIdx, initPara);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
uninit(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).uninit();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).uninit();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).uninit();

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
isAWBEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).isAWBEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).isAWBEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).isAWBEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).isAWBEnable();
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).isAWBEnable();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBMode(MINT32 i4SensorDev, MINT32 i4NewAWBMode)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 =  ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setAWBMode(i4NewAWBMode);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MINT32 i4BinWidth, MINT32 i4BinHeight, MINT32 i4QbinWidth, MINT32 i4QbinHeight)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setSensorMode(i4NewSensorMode, i4BinWidth, i4BinHeight, i4QbinWidth, i4QbinHeight);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getAWBMode();
    }

    CAM_LOGE("Err IAwbMgr::getAWBMode()\n");

    return 0;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setStrobeMode(MINT32 i4SensorDev, MINT32 i4NewStrobeMode)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setStrobeMode(i4NewStrobeMode);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getStrobeMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getStrobeMode();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getStrobeMode();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getStrobeMode();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getStrobeMode();
    }
	else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getStrobeMode();
    }
    CAM_LOGE("Err IAwbMgr::getStrobeMode()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setFlashAWBData(MINT32 i4SensorDev, FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setFlashAWBData(rFlashAwbData);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBLock(MINT32 i4SensorDev, MBOOL bAWBLock)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setAWBLock(bAWBLock);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
enableAWB(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).enableAWB();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).enableAWB();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).enableAWB();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).enableAWB();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).enableAWB();

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
disableAWB(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).disableAWB();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).disableAWB();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).disableAWB();
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).disableAWB();
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).disableAWB();

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
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
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPvAWB(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MUINT32 u4ExposureTime, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, u4ExposureTime, bApplyToHW);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doAFAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).doAFAWB(pAWBStatBuf, i4SceneLV, bApplyToHW);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPreCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doCapFlashAWBDisplay(MINT32 i4SensorDev, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doCapFlashAWBDisplay(bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doCapFlashAWBDisplay(bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doCapFlashAWBDisplay(bApplyToHW);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).doCapFlashAWBDisplay(bApplyToHW);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).doCapFlashAWBDisplay(bApplyToHW);

    return ret_main && ret_sub && ret_main2 && ret_sub2 && ret_main3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
applyAWB(MINT32 i4SensorDev, AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
	else if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    CAM_LOGE("Err IAwbMgr::applyAWB()\n");

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getDebugInfo(MINT32 i4SensorDev, AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    CAM_LOGE("Err IAwbMgr::getDebugInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBCCT(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getAWBCCT();
    }

    CAM_LOGE("Err IAwbMgr::getAWBCCT()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getASDInfo(MINT32 i4SensorDev, AWB_ASD_INFO_T &a_rAWBASDInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    CAM_LOGE("Err IAwbMgr::getASDInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getAWBOutput(MINT32 i4SensorDev, AWB_OUTPUT_T &a_rAWBOutput)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    CAM_LOGE("Err IAwbMgr::getAWBOutput()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getAWBGain(MINT32 i4SensorDev, AWB_GAIN_T &rAwbGain, MINT32& i4ScaleUnit) const
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getAWBGain(rAwbGain, i4ScaleUnit);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getAWBGain(rAwbGain, i4ScaleUnit);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getAWBGain(rAwbGain, i4ScaleUnit);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).getAWBGain(rAwbGain, i4ScaleUnit);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).getAWBGain(rAwbGain, i4ScaleUnit);
        return MTRUE;
    }

    CAM_LOGE("Err IAwbMgr::getAWBGain()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
getAWBParentStat(MINT32 i4SensorDev, AWB_PARENT_BLK_STAT_T &a_rAWBParentState, MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY )
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_SubSecond) {
        AwbMgr::getInstance(ESensorDev_SubSecond).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainThird) {
        AwbMgr::getInstance(ESensorDev_MainThird).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    return MFALSE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IAwbMgr::
setAFLV(MINT32 i4SensorDev, MINT32 i4AFLV)
{
    if (i4SensorDev & ESensorDev_Main)
        AwbMgr::getInstance(ESensorDev_Main).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_Sub)
        AwbMgr::getInstance(ESensorDev_Sub).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_MainSecond)
        AwbMgr::getInstance(ESensorDev_MainSecond).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_SubSecond)
        AwbMgr::getInstance(ESensorDev_SubSecond).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_MainThird)
        AwbMgr::getInstance(ESensorDev_MainThird).setAFLV(i4AFLV);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAFLV(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getAFLV();
    }
    CAM_LOGE("Err IAwbMgr::getAFLV()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*MBOOL
IAwbMgr::
getInitInputParam(MINT32 i4SensorDev, SYNC_CTC_INIT_PARAM &CCTInitParam, NORMAL_GAIN_INIT_INPUT_PARAM &NoramlGainInitParam)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    CAM_LOGE("Err IAwbMgr::getInitInputParam()\n");

    return MFALSE;
}*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setTGInfo(i4TGInfo);
    }
    CAM_LOGE("Err IAwbMgr::setTGInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setAWBNvramIdx(MINT32 const i4SensorDev, MINT32 const i4AWBNvramIdx)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setAWBNvramIdx(i4AWBNvramIdx);
    }
    CAM_LOGE("Err IAwbMgr::setAWBNvramIdx()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAwbMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 u4CamScenarioMode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = ret_main3 = S_AWB_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setCamScenarioMode(u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setCamScenarioMode(u4CamScenarioMode);

    return (ret_main | ret_sub | ret_main2 | ret_sub2 | ret_main3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::getColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT& fGain_R, MFLOAT& fGain_G, MFLOAT& fGain_B)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
	else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    CAM_LOGE("Err IAwbMgr::getColorCorrectionGain()\n");

    return MFALSE;
}

MBOOL
IAwbMgr::setIsMono(MINT32 const i4SensorDev, int bMono, MUINT32 i4SensorType)
{
	  if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setIsMono(bMono, i4SensorType);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setIsMono(bMono, i4SensorType);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setIsMono(bMono, i4SensorType);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).setIsMono(bMono, i4SensorType);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setIsMono(bMono, i4SensorType);
    }
    CAM_LOGE("Err IAwbMgr::setIsMono()\n");

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT fGain_R, MFLOAT fGain_G, MFLOAT fGain_B)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setColorCorrectionGain(fGain_R, fGain_G, fGain_B);
    }
    CAM_LOGE("Err IAwbMgr::setColorCorrectionGain()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::getAWBState(MINT32 const i4SensorDev, mtk_camera_metadata_enum_android_control_awb_state_t& eAWBState)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBState(eAWBState);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBState(eAWBState);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBState(eAWBState);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).getAWBState(eAWBState);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getAWBState(eAWBState);
    }
    CAM_LOGE("Err IAwbMgr::getAWBState()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Face detection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAwbMgr::setFDenable(MINT32 i4SensorDev, MBOOL bFDenable)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).setFDenable(bFDenable);
    CAM_LOGE("Err IAwbMgr::setFDenable()\n");

    return MFALSE;
}

MRESULT IAwbMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).setFDInfo(a_sFaces, i4tgwidth, i4tgheight);
    CAM_LOGE("Err IAwbMgr::setFDInfo()\n");

    return MFALSE;
}

MRESULT IAwbMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    CAM_LOGE("Err IAwbMgr::setZoomWinInfo()\n");

    return MFALSE;
}

MRESULT IAwbMgr::getFocusArea(MINT32 i4SensorDev, android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).getFocusArea(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).getFocusArea(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).getFocusArea(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).getFocusArea(vecOut, i4tgwidth, i4tgheight);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).getFocusArea(vecOut, i4tgwidth, i4tgheight);
    CAM_LOGE("Err IAwbMgr::getFocusArea()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::backup(MINT32 const i4SensorDev)
{
  if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).backup();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).backup();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).backup();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).backup();
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).backup();
    }
    CAM_LOGE("Error IAwbMgr::backup()\n");

    return MFALSE;

}
MBOOL IAwbMgr::restore(MINT32 const i4SensorDev)
{
  if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).restore();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).restore();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).restore();
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).restore();
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).restore();
    }
    CAM_LOGE("Error IAwbMgr::restore()\n");

    return MFALSE;

}

MBOOL IAwbMgr::SetAETargetMode(MINT32 const i4SensorDev, int mode) //for hdr
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).SetAETargetMode(mode);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).SetAETargetMode(mode);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).SetAETargetMode(mode);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).SetAETargetMode(mode);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).SetAETargetMode(mode);
    }
    CAM_LOGE("Err IAwbMgr::SetAETargetMode()\n");

    return MFALSE;

}

MBOOL
IAwbMgr::setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setColorCorrectionMode(i4ColorCorrectionMode);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setColorCorrectionMode(i4ColorCorrectionMode);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setColorCorrectionMode(i4ColorCorrectionMode);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
        return AwbMgr::getInstance(ESensorDev_SubSecond).setColorCorrectionMode(i4ColorCorrectionMode);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setColorCorrectionMode(i4ColorCorrectionMode);
    }
    CAM_LOGE("Err IAwbMgr::setColorCorrectionMode()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBEnable();
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBDisable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBDisable();

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBSetAWBGain(a_pAWBGain);
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 u4CamScenarioMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM, u4CamScenarioMode);
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, CAM_SCENARIO_T eIdx, MVOID *a_pAWBNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBSaveNVRAMParam();

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBMode(MINT32 i4SensorDev, MINT32 a_AWBMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBSetAWBMode(a_AWBMode);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBMode(MINT32 i4SensorDev, MINT32 *a_pAWBMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetLightProb(MINT32 i4SensorDev, MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBBypassCalibration(MINT32 i4SensorDev, MBOOL bBypassCalibration)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPAWBBypassCalibration(bBypassCalibration);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_SubSecond)
        return AwbMgr::getInstance(ESensorDev_SubSecond).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainThird)
        return AwbMgr::getInstance(ESensorDev_MainThird).CCTOPFlashAWBSaveNVRAMParam();
   return E_AWB_PARAMETER_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::setAAOMode(MINT32 i4SensorDev, MUINT32 u4AWBAAOmode)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAAOMode(u4AWBAAOmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAAOMode(u4AWBAAOmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAAOMode(u4AWBAAOmode);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).setAAOMode(u4AWBAAOmode);	
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).setAAOMode(u4AWBAAOmode);
      return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAwbMgr::getAAOConfig(MINT32 i4SensorDev, AWB_AAO_CONFIG_Param_T &rAWBConfig)
{
    MINT32 ret_main, ret_sub, ret_main2, ret_sub2, ret_main3;
    ret_main = ret_sub = ret_main2 = ret_sub2 = S_AE_OK;


    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).getAAOConfig(rAWBConfig);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).getAAOConfig(rAWBConfig);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).getAAOConfig(rAWBConfig);
    if (i4SensorDev & ESensorDev_SubSecond)
        ret_sub2 = AwbMgr::getInstance(ESensorDev_SubSecond).getAAOConfig(rAWBConfig);
    if (i4SensorDev & ESensorDev_MainThird)
        ret_main3 = AwbMgr::getInstance(ESensorDev_MainThird).getAAOConfig(rAWBConfig);

      return MFALSE;
}
MBOOL IAwbMgr::setMWBColorTemperature(MINT32 i4SensorDev, int colorTemperature)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setMWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setMWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setMWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).setMWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).setMWBColorTemperature(colorTemperature);
    }

    CAM_LOGE("Err IAwbMgr::setMWBColorTemperature()\n");

    return 0;
}
MBOOL IAwbMgr::getSupportMWBColorTemperature(MINT32 i4SensorDev,MUINT32 &max, MUINT32 &min)
{

    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getSuppotMWBColorTemperature(max, min);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getSuppotMWBColorTemperature(max, min);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getSuppotMWBColorTemperature(max, min);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).getSuppotMWBColorTemperature(max, min);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getSuppotMWBColorTemperature(max, min);
    }

    CAM_LOGE("Err IAwbMgr::GetSupportMWBColorTemperature()\n");

    return 0;
}

MBOOL IAwbMgr::getAWBColorTemperature(MINT32 i4SensorDev, MUINT32 &colorTemperature)
{

    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).getAWBColorTemperature(colorTemperature);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).getAWBColorTemperature(colorTemperature);
    }

    CAM_LOGE("Err IAwbMgr::GetSupportMWBColorTemperature()\n");

    return 0;
}
MBOOL IAwbMgr::SetTorchMode(MINT32 i4SensorDev, MBOOL is_torch_now)
{

    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).SetTorchMode(is_torch_now);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).SetTorchMode(is_torch_now);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).SetTorchMode(is_torch_now);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).SetTorchMode(is_torch_now);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).SetTorchMode(is_torch_now);
    }

    CAM_LOGE("Err IAwbMgr::SetTorchMode()\n");

    return 0;
}

MBOOL IAwbMgr::SetMainFlashInfo(MINT32 i4SensorDev, MBOOL is_main_flash_on)
{

    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).SetMainFlashInfo(is_main_flash_on);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).SetMainFlashInfo(is_main_flash_on);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).SetMainFlashInfo(is_main_flash_on);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).SetMainFlashInfo(is_main_flash_on);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
        return AwbMgr::getInstance(ESensorDev_MainThird).SetMainFlashInfo(is_main_flash_on);
    }

    CAM_LOGE("Err IAwbMgr::SetMainFlashInfo()\n");

    return 0;
}

MBOOL IAwbMgr::setMaxFPS(MINT32 i4SensorDev, MINT32 i4MaxFPS)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setMaxFPS(i4MaxFPS);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setMaxFPS(i4MaxFPS);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setMaxFPS(i4MaxFPS);
    }
    else if (i4SensorDev & ESensorDev_SubSecond) {
       return AwbMgr::getInstance(ESensorDev_SubSecond).setMaxFPS(i4MaxFPS);
    }
    else if (i4SensorDev & ESensorDev_MainThird) {
       return AwbMgr::getInstance(ESensorDev_MainThird).setMaxFPS(i4MaxFPS);
    }

    CAM_LOGE("Err IAwbMgr::setMaxFPS \n");

    return 0;
}

