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
#include <utils/threads.h>

/* 2-pixel: TG_CROP_W * TG_CROP_H * fps / (1-VB_percentage) ) > [isp clk rate*margin]*/
/* 4-pixel: TG_CROP_W * TG_CROP_H * fps / (1-VB_percentage) ) > 2 x [isp clk rate*margin] */
//#define TWO_PIXEL_MODE_THRESHOLD (360000) /* widht*height*fps = 4000x3000x30 >= 450Mhz*0.8 */
/*DVFS, High:450Mhz, Medium:320Mhz*/
//Deskew Function
//#define ENABLE_CSI_AUTO_DEKSKEW 1

#define HALSENSOR_PIXEL_MODE TWO_PIXEL_MODE

extern MUINT32 gmeSensorDev;

extern pthread_mutex_t gpower_info_mutex;

extern SENSORDRV_INFO_STRUCT sensorDrvInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
extern SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];

SENSOR_CONFIG_STRUCT sensorPara[IMGSENSOR_SENSOR_IDX_MAX_NUM];
SensorDynamicInfo sensorDynamicInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
SENSOR_VC_INFO_STRUCT gVCInfo;

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


#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HalSensor::querySensorDynamicInfo(
   MUINT32 sensorIdx,
   SensorDynamicInfo *pSensorDynamicInfo
)
{
    IMGSENSOR_SENSOR_IDX idx = IMGSENSOR_SENSOR_IDX_MAP(sensorIdx);

    if(idx != IMGSENSOR_SENSOR_IDX_NONE) {
        memcpy(pSensorDynamicInfo, &sensorDynamicInfo[idx], sizeof(SensorDynamicInfo));
        return MTRUE;
    } else {
        return MFALSE;
    }
}

MBOOL HalSensor::configure(
    MUINT const         uCountOfParam,
    ConfigParam const  *pArrayOfParam
)
{
    MINT32 ret = MFALSE;
    MINT32 pixelX0 = 0, pixelY0 = 0;
    MINT32 srcWidth = 0,srcHeight = 0;
    MUINT32 inDataFmt = 0;
    MUINT32 framerate;
    IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfParam->index)->getDeviceId();
    SENSOR_CONFIG_STRUCT    *pSensorPara        = &sensorPara[sensorIdx];
    SENSORDRV_INFO_STRUCT   *pSensorDrvInfo     = &sensorDrvInfo[sensorIdx];
    SensorStaticInfo        *pSensorStaticInfo  = &sensorStaticInfo[sensorIdx];
    SENSOR_HAL_RAW_INFO_STRUCT *pSensorRawInfo  = &sensorRawInfo[sensorIdx];

    SENSOR_VC_INFO_STRUCT VcInfo;
    SensorDrv *const pSensorDrv = SensorDrv::get();
    SENSOR_DRIVER_SCENARIO_T scenarioconf;
