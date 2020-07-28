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

#include "camera_custom_imgsensor_cfg.h"

namespace NSCamCustomSensor {

static CUSTOM_CFG gCustomCfg[] = {
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_MAIN,
        .mclk          = CUSTOM_CFG_MCLK_1,
        .port          = CUSTOM_CFG_CSI_PORT_0,
        .dir           = CUSTOM_CFG_DIR_REAR,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 90,
        .horizontalFov = 67,
        .verticalFov   = 49
    },
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_SUB,
        .mclk          = CUSTOM_CFG_MCLK_2,
        .port          = CUSTOM_CFG_CSI_PORT_1,
        .dir           = CUSTOM_CFG_DIR_FRONT,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 270,
        .horizontalFov = 63,
        .verticalFov   = 40
    },
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_MAIN2,
        .mclk          = CUSTOM_CFG_MCLK_3,
        .port          = CUSTOM_CFG_CSI_PORT_2,
        .dir           = CUSTOM_CFG_DIR_REAR,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 90,
        .horizontalFov = 75,
        .verticalFov   = 60
        },
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_SUB2,
        .mclk          = CUSTOM_CFG_MCLK_3,
        .port          = CUSTOM_CFG_CSI_PORT_2,
        .dir           = CUSTOM_CFG_DIR_FRONT,
        .bitOrder      = CUSTOM_CFG_BITORDER_9_2,
        .orientation   = 90,
        .horizontalFov = 75,
        .verticalFov   = 60
        },

    /* Add custom configuration before this line */
    {
        .sensorIdx     = IMGSENSOR_SENSOR_IDX_NONE,
        .mclk          = CUSTOM_CFG_MCLK_NONE,
        .port          = CUSTOM_CFG_CSI_PORT_NONE,
        .dir           = CUSTOM_CFG_DIR_NONE,
        .bitOrder      = CUSTOM_CFG_BITORDER_NONE,
        .orientation   = 0,
        .horizontalFov = 0,
        .verticalFov   = 0
    }
};

CUSTOM_CFG* getCustomConfig(IMGSENSOR_SENSOR_IDX const sensorIdx)
{
    CUSTOM_CFG *pCustomCfg = &gCustomCfg[IMGSENSOR_SENSOR_IDX_MIN_NUM];

    if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM)
        return 0;

    while(pCustomCfg->sensorIdx != IMGSENSOR_SENSOR_IDX_NONE && pCustomCfg->sensorIdx != sensorIdx)
        pCustomCfg++;

    if (pCustomCfg->sensorIdx == IMGSENSOR_SENSOR_IDX_NONE)
        return 0;

    return pCustomCfg;
}

};

