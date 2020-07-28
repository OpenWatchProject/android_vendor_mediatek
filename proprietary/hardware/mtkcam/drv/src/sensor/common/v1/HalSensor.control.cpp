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

extern MUINT32 gmeSensorDev;

extern pthread_mutex_t gpower_info_mutex;


extern SENSORDRV_INFO_STRUCT sensorDrvInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
extern SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];

SensorDynamicInfo sensorDynamicInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];

//hwsync
#include <mtkcam/drv/IHwSyncDrv.h>
#include <mtkcam/utils/std/ULog.h>

extern HWSyncDrv* mpHwSyncDrv;

/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif


#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

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
    MUINT32 mipiPad = 0;
    MUINT32 framerate = 0;
    SENINF_CONFIG_STRUCT seninfPara;
    IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfParam->index)->getDeviceId();
    SENSORDRV_INFO_STRUCT   *pSensorDrvInfo     = &sensorDrvInfo[sensorIdx];
    SENSOR_HAL_RAW_INFO_STRUCT *pSensorRawInfo  = &sensorRawInfo[sensorIdx];
    ACDK_SENSOR_INFO2_STRUCT            *pInfo           = &pSensorDrvInfo->info;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pResolutionInfo = &pSensorDrvInfo->resolutionInfo;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SENSOR_DRIVER_SCENARIO_T scenarioconf;
