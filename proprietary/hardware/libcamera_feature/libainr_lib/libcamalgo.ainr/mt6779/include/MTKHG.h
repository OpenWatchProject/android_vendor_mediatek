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

#ifndef _MTK_HG_H
#define _MTK_HG_H

#include "MTKHGType.h"
#include "MTKHGErrCode.h"

#define FEO_IN_LEN   (20)
#define FEO_OUT_LEN (16)

typedef enum DRVHGObject_s
{
    DRV_HG_OBJ_NONE = 0,
    DRV_HG_OBJ_SW,
    DRV_HG_OBJ_UNKNOWN = 0xFF,
} DrvHGObject_e;

/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum HG_PROC_ENUM
{
    HG_PROC1 = 0,
    HG_UNKNOWN_PROC,
} HG_PROC_ENUM;

typedef enum HG_FTCTRL_ENUM
{
    HG_FTCTRL_GET_PROC_INFO,
    HG_FTCTRL_SET_PROC_INFO,
    HG_FTCTRL_PROC_DEC,
    HG_FTCTRL_PROC_HG,
    HG_FTCTRL_PROC_GRID,
    HG_FTCTRL_GET_VERSION,        // feature id to get Version
    HG_FTCTRL_MAX
} HG_FTCTRL_ENUM;

struct HG_GET_PROC_INFO_IN_STRUCT
{
    MINT32 i4MaxFP;

    // constructor
    HG_GET_PROC_INFO_IN_STRUCT()
    : i4MaxFP(0)
    {}
};

struct HG_GET_PROC_INFO_OUT_STRUCT
{
    MINT32 i4ReqWMSize;

    // constructor
    HG_GET_PROC_INFO_OUT_STRUCT()
    : i4ReqWMSize(0)
    {}
};


struct HG_SET_PROC_INFO_IN_STRUCT
{
    void *pvWM;
    MINT32 i4WMSize;

    // constructor
    HG_SET_PROC_INFO_IN_STRUCT()
    : pvWM(NULL), i4WMSize(0)
    {}
};

struct HG_PROC1_IN_STRUCT
{
    MINT16 *pi2BaseFe;  // size: i4MaxFP*sizeof(MINT16)*FE_DATA_LEN
    MINT16 *pi2NewFe;
    MINT16 *pi2BaseFm;
    MINT16 *pi2NewFm;

    // grid data
    MINT32 i4SrcWidth;
    MINT32 i4SrcHeight;
    MINT32 i4RzWidth;
    MINT32 i4RzHeight;
    MINT32 i4WCP;
    MINT32 i4HCP;

    // map size
    MINT32 MapWidth;
    MINT32 MapHeight;

    // for debug
    MINT32 i4Idx;
    MUINT32 u4Rand;

    // constructor
    HG_PROC1_IN_STRUCT()
    : i4Idx(0), u4Rand(0)
    {}
};

struct HG_PROC1_OUT_STRUCT
{
    double dMartix[9];
    MINT32 i4FP;
    MFLOAT fBestInlierRatio;

    MINT32 MapAllocSize;
    MINT32 *pi4MapX;
    MINT32 *pi4MapY;

    // constructor
    HG_PROC1_OUT_STRUCT()
    : MapAllocSize(0)
    {}
};

struct HG_PROC_DEC_IN_STRUCT
{
    MINT16 *pi2BaseFe;  // size: i4MaxFP*sizeof(MINT16)*FE_DATA_LEN
    MINT16 *pi2NewFe;
    MINT16 *pi2BaseFm;
    MINT16 *pi2NewFm;
    MINT32 i4MaxFP;

    // constructor
    HG_PROC_DEC_IN_STRUCT()
    : i4MaxFP(0)
    {}
};

struct HG_PROC_DEC_OUT_STRUCT
{
    void *pvBaseFeOut;  // size: i4MaxFP*16
    void *pvNewFeOut;
    MFLOAT *pfBaseX;    // size: i4MaxFP*4
    MFLOAT *pfBaseY;
    MFLOAT *pfNewX;
    MFLOAT *pfNewY;
    MINT32 i4MatchFP;

    // constructor
    HG_PROC_DEC_OUT_STRUCT()
    : i4MatchFP(0)
    {}
};

struct HG_PROC_HG_IN_STRUCT
{
    MFLOAT *pfBaseX;    // size: i4MaxFP*4
    MFLOAT *pfBaseY;
    MFLOAT *pfNewX;
    MFLOAT *pfNewY;
    MINT32 i4MatchFP;

    MUINT32 u4Rand;
};

struct HG_PROC_HG_OUT_STRUCT
{
    double dMartix[9];
    MINT32 i4FP;
    MFLOAT fBestInlierRatio;
};

struct HG_PROC_GRID_IN_STRUCT
{
    double fMartix[9];
    MINT32 i4SrcWidth;
    MINT32 i4SrcHeight;
    MINT32 i4RzWidth;
    MINT32 i4RzHeight;
    MINT32 i4WCP;
    MINT32 i4HCP;
};

struct HG_PROC_GRID_OUT_STRUCT
{
    MINT32 MapWidth;
    MINT32 MapHeight;
    MINT32 MapAllocSize;
    MINT32 *pi4MapX;
    MINT32 *pi4MapY;
};



struct HG_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)

/*******************************************************************************
*
********************************************************************************/
class MTKHG
{
public:
    static MTKHG* createInstance(DrvHGObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKHG(){}
    virtual MRESULT HGInit(void* pParaIn, void* pParaOut);
    virtual MRESULT HGReset(void);
    virtual MRESULT HGMain(HG_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT HGFeatureCtrl(HG_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppHGTmp : public MTKHG
{
public:

    static MTKHG* getInstance();
    virtual void destroyInstance();

    AppHGTmp() {};
    virtual ~AppHGTmp() {};
};
#endif

