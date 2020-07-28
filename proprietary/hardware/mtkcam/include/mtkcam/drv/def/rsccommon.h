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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_COMMON_H_
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
/*RSC_CTRL*/
enum RSCIMGIFORMAT {
    RSC_IMGI_Y_FMT = 0,
    RSC_IMGI_YC_FMT,
    RSC_IMGI_CY_FMT
};

enum RSCDMAPort {
    DMA_RSC_NONE = 0,
    DMA_RSC_IMGI_C,
    DMA_RSC_IMGI_P,
    DMA_RSC_APLI_C,
    DMA_RSC_APLI_P,
    DMA_RSC_MVI,
    DMA_RSC_MVO,
    DMA_RSC_BVO,
    DMA_RSC_NUM,
};

struct RSCBufInfo
{
    RSCDMAPort  dmaport;
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;    //  Vir Address of pool
    MUINTPTR    u4BufPA;    //  Phy Address of pool
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4Stride;  //  Buffer Stride
        RSCBufInfo()
        : dmaport(DMA_RSC_NONE)
        , memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        {
        }
};

struct RSCCandidate
{
        MUINT32 RSC_CAND_SEL;
        RSCCandidate(MUINT32 val)
        : RSC_CAND_SEL(val)
        {
        }
};

struct feedback {
    MUINT32 RSC_STA_0;
    feedback(): RSC_STA_0(0)
              {}
};