#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagStart);
#endif
    MUINT32 SW_deskewEnable = 0, HW_deskewEnable = 0;

    Mutex::Autolock _l(mMutex);

    (void)uCountOfParam;
    pthread_mutex_lock(&gpower_info_mutex);
    if((gmeSensorDev & IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx))== 0) {
        MY_LOGW("Should powerOn before configure gmeSensorDev %d, sensorIdx %d!!",
            gmeSensorDev, sensorIdx);
        pthread_mutex_unlock(&gpower_info_mutex);
        return MFALSE;
    }
    pthread_mutex_unlock(&gpower_info_mutex);

    if(pArrayOfParam->isBypassScenario != 0) {
        return MFALSE;
    }

    if(pArrayOfParam->framerate) {
        // Unit : FPS , Driver Unit : 10*FPS
        framerate = pArrayOfParam->framerate * 10;
    } else {
        ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&(pArrayOfParam->scenarioId), (MUINTPTR)&framerate);
    }
    MY_LOGD("configure Scenario=%d, FPSx10=%d\n", pArrayOfParam->scenarioId, framerate);

    /*PDAF Virtual channel*/
    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    if((pSensorDrvInfo->PDAF_Support == 2)||(pSensorDrvInfo->PDAF_Support == 3)||(pSensorDrvInfo->PDAF_Support == 5))
    {
        pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&(pArrayOfParam->scenarioId), (MUINTPTR)&pSensorPara->PDAFMode);
        MY_LOGD("PDAF=%d,isPDAFsupport=%d\n", pSensorPara->PDAFMode, pSensorDrvInfo->PDAF_Support);
    }

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&(pArrayOfParam->scenarioId));
    memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));

    /*Get line / frame number CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM*/
    switch(pArrayOfParam->scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            pSensorPara->u1MIPIDataSettleDelay=pSensorDrvInfo->MIPIDataLowPwr2HSSettleDelayM0;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_PRE;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            pSensorPara->u1MIPIDataSettleDelay=pSensorDrvInfo->MIPIDataLowPwr2HSSettleDelayM1;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_CAP;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            pSensorPara->u1MIPIDataSettleDelay=pSensorDrvInfo->MIPIDataLowPwr2HSSettleDelayM2;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_VD;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            pSensorPara->u1MIPIDataSettleDelay=pSensorDrvInfo->MIPIDataLowPwr2HSSettleDelayM3;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_VD1;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            pSensorPara->u1MIPIDataSettleDelay=pSensorDrvInfo->MIPIDataLowPwr2HSSettleDelayM4;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_VD2;
            break;
        default:
            pSensorPara->u1MIPIDataSettleDelay = pSensorDrvInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount;
            pSensorPara->DPCM_TYPE=pSensorDrvInfo->IMGSENSOR_DPCM_TYPE_PRE;
            break;
    }

    pSensorPara->scenarioId = pArrayOfParam->scenarioId;
    pSensorPara->u1IsBypassSensorScenario = (MUINT8)pArrayOfParam->isBypassScenario;
    pSensorPara->u1IsContinuous = (MUINT8)pArrayOfParam->isContinuous;
    pSensorPara->u1MIPIDataTermDelay = pSensorDrvInfo->MIPIDataLowPwr2HighSpeedTermDelayCount;
    //pSensorPara->u1MIPIDataSettleDelay = pSensorDrvInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount;
    pSensorPara->u1MIPIClkTermDelay = pSensorDrvInfo->MIPICLKLowPwr2HighSpeedTermDelayCount;
    pSensorPara->u1MIPILaneNum = pSensorDrvInfo->SensorMIPILaneNumber;
    pSensorPara->u1MIPIPacketECCOrder = pSensorDrvInfo->SensorPacketECCOrder;
    pSensorPara->MIPI_PHY_TYPE = (MUINT)pSensorDrvInfo->MIPIsensorType;
    pSensorPara->NCSI2_HSRXDET_MODE = (MUINT)pSensorDrvInfo->SettleDelayMode;
    pSensorPara->cropWidth = pArrayOfParam->crop.w;
    pSensorPara->cropHeight = pArrayOfParam->crop.h;
    pSensorPara->frameRate = framerate;
    pSensorPara->twopixelOn = pArrayOfParam->twopixelOn;
    pSensorPara->debugMode = pArrayOfParam->debugMode;
    pSensorPara->HDRMode = pArrayOfParam->HDRMode;
    pSensorPara->SCAM_DataNumber = (MUINT)pSensorDrvInfo->SCAM_DataNumber;
    pSensorPara->SCAM_DDR_En = (MUINT)pSensorDrvInfo->SCAM_DDR_En;
    pSensorPara->SCAM_CLK_INV = (MUINT)pSensorDrvInfo->SCAM_CLK_INV;
    pSensorPara->SCAM_DEFAULT_DELAY = (MUINT)pSensorDrvInfo->SCAM_DEFAULT_DELAY;
    pSensorPara->SCAM_CRC_En = (MUINT)pSensorDrvInfo->SCAM_CRC_En;
    pSensorPara->SCAM_SOF_src = (MUINT)pSensorDrvInfo->SCAM_SOF_src;
    pSensorPara->SCAM_Timout_Cali = (MUINT)pSensorDrvInfo->SCAM_Timout_Cali;
    pSensorPara->SensorMIPIDeskew = 0;
    pSensorPara->u1HsyncPol = pSensorDrvInfo->SensorHsyncPolarity;
    pSensorPara->u1VsyncPol = pSensorDrvInfo->SensorVsyncPolarity;

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //Test Mode use property parameter
    {
        property_get("vendor.debug.senif.hdrmode", value, "0");
        int hdrModeTest = atoi(value);
        if((hdrModeTest == 1)||(hdrModeTest == 2)||(hdrModeTest == 9))
        {
            pSensorPara->HDRMode = hdrModeTest;
        }
    }
    //PDAFTest Mode use property parameter
    {
        property_get("vendor.debug.senif.pdafmode", value, "0");
        int PDAFModeTest = atoi(value);
        if(PDAFModeTest == 1)
        {
            pSensorPara->PDAFMode = 1;
        }
    }

    MY_LOGD("SenDev=%d, scenario=%d, HDR=%d, fps=%d, twopix=%d\n",
        sensorIdx,
        pSensorPara->scenarioId,
        pSensorPara->HDRMode,
        pSensorPara->frameRate,
        pSensorPara->twopixelOn);

    switch (pSensorPara->scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_PRV;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_PRV;
            srcWidth = pSensorDrvInfo->SensorPreviewWidth;
            srcHeight = pSensorDrvInfo->SensorPreviewHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CAP;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CAP;
            srcWidth = pSensorDrvInfo->SensorCapWidth;
            srcHeight = pSensorDrvInfo->SensorCapHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_VD;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_VD;
            srcWidth = pSensorDrvInfo->SensorVideoWidth;
            srcHeight = pSensorDrvInfo->SensorVideoHeight;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_VD1;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_VD1;
            srcWidth = pSensorDrvInfo->SensorVideo1Width;
            srcHeight = pSensorDrvInfo->SensorVideo1Height;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_VD2;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_VD2;
            srcWidth = pSensorDrvInfo->SensorVideo2Width;
            srcHeight = pSensorDrvInfo->SensorVideo2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CST1;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CST1;
            srcWidth = pSensorDrvInfo->SensorCustom1Width;
            srcHeight = pSensorDrvInfo->SensorCustom1Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CST2;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CST2;
            srcWidth = pSensorDrvInfo->SensorCustom2Width;
            srcHeight = pSensorDrvInfo->SensorCustom2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CST3;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CST3;
            srcWidth = pSensorDrvInfo->SensorCustom3Width;
            srcHeight = pSensorDrvInfo->SensorCustom3Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CST4;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CST4;
            srcWidth = pSensorDrvInfo->SensorCustom4Width;
            srcHeight = pSensorDrvInfo->SensorCustom4Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            pixelX0 = pSensorDrvInfo->SensorGrabStartX_CST5;
            pixelY0 = pSensorDrvInfo->SensorGrabStartY_CST5;
            srcWidth = pSensorDrvInfo->SensorCustom5Width;
            srcHeight = pSensorDrvInfo->SensorCustom5Height;
            break;
        default:
            MY_LOGE("main camera incorrect scenario");
            break;
    }

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

    // Source is from sensor
    switch(pSensorRawInfo->sensorType) {
        case IMAGE_SENSOR_TYPE_RAW: {
            // RAW
            pSensorPara->u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;
            pSensorPara->padSel = PAD_10BIT;//pad2cam_data_sel
            pSensorPara->inDataType = RAW_10BIT_FMT;//cam_tg_input_fmt
            pSensorPara->senInLsb = TG_12BIT;//cam_tg_path_cfg
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW8: {
            // RAW
            pSensorPara->u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                pSensorPara->padSel = PAD_8BIT_9_2;
            }
            else {
                pSensorPara->padSel = PAD_8BIT_7_0;
            }
            pSensorPara->inDataType = RAW_8BIT_FMT;
            pSensorPara->senInLsb = TG_12BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW12: {
            // RAW
            pSensorPara->u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            pSensorPara->padSel = PAD_10BIT;
            pSensorPara->inDataType = RAW_12BIT_FMT;
            pSensorPara->senInLsb = TG_12BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR: {
            // Yuv422 or YCbCr
            pSensorPara->u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w * 2;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                pSensorPara->padSel = PAD_8BIT_9_2;
            }
            else {
                pSensorPara->padSel = PAD_8BIT_7_0;
            }
            pSensorPara->inDataType = YUV422_FMT;
            pSensorPara->senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB565: {
            // RGB565
            pSensorPara->u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w * 2;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                pSensorPara->padSel = PAD_8BIT_9_2;
            }
            else {
                pSensorPara->padSel = PAD_8BIT_7_0;
            }
            pSensorPara->inDataType = RGB565_MIPI_FMT;
            pSensorPara->senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB888: {
            // RGB888
            pSensorPara->u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w * 2;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                pSensorPara->padSel = PAD_8BIT_9_2;
            }
            else {
                pSensorPara->padSel = PAD_8BIT_7_0;
            }
            pSensorPara->inDataType = RGB888_MIPI_FMT;
            pSensorPara->senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_JPEG: {
            pSensorPara->u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            pSensorPara->u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            pSensorPara->u4PixelX1 = pSensorPara->u4PixelX0 + pArrayOfParam->crop.w * 2;
            pSensorPara->u4PixelY1 = pSensorPara->u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                pSensorPara->padSel = PAD_8BIT_9_2;
            }
            else {
                pSensorPara->padSel = PAD_8BIT_7_0;
            }
            pSensorPara->inDataType = JPEG_FMT;
            pSensorPara->senInLsb = TG_8BIT;
            break;
        }

        default:
            break;
    }

    pSensorPara->inSrcTypeSel = 0;

    if (pSensorDrvInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
        pSensorPara->inSrcTypeSel = MIPI_SENSOR;
    }
    else if (pSensorDrvInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
        pSensorPara->inSrcTypeSel = PARALLEL_SENSOR;
    }
    else if (pSensorDrvInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
        pSensorPara->inSrcTypeSel = SERIAL_SENSOR;
    }
    else {
        pSensorPara->inSrcTypeSel = TEST_MODEL;
    }

    if(pSensorPara->debugMode == 1)
    {
        pSensorPara->inSrcTypeSel = TEST_MODEL;
    }
    //VR
    if(pSensorPara->HDRMode == 2)
    {
        pSensorPara->inSrcTypeSel = VIRTUAL_CHANNEL_1;
    }

    /* PDAF using VC2 */
    if(pSensorPara->PDAFMode == 1)
    {
        pSensorPara->inSrcTypeSel = VIRTUAL_CHANNEL_2;
        /*Mix mode : PDAF + HDR*/
        if(pSensorPara->HDRMode == 2)
        {
            pSensorPara->inSrcTypeSel = VIRTUAL_VC_12;
        }
    }


    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_STREAMING_SUSPEND);

    ret = seninfControl(1, sensorIdx);

    // set each sensor scenario separately
    scenarioconf.sId = pArrayOfParam->scenarioId;
    scenarioconf.InitFPS = framerate;
    scenarioconf.HDRMode = pArrayOfParam->HDRMode;
    scenarioconf.PDAFMode = pSensorPara->PDAFMode;
    if ((ret = pSensorDrv->setScenario(sensorIdx, scenarioconf)) < 0) {
        MY_LOGE("camera(%d) halSensorSetScenario fail ",sensorIdx);
        return MFALSE;
    }


    MY_LOGD("sensorIdx:%d, HDRMode:%d, exposureTime:%d, gain:%d, exposureTime_se:%d, gain_se:%d",
        sensorIdx, pArrayOfParam->HDRMode,
        pArrayOfParam->exposureTime, pArrayOfParam->gain,
        pArrayOfParam->exposureTime_se, pArrayOfParam->gain_se);

    if (pArrayOfParam->HDRMode != 0) { // vHDR mode
        MUINT exposureTime = pArrayOfParam->exposureTime;
        MUINT gain = pArrayOfParam->gain;
        MUINT exposureTime_se = pArrayOfParam->exposureTime_se;
        MUINT gain_se = pArrayOfParam->gain_se;
        if (exposureTime > 0 && exposureTime_se > 0 && gain > 0) {
            if (pArrayOfParam->HDRMode == SENSOR_VHDR_MODE_ZVHDR && gain_se > 0) { // ZVHDR mode: needs dual gain
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(exposureTime), (MUINTPTR)&(exposureTime_se));
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&(gain), (MUINTPTR)&(gain_se));
            }
            else {
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(exposureTime), (MUINTPTR)&(exposureTime_se));
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_SENSOR_GAIN, (MUINTPTR)&(gain));
            }
        }
    } else { // normal mode
        MUINT exposureTime = pArrayOfParam->exposureTime;
        MUINT gain = pArrayOfParam->gain;
        if (exposureTime > 0 && gain > 0) {
            ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_SENSOR_EXP_TIME, (MUINTPTR)&(exposureTime));
            ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_SENSOR_GAIN, (MUINTPTR)&(gain));
        }
    }

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_STREAMING_RESUME);

