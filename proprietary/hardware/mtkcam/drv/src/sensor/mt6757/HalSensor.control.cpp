/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/HalSensor"
//
#include "MyUtils.h"
#include "sensor_drv.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif
#include <mtkcam/def/common.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <mtkcam/custom/ExifFactory.h>
// For property_get().
#include <cutils/properties.h>

/* 2-pixel: TG_CROP_W * TG_CROP_H * fps / (1-VB_percentage) ) > [isp clk rate*margin]*/
/* 4-pixel: TG_CROP_W * TG_CROP_H * fps / (1-VB_percentage) ) > 2 x [isp clk rate*margin] */
#define TWO_PIXEL_MODE_THRESHOLD (360000) /* widht*height*fps = 4000x3000x30 >= 450Mhz*0.8 */
/*DVFS, High:450Mhz, Medium:320Mhz*/
#define TWO_PIXEL_MODE_MARGIN (900)// 1000*9/10
//Deskew Function
#define ENABLE_CSI_AUTO_DEKSKEW 1
#define MAIN_CSI_AUTO_DESKEW_THRESHOLD 1427*1000000 /*5642x4224x240x10= 1427_374080(1.5Gbps)*/
#define SUB_CSI_AUTO_DESKEW_THRESHOLD 1427*1000000
#define CSI_BLANKING_RATIO 1000

//

extern SENSORDRV_INFO_STRUCT sensorDrvInfo[3];
extern SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[3];

extern MUINT32 meSensorDev;
extern MINT32 mPowerRefCount;

//
#ifdef MTK_SUB2_IMGSENSOR
SENSOR_CONFIG_STRUCT sensorPara[4];
SensorDynamicInfo sensorDynamicInfo[4];
#else
SENSOR_CONFIG_STRUCT sensorPara[3];
SensorDynamicInfo sensorDynamicInfo[3];
#endif
SENSOR_HAL_TEST_MODEL_STRUCT sensorTMPara;
SENSOR_VC_INFO_STRUCT gVCInfo;
static MUINT32 gPDAFMode;
//hwsync
#include <mtkcam/drv/IHwSyncDrv.h>
extern HWSyncDrv* mpHwSyncDrv;

/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#define ROUND_TO_2X(x) ((x) & (~0x1))

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HalSensor::querySensorDynamicInfo(
   MUINT32 sensorIdx,
   SensorDynamicInfo *pSensorDynamicInfo
)
{
   MBOOL ret = MFALSE;
    switch (sensorIdx) {
        case SENSOR_DEV_MAIN:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[0],sizeof(SensorDynamicInfo));
            break;
        case SENSOR_DEV_SUB:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[1],sizeof(SensorDynamicInfo));
            break;
        case SENSOR_DEV_MAIN_2:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[2],sizeof(SensorDynamicInfo));
            break;
#ifdef MTK_SUB2_IMGSENSOR
        case SENSOR_DEV_SUB_2:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[3],sizeof(SensorDynamicInfo));
            break;
#endif
        default:
            return ret;
            break;

    }
    ret = MTRUE;
    return ret;
}

MBOOL HalSensor::configure(
    MUINT const         uCountOfParam,
    ConfigParam const*  pArrayOfParam
)
{
    MBOOL ret = MFALSE;

    MINT32 pixelX0 = 0, pixelY0 = 0;
    MINT32 srcWidth = 0,srcHeight = 0;
    MUINT sensorScenarioId;
    MUINT32 inDataFmt = 0,data1 = 0, data2 = 0;
    SENSOR_DEV_ENUM eSensorDev=SENSOR_NONE;
    MUINT32 currSensorDev=0;
    MUINT32 currFPS=0;
    MINT idx=0;
#ifdef MTK_SUB2_IMGSENSOR
    ConfigParam halSensorIFParam[4];
#else
    ConfigParam halSensorIFParam[3];
#endif
    SENSOR_VC_INFO_STRUCT VcInfo;
    SensorDrv *const pSensorDrv = SensorDrv::get();
    SENSOR_DRIVER_SCENARIO_T scenarioconf;
#ifndef USING_MTK_LDVT
        //CPTLog(Event_Sensor_setScenario, CPTFlagStart);
#endif
   MUINT32 SW_deskewEnable = 0, HW_deskewEnable = 0;

    gPDAFMode = 0;

    Mutex::Autolock _l(muImgSensorDrv);

    //get property
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    if  ( 0 == mPowerRefCount )
    {
        MY_LOGW("Should powerOn before configure !!");
        goto lbExit;
    }



    for (MUINT i = 0; i < uCountOfParam; i++)
    {
        //  pArrayOfParam[i].index -> eSensorDev
        MUINT const sensorDev = HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfParam[i].index)->meSensorDev;

        switch(sensorDev) {
            case SENSOR_DEV_MAIN:
                idx = 0;
                break;
            case SENSOR_DEV_SUB:
                idx = 1;
                break;
            case SENSOR_DEV_MAIN_2:
                idx = 2;
                break;
#ifdef MTK_SUB2_IMGSENSOR
            case SENSOR_DEV_SUB_2:
                idx = 3;
                break;
#endif
            default:
                idx = 0;
                break;
        }

        halSensorIFParam[idx].crop              = pArrayOfParam[i].crop;
        halSensorIFParam[idx].scenarioId        = pArrayOfParam[i].scenarioId;
        halSensorIFParam[idx].isBypassScenario  = pArrayOfParam[i].isBypassScenario;
        halSensorIFParam[idx].isContinuous      = pArrayOfParam[i].isContinuous;
        halSensorIFParam[idx].HDRMode           = pArrayOfParam[i].HDRMode; // 0: no, 1: IHDR, 2:Merge mode HDR, 9:ZHDR
        halSensorIFParam[idx].framerate         = pArrayOfParam[i].framerate*10; // Unit : FPS , Driver Unit : 10*FPS
        halSensorIFParam[idx].twopixelOn        = 0; /* Don't support force enable two pixel mode*/
        halSensorIFParam[idx].debugMode         = pArrayOfParam[i].debugMode;

        currSensorDev |= sensorDev;
    }

    currFPS = halSensorIFParam[idx].framerate;
    //Test Mode use property parameter
    {
        property_get("vendor.debug.senif.hdrmode", value, "0");
        int hdrModeTest=atoi(value);
        if((hdrModeTest == 1)||(hdrModeTest == 2)||(hdrModeTest == 9))
        {
            halSensorIFParam[idx].HDRMode = hdrModeTest;
        }
    }
    //PDAFTest Mode use property parameter
    {
        property_get("vendor.debug.senif.pdafmode", value, "0");
        int PDAFModeTest=atoi(value);
        if(PDAFModeTest == 1)
        {
            gPDAFMode = 1;
        }
    }

    MY_LOGD("SenDev=%d, scenario=%d, HDR=%d, fps=%d, twopix=%d\n",currSensorDev, halSensorIFParam[idx].scenarioId,
        halSensorIFParam[idx].HDRMode,halSensorIFParam[idx].framerate,halSensorIFParam[idx].twopixelOn);

    // TM setting for debug mode 1
    sensorTMPara.TM_Vsync       = 16;
    sensorTMPara.TM_DummyPixel  = 16;
    sensorTMPara.TM_Line        = 4500;
    sensorTMPara.TM_Pixel       = 6500;
    sensorTMPara.TM_PAT         = 0;
    sensorTMPara.TM_FMT         = 0;


    if(currSensorDev & SENSOR_DEV_MAIN ) {

        if(halSensorIFParam[0].isBypassScenario != 0) {
            goto lbExit;
        }
        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[0].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }
        /*HDR get information*/
        if(halSensorIFParam[idx].HDRMode == 2)
        {
            data1 = halSensorIFParam[0].scenarioId;
            VcInfo.ModeSelect = 2;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }
        /*PDAF Virtual channel*/
        /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
        if((sensorDrvInfo[0].PDAF_Support == 2)||(sensorDrvInfo[0].PDAF_Support == 3)||(sensorDrvInfo[0].PDAF_Support == 5))
        {
            MUINT32 isPDAFsupport = 0;
            data1 = halSensorIFParam[0].scenarioId;
            pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&data1, (MUINTPTR)&isPDAFsupport);

            MY_LOGD("PDAF=%d,isPDAFsupport=%d\n",sensorDrvInfo[0].PDAF_Support,isPDAFsupport);
            if(isPDAFsupport)
                gPDAFMode = 1;
        }
        if(gPDAFMode == 1)
        {
            data1 = halSensorIFParam[0].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }
        /*HDR + PDAF Mixed mode*/
        if((halSensorIFParam[idx].HDRMode == 2) && (gPDAFMode == 1))
        {
            data1 = halSensorIFParam[0].scenarioId;
            VcInfo.ModeSelect = 2;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
            MY_LOGD("PDAF and HDR mix mode\n");
        }
        /*Get line / frame number CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM*/
        switch(halSensorIFParam[0].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[0].u1MIPIDataSettleDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }

        sensorPara[0].scenarioId = halSensorIFParam[0].scenarioId;
        sensorPara[0].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[0].isBypassScenario;
        sensorPara[0].u1IsContinuous = (MUINT8)halSensorIFParam[0].isContinuous;
        sensorPara[0].u1MIPIDataTermDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[0].u1MIPIDataSettleDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[0].u1MIPIClkTermDelay = sensorDrvInfo[0].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[0].u1MIPILaneNum = sensorDrvInfo[0].SensorMIPILaneNumber;
        sensorPara[0].u1MIPIPacketECCOrder = sensorDrvInfo[0].SensorPacketECCOrder;
        sensorPara[0].MIPI_PHY_TYPE = (MUINT)sensorDrvInfo[0].MIPIsensorType;
        sensorPara[0].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[0].SettleDelayMode;
        sensorPara[0].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[0].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[0].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[0].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[0].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[0].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[0].SCAM_DataNumber = (MUINT)sensorDrvInfo[0].SCAM_DataNumber;
        sensorPara[0].SCAM_DDR_En = (MUINT)sensorDrvInfo[0].SCAM_DDR_En;
        sensorPara[0].SCAM_CLK_INV = (MUINT)sensorDrvInfo[0].SCAM_CLK_INV;
        sensorPara[0].PDAFMode = gPDAFMode;
        sensorPara[0].SCAM_DEFAULT_DELAY = (MUINT)sensorDrvInfo[0].SCAM_DEFAULT_DELAY;
        sensorPara[0].SCAM_CRC_En = (MUINT)sensorDrvInfo[0].SCAM_CRC_En;
        sensorPara[0].SCAM_SOF_src = (MUINT)sensorDrvInfo[0].SCAM_SOF_src;
        sensorPara[0].SCAM_Timout_Cali = (MUINT)sensorDrvInfo[0].SCAM_Timout_Cali;
        sensorPara[0].SensorMIPIDeskew = 0;
     }


    if(currSensorDev & SENSOR_DEV_SUB ) {
        if(halSensorIFParam[1].isBypassScenario != 0) {
            goto lbExit;
        }

        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[1].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }
        if(halSensorIFParam[idx].HDRMode == 2)
        {
            data1 = halSensorIFParam[1].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }

        /*PDAF Virtual channel*/
        /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
        if((sensorDrvInfo[1].PDAF_Support == 2)||(sensorDrvInfo[1].PDAF_Support == 3)||(sensorDrvInfo[1].PDAF_Support == 5))
        {
            MUINT32 isPDAFsupport = 0;
            data1 = halSensorIFParam[1].scenarioId;
            pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&data1, (MUINTPTR)&isPDAFsupport);

            MY_LOGD("PDAF=%d,isPDAFsupport=%d\n",sensorDrvInfo[1].PDAF_Support,isPDAFsupport);
            if(isPDAFsupport)
                gPDAFMode = 1;
        }
        if(gPDAFMode == 1)
        {
            data1 = halSensorIFParam[1].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }
        /*HDR + PDAF Mixed mode*/
        if((halSensorIFParam[idx].HDRMode == 2) && (gPDAFMode == 1))
        {
            data1 = halSensorIFParam[0].scenarioId;
            VcInfo.ModeSelect = 2;
            ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
            MY_LOGD("PDAF and HDR mix mode\n");
        }

        switch(halSensorIFParam[1].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[1].u1MIPIDataSettleDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }
        sensorPara[1].scenarioId = halSensorIFParam[1].scenarioId;
        sensorPara[1].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[1].isBypassScenario;
        sensorPara[1].u1IsContinuous = (MUINT8)halSensorIFParam[1].isContinuous;
        sensorPara[1].u1MIPIDataTermDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[1].u1MIPIDataSettleDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[1].u1MIPIClkTermDelay = sensorDrvInfo[1].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[1].u1MIPILaneNum = sensorDrvInfo[1].SensorMIPILaneNumber;
        sensorPara[1].u1MIPIPacketECCOrder = sensorDrvInfo[1].SensorPacketECCOrder;
        sensorPara[1].MIPI_PHY_TYPE = (MUINT)sensorDrvInfo[1].MIPIsensorType;
        sensorPara[1].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[1].SettleDelayMode;
        sensorPara[1].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[1].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[1].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[1].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[1].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[1].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[1].SCAM_DataNumber = (MUINT)sensorDrvInfo[1].SCAM_DataNumber;
        sensorPara[1].SCAM_DDR_En = (MUINT)sensorDrvInfo[1].SCAM_DDR_En;
        sensorPara[1].SCAM_CLK_INV = (MUINT)sensorDrvInfo[1].SCAM_CLK_INV;
        sensorPara[1].PDAFMode = gPDAFMode;
        sensorPara[1].SCAM_DEFAULT_DELAY = (MUINT)sensorDrvInfo[1].SCAM_DEFAULT_DELAY;
        sensorPara[1].SCAM_CRC_En = (MUINT)sensorDrvInfo[1].SCAM_CRC_En;
        sensorPara[1].SCAM_SOF_src = (MUINT)sensorDrvInfo[1].SCAM_SOF_src;
        sensorPara[1].SCAM_Timout_Cali = (MUINT)sensorDrvInfo[1].SCAM_Timout_Cali;
        sensorPara[1].SensorMIPIDeskew = 0;
    }

    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        if(halSensorIFParam[2].isBypassScenario != 0) {
            goto lbExit;
        }

        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[2].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }

        if(halSensorIFParam[idx].HDRMode == 2)
        {
            data1 = halSensorIFParam[2].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }

        /*PDAF Virtual channel*/
        /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
        if((sensorDrvInfo[2].PDAF_Support == 2)||(sensorDrvInfo[2].PDAF_Support == 3)||(sensorDrvInfo[2].PDAF_Support == 5))
        {
            MUINT32 isPDAFsupport = 0;
            data1 = halSensorIFParam[2].scenarioId;
            pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&data1, (MUINTPTR)&isPDAFsupport);

            MY_LOGD("PDAF=%d,isPDAFsupport=%d\n",sensorDrvInfo[2].PDAF_Support,isPDAFsupport);
            if(isPDAFsupport)
                gPDAFMode = 1;
        }
        if(gPDAFMode == 1)
        {
            data1 = halSensorIFParam[2].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }
        /*HDR + PDAF Mixed mode*/
        if((halSensorIFParam[idx].HDRMode == 2) && (gPDAFMode == 1))
        {
            data1 = halSensorIFParam[0].scenarioId;
            VcInfo.ModeSelect = 2;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
            MY_LOGD("PDAF and HDR mix mode\n");
        }
        switch(halSensorIFParam[2].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }
        sensorPara[2].scenarioId = halSensorIFParam[2].scenarioId;
        sensorPara[2].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[2].isBypassScenario;
        sensorPara[2].u1IsContinuous = (MUINT8)halSensorIFParam[2].isContinuous;
        sensorPara[2].u1MIPIDataTermDelay = sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[2].u1MIPIDataSettleDelay = sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[2].u1MIPIClkTermDelay = sensorDrvInfo[2].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[2].u1MIPILaneNum = sensorDrvInfo[2].SensorMIPILaneNumber;
        sensorPara[2].u1MIPIPacketECCOrder = sensorDrvInfo[2].SensorPacketECCOrder;
        sensorPara[2].MIPI_PHY_TYPE = (MUINT)sensorDrvInfo[2].MIPIsensorType;
        sensorPara[2].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[2].SettleDelayMode;
        sensorPara[2].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[2].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[2].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[2].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[2].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[2].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[2].SCAM_DataNumber = (MUINT)sensorDrvInfo[2].SCAM_DataNumber;
        sensorPara[2].SCAM_DDR_En = (MUINT)sensorDrvInfo[2].SCAM_DDR_En;
        sensorPara[2].SCAM_CLK_INV = (MUINT)sensorDrvInfo[2].SCAM_CLK_INV;
        sensorPara[2].PDAFMode = gPDAFMode;
        sensorPara[2].SCAM_DEFAULT_DELAY = (MUINT)sensorDrvInfo[2].SCAM_DEFAULT_DELAY;
        sensorPara[2].SCAM_CRC_En = (MUINT)sensorDrvInfo[2].SCAM_CRC_En;
        sensorPara[2].SCAM_SOF_src = (MUINT)sensorDrvInfo[2].SCAM_SOF_src;
        sensorPara[2].SCAM_Timout_Cali = (MUINT)sensorDrvInfo[2].SCAM_Timout_Cali;
        sensorPara[2].SensorMIPIDeskew = 0;
    }
