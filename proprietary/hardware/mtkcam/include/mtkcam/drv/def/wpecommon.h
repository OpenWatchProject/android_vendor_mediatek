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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_COMMON_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam/def/ImageFormat.h>


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
/*WPE_CTRL*/
enum WPEDMAPort {
    DMA_WPE_NONE = 0,
    DMA_WPE_TDR,
    DMA_WPE_VECI,
    DMA_WPE_VEC2I,
    DMA_WPE_VEC3I,
    DMA_WPE_WPEO,
    DMA_WPE_MASKO,
    DMA_WPE_IMGI,
    DMA_WPE_NUM,
};

struct WPEBuf_Info
{
    WPEDMAPort dmaport;
    MINT32      memID;          //  memory ID
    MUINTPTR    u4BufVA[3];        //  Vir Address of pool
    MUINTPTR    u4BufPA[3];        //  Phy Address of pool
    MUINT32     u4BufSize[3];      //  Per buffer size
    MUINT32     u4Stride[3];       //  Buffer Stride
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
    MUINT32     u4imgFmt;       //  Image Format
    MUINT32     planeNum;
    MUINT32     bitsPerPixel;
    MUINT32     secureTag;
        WPEBuf_Info()
        : dmaport(DMA_WPE_NONE)
        , memID(0)
        , u4ImgWidth(0)
        , u4ImgHeight(0)
        , u4imgFmt(0)
        , planeNum(1)
        , bitsPerPixel(8)
        , secureTag(0)
        {
        }
};


struct WPEBufInfo
{
    WPEDMAPort dmaport;
    MINT32      memID;          //  memory ID
    MUINTPTR    u4BufVA;        //  Vir Address of pool
    MUINTPTR    u4BufPA;        //  Phy Address of pool
    MUINT32     u4BufSize;      //  Per buffer size
    MUINT32     u4Stride;       //  Buffer Stride
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
    MUINT32     u4ElementSzie;  //  ElementSzie
    MUINT32     u4imgFmt;
    MUINT32     planeNum;
    MUINT32     secureTag;
        WPEBufInfo()
        : dmaport(DMA_WPE_NONE)
        , memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        , u4ImgWidth(0)
        , u4ImgHeight(0)
        , u4ElementSzie(1)
        , u4imgFmt(0)
        , planeNum(1)
        , secureTag(0)
        {
        }
};

