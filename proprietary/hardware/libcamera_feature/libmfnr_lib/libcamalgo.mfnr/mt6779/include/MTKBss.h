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

#ifndef _MTK_BSS_H
#define _MTK_BSS_H

#include "MTKBssType.h"
#include "MTKBssErrCode.h"
#include "MTKBssFace.h"
#include <string.h>


//#define D_BSS_FORCE_BEST_IDX
#define D_BSS_FRM_SKIP_1

#define MAX_FRAME_NUM 8


typedef enum BSS_PROC_ENUM
{
    BSS_PROC1 = 0,
    BSS_PROC2,
    BSS_UNKNOWN_PROC,
} BSS_PROC_ENUM;

typedef enum DRVBssObject_s
{
    DRV_BSS_OBJ_NONE = 0,
    DRV_BSS_OBJ_SW,
    DRV_BSS_OBJ_UNKNOWN = 0xFF,
} DrvBssObject_e;

typedef enum BSS_PROC_TYPE
{
    BSS_TYPE_PACK_RAW10 = 0,
    BSS_TYPE_PACK_Y10,
    BSS_UNKNOWN_TYPE
} BSS_PROC_TYPE;

typedef enum BSS_FTCTRL_ENUM
{
  BSS_FTCTRL_GET_WB_SIZE,
  BSS_FTCTRL_SET_WB_SIZE,
  BSS_FTCTRL_SET_PROC_INFO,
  BSS_FTCTRL_CONFIG_ZIP,
  BSS_FTCTRL_PROC_ZIP,
  BSS_FTCTRL_CONVERT_I422_YUY2,
  BSS_FTCTRL_GET_VERSION,        // feature id to get Version
  BSS_FTCTRL_MAX
} BSS_FTCTRL_ENUM;

struct BSS_WB_STRUCT
{
    BSS_PROC_ENUM rProcId;  // [User SET]process mode
    MUINT32 u4Width;        // [User SET] frame width
    MUINT32 u4Height;       // [User SET] frame height
    MUINT32 u4FrameNum;     // [User SET] frame number
    MUINT32 u4WKSize;       // [return value] working buffer require size
    MUINT8 *pu1BW;          // [User SET] frame number

    // constructor
    BSS_WB_STRUCT()
    {
        ::memset(this, 0x0, sizeof(struct BSS_WB_STRUCT));
    }
};

struct BSS_CONFIG_ZIP_IN_STRUCT
{
    MUINT32 u4Width;        // [User SET] frame width
    MUINT32 u4Height;       // [User SET] frame height
    BSS_PROC_TYPE eType;
    MUINT32 u4ROI_X0;
    MUINT32 u4ROI_Y0;
    MUINT32 u4ROI_WIDTH;
    MUINT32 u4ROI_HEIGHT;
    MUINT32 u4SCALE_FACTOR;

    // constructor
    BSS_CONFIG_ZIP_IN_STRUCT()
    {
        ::memset(this, 0x0, sizeof(struct BSS_CONFIG_ZIP_IN_STRUCT));
    }
};

struct BSS_CONFIG_ZIP_OUT_STRUCT
{
    MUINT32 u4ZipBufferSize;       // [return value] zip out buffer require size

    // constructor
    BSS_CONFIG_ZIP_OUT_STRUCT()
    {
        ::memset(this, 0x0, sizeof(struct BSS_CONFIG_ZIP_OUT_STRUCT));
    }
};

struct BSS_PROC_ZIP_STRUCT
{
    MUINT8 *pu8InImg;
    MUINT32 u4ImgInStride;
    MUINT32 u4ImgInSize;
    MUINT32 u4ImgInBayerOrder;    // for raw input
    MUINT8 *pu8OutImg;
    MUINT32 u4ImgOutSize;

    MINT32  i4GmvX;
    MINT32  i4GmvY;

    // constructor
    BSS_PROC_ZIP_STRUCT()
    {
        ::memset(this, 0x0, sizeof(struct BSS_PROC_ZIP_STRUCT));
    }
};



struct BSS_GYRO_INFO {
  MFLOAT fX;
  MFLOAT fY;
  MFLOAT fZ;

  // constructor
  BSS_GYRO_INFO()
           : fX(0), fY(0), fZ(0)
  {}
};