#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagStart);
#endif

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
    seninfPara.sensor_idx = sensorIdx;

    if(pArrayOfParam->framerate) {
        // Unit : FPS , Driver Unit : 10*FPS
        framerate = pArrayOfParam->framerate * 10;
    } else {
        ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&(pArrayOfParam->scenarioId), (MUINTPTR)&framerate);
    }
    MY_LOGD("configure Scenario=%d, FPSx10=%d", pArrayOfParam->scenarioId, framerate);

    pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&(pArrayOfParam->scenarioId), (MUINTPTR)&seninfPara.PDAFMode);
    MY_LOGD("isPDAFsupport = %d, pdaf type =%d", seninfPara.PDAFMode, pInfo->PDAF_Support);

    if(seninfPara.PDAFMode != 0){//check pdaf capacity first
        seninfPara.PDAFMode = pInfo->PDAF_Support;
    }

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&(seninfPara.csi_para.vcInfo), (MUINTPTR)&(pArrayOfParam->scenarioId));

    MUINT8 *pSettleDelay = &pInfo->MIPIDataLowPwr2HSSettleDelayM0;
    MUINT8 *pDPCMType    = &pInfo->IMGSENSOR_DPCM_TYPE_PRE;

    /*Get line / frame number CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM*/
    if(pArrayOfParam->scenarioId > SENSOR_SCENARIO_ID_SLIM_VIDEO2 || pArrayOfParam->scenarioId < SENSOR_SCENARIO_ID_NORMAL_PREVIEW) {
        seninfPara.csi_para.dataSettleDelay = pInfo->MIPIDataLowPwr2HSSettleDelayM0;
        seninfPara.csi_para.dpcm = pInfo->IMGSENSOR_DPCM_TYPE_PRE;
    } else {
        seninfPara.csi_para.dataSettleDelay = pSettleDelay[pArrayOfParam->scenarioId];
        seninfPara.csi_para.dpcm = pDPCMType[pArrayOfParam->scenarioId];
    }


    seninfPara.scenarioId = pArrayOfParam->scenarioId;
    seninfPara.u1IsBypassSensorScenario = (MUINT8)pArrayOfParam->isBypassScenario;
    seninfPara.u1IsContinuous = (MUINT8)pArrayOfParam->isContinuous;
    seninfPara.cropWidth = pArrayOfParam->crop.w;
    seninfPara.cropHeight = pArrayOfParam->crop.h;
    seninfPara.frameRate = framerate;
    seninfPara.twopixelOn = pArrayOfParam->twopixelOn;
    seninfPara.debugMode = pArrayOfParam->debugMode;
    seninfPara.HDRMode = pArrayOfParam->HDRMode;

    pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&mipiPad);
    seninfPara.csi_para.mipiPad = (EMipiPort)mipiPad;
    seninfPara.csi_para.dataTermDelay = pInfo->MIPIDataLowPwr2HighSpeedTermDelayCount;
    seninfPara.csi_para.clkTermDelay = pInfo->MIPICLKLowPwr2HighSpeedTermDelayCount;
    seninfPara.csi_para.dlaneNum = pInfo->SensorMIPILaneNumber;
    seninfPara.csi_para.dataHeaderOrder = pInfo->SensorPacketECCOrder;
    seninfPara.csi_para.mipi_type = pInfo->MIPIsensorType;
    seninfPara.csi_para.HSRXDE = (MUINT)pInfo->SettleDelayMode;
    seninfPara.csi_para.mipi_deskew = (MIPI_DESKEW_ENUM)pInfo->SensorMIPIDeskew;
    seninfPara.csi_para.vsyncType = pInfo->SensorVsyncPolarity;

    seninfPara.scam_para.SCAM_DataNumber = (MUINT)pInfo->SCAM_DataNumber;
    seninfPara.scam_para.SCAM_DDR_En = (MUINT)pInfo->SCAM_DDR_En;
    seninfPara.scam_para.SCAM_CLK_INV = (MUINT)pInfo->SCAM_CLK_INV;
    seninfPara.scam_para.SCAM_DEFAULT_DELAY = (MUINT)pInfo->SCAM_DEFAULT_DELAY;
    seninfPara.scam_para.SCAM_CRC_En = (MUINT)pInfo->SCAM_CRC_En;
    seninfPara.scam_para.SCAM_SOF_src = (MUINT)pInfo->SCAM_SOF_src;
    seninfPara.scam_para.SCAM_Timout_Cali = (MUINT)pInfo->SCAM_Timout_Cali;

    seninfPara.parallel_para.u1HsyncPol = pInfo->SensorHsyncPolarity;
    seninfPara.parallel_para.u1VsyncPol = pInfo->SensorVsyncPolarity;

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //Test Mode use property parameter
    {
        property_get("vendor.debug.senif.hdrmode", value, "0");
        int hdrModeTest = atoi(value);
        if((hdrModeTest == HDR_RAW)||(hdrModeTest == HDR_CAMSV)||(hdrModeTest == HDR_RAW_ZHDR))
        {
            seninfPara.HDRMode = hdrModeTest;
        }
    }
    //PDAFTest Mode use property parameter
    {
        property_get("vendor.debug.senif.pdafmode", value, "0");
        int PDAFModeTest = atoi(value);
        if(PDAFModeTest == 1)
        {
            seninfPara.PDAFMode = 1;
        }
    }

    MY_LOGD("SenDev=%d, scenario=%d, HDR=%d, fps=%d, twopix=%d",
        sensorIdx,
        seninfPara.scenarioId,
        seninfPara.HDRMode,
        seninfPara.frameRate,
        seninfPara.twopixelOn);

    MUINT16 *pSensorGrab       = &pInfo->SensorGrabStartX_PRV;
    MUINT16 *pSensorResolution = &pResolutionInfo->SensorPreviewWidth;

    if(seninfPara.scenarioId > SENSOR_SCENARIO_ID_CUSTOM5 || seninfPara.scenarioId < SENSOR_SCENARIO_ID_NORMAL_PREVIEW) {
        MY_LOGE("Incorrect scenario %d", seninfPara.scenarioId);
    } else {
        pixelX0   = pSensorGrab[seninfPara.scenarioId * 2];
        pixelY0   = pSensorGrab[seninfPara.scenarioId * 2 + 1];
        srcWidth  = pSensorResolution[seninfPara.scenarioId * 2];
        srcHeight = pSensorResolution[seninfPara.scenarioId * 2 + 1];
    }

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

    // Source is from sensor
    switch(pSensorRawInfo->sensorType) {
        case IMAGE_SENSOR_TYPE_RAW: {
            // RAW
            seninfPara.u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;
            seninfPara.padSel = PAD_10BIT;//pad2cam_data_sel
            seninfPara.inDataType = RAW_10BIT_FMT;//cam_tg_input_fmt
            seninfPara.senInLsb = TG_12BIT;//cam_tg_path_cfg
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW8: {
            // RAW
            seninfPara.u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                seninfPara.padSel = PAD_8BIT_9_2;
            }
            else {
                seninfPara.padSel = PAD_8BIT_7_0;
            }
            seninfPara.inDataType = RAW_8BIT_FMT;
            seninfPara.senInLsb = TG_12BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW12: {
            // RAW
            seninfPara.u4PixelX0 = pixelX0 + ((srcWidth - pArrayOfParam->crop.w)>>1);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            seninfPara.padSel = PAD_10BIT;
            seninfPara.inDataType = RAW_12BIT_FMT;
            seninfPara.senInLsb = TG_12BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR: {
            // Yuv422 or YCbCr
            seninfPara.u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w * 2;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                seninfPara.padSel = PAD_8BIT_9_2;
            }
            else {
                seninfPara.padSel = PAD_8BIT_7_0;
            }
            seninfPara.inDataType = YUV422_FMT;
            seninfPara.senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB565: {
            // RGB565
            seninfPara.u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w * 2;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                seninfPara.padSel = PAD_8BIT_9_2;
            }
            else {
                seninfPara.padSel = PAD_8BIT_7_0;
            }
            seninfPara.inDataType = RGB565_MIPI_FMT;
            seninfPara.senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB888: {
            // RGB888
            seninfPara.u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w * 2;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                seninfPara.padSel = PAD_8BIT_9_2;
            }
            else {
                seninfPara.padSel = PAD_8BIT_7_0;
            }
            seninfPara.inDataType = RGB888_MIPI_FMT;
            seninfPara.senInLsb = TG_8BIT;
            break;
        }

        case IMAGE_SENSOR_TYPE_JPEG: {
            seninfPara.u4PixelX0 = pixelX0 + (srcWidth - pArrayOfParam->crop.w);
            seninfPara.u4PixelY0 = pixelY0 + ((srcHeight - pArrayOfParam->crop.h)>>1);
            seninfPara.u4PixelX1 = seninfPara.u4PixelX0 + pArrayOfParam->crop.w * 2;
            seninfPara.u4PixelY1 = seninfPara.u4PixelY0 + pArrayOfParam->crop.h;

            if (inDataFmt == 0) {
                seninfPara.padSel = PAD_8BIT_9_2;
            }
            else {
                seninfPara.padSel = PAD_8BIT_7_0;
            }
            seninfPara.inDataType = JPEG_FMT;
            seninfPara.senInLsb = TG_8BIT;
            break;
        }

        default:
            break;
    }

    seninfPara.inSrcTypeSel = MIPI_SENSOR;

    if (pInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
        seninfPara.inSrcTypeSel = MIPI_SENSOR;
        /*should get mipi data rate from sensor here*/

        seninfPara.csi_para.mipi_pixel_rate = 0;
        pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_MIPI_PIXEL_RATE,
            (MUINTPTR)&seninfPara.scenarioId,(MUINTPTR)&seninfPara.csi_para.mipi_pixel_rate, (MUINTPTR)&framerate);
    }
    else if (pInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
        seninfPara.inSrcTypeSel = PARALLEL_SENSOR;
    }
    else if (pInfo->SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
        seninfPara.inSrcTypeSel = SERIAL_SENSOR;
    }
    else {
        seninfPara.inSrcTypeSel = TEST_MODEL;
    }

    if(seninfPara.debugMode == 1)
    {
        seninfPara.inSrcTypeSel = TEST_MODEL;
    }

    seninfPara.pixel_rate = 0;
    pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_PIXEL_RATE,
               (MUINTPTR)&seninfPara.scenarioId,(MUINTPTR)&seninfPara.pixel_rate, (MUINTPTR)&framerate);

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_STREAMING_SUSPEND);
    ret = seninfControl(&seninfPara, pInfo, 1);

    // set each sensor scenario separately
    scenarioconf.sId = seninfPara.scenarioId;
    scenarioconf.InitFPS = framerate;
    scenarioconf.HDRMode = seninfPara.HDRMode;
    scenarioconf.PDAFMode = seninfPara.PDAFMode;
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
        MUINT hdr_lv = 0xffffffff;
        if (exposureTime > 0 && exposureTime_se > 0 && gain > 0) {
            if (pArrayOfParam->HDRMode == SENSOR_VHDR_MODE_ZVHDR && gain_se > 0) { // ZVHDR mode: needs dual gain
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(exposureTime), (MUINTPTR)&(exposureTime_se), (MUINTPTR)&(hdr_lv));
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&(gain), (MUINTPTR)&(gain_se));
            }
            else {
                ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(exposureTime), (MUINTPTR)&(exposureTime_se), (MUINTPTR)&(hdr_lv));
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

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
#endif

    ret =pSeninfDrv->calibrateSensor(&seninfPara);
    pSeninfDrv->destroyInstance();

    if (mpHwSyncDrv != NULL) {
        //hwsync drv, sendCommand that the sensor is configured
        ret = mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE, IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx), pArrayOfParam->scenarioId, framerate);
        if (ret<0) {
            MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ", sensorIdx);
            return MFALSE;
        }
    }

