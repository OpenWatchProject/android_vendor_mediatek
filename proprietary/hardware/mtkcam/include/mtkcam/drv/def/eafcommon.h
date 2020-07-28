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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EAF_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EAF_COMMON_H_
//
#include <mtkcam/def/common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {


enum EAFDMAPort {
    DMA_EAF_NONE = 0,
	EAF_EAFI_MASK,
	EAF_EAFI_CUR_Y,
	EAF_EAFI_CUR_UV,
	EAF_EAFI_PRE_Y,
	EAF_EAFI_PRE_UV,
	EAF_EAFI_DEP,
	EAF_EAFI_LKH_WMAP,
	EAF_EAFI_LKH_EMAP,
	EAF_EAFI_PROB,
	EAF_EAFO_FOUT,
	EAF_EAFO_POUT,
	DMA_EAF_EAFO,
	DMA_EAF_EAFI,
	DMA_EAF_EAF2I,
	DMA_EAF_EAF3I,
	DMA_EAF_EAF4I,
	DMA_EAF_EAF5I,
	DMA_EAF_EAF6I,
};


struct EAFBufInfo
{
    EAFDMAPort  dmaport;
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;        //  Vir Address of pool
    MUINTPTR    u4BufPA;        //  Phy Address of pool
    MUINT32     u4BufSize;      //  Per buffer size
    MUINT32     u4Stride;       //  Buffer Stride
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
    MUINT32     u4Offset;  
    MUINT32     u4XOffset;  
    MUINT32     u4YOffset;  
	
        EAFBufInfo()
        : dmaport(DMA_EAF_NONE)
		, memID(0)
		, u4BufVA(0)
		, u4BufPA(0)
		, u4BufSize(0)
		, u4Stride(0)
		, u4ImgWidth(0)
		, u4ImgHeight(0)
		, u4Offset(0)
		, u4XOffset(0)
		, u4YOffset(0)
		{}
};


class EafTopCfg0
{
 public:
	 bool eaf_en;// bit0
	 bool jbfr_ref_sel;//bit1
	 bool eaf_hist_en;// bit2
	 bool eaf_lkh_ext_map_en;// bit3
	 bool eaf_jbfr_en;// bit4
	 bool eaf_boxf_en;// bit5
	 bool eaf_srz_en;// bit6
	 bool eaf_prob_en;// bit7
	 bool eaf_srz_pk8_en;// bit8
	 bool eaf_hist_pk8_en;// bit9
     bool eaf_div_en;// bit10
	 bool eaf_div_pk8_en;// bit11
	 bool eaf_lkhf_en;// bit12
	 bool eraf_jbfr_uvch_en;// bit19
	EafTopCfg0()
		: eaf_en(0)
		, jbfr_ref_sel(0)
		, eaf_hist_en(0)
		, eaf_lkh_ext_map_en(0)
		, eaf_jbfr_en(0)
		, eaf_boxf_en(0)
		, eaf_srz_en(0)
		, eaf_prob_en(0)
		, eaf_srz_pk8_en(0)
		, eaf_hist_pk8_en(0)
		, eaf_div_en(0)
		, eaf_div_pk8_en(0)
		, eaf_lkhf_en(0)
		, eraf_jbfr_uvch_en(0)
	{}
};

class EafTopCfg1
{
 public:
	 bool eafi_mask_en;// bit0
	 bool eafi_cur_y_en;// bit1
	 bool eafi_cur_uv_en;//bit2
	 bool eafi_pre_y_en;//bit3
	 bool eafi_pre_uv_en;//bit4
	 bool eafi_dep_en;//bit5
	 bool eafi_lkh_wmap_en;//bit6
	 bool eafi_lkh_emap_en;//bit7
	 bool eafi_prob_en;//bit8
	 bool eafo_fout_en;//bit9
	 bool eafo_pout_en;//bit10
	 bool eafi_en;//bit16
	 bool eaf2i_en;//bit17
	 bool eaf3i_en;//bit18
	 bool eaf4i_en;//bit19
	 bool eaf5i_en;//bit20
	 bool eaf6i_en;//bit21
	 bool eafo_en;//bit22

	EafTopCfg1()
		: eafi_mask_en(0)
		, eafi_cur_y_en(0)
		, eafi_cur_uv_en(0)
		, eafi_pre_y_en(0)
		, eafi_pre_uv_en(0)
		, eafi_dep_en(0)
		, eafi_lkh_wmap_en(0)
		, eafi_lkh_emap_en(0)
		, eafi_prob_en(0)
		, eafo_fout_en(0)
		, eafo_pout_en(0)
		, eafi_en(0)
		, eaf2i_en(0)
		, eaf3i_en(0)
		, eaf4i_en(0)
		, eaf5i_en(0)
		, eaf6i_en(0)
		, eafo_en(0)
	{}
};

