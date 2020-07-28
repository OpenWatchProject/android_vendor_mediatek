/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_IMGSENSOR_CFG_
#define _CAMERA_CUSTOM_IMGSENSOR_CFG_
//
#include "kd_camera_feature.h"
#include "camera_custom_types.h"
//
namespace NSCamCustomSensor
{
//
//

typedef enum
{
        eMclk_1 = 0x0,//mclk1
        eMclk_2 = 0x1,//mclk2
        eMclk_3 = 0x2,//mclk3
        eMclk_4 = 0x3,
        eMclk_count,
} EMclkId;

typedef enum
{
        EMipiPort_CSI0 = 0x0,// 4D1C
        EMipiPort_CSI1, // 4D1C
        EMipiPort_CSI2, // 4D1C
        EMipiPort_CSI0A,  // 2D1C
        EMipiPort_CSI0B,  // 2D1C
        EMipiPort_NONE = 0xFF,//for non-MIPI sensor
} EMipiPort;

typedef enum {
    CUSTOM_CFG_DIR_REAR,
    CUSTOM_CFG_DIR_FRONT,
    CUSTOM_CFG_DIR_MAX_NUM,
    CUSTOM_CFG_DIR_NONE
} CUSTOM_CFG_DIR;

typedef enum {
    CUSTOM_CFG_BITORDER_9_2,
    CUSTOM_CFG_BITORDER_7_0,
    CUSTOM_CFG_BITORDER_MAX_NUM,
    CUSTOM_CFG_BITORDER_NONE
} CUSTOM_CFG_BITORDER;


typedef struct {
    IMGSENSOR_SENSOR_IDX sensorIdx;
    EMclkId      mclk;
    EMipiPort  port;
    CUSTOM_CFG_DIR       dir;
    CUSTOM_CFG_BITORDER  bitOrder;
    unsigned int         orientation;
    unsigned int         horizontalFov;
    unsigned int         verticalFov;
	unsigned int         PadPclkInv;
} CUSTOM_CFG;

/*******************************************************************************
* Sensor Input Data Bit Order
*   Return:
*       0   : raw data input [9:2]
*       1   : raw data input [7:0]
*       -1  : error
*******************************************************************************/
MINT32  getSensorInputDataBitOrder(IMGSENSOR_SENSOR_IDX const idx);

/*******************************************************************************
* Sensor Pixel Clock Inverse in PAD side.
*   Return:
*       0   : no inverse
*       1   : inverse
*       -1  : error
*******************************************************************************/
MINT32  getSensorPadPclkInv(IMGSENSOR_SENSOR_IDX const idx);

/*******************************************************************************
* Sensor Placement Facing Direction
*   Return:
*       0   : Back side
*       1   : Front side (LCD side)
*       -1  : error
*******************************************************************************/
MINT32  getSensorFacingDirection(IMGSENSOR_SENSOR_IDX const idx);
/*******************************************************************************
* Sensor layout using mclk
*   Return: EMclkId
*******************************************************************************/
MINT32  getSensorMclkConnection(IMGSENSOR_SENSOR_IDX const idx);

/*******************************************************************************
* MIPI sensor pad usage
*   Return: EMipiPort
*******************************************************************************/
MINT32  getMipiSensorPort(IMGSENSOR_SENSOR_IDX const idx);


MUINT32  getSensorOrientation(IMGSENSOR_SENSOR_IDX const idx);

/*******************************************************************************
* Return fake orientation for front sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeSubOrientation();
/*******************************************************************************
* Return fake orientation for back sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeMainOrientation();
/*******************************************************************************
* Return fake orientation for back2 (3D)sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeMain2Orientation();

/*******************************************************************************
* Return fake orientation for sub2 sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeSub2Orientation();

/*******************************************************************************
* Return fake orientation for main3 sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeMain3Orientation();

/*******************************************************************************
* Image Sensor Module FOV
*******************************************************************************/
MUINT32  getSensorViewAngle_H(IMGSENSOR_SENSOR_IDX const idx);
MUINT32  getSensorViewAngle_V(IMGSENSOR_SENSOR_IDX const idx);


};  //NSCamCustomSensor
#endif  //  _CAMERA_CUSTOM_IMGSENSOR_CFG_

