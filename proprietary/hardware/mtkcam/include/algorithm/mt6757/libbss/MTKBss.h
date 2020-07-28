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

typedef enum BSS_FTCTRL_ENUM
{
  BSS_FTCTRL_SET_PROC_INFO,
  BSS_FTCTRL_CONVERT_I422_YUY2,
  BSS_FTCTRL_MAX
} BSS_FTCTRL_ENUM;

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
  MUINT32 BSS_ROI_WIDTH;
  MUINT32 BSS_ROI_HEIGHT;
  MUINT32 BSS_ROI_X0;
  MUINT32 BSS_ROI_Y0;
  MUINT32 BSS_SCALE_FACTOR;
  MUINT32 BSS_CLIP_TH0;
  MUINT32 BSS_CLIP_TH1;
  MUINT32 BSS_ZERO;
  MUINT32 BSS_FRAME_NUM;
  MUINT32 BSS_ADF_TH;
  MUINT32 BSS_SDF_TH;
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
};


struct Gmv {
  MINT32  x;
  MINT32  y;
};


struct BSS_INPUT_DATA {
  MUINT32 inWidth;
  MUINT32 inHeight;

#ifdef D_BSS_FORCE_BEST_IDX
  MUINT32 i4BSFEN; // force enable
  MUINT32 i4BSFBI; // force best index
#endif

  MUINT8 *apbyBssInImg[MAX_FRAME_NUM];


  Gmv gmv[MAX_FRAME_NUM];

  BSS_GYRO_INFO *prGyroInfo;
  MUINT32 u4GyroNum;
  MUINT32 u4GyroIntervalMS;

  // constructor
  BSS_INPUT_DATA()
                        : prGyroInfo(NULL), u4GyroNum(0), u4GyroIntervalMS(0)
  {
  }
};

struct BSS_INPUT_DATA_G {
  
  MUINT32 Bitnum;
  MUINT32 BayerOrder;
  MUINT32 Stride;

  MUINT32 inWidth;
  MUINT32 inHeight;

#ifdef D_BSS_FORCE_BEST_IDX
  MUINT32 i4BSFEN; // force enable
  MUINT32 i4BSFBI; // force best index
#endif

  MUINT8 *apbyBssInImg[MAX_FRAME_NUM];

  Gmv gmv[MAX_FRAME_NUM];

  BSS_GYRO_INFO *prGyroInfo;
  MUINT32 u4GyroNum;
  MUINT32 u4GyroIntervalMS;

  // constructor
  BSS_INPUT_DATA_G()
                        : prGyroInfo(NULL), u4GyroNum(0), u4GyroIntervalMS(0)
  {
  }
};

struct BSS_OUTPUT_DATA {
  MUINT32 originalOrder[MAX_FRAME_NUM];
  Gmv gmv[MAX_FRAME_NUM];
  MUINT32 i4SkipFrmCnt;
};

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

