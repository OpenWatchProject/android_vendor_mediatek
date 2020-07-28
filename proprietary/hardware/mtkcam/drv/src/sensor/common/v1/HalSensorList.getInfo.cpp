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

#define LOG_TAG "MtkCam/HalSensorList"
//
#include "MyUtils.h"
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/std/ULog.h>
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif

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
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

SensorStaticInfo sensorStaticInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
SENSORDRV_INFO_STRUCT sensorDrvInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
IMetadata  gSensorMetadata[IMGSENSOR_SENSOR_IDX_MAX_NUM];

/******************************************************************************
 *
 ******************************************************************************/
MVOID HalSensorList::querySensorDrvInfo(IMGSENSOR_SENSOR_IDX idx)
{
    SensorDrv *const pSensorDrv = SensorDrv::get();

    pSensorDrv->getInfo2(idx, &sensorDrvInfo[idx]);

    getRawInfo(idx, &sensorRawInfo[idx]);

    querySensorInfo(idx);//to remap data
}

MUINT HalSensorList::getRawInfo(IMGSENSOR_SENSOR_IDX idx, SENSOR_HAL_RAW_INFO_STRUCT *pInfo)
{
    MUINT32 ret = 0;

    pInfo->u1Order = sensorDrvInfo[idx].info.SensorOutputDataFormat;

    /* Modify getSensorType from getInfo Ioctl to directly compute sensorType from pInfo->u1Order
    //sensorType = pSensorDrv->getCurrentSensorType(sensorDevId);
    */

    // Compute sensorType from SensorOutputDataFormat
    if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW8;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_UYVY &&
                pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_YVYU) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_YUV;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_YCBCR;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW_RWB_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW_RWB_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order == SENSOR_OUTPUT_FORMAT_RAW_MONO) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order == SENSOR_OUTPUT_FORMAT_RAW8_MONO) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW8;
    }
    else {
        MY_LOGE("Unsupport SensorOutputDataFormat");
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }

    MY_LOGD("SensorOutputDataFormat: %d, ImageSensor Type: %d", pInfo->u1Order, pInfo->sensorType);


    switch (pInfo->sensorType) {
        case IMAGE_SENSOR_TYPE_RAW:
            pInfo->u4BitDepth = 10;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW8:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW12:
            pInfo->u4BitDepth = 12;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW14:
            pInfo->u4BitDepth = 14;
            pInfo->u4IsPacked = 1;
            break;

        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 0;
            break;
        case IMAGE_SENSOR_TYPE_RGB565:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 0;
            break;
        default:
            MY_LOGE("Unsupport Sensor Type");
            break;
    }

    return ret;
}