#ifdef MTK_SUB2_IMGSENSOR
    if(currSensorDev & SENSOR_DEV_SUB_2 ) {
            if(halSensorIFParam[3].isBypassScenario != 0) {
                goto lbExit;
            }

            if(halSensorIFParam[idx].framerate == 0)
            {
                data1 = halSensorIFParam[3].scenarioId;
                ret = pSensorDrv->sendCommand(SENSOR_SUB_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                halSensorIFParam[idx].framerate = data2;
                currFPS = halSensorIFParam[idx].framerate;
                MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
            }
            switch(halSensorIFParam[3].scenarioId) {
                case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HSSettleDelayM0;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_PRE;
                    break;
                case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HSSettleDelayM1;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_CAP;
                    break;
                case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HSSettleDelayM2;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_VD;
                    break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HSSettleDelayM3;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_VD1;
                    break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HSSettleDelayM4;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_VD2;
                    break;
                default:
                    sensorPara[3].u1MIPIDataSettleDelay=sensorDrvInfo[3].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                    sensorPara[3].DPCM_TYPE=sensorDrvInfo[3].IMGSENSOR_DPCM_TYPE_PRE;
                    break;
            }
            sensorPara[3].scenarioId = halSensorIFParam[3].scenarioId;
            sensorPara[3].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[3].isBypassScenario;
            sensorPara[3].u1IsContinuous = (MUINT8)halSensorIFParam[3].isContinuous;
            sensorPara[3].u1MIPIDataTermDelay = sensorDrvInfo[3].MIPIDataLowPwr2HighSpeedTermDelayCount;
            //sensorPara[3].u1MIPIDataSettleDelay = sensorDrvInfo[3].MIPIDataLowPwr2HighSpeedSettleDelayCount;
            sensorPara[3].u1MIPIClkTermDelay = sensorDrvInfo[3].MIPICLKLowPwr2HighSpeedTermDelayCount;
            sensorPara[3].u1MIPILaneNum = sensorDrvInfo[3].SensorMIPILaneNumber;
            sensorPara[3].u1MIPIPacketECCOrder = sensorDrvInfo[3].SensorPacketECCOrder;
            sensorPara[3].MIPI_PHY_TYPE = (MUINT)sensorDrvInfo[3].MIPIsensorType;
            sensorPara[3].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[3].SettleDelayMode;
            sensorPara[3].cropWidth = halSensorIFParam[idx].crop.w;
            sensorPara[3].cropHeight = halSensorIFParam[idx].crop.h;
            sensorPara[3].frameRate = halSensorIFParam[idx].framerate;
            sensorPara[3].twopixelOn = halSensorIFParam[idx].twopixelOn;
            sensorPara[3].debugMode = halSensorIFParam[idx].debugMode;
            sensorPara[3].HDRMode = halSensorIFParam[idx].HDRMode;
            sensorPara[3].SCAM_DataNumber = (MUINT)sensorDrvInfo[3].SCAM_DataNumber;
            sensorPara[3].SCAM_DDR_En = (MUINT)sensorDrvInfo[3].SCAM_DDR_En;
            sensorPara[3].SCAM_CLK_INV = (MUINT)sensorDrvInfo[3].SCAM_CLK_INV;
            sensorPara[3].PDAFMode = gPDAFMode;
            sensorPara[3].SCAM_DEFAULT_DELAY = (MUINT)sensorDrvInfo[3].SCAM_DEFAULT_DELAY;
            sensorPara[3].SCAM_CRC_En = (MUINT)sensorDrvInfo[3].SCAM_CRC_En;
            sensorPara[3].SCAM_SOF_src = (MUINT)sensorDrvInfo[3].SCAM_SOF_src;
            sensorPara[3].SCAM_Timout_Cali = (MUINT)sensorDrvInfo[3].SCAM_Timout_Cali;
            sensorPara[3].SensorMIPIDeskew = 0;
        }
#endif

    //main  sensor
    if(currSensorDev & SENSOR_DEV_MAIN) {
        eSensorDev = SENSOR_MAIN;


        switch (sensorPara[0].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[0].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[0].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[0].SensorCapWidth;
                srcHeight = sensorDrvInfo[0].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[0].SensorVideoWidth;
                srcHeight = sensorDrvInfo[0].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[0].SensorVideo1Width;
                srcHeight = sensorDrvInfo[0].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[0].SensorVideo2Width;
                srcHeight = sensorDrvInfo[0].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[0].SensorCustom1Width;
                srcHeight = sensorDrvInfo[0].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[0].SensorCustom2Width;
                srcHeight = sensorDrvInfo[0].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[0].SensorCustom3Width;
                srcHeight = sensorDrvInfo[0].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[0].SensorCustom4Width;
                srcHeight = sensorDrvInfo[0].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[0].SensorCustom5Width;
                srcHeight = sensorDrvInfo[0].SensorCustom5Height;
                break;
            default:
                MY_LOGE("main camera incorrect scenario");
                break;

        }


        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

        // Source is from sensor
        if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;
            sensorPara[0].padSel = PAD_10BIT;//pad2cam_data_sel
            sensorPara[0].inDataType = RAW_10BIT_FMT;//cam_tg_input_fmt
            sensorPara[0].senInLsb = TG_12BIT;//cam_tg_path_cfg
        }
        else if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RAW_8BIT_FMT;
            sensorPara[0].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            sensorPara[0].padSel = PAD_10BIT;
            sensorPara[0].inDataType = RAW_12BIT_FMT;
            sensorPara[0].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = YUV422_FMT;
            sensorPara[0].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RGB565_MIPI_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RGB888_MIPI_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = JPEG_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[0].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[0].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[0].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[0].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[0].debugMode == 1)
        {
            sensorPara[0].inSrcTypeSel  = TEST_MODEL;
        }
        //VR
        if(sensorPara[0].HDRMode == 2)
        {
            sensorPara[0].inSrcTypeSel  = VIRTUAL_CHANNEL_1;
        }

        /* PDAF using VC2 */
        if(sensorPara[0].PDAFMode == 1)
        {
            sensorPara[0].inSrcTypeSel  = VIRTUAL_CHANNEL_2;
        }
        /*Mix mode : PDAF + HDR*/
        if((sensorPara[0].HDRMode == 2) && (sensorPara[0].PDAFMode == 1))
        {
            sensorPara[0].inSrcTypeSel  = VIRTUAL_VC_12;
        }

        sensorPara[0].u1HsyncPol = sensorDrvInfo[0].SensorHsyncPolarity;
        sensorPara[0].u1VsyncPol = sensorDrvInfo[0].SensorVsyncPolarity;




    }


    // SUB sensor
    if(currSensorDev & SENSOR_DEV_SUB ) {
        eSensorDev = SENSOR_SUB;


        switch (sensorPara[1].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[1].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[1].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[1].SensorCapWidth;
                srcHeight = sensorDrvInfo[1].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[1].SensorVideoWidth;
                srcHeight = sensorDrvInfo[1].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[1].SensorVideo1Width;
                srcHeight = sensorDrvInfo[1].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[1].SensorVideo2Width;
                srcHeight = sensorDrvInfo[1].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[1].SensorCustom1Width;
                srcHeight = sensorDrvInfo[1].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[1].SensorCustom2Width;
                srcHeight = sensorDrvInfo[1].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[1].SensorCustom3Width;
                srcHeight = sensorDrvInfo[1].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[1].SensorCustom4Width;
                srcHeight = sensorDrvInfo[1].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[1].SensorCustom5Width;
                srcHeight = sensorDrvInfo[1].SensorCustom5Height;
                break;
            default:
                MY_LOGE("sub camera incorrect scenario");
                break;
        }

        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

        // Source is from sensor
        if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            sensorPara[1].padSel = PAD_10BIT;
            sensorPara[1].inDataType = RAW_10BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RAW_8BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            sensorPara[1].padSel = PAD_10BIT;
            sensorPara[1].inDataType = RAW_12BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = YUV422_FMT;
            sensorPara[1].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RGB565_MIPI_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;


            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RGB888_MIPI_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;


            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = JPEG_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[1].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[1].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[1].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[1].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[1].debugMode == 1)
        {
            sensorPara[1].inSrcTypeSel  = TEST_MODEL;
        }
        /*HDR using VC1*/
        if(sensorPara[1].HDRMode == 2)
        {
            sensorPara[1].inSrcTypeSel  = VIRTUAL_CHANNEL_1;
        }
        /* PDAF using VC2 */
        if(sensorPara[1].PDAFMode == 1)
        {
            sensorPara[1].inSrcTypeSel  = VIRTUAL_CHANNEL_2;
        }
        /*Mix mode : PDAF + HDR*/
        if((sensorPara[1].HDRMode == 2) && (sensorPara[1].PDAFMode == 1))
        {
            sensorPara[1].inSrcTypeSel  = VIRTUAL_VC_12;
        }
        sensorPara[1].u1HsyncPol = sensorDrvInfo[1].SensorHsyncPolarity;
        sensorPara[1].u1VsyncPol = sensorDrvInfo[1].SensorVsyncPolarity;



    }


    // MAIN_2 sensor
    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        eSensorDev = SENSOR_MAIN_2;



        switch (sensorPara[2].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[2].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[2].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[2].SensorCapWidth;
                srcHeight = sensorDrvInfo[2].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[2].SensorVideoWidth;
                srcHeight = sensorDrvInfo[2].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[2].SensorVideo1Width;
                srcHeight = sensorDrvInfo[2].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[2].SensorVideo2Width;
                srcHeight = sensorDrvInfo[2].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[2].SensorCustom1Width;
                srcHeight = sensorDrvInfo[2].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[2].SensorCustom2Width;
                srcHeight = sensorDrvInfo[2].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[2].SensorCustom3Width;
                srcHeight = sensorDrvInfo[2].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[2].SensorCustom4Width;
                srcHeight = sensorDrvInfo[2].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[2].SensorCustom5Width;
                srcHeight = sensorDrvInfo[2].SensorCustom5Height;
                break;
            default:
                MY_LOGE("main2 camera incorrect scenario");
                break;


        }

        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);



        // Source is from sensor
        if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            sensorPara[2].padSel = PAD_10BIT;
            sensorPara[2].inDataType = RAW_10BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RAW_8BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            sensorPara[2].padSel = PAD_10BIT;
            sensorPara[2].inDataType = RAW_12BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = YUV422_FMT;
            sensorPara[2].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RGB565_MIPI_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RGB888_MIPI_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = JPEG_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[2].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[2].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[2].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[2].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[2].debugMode == 1)
        {
            sensorPara[2].inSrcTypeSel  = TEST_MODEL;
        }
         /*HDR using VC1*/
        if(sensorPara[2].HDRMode == 2)
        {
            sensorPara[2].inSrcTypeSel  = VIRTUAL_CHANNEL_1;
        }
        /* PDAF using VC2 */
        if(sensorPara[2].PDAFMode == 1)
        {
            sensorPara[2].inSrcTypeSel  = VIRTUAL_CHANNEL_2;
        }
        /*Mix mode : PDAF + HDR*/
        if((sensorPara[2].HDRMode == 2) && (sensorPara[2].PDAFMode == 1))
        {
            sensorPara[2].inSrcTypeSel  = VIRTUAL_VC_12;
        }
        sensorPara[2].u1HsyncPol = sensorDrvInfo[2].SensorHsyncPolarity;
        sensorPara[2].u1VsyncPol = sensorDrvInfo[2].SensorVsyncPolarity;


    }
