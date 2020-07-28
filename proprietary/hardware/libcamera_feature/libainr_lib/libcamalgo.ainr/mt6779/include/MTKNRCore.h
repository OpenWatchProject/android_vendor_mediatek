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

#ifndef _MTK_NRCORE_H
#define _MTK_NRCORE_H

#include "MTKNRCoreType.h"
#include "MTKNRCoreErrCode.h"

#define NR_PROC_TYPE   1   // 0: I8, 1:I16, 2:FP16, 3:FP32

#if (NR_PROC_TYPE == 0) // int8
typedef char   AINR_MDLA_TYPE;
#elif (NR_PROC_TYPE == 1) // int16
typedef short   AINR_MDLA_TYPE;
#elif (NR_PROC_TYPE == 2)
#elif (NR_PROC_TYPE == 3)// FT32
typedef float   AINR_MDLA_TYPE;
#endif

#define SHARE_IN_OUT

typedef enum DRVNRCoreObject_s
{
    DRV_NRCORE_OBJ_NONE = 0,
    DRV_NRCORE_OBJ_SW,
    DRV_NRCORE_OBJ_UNKNOWN = 0xFF,
} DrvNRCoreObject_e;

typedef enum DRVNRCoreStage_s
{
    DRV_NRCORE_STAGE_PRE_PROC = 0,
    DRV_NRCORE_STAGE_MIAN_PROC,
    DRV_NRCORE_STAGE_POST_PROC,
    DRV_NRCORE_STAGE_UNKNOWN = 0xFF,
} DRVNRCoreStage_e;

typedef int (*NRStageCallback) (DRVNRCoreStage_e, void *);
/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum NRCORE_PROC_ENUM
{
    NRCORE_PROC1 = 0,
    NRCORE_PROC2,
    NRCORE_UNKNOWN_PROC,
} NRCORE_PROC_ENUM;

typedef enum NRCORE_FTCTRL_ENUM
{
    NRCORE_FTCTRL_GET_PROC_INFO,
    NRCORE_FTCTRL_SET_PROC_INFO,
    NRCORE_FTCTRL_GET_VERSION,        // feature id to get Version
    NRCORE_FTCTRL_TEST_PREPROC,
    NRCORE_FTCTRL_TEST_MDLA,
    NRCORE_FTCTRL_TEST_POSTPROC,
    NRCORE_FTCTRL_QUERY_CAPABILITY,
    NRCORE_FTCTRL_MAX
} NRCORE_FTCTRL_ENUM;

struct NRCORE_BUFFER_STRUCT
{
    MUINT32 u4Size; // [out] buffer size
    MBOOL bUseIon;  // [out] allocate ION
    MINT32 i4BuffFD; // [in] ION buff handle
    void* prBuff;// [in] Buff VA  address

    // constructor
    NRCORE_BUFFER_STRUCT()
    : u4Size(0), bUseIon(false), i4BuffFD(-1), prBuff(NULL)
    {}
};

struct NRCORE_INIT_IN_STRUCT
{
    MUINT32 u4CoreNum;
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4FrameNum;
    MUINT32 u4AlgoIdx;
    MUINT32 u4TileMode;

    // constructor
    NRCORE_INIT_IN_STRUCT()
    : u4FrameNum(6), u4AlgoIdx(0), u4TileMode(0)
    {}
};


struct NRCORE_GET_PROC_INFO_IN_STRUCT
{

};

struct NRCORE_GET_PROC_INFO_OUT_STRUCT
{
    MUINT32 u4Num; //[out] set ther number of rbuf[]
    NRCORE_BUFFER_STRUCT rBuf[4];

    // constructor
    NRCORE_GET_PROC_INFO_OUT_STRUCT()
    : u4Num(0)
    {}
};

struct NRCORE_SET_PROC_INFO_IN_STRUCT
{
    MUINT32 u4Num;
    NRCORE_BUFFER_STRUCT rBuf[4];

    // constructor
    NRCORE_SET_PROC_INFO_IN_STRUCT()
    : u4Num(0)
    {}
};

struct NRCORE_PROC1_IN_STRUCT
{
    MUINT16 *u2Img[6];
    MUINT32 u4ImgSize;

