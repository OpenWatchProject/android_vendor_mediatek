/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_lsc"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <lsc/ILscTbl.h>
#include <cutils/properties.h>

#include "isp_mgr.h"
#if 0
HHHHHH
#include <drv/isp_reg.h>
#endif

#define EXTEND_MODE 1

#define ROUND(a)  ((a) > 0.0f ? (int)(a + 0.5f):(int)(a - 0.5f))
//#define ALIGN(x, a) (((x)%(a))?((x)+((a)-((x)%(a)))):(x))
#define ALIGN(x, a) (((x)%(a))?((x)-((x)%(a))):(x))

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(LSC);
}

MVOID
ISP_MGR_LSC_T::
enableLsc(MUINT8 SubModuleIndex, MBOOL enable)
{
    if (m_fgOnOff[SubModuleIndex] != enable)
    {
        CAM_LOGD_IF(ENABLE_MY_LOG, "%s SubModuleIndex %d (before, after)=(%d, %d)",
            __FUNCTION__, SubModuleIndex, m_fgOnOff[SubModuleIndex], enable);
        m_fgOnOff[SubModuleIndex] = enable;
    }
}

MBOOL
ISP_MGR_LSC_T::
isEnable(MUINT8 SubModuleIndex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        CAM_LOGE("Error Index: %d", SubModuleIndex);
        return MFALSE;
    }
    return m_fgOnOff[SubModuleIndex];
}

ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
putRatio(MUINT8 SubModuleIndex, MUINT32 u4Rto)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d, Ratio(%d)", __FUNCTION__, SubModuleIndex, u4Rto);
    m_rIspRegInfo[SubModuleIndex][ERegInfo_RATIO_0].val = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;
    m_rIspRegInfo[SubModuleIndex][ERegInfo_RATIO_1].val = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    return  (*this);
}


template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_LSC_T const& rParam)
{
#if 1
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d", __FUNCTION__, SubModuleIndex);

    PUT_REG_INFO_MULTI(SubModuleIndex, CTL1, ctl1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CTL2, ctl2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CTL3, ctl3);
    PUT_REG_INFO_MULTI(SubModuleIndex, LBLOCK, lblock);
    PUT_REG_INFO_MULTI(SubModuleIndex, RATIO_0, ratio_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, RATIO_1, ratio_1);

#endif
    return  (*this);
}


ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
putAddr(MUINT8 SubModuleIndex, MUINT32 u4BaseAddr)
{
    CAM_LOGD_IF(ENABLE_MY_LOG, "[%s] SubModuleIndex %d, eSensorDev(%d), u4BaseAddr(0x%08x)", __FUNCTION__, SubModuleIndex, m_eSensorDev, u4BaseAddr);
    //m_rIspRegInfo[SubModuleIndex][ERegInfo_BASE_ADDR].val = u4BaseAddr;
    REG_INFO_VALUE_MULTI(SubModuleIndex, BASE_ADDR) = u4BaseAddr;
    return  (*this);
}


template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_LSC_T& rParam)
{
#if 1
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d", __FUNCTION__, SubModuleIndex);

    GET_REG_INFO_MULTI(SubModuleIndex, CTL1, ctl1);
    GET_REG_INFO_MULTI(SubModuleIndex, CTL2, ctl2);
    GET_REG_INFO_MULTI(SubModuleIndex, CTL3, ctl3);
    GET_REG_INFO_MULTI(SubModuleIndex, LBLOCK, lblock);
    GET_REG_INFO_MULTI(SubModuleIndex, RATIO_0, ratio_0);
    GET_REG_INFO_MULTI(SubModuleIndex, RATIO_1, ratio_1);
#endif
    return  (*this);
}

MUINT32
ISP_MGR_LSC_T::
getAddr(MUINT8 SubModuleIndex)
{
    MUINT32 u4Addr = REG_INFO_VALUE_MULTI(SubModuleIndex, BASE_ADDR);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d, u4BaseAddr(0x%08x)", __FUNCTION__, SubModuleIndex, u4Addr);
    return u4Addr;
}

