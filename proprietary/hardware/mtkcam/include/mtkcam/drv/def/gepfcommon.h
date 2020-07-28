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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_GEPF_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_GEPF_COMMON_H_
//
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
/*GEPF_CTRL*/
enum GEPFDMAPort {
    DMA_GEPF_NONE = 0,
    DMA_GEPF_GEPF_FOCUS,
    DMA_GEPF_GEPF_YUV,
    DMA_GEPF_GEPF_Y,
    DMA_GEPF_GEPF_UV,
    DMA_GEPF_GEPF_Y_480,
    DMA_GEPF_GEPF_UV_480,
    DMA_GEPF_GEPF_DEPTH,
    DMA_GEPF_GEPF_DEPTH_WR,
    DMA_GEPF_GEPF_BLUR,
    DMA_GEPF_GEPF_BLUR_WR,
    DMA_GEPF_TEMP_Y,
    DMA_GEPF_TEMP_PRE_Y,
    DMA_GEPF_TEMP_DEPTH,
    DMA_GEPF_TEMP_PRE_DEPTH,
    DMA_GEPF_TEMP_DEPTH_WR,
    DMA_GEPF_TEMP_BLUR,
    DMA_GEPF_TEMP_PRE_BLUR,    
    DMA_GEPF_TEMP_BLUR_WR,
    DMA_GEPF_BYPASS_DEPTH,
    DMA_GEPF_BYPASS_DEPTH_WR,
    DMA_GEPF_BYPASS_BLUR,
    DMA_GEPF_BYPASS_BLUR_WR,
    DMA_GEPF_NUM,
};

struct GEPFBufInfo
{
    GEPFDMAPort dmaport;
    MINT32      memID;          //  memory ID
    MUINTPTR    u4BufVA;        //  Vir Address of pool
    MUINTPTR    u4BufPA;        //  Phy Address of pool
    MUINT32     u4BufSize;      //  Per buffer size
    MUINT32     u4Stride;       //  Buffer Stride
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
        GEPFBufInfo()
        : dmaport(DMA_GEPF_NONE)
        , memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        , u4ImgWidth(0)
        , u4ImgHeight(0)
        {
        }
};

struct  GEPFConfig
{
    MUINT32                         Gepf_Ctr_Iteration;
    MUINT32                         Gepf_Ctr_Max_Filter_Val_Weight;
    MUINT32                         Gepf_Ctr_Iteration_480;
    MUINT32                         Gepf_Ctr_Gepf_Mode;
    MUINT32                         Gepf_Ctr_Focus_Value;
    MUINT32                         Gepf_Ctr_Lamda;
    MUINT32                         Gepf_Ctr_Dof_M;
    MUINT32                         Gepf_Ctr_Lamda_480;
    MUINT32                         Temp_Ctr_Coeff_Value;
    bool                            Temp_Ctr_Bypass_En;

    GEPFBufInfo                     Gepf_Focus;
    GEPFBufInfo                     Gepf_YUV;
    GEPFBufInfo                     Gepf_Y;
    GEPFBufInfo                     Gepf_UV;
    GEPFBufInfo                     Gepf_Y_480;
    GEPFBufInfo                     Gepf_UV_480;
    GEPFBufInfo                     Gepf_Depth;
    GEPFBufInfo                     Gepf_Depth_WR;
    GEPFBufInfo                     Gepf_Blur;
    GEPFBufInfo                     Gepf_Blur_WR;
    GEPFBufInfo                     Temp_Y;
    GEPFBufInfo                     Temp_Pre_Y;
    GEPFBufInfo                     Temp_Depth;
    GEPFBufInfo                     Temp_Pre_Depth;
    GEPFBufInfo                     Temp_Depth_WR;
    GEPFBufInfo                     Temp_Blur;
    GEPFBufInfo                     Temp_Pre_Blur;
    GEPFBufInfo                     Temp_Blur_WR;
    GEPFBufInfo                     Bypass_Depth;
    GEPFBufInfo                     Bypass_Depth_WR;
    GEPFBufInfo                     Bypass_Blur;
    GEPFBufInfo                     Bypass_Blur_WR;
                                    GEPFConfig()     //HW Default value
                                        : Gepf_Ctr_Iteration(0)     //((w+1)/2)-1)/7
                                        , Gepf_Ctr_Max_Filter_Val_Weight(0) //((w+1)/2)-1)%7
                                        , Gepf_Ctr_Iteration_480(0)
                                        , Gepf_Ctr_Gepf_Mode(0)
                                        , Gepf_Ctr_Focus_Value(0)
                                        , Gepf_Ctr_Lamda(0)
                                        , Gepf_Ctr_Dof_M(0)
                                        , Gepf_Ctr_Lamda_480(0)
                                        , Temp_Ctr_Coeff_Value(0)
                                        , Temp_Ctr_Bypass_En(0)
                                    {
                                    }
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_GEPF_COMMON_H_