struct BSS_PARAM_STRUCT
{
  MUINT8  BSS_ON;
  MUINT8  BSS_VER;
  MUINT32 BSS_ROI_WIDTH;
  MUINT32 BSS_ROI_HEIGHT;
  MUINT32 BSS_ROI_X0;
  MUINT32 BSS_ROI_Y0;
  MUINT32 BSS_SCALE_FACTOR;
  MUINT32 BSS_CLIP_TH0;
  MUINT32 BSS_CLIP_TH1;
  MUINT32 BSS_CLIP_TH2;
  MUINT32 BSS_CLIP_TH3;
  MUINT32 BSS_ZERO;
  MUINT32 BSS_FRAME_NUM;
  MUINT32 BSS_ADF_TH;
  MUINT32 BSS_SDF_TH;
  MUINT32 BSS_GAIN_TH0;
  MUINT32 BSS_GAIN_TH1;
  MUINT32 BSS_MIN_ISP_GAIN;
  MUINT32 BSS_LCSO_SIZE;

  MUINT8  BSS_YPF_EN;
  MUINT8  BSS_YPF_FAC;
  MUINT16 BSS_YPF_ADJTH;
  MUINT16 BSS_YPF_DFMED0;
  MUINT16 BSS_YPF_DFMED1;
  MUINT8  BSS_YPF_TH0;
  MUINT8  BSS_YPF_TH1;
  MUINT8  BSS_YPF_TH2;
  MUINT8  BSS_YPF_TH3;
  MUINT8  BSS_YPF_TH4;
  MUINT8  BSS_YPF_TH5;
  MUINT8  BSS_YPF_TH6;
  MUINT8  BSS_YPF_TH7;

  MUINT8  BSS_FD_EN;
  MUINT8  BSS_FD_FAC;
  MUINT8  BSS_FD_FNUM;
  MUINT8  BSS_FD_TH0;
  MUINT8  BSS_FD_TH1;
  MUINT8  BSS_EYE_EN;
  MUINT8  BSS_EYE_CFTH;
  MUINT8  BSS_EYE_RATIO0;
  MUINT8  BSS_EYE_RATIO1;
  MUINT8  BSS_EYE_FAC;
  MUINT8  BSS_AEVC_EN;
  MUINT16 BSS_AEVC_DCNT;

  // BSS 3.0
  MUINT8  BSS_FaceCVTh;       // range: 0 ~ 100
  MUINT8  BSS_GradThL;        // range: 0 ~ 255
  MUINT8  BSS_GradThH;        // range: 0 ~ 255
  MUINT16 BSS_FaceAreaThL[2]; // range: 0 ~ 65535
  MUINT16 BSS_FaceAreaThH[2]; // range: 0 ~ 65535
  MUINT16 BSS_APLDeltaTh[33]; // range: 0 ~ 4095
  MUINT16 BSS_GradRatioTh[8]; // range: 0 ~ 10000
  MUINT8  BSS_EyeDistThL;     // range: 0 ~ 255
  MUINT8  BSS_EyeDistThH;     // range: 0 ~ 255
  MUINT16 BSS_EyeMinWeight;   // range: 0 ~ 10000

  // constructor
  BSS_PARAM_STRUCT() :
      BSS_ON(1), BSS_VER(3),
      BSS_SCALE_FACTOR(8),
      BSS_CLIP_TH0(10), BSS_CLIP_TH1(20), BSS_CLIP_TH2(10), BSS_CLIP_TH3(20),
      BSS_ZERO(12), BSS_FRAME_NUM(4),
      BSS_ADF_TH(18), BSS_SDF_TH(80),
      BSS_GAIN_TH0(853), BSS_GAIN_TH1(1229),
      BSS_MIN_ISP_GAIN(546), BSS_LCSO_SIZE(147456),

      BSS_YPF_EN(1), BSS_YPF_FAC(50), BSS_YPF_ADJTH(12),
      BSS_YPF_DFMED0(20), BSS_YPF_DFMED1(32),
      BSS_YPF_TH0(102), BSS_YPF_TH1(104), BSS_YPF_TH2(98), BSS_YPF_TH3(96),
      BSS_YPF_TH4(96), BSS_YPF_TH5(96), BSS_YPF_TH6(96), BSS_YPF_TH7(96),