MBOOL
ISP_MGR_LSC_T::
putBuf(MUINT8 SubModuleIndex, NSIspTuning::ILscBuf& rBuf)
{
    ISP_NVRAM_LSC_T rLscCfg;
    ::memset(&rLscCfg, 0, sizeof(ISP_NVRAM_LSC_T));

    ILscBuf::Config rCfg = rBuf.getConfig();
    MUINT32 u4Addr = rBuf.getPhyAddr();
    MUINT32 u4Rto = rBuf.getRatio();
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d, rto(%d)", __FUNCTION__, SubModuleIndex, u4Rto);
    rBuf.validate();
#ifndef EXTEND_MODE
    rLscCfg.ctl1.val                        = 0x30000000;
#else
    rLscCfg.ctl1.val                        = 0x30400000;
#endif
    rLscCfg.ctl2.bits.LSC_SDBLK_XNUM        = rCfg.i4BlkX;
    rLscCfg.ctl3.bits.LSC_SDBLK_YNUM        = rCfg.i4BlkY;
    rLscCfg.ctl2.bits.LSC_SDBLK_WIDTH       = rCfg.i4BlkW;
    rLscCfg.ctl3.bits.LSC_SDBLK_HEIGHT      = rCfg.i4BlkH;
    rLscCfg.lblock.bits.LSC_SDBLK_lWIDTH    = rCfg.i4BlkLastW;
    rLscCfg.lblock.bits.LSC_SDBLK_lHEIGHT   = rCfg.i4BlkLastH;
    rLscCfg.ratio_0.val                     = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;
    rLscCfg.ratio_1.val                     = rLscCfg.ratio_0.val;
    put(SubModuleIndex, rLscCfg);
    putAddr(SubModuleIndex, u4Addr);
    m_FD = rBuf.getFD();
    return MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
putBufAndRatio(MUINT8 SubModuleIndex, NSIspTuning::ILscBuf& rBuf, MUINT32 ratio)
{
    ISP_NVRAM_LSC_T rLscCfg;
    ::memset(&rLscCfg, 0, sizeof(ISP_NVRAM_LSC_T));
    ILscBuf::Config rCfg = rBuf.getConfig();
    MUINT32 u4Addr = rBuf.getPhyAddr();
    MUINT32 u4Rto = ratio;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d, rto(%d)", __FUNCTION__, SubModuleIndex, u4Rto);
    rBuf.validate();
#if (!EXTEND_MODE)
    rLscCfg.ctl1.val                        = 0x30000000;
#else
    rLscCfg.ctl1.val                        = 0x30400000;
#endif
    rLscCfg.ctl2.bits.LSC_SDBLK_XNUM        = rCfg.i4BlkX;
    rLscCfg.ctl3.bits.LSC_SDBLK_YNUM        = rCfg.i4BlkY;
    rLscCfg.ctl2.bits.LSC_SDBLK_WIDTH       = rCfg.i4BlkW;
    rLscCfg.ctl3.bits.LSC_SDBLK_HEIGHT      = rCfg.i4BlkH;
    rLscCfg.lblock.bits.LSC_SDBLK_lWIDTH    = rCfg.i4BlkLastW;
    rLscCfg.lblock.bits.LSC_SDBLK_lHEIGHT   = rCfg.i4BlkLastH;
    rLscCfg.ratio_0.val                     = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;
    rLscCfg.ratio_1.val                     = rLscCfg.ratio_0.val;
    put(SubModuleIndex, rLscCfg);
    putAddr(SubModuleIndex, u4Addr);
    m_FD = rBuf.getFD();
    return MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
configTSFS_Size(MUINT8 SubModuleIndex, MUINT32 ImgWidth, MUINT32 ImgHeight, MUINT32 tsfsH, MUINT32 tsfsV)
{
    //formula provided by TW Wang
    MUINT32 TSFS_W_HNUM=tsfsH;
    MUINT32 TSFS_W_VNUM=tsfsV;
    MUINT32 TSFS_W_HPIT=ImgWidth/tsfsH;
    MUINT32 TSFS_W_VPIT=ImgHeight/tsfsV;
    MUINT32 TSFS_W_HSIZE=((MUINT32)(TSFS_W_HPIT/2))*2;
    MUINT32 TSFS_W_VSIZE=((MUINT32)(TSFS_W_VPIT/2))*2;
    MUINT32 TSFS_PIXEL_CNT0=ROUND((float)16777216/(((float)TSFS_W_HSIZE)*((float)TSFS_W_VSIZE)/4.0));
    MUINT32 TSFS_PIXEL_CNT1=ROUND((float)8388608/(((float)TSFS_W_HSIZE)*((float)TSFS_W_VSIZE)/4.0));
    MUINT32 TSFS_PIXEL_CNT2=TSFS_PIXEL_CNT0;

    MUINT32 croppingWidth=ImgWidth-tsfsH*TSFS_W_HPIT;
    MUINT32 croppingHeight=ImgHeight-tsfsV*TSFS_W_VPIT;
    MUINT32 TSFS_W_HORG=ROUND((float)croppingWidth/2.0);
    MUINT32 TSFS_W_VORG=ROUND((float)croppingHeight/2.0);

    MUINT32 TSFSO_XSIZE1=TSFS_W_HNUM*12;
    MUINT32 TSFSO_YSIZE1=TSFS_W_VNUM;

    reinterpret_cast<REG_TSFS_R1_TSFS_ORG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ORG))->Bits.TSFS_W_VORG = TSFS_W_VORG;
    reinterpret_cast<REG_TSFS_R1_TSFS_ORG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ORG))->Bits.TSFS_W_HORG = TSFS_W_HORG;
    reinterpret_cast<REG_TSFS_R1_TSFS_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SIZE))->Bits.TSFS_W_VSIZE = TSFS_W_VSIZE;
    reinterpret_cast<REG_TSFS_R1_TSFS_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SIZE))->Bits.TSFS_W_HSIZE = TSFS_W_HSIZE;
    reinterpret_cast<REG_TSFS_R1_TSFS_PIT*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PIT))->Bits.TSFS_W_VPIT = TSFS_W_VPIT;
    reinterpret_cast<REG_TSFS_R1_TSFS_PIT*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PIT))->Bits.TSFS_W_HPIT = TSFS_W_HPIT;
    reinterpret_cast<REG_TSFS_R1_TSFS_NUM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, NUM))->Bits.TSFS_W_VNUM = TSFS_W_VNUM;
    reinterpret_cast<REG_TSFS_R1_TSFS_NUM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, NUM))->Bits.TSFS_W_HNUM = TSFS_W_HNUM;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC0))->Bits.TSFS_PIXEL_CNT0 = TSFS_PIXEL_CNT0;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC1))->Bits.TSFS_PIXEL_CNT1 = TSFS_PIXEL_CNT1;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC2))->Bits.TSFS_PIXEL_CNT2 = TSFS_PIXEL_CNT2;
    reinterpret_cast<REG_TSFSO_R1_TSFSO_XSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_XSIZE))->Bits.TSFSO_XSIZE = TSFSO_XSIZE1;
    reinterpret_cast<REG_TSFSO_R1_TSFSO_YSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_YSIZE))->Bits.TSFSO_YSIZE = TSFSO_YSIZE1;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] (w, h, tsfH, tsfV) = (%d, %d, %d, %d), TSFS: W_HNUM: %d, W_VNUM: %d, W_HPIT: %d, W_VPIT: %d, W_HSIZE: %d, W_VSIZE: %d, PIXEL_CNT0: %d, PIXEL_CNT1: %d, PIXEL_CNT2: %d, TSFSO_XSIZE: %d, TSFSO_YSIZE: %d",
        __FUNCTION__, ImgWidth, ImgHeight, tsfsH, tsfsV,
        TSFS_W_HNUM, TSFS_W_VNUM, TSFS_W_HPIT, TSFS_W_VPIT,
        TSFS_W_HSIZE, TSFS_W_VSIZE, TSFS_PIXEL_CNT0, TSFS_PIXEL_CNT1, TSFS_PIXEL_CNT2,
        TSFSO_XSIZE1, TSFSO_YSIZE1);

    return MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