#ifdef MTK_SUB2_IMGSENSOR

    if(currSensorDev & SENSOR_DEV_SUB_2 ) {
        eSensorDev = SENSOR_SUB_2;


        switch (sensorPara[3].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[3].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[3].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[3].SensorCapWidth;
                srcHeight = sensorDrvInfo[3].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[3].SensorVideoWidth;
                srcHeight = sensorDrvInfo[3].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[3].SensorVideo1Width;
                srcHeight = sensorDrvInfo[3].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[3].SensorVideo2Width;
                srcHeight = sensorDrvInfo[3].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[3].SensorCustom1Width;
                srcHeight = sensorDrvInfo[3].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[3].SensorCustom2Width;
                srcHeight = sensorDrvInfo[3].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[3].SensorCustom3Width;
                srcHeight = sensorDrvInfo[3].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[3].SensorCustom4Width;
                srcHeight = sensorDrvInfo[3].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[3].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[3].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[3].SensorCustom5Width;
                srcHeight = sensorDrvInfo[3].SensorCustom5Height;
                break;
            default:
                MY_LOGE("sub2 camera incorrect scenario");
                break;


        }

        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);



        // Source is from sensor
        if (sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[3].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[3].crop.w)>>1);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            sensorPara[3].padSel = PAD_10BIT;
            sensorPara[3].inDataType = RAW_10BIT_FMT;
            sensorPara[3].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[3].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[3].crop.w)>>1);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            if (inDataFmt == 0) {
                sensorPara[3].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[3].padSel = PAD_8BIT_7_0;
            }
            sensorPara[3].inDataType = RAW_8BIT_FMT;
            sensorPara[3].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[3].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[3].crop.w)>>1);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            sensorPara[3].padSel = PAD_10BIT;
            sensorPara[3].inDataType = RAW_12BIT_FMT;
            sensorPara[3].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[3].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[3].crop.w);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w * 2;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            if (inDataFmt == 0) {
                sensorPara[3].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[3].padSel = PAD_8BIT_7_0;
            }
            sensorPara[3].inDataType = YUV422_FMT;
            sensorPara[3].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[3].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[3].crop.w);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w * 2;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            if (inDataFmt == 0) {
                sensorPara[3].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[3].padSel = PAD_8BIT_7_0;
            }
            sensorPara[3].inDataType = RGB565_MIPI_FMT;
            sensorPara[3].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[3].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[3].crop.w);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w * 2;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            if (inDataFmt == 0) {
                sensorPara[3].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[3].padSel = PAD_8BIT_7_0;
            }
            sensorPara[3].inDataType = RGB888_MIPI_FMT;
            sensorPara[3].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[3].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[3].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[3].crop.w);
            sensorPara[3].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[3].crop.h)>>1);
            sensorPara[3].u4PixelX1 = sensorPara[3].u4PixelX0 + halSensorIFParam[3].crop.w * 2;
            sensorPara[3].u4PixelY1 = sensorPara[3].u4PixelY0 + halSensorIFParam[3].crop.h;

            if (inDataFmt == 0) {
                sensorPara[3].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[3].padSel = PAD_8BIT_7_0;
            }
            sensorPara[3].inDataType = JPEG_FMT;
            sensorPara[3].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[3].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[3].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[3].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[3].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[3].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[3].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[3].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[3].debugMode == 1)
        {
            sensorPara[3].inSrcTypeSel  = TEST_MODEL;
        }

        sensorPara[3].u1HsyncPol = sensorDrvInfo[3].SensorHsyncPolarity;
        sensorPara[3].u1VsyncPol = sensorDrvInfo[3].SensorVsyncPolarity;


    }
#endif
    // Deskew threadhold
#if ENABLE_CSI_AUTO_DEKSKEW
    if((currSensorDev & SENSOR_DEV_MAIN)
        &&( sensorPara[0].inSrcTypeSel == MIPI_SENSOR)){
            int bpp=10;
            if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_RAW){
                if(sensorStaticInfo[0].rawSensorBit == RAW_SENSOR_10BIT)
                    bpp = 10;//RAW10
                else
                    bpp = 8;//RAW8
            } else {
                    bpp = 16;//YUV422
            }
                MY_LOGD("cropWidth %d, cropHeight %d,frameRate %d,u1MIPILaneNum %d,CSI_BLANKING_RATIO %d\n",
                sensorPara[0].cropWidth,sensorPara[0].cropHeight,sensorPara[0].frameRate,sensorPara[0].u1MIPILaneNum+1,CSI_BLANKING_RATIO);
                MY_LOGD("main lane speed = %ld, auto deskew threshold = %ld\n",
                ((((sensorPara[0].cropWidth * sensorPara[0].cropHeight)/1000) * (sensorPara[0].frameRate/10)*bpp)/(sensorPara[0].u1MIPILaneNum+1)),
                (MAIN_CSI_AUTO_DESKEW_THRESHOLD/CSI_BLANKING_RATIO));
            if(((((sensorPara[0].cropWidth * sensorPara[0].cropHeight)/1000) * (sensorPara[0].frameRate/10)*bpp)/(sensorPara[0].u1MIPILaneNum+1))
                > (MAIN_CSI_AUTO_DESKEW_THRESHOLD/1000)){


                //Main Cam over 1.5Gbps should enable deskew funciton
                if(sensorDrvInfo[0].SensorMIPIDeskew == 1)
                {
                    SW_deskewEnable = 0;
                    HW_deskewEnable = 1;
                }
                else
                {
                    SW_deskewEnable = 1;
                    HW_deskewEnable = 0;
                }
                sensorPara[0].SensorMIPIDeskew = HW_deskewEnable;
            }
    } else if((currSensorDev & SENSOR_DEV_SUB)
        &&( sensorPara[1].inSrcTypeSel == MIPI_SENSOR)){
            int bpp=10;
            if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_RAW){
                if(sensorStaticInfo[1].rawSensorBit == RAW_SENSOR_10BIT)
                    bpp = 10;//RAW10
                else
                    bpp = 8;//RAW8
            } else {
                    bpp = 16;//YUV422
            }

            if(((((sensorPara[1].cropWidth * sensorPara[1].cropHeight)/1000) * (sensorPara[1].frameRate/10)*bpp)/(sensorPara[1].u1MIPILaneNum+1))
                > (SUB_CSI_AUTO_DESKEW_THRESHOLD/CSI_BLANKING_RATIO)){
                MY_LOGD("sub cropWidth %d, cropHeight %d,frameRate %d,u1MIPILaneNum %d,CSI_BLANKING_RATIO %d\n",
                sensorPara[1].cropWidth,sensorPara[1].cropHeight,sensorPara[1].frameRate,sensorPara[1].u1MIPILaneNum+1,CSI_BLANKING_RATIO);
                MY_LOGD("sub lane speed = %ld, auto deskew threshold = %ld\n",
                ((((sensorPara[1].cropWidth * sensorPara[1].cropHeight)/1000) * (sensorPara[1].frameRate/10)*bpp)/(sensorPara[1].u1MIPILaneNum+1)),
                (SUB_CSI_AUTO_DESKEW_THRESHOLD/CSI_BLANKING_RATIO));

                //Sub Cam over 1.5Gbps should enable deskew funciton
                if(sensorDrvInfo[1].SensorMIPIDeskew == 1)
                {
                    SW_deskewEnable = 0;
                    HW_deskewEnable = 1;
                }
                else
                {
                    SW_deskewEnable = 1;
                    HW_deskewEnable = 0;
                }
                sensorPara[1].SensorMIPIDeskew = HW_deskewEnable;


            }
    }
#endif

    //Determine one pixel/two pixel
    pixelModeArrange(currSensorDev);

    ret = seninfControl(1, currSensorDev);

    if(currSensorDev & SENSOR_DEV_MAIN ) {
        eSensorDev = SENSOR_MAIN;
        sensorScenarioId = halSensorIFParam[0].scenarioId;
        // set each sensor scenario separately
        scenarioconf.sId = sensorScenarioId;
        scenarioconf.sensorDevId = eSensorDev;
        scenarioconf.InitFPS = halSensorIFParam[0].framerate;
        scenarioconf.HDRMode = halSensorIFParam[0].HDRMode;
        scenarioconf.PDAFMode = gPDAFMode;
        ret = pSensorDrv->setScenario(scenarioconf);
        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail ",eSensorDev);
            goto lbExit;
        }
        //
        if(mpHwSyncDrv != NULL)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId,currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }
        //
    }


    if(currSensorDev & SENSOR_DEV_SUB ) {
        eSensorDev = SENSOR_SUB;
        sensorScenarioId = halSensorIFParam[1].scenarioId;

        //set each sensor scenario separately
        scenarioconf.sId = sensorScenarioId;
        scenarioconf.sensorDevId = eSensorDev;
        scenarioconf.InitFPS = halSensorIFParam[1].framerate;
        scenarioconf.HDRMode = halSensorIFParam[1].HDRMode;
        scenarioconf.PDAFMode = gPDAFMode;
        ret = pSensorDrv->setScenario(scenarioconf);
        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail ",eSensorDev);
            goto lbExit;
        }
        //
        if(mpHwSyncDrv != NULL)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId,currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }
        //
    }

    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        eSensorDev = SENSOR_MAIN_2;
        sensorScenarioId = halSensorIFParam[2].scenarioId;

        // set each sensor scenario separately
        scenarioconf.sId = sensorScenarioId;
        scenarioconf.sensorDevId = eSensorDev;
        scenarioconf.InitFPS = halSensorIFParam[2].framerate;
        scenarioconf.HDRMode = halSensorIFParam[2].HDRMode;
        scenarioconf.PDAFMode = gPDAFMode;
        ret = pSensorDrv->setScenario(scenarioconf);
        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail",eSensorDev);
            goto lbExit;
        }
        //
        if(mpHwSyncDrv != NULL)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId, currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }
        //
    }
#ifdef MTK_SUB2_IMGSENSOR
    if(currSensorDev & SENSOR_DEV_SUB_2 ) {
        eSensorDev = SENSOR_SUB_2;
        sensorScenarioId = halSensorIFParam[3].scenarioId;

        // set each sensor scenario separately
        scenarioconf.sId = sensorScenarioId;
        scenarioconf.sensorDevId = eSensorDev;
        scenarioconf.InitFPS = halSensorIFParam[3].framerate;
        scenarioconf.HDRMode = halSensorIFParam[3].HDRMode;
        scenarioconf.PDAFMode = gPDAFMode;
        ret = pSensorDrv->setScenario(scenarioconf);
        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail",eSensorDev);
            goto lbExit;
        }
        //
        if(mpHwSyncDrv != NULL)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId, currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }
        //
    }
#endif

    //SW Deskew Function
#if ENABLE_CSI_AUTO_DEKSKEW
        if((currSensorDev & SENSOR_DEV_MAIN)&&( sensorPara[0].inSrcTypeSel == MIPI_SENSOR)){
            if(SW_deskewEnable){
                SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
                pSeninfDrv->autoDeskewCalibrationSeninf1();
                pSeninfDrv->destroyInstance();
            }
        } else if((currSensorDev & SENSOR_DEV_SUB)&&( sensorPara[1].inSrcTypeSel == MIPI_SENSOR)){
            if(SW_deskewEnable){
                SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
                pSeninfDrv->autoDeskewCalibrationSeninf2();
                pSeninfDrv->destroyInstance();
            }
        }
#endif
    //Debug
    //{
    //    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    //    pSeninfDrv->sendCommand(CMD_DEBUG_TASK);
    //}

    /*Rolling shutter support*/
    //pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&data1, (MUINTPTR)&data2);
    //MY_LOGE("CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER(%d)(%d)",data1,data2);



