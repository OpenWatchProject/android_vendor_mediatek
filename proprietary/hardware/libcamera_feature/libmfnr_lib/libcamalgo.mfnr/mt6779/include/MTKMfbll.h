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

#include <string.h>

#include "MTKMfbllType.h"
#include "MTKMfbllErrCode.h"

//#define FOR_SIM
//#define KEEP_PROC2
#define PROC1_DN_RATIO 4

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
#ifdef KEEP_PROC2
    MFBLL_PROC2,
#endif
    MFBLL_UNKNOWN_PROC,
} MFBLL_PROC_ENUM;

typedef enum MFBLL_FTCTRL_ENUM
{
    MFBLL_FTCTRL_GET_PROC_INFO,
    MFBLL_FTCTRL_SET_PROC_INFO,
    MFBLL_FTCTRL_GET_VERSION,        // feature id to get Version
    MFBLL_FTCTRL_MAX
} MFBLL_FTCTRL_ENUM;

/*****************************************************************************
  MFBLL INIT
******************************************************************************/
struct MFBLL_INIT_PARAM_STRUCT
{
    MUINT16 Proc1_imgW;
    MUINT16 Proc1_imgH;
    MUINT32 core_num;
    MBOOL Proc1_DSUS_mode;   // 0: 1/4, 1: 1/2
#ifdef FOR_SIM
    char achDmpPath[512];
#endif

    // constructor
    MFBLL_INIT_PARAM_STRUCT()
    : core_num(4)
    {}
};

/*****************************************************************************
  PROC1 PART (Type definition should be exactly same as that in core Proc1)
******************************************************************************/
typedef enum PROC_IMAGE_FORMAT
{
    PROC1_FMT_YV16 = 0, // 422 : 3 plane , Y..U..V..
    PROC1_FMT_YUY2, // 422 : YUV YUV ...
    PROC1_FMT_NV12, // 420 : 2 plane , Y... UVUV..
    PROC1_FMT_Y,
    PROC1_FMT_Y16bit,
    PROC1_FMT_MAX      // maximum image format enum
} PROC_IMAGE_FORMAT;

typedef struct MFBLL_PROC1_OUT
{
    MUINT8  bSkip_proc;
    MUINT8 *pu1ConfMap;
    MUINT32 u4MapSize;
    MUINT8 *pu1MV;
    MUINT32 u4MVSize;
    MUINT32 u4FrmLevel;
} MFBLL_PROC1_OUT_STRUCT,*P_MFBLL_PROC1_OUT_STRUCT;

#ifdef KEEP_PROC2
typedef struct
{
    PROC_IMAGE_FORMAT ImgFmt;

    MUINT8 *pbyInImg;
    MUINT32  i4InWidth;
    MUINT32  i4InHeight;

    MUINT8 *pbyOuImg;
    MUINT32  i4OuWidth;
    MUINT32  i4OuHeight;
} MFBLL_PROC2_OUT_STRUCT,*P_MFBLL_PROC2_OUT_STRUCT;
#endif

typedef struct
{
    MUINT32 Ext_mem_size;
} MFBLL_GET_PROC_INFO_STRUCT, *P_MFBLL_GET_PROC_INFO_STRUCT;