MVOID HalSensorList::querySensorInfo(IMGSENSOR_SENSOR_IDX idx)
{
    MUINTPTR data1,data2;

    SensorStaticInfo                   *pSensorStaticInfo = &sensorStaticInfo[idx];
    SENSORDRV_INFO_STRUCT              *pSensorDrvInfo  = &sensorDrvInfo[idx];
    ACDK_SENSOR_INFO2_STRUCT           *pInfo           = &pSensorDrvInfo->info;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pResolutionInfo = &pSensorDrvInfo->resolutionInfo;

    SensorDrv *const pSensorDrv = SensorDrv::get();

    pSensorStaticInfo->sensorDevID = pSensorDrv->getSensorID(idx);
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE, (MUINTPTR)&data1);
    pSensorStaticInfo->orientationAngle = data1;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_SENSOR_FACING_DIRECTION, (MUINTPTR)&data1);
    pSensorStaticInfo->facingDirection = data1;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_FAKE_ORIENTATION, (MUINTPTR)&data1);
    pSensorStaticInfo->fakeOrientation = data1;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_SENSOR_VIEWANGLE, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->horizontalViewAngle =data1;
    pSensorStaticInfo->verticalViewAngle = data2;

    data1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->previewFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->captureFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->videoFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->video1FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->video2FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM1;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom1FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM2;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom2FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM3;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom3FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM4;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom4FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM5;
    pSensorDrv->sendCommand(idx, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom5FrameRate = data2;

    switch (sensorRawInfo[idx].sensorType) {
        case IMAGE_SENSOR_TYPE_RAW:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_10BIT;
            break;
        case IMAGE_SENSOR_TYPE_RAW8:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_8BIT;
            break;

        case IMAGE_SENSOR_TYPE_RAW12:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_12BIT;
            break;

        case IMAGE_SENSOR_TYPE_RAW14:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_14BIT;
            break;
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_YUV;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        case IMAGE_SENSOR_TYPE_RGB565:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RGB;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        case IMAGE_SENSOR_TYPE_JPEG:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_JPEG;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        default:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_UNKNOWN;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
    }

     switch (sensorRawInfo[idx].u1Order) {
         case SENSOR_OUTPUT_FORMAT_RAW_B:
         case SENSOR_OUTPUT_FORMAT_RAW8_B:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_Gb:
         case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_Gr:
         case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_R:
         case SENSOR_OUTPUT_FORMAT_RAW8_R:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
         case SENSOR_OUTPUT_FORMAT_UYVY:
         case SENSOR_OUTPUT_FORMAT_CbYCrY:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_UYVY;
            break;
         case SENSOR_OUTPUT_FORMAT_VYUY:
         case SENSOR_OUTPUT_FORMAT_CrYCbY:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_VYUY;
            break;
         case SENSOR_OUTPUT_FORMAT_YUYV:
         case SENSOR_OUTPUT_FORMAT_YCbYCr:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_YUYV;
            break;
         case SENSOR_OUTPUT_FORMAT_YVYU:
         case SENSOR_OUTPUT_FORMAT_YCrYCb:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_YVYU;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_RWB_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_RWB_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_MONO:
         case SENSOR_OUTPUT_FORMAT_RAW8_MONO:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_MONO;
            break;
       case SENSOR_OUTPUT_FORMAT_RAW_4CELL_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
         case SENSOR_OUTPUT_FORMAT_RAW_4CELL_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
         default:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_NONE;
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_FMT_NONE;
            break;

     }

    /// Add SENSOR_RAW_PD for 3A use to distinguish whether enable pdc table or not
    /// Note: Only pdaf sensor rather than dual pd need to enable pdc table
    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(Full), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    switch(pInfo->PDAF_Support) {
        case 1: /* 1: PDAF Raw Data mode */
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_PD;
            break;
        default:
            break;
    }

    pSensorStaticInfo->iHDRSupport = pInfo->IHDR_Support;
    pSensorStaticInfo->PDAF_Support = pInfo->PDAF_Support;
    pSensorStaticInfo->HDR_Support = pInfo->HDR_Support;
    pSensorStaticInfo->previewWidth = pResolutionInfo->SensorPreviewWidth;
    pSensorStaticInfo->previewHeight = pResolutionInfo->SensorPreviewHeight;
    pSensorStaticInfo->captureWidth = pResolutionInfo->SensorFullWidth;
    pSensorStaticInfo->captureHeight = pResolutionInfo->SensorFullHeight;
    pSensorStaticInfo->videoWidth = pResolutionInfo->SensorVideoWidth;
    pSensorStaticInfo->videoHeight = pResolutionInfo->SensorVideoHeight;
    pSensorStaticInfo->video1Width = pResolutionInfo->SensorHighSpeedVideoWidth;
    pSensorStaticInfo->video1Height = pResolutionInfo->SensorHighSpeedVideoHeight;
    pSensorStaticInfo->video2Width = pResolutionInfo->SensorSlimVideoWidth;
    pSensorStaticInfo->video2Height = pResolutionInfo->SensorSlimVideoHeight;
    pSensorStaticInfo->SensorCustom1Width = pResolutionInfo->SensorCustom1Width;
    pSensorStaticInfo->SensorCustom1Height = pResolutionInfo->SensorCustom1Height;
    pSensorStaticInfo->SensorCustom2Width = pResolutionInfo->SensorCustom2Width;
    pSensorStaticInfo->SensorCustom2Height = pResolutionInfo->SensorCustom2Height;
    pSensorStaticInfo->SensorCustom3Width = pResolutionInfo->SensorCustom3Width;
    pSensorStaticInfo->SensorCustom3Height = pResolutionInfo->SensorCustom3Height;
    pSensorStaticInfo->SensorCustom4Width = pResolutionInfo->SensorCustom4Width;
    pSensorStaticInfo->SensorCustom4Height = pResolutionInfo->SensorCustom4Height;
    pSensorStaticInfo->SensorCustom5Width = pResolutionInfo->SensorCustom5Width;
    pSensorStaticInfo->SensorCustom5Height = pResolutionInfo->SensorCustom5Height;
    pSensorStaticInfo->previewDelayFrame = pInfo->PreviewDelayFrame;
    pSensorStaticInfo->captureDelayFrame = pInfo->CaptureDelayFrame;
    pSensorStaticInfo->videoDelayFrame = pInfo->VideoDelayFrame;
    pSensorStaticInfo->video1DelayFrame = pInfo->HighSpeedVideoDelayFrame;
    pSensorStaticInfo->video2DelayFrame = pInfo->SlimVideoDelayFrame;
    pSensorStaticInfo->Custom1DelayFrame = pInfo->Custom1DelayFrame;
    pSensorStaticInfo->Custom2DelayFrame = pInfo->Custom2DelayFrame;
    pSensorStaticInfo->Custom3DelayFrame = pInfo->Custom3DelayFrame;
    pSensorStaticInfo->Custom4DelayFrame = pInfo->Custom4DelayFrame;
    pSensorStaticInfo->Custom5DelayFrame = pInfo->Custom5DelayFrame;
    pSensorStaticInfo->aeShutDelayFrame = pInfo->AEShutDelayFrame;
    pSensorStaticInfo->aeSensorGainDelayFrame = pInfo->AESensorGainDelayFrame;
    pSensorStaticInfo->aeISPGainDelayFrame = pInfo->AEISPGainDelayFrame;
    pSensorStaticInfo->FrameTimeDelayFrame = pInfo->FrameTimeDelayFrame;
    pSensorStaticInfo->SensorGrabStartX_PRV = pInfo->SensorGrabStartX_PRV;
    pSensorStaticInfo->SensorGrabStartY_PRV = pInfo->SensorGrabStartY_PRV;
    pSensorStaticInfo->SensorGrabStartX_CAP = pInfo->SensorGrabStartX_CAP;
    pSensorStaticInfo->SensorGrabStartY_CAP = pInfo->SensorGrabStartY_CAP;
    pSensorStaticInfo->SensorGrabStartX_VD = pInfo->SensorGrabStartX_VD;
    pSensorStaticInfo->SensorGrabStartY_VD = pInfo->SensorGrabStartY_VD;
    pSensorStaticInfo->SensorGrabStartX_VD1 = pInfo->SensorGrabStartX_VD1;
    pSensorStaticInfo->SensorGrabStartY_VD1 = pInfo->SensorGrabStartY_VD1;
    pSensorStaticInfo->SensorGrabStartX_VD2 = pInfo->SensorGrabStartX_VD2;
    pSensorStaticInfo->SensorGrabStartY_VD2 = pInfo->SensorGrabStartY_VD2;
    pSensorStaticInfo->SensorGrabStartX_CST1 = pInfo->SensorGrabStartX_CST1;
    pSensorStaticInfo->SensorGrabStartY_CST1 = pInfo->SensorGrabStartY_CST1;
    pSensorStaticInfo->SensorGrabStartX_CST2 = pInfo->SensorGrabStartX_CST2;
    pSensorStaticInfo->SensorGrabStartY_CST2 = pInfo->SensorGrabStartY_CST2;
    pSensorStaticInfo->SensorGrabStartX_CST3 = pInfo->SensorGrabStartX_CST3;
    pSensorStaticInfo->SensorGrabStartY_CST3 = pInfo->SensorGrabStartY_CST3;
    pSensorStaticInfo->SensorGrabStartX_CST4 = pInfo->SensorGrabStartX_CST4;
    pSensorStaticInfo->SensorGrabStartY_CST4 = pInfo->SensorGrabStartY_CST4;
    pSensorStaticInfo->SensorGrabStartX_CST5 = pInfo->SensorGrabStartX_CST5;
    pSensorStaticInfo->SensorGrabStartY_CST5 = pInfo->SensorGrabStartY_CST5;

    pSensorStaticInfo->previewActiveWidth = pResolutionInfo->SensorEffectivePreviewWidth;//3d use onlyl?
    pSensorStaticInfo->previewActiveHeight = pResolutionInfo->SensorEffectivePreviewHeight;//3d use onlyl?
    pSensorStaticInfo->captureActiveWidth = pResolutionInfo->SensorEffectiveFullWidth;//3d use onlyl?
    pSensorStaticInfo->captureActiveHeight = pResolutionInfo->SensorEffectiveFullHeight;//3d use onlyl?
    pSensorStaticInfo->videoActiveWidth = pResolutionInfo->SensorEffectiveVideoWidth;//3d use onlyl?
    pSensorStaticInfo->videowActiveHeight = pResolutionInfo->SensorEffectiveVideoHeight;//3d use onlyl?
    pSensorStaticInfo->previewHoizontalOutputOffset = pResolutionInfo->SensorPreviewWidthOffset;//3d use onlyl?
    pSensorStaticInfo->previewVerticalOutputOffset = pResolutionInfo->SensorPreviewHeightOffset; //3d use onlyl?
    pSensorStaticInfo->captureHoizontalOutputOffset = pResolutionInfo->SensorFullWidthOffset;//3d use onlyl?
    pSensorStaticInfo->captureVerticalOutputOffset = pResolutionInfo->SensorFullHeightOffset; //3d use onlyl?
    pSensorStaticInfo->videoHoizontalOutputOffset = pResolutionInfo->SensorVideoWidthOffset;//3d use onlyl?
    pSensorStaticInfo->videoVerticalOutputOffset = pResolutionInfo->SensorVideoHeightOffset; //3d use onlyl?
    pSensorStaticInfo->virtualChannelSupport = MFALSE;
    pSensorStaticInfo->iHDR_First_IS_LE = pInfo->IHDR_LE_FirstLine;
    pSensorStaticInfo->SensorModeNum = pInfo->SensorModeNum;
    pSensorStaticInfo->PerFrameCTL_Support = pInfo->PerFrameCTL_Support;
    pSensorStaticInfo->ZHDR_MODE = pInfo->ZHDR_Mode;
    if(pInfo->SensorHorFOV != 0) /*from sensor driver*/
        pSensorStaticInfo->horizontalViewAngle = pInfo->SensorHorFOV;
    if(pInfo->SensorVerFOV != 0)
        pSensorStaticInfo->verticalViewAngle = pInfo->SensorVerFOV;
    if(pInfo->SensorOrientation != 0)
        pSensorStaticInfo->SensorOrientation = pInfo->SensorOrientation;
    /*MY_LOGD("fov idx %d, hor %d, ver = %d orientation %d",idx,
    *pSensorStaticInfo->horizontalViewAngle,
    *pSensorStaticInfo->verticalViewAngle,
    *pSensorStaticInfo->SensorOrientation);
    */
}