#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagEnd);
#endif

     return (ret == 0);
}

MINT HalSensor::setSensorMclk(IMGSENSOR_SENSOR_IDX sensorIdx, MINT32 pcEn) //CMMCLK: Main/sub, CMMCLK2:Main_2 (external signal design is not sync with internal signal in TG/I2C)
{
    SENINF_MCLK_PARA mclk_para;
    MINT32  ret = 0;
    MUINT32 mclk_src;

    SENSORDRV_INFO_STRUCT *pSensorDrvInfo = &sensorDrvInfo[sensorIdx];
    SensorDrv *const pSensorDrv = SensorDrv::get();

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
#endif

    mclk_para.sensorIdx = sensorIdx;
    mclk_para.mclkFreq = pSensorDrvInfo->info.SensorClockFreq;
    mclk_para.mclkPolarityLow = pSensorDrvInfo->info.SensorClockPolarity;
    mclk_para.mclkFallingCnt = pSensorDrvInfo->info.SensorClockFallingCount;
    mclk_para.mclkRisingCnt = pSensorDrvInfo->info.SensorClockRisingCount;

    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&mclk_para.pclkInv);
    if (ret < 0) {
        MY_LOGE("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x)", ret);
    }


    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&mclk_src);
    mclk_para.mclkIdx = (EMclkId)mclk_src;


    ret = pSeninfDrv->configMclk(&mclk_para, pcEn);
    if (ret < 0) {
        MY_LOGE("configMclk fail");
        return ret;
    }
    MY_LOGD("sensorIdx %d, mclk_src %d, SensorMCLKPLL %d",
        sensorIdx, mclk_src, mclk_para.mclkPLL);

    pSeninfDrv->destroyInstance();

    return ret;
}