      BSS_FD_EN(0), BSS_FD_FAC(2), BSS_FD_FNUM(1), BSS_FD_TH0(5), BSS_FD_TH1(40),
      BSS_EYE_EN(0), BSS_EYE_CFTH(60), BSS_EYE_RATIO0(75), BSS_EYE_RATIO1(55), BSS_EYE_FAC(50),
      BSS_AEVC_EN(0), BSS_AEVC_DCNT(512),

      BSS_FaceCVTh(50), BSS_GradThL(4), BSS_GradThH(58),
      BSS_EyeDistThL(7), BSS_EyeDistThH(11), BSS_EyeMinWeight(9700)
  {
      BSS_FaceAreaThL[0] = 256;
      BSS_FaceAreaThL[1] = 256;
      BSS_FaceAreaThH[0] = 16384;
      BSS_FaceAreaThH[1] = 32768;
      BSS_APLDeltaTh[0]  = 70;  // 0
      BSS_APLDeltaTh[1]  = 75;  // 128
      BSS_APLDeltaTh[2]  = 80;  // 256
      BSS_APLDeltaTh[3]  = 85;  // 384
      BSS_APLDeltaTh[4]  = 90;  // 512
      BSS_APLDeltaTh[5]  = 95;  // 640
      BSS_APLDeltaTh[6]  = 100; // 768
      BSS_APLDeltaTh[7]  = 105; // 896
      BSS_APLDeltaTh[8]  = 110; // 1024
      BSS_APLDeltaTh[9]  = 120; // 1152
      BSS_APLDeltaTh[10] = 130; // 1280
      BSS_APLDeltaTh[11] = 140; // 1408
      BSS_APLDeltaTh[12] = 150; // 1536
      BSS_APLDeltaTh[13] = 160; // 1664
      BSS_APLDeltaTh[14] = 170; // 1792
      BSS_APLDeltaTh[15] = 180; // 1920
      BSS_APLDeltaTh[16] = 190; // 2048
      BSS_APLDeltaTh[17] = 200; // 2176
      BSS_APLDeltaTh[18] = 200; // 2304
      BSS_APLDeltaTh[19] = 200; // 2432
      BSS_APLDeltaTh[20] = 200; // 2560
      BSS_APLDeltaTh[21] = 200; // 2688
      BSS_APLDeltaTh[22] = 200; // 2816
      BSS_APLDeltaTh[23] = 200; // 2944
      BSS_APLDeltaTh[24] = 200; // 3072
      BSS_APLDeltaTh[25] = 200; // 3200
      BSS_APLDeltaTh[26] = 200; // 3328
      BSS_APLDeltaTh[27] = 200; // 3456
      BSS_APLDeltaTh[28] = 200; // 3584
      BSS_APLDeltaTh[29] = 200; // 3712
      BSS_APLDeltaTh[30] = 200; // 3840
      BSS_APLDeltaTh[31] = 200; // 3968
      BSS_APLDeltaTh[32] = 200; // 4095
      BSS_GradRatioTh[0] = 1500;
      BSS_GradRatioTh[1] = 2000;
      BSS_GradRatioTh[2] = 2500;
      BSS_GradRatioTh[3] = 3000;
      BSS_GradRatioTh[4] = 4000;
      BSS_GradRatioTh[5] = 6000;
      BSS_GradRatioTh[6] = 8000;
      BSS_GradRatioTh[7] = 10000;
  }
};

struct BSS_CONVERT_STRUCT
{
  MUINT8 * pu1Y;
  MUINT8 * pu1U;
  MUINT8 * pu1V;
  MUINT8 * pu1Out;
  MINT32 u4Width;
  MINT32 u4Height;
  MINT32 u4Core;

    // constructor
    BSS_CONVERT_STRUCT()
    {
        ::memset(this, 0x0, sizeof(struct BSS_CONVERT_STRUCT));
    }
};

struct Gmv {
  MINT32  x;
  MINT32  y;

  // constructor
  Gmv()
        : x(0), y(0)
  {}
};

