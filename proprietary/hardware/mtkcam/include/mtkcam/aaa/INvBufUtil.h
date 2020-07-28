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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_INVBUFUTIL_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_INVBUFUTIL_H_
//
#include <mtkcam/utils/module/module.h>
#include <camera_custom_nvram.h>

class INvBufUtil
{
public:
    enum
    {
        e_SensorDevWrong=-1000,
        e_NvramIdWrong,
        e_NV_SensorDevWrong,
        e_NV_SensorIdNull,
    };
public:
    virtual             ~INvBufUtil() {}

    virtual void        setAndroidMode(int isAndroid=1) = 0; //anroid mode: android:1, meta(cct, factory):0

    /*
    nvRamId:
    CAMERA_NVRAM_DATA_ISP,
    CAMERA_NVRAM_DATA_3A,
    CAMERA_NVRAM_DATA_SHADING,
    CAMERA_NVRAM_DATA_LENS,
    CAMERA_DATA_AE_PLINETABLE,
    CAMERA_NVRAM_DATA_STROBE,
    CAMERA_NVRAM_DATA_N3D3A,
    CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_VERSION,
    */

    virtual int         getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p) = 0;
    virtual int         getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead=0) = 0;
    virtual int         getBufAndReadNoDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead=0) = 0;
    int         getSensorIdAndModuleId(MINT32 sensorType , MINT32 sensoridx, MINT32 &sensorId, MUINT32 &moduleId);

    virtual int         write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev) = 0;

    // note: please provide memory to call the function.
    // For sync the buf data with NvRam data, the internal buf can't be used in the function.
    virtual int         readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void* p) = 0;
    virtual int         readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev) = 0;

};


/**
 * @brief The definition of the maker of INvBufUtil instance.
 */
typedef INvBufUtil* (*NvBufUtil_FACTORY_T)();
#define MAKE_NvBufUtil(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_NVBUF_UTIL, NvBufUtil_FACTORY_T, __VA_ARGS__)


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_INVBUFUTIL_H_
