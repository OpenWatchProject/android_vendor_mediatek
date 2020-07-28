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
#include <DPE_tuning.h>

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

struct DVS_SubModule_EN
{
    bool dvme_en;
    bool conf_en;
    bool cv_en;
    bool occ_en;
    DVS_SubModule_EN()
    	: dvme_en(1)
    	, conf_en(1)
    	, cv_en(1)
    	, occ_en(1)
    	{}
};

struct DVP_SubModule_EN
{
    bool asf_en;
    bool asf_rm_en;
    bool asf_rd_en;
    bool asf_hf_en;
    bool wmf_hf_en;
    bool wmf_filt_en;
    unsigned int asf_hf_rounds;
    unsigned int wmf_filt_rounds;
    bool asf_recursive_en;
    DVP_SubModule_EN()
    	: asf_en(1)
    	, asf_rm_en(1)
    	, asf_rd_en(1)
    	, asf_hf_en(1)
    	, wmf_hf_en(1)
    	, wmf_filt_en(1)
    	, asf_hf_rounds(4)
    	, wmf_filt_rounds(1)
    	, asf_recursive_en(1)
    	{}
};

struct DVS_Iteration
{
    unsigned int	y_IterTimes;
    unsigned int	y_IterStartDirect;
    unsigned int	x_IterTimes_Down;
    unsigned int	x_IterTimes_Up;
    unsigned int	x_IterStartDirect_Down;
    unsigned int	x_IterStartDirect_Up;
    	DVS_Iteration()
	: y_IterTimes(2)
	, y_IterStartDirect(0)
	, x_IterTimes_Down(2)
	, x_IterTimes_Up(2)
	, x_IterStartDirect_Down(0)
	, x_IterStartDirect_Up(0)
	{}
};

struct DVS_Settings
{
    DPE_MAINEYE_SEL	mainEyeSel;
    DVS_ME_CFG		TuningBuf_ME;
    DVS_OCC_CFG		TuningBuf_OCC;
    struct DVS_SubModule_EN	SubModule_EN;
    struct DVS_Iteration	Iteration;
    unsigned int	frmWidth;
    unsigned int	frmHeight;
    unsigned int	engStart_x;
    unsigned int	engStart_y;
    unsigned int	engWidth;
    unsigned int	engHeight;
    unsigned int	occWidth;
    unsigned int	occStart_x;
    bool		haveDVInput;
    	DVS_Settings()
	: mainEyeSel(RIGHT)
	, frmWidth(0)
	, frmHeight(0)
	, engStart_x(0)
	, engStart_y(0)
	, engWidth(0)
	, engHeight(0)
	, occWidth(0)
	, occStart_x(0)
	, haveDVInput(0)
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
    bool		DV_guide_en;
    unsigned int	frmWidth;
    unsigned int	frmHeight;
    unsigned int	engStart_x;
    unsigned int	engStart_y;
    unsigned int	engWidth;
    unsigned int	engHeight;
    	DVP_Settings()
	: mainEyeSel(RIGHT)
	, Y_only(0)
	, DV_guide_en(0)
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
 * 4. Allocate buffer size: DV OutBuf Size = (ALIGN16(Engine Width)) * (Engine Height) * 4
 * 5. DVS_haveDVInput: First run should set 0, later runs set 1 and give DV InBuf
 * 6. DVS: conf and dcv should turn on/off simultaneously
 * 7. WMF doesn't support 16bit mode
 * 8. Always assign the same buffer into OutBuf_OCC and InBuf_OCC
 * 9. Always assign the same buffer into OutBuf_OCC_Ext and InBuf_OCC_Ext in 16Bit mode
*/

struct  DPEConfig
{
    DPEMODE		Dpe_engineSelect;
    unsigned int	Dpe_is16BitMode;
    struct DVS_Settings	Dpe_DVSSettings;
    struct DVP_Settings	Dpe_DVPSettings;
    IImageBuffer	*Dpe_InBuf_SrcImg_Y_L;
    IImageBuffer	*Dpe_InBuf_SrcImg_Y_R;
    IImageBuffer	*Dpe_InBuf_SrcImg_C;
    IImageBuffer	*Dpe_InBuf_ValidMap_L;
    IImageBuffer	*Dpe_InBuf_ValidMap_R;
    IImageBuffer	*Dpe_InBuf_DV;  // DVS DV Input
    IImageBuffer	*Dpe_OutBuf_DV; // DVS DV Output for next run
    IImageBuffer	*Dpe_OutBuf_CONF;
    IImageBuffer	*Dpe_OutBuf_OCC;
    IImageBuffer	*Dpe_OutBuf_OCC_Ext;

    IImageBuffer	*Dpe_InBuf_OCC;
    IImageBuffer	*Dpe_InBuf_OCC_Ext;
    IImageBuffer	*Dpe_OutBuf_CRM;
    //IImageBuffer	*Dpe_OutBuf_ASF_RM;
    //IImageBuffer	*Dpe_OutBuf_ASF_RM_Ext;
    IImageBuffer	*Dpe_OutBuf_ASF_RD;
    IImageBuffer	*Dpe_OutBuf_ASF_RD_Ext;
    IImageBuffer	*Dpe_OutBuf_ASF_HF;
    IImageBuffer	*Dpe_OutBuf_ASF_HF_Ext;
    //IImageBuffer	*Dpe_OutBuf_WMF_HF;
    IImageBuffer	*Dpe_OutBuf_WMF_FILT;
    DPE_feedback	Dpe_feedback;
        DPEConfig()     //HW Default value
            : Dpe_engineSelect(MODE_DVS_DVP_BOTH)
            , Dpe_is16BitMode(0)
            , Dpe_InBuf_SrcImg_Y_L(0x0)
            , Dpe_InBuf_SrcImg_Y_R(0x0)
            , Dpe_InBuf_SrcImg_C(0x0)
            , Dpe_InBuf_ValidMap_L(0x0)
            , Dpe_InBuf_ValidMap_R(0x0)
            , Dpe_InBuf_DV(0x0)
            , Dpe_OutBuf_DV(0x0)
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

