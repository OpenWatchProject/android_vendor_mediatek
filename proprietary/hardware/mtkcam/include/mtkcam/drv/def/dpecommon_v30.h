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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DPE_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DPE_COMMON_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
//#include <DPE_tuning.h>

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

#define DPE_MAX_FRAME_NUM 4

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

enum DPE_SUB_ENGINE_ID {
   eDPE = 0,
   eENGINE_MAX
};

enum DPEMODE {
    MODE_DVS_DVP_BOTH = 0,
    MODE_DVS_ONLY,
    MODE_DVP_ONLY
};

enum DPE_MAINEYE_SEL {
    LEFT = 0,
    RIGHT = 1
};

enum DPEFORMAT {

};

enum DPEDMAPort {
    DMA_DPE_NONE = 0,
    DMA_DPE_NUM
};

struct EGNBufInfo
{
    DPEDMAPort  dmaport;
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;    //  Vir Address of pool
    MUINTPTR    u4BufPA;    //  Phy Address of pool
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4Stride;   //  Buffer Stride
    MUINT32     width;
    MUINT32     height;
        EGNBufInfo()
        : dmaport(DMA_DPE_NONE)
        , memID(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , u4BufSize(0)
        , u4Stride(0)
        , width(0)
        , height(0)
        {
        }
};

struct TuningRegs
{
};

struct DPE_feedback {
    MUINT32 reg1;
    MUINT32 reg2;
    DPE_feedback()
        : reg1(0)
        , reg2(0)
        {}
};

typedef struct
{
    unsigned int                   DVS_ME_00;           //1B100300
    unsigned int                   DVS_ME_01;           //1B100304
    unsigned int                   DVS_ME_02;           //1B100308
    unsigned int                   DVS_ME_03;           //1B10030C
    unsigned int                   DVS_ME_04;           //1B100310
    unsigned int                   DVS_ME_05;           //1B100314
    unsigned int                   DVS_ME_06;           //1B100318
    unsigned int                   DVS_ME_07;           //1B10031C
    unsigned int                   DVS_ME_08;           //1B100320
    unsigned int                   DVS_ME_09;           //1B100324
    unsigned int                   DVS_ME_10;           //1B100328
    unsigned int                   DVS_ME_11;           //1B10032C
    unsigned int                   DVS_ME_12;           //1B100330
    unsigned int                   DVS_ME_13;           //1B100334
    unsigned int                   DVS_ME_14;           //1B100338
    unsigned int                   DVS_ME_15;           //1B10033C
    unsigned int                   DVS_ME_16;           //1B100340
    unsigned int                   DVS_ME_17;           //1B100344
    unsigned int                   DVS_ME_18;           //1B100348
    unsigned int                   DVS_ME_19;           //1B10034C
    unsigned int                   DVS_ME_20;           //1B100350
    unsigned int                   DVS_ME_21;           //1B100354
    unsigned int                   DVS_ME_22;           //1B100358
    unsigned int                   DVS_ME_23;           //1B10035C
    unsigned int                   DVS_ME_24;           //1B100360
}DVS_ME_CFG;

// ----------------- DPE_DVS_OCC  Grouping Definitions -------------------
typedef struct
{
    unsigned int                DVS_OCC_PQ_0;        //1B1003A0
    unsigned int                DVS_OCC_PQ_1;        //1B1003A4
    unsigned int                DVS_OCC_PQ_2;        //1B1003A8
    unsigned int                DVS_OCC_PQ_3;        //1B1003AC
    unsigned int                DVS_OCC_PQ_4;        //1B1003B0
    unsigned int                DVS_OCC_PQ_5;        //1B1003B4
}DVS_OCC_CFG;

// ----------------- DPE_DVP_CTRL  Grouping Definitions -------------------
typedef struct
{
    unsigned int                 DVP_CORE_00;         //1B100900
    unsigned int                 DVP_CORE_01;         //1B100904
    unsigned int                 DVP_CORE_02;         //1B100908
    unsigned int                 DVP_CORE_03;         //1B10090C
    unsigned int                 DVP_CORE_04;         //1B100910
    unsigned int                 DVP_CORE_05;         //1B100914
    unsigned int                 DVP_CORE_06;         //1B100918
    unsigned int                 DVP_CORE_07;         //1B10091C
    unsigned int                 DVP_CORE_08;         //1B100920
    unsigned int                 DVP_CORE_09;         //1B100924
    unsigned int                 DVP_CORE_10;         //1B100928
    unsigned int                 DVP_CORE_11;         //1B10092C
    unsigned int                 DVP_CORE_12;         //1B100930
    unsigned int                 DVP_CORE_13;         //1B100934
    unsigned int                 DVP_CORE_14;         //1B100938
    unsigned int                 DVP_CORE_15;         //1B10093C
}DVP_CORE_CFG;

struct DVS_SubModule_EN
{
    bool sbf_en;
    bool conf_en;
    bool occ_en;
    DVS_SubModule_EN()
    	: sbf_en(0)
    	, conf_en(1)
    	, occ_en(1)
    	{}
};

struct DVP_SubModule_EN
{
    bool asf_crm_en;
    bool asf_rm_en;
    bool asf_rd_en;
    bool asf_hf_en;
    bool wmf_hf_en;
    bool wmf_filt_en;
    unsigned int asf_hf_rounds;
    unsigned int asf_nb_rounds;
    unsigned int wmf_filt_rounds;
    bool asf_recursive_en;
    DVP_SubModule_EN()
    	: asf_crm_en(1)
    	, asf_rm_en(1)
    	, asf_rd_en(1)
    	, asf_hf_en(1)
    	, wmf_hf_en(0)
    	, wmf_filt_en(0)
    	, asf_hf_rounds(2)
    	, asf_nb_rounds(2)
    	, wmf_filt_rounds(1)
    	, asf_recursive_en(1)
    	{}
};

struct DVS_Iteration
{
    unsigned int	y_IterTimes;
    unsigned int	y_IterStartDirect_0;
    unsigned int	y_IterStartDirect_1;
    unsigned int	x_IterStartDirect_0;
    unsigned int	x_IterStartDirect_1;
    DVS_Iteration()
    	: y_IterTimes(1)
    	, y_IterStartDirect_0(0)
    	, y_IterStartDirect_1(1)
    	, x_IterStartDirect_0(0)
    	, x_IterStartDirect_1(1)
    	{}
};

struct DVS_Settings
{
    DPE_MAINEYE_SEL	mainEyeSel;
    DVS_ME_CFG		TuningBuf_ME;
    DVS_OCC_CFG		TuningBuf_OCC;
    struct DVS_SubModule_EN	SubModule_EN;
    struct DVS_Iteration	Iteration;
    bool is_pd_mode;
    unsigned int	pd_frame_num;
    unsigned int	frmWidth;
    unsigned int	frmHeight;
    unsigned int	L_engStart_x;
    unsigned int	R_engStart_x;
    unsigned int	engStart_y;
    unsigned int	engWidth;
    unsigned int	engHeight;
    unsigned int	occWidth;
    unsigned int	occStart_x;
    unsigned int  pitch;
    DVS_Settings()
    	: mainEyeSel(RIGHT)
    	, is_pd_mode(0)
    	, pd_frame_num(0)
    	, frmWidth(0)
    	, frmHeight(0)
    	, L_engStart_x(0)
    	, R_engStart_x(0)
    	, engStart_y(0)
    	, engWidth(0)
    	, engHeight(0)
    	, occWidth(0)
    	, occStart_x(0)
    	{
	        memset(&TuningBuf_ME, 0x0, sizeof(TuningBuf_ME));
	        memset(&TuningBuf_OCC, 0x0, sizeof(TuningBuf_OCC));
    	}
};

struct DVP_Settings
{
    DPE_MAINEYE_SEL	mainEyeSel;
    bool		Y_only;
    DVP_CORE_CFG	TuningBuf_CORE;
    struct DVP_SubModule_EN	SubModule_EN;
    bool		disp_guide_en;
    unsigned int	frmWidth;
    unsigned int	frmHeight;
    unsigned int	engStart_x;
    unsigned int	engStart_y;
    unsigned int	engWidth;
    unsigned int	engHeight;
    DVP_Settings()
    	: mainEyeSel(RIGHT)
    	, Y_only(0)
    	, disp_guide_en(0)
    	, frmWidth(0)
    	, frmHeight(0)
    	, engStart_x(0)
    	, engStart_y(0)
    	, engWidth(0)
    	, engHeight(0)
    	{
	        memset(&TuningBuf_CORE, 0x0, sizeof(TuningBuf_CORE));
    	}
};

/* Constrains
 * 1. Max input frame size = 5120 * 3840
 * 2. Frame width should be 16x, frame height should be 2x, which are 8bit src Y img width/height
 * 3. Allocate buffer size: OCC/ASF/WMF OutBuf size = (ALIGN128(Engine Width)) * (Engine Height)
 * 4. WMF doesn't support 16bit mode
 * 5. Always assign the same buffer into OutBuf_OCC and InBuf_OCC
 * 6. Always assign the same buffer into OutBuf_OCC_Ext and InBuf_OCC_Ext in 16Bit mode
*/

struct  DPEConfig
{
    DPEMODE		Dpe_engineSelect;
    unsigned int	Dpe_is16BitMode;
    struct DVS_Settings	Dpe_DVSSettings;
    struct DVP_Settings	Dpe_DVPSettings;
    MUINT32 	Dpe_InBuf_SrcImg_Y_L;
    MUINT32 	Dpe_InBuf_SrcImg_Y_R;
    MUINT32 	Dpe_InBuf_SrcImg_Y;
    MUINT32 	Dpe_InBuf_SrcImg_C;
    MUINT32 	Dpe_InBuf_ValidMap_L;
    MUINT32 	Dpe_InBuf_ValidMap_R;
    MUINT32 	Dpe_OutBuf_CONF;
    MUINT32 	Dpe_OutBuf_OCC;
    MUINT32 	Dpe_OutBuf_OCC_Ext;