struct  RSCConfig
{
    MUINT32                         Rsc_Size_Height_p;
    MUINT32                         Rsc_Size_Width_p;
    MUINT32                         Rsc_Ctrl_Init_MV_Waddr;
    MUINT32                         Rsc_Ctrl_Init_MV_Flush_cnt;
    MUINT32                         Rsc_Ctrl_Trig_Num;
    RSCIMGIFORMAT                   Rsc_Ctrl_Imgi_c_Fmt;
    RSCIMGIFORMAT                   Rsc_Ctrl_Imgi_p_Fmt;
    MUINT32                         Rsc_Ctrl_Gmv_mode;
    bool                            Rsc_Ctrl_First_Me;
    bool                            Rsc_Ctrl_Skip_Pre_Mv;
    MUINT32                         Rsc_Ctrl_Start_Scan_Order;
    MUINT32                         Rsc_Size_Height;
    MUINT32                         Rsc_Size_Width;
    MUINT32                         Rsc_Horz_Sr;
    MUINT32                         Rsc_Vert_Sr;
    MUINT32                         Rsc_Br_Top;
    MUINT32                         Rsc_Br_Bottom;
    MUINT32                         Rsc_Br_Right;
    MUINT32                         Rsc_Br_Left;
    MUINT32                         Rsc_Vert_P_Mv_Offset;
    MUINT32                         Rsc_Horz_P_Mv_Offset;
    MUINT32                         Rsc_Vert_C_Mv_Offset;
    MUINT32                         Rsc_Horz_C_Mv_Offset;
    MUINT32                         Rsc_Vert_G_Mv_Offset;
    MUINT32                         Rsc_Horz_G_Mv_Offset;
    MUINT32                         Rsc_Prepare_Mv_Avg_Cand_Diff_Th;
    MUINT32                         Rsc_Prepare_Mv_Vert_Rand_Gain;
    MUINT32                         Rsc_Prepare_Mv_Horz_Rand_Gain;
    MUINT32                         Rsc_Cand_Num;
    RSCCandidate                    Rsc_Cand_Even_00;
    RSCCandidate                    Rsc_Cand_Even_01;
    RSCCandidate                    Rsc_Cand_Even_02;
    RSCCandidate                    Rsc_Cand_Even_03;
    RSCCandidate                    Rsc_Cand_Even_04;
    RSCCandidate                    Rsc_Cand_Even_05;
    RSCCandidate                    Rsc_Cand_Even_06;
    RSCCandidate                    Rsc_Cand_Even_07;
    RSCCandidate                    Rsc_Cand_Even_08;
    RSCCandidate                    Rsc_Cand_Even_09;
    RSCCandidate                    Rsc_Cand_Even_10;
    RSCCandidate                    Rsc_Cand_Even_11;
    RSCCandidate                    Rsc_Cand_Even_12;
    RSCCandidate                    Rsc_Cand_Even_13;
    RSCCandidate                    Rsc_Cand_Even_14;
    RSCCandidate                    Rsc_Cand_Even_15;
    RSCCandidate                    Rsc_Cand_Even_16;
    RSCCandidate                    Rsc_Cand_Even_17;
    RSCCandidate                    Rsc_Cand_Odd_00;
    RSCCandidate                    Rsc_Cand_Odd_01;
    RSCCandidate                    Rsc_Cand_Odd_02;
    RSCCandidate                    Rsc_Cand_Odd_03;
    RSCCandidate                    Rsc_Cand_Odd_04;
    RSCCandidate                    Rsc_Cand_Odd_05;
    RSCCandidate                    Rsc_Cand_Odd_06;
    RSCCandidate                    Rsc_Cand_Odd_07;
    RSCCandidate                    Rsc_Cand_Odd_08;
    RSCCandidate                    Rsc_Cand_Odd_09;
    RSCCandidate                    Rsc_Cand_Odd_10;
    RSCCandidate                    Rsc_Cand_Odd_11;
    RSCCandidate                    Rsc_Cand_Odd_12;
    RSCCandidate                    Rsc_Cand_Odd_13;
    RSCCandidate                    Rsc_Cand_Odd_14;
    RSCCandidate                    Rsc_Cand_Odd_15;
    RSCCandidate                    Rsc_Cand_Odd_16;
    RSCCandidate                    Rsc_Cand_Odd_17;
    MUINT32                         Rsc_Rand_Horz_Lut_0;
    MUINT32                         Rsc_Rand_Horz_Lut_1;
    MUINT32                         Rsc_Rand_Horz_Lut_2;
    MUINT32                         Rsc_Rand_Horz_Lut_3;
    MUINT32                         Rsc_Rand_Vert_Lut_0;
    MUINT32                         Rsc_Rand_Vert_Lut_1;
    MUINT32                         Rsc_Rand_Vert_Lut_2;
    MUINT32                         Rsc_Rand_Vert_Lut_3;
    MUINT32                         Rsc_Curr_Blk_Edge_Th;
    MUINT32                         Rsc_Curr_Blk_Var_Coring_Th;
    MUINT32                         Rsc_Curr_Blk_Nonzero_Blk_Can_Sel;
    MUINT32                         Rsc_Curr_Blk_Sort_Can_num;
    MUINT32                         Rsc_Curr_Blk_Out_Var_shift;
    MUINT32                         Rsc_Curr_Blk_No_Zero_Sort_Mv_En;
    MUINT32                         Rsc_Curr_Blk_Sort_Mv_En;
    MUINT32                         Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th;
    MUINT32                         Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode;
    MUINT32                         Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En;
    MUINT32                         Rsc_Sad_Ctrl_Sad_Gain;
    MUINT32                         Rsc_Sad_Ctrl_Sad_Coring_Th;
    MUINT32                         Rsc_Sad_Ctrl_Prev_Y_Offset;
    MUINT32                         Rsc_Sad_Edge_Gain_Step;
    MUINT32                         Rsc_Sad_Edge_Gain_TH_L;
    MUINT32                         Rsc_Sad_Edge_Gain;
    MUINT32                         Rsc_Sad_Crnr_Gain_Step;
    MUINT32                         Rsc_Sad_Crnr_Gain_TH_L;
    MUINT32                         Rsc_Sad_Crnr_Gain;
    MUINT32                         Rsc_Still_Strip_Zero_Pnlty_Dis;
    MUINT32                         Rsc_Still_Strip_Blk_Th_En;
    MUINT32                         Rsc_Still_Strip_Var_Step;
    MUINT32                         Rsc_Still_Strip_Var_Step_Th_L;
    MUINT32                         Rsc_Still_Strip_Sad_Step;
    MUINT32                         Rsc_Still_Strip_Sad_Step_Th_L;
    MUINT32                         Rsc_Mv_Pnlty_Edge_Th;
    MUINT32                         Rsc_Mv_Pnlty_Blk_Th_1;
    MUINT32                         Rsc_Mv_Pnlty_Blk_Th_0;
    MUINT32                         Rsc_Mv_Pnlty_Sel;
    MUINT32                         Rsc_Mv_Pnlty_Clip_Th;
    MUINT32                         Rsc_Mv_Pnlty_Clip_En;
    MUINT32                         Rsc_Zero_Pnlty_Blk_Th;
    MUINT32                         Rsc_Zero_Pnlty_Bndry;
    MUINT32                         Rsc_Zero_Pnlty;
    MUINT32                         Rsc_Rand_Pnlty_Bndry;
    MUINT32                         Rsc_Rand_Pnlty;
    MUINT32                         Rsc_Rand_Pnlty_Gain;
    MUINT32                         Rsc_Rand_Pnlty_Edge_Resp_Step;
    MUINT32                         Rsc_Rand_Pnlty_Edge_Resp_Th_L;
    MUINT32                         Rsc_Rand_Pnlty_Edge_Gain;
    MUINT32                         Rsc_Rand_Pnlty_Var_Resp_Step;
    MUINT32                         Rsc_Rand_Pnlty_Var_Resp_Th_L;
    MUINT32                         Rsc_Rand_Pnlty_Avg_Gain;
    MUINT32                         Rsc_Bndry_Tmpr_Pnlty;
    MUINT32                         Rsc_Tmpr_Pnlty_Blk_Th;
    MUINT32                         Rsc_Tmpr_Gain;
    MUINT32                         Rsc_Tmpr_Pnlty_Motion_1;
    MUINT32                         Rsc_Tmpr_Pnlty_Motion_0;
    MUINT32                         Rsc_Tmpr_Pnlty_Static_1;
    MUINT32                         Rsc_Tmpr_Pnlty_Static_0;
    RSCBufInfo                      Rsc_Imgi_c;
    RSCBufInfo                      Rsc_Imgi_p;
    RSCBufInfo                      Rsc_Apli_c;
    RSCBufInfo                      Rsc_Apli_p;
    RSCBufInfo                      Rsc_mvi;
    RSCBufInfo                      Rsc_mvo;
    RSCBufInfo                      Rsc_bvo;
    struct feedback                 feedback;
                                    RSCConfig()     //HW Default value
                                        : Rsc_Size_Height_p(511)
                                        , Rsc_Size_Width_p(288)
                                        , Rsc_Ctrl_Init_MV_Waddr(0)     //((w+1)/2)-1)/7
                                        , Rsc_Ctrl_Init_MV_Flush_cnt(0) //((w+1)/2)-1)%7
                                        , Rsc_Ctrl_Trig_Num(2)
                                        , Rsc_Ctrl_Imgi_c_Fmt(RSC_IMGI_Y_FMT)
                                        , Rsc_Ctrl_Imgi_p_Fmt(RSC_IMGI_Y_FMT)
                                        , Rsc_Ctrl_Gmv_mode(1)
                                        , Rsc_Ctrl_First_Me(1)
                                        , Rsc_Ctrl_Skip_Pre_Mv(0)
                                        , Rsc_Ctrl_Start_Scan_Order(1)
                                        , Rsc_Size_Height(511)
                                        , Rsc_Size_Width(288)
                                        , Rsc_Horz_Sr(32)
                                        , Rsc_Vert_Sr(8)
                                        , Rsc_Br_Top(1)
                                        , Rsc_Br_Bottom(1)
                                        , Rsc_Br_Right(2)
                                        , Rsc_Br_Left(2)
                                        , Rsc_Vert_P_Mv_Offset(0)
                                        , Rsc_Horz_P_Mv_Offset(0)
                                        , Rsc_Vert_C_Mv_Offset(0)
                                        , Rsc_Horz_C_Mv_Offset(0)
                                        , Rsc_Vert_G_Mv_Offset(0)
                                        , Rsc_Horz_G_Mv_Offset(0)
                                        , Rsc_Prepare_Mv_Avg_Cand_Diff_Th(8)
                                        , Rsc_Prepare_Mv_Vert_Rand_Gain(1)
                                        , Rsc_Prepare_Mv_Horz_Rand_Gain(1)
                                        , Rsc_Cand_Num(7)
                                        , Rsc_Cand_Even_00(0x19)
                                        , Rsc_Cand_Even_01(0x1A)
                                        , Rsc_Cand_Even_02(0x1B)
                                        , Rsc_Cand_Even_03(0x0B)
                                        , Rsc_Cand_Even_04(0x18)
                                        , Rsc_Cand_Even_05(0x07)
                                        , Rsc_Cand_Even_06(0x0e)
                                        , Rsc_Cand_Even_07(0x06)
                                        , Rsc_Cand_Even_08(0x11)
                                        , Rsc_Cand_Even_09(0x0d)
                                        , Rsc_Cand_Even_10(0x10)
                                        , Rsc_Cand_Even_11(0x12)
                                        , Rsc_Cand_Even_12(0x16)
                                        , Rsc_Cand_Even_13(0x15)
                                        , Rsc_Cand_Even_14(0x17)
                                        , Rsc_Cand_Even_15(0x0F)
                                        , Rsc_Cand_Even_16(0x13)
                                        , Rsc_Cand_Even_17(0x14)
                                        , Rsc_Cand_Odd_00(0x19)
                                        , Rsc_Cand_Odd_01(0x1A)
                                        , Rsc_Cand_Odd_02(0x1B)
                                        , Rsc_Cand_Odd_03(0x0D)
                                        , Rsc_Cand_Odd_04(0x14)
                                        , Rsc_Cand_Odd_05(0x07)
                                        , Rsc_Cand_Odd_06(0x0A)
                                        , Rsc_Cand_Odd_07(0x08)
                                        , Rsc_Cand_Odd_08(0x11)
                                        , Rsc_Cand_Odd_09(0x0B)
                                        , Rsc_Cand_Odd_10(0x10)
                                        , Rsc_Cand_Odd_11(0x12)
                                        , Rsc_Cand_Odd_12(0x16)
                                        , Rsc_Cand_Odd_13(0x15)
                                        , Rsc_Cand_Odd_14(0x17)
                                        , Rsc_Cand_Odd_15(0x0F)
                                        , Rsc_Cand_Odd_16(0x13)
                                        , Rsc_Cand_Odd_17(0x18)
                                        , Rsc_Rand_Horz_Lut_0(0x01)
                                        , Rsc_Rand_Horz_Lut_1(0x02)
                                        , Rsc_Rand_Horz_Lut_2(0x0A)
                                        , Rsc_Rand_Horz_Lut_3(0x14)
                                        , Rsc_Rand_Vert_Lut_0(0x01)
                                        , Rsc_Rand_Vert_Lut_1(0x02)
                                        , Rsc_Rand_Vert_Lut_2(0x0A)
                                        , Rsc_Rand_Vert_Lut_3(0x14)
                                        , Rsc_Curr_Blk_Edge_Th(8)
                                        , Rsc_Curr_Blk_Var_Coring_Th(1)
                                        , Rsc_Curr_Blk_Nonzero_Blk_Can_Sel(3)
                                        , Rsc_Curr_Blk_Sort_Can_num(11)
                                        , Rsc_Curr_Blk_Out_Var_shift(0)
                                        , Rsc_Curr_Blk_No_Zero_Sort_Mv_En(1)
                                        , Rsc_Curr_Blk_Sort_Mv_En(1)
                                        , Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th(0)
                                        , Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode(0)
                                        , Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En(1)
                                        , Rsc_Sad_Ctrl_Sad_Gain(4)
                                        , Rsc_Sad_Ctrl_Sad_Coring_Th(1)
                                        , Rsc_Sad_Ctrl_Prev_Y_Offset(0)
                                        , Rsc_Sad_Edge_Gain_Step(0x8)
                                        , Rsc_Sad_Edge_Gain_TH_L(0x62)
                                        , Rsc_Sad_Edge_Gain(0x1)
                                        , Rsc_Sad_Crnr_Gain_Step(0x8)
                                        , Rsc_Sad_Crnr_Gain_TH_L(0x62)
                                        , Rsc_Sad_Crnr_Gain(0x18)
                                        , Rsc_Still_Strip_Zero_Pnlty_Dis(1)
                                        , Rsc_Still_Strip_Blk_Th_En(1)
                                        , Rsc_Still_Strip_Var_Step(0x10)
                                        , Rsc_Still_Strip_Var_Step_Th_L(0x10)
                                        , Rsc_Still_Strip_Sad_Step(0x03)
                                        , Rsc_Still_Strip_Sad_Step_Th_L(0x03)
                                        , Rsc_Mv_Pnlty_Edge_Th(0x0)
                                        , Rsc_Mv_Pnlty_Blk_Th_1(0x7)
                                        , Rsc_Mv_Pnlty_Blk_Th_0(0x5)
                                        , Rsc_Mv_Pnlty_Sel(0x04)
                                        , Rsc_Mv_Pnlty_Clip_Th(0x26)
                                        , Rsc_Mv_Pnlty_Clip_En(0)
                                        , Rsc_Zero_Pnlty_Blk_Th(0x5)
                                        , Rsc_Zero_Pnlty_Bndry(0x95)
                                        , Rsc_Zero_Pnlty(0x5B)
                                        , Rsc_Rand_Pnlty_Bndry(0x10)
                                        , Rsc_Rand_Pnlty(0x10)
                                        , Rsc_Rand_Pnlty_Gain(0x4)
                                        , Rsc_Rand_Pnlty_Edge_Resp_Step(0x08)
                                        , Rsc_Rand_Pnlty_Edge_Resp_Th_L(0x62)
                                        , Rsc_Rand_Pnlty_Edge_Gain(0x8)
                                        , Rsc_Rand_Pnlty_Var_Resp_Step(0x3)
                                        , Rsc_Rand_Pnlty_Var_Resp_Th_L(0x50)
                                        , Rsc_Rand_Pnlty_Avg_Gain(0x3)
                                        , Rsc_Bndry_Tmpr_Pnlty(0x0A)
                                        , Rsc_Tmpr_Pnlty_Blk_Th(0x5)
                                        , Rsc_Tmpr_Gain(0x3)
                                        , Rsc_Tmpr_Pnlty_Motion_1(0x21)
                                        , Rsc_Tmpr_Pnlty_Motion_0(0x51)
                                        , Rsc_Tmpr_Pnlty_Static_1(0x21)
                                        , Rsc_Tmpr_Pnlty_Static_0(0x78)
                                    {
                                    }
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_COMMON_H_