struct  WPEConfig
{
    MUINT32                        Wpe_TDR_EN;
    MUINT32                        Wpe_VGEN_EN;
    MUINT32                        Wpe_SYNC_EN;
    MUINT32                        Wpe_MDP_CROP_EN;
    MUINT32                        Wpe_ISP_CROP_EN;
    MUINT32                        Wpe_C24_EN;
    MUINT32                        Wpe_CTL_EXTENSION;
    MUINT32                        Wpe_TDR_SOF_RST_EN;
    MUINT32                        Wpe_TDR_SIZE_DETECT;
    MUINT32                        Wpe_CACHI_EN;
    MUINT32                        Wpe_VECI_EN;
    MUINT32                        Wpe_VEC2I_EN;
    MUINT32                        Wpe_VEC3I_EN;
    MUINT32                        Wpe_WPEO_EN;
    MUINT32                        Wpe_MSKO_EN;
    MUINT32                        Wpe_UV_INTER_EN;
    MUINT32                        Wpe_CACHE_BURST_LEN;
    MUINT32                        Wpe_CACHI_FMT_BIT;
    MUINT32                        Wpe_CACHI_PLANE;
    MUINT32                        Wpe_CACHI_FMT;
    MUINT32                        Wpe_CACHI_FMT_SEQ;
    MUINT32                        Wpe_WPE_DONE_EN;
    MUINT32                        Wpe_TDR_ERR_INT_EN;
    MUINT32                        Wpe_CACHE_MISS_INT_EN;
    MUINT32                        Wpe_CACHI_DONE_EN;
    MUINT32                        Wpe_VECI_DONE_EN;
    MUINT32                        Wpe_WPEO_DONE_EN;
    MUINT32                        Wpe_MSKO_DONE_EN;
    MUINT32                        Wpe_DMA_ERR_INT_EN;
    MUINT32                        Wpe_INT_WCLR_EN;
    MUINT32                        Wpe_TILE_EDGE;
    MUINT32                        Wpe_TILE_IRQ;
    MUINT32                        Wpe_LAST_TILE;
    MUINT32                        Wpe_TPIPE_CNT;
    MUINT32                        Wpe_LOAD_SIZE;
    MUINT32                        Wpe_SW_RST_ST;
    MUINT32                        Wpe_VGEN_VERTICAL_FIRST;
    MUINT32                        Wpe_VGEN_HMG_MODE;
    MUINT32                        Wpe_VGEN_IN_WD;
    MUINT32                        Wpe_VGEN_IN_HT;
    MUINT32                        Wpe_VGEN_OUT_WD;
    MUINT32                        Wpe_VGEN_OUT_HT;
    MUINT32                        Wpe_VGEN_HORI_STEP;
    MUINT32                        Wpe_VGEN_VERT_STEP;
    MUINT32                        Wpe_VGEN_HORI_INT_OFST;
    MUINT32                        Wpe_VGEN_HORI_SUB_OFST;
    MUINT32                        Wpe_VGEN_VERT_INT_OFST;
    MUINT32                        Wpe_VGEN_VERT_SUB_OFST;
    MUINT32                        Wpe_VGEN_POST_SUB_H;
    MUINT32                        Wpe_VGEN_POST_SUB_V;
    MUINT32                        Wpe_VGEN_POST_COMP_EN;
    MUINT32                        Wpe_VGEN_COMP_VEC_X;
    MUINT32                        Wpe_VGEN_COMP_VEC_Y;
    MUINT32                        Wpe_VGEN_VEC_WD;  
    MUINT32                        Wpe_VGEN_VEC_HT;
    MUINT32                        Wpe_VFC_SRAM_SIZE;
    MUINT32                        Wpe_CFIFO_SRAM_SIZE;
    MUINT32                        Wpe_CACHE_PREFETCH_MODE;
    MUINT32                        Wpe_C24_TILE_EDGE;
    MUINT32                        Wpe_MDP_CROP_STR_X;
    MUINT32                        Wpe_MDP_CROP_END_X;
    MUINT32                        Wpe_MDP_CROP_STR_Y;
    MUINT32                        Wpe_MDP_CROP_END_Y;
    MUINT32                        Wpe_ISPCROP_STR_X;
    MUINT32                        Wpe_ISPCROP_END_X;
    MUINT32                        Wpe_ISPCROP_STR_Y;
    MUINT32                        Wpe_ISPCROP_END_Y; 
    MUINT32                        Wpe_PSP_UNSIGN_TO_SIGN;
    MUINT32                        Wpe_PSP_SIGN_EXT;
    MUINT32                        Wpe_PSP_TBL_SEL_H;
    MUINT32                        Wpe_PSP_TBL_SEL_V;
    MUINT32                        Wpe_PSP2_UNSIGN_TO_SIGN;
    MUINT32                        Wpe_PSP2_SIGN_EXT;
    MUINT32                        Wpe_PSP2_TBL_SEL_H;
    MUINT32                        Wpe_PSP2_TBL_SEL_V;
    MUINT32                        Wpe_PSP_BORDER;
    MUINT32                        Wpe_ADDR_GEN_BASE_ADDR;
    MUINT32                        Wpe_ADDR_GEN_OFFSET_ADDR;
    MUINT32                        Wpe_ADDR_GEN_STRIDE;
    MUINT32                        Wpe_ADDR_GEN_FIFO_SIZE;
    MUINT32                        Wpe_ADDR_GEN_MAX_BURST_LEN;
    MUINT32                        Wpe_ADDR_GEN_ERR_EN;
    MUINT32                        Wpe_ADDR_GEN_ERR_STAT; 
    MUINT32                        Wpe_WPEO_SOFT_RST_STAT; 
    MUINT32                        Wpe_MSKO_SOFT_RST_STAT; 
    MUINT32                        Wpe_VECI_SOFT_RST_STAT; 
    MUINT32                        Wpe_VEC2I_SOFT_RST_STAT; 
    MUINT32                        Wpe_VEC3I_SOFT_RST_STAT; 
    MUINT32                        Wpe_TDRI_BASE_ADDR;
    MUINT32                        Wpe_TDRI_OFST_ADDR;
    MUINT32                        Wpe_TDRI_XSIZE;
    MUINT32                        Wpe_WPEO_V_FLIP_EN;
    MUINT32                        Wpe_MSKO_V_FLIP_EN;
    MUINT32                        Wpe_VECI_V_FLIP_EN;
    MUINT32                        Wpe_VEC2I_V_FLIP_EN;
    MUINT32                        Wpe_VEC3I_V_FLIP_EN;
    MUINT32                        Wpe_WPEO_BASE_ADDR;
    MUINT32                        Wpe_WPEO_OFST_ADDR;
    MUINT32                        Wpe_WPEO_XSIZE;
    MUINT32                        Wpe_WPEO_YSIZE;
    MUINT32                        Wpe_WPEO_STRIDE;
    MUINT32                        Wpe_WPEO_CON;
    MUINT32                        Wpe_WPEO_CON2;
    MUINT32                        Wpe_WPEO_CON3;
    MUINT32                        Wpe_WPEO_CROP;
    MUINT32                        Wpe_MSKO_BASE_ADDR;
    MUINT32                        Wpe_MSKO_OFST_ADDR;
    MUINT32                        Wpe_MSKO_XSIZE;
    MUINT32                        Wpe_MSKO_YSIZE;
    MUINT32                        Wpe_MSKO_STRIDE;
    MUINT32                        Wpe_MSKO_CON;
    MUINT32                        Wpe_MSKO_CON2;
    MUINT32                        Wpe_MSKO_CON3;
    MUINT32                        Wpe_MSKO_CROP;
    MUINT32                        Wpe_VECI_BASE_ADDR;
    MUINT32                        Wpe_VECI_OFST_ADDR;
    MUINT32                        Wpe_VECI_XSIZE;  
    MUINT32                        Wpe_VECI_YSIZE;
    MUINT32                        Wpe_VECI_STRIDE;
    MUINT32                        Wpe_VECI_CON;
    MUINT32                        Wpe_VECI_CON2;
    MUINT32                        Wpe_VECI_CON3;
    MUINT32                        Wpe_VEC2I_BASE_ADDR;
    MUINT32                        Wpe_VEC2I_OFST_ADDR;
    MUINT32                        Wpe_VEC2I_XSIZE;  
    MUINT32                        Wpe_VEC2I_YSIZE;
    MUINT32                        Wpe_VEC2I_STRIDE;
    MUINT32                        Wpe_VEC2I_CON;
    MUINT32                        Wpe_VEC2I_CON2;
    MUINT32                        Wpe_VEC2I_CON3;
    MUINT32                        Wpe_VEC3I_BASE_ADDR;
    MUINT32                        Wpe_VEC3I_OFST_ADDR;
    MUINT32                        Wpe_VEC3I_XSIZE;  
    MUINT32                        Wpe_VEC3I_YSIZE;
    MUINT32                        Wpe_VEC3I_STRIDE;
    MUINT32                        Wpe_VEC3I_CON;
    MUINT32                        Wpe_VEC3I_CON2;
    MUINT32                        Wpe_VEC3I_CON3;

