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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MFB_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MFB_COMMON_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

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

enum SUB_ENGINE_ID{
   eMFB = 0,
   eENGINE_MAX
};

/*MFB_CTRL*/
enum MFBFORMAT {
    FMT_MFB_YUV422 = 0,
    FMT_MFB_YUV420_2P = 1
};

enum FORMATBIT {
    FMT_4202P_8BIT = 0x8,
    FMT_4202P_10BIT_PAK = 0xA,
    FMT_4202P_10BIT_UNPAK = 0xAA
};
enum MFBDMAPort {
    DMA_MFB_NONE = 0,
    DMA_MFB_MFBI,		// base frame addr
    DMA_MFB_MFBI_B,		// base frame second plane addr
    DMA_MFB_MFB2I,		// ref frame addr
    DMA_MFB_MFB2I_B,	// ref frame second plane addr
    DMA_MFB_MFB3I,		// blend wight table input addr
    DMA_MFB_MFB4I,		// confidence map addr
    DMA_MFB_MFBO,		// output frame addr
    DMA_MFB_MFBO_B, 	// output frame second plane addr
    DMA_MFB_MFB2O,		// blend weight table output addr
    DMA_MFB_NUM
};

struct EGNBufInfo
{
    MFBDMAPort  dmaport;
    MINT32      memID;      //  memory ID
    MUINTPTR    u4BufVA;    //  Vir Address of pool
    MUINTPTR    u4BufPA;    //  Phy Address of pool
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4Stride;   //  Buffer Stride
    MUINT32     width;
	MUINT32     height;
        EGNBufInfo()
        : dmaport(DMA_MFB_NONE)
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

struct SRZSizeInfo
{
 public:
     MUINT32 in_w;
     MUINT32 in_h;
     MUINT32 out_w;
     MUINT32 out_h;
    MUINT32	crop_x;
    MUINT32	crop_y;
    MUINT32	crop_floatX; /* x float precise - 32 bit */
    MUINT32	crop_floatY; /* y float precise - 32 bit */
    unsigned long	crop_w;
    unsigned long	crop_h;

    SRZSizeInfo()
        : in_w(0x0)
        , in_h(0x0)
        , out_w(0x0)
        , out_h(0x0)
        , crop_x(0x0)
        , crop_y(0x0)
        , crop_floatX(0x0)
        , crop_floatY(0x0)
        , crop_w(0x0)
        , crop_h(0x0)
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
	{
	}
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

struct TuningRegs
{
    unsigned int MFB_CON;
	unsigned int MFB_LL_CON1;
	unsigned int MFB_LL_CON2;
	unsigned int MFB_LL_CON3;
	unsigned int MFB_LL_CON4;
	unsigned int MFB_EDGE;
	unsigned int MFB_LL_CON5;
	unsigned int MFB_LL_CON6;
	unsigned int MFB_LL_CON7;
	unsigned int MFB_LL_CON8;
	unsigned int MFB_LL_CON9;
	unsigned int MFB_LL_CON10;
	unsigned int MFB_MBD_CON0;
	unsigned int MFB_MBD_CON1;
	unsigned int MFB_MBD_CON2;
	unsigned int MFB_MBD_CON3;
	unsigned int MFB_MBD_CON4;
	unsigned int MFB_MBD_CON5;
	unsigned int MFB_MBD_CON6;
	unsigned int MFB_MBD_CON7;
	unsigned int MFB_MBD_CON8;
	unsigned int MFB_MBD_CON9;
	unsigned int MFB_MBD_CON10;
};

struct feedback {
    MUINT32 reg1;
    MUINT32 reg2;
    feedback(): reg1(0)
              , reg2(0)
              {}
};

struct  MFBConfig
{
    unsigned int	Mfb_bldmode;
    MFBFORMAT		Mfb_format;
    bool		Mfb_srzEn;
    struct TuningRegs	Mfb_tuningReg;
    unsigned int	*Mfb_tuningBuf;
    struct SRZSizeInfo	Mfb_srz5Info;
    IImageBuffer	*Mfb_inbuf_baseFrame;
    IImageBuffer	*Mfb_inbuf_refFrame;
    IImageBuffer	*Mfb_inbuf_blendWeight;
    IImageBuffer	*Mfb_inbuf_confidenceMap;
    IImageBuffer	*Mfb_inbuf_tdri;
    IImageBuffer	*Mfb_outbuf_frame;
    IImageBuffer	*Mfb_outbuf_blendWeight;
    feedback		feedback;
    IImageBuffer	*Mfb_inbuf_mv;
    FORMATBIT		Mfb_format_bit;
    bool		Mfb_omc_bicub;
    bool		Mfb_bypassOMC;
        MFBConfig()     //HW Default value
            : Mfb_bldmode(0)
            , Mfb_format(FMT_MFB_YUV420_2P)
            , Mfb_srzEn(0)
            , Mfb_tuningBuf(0x0)
            , Mfb_inbuf_baseFrame(0x0)
            , Mfb_inbuf_refFrame(0x0)
            , Mfb_inbuf_blendWeight(0x0)
            , Mfb_inbuf_confidenceMap(0x0)
            , Mfb_inbuf_tdri(0x0)
            , Mfb_outbuf_frame(0x0)
            , Mfb_outbuf_blendWeight(0x0)
            , Mfb_inbuf_mv(0x0)
            , Mfb_format_bit(FMT_4202P_10BIT_PAK)
            , Mfb_omc_bicub(1)
            , Mfb_bypassOMC(0)
            {
				memset(&Mfb_tuningReg, 0x0, sizeof(Mfb_tuningReg));
				memset(&Mfb_srz5Info, 0x0, sizeof(Mfb_srz5Info));
            }
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MFB_COMMON_H_