lbExit:
#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagEnd);
#endif


    return ret;
}


 MINT HalSensor::setTgPhase(MINT32 sensorIdx, MINT32 pcEn) //CMMCLK: Main/sub, CMMCLK2:Main_2 (external signal design is not sync with internal signal in TG/I2C)
 {
     MINT32 ret = 0, ret2 = 0;
     MUINT32 u4PadPclkInv1 = 0, u4PadPclkInv2 = 0, u4PadPclkInv3 = 0, u4PadPclkInv4 = 0;
     MINT32 clkInKHz1=0, clkCnt1=0, mclk1=0, mclkSel1=0;
     MINT32 clkInKHz2=0, clkCnt2=0, mclk2=0, mclkSel2=0;
     MINT32 clkInKHz3=0, clkCnt3=0, mclk3=0, mclkSel3=0;
     MINT32 clkInKHz4=0, clkCnt4=0, mclk4=0, mclkSel4=0;
     MINT32 csr_tg_tmp_stp = 0;

     SensorDrv *const pSensorDrv = SensorDrv::get();
     SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

     MY_LOGD("[setTgPhase] Tg1clk: %d, Tg2clk: %d, Tg3clk : %d \n", sensorDrvInfo[0].SensorClockFreq, sensorDrvInfo[1].SensorClockFreq,sensorDrvInfo[2].SensorClockFreq);
     MY_LOGD("sensorDev = %d, pcEn = %d\n", sensorIdx, pcEn);//JH debug

     /*
             SENSOR_DEV_NONE = 0x00,
             SENSOR_DEV_MAIN = 0x01,
             SENSOR_DEV_SUB  = 0x02,
             SENSOR_DEV_PIP = 0x03,
             SENSOR_DEV_MAIN_2 = 0x04,
             SENSOR_DEV_MAIN_3D = 0x05,
     */
     if(sensorIdx & SENSOR_DEV_MAIN) {
	 /*get property*/
	 char value[PROPERTY_VALUE_MAX] = {'\0'};
	 int _Tg1clk = 0;
         property_get("debug.seninf.Tg1clk", value, "-1");
	 _Tg1clk = atoi(value);
	 if(_Tg1clk >= 0){
		 sensorDrvInfo[0].SensorClockFreq = _Tg1clk;
		 MY_LOGD("[setTgPhase] setproperty Tg1clk: %d \n", sensorDrvInfo[0].SensorClockFreq);
	 }
         clkInKHz1 = sensorDrvInfo[0].SensorClockFreq * 1000;

         if ((clkInKHz1 < 3250) || (clkInKHz1 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz1);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz1) == 0) {            // Clock is in 48MHz group, original source is 48MHz
             mclk1 = 48000;
             mclkSel1 = CAM_PLL_48_GROUP;
         }
         else {
             // Clock is in 52MHz group
             mclk1 = 208000;//52000;
             mclkSel1 = CAM_PLL_52_GROUP;
         }
         /* Subsample period is  2*(csr_tg_tmp_stp +1)*period_of_tg_pll_ck  , tg_pll_ck = 48Mhz*/
         /* Target : 2*(23+1)*(1/48M) =  1Mhz */
         sensorDrvInfo[0].SensorMCLKPLL = 1000;
         /*sub sample:csr_tg_tmp_stp */
         if(mclk1 == 48000)
            csr_tg_tmp_stp = 23;/*unit :Khz. 1000kHz = 1Mhz */
         else
            csr_tg_tmp_stp = 103;/*unit :Khz. 1000kHz = 1Mhz */
         //
         clkCnt1 = (mclk1 + (clkInKHz1 >> 1)) / clkInKHz1;
         // Maximum CLKCNT is 15
         clkCnt1 = clkCnt1 > 15 ? 15 : clkCnt1-1;
         MY_LOGD("  mclk0: %d, clkCnt1: %d \n", mclk1, clkCnt1);
     }

     if(sensorIdx & SENSOR_DEV_SUB) {
	 /*get property*/
	 char value[PROPERTY_VALUE_MAX] = {'\0'};
	 int _Tg2clk = 0;
	 property_get("vendor.debug.seninf.Tg2clk", value, "-1");
	 _Tg2clk = atoi(value);
	 if(_Tg2clk >= 0){
		 sensorDrvInfo[1].SensorClockFreq = _Tg2clk;
		 MY_LOGD("[setTgPhase] setproperty Tg2clk: %d \n", sensorDrvInfo[1].SensorClockFreq);
	 }
         clkInKHz2 = sensorDrvInfo[1].SensorClockFreq * 1000;


         if ((clkInKHz2 < 3250) || (clkInKHz2 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz2);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz2) == 0) {
             // Clock is in 48MHz group, original source is 48MHz
             mclk2 = 48000;
             mclkSel2 = CAM_PLL_48_GROUP;
         }
         else {
             // Clock is in 52MHz group
             mclk2 = 208000;//52000;
             mclkSel2 = CAM_PLL_52_GROUP;
         }
         /* Subsample period is  2*(csr_tg_tmp_stp +1)*period_of_tg_pll_ck  , tg_pll_ck = 48Mhz*/
         /* Target : 2*(23+1)*(1/48M) =  1Mhz */
         sensorDrvInfo[1].SensorMCLKPLL = 1000;
         /*sub sample:csr_tg_tmp_stp */
         if(mclk2 == 48000)
            csr_tg_tmp_stp = 23;/*unit :Khz. 1000kHz = 1Mhz */
         else
            csr_tg_tmp_stp = 103;/*unit :Khz. 1000kHz = 1Mhz */
         //
         clkCnt2 = (mclk2 + (clkInKHz2 >> 1)) / clkInKHz2;
         clkCnt2 = clkCnt2 > 15 ? 15 : clkCnt2-1;
         MY_LOGD("  mclk1: %d, clkCnt1: %d \n", mclk2, clkCnt2);
     }


     if(sensorIdx & SENSOR_DEV_MAIN_2){
	  /*get property*/
	  char value[PROPERTY_VALUE_MAX] = {'\0'};
	  int _Tg3clk = 0;
	  property_get("vendor.debug.seninf.Tg3clk", value, "-1");
	  _Tg3clk = atoi(value);
	  if(_Tg3clk >= 0){
		  sensorDrvInfo[2].SensorClockFreq = _Tg3clk;
		  MY_LOGD("[setTgPhase] setproperty Tg3clk: %d \n", sensorDrvInfo[2].SensorClockFreq);
	  }

         clkInKHz3 = sensorDrvInfo[2].SensorClockFreq * 1000;
         if ((clkInKHz3 < 3250) || (clkInKHz3 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz3);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz3) == 0) {
             // Clock is in 48MHz group, original source is 48MHz
             mclk3 = 48000;
             mclkSel3 = CAM_PLL_48_GROUP;
         }
         else {
             mclk3 = 208000;
             mclkSel3 = CAM_PLL_52_GROUP;
         }
         /* Subsample period is  2*(csr_tg_tmp_stp +1)*period_of_tg_pll_ck  , tg_pll_ck = 48Mhz*/
         /* Target : 2*(23+1)*(1/48M) =  1Mhz */
         sensorDrvInfo[2].SensorMCLKPLL = 1000;
         /*sub sample:csr_tg_tmp_stp */
         if(mclk3 == 48000)
            csr_tg_tmp_stp = 23;/*unit :Khz. 1000kHz = 1Mhz */
         else
            csr_tg_tmp_stp = 103;/*unit :Khz. 1000kHz = 1Mhz */
         //
         clkCnt3 = (mclk3 + (clkInKHz3 >> 1)) / clkInKHz3;
         // Maximum CLKCNT is 15
         clkCnt3 = clkCnt3 > 15 ? 15 : clkCnt3-1;
         MY_LOGD("  mclk3: %d, clkCnt3: %d \n", mclk3, clkCnt3);
     }

#ifdef MTK_SUB2_IMGSENSOR
     if(sensorIdx & SENSOR_DEV_SUB_2){

          clkInKHz4 = sensorDrvInfo[3].SensorClockFreq * 1000;
          if ((clkInKHz4 < 3250) || (clkInKHz4 >= 104000)) {
              MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz4);
              return -EINVAL;
          }
          //
          if ((48000 % clkInKHz4) == 0) {
              // Clock is in 48MHz group, original source is 48MHz
              mclk4 = 48000;
              mclkSel4 = CAM_PLL_48_GROUP;
          }
          else {
              mclk4 = 208000;
              mclkSel4 = CAM_PLL_52_GROUP;
          }
          /* Subsample period is  2*(csr_tg_tmp_stp +1)*period_of_tg_pll_ck  , tg_pll_ck = 48Mhz*/
          /* Target : 2*(23+1)*(1/48M) =  1Mhz */
          sensorDrvInfo[3].SensorMCLKPLL = 1000;
          /*sub sample:csr_tg_tmp_stp */
          if(mclk4 == 48000)
             csr_tg_tmp_stp = 23;/*unit :Khz. 1000kHz = 1Mhz */
          else
             csr_tg_tmp_stp = 103;/*unit :Khz. 1000kHz = 1Mhz */
          //
          clkCnt4 = (mclk4 + (clkInKHz4 >> 1)) / clkInKHz4;
          // Maximum CLKCNT is 15
          clkCnt4 = clkCnt4 > 15 ? 15 : clkCnt4-1;
          MY_LOGD("  mclk4: %d, clkCnt4: %d \n", mclk4, clkCnt4);
      }
#endif

     switch (sensorIdx)
     {
     case SENSOR_DEV_MAIN:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         break;
     case SENSOR_DEV_SUB:
         ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv2);
         break;
     case SENSOR_DEV_PIP:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         ret2 = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv2);
         break;
     case SENSOR_DEV_MAIN_2:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv3);
         break;
     case SENSOR_DEV_MAIN_3D:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         ret2 = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv3);
         break;
#ifdef MTK_SUB2_IMGSENSOR
     case SENSOR_DEV_SUB_2:
          ret = pSensorDrv->sendCommand(SENSOR_SUB_2, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv4);
          break;
#endif
     default:
         u4PadPclkInv1 = 0;
         u4PadPclkInv2 = 0;
         u4PadPclkInv3 = 0;
#ifdef MTK_SUB2_IMGSENSOR
         u4PadPclkInv4 = 0;
#endif
         ret = 0;
         ret2 = 0;
         break;
     }
     if ((ret < 0)||(ret2 < 0)) {
         MY_LOGE("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x), err2(%x)\n", ret, ret2);
     }
     MY_LOGD("[setTgPhase] u4PadPclkInv_1(%d),u4PadPclkInv_2(%d) \n", u4PadPclkInv1,u4PadPclkInv2);

     // Config TG, always use Camera PLL, 1: 48MHz, 2: 104MHz

     if(sensorIdx & SENSOR_DEV_MAIN ) {
         if(sensorPara[0].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[0].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[0].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1,
                 csr_tg_tmp_stp);
         }
         else {
             MY_LOGE("Main camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[0].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg1PhaseCounter fail\n");
             return ret;
         }
     }


     //notice SUB sensorInfo[1] but use Tg1 mclk
     if(sensorIdx & SENSOR_DEV_SUB) {
         if(sensorPara[1].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[1].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[1].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2,
                 csr_tg_tmp_stp);
         }
         else {
             MY_LOGE("Sub camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[1].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg3PhaseCounter fail\n");
             return ret;
         }

     }



     if(sensorIdx & SENSOR_DEV_MAIN_2){
         if(sensorPara[2].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[2].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[2].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3,
                 csr_tg_tmp_stp);
         }
         else {
             MY_LOGE("Main2 camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[2].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg2PhaseCounter fail\n");
             return ret;
         }
     }


#ifdef MTK_SUB2_IMGSENSOR
     if(sensorIdx & SENSOR_DEV_SUB_2){
         if(sensorPara[3].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel4 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt4, sensorDrvInfo[3].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[3].SensorClockFallingCount, sensorDrvInfo[3].SensorClockRisingCount, u4PadPclkInv4,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[3].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel4 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt4, sensorDrvInfo[3].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[3].SensorClockFallingCount, sensorDrvInfo[3].SensorClockRisingCount, u4PadPclkInv4,
                 csr_tg_tmp_stp);
         }
         else if(sensorPara[3].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel4 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt4, sensorDrvInfo[3].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[3].SensorClockFallingCount, sensorDrvInfo[3].SensorClockRisingCount, u4PadPclkInv4,
                 csr_tg_tmp_stp);
         }
         else {
             MY_LOGE("sub2 camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[3].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg2PhaseCounter fail\n");
             return ret;
         }
     }
#endif

     pSeninfDrv->destroyInstance();

     return ret;
 }