    WPEBufInfo                     Wpe_TDR;
    WPEBufInfo                     Wpe_VECI;
    WPEBufInfo                     Wpe_VEC2I;
    WPEBufInfo                     Wpe_VEC3I;
    WPEBufInfo                     Wpe_WPEO;
    WPEBufInfo                     Wpe_MASKO;
    WPEBufInfo                     Wpe_CACHI;
    WPEBuf_Info                    Wpe_WPEO_AllFormats;
    WPEBuf_Info                    Wpe_CACHI_AllFormats;

                                    WPEConfig()     //HW Default value
                                        :Wpe_TDR_EN(0)
                                        ,Wpe_VGEN_EN(1)
                                        ,Wpe_SYNC_EN(0)
                                        ,Wpe_MDP_CROP_EN(0)
                                        ,Wpe_ISP_CROP_EN(0)
                                        ,Wpe_C24_EN(0)
                                        ,Wpe_CTL_EXTENSION(0)
                                        ,Wpe_TDR_SOF_RST_EN(0)
                                        ,Wpe_TDR_SIZE_DETECT(0)
                                        ,Wpe_CACHI_EN(0)
                                        ,Wpe_VECI_EN(0)
                                        ,Wpe_VEC2I_EN(0)
                                        ,Wpe_VEC3I_EN(0)
                                        ,Wpe_WPEO_EN(0)
                                        ,Wpe_MSKO_EN(0)
                                        ,Wpe_UV_INTER_EN(0)
                                        ,Wpe_CACHE_BURST_LEN(0)
                                        ,Wpe_CACHI_FMT_BIT(8)
                                        ,Wpe_CACHI_PLANE(0)
                                        ,Wpe_CACHI_FMT(0)
                                        ,Wpe_CACHI_FMT_SEQ(0)
                                        ,Wpe_WPE_DONE_EN(0)
                                        ,Wpe_TDR_ERR_INT_EN(0)
                                        ,Wpe_CACHE_MISS_INT_EN(0)
                                        ,Wpe_CACHI_DONE_EN(0)
                                        ,Wpe_VECI_DONE_EN(0)
                                        ,Wpe_WPEO_DONE_EN(0)
                                        ,Wpe_MSKO_DONE_EN(0)
                                        ,Wpe_DMA_ERR_INT_EN(0)
                                        ,Wpe_INT_WCLR_EN(0)
                                        ,Wpe_TILE_EDGE(0)
                                        ,Wpe_TILE_IRQ(0)
                                        ,Wpe_LAST_TILE(0)
                                        ,Wpe_TPIPE_CNT(0)
                                        ,Wpe_LOAD_SIZE(0)
                                        ,Wpe_SW_RST_ST(0)
                                        ,Wpe_VGEN_VERTICAL_FIRST(0)
                                        ,Wpe_VGEN_HMG_MODE(0)
                                        ,Wpe_VGEN_IN_WD(0)
                                        ,Wpe_VGEN_IN_HT(0)
                                        ,Wpe_VGEN_OUT_WD(0)
                                        ,Wpe_VGEN_OUT_HT(0)
                                        ,Wpe_VGEN_HORI_STEP(0)
                                        ,Wpe_VGEN_VERT_STEP(0)
                                        ,Wpe_VGEN_HORI_INT_OFST(0)
                                        ,Wpe_VGEN_HORI_SUB_OFST(0)
                                        ,Wpe_VGEN_VERT_INT_OFST(0)
                                        ,Wpe_VGEN_VERT_SUB_OFST(0)
                                        ,Wpe_VGEN_POST_SUB_H(0)
                                        ,Wpe_VGEN_POST_SUB_V(0)
                                        ,Wpe_VGEN_POST_COMP_EN(0)
                                        ,Wpe_VGEN_COMP_VEC_X(0)
                                        ,Wpe_VGEN_COMP_VEC_Y(0)
                                        ,Wpe_VGEN_VEC_WD(0)  
                                        ,Wpe_VGEN_VEC_HT(0)
                                        ,Wpe_VFC_SRAM_SIZE(0)
                                        ,Wpe_CFIFO_SRAM_SIZE(0)
                                        ,Wpe_CACHE_PREFETCH_MODE(0)
                                        ,Wpe_C24_TILE_EDGE(0)
                                        ,Wpe_MDP_CROP_STR_X(0)
                                        ,Wpe_MDP_CROP_END_X(0)
                                        ,Wpe_MDP_CROP_STR_Y(0)
                                        ,Wpe_MDP_CROP_END_Y(0)
                                        ,Wpe_ISPCROP_STR_X(0)
                                        ,Wpe_ISPCROP_END_X(0)
                                        ,Wpe_ISPCROP_STR_Y(0)
                                        ,Wpe_ISPCROP_END_Y(0) 
                                        ,Wpe_PSP_UNSIGN_TO_SIGN(0)
                                        ,Wpe_PSP_SIGN_EXT(0)
                                        ,Wpe_PSP_TBL_SEL_H(0)
                                        ,Wpe_PSP_TBL_SEL_V(0)
                                        ,Wpe_PSP2_UNSIGN_TO_SIGN(0)
                                        ,Wpe_PSP2_SIGN_EXT(0)
                                        ,Wpe_PSP2_TBL_SEL_H(0)
                                        ,Wpe_PSP2_TBL_SEL_V(0)
                                        ,Wpe_PSP_BORDER(0)
                                        ,Wpe_ADDR_GEN_BASE_ADDR(0)
                                        ,Wpe_ADDR_GEN_OFFSET_ADDR(0)
                                        ,Wpe_ADDR_GEN_STRIDE(0)
                                        ,Wpe_ADDR_GEN_FIFO_SIZE(0)
                                        ,Wpe_ADDR_GEN_MAX_BURST_LEN(0)
                                        ,Wpe_ADDR_GEN_ERR_EN(0)
                                        ,Wpe_ADDR_GEN_ERR_STAT(0) 
                                        ,Wpe_WPEO_SOFT_RST_STAT(0) 
                                        ,Wpe_MSKO_SOFT_RST_STAT(0)
                                        ,Wpe_VECI_SOFT_RST_STAT(0) 
                                        ,Wpe_VEC2I_SOFT_RST_STAT(0) 
                                        ,Wpe_VEC3I_SOFT_RST_STAT(0) 
                                        ,Wpe_TDRI_BASE_ADDR(0)
                                        ,Wpe_TDRI_OFST_ADDR(0)
                                        ,Wpe_TDRI_XSIZE(0)
                                        ,Wpe_WPEO_V_FLIP_EN(0)
                                        ,Wpe_MSKO_V_FLIP_EN(0)
                                        ,Wpe_VECI_V_FLIP_EN(0)
                                        ,Wpe_VEC2I_V_FLIP_EN(0)
                                        ,Wpe_VEC3I_V_FLIP_EN(0)
                                        ,Wpe_WPEO_BASE_ADDR(0)
                                        ,Wpe_WPEO_OFST_ADDR(0)
                                        ,Wpe_WPEO_XSIZE(0)
                                        ,Wpe_WPEO_YSIZE(0)
                                        ,Wpe_WPEO_STRIDE(0)
                                        ,Wpe_WPEO_CON(0)
                                        ,Wpe_WPEO_CON2(0)
                                        ,Wpe_WPEO_CON3(0)
                                        ,Wpe_WPEO_CROP(0)
                                        ,Wpe_MSKO_BASE_ADDR(0)
                                        ,Wpe_MSKO_OFST_ADDR(0)
                                        ,Wpe_MSKO_XSIZE(0)
                                        ,Wpe_MSKO_YSIZE(0)
                                        ,Wpe_MSKO_STRIDE(0)
                                        ,Wpe_MSKO_CON(0)
                                        ,Wpe_MSKO_CON2(0)
                                        ,Wpe_MSKO_CON3(0)
                                        ,Wpe_MSKO_CROP(0)
                                        ,Wpe_VECI_BASE_ADDR(0)
                                        ,Wpe_VECI_OFST_ADDR(0)
                                        ,Wpe_VECI_XSIZE(0)  
                                        ,Wpe_VECI_YSIZE(0)
                                        ,Wpe_VECI_STRIDE(0)
                                        ,Wpe_VECI_CON(0)
                                        ,Wpe_VECI_CON2(0)
                                        ,Wpe_VECI_CON3(0)
                                        ,Wpe_VEC2I_BASE_ADDR(0)
                                        ,Wpe_VEC2I_OFST_ADDR(0)
                                        ,Wpe_VEC2I_XSIZE(0)  
                                        ,Wpe_VEC2I_YSIZE(0)
                                        ,Wpe_VEC2I_STRIDE(0)
                                        ,Wpe_VEC2I_CON(0)
                                        ,Wpe_VEC2I_CON2(0)
                                        ,Wpe_VEC2I_CON3(0)
                                        ,Wpe_VEC3I_BASE_ADDR(0)
                                        ,Wpe_VEC3I_OFST_ADDR(0)
                                        ,Wpe_VEC3I_XSIZE(0)  
                                        ,Wpe_VEC3I_YSIZE(0)
                                        ,Wpe_VEC3I_STRIDE(0)
                                        ,Wpe_VEC3I_CON(0)
                                        ,Wpe_VEC3I_CON2(0)
                                        ,Wpe_VEC3I_CON3(0)
                                    {
                                    }
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_COMMON_H_

