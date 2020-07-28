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

#ifndef _MTK_MFBLL_H
#define _MTK_MFBLL_H

#include "MTKMfbllType.h"
#include "MTKMfbllErrCode.h"

#define FLOW_V20  // compatiable with V15
#ifdef FLOW_V20
  #define FLOW_V15
  #define MEMC_SUPPORT_YV16_16ALIGN
  #define CO_BRANCH
#else
  #define FLOW_V15
  #define MEMC_SUPPORT_YV16_16ALIGN
#endif
//#define FOR_SIM


typedef enum DRVMfbllObject_s
{
    DRV_MFBLL_OBJ_NONE = 0,
    DRV_MFBLL_OBJ_SW,
    DRV_MFBLL_OBJ_UNKNOWN = 0xFF,
} DrvMfbllObject_e;

/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum MFBLL_PROC_ENUM
{
    MFBLL_PROC1 = 0,
    MFBLL_PROC2,
    MFBLL_UNKNOWN_PROC,
} MFBLL_PROC_ENUM;

typedef enum MFBLL_FTCTRL_ENUM
{
    MFBLL_FTCTRL_GET_PROC_INFO,
    MFBLL_FTCTRL_SET_PROC_INFO,
    MFBLL_FTCTRL_MAX
} MFBLL_FTCTRL_ENUM;

/*****************************************************************************
  MFBLL INIT
******************************************************************************/
typedef struct MFBLL_INIT_PARAM
{
    MUINT16 Proc1_imgW;
    MUINT16 Proc1_imgH;
    MUINT32 core_num;
#ifdef FLOW_V20
    MBOOL Proc1_MPME;
    MBOOL Proc1_ConfMap;
#endif
#ifdef FOR_SIM
    char achDmpPath[512];
#endif
} MFBLL_INIT_PARAM_STRUCT,*P_MFBLL_INIT_PARAM_STRUCT;

/*****************************************************************************
  PROC1 PART (Type definition should be exactly same as that in core Proc1)
******************************************************************************/
typedef enum PROC_IMAGE_FORMAT
{
    PROC1_FMT_YV16 = 0,
    PROC1_FMT_YUY2,
    PROC1_FMT_Y,
    PROC1_FMT_MAX      // maximum image format enum
} PROC_IMAGE_FORMAT;

typedef struct MFBLL_PROC1_OUT
{
    MUINT8   bSkip_proc;
#ifdef FLOW_V20
    MUINT8 *pu1ConfMap;
    MUINT32 u4MapSize;
#endif
} MFBLL_PROC1_OUT_STRUCT,*P_MFBLL_PROC1_OUT_STRUCT;

typedef struct
{
    PROC_IMAGE_FORMAT ImgFmt;

    #ifdef FLOW_V15
    MUINT8 *pbyInImg;
    MUINT32  i4InWidth;
    MUINT32  i4InHeight;

    MUINT8 *pbyOuImg;
    MUINT32  i4OuWidth;
    MUINT32  i4OuHeight;
    #else
    MUINT8 *ResultImg;
    MUINT32  Width;
    MUINT32  Height;
    #endif
} MFBLL_PROC2_OUT_STRUCT,*P_MFBLL_PROC2_OUT_STRUCT;

typedef struct
{
    MUINT32 Ext_mem_size;
} MFBLL_GET_PROC_INFO_STRUCT, *P_MFBLL_GET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT8  *workbuf_addr;
    MUINT32  buf_size;
    MUINT8  *Proc1_base;
    MUINT8  *Proc1_ref;
    MUINT32  Proc1_width;
    MUINT32  Proc1_height;
    MINT32   Proc1_V1;
    MINT32   Proc1_V2;
    MUINT8   Proc1_bad_range;
    MUINT32  Proc1_bad_TH;
    PROC_IMAGE_FORMAT Proc1_ImgFmt;
    MINT32   Proc2_full_size;
#ifdef FLOW_V20
    MINT8   Noise_lvl;
#endif
#ifdef FOR_SIM
    MINT8   Proc_idx;
#endif
} MFBLL_SET_PROC_INFO_STRUCT, *P_MFBLL_SET_PROC_INFO_STRUCT;

/*******************************************************************************
*
********************************************************************************/
class MTKMfbll
{
public:
    static MTKMfbll* createInstance(DrvMfbllObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKMfbll(){}
    virtual MRESULT MfbllInit(void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllReset(void);
    virtual MRESULT MfbllMain(MFBLL_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllFeatureCtrl(MFBLL_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppMfbllTmp : public MTKMfbll
{
public:

    static MTKMfbll* getInstance();
    virtual void destroyInstance();

    AppMfbllTmp() {};
    virtual ~AppMfbllTmp() {};
};
#endif

