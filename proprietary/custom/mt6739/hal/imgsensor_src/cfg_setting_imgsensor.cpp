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
//#ifndef _CFG_SETTING_IMGSENSOR_H_
//#define _CFG_SETTING_IMGSENSOR_H_
#include "camera_custom_imgsensor_cfg.h"
using namespace NSCamCustomSensor;

namespace NSCamCustomSensor {

static CUSTOM_CFG gCustomCfg[IMGSENSOR_SENSOR_IDX_MAX_NUM] = {
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_MAIN,
        .mclk          = eMclk_1,
        .port          = EMipiPort_CSI0,
        .dir           = CUSTOM_CFG_DIR_REAR,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 90,
        .horizontalFov = 65,
        .verticalFov   = 51,
        .PadPclkInv    = 0,

    },
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_SUB,
        .mclk          = eMclk_1,
        .port          = EMipiPort_CSI1,
        .dir           = CUSTOM_CFG_DIR_FRONT,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 270,
        .horizontalFov = 65,
        .verticalFov   = 51,
        .PadPclkInv    = 0,
    },
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_MAIN2,
        .mclk          = eMclk_2,
        .port          = EMipiPort_CSI1,
        .dir           = CUSTOM_CFG_DIR_REAR,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 90,
        .horizontalFov = 65,
        .verticalFov   = 51,
        .PadPclkInv    = 0,
    },

    /* Add custom configuration before this line */
};

/*******************************************************************************
* Image Sensor Orientation
*******************************************************************************/
MUINT32 getSensorOrientation(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].orientation;
}

/*******************************************************************************
* Return fake orientation for front sensor or not
*       MTRUE: return 90 for front sensor in degree 0,
*              return 270 for front sensor in degree 180.
*       MFALSE: not return fake orientation.
*******************************************************************************/
MBOOL isRetFakeSubOrientation() {
    return MFALSE;
}

/*******************************************************************************
* Return fake orientation for back sensor or not
*       MTRUE: return 90 for back sensor in degree 0,
*              return 270 for back sensor in degree 180.
*       MFALSE: not return fake orientation.
*******************************************************************************/
MBOOL isRetFakeMainOrientation() {
    return MFALSE;
}

/*******************************************************************************
* Return fake orientation for back (3D) sensor or not
*       MTRUE: return 90 for back sensor in degree 0,
*              return 270 for back sensor in degree 180.
*       MFALSE: not return fake orientation.
*******************************************************************************/
MBOOL isRetFakeMain2Orientation() {
    return MFALSE;
}

/*******************************************************************************
* Return fake orientation for sub2 sensor or not
*       MTRUE: return 90 for back sensor in degree 0,
*              return 270 for back sensor in degree 180.
*       MFALSE: not return fake orientation.
*******************************************************************************/
MBOOL isRetFakeSub2Orientation() {
    return MFALSE;
}

/*******************************************************************************
* Sensor Input Data Bit Order
*   Return:
*       0   : raw data input [9:2]
*       1   : raw data input [7:0]
*       -1  : error
*******************************************************************************/
MINT32
getSensorInputDataBitOrder(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].bitOrder;
}


/*******************************************************************************
* Sensor Pixel Clock Inverse in PAD side.
*   Return:
*       0   : no inverse
*       1   : inverse
*       -1  : error
*******************************************************************************/
MINT32
getSensorPadPclkInv(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].PadPclkInv;
}

/*******************************************************************************
* Sensor Placement Facing Direction
*   Return:
*       0   : Back side
*       1   : Front side (LCD side)
*       -1  : error
*******************************************************************************/
MINT32
getSensorFacingDirection(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].dir;
}

/*******************************************************************************
* Sensor layout using mclk
*   Return: EMclkId
*******************************************************************************/
MINT32  getSensorMclkConnection(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].mclk;
}

/*******************************************************************************
* MIPI sensor pad usage
*   Return: EMipiPort
*******************************************************************************/
MINT32  getMipiSensorPort(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].port;
}

/*******************************************************************************
* Image Sensor Module FOV
*******************************************************************************/
MUINT32  getSensorViewAngle_H(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].horizontalFov;
}

MUINT32  getSensorViewAngle_V(IMGSENSOR_SENSOR_IDX const idx) {
    return gCustomCfg[idx].verticalFov;
}

};

//#endif //  _CFG_SETTING_IMGSENSOR_H_