MINT HalSensor::sendCommand(
    MUINT sensorDevIdx,
    MUINTPTR cmd,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3)
{
    MINT32 ret = 0;
    MUINT32 cmdId = 0;

    MUINT32 sensorDevId = sensorDevIdx;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    switch (cmd) {
    //0x1000

    case SENSOR_CMD_SET_SENSOR_EXP_TIME:
        /* Set Exposure time (unit : us) */
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINTPTR)arg1);
        MY_LOGD("Exposure Time: %d \n", *(MUINT32 *) arg1);

        break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_DUAL_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_DUAL_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_FLICKER_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_FLICKER_FRAME_RATE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_VIDEO_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_VIDEO_FRAME_RATE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC:
        cmdId = CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_CCT_FEATURE_CONTROL:
        cmdId = CMD_SENSOR_SET_CCT_FEATURE_CONTROL;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    case SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA:
        cmdId = CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO: scenario = %d, frame rates = %d (10base) \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_SET_TEST_PATTERN_OUTPUT:
        cmdId = CMD_SENSOR_SET_TEST_PATTERN_OUTPUT;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_ESHUTTER_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_OB_LOCK:
        cmdId = CMD_SENSOR_SET_OB_LOCK;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MIN_MAX_FPS:
        cmdId = CMD_SENSOR_SET_MIN_MAX_FPS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME:
            /* Set Exposure time  and Frame time(unit : us) */
            cmdId = CMD_SENSOR_SET_SENSOR_EXP_FRAME_TIME;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINTPTR)arg1, (MUINTPTR)arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE:
           cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE;
           ret = pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINTPTR)arg1);
           MY_LOGD("SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE Time: %d \n", *(MUINT32 *) arg1);
       break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE_BUF_MODE:
           cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE;
           pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
            MY_LOGD("SENSOR_CMD_SET_SENSOR_EXP_LINE_BUF_MODE: %d \n", *(MUINT32 *) arg1);
       break;

    case SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE:
           cmdId = CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE;
           pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
            MY_LOGD("CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE: %d \n", *(MUINT32 *) arg1);
       break;
    case SENSOR_CMD_SET_I2C_BUF_MODE_EN:
            cmdId = CMD_SENSOR_SET_I2C_BUF_MODE_EN;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
            MY_LOGD("SENSOR_CMD_SET_I2C_BUF_MODE_EN: %d \n", *(MUINT32 *) arg1);
       break;
    case SENSOR_CMD_SET_STREAMING_SUSPEND:
             cmdId = CMD_SENSOR_SET_STREAMING_SUSPEND;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
            break;
    case SENSOR_CMD_SET_STREAMING_RESUME:
            cmdId = CMD_SENSOR_SET_STREAMING_RESUME;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
            break;
    case SENSOR_CMD_SET_N3D_CONFIG:
        ret = pSeninfDrv->setN3DCfg(*(MUINT32 *) arg1,*((MUINT32 *)arg1+1),*((MUINT32 *)arg1+2),*((MUINT32 *)arg1+3),*((MUINT32 *)arg1+4),*((MUINT32 *)arg1+5));
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail! \n");
        }
        break;
    case SENSOR_CMD_SET_N3D_I2C_POS:
        ret = pSeninfDrv->setN3DI2CPos(*(MUINTPTR*)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_I2C_POS fail! \n");
        }
        break;
    case SENSOR_CMD_SET_N3D_I2C_TRIGGER:
        ret = pSeninfDrv->setN3DTrigger(*(MUINTPTR *)arg1, *(MUINTPTR *)arg2);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_I2C_TRIGGER fail! \n");
        }
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_IHDR_SHUTTER_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_SHUTTER:
        cmdId = CMD_SENSOR_SET_HDR_SHUTTER;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_AWB_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    /*RGBW Sensor CMD*/
    case SENSOR_CMD_SET_SENSOR_AWB_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_AWB_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_ISO:
        cmdId = CMD_SENSOR_SET_SENSOR_ISO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    /*End of RGBW CMD*/
    //0x2000
    case SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT:
        cmdId = CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2); //arg1 = mode
        break;


    case SENSOR_CMD_GET_PIXEL_CLOCK_FREQ:
        cmdId = CMD_SENSOR_GET_PIXEL_CLOCK_FREQ;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        cmdId = CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_FEATURE_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_FEATURE_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;


    case SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;


    case SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE:
        cmdId = CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_TEMPERATURE_VALUE:
        cmdId = CMD_SENSOR_GET_TEMPERATURE_VALUE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO: scenario = %d, crop = 0x%x  \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_PIXELMODE:
        *(MUINT32 *)arg3  = GetpixelMode((MUINT)sensorDevId, *(MUINT32*)arg1, *(MUINT32 *)arg2);
        MY_LOGD("SENSOR_CMD_GET_SENSOR_PIXELMODE:scenario = %d, fps = %d, Pixelmode =%d \n", *(MUINT32 *)arg1, *(MUINT32 *)arg2, *(MUINT32 *)arg3);
        break;

    case SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT:
        ret = pSeninfDrv->getN3DDiffCnt((MUINT32 *)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail! \n");
        }
        break;
   case SENSOR_CMD_GET_SENSOR_VC_INFO:
       ret = pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId, CMD_SENSOR_GET_SENSOR_VC_INFO, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_VC_INFO fail! \n");
        }
       break;

   case SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING:
       ret = pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId, CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING fail! \n");
       }
       break;
   case SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING:
       ret = pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId, CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING fail! \n");
       }
       break;
   case SENSOR_CMD_GET_SENSOR_PDAF_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_PDAF_INFO: scenario = %x %x\n", *(MUINT32 *) arg1,arg2);
        if(ret < 0) {
             MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_PDAF_INFO fail! \n");
        }
        break;
	case SENSOR_CMD_SET_PDFOCUS_AREA:
		cmdId = CMD_SET_PDFOCUS_AREA;
		pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
		break;
	case SENSOR_CMD_GET_PDAF_DATA:
        cmdId = CMD_SENSOR_GET_PDAF_DATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_4CELL_SENSOR:
        cmdId = CMD_SENSOR_GET_4CELL_DATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    case SENSOR_CMD_GET_SENSOR_HDR_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_HDR_CAPACITY;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    //0x3000
    case SENSOR_CMD_SET_YUV_FEATURE_CMD:
        cmdId = CMD_SENSOR_SET_YUV_FEATURE_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE:
        cmdId = CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CANCEL_AF:
        cmdId = CMD_SENSOR_SET_YUV_CANCEL_AF;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CONSTANT_AF:
        cmdId = CMD_SENSOR_SET_YUV_CONSTANT_AF;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_INFINITY_AF:
            cmdId = CMD_SENSOR_SET_YUV_INFINITY_AF;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
            break;

    case SENSOR_CMD_SET_YUV_AF_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AF_WINDOW;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_AE_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AE_WINDOW;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_GAIN_AND_EXP_LINE:
        cmdId = CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_3A_CMD:
        cmdId = CMD_SENSOR_SET_YUV_3A_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    //0x4000
    case SENSOR_CMD_GET_YUV_AF_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AF_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AE_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AWB_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AWB_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EXIF_INFO:
        cmdId = CMD_SENSOR_GET_YUV_EXIF_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_DELAY_INFO:
        cmdId = CMD_SENSOR_GET_YUV_DELAY_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_AE_AWB_LOCK:
        cmdId = CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_YUV_STROBE_INFO:
        cmdId = CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        cmdId = CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_YUV_AUTOTEST:
        cmdId = CMD_SENSOR_SET_YUV_AUTOTEST;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        cmdId=CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA:
        cmdId=CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_START_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_START_STREAMING;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_STOP_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_STOP_STREAMING;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER:
        /*Warning: Disable Rolling shutter support*/
        cmdId = CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER;
        //pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_POWER_ON_STETE: /*LSC funciton need open after sensor Power On*/
        *((MUINT32*) arg1) = (meSensorDev & sensorDevId);
        break;
    case SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS: /*Pass1 deque fail would call this command*/
        switch(arg1){
            case 0:
               ret =  pSeninfDrv->sendCommand(CMD_DEBUG_TASK); /* For CAM */
                break;
            case 1:
               ret =  pSeninfDrv->sendCommand(CMD_DEBUG_TASK_CAMSV); /* For CAMSV */
                break;
            default:
                MY_LOGE("Unsupported module for seninf debug \n");
                break;
        }
        break;
    case SENSOR_CMD_GET_SENSOR_SYNC_MODE_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_SENSOR_SYNC_MODE:
        cmdId = CMD_SENSOR_GET_SENSOR_SYNC_MODE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_SYNC_MODE:
        cmdId = CMD_SENSOR_SET_SENSOR_SYNC_MODE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    default:
        ret = -1;
        MY_LOGD("[sendCommand] err: 0x%x \n", cmd);
        break;
    }
    //

    pSeninfDrv->destroyInstance();


    return ret;

}


 MINT HalSensor::setSensorIODrivingCurrent(MINT32 sensorIdx)
 {

     MINT32 ret = 0;
     MINT32 increaseDivingCurrent1 = 0x08; // set to default 2mA and slew raw control

     SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

     if( sensorIdx & SENSOR_DEV_MAIN ) { //Main/sub use TG1 mclk
         switch(sensorDrvInfo[0].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA://4 //4mA
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:// 8mA
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA://12mA
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA://16mA
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }
         if(sensorPara[0].mclkSrc == 0) {
         ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
     }
         else if(sensorPara[0].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[0].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
         }
     }


     if( sensorIdx & SENSOR_DEV_SUB ) { //Main/sub use TG1 mclk
         switch(sensorDrvInfo[1].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA://4 //4mA
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:// 8mA
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA://12mA
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA://16mA
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }
         if(sensorPara[1].mclkSrc == 2) {
         ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
     }
         else if(sensorPara[1].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[1].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
         }
     }
     if (ret < 0) {
         MY_LOGE("The Tg1 driving current setting is wrong\n");
     }


     if(sensorIdx & SENSOR_DEV_MAIN_2) {
         switch(sensorDrvInfo[2].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA:
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA:
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA:
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }

         if(sensorPara[2].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[2].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[2].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
         }

     }
#ifdef MTK_SUB2_IMGSENSOR
       if(sensorIdx & SENSOR_DEV_SUB_2) {
         switch(sensorDrvInfo[2].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA:
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA:
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA:
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }

         if(sensorPara[3].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[3].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[3].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
         }

     }
#endif

     if (ret < 0) {
         MY_LOGE("The Tg2 driving current setting is wrong\n");
     }

     pSeninfDrv->destroyInstance();

     return ret;
 }