MINT HalSensor::sendCommand(
    MUINT sensorDev,
    MUINTPTR cmd,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3)
{
    MINT32 ret = 0;

    IMGSENSOR_SENSOR_IDX sensorIdx = IMGSENSOR_SENSOR_IDX_MAP(sensorDev);

    SensorDrv *const pSensorDrv = SensorDrv::get();

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
#endif

    if (pSensorDrv)
        pSensorDrv->init();
    switch (cmd) {
    case SENSOR_CMD_GET_START_OF_EXPOSURE:
    {
        //arg1 scenario id, arg2 tSof, arg3 = EXP/tSoE;
        if (!arg2 || !arg3 || !arg1) {
            MY_LOGE("SENSOR_CMD_GET_START_OF_EXPOSURE fail arg1 %p arg2 %p arg3 %p", (MINT64*)arg1, (MINT64*)arg2, (MINT64*)arg3);
            break;
        }

        MINT32 offset = 0;
        MINT64 tSoF = (*((MINT64*) arg2));
        MINT64 tExp = (*((MINT64*) arg3));
        MINT64 tSoE = 0;
        int offset_predefined = property_get_int32("vendor.debug.seninf.offset_start_exposure", 0);
        if (offset_predefined != 0)
            tSoE = tSoF - tExp + offset_predefined;
        else {
            ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_OFFSET_TO_START_OF_EXPOSURE, arg1, (MUINTPTR)&offset);
#ifdef OFFSET_START_EXPOSURE
        if (offset == 0)
            offset = OFFSET_START_EXPOSURE;
#endif
            tSoE = tSoF - tExp + offset;
        }
        //MY_LOGD("SENSOR_CMD_GET_START_OF_EXPOSURE offset %d tSoF %lld, tExp %lld tSoE %lld offset_predefined %d",
          //  offset, tSoF, tExp, tSoE, offset_predefined);
        (*((MINT64*) arg3)) = tSoE;
    }
        break;
    case SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER:
    {
        SensorCropWinInfo cropInfo;
        MUINT32 pixelClock = 0;
        MUINT32 lineLength = 0;
        MUINT32 scenarioId = 0;
        if (arg2 == 0) {
          *((MUINT32*) arg1) = 0;
          MY_LOGW("no scenario id\n");
          break;
        } else
          scenarioId = *((MUINT32*) arg2);

        ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MUINTPTR)&lineLength);
        ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&scenarioId, (MUINTPTR)&cropInfo);
        ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_PIXEL_CLOCK_FREQ, (MUINTPTR)&pixelClock);

        if (pixelClock != 0) {
            *((MUINT32*) arg1) = (MUINT32)((double)(lineLength & 0xFFFF) * cropInfo.h2_tg_size * 1000000000 / pixelClock); /* unit: ns */
        } else {
            *((MUINT32*) arg1) = 0;
            MY_LOGW("Wrong pixel clock\n");
        }
        break;
    }
    case SENSOR_CMD_GET_SENSOR_PIXELMODE:
        {
#ifdef HALSENSOR_CAM_PIXEL_MODE
        *(MUINT32 *)arg3  = HALSENSOR_CAM_PIXEL_MODE;
        MY_LOGD("SENSOR_CMD_GET_SENSOR_PIXELMODE:scenario = %d, fps = %d, Pixelmode =%d", *(MUINT32 *)arg1, *(MUINT32 *)arg2, *(MUINT32 *)arg3);
#else
        MUINT64  pixel_rate[2] = {0, 0};
        pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_MIPI_PIXEL_RATE, arg1,(MUINTPTR)&pixel_rate[0], arg2);
        if (pixel_rate[0] == 0)
            MY_LOGE("SENSOR_CMD_GET_SENSOR_PIXELMODE: mipi pixel rate should not be 0");

        pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_PIXEL_RATE, arg1,(MUINTPTR)&pixel_rate[1], arg2);
        if (pixel_rate[1] == 0)
            MY_LOGE("CMD_SENSOR_GET_PIXEL_RATE: pixel rate should not be 0");
        pSeninfDrv->sendCommand(CMD_SENINF_GET_PIXEL_MODE, sensorIdx, (MUINTPTR)pixel_rate, arg3);
        MY_LOGD("SENSOR_CMD_GET_SENSOR_PIXELMODE:sensorIdx %d scenario = %d, fps = %d, Pixelmode =%d mipi pixel rate %llu pixel rate %llu",sensorIdx, *(MUINT32 *)arg1, *(MUINT32 *)arg2, *(MUINT32 *)arg3, pixel_rate[0], pixel_rate[1]);
