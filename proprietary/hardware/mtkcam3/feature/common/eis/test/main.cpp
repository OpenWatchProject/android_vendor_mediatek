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

#include <iostream>
#include <stdlib.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam3/feature/eis/eis_hal.h>
#include <camera_custom_eis.h>


#undef LOG_TAG
#define LOG_TAG "EisHal_UT"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_EIS_HAL);

using namespace std;
using namespace NSCam;

const MUINT32   gSensorIndex1 = 0;
const MUINT32   gSensorIndex2 = 1;

IHalSensor*     gpSensorHalObj1 = NULL;
IHalSensor*     gpSensorHalObj2 = NULL;


void prepareSensor()
{
    MUINT32 sensorArray[2] = {gSensorIndex1, gSensorIndex2};

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();

    gpSensorHalObj1 = pHalSensorList->createSensor(LOG_TAG, 1, &sensorArray[0]);
    gpSensorHalObj1->powerOn(LOG_TAG, 1, &sensorArray[0]);

    gpSensorHalObj2 = pHalSensorList->createSensor(LOG_TAG, 1, &sensorArray[1]);
    gpSensorHalObj2->powerOn(LOG_TAG, 1, &sensorArray[1]);

}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor()
{
    MUINT32 sensorArray[2] = {gSensorIndex1, gSensorIndex2};
    gpSensorHalObj1->powerOff(LOG_TAG, 1, &sensorArray[0]);
    gpSensorHalObj1->destroyInstance(LOG_TAG);
    gpSensorHalObj1 = NULL;

    gpSensorHalObj2->powerOff(LOG_TAG, 1, &sensorArray[1]);
    gpSensorHalObj2->destroyInstance(LOG_TAG);
    gpSensorHalObj2 = NULL;
}

MUINT32 warpMap[2][31][18];
/*
 * EIS UT
 *
 * usage: ./libeishal_test [<test_version>]
 * <test_version>: 0(default) # EIS 3.0
 *                 1           # EIS 2.5
 *
 * e.g. ./libeishal_test       # run EIS 3.0 UT
 *      ./libeishal_test 1     # run EIS 2.5 UT
 */