MVOID HalSensor::pixelModeArrange(MUINT currSensorDev)
{
    unsigned int Isp_clk = 0;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    Isp_clk = pSeninfDrv->getISPClk();

    //One pixel, two pixel
    if(currSensorDev & SENSOR_DEV_MAIN ) {
        if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_RAW){
            if((sensorPara[0].cropWidth * sensorPara[0].cropHeight/1000 * (sensorPara[0].frameRate/10)) >= (Isp_clk*TWO_PIXEL_MODE_MARGIN)){
            //if((sensorPara[0].cropWidth * sensorPara[0].cropHeight/1000 * (sensorPara[0].frameRate/10)) >= (TWO_PIXEL_MODE_THRESHOLD)){
                sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
            }

            // use 2-pix mode when sensor is dual pd
            if(sensorDrvInfo[0].PDAF_Support == 4) {
                sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
            }

        }
        else if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_YUV) {
            if(sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI){
                sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
            }
            else{
                sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
        }
        MY_LOGI("MainCam pixel mode: %d, sensorType: %d, Isp_clk = %d, pd_support=%d\n", sensorDynamicInfo[0].pixelMode, sensorStaticInfo[0].sensorType, Isp_clk, sensorDrvInfo[0].PDAF_Support);
    }
   if(currSensorDev & SENSOR_DEV_SUB ) {
        if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_RAW){
            if((sensorPara[1].cropWidth * sensorPara[1].cropHeight/1000 * (sensorPara[1].frameRate/10)) >= (Isp_clk*TWO_PIXEL_MODE_MARGIN)){
            //if((sensorPara[1].cropWidth * sensorPara[1].cropHeight/1000 * (sensorPara[1].frameRate/10)) >= (TWO_PIXEL_MODE_THRESHOLD)){
                sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
            }

            // use 2-pix mode when sensor is dual pd
            if(sensorDrvInfo[1].PDAF_Support == 4 || sensorDrvInfo[1].PDAF_Support == 5) {
                sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_YUV) {
            if(sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI){
                sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
            }
            else{
                sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
        }
        MY_LOGI("SubCam pixel mode: %d, sensorType: %d, Isp_clk = %d, pd_support=%d\n", sensorDynamicInfo[1].pixelMode, sensorStaticInfo[1].sensorType, Isp_clk, sensorDrvInfo[1].PDAF_Support);
    }
   if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        if (sensorStaticInfo[2].sensorType == SENSOR_TYPE_RAW){
            if((sensorPara[2].cropWidth * sensorPara[2].cropHeight/1000 * (sensorPara[2].frameRate/10)) >= (Isp_clk*TWO_PIXEL_MODE_MARGIN)){
            //if((sensorPara[2].cropWidth * sensorPara[2].cropHeight/1000 * (sensorPara[2].frameRate/10)) >= (TWO_PIXEL_MODE_THRESHOLD)){
                sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
            }

            // use 2-pix mode when sensor is dual pd
            if(sensorDrvInfo[2].PDAF_Support == 4 || sensorDrvInfo[2].PDAF_Support == 5) {
                sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[2].sensorType == SENSOR_TYPE_YUV) {
            if(sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI){
                sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
            }
            else{
                sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[2].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
        }
        MY_LOGI("Main2Cam pixel mode: %d, sensorType: %d, Isp_clk = %d, pd_support=%d\n", sensorDynamicInfo[2].pixelMode, sensorStaticInfo[2].sensorType, Isp_clk, sensorDrvInfo[2].PDAF_Support);
    }
#ifdef MTK_SUB2_IMGSENSOR
   if(currSensorDev & SENSOR_DEV_SUB_2 ) {
        if (sensorStaticInfo[3].sensorType == SENSOR_TYPE_RAW){
            if((sensorPara[3].cropWidth * sensorPara[3].cropHeight/1000 * (sensorPara[3].frameRate/10)) >= (Isp_clk*TWO_PIXEL_MODE_MARGIN)){
            //if((sensorPara[3].cropWidth * sensorPara[3].cropHeight/1000 * (sensorPara[3].frameRate/10)) >= (TWO_PIXEL_MODE_THRESHOLD)){
                sensorDynamicInfo[3].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[3].pixelMode = ONE_PIXEL_MODE;
            }

            // use 2-pix mode when sensor is dual pd
            if(sensorDrvInfo[3].PDAF_Support == 4 || sensorDrvInfo[3].PDAF_Support == 5) {
                sensorDynamicInfo[3].pixelMode = TWO_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[3].sensorType == SENSOR_TYPE_YUV) {
            if(sensorDrvInfo[3].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI){
                sensorDynamicInfo[3].pixelMode = TWO_PIXEL_MODE;
            }
            else{
                sensorDynamicInfo[3].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[3].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[3].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[3].pixelMode = ONE_PIXEL_MODE;
        }
        MY_LOGI("sub2 pixel mode: %d, sensorType: %d, Isp_clk = %d, pd_support=%d\n", sensorDynamicInfo[3].pixelMode, sensorStaticInfo[3].sensorType, Isp_clk, sensorDrvInfo[3].PDAF_Support);
    }
#endif

   pSeninfDrv->destroyInstance();

}

MINT HalSensor::GetpixelMode(MUINT currSensorDev, MUINT scenario, MUINT fps)
{
    MINT ret = ONE_PIXEL_MODE;
    MUINT16 index;
    MUINT16 srcWidth=0;
    MUINT16 srcHeight=0;
    MUINT16 u16fps;
    unsigned int Isp_clk = 0;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    Isp_clk = pSeninfDrv->getISPClk();

    if(currSensorDev & SENSOR_DEV_MAIN)
    {
        index = 0;
    }
    else if(currSensorDev & SENSOR_DEV_SUB)
    {
        index = 1;
    }
#ifdef MTK_SUB2_IMGSENSOR
    else if(currSensorDev & SENSOR_DEV_SUB_2)
    {
        index = 3;
    }
#endif
    else
    {
        index = 2;
    }

    switch (scenario) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            srcWidth = sensorDrvInfo[index].SensorPreviewWidth;
            srcHeight = sensorDrvInfo[index].SensorPreviewHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            srcWidth = sensorDrvInfo[index].SensorCapWidth;
            srcHeight = sensorDrvInfo[index].SensorCapHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            srcWidth = sensorDrvInfo[index].SensorVideoWidth;
            srcHeight = sensorDrvInfo[index].SensorVideoHeight;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            srcWidth = sensorDrvInfo[index].SensorVideo1Width;
            srcHeight = sensorDrvInfo[index].SensorVideo1Height;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            srcWidth = sensorDrvInfo[index].SensorVideo2Width;
            srcHeight = sensorDrvInfo[index].SensorVideo2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            srcWidth = sensorDrvInfo[index].SensorCustom1Width;
            srcHeight = sensorDrvInfo[index].SensorCustom1Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            srcWidth = sensorDrvInfo[index].SensorCustom2Width;
            srcHeight = sensorDrvInfo[index].SensorCustom2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            srcWidth = sensorDrvInfo[index].SensorCustom3Width;
            srcHeight = sensorDrvInfo[index].SensorCustom3Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            srcWidth = sensorDrvInfo[index].SensorCustom4Width;
            srcHeight = sensorDrvInfo[index].SensorCustom4Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            srcWidth = sensorDrvInfo[index].SensorCustom5Width;
            srcHeight = sensorDrvInfo[index].SensorCustom5Height;
            break;
        default:
            MY_LOGE("camera incorrect scenario");
            break;
    }

    // Frame rate , Unit : FPS , Driver Unit : 10*FPS
    u16fps = 10*fps;
    if (sensorStaticInfo[index].sensorType == SENSOR_TYPE_RAW){
        //One pixel, two pixel
        if((srcWidth * srcHeight/1000 * (u16fps/10)) >= (TWO_PIXEL_MODE_THRESHOLD))
        //if((srcWidth * srcHeight/1000 * (u16fps/10)) >= (Isp_clk*TWO_PIXEL_MODE_MARGIN))
        {
            ret = TWO_PIXEL_MODE;
        }
        else
        {
            ret = ONE_PIXEL_MODE;
        }

        // use 2-pix mode when sensor is dual pd
        if(sensorDrvInfo[index].PDAF_Support == 4 || sensorDrvInfo[index].PDAF_Support == 5) {
            ret = TWO_PIXEL_MODE;
        }
    }
    else if (sensorStaticInfo[index].sensorType == SENSOR_TYPE_YUV) {
            if(sensorDrvInfo[index].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI){
                ret = TWO_PIXEL_MODE;
            }
            else{
                ret = ONE_PIXEL_MODE;
            }
    }
    else if (sensorStaticInfo[index].sensorType == SENSOR_TYPE_JPEG) {
        ret = TWO_PIXEL_MODE;
    }

    return ret;
}

MINT HalSensor::seninfControl(MUINT8 enable, MUINT32 currSensorDev)
{
    MINT ret = 0;
    unsigned long CSI_TYPE = 0;
    SENINF_SOURCE_ENUM inSrcTypeSel = CSI2;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    SENSOR_CONFIG_STRUCT *psensorPara = NULL;
    SensorDynamicInfo *psensorDynamicInfo = NULL;
    SENSORDRV_INFO_STRUCT* psensorDrvInfo = NULL;
    Seninf_para seninfpara;
    unsigned int mipi_type;
    unsigned int seninfSrc;

    MY_LOGD("seninfControl meSensorDev = %d, currSensorDev = %d, enable = %d, PixelMode0=%d, PixelMode1=%d, PixelMode2=%d,\n",
        meSensorDev,currSensorDev,enable,sensorDynamicInfo[0].pixelMode, sensorDynamicInfo[1].pixelMode, sensorDynamicInfo[2].pixelMode);
    MY_LOGD("sensorPara[0].inSrcTypeSel = %d, sensorPara[1].inSrcTypeSel=%d, sensorPara[2].inSrcTypeSel = %d\n",
        sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel,sensorPara[2].inSrcTypeSel);




    if (1 == enable) {

        switch(meSensorDev) {
            case SENSOR_DEV_MAIN:
            case SENSOR_DEV_SUB:
            case SENSOR_DEV_MAIN_2:
            case SENSOR_DEV_SUB_2:
                if(meSensorDev != currSensorDev) {
                    MY_LOGE("seninfControl current control not match. meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                }



                if(currSensorDev == SENSOR_DEV_MAIN)
                {
                    /*Set Sensor interface parameter*/
                    psensorPara = &sensorPara[0];
                    /*Set Dynamic information*/
                    psensorDynamicInfo = &sensorDynamicInfo[0];
                    psensorDrvInfo = &sensorDrvInfo[0];
                }
                else if(currSensorDev == SENSOR_DEV_SUB)
                {
                    /*Set Sensor interface parameter*/
                    psensorPara = &sensorPara[1];
                    /*Set Dynamic information*/
                    psensorDynamicInfo = &sensorDynamicInfo[1];
                    psensorDrvInfo = &sensorDrvInfo[1];
                }
#ifdef MTK_SUB2_IMGSENSOR
                else if(meSensorDev == SENSOR_DEV_SUB_2)
                {
                    /*Set Sensor interface parameter*/
                    psensorPara = &sensorPara[3];
                    /*Set Dynamic information*/
                    psensorDynamicInfo = &sensorDynamicInfo[3];
                    psensorDrvInfo = &sensorDrvInfo[3];
                }
#endif
                else
                {
                    /*Set Sensor interface parameter*/
                    psensorPara = &sensorPara[2];
                    /*Set Dynamic information*/
                    psensorDynamicInfo = &sensorDynamicInfo[2];
                    psensorDrvInfo = &sensorDrvInfo[2];
                }
                /*Set Sensor interface source */
                if(psensorPara->mipiPad == 0){
                    seninfSrc = SENINF_1;
                    seninfpara.CSI2_IP  = SENINF1_CSI0;
                }
                else if(psensorPara->mipiPad == 1){
                    seninfSrc = SENINF_2;
                    seninfpara.CSI2_IP  = SENINF2_CSI1;
                }
                else if(psensorPara->mipiPad == 2){
                    seninfSrc = SENINF_4;
                    seninfpara.CSI2_IP  = SENINF4_CSI2;
                }
                else{
                    seninfSrc = SENINF_1;
                    seninfpara.CSI2_IP  = SENINF1_CSI0;
                }

                if(psensorPara->MIPI_PHY_TYPE == 1){
                    CSI_TYPE = 1; inSrcTypeSel = MIPI_SENSOR;
                }
                else{
                    CSI_TYPE = 0; inSrcTypeSel = MIPI_SENSOR;
                }

                MY_LOGD("seninfControl meSensorDev = %d, currSensorDev = %d, enable = %d, PixelMode=%d\n",meSensorDev,currSensorDev,enable,psensorDynamicInfo->pixelMode);
                    MY_LOGD("sensorPara.inSrcTypeSel = %d, sensorPara.inSrcTypeSel=%d,\n",psensorPara->inSrcTypeSel,psensorPara->inSrcTypeSel);
                MY_LOGD("seninfSrc=%d, MIPI_PHY_TYPE=%d\n", seninfSrc, psensorPara->MIPI_PHY_TYPE);
                seninfpara.padSel           = (PAD2CAM_DATA_ENUM)psensorPara->padSel;
                seninfpara.inSrcTypeSel     = (SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel;
                seninfpara.dataTermDelay    = psensorPara->u1MIPIDataTermDelay;
                seninfpara.dataSettleDelay  = psensorPara->u1MIPIDataSettleDelay;
                seninfpara.clkTermDelay     = psensorPara->u1MIPIClkTermDelay;
                seninfpara.vsyncType        = psensorPara->u1VsyncPol;
                seninfpara.dlaneNum         = psensorPara->u1MIPILaneNum;
                seninfpara.Enable           = 1;
                seninfpara.dataheaderOrder  = psensorPara->u1MIPIPacketECCOrder;
                seninfpara.mipi_type        = CSI_TYPE;
                seninfpara.HSRXDE           = 1; // Use manual mode
                seninfpara.dpcm             = psensorPara->DPCM_TYPE;
                seninfpara.MIPIDeskew       = psensorPara->SensorMIPIDeskew;


                ret = pSeninfDrv->setSeninf1NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);    // Disable main's NCSI first
                ret = pSeninfDrv->setSeninf2NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // Disable sub's NCSI first

                ret = pSeninfDrv->setTg1ViewFinderMode(0); //Disable tg1
                ret = pSeninfDrv->setTg2ViewFinderMode(0); //Disable tg2

                ret = pSeninfDrv->setTg1GrabRange(psensorPara->u4PixelX0, psensorPara->u4PixelX1, psensorPara->u4PixelY0, psensorPara->u4PixelY1);

                //Move to isp driver
                ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)psensorPara->inDataType, (SENSOR_DATA_BITS_ENUM)psensorPara->senInLsb, psensorDynamicInfo->pixelMode,
                    psensorPara->inSrcTypeSel == SERIAL_SENSOR ?1 :0);
                ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1,
                    psensorPara->u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);

                ret = pSeninfDrv->setTg1ViewFinderMode(psensorPara->u1IsContinuous? 0 : 1 );

                if (psensorPara->inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                    psensorDynamicInfo->HDRPixelMode = TWO_PIXEL_MODE;
                    psensorDynamicInfo->PDAFPixelMode = TWO_PIXEL_MODE;
                    //if(psensorPara->mipiPad == 0) {
                        if (psensorPara->inSrcTypeSel == VIRTUAL_CHANNEL_1) {
                            /***  1 input, 2 output ***/
                            /* Seninf 1 --> sensor Mux1 and sensor Mux3*/
                            //sensor interface control (0x1A04_0100)
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, MIPI_SENSOR);
                            //sensor muxer (0x1A04_0120)
                            ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);

                            //virtual channel using CAM_SV(x1A04_0900)
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, VIRTUAL_CHANNEL_1);
                            ret = pSeninfDrv->setSeninfMuxCtrl(3, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->HDRPixelMode);
                            // Sensor interface for VC
                            MY_LOGD("HDR VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                     gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                     gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                            ret = pSeninfDrv->setSeninfVC(seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                            (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                            //NCSI2 control reg
                            ret = pSeninfDrv->setSeninfNCSI2(&seninfpara);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,seninfSrc);/*seninf1 ->TG1*/
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,seninfSrc);
                            psensorDynamicInfo->HDRInfo = CAM_SV_1;
                            psensorDynamicInfo->PDAFInfo  = CAM_TG_NONE;
                        }
                        else if(psensorPara->inSrcTypeSel == VIRTUAL_CHANNEL_2){
                            //sensor interface control (0x1A04_0100)
                            //ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, MIPI_SENSOR);
                            //sensor muxer (0x1A04_0120)
                            ret = pSeninfDrv->setSeninfMuxCtrl(1,psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);

                            //virtual channel using CAM_SV(0x1A04_0D00)
                           // ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, VIRTUAL_CHANNEL_2);
                            ret = pSeninfDrv->setSeninfMuxCtrl(4, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->PDAFPixelMode);
                            // Sensor interface for VC
                            MY_LOGD("PDAF VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                     gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                     gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                            ret = pSeninfDrv->setSeninfVC(seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                            (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                            //NCSI2 control reg
                            ret = pSeninfDrv->setSeninfNCSI2(&seninfpara);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,seninfSrc);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,seninfSrc);
                            psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
                            psensorDynamicInfo->PDAFInfo = CAM_SV_2;

                        }
                        else if (psensorPara->inSrcTypeSel == VIRTUAL_VC_12) {

                            //sensor interface control (0x1A04_0100)
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, MIPI_SENSOR);
                            //sensor muxer (0x1A04_0120)
                            ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);

                            //virtual channel using CAM_SV1(0x1A04_0900)
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, VIRTUAL_CHANNEL_1);
                            ret = pSeninfDrv->setSeninfMuxCtrl(3, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->HDRPixelMode);
                            //virtual channel using CAM_SV2(0x1A04_0D00)
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, VIRTUAL_CHANNEL_2);
                            ret = pSeninfDrv->setSeninfMuxCtrl(4,psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                                VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->PDAFPixelMode);

                            // Sensor interface for VC
                            MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                     gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                     gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                            ret = pSeninfDrv->setSeninfVC(seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                            (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                            //main_m
                            //NCSI2 control reg
                            ret = pSeninfDrv->setSeninfNCSI2(&seninfpara);

                            /*Top select 0x1A04_0008*/
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,seninfSrc);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,seninfSrc);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,seninfSrc);
                            psensorDynamicInfo->HDRInfo = CAM_SV_1;
                            psensorDynamicInfo->PDAFInfo = CAM_SV_2;
                        }
                        else {
                            MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",psensorPara->mipiPad);
                        }
                    //}
                }
                else if (psensorPara->inSrcTypeSel == MIPI_SENSOR) {
                    /*Don't use virtual information*/
                    psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
                    psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;
                    if(psensorPara->mipiPad == 0) {
                        if(psensorPara->MIPI_PHY_TYPE == 1) // CSI2
                        {
                            //sensor interface control (0x1A04_0100)
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, CSI2);
                            //sensor muxer (0x1A04_0120)
                            ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);
                            ret = pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, (SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel);
                            ret = pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    }
                    else if(psensorPara->mipiPad == 1) {
                        if(psensorPara->MIPI_PHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, CSI2);
                            ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);
                            pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, (SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel);
                            pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                    }
                    else if(psensorPara->mipiPad == 2) {
                        if(psensorPara->MIPI_PHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, CSI2);
                            ret = pSeninfDrv->setSeninfMuxCtrl(1, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode);
                            pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, (SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel);
                            pSeninfDrv->setSeninfNCSI2(&seninfpara);
                        }
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                    }
                    else {
                        MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",psensorPara->mipiPad);
                    }
                }
                else if (psensorPara->inSrcTypeSel == TEST_MODEL) {
                   /*Don't use virtual information*/
                   psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
                   psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;
                   ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, (SENINF_SOURCE_ENUM)psensorPara->inSrcTypeSel);
                   ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                   ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                }
                else {
                    MY_LOGE("[seninfControl]main type = %d not support \n",psensorPara->inSrcTypeSel);
                }
                psensorDynamicInfo->TgInfo = CAM_TG_1;
                psensorDynamicInfo->TgCLKInfo = psensorDrvInfo->SensorMCLKPLL;
                break;
            case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB):
                switch (currSensorDev) {
                    case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB):
                        MY_LOGD("seninfControl setting main &sub of man & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[0].mipiPad);

                        //main
                        ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                        ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);



                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                        ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                        //sub
                        ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                        ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);



                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                        ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                        if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }

                        }
                        else if ((sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }
                        }
                        else if((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR)) {
                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                        }
                        else {
                            MY_LOGE("[seninfControl]main type = %d,sub type = %d not support \n",sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel);
                        }

                        sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        sensorDynamicInfo[0].TgCLKInfo = sensorDrvInfo[0].SensorMCLKPLL;
                        sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                        sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;

                        break;

                    case SENSOR_DEV_MAIN://SENSOR_DEV_MAIN|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting main of man & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[0].mipiPad);
                        if ((sensorDynamicInfo[1].TgInfo & CAM_TG_2) || (sensorDynamicInfo[1].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                //PDAF/HDR force enable Two pixel mode
                                sensorDynamicInfo[0].HDRPixelMode = TWO_PIXEL_MODE;
                                sensorDynamicInfo[0].PDAFPixelMode = TWO_PIXEL_MODE;
                                if(sensorPara[0].mipiPad == 0) {
                                    if (sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[0].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[0].inSrcTypeSel == MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                }

                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }
                            sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[0].TgCLKInfo = sensorDrvInfo[0].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[1].TgInfo & CAM_TG_1) {//SENSOR_DEV_MAIN|SENSOR_DEV_SUB
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                if(sensorPara[0].mipiPad == 0) {
                                    if (sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC (0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg (0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[0].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {//SENSOR_DEV_MAIN|SENSOR_DEV_SUB
                                /*Don't use virtual information*/
                                sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                }

                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }
                            sensorDynamicInfo[0].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[0].TgCLKInfo = sensorDrvInfo[0].SensorMCLKPLL;
                        }
                        break;

                    case SENSOR_DEV_SUB://SENSOR_DEV_MAIN|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting sub of man & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[1].mipiPad);

                        if ( (sensorDynamicInfo[0].TgInfo & CAM_TG_1) || (sensorDynamicInfo[0].TgInfo == CAM_TG_NONE)) {

                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );
#ifndef USING_MTK_LDVT

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }
#else

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                //no virtual channel
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);

                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

#endif
                            sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[0].TgInfo & CAM_TG_2) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

                            sensorDynamicInfo[1].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;
                }

                break;

            case (SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2):
                switch (currSensorDev) {
                    case SENSOR_DEV_MAIN://SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2
                     //main2
                        MY_LOGD("seninfControl setting main of main & main2 tginfo0 %d, tginfo2 %d, mipi pad %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[2].TgInfo,sensorPara[0].mipiPad);

                        if ( (sensorDynamicInfo[2].TgInfo & CAM_TG_2) || (sensorDynamicInfo[2].TgInfo == CAM_TG_NONE)) {
                        //    ret = pSeninfDrv->setTg2GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                       //     ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                       //     ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;

                                if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfMuxCtrl(1, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfMuxCtrl(1, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                }
                                if(sensorPara[0].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                    sensorDynamicInfo[0].HDRPixelMode = TWO_PIXEL_MODE;
                                    sensorDynamicInfo[0].PDAFPixelMode = TWO_PIXEL_MODE;

                                    MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                             gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                             gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                    if (sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_1) {
                                        // Sensor interface for VC
                                            ret = pSeninfDrv->setSeninfVC((sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                                            ret = pSeninfDrv->setSeninfMuxCtrl(3, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,(sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2);
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[0].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[0].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        ret = pSeninfDrv->setSeninfVC((sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                                        ret = pSeninfDrv->setSeninfMuxCtrl(4, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, TWO_PIXEL_MODE);
                                       ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2, (sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2);

                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;
                                        sensorDynamicInfo[0].HDRInfo = CAM_TG_NONE;


                                    }
                                    else if (sensorPara[0].inSrcTypeSel == VIRTUAL_VC_12) {
                                        ret = pSeninfDrv->setSeninfVC((sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                    (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));

                                            ret = pSeninfDrv->setSeninfMuxCtrl(4, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,(sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2);
                                            ret = pSeninfDrv->setSeninfMuxCtrl(3, sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,(sensorPara[0].mipiPad == 0) ?SENINF_1 : SENINF_2);


                                        sensorDynamicInfo[0].PDAFInfo = CAM_SV_2;
                                        sensorDynamicInfo[0].HDRInfo = CAM_SV_1;

                                        }
                                    }
                                }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }

                            sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[0].TgCLKInfo = sensorDrvInfo[0].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[2].TgInfo & CAM_TG_1) {//SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2
                            MY_LOGE("sensorDynamicInfo[2].TgInfo & CAM_TG_1 never here, main2 never tg1 \n");
                        }

                        break;

                    case SENSOR_DEV_MAIN_2://SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2
                        //main2
                        MY_LOGD("seninfControl setting main2 of main & main2 tginfo0 %d, tginfo2 %d, mipi pad %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[2].TgInfo,sensorPara[2].mipiPad);

                        if ( (sensorDynamicInfo[0].TgInfo & CAM_TG_1) || (sensorDynamicInfo[0].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

//                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

  //                          ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );
                            if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;

                                if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfMuxCtrl(2, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfMuxCtrl(2, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else {
                                    MY_LOGE("main2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                }
                                if(sensorPara[2].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                    sensorDynamicInfo[2].HDRPixelMode = TWO_PIXEL_MODE;
                                    sensorDynamicInfo[2].PDAFPixelMode = TWO_PIXEL_MODE;

                                    MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                             gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                             gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                    if (sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_1) {
                                        // Sensor interface for VC
                                            ret = pSeninfDrv->setSeninfVC((sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                                            ret = pSeninfDrv->setSeninfMuxCtrl(5, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV3,(sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2);
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_3;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        ret = pSeninfDrv->setSeninfVC((sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));
                                        ret = pSeninfDrv->setSeninfMuxCtrl(6, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, TWO_PIXEL_MODE);
                                       ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV4, (sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2);

                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_4;
                                        sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;


                                    }
                                    else if (sensorPara[2].inSrcTypeSel == VIRTUAL_VC_12) {
                                        ret = pSeninfDrv->setSeninfVC((sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2,(gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                    (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03));

                                            ret = pSeninfDrv->setSeninfMuxCtrl(6, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV4,(sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2);
                                            ret = pSeninfDrv->setSeninfMuxCtrl(5, sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                                VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, TWO_PIXEL_MODE);
                                           ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV3,(sensorPara[2].mipiPad == 0) ?SENINF_1 : SENINF_2);


                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_4;
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_3;

                                        }
                                    }
                                }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main2 type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }

                            sensorDynamicInfo[2].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[0].TgInfo & CAM_TG_2) {//SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2
                        MY_LOGE("sensorDynamicInfo[0].TgInfo & CAM_TG_2 never here, main always tg1 \n");
                        #if 0
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                            if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[2].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                }
                            }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main2 type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }

                            sensorDynamicInfo[2].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                              #endif
                            }

                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;

                }
                break;
            case (SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB):
                switch (currSensorDev) {
                    case (SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB):
                        MY_LOGD("seninfControl setting main2 &sub of main2 & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[2].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[2].mipiPad);

                        //main2
                        ret = pSeninfDrv->setTg1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                        ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);



                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                        ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                        //sub
                        ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                        ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);



                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                        ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                        if ((sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            if(sensorPara[2].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[2].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[2].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }

                        }
                        else if ((sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }
                        }
                        else if((sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR)) {
                            if(sensorPara[2].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[2].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[2].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                        }
                        else {
                            MY_LOGE("[seninfControl]main2 type = %d,sub type = %d not support \n",sensorPara[2].inSrcTypeSel,sensorPara[1].inSrcTypeSel);
                        }

                        sensorDynamicInfo[2].TgInfo = CAM_TG_1;
                        sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                        sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                        sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;

                        break;

                    case SENSOR_DEV_MAIN_2://SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting main2 of main2 & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[2].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[2].mipiPad);
                        if ((sensorDynamicInfo[1].TgInfo & CAM_TG_2) || (sensorDynamicInfo[1].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                            if (sensorPara[2].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                //PDAF/HDR force enable Two pixel mode
                                sensorDynamicInfo[2].HDRPixelMode = TWO_PIXEL_MODE;
                                sensorDynamicInfo[2].PDAFPixelMode = TWO_PIXEL_MODE;
                                if(sensorPara[2].mipiPad == 0) {
                                    if (sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[2].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Main2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[2].inSrcTypeSel == MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                }

                            }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main2 type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }
                            sensorDynamicInfo[2].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[1].TgInfo & CAM_TG_1) {//SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                            if (sensorPara[2].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                if(sensorPara[2].mipiPad == 0) {
                                    if (sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[2].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC (0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg (0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[2].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[2].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[2].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Main2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {//SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB
                                /*Don't use virtual information*/
                                sensorDynamicInfo[2].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[2].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[2].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                                }

                            }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main2 type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }
                            sensorDynamicInfo[2].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                        }
                        break;

                    case SENSOR_DEV_SUB://SENSOR_DEV_MAIN_2|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting sub of main2 & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                            sensorDynamicInfo[2].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[1].mipiPad);

                        if ( (sensorDynamicInfo[2].TgInfo & CAM_TG_1) || (sensorDynamicInfo[2].TgInfo == CAM_TG_NONE)) {

                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );
#ifndef USING_MTK_LDVT

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }
#else

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                //no virtual channel
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);

                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

#endif
                            sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[2].TgInfo & CAM_TG_2) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

                            sensorDynamicInfo[1].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;
                }
                break;
#ifdef MTK_SUB2_IMGSENSOR
            case (SENSOR_DEV_SUB_2|SENSOR_DEV_SUB):
                switch (currSensorDev) {
                    case (SENSOR_DEV_SUB_2|SENSOR_DEV_SUB):
                        MY_LOGD("seninfControl setting sub2 &sub of sub2 & sub tginfo0, %d tginfo1 %d, mipi pad %d\n",
                        sensorDynamicInfo[3].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[3].mipiPad);

                        //sub2
                        ret = pSeninfDrv->setTg1GrabRange(sensorPara[3].u4PixelX0, sensorPara[3].u4PixelX1, sensorPara[3].u4PixelY0, sensorPara[3].u4PixelY1);

                        ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[3].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[3].senInLsb, sensorDynamicInfo[3].pixelMode);

                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[3].u1HsyncPol?0 : 1,
                        sensorPara[3].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[3].inDataType, sensorDynamicInfo[3].pixelMode);

                        ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[3].u1IsContinuous? 0 : 1 );

                        //sub
                        ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                        ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                            sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                        ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                        if ((sensorPara[3].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            if(sensorPara[3].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                  sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[3].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[3].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                  sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }

                        }
                        else if ((sensorPara[3].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[3].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                  sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                  sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                  sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }
                        }
                        else if((sensorPara[3].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR)) {
                            if(sensorPara[3].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                  sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[3].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                  sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[3].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                  sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                        }
                        else {
                            MY_LOGE("[seninfControl]sub2 type = %d,sub type = %d not support \n",sensorPara[3].inSrcTypeSel,sensorPara[1].inSrcTypeSel);
                        }

                        sensorDynamicInfo[3].TgInfo = CAM_TG_1;
                        sensorDynamicInfo[3].TgCLKInfo = sensorDrvInfo[3].SensorMCLKPLL;
                        sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                        sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;

                        break;

                    case SENSOR_DEV_SUB://SENSOR_DEV_SUB_2|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting sub of sub2 & sub tginfo0 %d tginfo1 %d, mipi pad %d\n",
                                sensorDynamicInfo[3].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[1].mipiPad);
                        if ((sensorDynamicInfo[3].TgInfo & CAM_TG_2) || (sensorDynamicInfo[3].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                            if (sensorPara[1].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                //PDAF/HDR force enable Two pixel mode
                                sensorDynamicInfo[1].HDRPixelMode = TWO_PIXEL_MODE;
                                sensorDynamicInfo[1].PDAFPixelMode = TWO_PIXEL_MODE;
                                if(sensorPara[1].mipiPad == 0) {
                                    if (sensorPara[1].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                        MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[1].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[3].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[1].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8100)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8120)
                                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[3].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                      (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Sub2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[3].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Sub2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }

                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                    //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                    ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                    MY_LOGE("[seninfControl]sub2 type = %d not support \n",sensorPara[3].inSrcTypeSel);
                            }
                            sensorDynamicInfo[1].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[3].TgInfo & CAM_TG_1) {//SENSOR_DEV_SUB_2|SENSOR_DEV_SUB
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                            if (sensorPara[1].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                                if(sensorPara[1].mipiPad == 0) {
                                    if (sensorPara[1].inSrcTypeSel == VIRTUAL_CHANNEL_1) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].HDRPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                      (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                    }
                                    else if(sensorPara[1].inSrcTypeSel == VIRTUAL_CHANNEL_2){
                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                             VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].PDAFPixelMode);
                                        //Cam SV Tg Grab
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC (0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                       (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),5);
                                        //NCSI2 control reg (0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[3].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_SV_2;

                                    }
                                    else if (sensorPara[1].inSrcTypeSel == VIRTUAL_VC_12) {

                                        //sensor interface control (0x8500)
                                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, MIPI_SENSOR);
                                        //sensor muxer (0x8520)
                                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                                        //virtual channel using CAM_SV1(0x8900)
                                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_1);
                                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].HDRPixelMode);
                                        //Cam SV Tg Grab for CAMSV1 (0x9400)
                                        ret = pSeninfDrv->setSV1GrabRange(0x00, gVCInfo.VC1_SIZEH, 0x00, gVCInfo.VC1_SIZEV);
                                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        //virtual channel using CAM_SV2(0x8D00)
                                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, VIRTUAL_CHANNEL_2);
                                        ret = pSeninfDrv->setSeninf4MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                                                            VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].PDAFPixelMode);
                                        //Cam SV Tg Grab for CAMSV2 (0x9c00)
                                        ret = pSeninfDrv->setSV2GrabRange(0x00, gVCInfo.VC2_SIZEH, 0x00, gVCInfo.VC2_SIZEV);
                                        ret = pSeninfDrv->setSV2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);
                                        ret = pSeninfDrv->setSV2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                                        // Sensor interface for VC(0x83e4)
                                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                                      (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                                        //NCSI2 control reg(0x83A0)
                                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                          sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                                        /*Top select 0x1500_8008*/
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV2,SENINF_1);
                                        sensorDynamicInfo[1].HDRInfo = CAM_SV_1;
                                        sensorDynamicInfo[1].PDAFInfo = CAM_SV_2;
                                    }
                                    else {
                                        MY_LOGE("Sub2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[3].mipiPad);
                                    }
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {//SENSOR_DEV_SUB_2|SENSOR_DEV_SUB
                                /*Don't use virtual information*/
                                sensorDynamicInfo[1].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[1].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                                                      sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Sub2 camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }

                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub2 type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }
                            sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                        }
                        break;

                    case SENSOR_DEV_SUB_2://SENSOR_DEV_SUB_2|SENSOR_DEV_SUB
                        MY_LOGD("seninfControl setting sub2 of sub2 & sub tginfo0 %d tginfo1 %d, mipi pad %d\n",
                                 sensorDynamicInfo[3].TgInfo,sensorDynamicInfo[1].TgInfo,sensorPara[3].mipiPad);

                        if ( (sensorDynamicInfo[1].TgInfo & CAM_TG_1) || (sensorDynamicInfo[1].TgInfo == CAM_TG_NONE)) {

                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[3].u4PixelX0, sensorPara[3].u4PixelX1, sensorPara[3].u4PixelY0, sensorPara[3].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[3].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[3].senInLsb, sensorDynamicInfo[3].pixelMode);

                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[3].u1HsyncPol?0 : 1,
                                                                sensorPara[3].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[3].inDataType, sensorDynamicInfo[3].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[3].u1IsContinuous? 0 : 1 );
#ifndef USING_MTK_LDVT

                            if (sensorPara[3].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[3].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[3].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[3].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                      sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else if(sensorPara[3].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                      sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[3].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                      sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[3].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[3].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[3].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }
#else

                            if (sensorPara[3].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[3].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[3].PDAFInfo = CAM_TG_NONE;
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                //no virtual channel
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                  sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);

                            }
                            else if (sensorPara[3].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[3].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[3].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[3].inSrcTypeSel);
                            }

#endif
                            sensorDynamicInfo[3].TgInfo = CAM_TG_2;
                            sensorDynamicInfo[3].TgCLKInfo = sensorDrvInfo[3].SensorMCLKPLL;
                        }
                        else if ( sensorDynamicInfo[1].TgInfo & CAM_TG_2) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[3].u4PixelX0, sensorPara[3].u4PixelX1, sensorPara[3].u4PixelY0, sensorPara[3].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[3].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[3].senInLsb, sensorDynamicInfo[3].pixelMode);

                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[3].u1HsyncPol?0 : 1,
                                                                sensorPara[3].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[3].inDataType, sensorDynamicInfo[3].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[3].u1IsContinuous? 0 : 1 );

                            if (sensorPara[3].inSrcTypeSel >= MIPI_SENSOR) {
                                /*Don't use virtual information*/
                                sensorDynamicInfo[3].HDRInfo = CAM_TG_NONE;
                                sensorDynamicInfo[3].PDAFInfo = CAM_TG_NONE;
                                if(sensorPara[3].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                      sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[3].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                    sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[3].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[3].u1MIPIDataTermDelay,sensorPara[3].u1MIPIDataSettleDelay,sensorPara[3].u1MIPIClkTermDelay,
                                                                      sensorPara[3].u1VsyncPol,sensorPara[3].u1MIPILaneNum,1,sensorPara[3].u1MIPIPacketECCOrder,0,sensorPara[3].NCSI2_HSRXDET_MODE,sensorPara[3].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[3].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[3].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[3].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum,0);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[3].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[3].padSel, (SENINF_SOURCE_ENUM)sensorPara[3].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[3].inSrcTypeSel);
                            }

                            sensorDynamicInfo[3].TgInfo = CAM_TG_1;
                            sensorDynamicInfo[3].TgCLKInfo = sensorDrvInfo[3].SensorMCLKPLL;
                        }
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error, meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;
                    }
            break;
