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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_NRUTILS_H
#define _MTK_NRUTILS_H

#include "MTKNRUtilsType.h"
#include "MTKNRUtilsErrCode.h"

//efine EN_NORMAL
#define CHI_STD_LAMBDA

typedef enum DRVNRUtilsObject_s
{
    DRV_NRUTILS_OBJ_NONE = 0,
    DRV_NRUTILS_OBJ_SW,
    DRV_NRUTILS_OBJ_UNKNOWN = 0xFF,
} DrvNRUtilsObject_e;

/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum NRUTILS_PROC_ENUM
{
    NRUTILS_PROC1 = 0,
    NRUTILS_UNKNOWN_PROC,
} NRUTILS_PROC_ENUM;

typedef enum NRUTILS_FTCTRL_ENUM
{
    NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER,
    NRUTILS_FTCTRL_PROC_UNPACK_BAYER,
    NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER_UV,
    NRUTILS_FTCTRL_PROC_UNPACK_BAYER_UV,
    NRUTILS_FTCTRL_CONFIG_YUV8_MIX,
    NRUTILS_FTCTRL_PROC_YUV8_MIX,
    NRUTILS_FTCTRL_CONFIG_YUV_MIX,
    NRUTILS_FTCTRL_PROC_YUV_MIX,
    NRUTILS_FTCTRL_GET_VERSION,        // feature id to get Version
    NRUTILS_FTCTRL_MAX
} NRUTILS_FTCTRL_ENUM;

typedef enum NRUTILS_IMAGE_FMT_ENUM
{
    NRUTILS_IMAGE_PACK_BAYER10_BASE = 0,
    NRUTILS_IMAGE_PACK_BAYER10_UV = 1,
    NRUTILS_IMAGE_BAYER_NHWC_I8 = 2,
    NRUTILS_IMAGE_BAYER_NHWC_I16 = 3,
    NRUTILS_IMAGE_BAYER_NHWC_FP16 = 4,
    NRUTILS_IMAGE_BAYER_NHWC_FP32 = 5
} NRUTILS_IMAGE_FMT_ENUM;


struct NRUTILS_BUFFER_STRUCT
{
    MUINT32 Size; // buffer size
    MUINT32 BufferNum; //  plane number
    MINT32 BuffFD[4]; // ION buff handle
    void* pBuff[4];// Buff VA  address

    // constructor
    NRUTILS_BUFFER_STRUCT()
    : Size(0), BufferNum(0)
    {}
};

struct NRUTILS_CONFIG_UNPACK_BAYER_IN_STRUCT
{
    MUINT32 u4CoreNum; // CPU core number
    MINT32 i4Width;
    MINT32 i4Height;
    MINT32 i4Stride;
    MINT32 i4OutBits; // 10 or 12
    MINT32 i4Mode; //0: default, 1: sw mode 2: performance mode
#ifndef CHI_STD_LAMBDA
    MUINT32 ru4Chi[4];
    MUINT32 ru4Std[4];
#endif
    MINT32 i4ObOffst[4];
    MINT32 i4DgnGain[4];

    // constructor
    NRUTILS_CONFIG_UNPACK_BAYER_IN_STRUCT()
    : i4Mode(0)
    {
        i4ObOffst[0] = 0;
        i4DgnGain[0] = 0;
    }
};

struct NRUTILS_CONFIG_UNPACK_BAYER_OUT_STRUCT
{
    MINT32 i4WMSize;
#ifdef EN_NORMAL
    NRUTILS_IMAGE_FMT_ENUM eOutFmt; //INT16/INT8/FP32/FP16
#endif

#ifndef CHI_STD_LAMBDA
    NRUTILS_IMAGE_FMT_ENUM eNoiseLevelFmt; //INT16/INT8/FP32/FP16
#endif
};

struct NRUTILS_UNPACK_BAYER_IN_STRUCT
{
    NRUTILS_BUFFER_STRUCT rInput;
    NRUTILS_BUFFER_STRUCT rWM;
};