configTSFS_Size_V1(MUINT8 SubModuleIndex, const TSFS_INFO_STRUCT &rTsfsConfig)
{
    reinterpret_cast<REG_TSFS_R1_TSFS_ORG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ORG))->Bits.TSFS_W_VORG = rTsfsConfig.TSFS_W_VORG;
    reinterpret_cast<REG_TSFS_R1_TSFS_ORG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ORG))->Bits.TSFS_W_HORG = rTsfsConfig.TSFS_W_HORG;
    reinterpret_cast<REG_TSFS_R1_TSFS_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SIZE))->Bits.TSFS_W_VSIZE = rTsfsConfig.TSFS_W_VSIZE;
    reinterpret_cast<REG_TSFS_R1_TSFS_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SIZE))->Bits.TSFS_W_HSIZE = rTsfsConfig.TSFS_W_HSIZE;
    reinterpret_cast<REG_TSFS_R1_TSFS_PIT*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PIT))->Bits.TSFS_W_VPIT = rTsfsConfig.TSFS_W_VPIT;
    reinterpret_cast<REG_TSFS_R1_TSFS_PIT*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PIT))->Bits.TSFS_W_HPIT = rTsfsConfig.TSFS_W_HPIT;
    reinterpret_cast<REG_TSFS_R1_TSFS_NUM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, NUM))->Bits.TSFS_W_VNUM = rTsfsConfig.TSFS_W_VNUM;
    reinterpret_cast<REG_TSFS_R1_TSFS_NUM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, NUM))->Bits.TSFS_W_HNUM = rTsfsConfig.TSFS_W_HNUM;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC0))->Bits.TSFS_PIXEL_CNT0 = rTsfsConfig.TSFS_PIXEL_CNT0;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC1))->Bits.TSFS_PIXEL_CNT1 = rTsfsConfig.TSFS_PIXEL_CNT1;
    reinterpret_cast<REG_TSFS_R1_TSFS_PC2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PC2))->Bits.TSFS_PIXEL_CNT2 = rTsfsConfig.TSFS_PIXEL_CNT2;

    MUINT32 TSFSO_XSIZE1=rTsfsConfig.TSFS_W_HNUM*12;
    MUINT32 TSFSO_YSIZE1=rTsfsConfig.TSFS_W_VNUM;

    reinterpret_cast<REG_TSFSO_R1_TSFSO_XSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_XSIZE))->Bits.TSFSO_XSIZE = TSFSO_XSIZE1;
    reinterpret_cast<REG_TSFSO_R1_TSFSO_YSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_YSIZE))->Bits.TSFSO_YSIZE = TSFSO_YSIZE1;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] (w, h, tsfH, tsfV) = (%d, %d, %d, %d), TSFS: W_HNUM: %d, W_VNUM: %d, W_HPIT: %d, W_VPIT: %d, W_HSIZE: %d, W_VSIZE: %d, PIXEL_CNT0: %d, PIXEL_CNT1: %d, PIXEL_CNT2: %d, TSFSO_XSIZE: %d, TSFSO_YSIZE: %d",
        __FUNCTION__, rTsfsConfig.IMG_WD, rTsfsConfig.IMG_HT, rTsfsConfig.TSFS_W_HNUM, rTsfsConfig.TSFS_W_VNUM,
        rTsfsConfig.TSFS_W_HNUM, rTsfsConfig.TSFS_W_VNUM, rTsfsConfig.TSFS_W_HPIT, rTsfsConfig.TSFS_W_VPIT,
        rTsfsConfig.TSFS_W_HSIZE, rTsfsConfig.TSFS_W_VSIZE, rTsfsConfig.TSFS_PIXEL_CNT0, rTsfsConfig.TSFS_PIXEL_CNT1, rTsfsConfig.TSFS_PIXEL_CNT2,
        TSFSO_XSIZE1, TSFSO_YSIZE1);

    return MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