struct BSS_INPUT_DATA {
  MUINT32 inWidth;
  MUINT32 inHeight;
  MUINT32 fdWidth;
  MUINT32 fdHeight;

#ifdef D_BSS_FORCE_BEST_IDX
  MUINT32 i4BSFEN; // force enable
  MUINT32 i4BSFBI; // force best index
#endif

  MUINT8 *apbyBssInImg[MAX_FRAME_NUM];

  Gmv gmv[MAX_FRAME_NUM];
  BssFaceMetadata *Face[MAX_FRAME_NUM];
  MUINT32 u4AGain[MAX_FRAME_NUM];
  MUINT32 u4DGain[MAX_FRAME_NUM];
  MUINT32 u4ExpT[MAX_FRAME_NUM];
  MUINT16 *u2Lcso[MAX_FRAME_NUM];

  BSS_GYRO_INFO *prGyroInfo;
  MUINT32 u4GyroNum;
  MUINT32 u4GyroIntervalMS;

  // constructor
  BSS_INPUT_DATA()
  {
      ::memset(this, 0x0, sizeof(struct BSS_INPUT_DATA));
  }
};

struct BSS_INPUT_DATA_G {

  MUINT32 Bitnum;
  MUINT32 BayerOrder;
  MUINT32 Stride;

  MUINT32 inWidth;
  MUINT32 inHeight;
  MUINT32 fdWidth;
  MUINT32 fdHeight;

  BSS_PROC_TYPE eType;
  MBOOL InputZip;

#ifdef D_BSS_FORCE_BEST_IDX
  MUINT32 i4BSFEN; // force enable
  MUINT32 i4BSFBI; // force best index
#endif

  MUINT8 *apbyBssInImg[MAX_FRAME_NUM];

  Gmv gmv[MAX_FRAME_NUM];
  BssFaceMetadata *Face[MAX_FRAME_NUM];
  MUINT32 u4AGain[MAX_FRAME_NUM];
  MUINT32 u4DGain[MAX_FRAME_NUM];
  MUINT32 u4ExpT[MAX_FRAME_NUM];
  MUINT16 *u2Lcso[MAX_FRAME_NUM];

  BSS_GYRO_INFO *prGyroInfo;
  MUINT32 u4GyroNum;
  MUINT32 u4GyroIntervalMS;

  // constructor
  BSS_INPUT_DATA_G()
  {
      ::memset(this, 0x0, sizeof(struct BSS_INPUT_DATA_G));
  }
};

struct BSS_OUTPUT_DATA
{
  MUINT32 originalOrder[MAX_FRAME_NUM];
  Gmv gmv[MAX_FRAME_NUM];
  long long SharpScore[MAX_FRAME_NUM];
  long long adj1_score[MAX_FRAME_NUM];
  long long adj2_score[MAX_FRAME_NUM];
  long long adj3_score[MAX_FRAME_NUM];
  long long final_score[MAX_FRAME_NUM];
  MINT32 AvgPxLvl[MAX_FRAME_NUM];
  MUINT32 i4SkipFrmCnt;
  MUINT32 u4DGain[MAX_FRAME_NUM];
  MUINT16 *u2Lcso[MAX_FRAME_NUM];

  // constructor
  BSS_OUTPUT_DATA()
  {
      ::memset(this, 0x0, sizeof(struct BSS_OUTPUT_DATA));
  }
};

struct BSS_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];

    // constructor
    BSS_VerInfo()
    {
      ::memset(this, 0x0, sizeof(struct BSS_VerInfo));
    }
} ;                      // get Version by FeatureCtrl(GET_VERSION)


/*****************************************************************************
  Main Module
******************************************************************************/


/*****************************************************************************
  BSS INIT
******************************************************************************/


/*******************************************************************************
*
********************************************************************************/
class MTKBss
{
public:
    static MTKBss* createInstance(DrvBssObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKBss(){}
    virtual MRESULT BssInit(void* pParaIn, void* pParaOut);
    virtual MRESULT BssReset(void);
    virtual MRESULT BssMain(BSS_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT BssFeatureCtrl(BSS_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppBssTmp : public MTKBss
{
public:

    static MTKBss* getInstance();
    virtual void destroyInstance();

    AppBssTmp() {};
    virtual ~AppBssTmp() {};
};
#endif