struct MFBLL_SET_PROC_INFO_STRUCT
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
    MUINT32  Proc1_noise_level;
    MUINT32  Proc1_Bst_FDROI_X0;
    MUINT32  Proc1_Bst_FDROI_Y0;
    MUINT32  Proc1_Bst_FDROI_X1;
    MUINT32  Proc1_Bst_FDROI_Y1;
    MUINT32  Proc1_Ref_FDROI_X0;
    MUINT32  Proc1_Ref_FDROI_Y0;
    MUINT32  Proc1_Ref_FDROI_X1;
    MUINT32  Proc1_Ref_FDROI_Y1;

    MUINT32  Proc1_Bst_LEYE_X0;
    MUINT32  Proc1_Bst_LEYE_X1;
    MUINT32  Proc1_Bst_LEYE_Y0;
    MUINT32  Proc1_Bst_LEYE_Y1;
    MUINT32  Proc1_Bst_LEYE_UX;
    MUINT32  Proc1_Bst_LEYE_UY;
    MUINT32  Proc1_Bst_LEYE_DX;
    MUINT32  Proc1_Bst_LEYE_DY;
    MUINT32  Proc1_Bst_REYE_X0;
    MUINT32  Proc1_Bst_REYE_X1;
    MUINT32  Proc1_Bst_REYE_Y0;
    MUINT32  Proc1_Bst_REYE_Y1;
    MUINT32  Proc1_Bst_REYE_UX;
    MUINT32  Proc1_Bst_REYE_UY;
    MUINT32  Proc1_Bst_REYE_DX;
    MUINT32  Proc1_Bst_REYE_DY;


    MUINT32  Proc1_Ref_LEYE_X0;
    MUINT32  Proc1_Ref_LEYE_X1;
    MUINT32  Proc1_Ref_LEYE_Y0;
    MUINT32  Proc1_Ref_LEYE_Y1;
    MUINT32  Proc1_Ref_LEYE_UX;
    MUINT32  Proc1_Ref_LEYE_UY;
    MUINT32  Proc1_Ref_LEYE_DX;
    MUINT32  Proc1_Ref_LEYE_DY;
    MUINT32  Proc1_Ref_REYE_X0;
    MUINT32  Proc1_Ref_REYE_X1;
    MUINT32  Proc1_Ref_REYE_Y0;
    MUINT32  Proc1_Ref_REYE_Y1;
    MUINT32  Proc1_Ref_REYE_UX;
    MUINT32  Proc1_Ref_REYE_UY;
    MUINT32  Proc1_Ref_REYE_DX;
    MUINT32  Proc1_Ref_REYE_DY;
    MUINT32  Proc1_me_refine_en;
    MUINT32  Proc1_me_refine_mv_ratio;

    MUINT32  Proc1_me_refine_face_ratio_ThL;
    MUINT32  Proc1_me_refine_face_ratio_ThH;
    MUINT32  Proc1_me_refine_full_ratio_ThL;
    MUINT32  Proc1_me_refine_full_ratio_ThH;
    MUINT32  Proc1_me_refine_edge_ccl_Th0;
    MUINT32  Proc1_me_refine_edge_ccl_Th1;
    MUINT32  Proc1_me_refine_edge_cclnum_Th;
    MUINT32  Proc1_me_refine_edge_FDAreaThL;
    MUINT32  Proc1_me_refine_edge_FDAreaThH;

    MUINT32  Proc1_me_lcl_deconf_en;
    MUINT32  Proc1_me_lcl_deconf_noise_lv;
    MUINT32  Proc1_me_lcl_deconf_bg_bss_ratio;
    MUINT32  Proc1_me_lcl_deconf_fd_bss_ratio;
    MUINT32  Proc1_me_lcl_deconf_dltvar_en;
    MUINT32  Proc1_me_large_mv_thd;
    MUINT32  Proc1_me_large_mv_SAD_thd;
    MUINT32  Proc1_me_large_mv_ratio;
    MBOOL    Proc1_me_large_mv_txtr_en;
    MUINT32  Proc1_me_large_mv_txtr_wei;
    MUINT32  Proc1_me_large_mv_txtr_thd;
    MBOOL    Proc1_me_blink_eye_en;
    MUINT32    Proc1_me_blink_eye_ThL;
    MUINT32    Proc1_me_blink_eye_ThH;
    MUINT32    Proc1_me_blink_eye_qstep;

    PROC_IMAGE_FORMAT Proc1_ImgFmt;
    MINT8   Noise_lvl;

    MUINT32 iBssOrgScore_base;
    MUINT32 iBssOrgScore_ref;

    MINT8   Proc_idx;  //debug

    // constructor
    MFBLL_SET_PROC_INFO_STRUCT()
    {
        memset(this, 0x0, sizeof(struct MFBLL_SET_PROC_INFO_STRUCT));
        Proc1_me_large_mv_ratio = 16;
    }
};
typedef MFBLL_SET_PROC_INFO_STRUCT*  P_MFBLL_SET_PROC_INFO_STRUCT;


struct MFBLL_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)

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