    MUINT32 	Dpe_InBuf_OCC;
    MUINT32 	Dpe_InBuf_OCC_Ext;
    MUINT32 	Dpe_OutBuf_CRM;
    //MUINT32 	Dpe_OutBuf_ASF_RM;
    //MUINT32 	Dpe_OutBuf_ASF_RM_Ext;
    MUINT32 	Dpe_OutBuf_ASF_RD;
    MUINT32 	Dpe_OutBuf_ASF_RD_Ext;
    MUINT32 	Dpe_OutBuf_ASF_HF;
    MUINT32 	Dpe_OutBuf_ASF_HF_Ext;
    //MUINT32 	Dpe_OutBuf_WMF_HF;
    MUINT32 	Dpe_OutBuf_WMF_FILT;
    DPE_feedback	Dpe_feedback;
        DPEConfig()     //HW Default value
            : Dpe_engineSelect(MODE_DVS_DVP_BOTH)
            , Dpe_is16BitMode(0)
            , Dpe_InBuf_SrcImg_Y_L(0x0)
            , Dpe_InBuf_SrcImg_Y_R(0x0)
            , Dpe_InBuf_SrcImg_Y(0x0)
            , Dpe_InBuf_SrcImg_C(0x0)
            , Dpe_InBuf_ValidMap_L(0x0)
            , Dpe_InBuf_ValidMap_R(0x0)
            , Dpe_OutBuf_CONF(0x0)
            , Dpe_OutBuf_OCC(0x0)
            , Dpe_OutBuf_OCC_Ext(0x0)
            , Dpe_InBuf_OCC(0x0)
            , Dpe_InBuf_OCC_Ext(0x0)
            , Dpe_OutBuf_CRM(0x0)
            //, Dpe_OutBuf_ASF_RM(0x0)
            //, Dpe_OutBuf_ASF_RM_Ext(0x0)
            , Dpe_OutBuf_ASF_RD(0x0)
            , Dpe_OutBuf_ASF_RD_Ext(0x0)
            , Dpe_OutBuf_ASF_HF(0x0)
            , Dpe_OutBuf_ASF_HF_Ext(0x0)
            //, Dpe_OutBuf_WMF_HF(0x0)
            , Dpe_OutBuf_WMF_FILT(0x0)
            {}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DPE_COMMON_H_