#endif
        }
        break;

    case SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT:
        ret = pSeninfDrv->getN3DDiffCnt((MUINT32 *)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail!");
        }
        break;

    case SENSOR_CMD_GET_SENSOR_POWER_ON_STETE: /*LSC funciton need open after sensor Power On*/
        pthread_mutex_lock(&gpower_info_mutex);
        *((MUINT32*) arg1) = (gmeSensorDev & IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx));
        pthread_mutex_unlock(&gpower_info_mutex);
        break;

    case SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS: /*Pass1 deque fail would call this command*/
        switch(arg1){
            case 0:
                ret =  pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_TASK, sensorIdx); /* For CAM */
                break;
            case 1:
                ret =  pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_TASK_CAMSV, sensorIdx); /* For CAMSV */
                break;
            default:
                MY_LOGE("Unsupported module for seninf debug");
                break;
        }
        break;
#if 0
    case SENSOR_CMD_SET_SENINF_CAM_TG_MUX:
    {
        SensorDynamicInfo *psensorDynamicInfo = &sensorDynamicInfo[sensorDevId];
        MUINT32            initMux = CAM_TG_1;
        if(psensorDynamicInfo->TgInfo == *(MUINT32 *)arg1) {
            psensorDynamicInfo->TgInfo = *(MUINT32 *)arg2;
            initMux = CAM_TG_1;
        } else if(psensorDynamicInfo->HDRInfo == *(MUINT32 *)arg1) {
            psensorDynamicInfo->HDRInfo = *(MUINT32 *)arg2;
            initMux = CAM_SV_1;
        } else if(psensorDynamicInfo->PDAFInfo == *(MUINT32 *)arg1) {
            psensorDynamicInfo->PDAFInfo = *(MUINT32 *)arg2;
            initMux = CAM_SV_1;
        } else {
            MY_LOGE("Error Cam MUX Setting originalCamTG=%d targetCamTG=%d", *(MUINT32 *)arg1, *(MUINT32 *)arg2);
        }

        ret = pSeninfDrv->setSeninfCamTGMuxCtrl(*(MUINT32 *)arg2 - initMux, (SENINF_MUX_ENUM)pSeninfDrv->getSeninfCamTGMuxCtrl(*(MUINT32 *)arg1 - initMux));

        break;
    }