#if 1
    MBOOL fgOnOff = m_fgOnOff[SubModuleIndex];

    MUINT32 u4XNum, u4YNum, u4Wd, u4Ht;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE, _CTL1;

    u4XNum = reinterpret_cast<REG_LSC_R1_LSC_CTL2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL2))->Bits.LSC_SDBLK_XNUM;
    u4YNum = reinterpret_cast<REG_LSC_R1_LSC_CTL3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL3))->Bits.LSC_SDBLK_YNUM;
    //u4Wd = reinterpret_cast<REG_LSC_R1_LSC_CTL2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL2))->Bits.LSC_SDBLK_WIDTH;
    //u4Ht = reinterpret_cast<REG_LSC_R1_LSC_CTL3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL3))->Bits.LSC_SDBLK_HEIGHT;

#ifndef EXTEND_MODE
    LSCI_XSIZE = (u4XNum+1)*4*4*4;
    LSCI_YSIZE = u4YNum+1;
    LSCI_STRIDE = LSCI_XSIZE;
    _CTL1 = 0x30000000;
#else
    LSCI_XSIZE = (u4XNum+1)*4*4*6;
    LSCI_YSIZE = u4YNum+1;
    LSCI_STRIDE = (LSCI_XSIZE);
    _CTL1 = 0x30400000;
