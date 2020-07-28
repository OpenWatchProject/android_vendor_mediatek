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

#ifndef _MTK_AIS_H
#define _MTK_AIS_H

#include "MTKAisType.h"
#include "MTKAisErrCode.h"

typedef enum AIS_PROC_ENUM
{
    AIS_PROC1 = 0,
    AIS_PROC2,
    AIS_UNKNOWN_PROC,
} AIS_PROC_ENUM;

typedef enum DRVAisObject_s
{
    DRV_AIS_OBJ_NONE = 0,
    DRV_AIS_OBJ_SW,
    DRV_AIS_OBJ_UNKNOWN = 0xFF,
} DrvAisObject_e;

typedef enum AIS_FTCTRL_ENUM
{
  AIS_FTCTRL_SET_PROC1_INFO,
  AIS_FTCTRL_GET_PROC2_INFO,
  AIS_FTCTRL_SET_PROC2_INFO,
  AIS_FTCTRL_GET_VERSION,        // feature id to get Version
  AIS_FTCTRL_MAX
} AIS_FTCTRL_ENUM;

struct AIS_GYRO_INFO {
  MFLOAT fX;
  MFLOAT fY;
  MFLOAT fZ;
  MUINT64 ts;

  // constructor
  AIS_GYRO_INFO()
           : fX(0), fY(0), fZ(0), ts(0)
  {}
};

struct AIS_Gmv {
  MINT32 i4X;
  MINT32 i4Y;

  // constructor
  AIS_Gmv()
           : i4X(0), i4Y(0)
  {}
};

struct AIS_INIT_PARAM
{
    AIS_PROC_ENUM eProcID;

    AIS_INIT_PARAM()
                         : eProcID(AIS_PROC1)
    {}
};

struct AIS_SET_PROC1_IN_INFO
{
    MUINT16 u2IsoLvl1;
    MUINT16 u2IsoLvl2;
    MUINT16 u2IsoLvl3;
    MUINT16 u2IsoLvl4;
    MUINT16 u2IsoLvl5;
    MUINT8 u1FrmNum1;
    MUINT8 u1FrmNum2;
    MUINT8 u1FrmNum3;
    MUINT8 u1FrmNum4;
    MUINT8 u1FrmNum5;
    MUINT8 u1FrmNum6;
    MUINT8 u1SrcNum1;
    MUINT8 u1SrcNum2;
    MUINT8 u1SrcNum3;
    MUINT8 u1SrcNum4;
    MUINT8 u1SrcNum5;
    MUINT8 u1SrcNum6;
};

struct AIS_PROC1_PARA_IN
{
    MUINT32 u4CurrentIso;
};

struct AIS_PROC1_PARA_OUT
{
    MUINT8 u1ReqFrmNum;
    MUINT8 u1ReqSrcNum;
};

struct AIS_GET_PROC2_INFO
{
    MUINT32 u4WorkingBufSize;
};

struct AIS_SET_PROC2_INFO
{
    MUINT8  *pu1WorkingBuf;
    MUINT32  u4WorkingBufSize;

    // Proc1
    MUINT16 u2IsoLvl1;
    MUINT16 u2IsoLvl2;
    MUINT16 u2IsoLvl3;
    MUINT16 u2IsoLvl4;
    MUINT16 u2IsoLvl5;
    MUINT8 u1FrmNum1;
    MUINT8 u1FrmNum2;
    MUINT8 u1FrmNum3;
    MUINT8 u1FrmNum4;
    MUINT8 u1FrmNum5;
    MUINT8 u1FrmNum6;
    MUINT8 u1SrcNum1;
    MUINT8 u1SrcNum2;
    MUINT8 u1SrcNum3;
    MUINT8 u1SrcNum4;
    MUINT8 u1SrcNum5;
    MUINT8 u1SrcNum6;

    // gyro
    MFLOAT fRsc;
    MFLOAT fBiasX;
    MFLOAT fBiasY;
    MFLOAT fBiasZ;
    MFLOAT fFocal;
    MFLOAT fTofst;
    MUINT32 u4MotionBase;
    MUINT32 u4MVThLow;
    MUINT32 u4MVThHigh;

    MUINT32 u4MinExp;
    MUINT32 u4MaxIso;

};

struct AIS_PROC2_PARA_IN
{
    MUINT32 u4CurrentIso;
    MUINT32 u4CurrentExp;

    AIS_GYRO_INFO *prGyroInfo;
    MUINT32 u4GyroNum;
    MUINT32 u4GyroIntervalMS;
    MUINT32 u4sensorWidth;
    MUINT32 u4sensorHeight;
    MUINT32 u4paramWidth;
    MUINT32 u4paramHeight;

    // constructor
    AIS_PROC2_PARA_IN()
                         : prGyroInfo(NULL), u4GyroNum(0), u4GyroIntervalMS(0)
    {
    }

};

struct AIS_PROC2_PARA_OUT
{
    MUINT32 u4NewIso;
    MUINT32 u4NewExp;
    MUINT8 u1ReqFrmNum;
    MUINT8 u1ReqSrcNum;
};

struct AIS_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)


/*******************************************************************************
*
********************************************************************************/
class MTKAis
{
public:
    static MTKAis* createInstance(DrvAisObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKAis(){}
    virtual MRESULT AisInit(void* pParaIn, void* pParaOut);
    virtual MRESULT AisReset(void);
    virtual MRESULT AisMain(AIS_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT AisFeatureCtrl(AIS_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppAisTmp : public MTKAis
{
public:

    static MTKAis* getInstance();
    virtual void destroyInstance();

    AppAisTmp() {};
    virtual ~AppAisTmp() {};
};
#endif