#if ENABLE_CSI_AUTO_DEKSKEW
#define MAIN_CSI_AUTO_DESKEW_THRESHOLD 1427*1000000 /*5642x4224x240x10= 1427_374080(1.5Gbps)*/
#define SUB_CSI_AUTO_DESKEW_THRESHOLD 1427*1000000
#define CSI_BLANKING_RATIO 1000

    if(pSensorPara->inSrcTypeSel == MIPI_SENSOR) {
        int bpp=10;
        if (pSensorStaticInfo->sensorType == SENSOR_TYPE_RAW){
            if(pSensorStaticInfo->rawSensorBit == RAW_SENSOR_10BIT)
                bpp = 10;//RAW10
            else
                bpp = 8;//RAW8
        } else {
            bpp = 16;//YUV422
        }

        MY_LOGD("cropWidth %d, cropHeight %d,frameRate %d,u1MIPILaneNum %d,CSI_BLANKING_RATIO %d\n",
        pSensorPara->cropWidth,pSensorPara->cropHeight,pSensorPara->frameRate,pSensorPara->u1MIPILaneNum+1,CSI_BLANKING_RATIO);
        MY_LOGD("main lane speed = %ld, auto deskew threshold = %ld\n",
        ((((pSensorPara->cropWidth * pSensorPara->cropHeight)/1000) * (pSensorPara->frameRate/10)*bpp)/(pSensorPara->u1MIPILaneNum+1)),
        (MAIN_CSI_AUTO_DESKEW_THRESHOLD/CSI_BLANKING_RATIO));

        if(((((pSensorPara->cropWidth * pSensorPara->cropHeight)/1000) * (pSensorPara->frameRate/10)*bpp)/(pSensorPara->u1MIPILaneNum+1))
            > (MAIN_CSI_AUTO_DESKEW_THRESHOLD/1000)){

            //Main Cam over 1.5Gbps should enable deskew funciton
            if(pSensorDrvInfo->SensorMIPIDeskew == 1)
            {
                SW_deskewEnable = 0;
                HW_deskewEnable = 1;
            }
            else
            {
                SW_deskewEnable = 0;
                HW_deskewEnable = 0;
            }
            pSensorPara->SensorMIPIDeskew = HW_deskewEnable;
        }
    }

    //SW Deskew Function
    if(SW_deskewEnable){
        SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
        pSeninfDrv->autoDeskewCalibrationSeninf(sensorIdx, pSensorPara->u1MIPILaneNum+1);
        pSeninfDrv->destroyInstance();
    }