#endif

    REG_INFO_VALUE_MULTI(SubModuleIndex, OFST_ADDR)  = 0;
    REG_INFO_VALUE_MULTI(SubModuleIndex, XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, CTL1)         = _CTL1;

    rTuning.updateEngineFD(eTuningMgrFunc_LSC_R1, i4SubsampleIdex, m_FD);
    rTuning.updateEngine(eTuningMgrFunc_LSC_R1, fgOnOff, i4SubsampleIdex);
    rTuning.updateEngine(eTuningMgrFunc_TSFS_R1, 1, i4SubsampleIdex);

    MUINT32 TSFSO_XSIZE1 = reinterpret_cast<REG_TSFSO_R1_TSFSO_XSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_XSIZE))->Bits.TSFSO_XSIZE;
    MUINT32 TSFSO_YSIZE1 = reinterpret_cast<REG_TSFSO_R1_TSFSO_YSIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TSFSO_YSIZE))->Bits.TSFSO_YSIZE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] SubModuleIndex %d LSCI_XSIZE(%d), LSCI_YSIZE (%d), TSFSO_XSIZE(%d), TSFSO_YSIZE(%d), OnOff(%d), addr(0x%08x)", __FUNCTION__, SubModuleIndex, LSCI_XSIZE, LSCI_YSIZE, TSFSO_XSIZE1, TSFSO_YSIZE1, fgOnOff, getAddr(SubModuleIndex));

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAMCTL_R1_CAMCTL_EN     , CAMCTL_LSC_R1_EN  , fgOnOff, i4SubsampleIdex);
    //TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAMCTL_R1_CAMCTL_EN3    , CAMCTL_MLSC_R1_EN , fgOnOff, i4SubsampleIdex);
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAMCTL_R1_CAMCTL_DMA2_EN , CAMCTL_LSCI_R1_EN , fgOnOff, i4SubsampleIdex);
    //TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAMCTL_R1_CAMCTL_DMA2_EN, CAMCTL_MLSCI_R1_EN, fgOnOff, i4SubsampleIdex);

    ISP_MGR_CAMCTL::getInstance(m_eSensorDev).setEnable_LSC_R1(fgOnOff);

    AAA_TRACE_DRV(DRV_LSC);
    // Register setting

    // Register setting

				char value[PROPERTY_VALUE_MAX] = {'\0'};
				property_get("vendor.debug.isp_mgr.dump", value, "0");
				MBOOL bEn = atoi(value);