#endif
#if 0
            case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB|SENSOR_DEV_MAIN_2)://should not support this case
                //main
                ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                //sub
                ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                //main2
                ret = pSeninfDrv->setSV1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );



                if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);

                }
                else if ((sensorPara[0].inSrcTypeSel== PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);
                }
                else if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);
                }
                else if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_4);
                }
                else {
                    MY_LOGE("[seninfControl]main type = %d,sub type  = %d, main2 type = %d not support \n",sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel,sensorPara[2].inSrcTypeSel);
                }
                sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                sensorDynamicInfo[0].TgCLKInfo = sensorDrvInfo[0].SensorMCLKPLL;
                sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                sensorDynamicInfo[1].TgCLKInfo = sensorDrvInfo[1].SensorMCLKPLL;
                sensorDynamicInfo[2].TgInfo = CAM_SV_1;
                sensorDynamicInfo[2].TgCLKInfo = sensorDrvInfo[2].SensorMCLKPLL;
                break;
            #endif
            default:
                MY_LOGE("Not support, meSensorDev = 0x%x",meSensorDev);
                break;
        }
#ifdef MTK_SUB2_IMGSENSOR
        MY_LOGD("Tg usage infomation: Main = %d, Sub = %d, Main_2 =%d, Sub_2=%d\n",sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo,sensorDynamicInfo[3].TgInfo);
        MY_LOGD("Tg CLK : Main = %d, Sub = %d, Main_2 =%d Sub_2 =%d\n",sensorDynamicInfo[0].TgCLKInfo,sensorDynamicInfo[1].TgCLKInfo,sensorDynamicInfo[2].TgCLKInfo,sensorDynamicInfo[3].TgCLKInfo);
