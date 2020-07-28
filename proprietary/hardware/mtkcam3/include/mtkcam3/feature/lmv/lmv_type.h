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

/**
* @file lmv_type.h
*
* LMV Type Header File
*/


#ifndef _LMV_TYPE_H_
#define _LMV_TYPE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam3/feature/lmv/lmv_ext.h>

/*********************************************************************
* Define Value
*********************************************************************/

#include <MTKEis.h>

#define LMV_MAX_WIN_NUM EIS_WIN_NUM


#define CIF_WIDTH 352
#define CIF_HEIGHT 320

#define D1_WIDTH 792
#define D1_HEIGHT 528

#define HD_720P_WIDTH 1536
#define HD_720P_HEIGHT 864

#define HD_1080P_WIDTH 2112
#define HD_1080P_HEIGHT 1188

#define HD_8M_WIDTH 3264
#define HD_8M_HEIGHT 2200

#define HD_12M_WIDTH 4000
#define HD_12M_HEIGHT 3000

#define HD_16M_WIDTH 4608
#define HD_16M_HEIGHT 3456

#define HD_20M_WIDTH 5164
#define HD_20M_HEIGHT 3872

#define ALIGN_SIZE(in,align) (in & ~(align-1))



#define LMVO_MEMORY_SIZE (256)    // 32 * 64 (bits) = 256 bytes

/*********************************************************************
* ENUM
*********************************************************************/

typedef enum
{
    LMV_RETURN_NO_ERROR         = 0,       //! The function work successfully
    LMV_RETURN_UNKNOWN_ERROR    = 0x0001,  //! Unknown error
    LMV_RETURN_INVALID_DRIVER   = 0x0002,  //! invalid driver object
    LMV_RETURN_API_FAIL         = 0x0003,  //! api fail
    LMV_RETURN_INVALID_PARA     = 0x0004,  //! invalid parameter
    LMV_RETURN_NULL_OBJ         = 0x0005,  //! null object
    LMV_RETURN_MEMORY_ERROR     = 0x0006,  //! memory error
    LMV_RETURN_EISO_MISS        = 0x0007   //! EISO data missed
}LMV_ERROR_ENUM;


/**
*@brief Sensor type
*/
typedef enum
{
    LMV_RAW_SENSOR,
    LMV_YUV_SENSOR,
    LMV_JPEG_SENSOR,
    LMV_NULL_SENSOR
}LMV_SENSOR_ENUM;



/*********************************************************************
* Struct
*********************************************************************/

/**
*@brief  LMV statistic data structure
*/
typedef struct
{
    MINT32 i4LMV_X[LMV_MAX_WIN_NUM];
    MINT32 i4LMV_Y[LMV_MAX_WIN_NUM];

    MINT32 i4LMV_X2[LMV_MAX_WIN_NUM];
    MINT32 i4LMV_Y2[LMV_MAX_WIN_NUM];

    MUINT32 NewTrust_X[LMV_MAX_WIN_NUM];
    MUINT32 NewTrust_Y[LMV_MAX_WIN_NUM];

    MUINT32 SAD[LMV_MAX_WIN_NUM];
} LMV_STATISTIC_T;


typedef struct LMV_HAL_CONFIG
{
    MUINT32 sensorType;     // use in pass1
    MUINT32 p1ImgW;         // use in pass1
    MUINT32 p1ImgH;         // use in pass1
    LMV_STATISTIC_T lmvData;

    //
    LMV_HAL_CONFIG()
    : sensorType(0)
    , p1ImgW(0)
    , p1ImgH(0)
    {
        for( MINT32 i = 0; i<LMV_MAX_WIN_NUM; i++)
        {
            lmvData.i4LMV_X[i]=0;
            lmvData.i4LMV_Y[i]=0;
            lmvData.i4LMV_X2[i]=0;
            lmvData.i4LMV_Y2[i]=0;
            lmvData.NewTrust_X[i]=0;
            lmvData.NewTrust_Y[i]=0;
            lmvData.SAD[i]=0;
        }
    }
}LMV_HAL_CONFIG_DATA;

typedef struct LMV_HAL_ENQUEUE
{
    MINT32 magicNum = 0;
    NSCam::IMetadata* inApp = nullptr;
    NSCam::IMetadata* inHal = nullptr;
}LMV_HAL_ENQUEUE_DATA;

#endif // _LMV_TYPE_H_