int main(int argc, char **argv)
{
    EisHal*   mpEisHal;
    MUINT32 mEisMode = 0,cropRatio;
    MINT32 i = 0;
    EIS_HAL_CONFIG_DATA eisHalConfigData;
    MSize sensorSize;
    MRect scalerCrop;
    MSize scalerOutSize;
    MINT32 PostProcOutWidth,PostProcOutHeight,expTime;
    IMAGE_BASED_DATA imgBaseData;
    MINT64 ts;

    // Arguments parsing
    int ver_num = 0;
    CAM_ULOGMD( "argc = %d\n", argc);
    for (i=0; i<argc; ++i)
    {
        CAM_ULOGMD( "argv[%d] = %s\n", i, argv[i]);
    }
    if( argc == 2 )
    {
        int tmp_num = atoi(argv[1]);
        ver_num = (tmp_num == 1 && tmp_num == 1) ? tmp_num : 0;
        CAM_ULOGMD("Use user-specifying test version = %d", ver_num);
    }
    else
    {
        CAM_ULOGMW("Unknown test version specifying, use default = 0");
    }

    prepareSensor();

    MUINT32 eisMode = EIS_MODE_OFF;
    if( ver_num == 1) // EIS 2.5 Gyro-based only
    {
        EIS_MODE_ENABLE_EIS_25(eisMode);
        EIS_MODE_ENABLE_EIS_GYRO(eisMode);
    }
    else  // EIS 3.0 Gyro-based only
    {
        EIS_MODE_ENABLE_EIS_30(eisMode);
        EIS_MODE_ENABLE_EIS_GYRO(eisMode);
    }

    mpEisHal = EisHal::CreateInstance(LOG_TAG, gSensorIndex1, MTRUE);
    if(EIS_RETURN_NO_ERROR != mpEisHal->Init())
    {
        CAM_ULOGME("mpEisHal init failed");
        return MFALSE;
    }

    cropRatio = 125;

    mpEisHal->AddMultiSensors(gSensorIndex2);

    sensorSize = MSize(2672,2008);
    scalerCrop = MRect(MPoint(0,252),MSize(2672,1503));
    scalerOutSize = MSize(2400,1350);

    PostProcOutWidth  = scalerOutSize.w;
    PostProcOutHeight = scalerOutSize.h;

    eisHalConfigData.sensor_Width = sensorSize.w;
    eisHalConfigData.sensor_Height = sensorSize.h;
    eisHalConfigData.rrz_crop_Width = scalerCrop.s.w;
    eisHalConfigData.rrz_crop_Height = scalerCrop.s.h;
    eisHalConfigData.rrz_crop_X = scalerCrop.p.x;
    eisHalConfigData.rrz_crop_Y = scalerCrop.p.y;
    eisHalConfigData.rrz_scale_Width = scalerOutSize.w;
    eisHalConfigData.rrz_scale_Height = scalerOutSize.h;

    eisHalConfigData.gpuTargetW = PostProcOutWidth*100.0/cropRatio;
    eisHalConfigData.gpuTargetH = PostProcOutHeight*100.0/cropRatio;

#define ROUND_TO_2X(x) ((x) & (~0x1))

    eisHalConfigData.gpuTargetW = ROUND_TO_2X(eisHalConfigData.gpuTargetW);
    eisHalConfigData.gpuTargetH = ROUND_TO_2X(eisHalConfigData.gpuTargetH);

#undef ROUND_TO_2X

    eisHalConfigData.imgiW = PostProcOutWidth;
    eisHalConfigData.imgiH = PostProcOutHeight;

    eisHalConfigData.crzOutW = PostProcOutWidth;
    eisHalConfigData.crzOutH = PostProcOutHeight;
    eisHalConfigData.srzOutW = PostProcOutWidth;
    eisHalConfigData.srzOutH = PostProcOutHeight;
    eisHalConfigData.feTargetW = PostProcOutWidth;
    eisHalConfigData.feTargetH = PostProcOutHeight;

    eisHalConfigData.cropX   = 0;
    eisHalConfigData.cropY   = 0;

    eisHalConfigData.rssoWidth  = 288;
    eisHalConfigData.rssoHeight = 162;
    eisHalConfigData.vHDREnabled = 0;
    mpEisHal->SetEisPlusWarpInfo((MINT32 *) &warpMap[0][0][0],
                                 (MINT32 *) &warpMap[1][0][0]);

    MUINT32 counter = 0;
    ts = 0;

    do
    {

        eisHalConfigData.sensorIdx = ((counter%2) == 0)? gSensorIndex1 : gSensorIndex2;

        if( ver_num == 1 )
        {
            if( EIS_RETURN_NO_ERROR != mpEisHal->ConfigFEFMEis(eisHalConfigData, eisMode) )
            {
                CAM_ULOGME("Test ConfigFFFMEis fail");
                return MFALSE;
            }
        }
        else
        {
            if( EIS_RETURN_NO_ERROR != mpEisHal->ConfigRSCMEEis(eisHalConfigData, eisMode) )
            {
                CAM_ULOGME("Test ConfigRSCMEEis fail");
                return MFALSE;
            }
        }

        eisHalConfigData.process_idx = 0;
        eisHalConfigData.process_mode = 0;

        //CAM_ULOGMD("ts: (%" PRId64")\n",ts);

        expTime = 20000;
        if( ver_num == 1 )
        {
            mpEisHal->DoFEFMEis(&eisHalConfigData, &imgBaseData, ts, expTime);
        }
        else
        {
            mpEisHal->DoRSCMEEis(&eisHalConfigData, &imgBaseData, ts, expTime, expTime);
        }
        ts += 33000000L;
        usleep(30000);
        counter++;
    }while (counter<10);


#if 0
    MUINT32 j,k;
    for (j= 0; j<mEisPlusGpuInfo.gridW; ++j)
    {
        for (k= 0; k<mEisPlusGpuInfo.gridH; ++k)
        {
            CAM_ULOGMD("X,Y [%u, %u]= ( %d,  %d)", j, k,
                                                warpMap[0][j][k],warpMap[1][j][k]);
        }
    }
#endif


    if (mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance("FeaturePipe_EisNode");
        mpEisHal = NULL;
    }

    closeSensor();

    CAM_ULOGMD( "EISHal test done\n");
    return 0;
}