class CtrlCfg
{
 public:
 	 MUINT32 ctrl;
 	 MUINT32 w;
 	 MUINT32 h;
	 CtrlCfg()
	 	:ctrl(0)
	 	,w(0)
	 	,h(0)
	 {}
};

class SizeCfg
{
 public:
 	 MUINT32 w;
 	 MUINT32 h;
	 SizeCfg()
	 	:w(0)
	 	,h(0)
	 {}
};

class SrzSizeCfg
{
 public:
	 MUINT32 in_w;
	 MUINT32 in_h;
	 MUINT32 out_w;
	 MUINT32 out_h;
	SrzSizeCfg()
		: in_w(0x0)
		, in_h(0x0)
		, out_w(0x0)
		, out_h(0x0)
	{}
};

class IspRect
{
public:
    MUINT32         x;
    MUINT32         y;
    MUINT32         floatX; /* x float precise - 32 bit */
    MUINT32         floatY; /* y float precise - 32 bit */
    unsigned long   w;
    unsigned long   h;


public:
    IspRect():
        x(0),y(0),floatX(0),floatY(0),w(0),h(0)
        {};

   IspRect(long _x, long _y, long _floatX, long _floatY, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; floatX = _floatX; floatY = _floatY; w = _w; h = _h;
        };

};


class SrzCfg
{
public:
	MUINT32 ctrl;
	SrzSizeCfg inout_size;
	IspRect crop;
	MUINT32 h_step;
	MUINT32 v_step;

	SrzCfg()
   : ctrl(0x0)
   , h_step(0x0)
   , v_step(0x0)
	{}
};

class JbfrCfg
{
public:
	MUINT32	cfg0;
	MUINT32	cfg1;
	MUINT32	cfg2;
	MUINT32	cfg3;
	MUINT32	cfg4;
	MUINT32	cfg5;
	MUINT32	cfg6;
	MUINT32	cfg7;
	MUINT32	cfg8;
	MUINT32	size;

	JbfrCfg()
   : cfg0(0x0)
	, cfg1(0x0)
	, cfg2(0x0)
	, cfg3(0x0)
	, cfg4(0x0)
	, cfg5(0x0)
	, cfg6(0x0)
	, cfg7(0x0)
	, cfg8(0x0)
	, size(0x0)
	{}
};

struct  EAFConfig
{
	MUINT32 mode;
	EafTopCfg0 MAIN_CFG0;
	EafTopCfg1 MAIN_CFG1;

	CtrlCfg EAF_HIST_CFG;	           
	SizeCfg	EAF_SRZ_CFG;     
	SizeCfg	EAF_BOXF_CFG;	          
	SizeCfg	EAF_DIV_CFG;	             
	CtrlCfg	EAF_LKHF_CFG;	
	
	CtrlCfg	EAF_MASK_LB_CTL;	           
	CtrlCfg	EAF_PRE_Y_LB_CTL;	           
	CtrlCfg	EAF_PRE_UV_LB_CTL;	           
	CtrlCfg	EAF_CUR_UV_LB_CTL;	           

	JbfrCfg	jbfrcfg;
    SrzCfg 	srz6Cfg;	
	
	EAFBufInfo	EAFI_MASK;
	EAFBufInfo	EAFI_CUR_Y;
	EAFBufInfo	EAFI_CUR_UV;
	EAFBufInfo	EAFI_PRE_Y;
	EAFBufInfo	EAFI_PRE_UV;
	EAFBufInfo	EAFI_DEP;
	EAFBufInfo	EAFI_LKH_WMAP;
	EAFBufInfo	EAFI_LKH_EMAP;
	EAFBufInfo	EAFI_PROB;
	EAFBufInfo	EAFO_FOUT;
	EAFBufInfo	EAFO_POUT;
	EAFBufInfo	EAFO;
	EAFBufInfo	EAFI;
	EAFBufInfo	EAF2I;
	EAFBufInfo	EAF3I;
	EAFBufInfo	EAF4I;
	EAFBufInfo	EAF5I;
	EAFBufInfo	EAF6I;
	
	MUINT32	EAF_TDRI_BASE_ADDR;	    
	MUINT32	EAF_TDRI_OFST_ADDR;	    
	MUINT32	EAF_TDRI_XSIZE;	        
   
	
            EAFConfig()     //HW Default value
            :mode(0)
            ,EAF_TDRI_BASE_ADDR(0x0)
			,EAF_TDRI_OFST_ADDR(0x0)         
			,EAF_TDRI_XSIZE(0x0)              
            {}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EAF_COMMON_H_