#endif
    case SENSOR_CMD_SET_TEST_PATTERN_OUTPUT:
    {
#define HALSENSOR_TEST_MODEL_DUMMY_PIXEL    16
#define HALSENSOR_TEST_MODEL_VSYNC          16
#define HALSENSOR_TEST_MODEL_LINE           4500
#define HALSENSOR_TEST_MODEL_PIXEL          6500
        if(arg1 == 1) {
         //   ret = pSeninfDrv->setSeninf1Ctrl(PAD_10BIT, TEST_MODEL);
         //   ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_MUX1, SENINF_1);
            /*FPS = (TM_DUMMYPXL+TM_PXL) * (TM_VSYNC+TM_LINE) / ISP clock */
         //   ret = pSeninfDrv->setTG1_TM_Ctl(SENINF1_CSI0, 1, HALSENSOR_TEST_MODEL_DUMMY_PIXEL, HALSENSOR_TEST_MODEL_VSYNC,HALSENSOR_TEST_MODEL_LINE,HALSENSOR_TEST_MODEL_PIXEL);
        } else {
            ret = pSensorDrv->sendCommand(sensorIdx, cmd, arg1, arg2, arg3);
            if(ret < 0) {
                MY_LOGE("[sendCommand] sendCommand fail! %d", (int)cmd);
            }
        }
        break;
    }
    case SENSOR_CMD_SET_DUAL_CAM_MODE: /*set dual cam or not*/
        ret = pSeninfDrv->sendCommand(CMD_SENINF_SET_DUAL_CAM_MODE, arg1);
        break;
    case SENSOR_CMD_DEBUG_GET_SENINF_METER:
        ret =  pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_PIXEL_METER, arg1);
        break;