MVOID HalSensorList::buildSensorMetadata(IMGSENSOR_SENSOR_IDX idx)
{
    MINT64 exposureTime1 = 0x4000;
    MINT64 exposureTime2 = 0x4000;
    MUINT8 u8Para = 0;
    MINT32 s32Para = 0;

    MY_LOGD("impBuildSensorInfo start!");

    IMetadata& metadataA = gSensorMetadata[idx];
    SensorStaticInfo *pSensorStaticInfo = &sensorStaticInfo[idx];

    {
        IMetadata::IEntry entryA(MTK_SENSOR_EXPOSURE_TIME);
        entryA.push_back(exposureTime1, Type2Type<MINT64>());
        entryA.push_back(exposureTime2, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_EXPOSURE_TIME, entryA);
    }

    { //using full size?
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        MRect region1(MPoint(pSensorStaticInfo->captureHoizontalOutputOffset,pSensorStaticInfo->captureVerticalOutputOffset), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
        entryA.push_back(region1, Type2Type<MRect>());
        metadataA.update(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
        switch(pSensorStaticInfo->sensorFormatOrder) {
            case SENSOR_FORMAT_ORDER_RAW_B:
                u8Para = 0x3;//BGGR
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gb:
                u8Para = 0x2;//GBRG
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gr:
                u8Para = 0x1;//GRBG
                break;
            case SENSOR_FORMAT_ORDER_RAW_R:
                u8Para = 0x0;//RGGB
                break;
            default:
                u8Para = 0x4;//BGR not bayer
                break;
        }
        entryA.push_back(u8Para, Type2Type<MUINT8>());
        metadataA.update(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE);
        entryA.push_back(3000, Type2Type<MINT64>());
        entryA.push_back(3000000000, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_MAX_FRAME_DURATION);
        entryA.push_back(50000000000, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_INFO_MAX_FRAME_DURATION, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PHYSICAL_SIZE);
        entryA.push_back(5.82, Type2Type<MFLOAT>());
        entryA.push_back(4.76, Type2Type<MFLOAT>());
        metadataA.update(MTK_SENSOR_INFO_PHYSICAL_SIZE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE);
        MSize Size1(4000,3000);
        entryA.push_back(Size1, Type2Type<MSize>());
        metadataA.update(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_WHITE_LEVEL);
        switch (pSensorStaticInfo->rawSensorBit) {
            case RAW_SENSOR_8BIT:
                s32Para = 256;
                break;
            case RAW_SENSOR_10BIT:
                s32Para = 1024;
                break;
            case RAW_SENSOR_12BIT:
                s32Para = 4096;
                break;
            case RAW_SENSOR_14BIT:
                s32Para = 16384;
                break;
            default:
                s32Para = 256;
                break;
        }
        entryA.push_back(s32Para, Type2Type<MINT32>());
        metadataA.update(MTK_SENSOR_INFO_WHITE_LEVEL, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        metadataA.update(MTK_SENSOR_INFO_ORIENTATION, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PACKAGE);
        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_PREVIEW,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->previewFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->previewWidth,pSensorStaticInfo->previewHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->previewWidth,pSensorStaticInfo->previewHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_CAPTURE,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->captureFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_VIDEO,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->videoFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->videoWidth,pSensorStaticInfo->videoHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->videoWidth,pSensorStaticInfo->videoHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO1,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->video1FrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->video1Width,pSensorStaticInfo->video1Height);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->video1Width,pSensorStaticInfo->video1Height));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO2,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->video2FrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->video2Width,pSensorStaticInfo->video2Height);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->video2Width,pSensorStaticInfo->video2Height));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }
        metadataA.update(MTK_SENSOR_INFO_PACKAGE, entryA);
    }
    metadataA.sort();

    MY_LOGD("impBuildSensorInfo end!");
}