struct NRUTILS_UNPACK_BAYER_OUT_STRUCT
{
    NRUTILS_BUFFER_STRUCT rOutput;     //INT16
#ifdef EN_NORMAL
    NRUTILS_BUFFER_STRUCT rNormalOut;  //set buffer when eOutFmt is FP32/FP16
#endif

#ifndef CHI_STD_LAMBDA
    NRUTILS_BUFFER_STRUCT rNosieLevel; //set buffe by eNoiseLevelFmt
#endif
};

struct NRUTILS_CONFIG_UNPACK_BAYER_UV_IN_STRUCT
{
    MUINT32 u4CoreNum; // CPU core number
    MINT32 i4Width;
    MINT32 i4Height;
    MINT32 i4Stride;
    MINT32 i4OutBits; // 10 or 12
    MINT32 i4Mode; //0: default, 1: sw mode 2: performance mode
    MINT32 i4ObOffst[4];
    MINT32 i4DgnGain[4];

    // constructor
    NRUTILS_CONFIG_UNPACK_BAYER_UV_IN_STRUCT()
    : i4Mode(0)
    {
        i4ObOffst[0] = 0;
        i4DgnGain[0] = 0;
    }
};

struct NRUTILS_CONFIG_UNPACK_BAYER_UV_OUT_STRUCT
{
    MINT32 i4WMSize;
#ifdef EN_NORMAL
    NRUTILS_IMAGE_FMT_ENUM eOutFmt;  //INT16/INT8/FP32/FP16
#endif
};

struct NRUTILS_UNPACK_BAYER_UV_IN_STRUCT
{
    NRUTILS_BUFFER_STRUCT rInput1;
    NRUTILS_BUFFER_STRUCT rInput2;
    NRUTILS_BUFFER_STRUCT rWM;
};

struct NRUTILS_UNPACK_BAYER_UV_OUT_STRUCT
{
    NRUTILS_BUFFER_STRUCT rOutput;     //INT16
#ifdef EN_NORMAL
    NRUTILS_BUFFER_STRUCT rNormalOut;  //FP32
#endif
};

struct NRUTILS_CONFIG_YUV8_MIX_IN_STRUCT
{
    MUINT32 u4CoreNum;
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4YBlend;
    MUINT32 u4UVBlend;
};

struct NRUTILS_PROC_YUV8_MIX_IN_STRUCT
{
    MUINT8 *u1Img1[2];
    MUINT8 *u1Img2[2];
};

struct NRUTILS_PROC_YUV8_MIX_OUT_STRUCT
{
    MUINT8 *u1OutImg[2];
};

struct NRUTILS_CONFIG_YUV_MIX_IN_STRUCT
{
    MUINT32 u4CoreNum;
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4YBlend;
    MUINT32 u4UVBlend;
};

struct NRUTILS_PROC_YUV_MIX_IN_STRUCT
{
    MUINT32 u4PlaneNum; // 2: Y, uv.  3: Y, u, v
    MUINT16 *u2Img1[3];
    MUINT16 *u2Img2[3];
};

struct NRUTILS_PROC_YUV_MIX_OUT_STRUCT
{
    MUINT16 *u2OutImg[3];
};


struct NRUTILS_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)

/*******************************************************************************
*
********************************************************************************/
class MTKNRUtils
{
public:
    static MTKNRUtils* createInstance(DrvNRUtilsObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKNRUtils(){}
    virtual MRESULT NRUtilsInit(void* pParaIn, void* pParaOut);
    virtual MRESULT NRUtilsReset(void);
    virtual MRESULT NRUtilsMain(NRUTILS_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT NRUtilsFeatureCtrl(NRUTILS_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppNRUtilsTmp : public MTKNRUtils
{
public:

    static MTKNRUtils* getInstance();
    virtual void destroyInstance();

    AppNRUtilsTmp() {};
    virtual ~AppNRUtilsTmp() {};
};
#endif

