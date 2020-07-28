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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_OWE_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_OWE_COMMON_H_
//
#include <vector>
#include <mtkcam/def/common.h>


#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#include <utils/Trace.h>
#define DRV_TRACE_NAME_LENGTH                   32
#define DRV_TRACE_CALL()                        ATRACE_CALL()
#define DRV_TRACE_NAME(name)                    ATRACE_NAME(name)
#define DRV_TRACE_INT(name, value)              ATRACE_INT(name, value)
#define DRV_TRACE_BEGIN(name)                   ATRACE_BEGIN(name)
#define DRV_TRACE_END()                         ATRACE_END()
#define DRV_TRACE_ASYNC_BEGIN(name, cookie)     ATRACE_ASYNC_BEGIN(name, cookie)
#define DRV_TRACE_ASYNC_END(name, cookie)       ATRACE_ASYNC_END(name, cookie)
#define DRV_TRACE_FMT_BEGIN(fmt, arg...)                    \
do{                                                         \
    if( ATRACE_ENABLED() )                                  \
    {                                                       \
        char buf[DRV_TRACE_NAME_LENGTH];                    \
        snprintf(buf, DRV_TRACE_NAME_LENGTH, fmt, ##arg);   \
        DRV_TRACE_BEGIN(buf);                               \
    }                                                       \
}while(0)
#define DRV_TRACE_FMT_END()                     DRV_TRACE_END()



/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
/*
 * Sub-engine Enums
 */
enum SUB_ENGINE_ID{
   eOCC = 0,
   eWMFE = 1,
   eENGINE_MAX
};

enum OWEIMGIFORMAT {
    OWE_IMGI_Y_FMT = 0,
    OWE_IMGI_YC_FMT,
    OWE_IMGI_CY_FMT
};


enum OWEDMAPort {
    DMA_OCC_NONE = 0,
    DMA_OCC_REF_VEC,
    DMA_OCC_REF_PXL,
    DMA_OCC_MAJ_VEC,
    DMA_OCC_MAJ_PXL,
    DMA_OCC_WDMAO,
    DMA_OWMFE_IMGI,
    DMA_OWMFE_DPI,
    DMA_OWMFE_TBLI,
    DMA_OWMFE_DPO,
    DMA_OWMFE_MASKI
};

struct EGNBufInfo
{
    OWEDMAPort  dmaport;
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;    //  Vir Address of pool
    MUINTPTR    u4BufPA;    //  Phy Address of pool
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4Stride;  //  Buffer Stride
    MUINT32     is_sec;
        EGNBufInfo()
        : dmaport(DMA_OCC_NONE)
        , memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        , is_sec(0)
        {
        }
};

struct owe_feedback {
    MUINT32 reg1;
    MUINT32 reg2;
    owe_feedback(): reg1(0)
              , reg2(0)
              {}
};

struct  OCCConfig
{
#if 0
    _DPE_REG_OCC_CTRL_0_              OCC_CTRL_0;  /* 0030, 0x1502C030 */
    _DPE_REG_OCC_CTRL_1_              OCC_CTRL_1;  /* 0034, 0x1502C034 */
    _DPE_REG_OCC_CTRL_2_              OCC_CTRL_2;  /* 0038, 0x1502C038 */
    _DPE_REG_OCC_CTRL_3_              OCC_CTRL_3;  /* 003C, 0x1502C03C */
    _DPE_REG_OCC_PQ_0_                OCC_PQ_0;    /* 0068, 0x1502C068 */
    _DPE_REG_OCC_PQ_1_                OCC_PQ_1;    /* 006C, 0x1502C06C */
    _DPE_REG_OCC_SPARE_               OCC_SPARE;   /* 0070, 0x1502C070 */
#else
    MUINT32 occ_scan_r2l     ;		/*  0.. 0, 0x00000001 */
    MUINT32 occ_horz_ds4     ;		/*  1.. 1, 0x00000002 */
    MUINT32 occ_vert_ds4     ;		/*  2.. 2, 0x00000004 */
    MUINT32 occ_h_skip_mode  ;		/*  3.. 3, 0x00000008 */
    OWEIMGIFORMAT occ_imgi_maj_fmt ;		/*  8.. 9, 0x00000300 */
    OWEIMGIFORMAT occ_imgi_ref_fmt ;		/* 10..11, 0x00000C00 */
    MUINT32 occ_hsize        ;		/*  0.. 9, 0x000003FF */
    MUINT32 occ_vsize        ;		/* 16..27, 0x0FFF0000 */
    MUINT32 occ_v_crop_s     ;		/*  0..11, 0x00000FFF */
    MUINT32 occ_v_crop_e     ;		/* 16..27, 0x0FFF0000 */
    MUINT32 occ_h_crop_s     ;		/*  0.. 9, 0x000003FF */
    MUINT32 occ_h_crop_e     ;		/* 16..25, 0x03FF0000 */
    MUINT32 occ_th_luma      ;		/*  0.. 7, 0x000000FF */
    MUINT32 occ_th_h         ;		/*  8..15, 0x0000FF00 */
    MUINT32 occ_th_v         ;		/* 16..19, 0x000F0000 */
    MUINT32 occ_vec_shift    ;		/* 20..22, 0x00700000 */
    MUINT32 occ_vec_offset   ;		/* 24..31, 0xFF000000 */
    MUINT32 occ_invalid_value;		/*  0.. 7, 0x000000FF */
    MUINT32 occ_owc_th       ;		/*  8..15, 0x0000FF00 */
    MUINT32 occ_owc_en       ;		/* 16..16, 0x00010000 */
    MUINT32 occ_depth_clip_en;		/* 17..17, 0x00020000 */
    MUINT32 occ_spare        ;		/*  0..31, 0xFFFFFFFF */
#endif
    MUINT32 egn_secured;
    EGNBufInfo                      OCC_REF_VEC;
    EGNBufInfo                      OCC_REF_PXL;
    EGNBufInfo                      OCC_MAJ_VEC;
    EGNBufInfo                      OCC_MAJ_PXL;
    EGNBufInfo                      OCC_WDMA;
    owe_feedback                        feedback;
};

enum OWMFEFILTERSIZE {
    OWMFE_FILTER_SIZE_1x1 = 0,
    OWMFE_FILTER_SIZE_3x3,
    OWMFE_FILTER_SIZE_5x5,
    OWMFE_FILTER_SIZE_7x7,
};

enum OWMFEDPIFORMAT {
    OWMFE_DPI_D_FMT = 0,
    OWMFE_DPI_DX_FMT,
    OWMFE_DPI_XD_FMT
};

enum OWMFEVERTSCANORDER {
    OWMFE_VERT_TOP2BOTTOM = 0,
    OWMFE_VERT_BOTTOM2TOP
};

enum OWMFEHORZSCANORDER {
    OWMFE_HORZ_LEFT2RIGHT = 0,
    OWMFE_HORZ_RIGHT2LEFT
};


struct  OWMFECtrl
{
    bool Wmfe_Enable;
    OWMFEFILTERSIZE WmfeFilterSize;
    MUINT32 Wmfe_Width;
    MUINT32 Wmfe_Height;
    OWEIMGIFORMAT WmfeImgiFmt;
    OWMFEDPIFORMAT WmfeDpiFmt;
    MUINT32 egn_secured;
    EGNBufInfo Wmfe_Imgi;
    EGNBufInfo Wmfe_Dpi;
    EGNBufInfo Wmfe_Tbli;
    EGNBufInfo Wmfe_Dpo;
    EGNBufInfo Wmfe_Maski;  //V1.2
    bool Wmfe_Dpnd_En;      //V1.2
    bool Wmfe_Mask_En;      //V1.2
    OWMFEHORZSCANORDER WmfeHorzScOrd; //V1.2
    OWMFEVERTSCANORDER WmfeVertScOrd; //V1.2
    MUINT32 Wmfe_Mask_Value;   //V1.2
    MUINT32 Wmfe_Mask_Mode;    //V1.2
    MUINT32 Wmfe_Chroma_En;    //V1.3
    //
    OWMFECtrl()     //HW Default value
        : Wmfe_Enable(false)
        , WmfeFilterSize(OWMFE_FILTER_SIZE_1x1)
        , Wmfe_Width(0)
        , Wmfe_Height(0)
        , WmfeImgiFmt(OWE_IMGI_Y_FMT)
        , WmfeDpiFmt(OWMFE_DPI_D_FMT)
        , egn_secured(0)
        , Wmfe_Dpnd_En(false)
        , Wmfe_Mask_En(false)
        , WmfeHorzScOrd(OWMFE_HORZ_LEFT2RIGHT)
        , WmfeVertScOrd(OWMFE_VERT_TOP2BOTTOM)
        , Wmfe_Mask_Value(0)
        , Wmfe_Mask_Mode(0)
        , Wmfe_Chroma_En(0)
    {

    }
};



struct  OWMFEConfig
{
    std::vector<OWMFECtrl> mWMFECtrlVec;
    owe_feedback                        feedback;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_OWE_COMMON_H_