#endif

    if (mpHwSyncDrv != NULL) {
        //hwsync drv, sendCommand that the sensor is configured
        ret = mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE, IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx), pArrayOfParam->scenarioId, framerate);
        if(ret<0)
        {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ", sensorIdx);
                return MFALSE;
        }
    }

#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagEnd);
#endif

    return (ret == 0);
}

MINT HalSensor::setTgPhase(IMGSENSOR_SENSOR_IDX sensorIdx, MINT32 pcEn) //CMMCLK: Main/sub, CMMCLK2:Main_2 (external signal design is not sync with internal signal in TG/I2C)
{
    MINT32  ret = 0;
    MUINT32 u4PadPclkInv;
    MINT32  clkCnt, mclk, mclkSel;
    MINT32  csr_tg_tmp_stp = 0;

    SENSORDRV_INFO_STRUCT *pSensorDrvInfo = &sensorDrvInfo[sensorIdx];

    MINT32 freq_indx = MCLK_26MHZ;
    MCLK_FREQ_ENUM supported_mclk_freq[MCLK_MAX] = {
        FREQ_26MHZ, FREQ_24MHZ, FREQ_52MHZ, FREQ_48MHZ, FREQ_26MHZ_2, FREQ_13MHZ, FREQ_12MHZ, FREQ_6MHZ,
    };

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

    static const char *propertyTg[IMGSENSOR_SENSOR_IDX_MAX_NUM] = {"vendor.debug.seninf.Tg1clk",
                                                                   "vendor.debug.seninf.Tg2clk",
                                                                   "vendor.debug.seninf.Tg3clk",
                                                                   "vendor.debug.seninf.Tg4clk"};

    MY_LOGD("[setTgPhase] Tg%dclk: %d\n", sensorIdx, pSensorDrvInfo->SensorClockFreq);
    MY_LOGD("sensorDev = %d, pcEn = %d\n", sensorIdx, pcEn);//JH debug

    /*get property*/
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(propertyTg[sensorIdx], value, "-1");

    int tgClk = atoi(value);

    if(tgClk > 0) {
        pSensorDrvInfo->SensorClockFreq = tgClk;
        MY_LOGD("[setTgPhase] setproperty Tg%dclk: %d \n", sensorIdx, pSensorDrvInfo->SensorClockFreq);
    }

    for (freq_indx = MCLK_26MHZ; freq_indx < MCLK_MAX; freq_indx ++){
        if (pSensorDrvInfo->SensorClockFreq == supported_mclk_freq[freq_indx]) {
            mclkSel = freq_indx;
            mclk = supported_mclk_freq[freq_indx];
            break;
        }
    }

    if (freq_indx == MCLK_MAX){
        MY_LOGE("Err-Input clock rate error, %d \n", pSensorDrvInfo->SensorClockFreq);
        return -EINVAL;
    }

/*bk_*/
    /* Target: 2*(11+1)*(1/24M) = 1Mhz */
    pSensorDrvInfo->SensorMCLKPLL = 1000;
    /*sub sample:csr_tg_tmp_stp */
    if(mclk == 24)
        csr_tg_tmp_stp = 11;/*unit :Khz. 1000kHz = 1Mhz */
    else /*mclk1 == 26000*/
        csr_tg_tmp_stp = 12;/*unit :Khz. 1000kHz = 1Mhz */

    clkCnt = (mclk + (pSensorDrvInfo->SensorClockFreq >> 1)) / pSensorDrvInfo->SensorClockFreq;
    // Maximum CLKCNT is 15
    clkCnt = clkCnt > 15 ? 15 : clkCnt - 1;
    MY_LOGD("mclk%d: %d, clkCnt1: %d \n",sensorIdx, mclk, clkCnt);

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv);

    if (ret < 0) {
        MY_LOGE("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x)\n", ret);
    }
    MY_LOGD("[setTgPhase] u4PadPclkInv(%d)\n", u4PadPclkInv);

    // Config TG, always use Camera PLL, 1: 48MHz, 2: 104MHz

    ret = pSeninfDrv->setMclk(sensorPara[sensorIdx].mclkSrc,
    pcEn, mclkSel /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
    clkCnt, pSensorDrvInfo->SensorClockPolarity ? 0 : 1,
    pSensorDrvInfo->SensorClockFallingCount, pSensorDrvInfo->SensorClockRisingCount, u4PadPclkInv,
    csr_tg_tmp_stp);

    if (ret < 0) {
        MY_LOGE("setTg1PhaseCounter fail\n");
        return ret;
    }

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

    IMGSENSOR_SENSOR_IDX sensorDevId = IMGSENSOR_SENSOR_IDX_MAP(sensorDevIdx);

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

    switch (cmd) {
    //0x1000

    case SENSOR_CMD_SET_SENSOR_EXP_TIME:
        /* Set Exposure time (unit : us) */
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME;
        pSensorDrv->sendCommand(sensorDevId,cmdId, (MUINTPTR)arg1);
        MY_LOGD("Exposure Time: %d \n", *(MUINT32 *) arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_DUAL_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_DUAL_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_FLICKER_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_FLICKER_FRAME_RATE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_VIDEO_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_VIDEO_FRAME_RATE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC:
        cmdId = CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_CCT_FEATURE_CONTROL:
        cmdId = CMD_SENSOR_SET_CCT_FEATURE_CONTROL;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    case SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA:
        cmdId = CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO: scenario = %d, frame rates = %d (10base) \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_SET_TEST_PATTERN_OUTPUT:
        cmdId = CMD_SENSOR_SET_TEST_PATTERN_OUTPUT;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_ESHUTTER_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_OB_LOCK:
        cmdId = CMD_SENSOR_SET_OB_LOCK;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MIN_MAX_FPS:
        cmdId = CMD_SENSOR_SET_MIN_MAX_FPS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME:
        /* Set Exposure time  and Frame time(unit : us) */
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_FRAME_TIME;
        ret = pSensorDrv->sendCommand(sensorDevId,cmdId, (MUINTPTR)arg1, (MUINTPTR)arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, (MUINTPTR)arg1);
        MY_LOGD("SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE Time: %d \n", *(MUINT32 *) arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE_BUF_MODE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        MY_LOGD("SENSOR_CMD_SET_SENSOR_EXP_LINE_BUF_MODE: %d \n", *(MUINT32 *) arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE:
        cmdId = CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        MY_LOGD("CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE: %d \n", *(MUINT32 *) arg1);
        break;
    case SENSOR_CMD_SET_I2C_BUF_MODE_EN:
        cmdId = CMD_SENSOR_SET_I2C_BUF_MODE_EN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        MY_LOGD("SENSOR_CMD_SET_I2C_BUF_MODE_EN: %d \n", *(MUINT32 *) arg1);
        break;
    case SENSOR_CMD_SET_STREAMING_SUSPEND:
        cmdId = CMD_SENSOR_SET_STREAMING_SUSPEND;
        pSensorDrv->sendCommand(sensorDevId,cmdId);
        break;
    case SENSOR_CMD_SET_STREAMING_RESUME:
        cmdId = CMD_SENSOR_SET_STREAMING_RESUME;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
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
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_SHUTTER:
        cmdId = CMD_SENSOR_SET_HDR_SHUTTER;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_AWB_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    /*RGBW Sensor CMD*/
    case SENSOR_CMD_SET_SENSOR_AWB_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_AWB_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_ISO:
        cmdId = CMD_SENSOR_SET_SENSOR_ISO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    /*End of RGBW CMD*/
    //0x2000
    case SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT:
        cmdId = CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2); //arg1 = mode
        break;

    case SENSOR_CMD_GET_PIXEL_CLOCK_FREQ:
        cmdId = CMD_SENSOR_GET_PIXEL_CLOCK_FREQ;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        cmdId = CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_FEATURE_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_FEATURE_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE:
        cmdId = CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_TEMPERATURE_VALUE:
        cmdId = CMD_SENSOR_GET_TEMPERATURE_VALUE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO: scenario = %d, crop = 0x%x  \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_PIXELMODE:
        *(MUINT32 *)arg3  = HALSENSOR_PIXEL_MODE;
        MY_LOGD("SENSOR_CMD_GET_SENSOR_PIXELMODE:scenario = %d, fps = %d, Pixelmode =%d \n", *(MUINT32 *)arg1, *(MUINT32 *)arg2, *(MUINT32 *)arg3);
        break;

    case SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT:
        ret = pSeninfDrv->getN3DDiffCnt((MUINT32 *)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail! \n");
        }
        break;
   case SENSOR_CMD_GET_SENSOR_VC_INFO:
       ret = pSensorDrv->sendCommand(sensorDevId, CMD_SENSOR_GET_SENSOR_VC_INFO, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_VC_INFO fail! \n");
        }
       break;
    case SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING:
       ret = pSensorDrv->sendCommand(sensorDevId, CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING fail! \n");
       }
       break;
    case SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING:
        ret = pSensorDrv->sendCommand(sensorDevId, CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING, arg1, arg2);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING fail! \n");
        }
        break;
    case SENSOR_CMD_GET_SENSOR_PDAF_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_PDAF_INFO: scenario = %x %x\n", *(MUINT32 *)arg1, *(MUINT32 *)arg2);
        if(ret < 0) {
             MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_PDAF_INFO fail! \n");
        }
        break;
	case SENSOR_CMD_SET_PDFOCUS_AREA:
		cmdId = CMD_SET_PDFOCUS_AREA;
		pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
		break;
	case SENSOR_CMD_GET_PDAF_DATA:
        cmdId = CMD_SENSOR_GET_PDAF_DATA;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_4CELL_SENSOR:
        cmdId = CMD_SENSOR_GET_4CELL_DATA;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_SENSOR_HDR_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_HDR_CAPACITY;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    //0x3000
    case SENSOR_CMD_SET_YUV_FEATURE_CMD:
        cmdId = CMD_SENSOR_SET_YUV_FEATURE_CMD;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE:
        cmdId = CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CANCEL_AF:
        cmdId = CMD_SENSOR_SET_YUV_CANCEL_AF;
        pSensorDrv->sendCommand(sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CONSTANT_AF:
        cmdId = CMD_SENSOR_SET_YUV_CONSTANT_AF;
        pSensorDrv->sendCommand(sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_INFINITY_AF:
        cmdId = CMD_SENSOR_SET_YUV_INFINITY_AF;
        pSensorDrv->sendCommand(sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_AF_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AF_WINDOW;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_AE_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AE_WINDOW;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_GAIN_AND_EXP_LINE:
        cmdId = CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_3A_CMD:
        cmdId = CMD_SENSOR_SET_YUV_3A_CMD;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    //0x4000
    case SENSOR_CMD_GET_YUV_AF_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AF_STATUS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AE_STATUS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AWB_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AWB_STATUS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EXIF_INFO:
        cmdId = CMD_SENSOR_GET_YUV_EXIF_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_DELAY_INFO:
        cmdId = CMD_SENSOR_GET_YUV_DELAY_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_AE_AWB_LOCK:
        cmdId = CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_YUV_STROBE_INFO:
        cmdId = CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        cmdId = CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_YUV_AUTOTEST:
        cmdId = CMD_SENSOR_SET_YUV_AUTOTEST;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        cmdId=CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA:
        cmdId=CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_START_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_START_STREAMING;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_STOP_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_STOP_STREAMING;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER:
        /*Warning: Disable Rolling shutter support*/
        cmdId = CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER;
        //pSensorDrv->sendCommand(sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_POWER_ON_STETE: /*LSC funciton need open after sensor Power On*/
        pthread_mutex_lock(&gpower_info_mutex);
        *((MUINT32*) arg1) = (gmeSensorDev & IMGSENSOR_SENSOR_IDX2DUAL(sensorDevId));
        pthread_mutex_unlock(&gpower_info_mutex);
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
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_SENSOR_SYNC_MODE:
        cmdId = CMD_SENSOR_GET_SENSOR_SYNC_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_SYNC_MODE:
        cmdId = CMD_SENSOR_SET_SENSOR_SYNC_MODE;
        pSensorDrv->sendCommand(sensorDevId,cmdId, arg1);
        break;
    default:
        ret = -1;
        MY_LOGD("[sendCommand] err: 0x%x \n", cmd);
        break;
    }

    pSeninfDrv->destroyInstance();

    return ret;
}

MINT HalSensor::setSensorIODrivingCurrent(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MINT32 ret = 0;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

    ret = pSeninfDrv->setMclkIODrivingCurrent(sensorPara[sensorIdx].mclkSrc, sensorDrvInfo[sensorIdx].SensorDrivingCurrent);
    if (ret < 0) {
        MY_LOGE("The driving current for cam%d is wrong\n", sensorIdx);
    }

    pSeninfDrv->destroyInstance();

    return ret;
}

#define MULTI_SENSOR (gmeSensorDev == (SENSOR_DEV_MAIN|SENSOR_DEV_SUB))|| \
                     (gmeSensorDev == (SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2))|| \
                     (gmeSensorDev == (SENSOR_DEV_SUB|SENSOR_DEV_MAIN_2))|| \
                     (gmeSensorDev == (SENSOR_DEV_SUB|SENSOR_DEV_SUB_2))

MINT HalSensor::seninfControl(MUINT8 enable, IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MINT ret = 0;
    SENINF_CSI2_ENUM CSI_TYPE = CSI2_2_5G;
    SENINF_TOP_P1_ENUM SENINF_TOP_TG_selected = SENINF_TOP_TG1;
    SENINF_MUX_ENUM SENINF_MUX_selected = SENINF_MUX1;
    unsigned long TG_SEL = CAM_TG_NONE;
    SENINF_SOURCE_ENUM inSrcTypeSel = CSI2;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

    SENSOR_CONFIG_STRUCT *psensorPara = &sensorPara[sensorIdx];
    SensorDynamicInfo *psensorDynamicInfo = &sensorDynamicInfo[sensorIdx];
    SensorDynamicInfo *psensorDynamicInfo_other = NULL;
    SENSORDRV_INFO_STRUCT* psensorDrvInfo = &sensorDrvInfo[sensorIdx];
    seninf_csi_para seninfpara;

    MY_LOGD("seninfControl meSensorDev = %d, sensorIdx = %d, enable = %d\n", gmeSensorDev, sensorIdx, enable);
    MY_LOGD("sensorPara.inSrcTypeSel = %d\n", psensorPara->inSrcTypeSel);

    if (1 == enable) {
        pSeninfDrv->getCSIpara(psensorPara->mipiPad, &seninfpara);

        if(psensorPara->MIPI_PHY_TYPE == MIPI_CPHY){
            CSI_TYPE = CSI2_2_5G_CPHY; inSrcTypeSel = MIPI_SENSOR;
        }
        else{
            CSI_TYPE = CSI2_2_5G ; inSrcTypeSel = MIPI_SENSOR;
        }
        MY_LOGD("CSI2_IP=%d, MIPI_PHY_TYPE=%d psensorPara->mipiPad %d\n", seninfpara.CSI2_IP, psensorPara->MIPI_PHY_TYPE, psensorPara->mipiPad);

        seninfpara.padSel           = (PAD2CAM_DATA_ENUM)psensorPara->padSel;
        seninfpara.inSrcTypeSel     = inSrcTypeSel;
        seninfpara.dataTermDelay    = psensorPara->u1MIPIDataTermDelay;
        seninfpara.dataSettleDelay  = psensorPara->u1MIPIDataSettleDelay;
        seninfpara.clkTermDelay     = psensorPara->u1MIPIClkTermDelay;
        seninfpara.vsyncType        = psensorPara->u1VsyncPol;
        seninfpara.dlaneNum         = psensorPara->u1MIPILaneNum;
        seninfpara.Enable           = 1;
        seninfpara.dataheaderOrder  = psensorPara->u1MIPIPacketECCOrder;
        seninfpara.mipi_type        = CSI_TYPE;
        seninfpara.HSRXDE           = 0;
        seninfpara.dpcm             = psensorPara->DPCM_TYPE;
        seninfpara.MIPIDeskew       = psensorPara->SensorMIPIDeskew;

        switch(gmeSensorDev) {
            case SENSOR_DEV_MAIN:
            case SENSOR_DEV_SUB:
            case SENSOR_DEV_MAIN_2:
            case SENSOR_DEV_SUB_2:
                if(gmeSensorDev != IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx)) {
                    MY_LOGE("seninfControl current control not match. meSensorDev = %d, sensorIdx = %d\n",gmeSensorDev,sensorIdx);
                }

                SENINF_TOP_TG_selected = SENINF_TOP_TG1;
                TG_SEL = CAM_TG_1;
                SENINF_MUX_selected = SENINF_MUX1;

                ret = pSeninfDrv->setSeninf1CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);    // Disable all NCSI first
                ret = pSeninfDrv->setSeninf2CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);
                ret = pSeninfDrv->setSeninf3CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);

                break;
            case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB):
                switch (sensorIdx) {

                    case IMGSENSOR_SENSOR_IDX_MAIN://DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_SUB_SENSOR
                        MY_LOGD("seninfControl setting main of man & sub tginfo0, %d tginfo1 %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo);
                        psensorDynamicInfo_other = &sensorDynamicInfo[1];
                        break;
                    case IMGSENSOR_SENSOR_IDX_SUB://DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_SUB_SENSOR
                        MY_LOGD("seninfControl setting sub of man & sub tginfo0, %d tginfo1 %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo);
                                               /*Set Sensor interface parameter*/
                        psensorDynamicInfo_other = &sensorDynamicInfo[0];
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, sensorIdx = %d\n",gmeSensorDev,sensorIdx);
                        break;
                }
                break;

            case (SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2):
                switch (sensorIdx) {

                    case IMGSENSOR_SENSOR_IDX_MAIN://DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_MAIN_2_SENSOR
                        MY_LOGD("seninfControl setting main of main_main2 tginfo0(%d), tginfo2(%d)\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[2].TgInfo);
                        psensorDynamicInfo_other = &sensorDynamicInfo[2];
                        break;
                    case IMGSENSOR_SENSOR_IDX_MAIN2://DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_MAIN_2_SENSOR
                        //main2
                        MY_LOGD("seninfControl setting main2 of man & main2 tginfo0, %d tginfo2 %d\n",
                            sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[2].TgInfo);
                        psensorDynamicInfo_other = &sensorDynamicInfo[0];

                        break;
                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, sensorIdx = %d\n",gmeSensorDev,sensorIdx);
                        break;

                }
                break;
            case (SENSOR_DEV_SUB|SENSOR_DEV_MAIN_2):
				switch (sensorIdx) {

				  case IMGSENSOR_SENSOR_IDX_SUB:
				      MY_LOGD("seninfControl setting sub of sub_main2 tginfo1(%d), tginfo2(%d)\n",
				          sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo);
				      psensorDynamicInfo_other = &sensorDynamicInfo[2];
				      break;
				  case IMGSENSOR_SENSOR_IDX_MAIN2:
				      MY_LOGD("seninfControl setting main2 of sub & main2 tginfo1, %d tginfo2 %d\n",
				          sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo);
				      psensorDynamicInfo_other = &sensorDynamicInfo[1];
				      break;
				  default:
				      MY_LOGE("seninfControl mismatch error,  gmeSensorDev = %d, sensorIdx = %d\n",gmeSensorDev,sensorIdx);
				      break;

				}
				break;
            case (SENSOR_DEV_SUB|SENSOR_DEV_SUB_2):
				switch (sensorIdx) {
				  case IMGSENSOR_SENSOR_IDX_SUB:
				      MY_LOGD("seninfControl setting sub of sub_sub2 tginfo1(%d), tginfo2(%d)\n",
				          sensorDynamicInfo[1].TgInfo, sensorDynamicInfo[3].TgInfo);
				      psensorDynamicInfo_other = &sensorDynamicInfo[3];
				      break;
				  case IMGSENSOR_SENSOR_IDX_SUB2:
				      MY_LOGD("seninfControl setting sub2 of sub & sub2 tginfo1, %d tginfo2 %d\n",
				          sensorDynamicInfo[1].TgInfo, sensorDynamicInfo[3].TgInfo);
				      psensorDynamicInfo_other = &sensorDynamicInfo[1];
				      break;
				  default:
				      MY_LOGE("seninfControl mismatch error,  gmeSensorDev = %d, sensorIdx = %d\n",gmeSensorDev,sensorIdx);
				      break;

				}
				break;
            default:
                MY_LOGE("Not support, gmeSensorDev = 0x%x",gmeSensorDev);
                break;
        }

        if (MULTI_SENSOR){
            if (psensorDynamicInfo_other != NULL ){
                if (psensorDynamicInfo_other->TgInfo == CAM_TG_NONE) {
                    if ((sensorIdx == IMGSENSOR_SENSOR_IDX_MAIN) || ((gmeSensorDev & DUAL_CAMERA_MAIN_SENSOR)==0)){
                        SENINF_TOP_TG_selected = SENINF_TOP_TG1;
                        SENINF_MUX_selected = SENINF_MUX1;
                        TG_SEL         = CAM_TG_1;
                    } else {
                        SENINF_TOP_TG_selected = SENINF_TOP_TG2;
                        TG_SEL         = CAM_TG_2;
                        SENINF_MUX_selected = SENINF_MUX2;
                    }
                }
                else if (psensorDynamicInfo_other->TgInfo & CAM_TG_2) {
                    SENINF_TOP_TG_selected = SENINF_TOP_TG1;
                    SENINF_MUX_selected = SENINF_MUX1;
                    TG_SEL         = CAM_TG_1;
                }
                else if ( psensorDynamicInfo_other->TgInfo & CAM_TG_1) {
                    SENINF_TOP_TG_selected = SENINF_TOP_TG2;
                    TG_SEL         = CAM_TG_2;
                    SENINF_MUX_selected = SENINF_MUX2;
                } else {
                    MY_LOGE("Not supported tgInfo %d",psensorDynamicInfo_other->TgInfo);
                }
            }
        }

        //Fix pixel mode
        psensorDynamicInfo->pixelMode = HALSENSOR_PIXEL_MODE;

        if (TG_SEL != CAM_TG_NONE) {
            if (psensorPara->inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                //PDAF/HDR force enable Two pixel mode
                psensorDynamicInfo->HDRPixelMode = FOUR_PIXEL_MODE;
                psensorDynamicInfo->PDAFPixelMode = FOUR_PIXEL_MODE;
                if (psensorPara->inSrcTypeSel == VIRTUAL_CHANNEL_1) {
                    /***  1 input, 2 output ***/
                    /* Seninf 1 --> sensor Mux1 and sensor Mux3*/
                    ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX_selected, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode,CSI_TYPE);
                    //seninf muxer4(0x1a04_2d00)  for  HDR using virtual channel 1
                    ret = pSeninfDrv->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX3 :SENINF_MUX5), psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->HDRPixelMode,CSI_TYPE);
                    // Sensor interface for VC
                    MY_LOGD("HDR VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                             gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                             gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                    ret = pSeninfDrv->setSeninfVC(seninfpara.seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03), (gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03), (gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03), 0, 0);
                    ret = pSeninfDrv->setSeninfCSI2(&seninfpara);

                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, seninfpara.seninfSrc);/*seninf1 ->TG1*/
                    ret = pSeninfDrv->setSeninfTopMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_TOP_SV1 :SENINF_TOP_SV3), seninfpara.seninfSrc);
                    psensorDynamicInfo->HDRInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_1 :CAM_SV_3);
                    psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;
                }
                else if(psensorPara->inSrcTypeSel == VIRTUAL_CHANNEL_2){
                    /***  1 input, 2 output ***/
                    /* Seninf 1 --> sensor Mux1 and sensor Mux4*/
                    ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX_selected, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode,CSI_TYPE);
                    //seninf muxer4 for  PDAF using virtual channel 2
                    ret = pSeninfDrv->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX4 :SENINF_MUX6), psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->PDAFPixelMode,CSI_TYPE);
                    // Sensor interface for VC
                    MY_LOGD("PDAF VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                             gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                             gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                    ret = pSeninfDrv->setSeninfVC(seninfpara.seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03), (gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                    (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03), (gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03), 0, 0);
                    ret = pSeninfDrv->setSeninfCSI2(&seninfpara);

                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, seninfpara.seninfSrc);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_TOP_SV2 :SENINF_TOP_SV4), seninfpara.seninfSrc);
                    psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
                    psensorDynamicInfo->PDAFInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_2 :CAM_SV_4);

                }
                else if (psensorPara->inSrcTypeSel == VIRTUAL_VC_12) {
                    /***  1 input, 3 output ***/
                    /* Seninf 1 --> sensor Mux1, sensor Mux3 and Mux4*/
                    ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX_selected, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode,CSI_TYPE);
                    //seninf muxer3 for  mvhdr using virtual channel
                    ret = pSeninfDrv->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX3 :SENINF_MUX5), psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->HDRPixelMode,CSI_TYPE);
                    //seninf muxer4 for  PDAF using virtual channel 2
                    ret = pSeninfDrv->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX4 :SENINF_MUX6), psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                                                        VIRTUAL_CHANNEL_2, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->PDAFPixelMode,CSI_TYPE);
                    // Sensor interface for VC
                    MY_LOGD("HDR & PDAF VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                             gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                             gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                    ret = pSeninfDrv->setSeninfVC(seninfpara.seninfSrc, (gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03), (gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03), (gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03), 0, 0);
                    ret = pSeninfDrv->setSeninfCSI2(&seninfpara);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, seninfpara.seninfSrc);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(TG_SEL == CAM_TG_1 ?SENINF_TOP_SV1 :SENINF_TOP_SV3, seninfpara.seninfSrc);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(TG_SEL == CAM_TG_1 ?SENINF_TOP_SV2 :SENINF_TOP_SV4, seninfpara.seninfSrc);
                    psensorDynamicInfo->HDRInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_1 :CAM_SV_3);
                    psensorDynamicInfo->PDAFInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_2 :CAM_SV_4);
                }
            }
            else if (psensorPara->inSrcTypeSel == MIPI_SENSOR) {
                /*Don't use virtual information*/
                psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
                psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;
                //seninf muxer
                ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX_selected, psensorPara->u1HsyncPol?0 : 1, psensorPara->u1VsyncPol? 0 : 1,
                    MIPI_SENSOR, (TG_FORMAT_ENUM)psensorPara->inDataType, psensorDynamicInfo->pixelMode,CSI_TYPE);
                //CSI2 config
                ret = pSeninfDrv->setSeninfCSI2(&seninfpara);
                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, seninfpara.seninfSrc);
            }
            else if (psensorPara->inSrcTypeSel == TEST_MODEL) {
#define HALSENSOR_TEST_MODEL_DUMMY_PIXEL    16
#define HALSENSOR_TEST_MODEL_VSYNC          16
#define HALSENSOR_TEST_MODEL_LINE           4500
#define HALSENSOR_TEST_MODEL_PIXEL          6500

               /*need checking*/
               psensorDynamicInfo->HDRInfo = CAM_TG_NONE;
               psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

               ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)psensorPara->padSel, TEST_MODEL);
               ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, seninfpara.seninfSrc);
               ret = pSeninfDrv->setTG1_TM_Ctl(seninfpara.CSI2_IP,1,HALSENSOR_TEST_MODEL_DUMMY_PIXEL,HALSENSOR_TEST_MODEL_VSYNC,HALSENSOR_TEST_MODEL_LINE,HALSENSOR_TEST_MODEL_PIXEL);
            }
            else {
                MY_LOGE("[seninfControl]inSrcTypeSel type = %d not support \n",psensorPara->inSrcTypeSel);
            }
            psensorDynamicInfo->TgInfo = TG_SEL;
            psensorDynamicInfo->TgCLKInfo = psensorDrvInfo->SensorMCLKPLL;
        }

        MY_LOGD("Tg usage infomation: Main = %d, Sub = %d, Main_2 =%d\n",sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo);
        MY_LOGD("Tg CLK : Main = %d, Sub = %d, Main_2 =%d\n",sensorDynamicInfo[0].TgCLKInfo,sensorDynamicInfo[1].TgCLKInfo,sensorDynamicInfo[2].TgCLKInfo);
    }
    else {
        ret = pSeninfDrv->setSeninf1CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);    // Disable main's NCSI first
        ret = pSeninfDrv->setSeninf2CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // Disable sub's NCSI first
        ret = pSeninfDrv->setSeninf3CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // Disable sub's NCSI first
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