#else
        MY_LOGD("Tg usage infomation: Main = %d, Sub = %d, Main_2 =%d\n",sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo);
        MY_LOGD("Tg CLK : Main = %d, Sub = %d, Main_2 =%d\n",sensorDynamicInfo[0].TgCLKInfo,sensorDynamicInfo[1].TgCLKInfo,sensorDynamicInfo[2].TgCLKInfo);
#endif
    }
    else {

        // ret = pSeninfDrv->setSeninf1CSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf2CSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf3CSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf4Scam(0, 0, 0, 0, 0, 0,0);
        // ret = pSeninfDrv->setSeninf4Parallel(0);
    }

    //pSeninfDrv->sendCommand(CMD_DEBUG_TASK, 0, 0, 0); // for observe seninf setting

    pSeninfDrv->destroyInstance();

    return ret;

}



/*******************************************************************************
*
********************************************************************************/
template <typename T>
inline void setDebugTag(T& a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = DBGEXIF_TAG(DEBUG_EXIF_MID_CAM_SENSOR, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HalSensor::setDebugInfo(IBaseCamExif *pIBaseCamExif)
{
    MUINT32 exifId;
    MINT32 ret = 0;

    //Exif debug info
#ifndef  USING_MTK_LDVT
    auto pDebugExif = MAKE_DebugExif();
    if  ( ! pDebugExif ) {
        return MFALSE;
    }

    auto pBufInfo = pDebugExif->getBufInfo(DEBUG_EXIF_KEYID_CAM);
    if  ( ! pBufInfo ) {
        return MFALSE;
    }

    auto it = pBufInfo->body_layout.find(DEBUG_EXIF_MID_CAM_SENSOR);
    if ( it == pBufInfo->body_layout.end() ) {
        return MFALSE;
    }

    auto const& info = it->second;
    switch ( info.version )
    {
    case 0:
    default:{
        using namespace dbg_cam_sensor_param_0;

        DEBUG_SENSOR_INFO_T sensorDebugInfo;

        setDebugTag(sensorDebugInfo, SENSOR_TAG_VERSION, (MUINT32)SENSOR_DEBUG_TAG_VERSION);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_COLORORDER, (MUINT32)sensorStaticInfo[0].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_DATATYPE, (MUINT32)sensorRawInfo[0].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[0].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_X, (MUINT32)sensorPara[0].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_Y, (MUINT32)sensorPara[0].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[0].u4PixelX1-sensorPara[0].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[0].u4PixelY1-sensorPara[0].u4PixelY0));
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_COLORORDER, (MUINT32)sensorStaticInfo[1].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_DATATYPE, (MUINT32)sensorRawInfo[1].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[1].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_X, (MUINT32)sensorPara[1].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_Y, (MUINT32)sensorPara[1].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[1].u4PixelX1-sensorPara[1].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[1].u4PixelY1-sensorPara[1].u4PixelY0));
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_COLORORDER, (MUINT32)sensorStaticInfo[2].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_DATATYPE, (MUINT32)sensorRawInfo[2].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[2].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_START_X, (MUINT32)sensorPara[2].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_START_Y, (MUINT32)sensorPara[2].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[2].u4PixelX1-sensorPara[2].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[2].u4PixelY1-sensorPara[2].u4PixelY0));

        ret = pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_EXIF_MID_CAM_SENSOR, (MUINTPTR)(&exifId));
        ret = pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, exifId, (MUINTPTR)(&sensorDebugInfo), sizeof(DEBUG_SENSOR_INFO_T));

        }break;
    }
#endif

    return ret;
}


MINT32 HalSensor::releaseI2CTrigLock()
{
    MINT32 ret;
    SensorDrv *const pSensorDrv = SensorDrv::get();

    ret = pSensorDrv->releaseI2CTriggerLock();
    return ret;
}