    MUINT32 u4Alpha[4]; // for blending
    MINT32 i4ObOffst[4];
    MINT32 i4DgnGain[4];

    MINT32 i4ChParam[8];

    NRStageCallback fpStageCallback;
    void * pvCookie;

    // constructor
    NRCORE_PROC1_IN_STRUCT()
    : fpStageCallback(NULL), pvCookie(NULL)
    {}
};

struct NRCORE_PROC1_OUT_STRUCT
{
    MUINT16 *u2OutImg;
    MUINT32 u4OutSize;
};

struct NRCORE_PROC2_IN_STRUCT
{
    MUINT16 *u2Img[6];
    MUINT32 u4ImgSize;

    MUINT32 u4Alpha[4]; // for blending
    MINT32 i4ObOffst[4];
    MINT32 i4DgnGain[4];

    MINT32 i4ChParam[8];
};

struct NRCORE_PROC2_OUT_STRUCT
{
    MUINT16 *u2OutImg;
    MUINT32 u4OutSize;
};

struct NRCore_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)


struct NRCORE_TEST_PREPROC_IN_STRUCT
{
    MUINT16 *u2Img[6];
    MUINT32 u4ImgSize;

    MINT32 i4ChParam[8];

    MUINT32 u4NrType;

    MUINT32 u4IsTranspose;

    MINT32 i4TileIdx;

    // constructor
    NRCORE_TEST_PREPROC_IN_STRUCT()
    : i4TileIdx(0)
    {}
};

struct NRCORE_TEST_PREPROC_OUT_STRUCT
{
    AINR_MDLA_TYPE *pOutImg;
    MUINT32 u4OutSize;
};

struct NRCORE_TEST_TEST_MDLA_IN_STRUCT
{
    void *prInput;
    MINT32 i4FD;
    MUINT32 u4InputSize;
    MBOOL bUseFileBin;

    // buffer mode
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4FrameNum;
    MUINT32 u4AlgoIdx;

    // file mode
    MINT8 aBinPath[100];  // set NULL is buffer mode
    uint64_t uIn4Handle;
    uint64_t uOut4Handle0;
    uint64_t uOut4Handle1;
};

struct NRCORE_TEST_TEST_MDLA_OUT_STRUCT
{
    void *prOutput0;
    void *prOutput1;
    MINT32 i4FD0;
    MINT32 i4FD1;
    MUINT32 u4OutputSize;
};

struct NRCORE_TEST_POSTPROC_IN_STRUCT
{
    AINR_MDLA_TYPE *prInL;
    AINR_MDLA_TYPE *prInR;
    MUINT32 u4ImgSize;

    MUINT16 *pu2Base;
    MUINT32 u4BaseSize;

    MUINT32 u4Alpha[4];
    MINT32 i4ObOffst[4];
    MINT32 i4DgnGain[4];

    MINT32 i4ChParam[8];

    MUINT32 u4NrType;

    MUINT32 u4IsTranspose;

    MINT32 i4TileIdx;

    // constructor
    NRCORE_TEST_POSTPROC_IN_STRUCT()
    : i4TileIdx(0)
    {}
};

struct NRCORE_TEST_POSTPROC_OUT_STRUCT
{
    MUINT16 *pu2OutImg;
    MUINT32 u4OutSize;
};

struct NRCORE_QUERY_CAPABILITY_IN_STRUCT
{
    MUINT32 u4Width;
    MUINT32 u4Height;
};

struct NRCORE_QUERY_CAPABILITY_OUT_STRUCT
{

};

/*******************************************************************************
*
********************************************************************************/
class MTKNRCore
{
public:
    static MTKNRCore* createInstance(DrvNRCoreObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKNRCore(){}
    virtual MRESULT NRCoreInit(void* pParaIn, void* pParaOut);
    virtual MRESULT NRCoreReset(void);
    virtual MRESULT NRCoreMain(NRCORE_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT NRCoreFeatureCtrl(NRCORE_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppNRCoreTmp : public MTKNRCore
{
public:

    static MTKNRCore* getInstance();
    virtual void destroyInstance();

    AppNRCoreTmp() {};
    virtual ~AppNRCoreTmp() {};
};
#endif