#define ISP_MGR_LSC_PARAM_SIZE       (21)

				if (bEn == 1){
					char strTuningFile[512] = {'\0'};
					sprintf(strTuningFile, "data/vendor/camera_dump/isp_lsc_mgr.param");
					FILE* fidTuning = fopen(strTuningFile, "wb");
					if (fidTuning)
					{
					  fwrite(&(m_rIspRegInfo[SubModuleIndex][0]), sizeof(RegInfo_T)*ISP_MGR_LSC_PARAM_SIZE, 1, fidTuning);
					  fclose(fidTuning);
					}
				}
				if (bEn == 2){
					FILE* fidTuning = fopen("data/vendor/camera_dump/isp_lsc_mgr.param", "rb");
					if (fidTuning)
					{
						RegInfo_T	tempInfo[ISP_MGR_LSC_PARAM_SIZE];
						fread((&tempInfo), sizeof(tempInfo), 1, fidTuning);
						fclose(fidTuning);
						rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(tempInfo)),
							ISP_MGR_LSC_PARAM_SIZE, i4SubsampleIdex);
					}
				} else {
					rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
						m_u4RegInfoNum, i4SubsampleIdex);
				}

    AAA_TRACE_END_DRV;

    dumpRegInfoP1("LSC");
#endif
    return  MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
#if 1
    MBOOL fgOnOff = m_fgOnOff[SubModuleIndex];

    MUINT32 u4XNum, u4YNum, u4Wd, u4Ht;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE, _CTL1;

    u4XNum = reinterpret_cast<LSC_REG_D1A_LSC_CTL2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL2))->Bits.LSC_SDBLK_XNUM;
    u4YNum = reinterpret_cast<LSC_REG_D1A_LSC_CTL3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL3))->Bits.LSC_SDBLK_YNUM;
    u4Wd = reinterpret_cast<LSC_REG_D1A_LSC_CTL2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL2))->Bits.LSC_SDBLK_WIDTH;
    u4Ht = reinterpret_cast<LSC_REG_D1A_LSC_CTL3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTL3))->Bits.LSC_SDBLK_HEIGHT;
#ifndef EXTEND_MODE
    LSCI_XSIZE = (u4XNum+1)*4*4*4;
    LSCI_YSIZE = u4YNum+1;
    LSCI_STRIDE = LSCI_XSIZE;
    _CTL1 = 0x30000000;
#else
    LSCI_XSIZE = (u4XNum+1)*4*4*6;
    LSCI_YSIZE = u4YNum+1;
    LSCI_STRIDE = (LSCI_XSIZE);
    _CTL1 = 0x30400000;
#endif

    REG_INFO_VALUE_MULTI(SubModuleIndex, OFST_ADDR)  = 0;
    REG_INFO_VALUE_MULTI(SubModuleIndex, XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE_MULTI(SubModuleIndex, CTL1)         = _CTL1;

    CAM_LOGD_IF(m_bDebugEnable, "LSC p2=%d IMGCI_(XSIZE, YSIZE, STRIDE)=(%d, %d, %d), (u4XNum, u4YNum)=(%d, %d)", fgOnOff, LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE, u4XNum, u4YNum);

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LSC_D1_EN, fgOnOff);
    ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_IMGCI_D1_EN, fgOnOff);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_LSC2(fgOnOff);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(&m_rIspRegInfo[SubModuleIndex][0]), 12, pReg);

    //dumpRegInfo("LSC2");
#endif
    return  MTRUE;
}

};