#ifdef CONFIG_MTK_CAM_SECURE
        case SENSOR_CMD_SET_STREAMING_RESUME:
        {
            ret = pSeninfDrv->sendCommand(CMD_SENINF_CHECK_PIPE);
            ret = pSensorDrv->sendCommand(sensorIdx, cmd, arg1, arg2, arg3);
            if(ret < 0) {
                MY_LOGE("[sendCommand] sendCommand resume fail! %d", cmd);
            }
            break;
        }
        case SENSOR_CMD_SET_STREAMING_SUSPEND:
        {
            ret = pSeninfDrv->sendCommand(CMD_SENINF_RESUME_FREE);
            ret = pSensorDrv->sendCommand(sensorIdx, cmd, arg1, arg2, arg3);
            if(ret < 0) {
                MY_LOGE("[sendCommand] sendCommand suspend fail! %d\n", cmd);
            }
            break;
        }
#endif

    default:
        ret = pSensorDrv->sendCommand(sensorIdx, cmd, arg1, arg2, arg3);
        if(ret < 0) {
            MY_LOGE("[sendCommand] sendCommand fail! %d", (int)cmd);
        }
        break;
    }
    if (pSensorDrv)
        pSensorDrv->uninit();
    pSeninfDrv->destroyInstance();

    return ret;
}

MINT HalSensor::setSensorMclkDrivingCurrent(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MINT32 ret = 0;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    MUINT32 InputCurrent = sensorDrvInfo[sensorIdx].info.SensorDrivingCurrent;

#ifdef    MCLK_DRIVE_CURRENT_BY_PINCTRL
    ret = pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_SET_DRIVE_CURRENT, (MUINTPTR)&InputCurrent);
#else

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
#endif

    MUINT32 mclkSrc;

    pSensorDrv->sendCommand(sensorIdx, CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&mclkSrc);
    ret = pSeninfDrv->setMclkIODrivingCurrent((EMclkId)mclkSrc, sensorDrvInfo[sensorIdx].info.SensorDrivingCurrent);
    pSeninfDrv->destroyInstance();
#endif

    if (ret < 0) {
        MY_LOGE("The driving current for cam%d is wrong", sensorIdx);
    }

    return ret;
}

MINT HalSensor::seninfControl(SENINF_CONFIG_STRUCT *psensorPara, ACDK_SENSOR_INFO2_STRUCT *pInfo, MUINT8 enable)
{
    MINT ret = 0;

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
#endif

    IMGSENSOR_SENSOR_IDX sensorIdx = psensorPara->sensor_idx;

    SensorDynamicInfo       *psensorDynamicInfo = &sensorDynamicInfo[sensorIdx];
    //SENINF_CSI_PARA          seninfpara;

    MY_LOGD("seninfControl sensorIdx = %d, enable = %d sensorPara.inSrcTypeSel = %d", sensorIdx, enable ,psensorPara->inSrcTypeSel);

    if(!enable) {
        ret = pSeninfDrv->configSensorInput(psensorPara, psensorDynamicInfo, pInfo, false);
        return ret;
    }
    ret = pSeninfDrv->configSensorInput(psensorPara, psensorDynamicInfo, pInfo, true);
    MY_LOGD("Tg usage infomation: CAM = %d, HDR = %d, PDAF = %d", psensorDynamicInfo->TgInfo, psensorDynamicInfo->HDRInfo, psensorDynamicInfo->PDAFInfo);

    //pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_TASK, 0, 0, 0); // for observe seninf setting

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

    MINT32 ret = 0;
    (void) pIBaseCamExif;

    //Exif debug info
#if 0//ndef  USING_MTK_LDVT
    //    MUINT32 exifId;
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

