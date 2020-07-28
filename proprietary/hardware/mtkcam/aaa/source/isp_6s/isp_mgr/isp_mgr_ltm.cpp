/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_ltm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LTM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_LTM_T&
ISP_MGR_LTM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(LTM);
}
template <>
ISP_MGR_LTM_T&
ISP_MGR_LTM_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_LTM_CURVE_T const& rParam)
{
    for (MINT32 i = 0; i < LTM_CURVE_SIZE; i++) {
        m_rIspRegInfo_CURVE[SubModuleIndex][i].val = rParam.set[i];
    }
    return  (*this);
}

template <>
ISP_MGR_LTM_T&
ISP_MGR_LTM_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_LTM_CURVE_T & rParam)
{
    for (MINT32 i = 0; i < LTM_CURVE_SIZE; i++) {
        rParam.set[i] = m_rIspRegInfo_CURVE[SubModuleIndex][i].val;
    }

    return  (*this);
}
#if 0
template <>
ISP_MGR_LTM_T&
ISP_MGR_LTM_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_LTM_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CTRL,                      ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLK_NUM,                   blk_num);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLK_SZ,                    blk_sz);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLK_DIVX,                  blk_divx);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLK_DIVY,                  blk_divy);
    PUT_REG_INFO_MULTI(SubModuleIndex, MAX_DIV,                   max_div);
    PUT_REG_INFO_MULTI(SubModuleIndex, CLIP,                      clip);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_NUM,        tile_num);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_CNTX,       tile_cntx);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_CNTY,       tile_cnty);
    PUT_REG_INFO_MULTI(SubModuleIndex, CFG,             cfg);
    PUT_REG_INFO_MULTI(SubModuleIndex, RESET,           reset);
    PUT_REG_INFO_MULTI(SubModuleIndex, INTEN,           inten);
    PUT_REG_INFO_MULTI(SubModuleIndex, INTSTA,          intsta);
    PUT_REG_INFO_MULTI(SubModuleIndex, STATUS,          status);
    PUT_REG_INFO_MULTI(SubModuleIndex, INPUT_COUNT,     input_count);
    PUT_REG_INFO_MULTI(SubModuleIndex, OUTPUT_COUNT,    output_count);
    PUT_REG_INFO_MULTI(SubModuleIndex, CHKSUM,          chksum);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_SIZE,       tile_size);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_EDGE,       tile_edge);
    PUT_REG_INFO_MULTI(SubModuleIndex, TILE_CROP,       tile_crop);
    PUT_REG_INFO_MULTI(SubModuleIndex, DUMMY_REG,       dummy_reg);
    PUT_REG_INFO_MULTI(SubModuleIndex, SRAM_CFG,        sram_cfg);
    PUT_REG_INFO_MULTI(SubModuleIndex, SRAM_STATUS,     sram_status);
    PUT_REG_INFO_MULTI(SubModuleIndex, ATPG,            atpg);
    PUT_REG_INFO_MULTI(SubModuleIndex, SHADOW_CTRL,     shadow_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CLIP_TH,                   clip_th);
    PUT_REG_INFO_MULTI(SubModuleIndex, GAIN_MAP,                  gain_map);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP0,               cvnode_grp0);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP1,               cvnode_grp1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP2,               cvnode_grp2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP3,               cvnode_grp3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP4,               cvnode_grp4);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP5,               cvnode_grp5);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP6,               cvnode_grp6);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP7,               cvnode_grp7);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP8,               cvnode_grp8);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP9,               cvnode_grp9);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP10,              cvnode_grp10);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP11,              cvnode_grp11);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP12,              cvnode_grp12);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP13,              cvnode_grp13);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP14,              cvnode_grp14);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP15,              cvnode_grp15);
    PUT_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP16,              cvnode_grp16);
    PUT_REG_INFO_MULTI(SubModuleIndex, OUT_STR,                   out_str);
    PUT_REG_INFO_MULTI(SubModuleIndex, SRAM_PINGPONG,   sram_pingpong);
    return (*this);
}

template <>
ISP_MGR_LTM_T&
ISP_MGR_LTM_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_LTM_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CTRL,                      ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, BLK_NUM,                   blk_num);
    GET_REG_INFO_MULTI(SubModuleIndex, BLK_SZ,                    blk_sz);
    GET_REG_INFO_MULTI(SubModuleIndex, BLK_DIVX,                  blk_divx);
    GET_REG_INFO_MULTI(SubModuleIndex, BLK_DIVY,                  blk_divy);
    GET_REG_INFO_MULTI(SubModuleIndex, MAX_DIV,                   max_div);
    GET_REG_INFO_MULTI(SubModuleIndex, CLIP,                      clip);
    GET_REG_INFO_MULTI(SubModuleIndex, CLIP_TH,                   clip_th);
    GET_REG_INFO_MULTI(SubModuleIndex, GAIN_MAP,                  gain_map);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP0,               cvnode_grp0);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP1,               cvnode_grp1);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP2,               cvnode_grp2);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP3,               cvnode_grp3);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP4,               cvnode_grp4);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP5,               cvnode_grp5);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP6,               cvnode_grp6);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP7,               cvnode_grp7);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP8,               cvnode_grp8);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP9,               cvnode_grp9);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP10,              cvnode_grp10);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP11,              cvnode_grp11);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP12,              cvnode_grp12);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP13,              cvnode_grp13);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP14,              cvnode_grp14);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP15,              cvnode_grp15);
    GET_REG_INFO_MULTI(SubModuleIndex, CVNODE_GRP16,              cvnode_grp16);
    GET_REG_INFO_MULTI(SubModuleIndex, OUT_STR,                   out_str);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_NUM,        tile_num);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_CNTX,       tile_cntx);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_CNTY,       tile_cnty);
    GET_REG_INFO_MULTI(SubModuleIndex, CFG,             cfg);
    GET_REG_INFO_MULTI(SubModuleIndex, RESET,           reset);
    GET_REG_INFO_MULTI(SubModuleIndex, INTEN,           inten);
    GET_REG_INFO_MULTI(SubModuleIndex, INTSTA,          intsta);
    GET_REG_INFO_MULTI(SubModuleIndex, STATUS,          status);
    GET_REG_INFO_MULTI(SubModuleIndex, INPUT_COUNT,     input_count);
    GET_REG_INFO_MULTI(SubModuleIndex, OUTPUT_COUNT,    output_count);
    GET_REG_INFO_MULTI(SubModuleIndex, CHKSUM,          chksum);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_SIZE,       tile_size);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_EDGE,       tile_edge);
    GET_REG_INFO_MULTI(SubModuleIndex, TILE_CROP,       tile_crop);
    GET_REG_INFO_MULTI(SubModuleIndex, DUMMY_REG,       dummy_reg);
    GET_REG_INFO_MULTI(SubModuleIndex, SRAM_CFG,        sram_cfg);
    GET_REG_INFO_MULTI(SubModuleIndex, SRAM_STATUS,     sram_status);
    GET_REG_INFO_MULTI(SubModuleIndex, ATPG,            atpg);
    GET_REG_INFO_MULTI(SubModuleIndex, SHADOW_CTRL,     shadow_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, SRAM_PINGPONG,   sram_pingpong);
    return (*this);
}
#endif

static MUINT32 coldLaunchSerialNum=0;

MBOOL
ISP_MGR_LTM_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control for CCU
    switch (SubModuleIndex)
    {
        case ELTM_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_LTM_R1(bEnable);
            // Top Control
            rTuning.enableEngine(eTuningMgrFunc_LTM_R1, ( bEnable &&(rRawIspCamInfo.u4Id == 1) ), i4SubsampleIdex);
            // Update
            rTuning.tuningEngine(eTuningMgrFunc_LTM_R1,( bEnable &&(rRawIspCamInfo.u4Id == 1) ), i4SubsampleIdex);
            rTuning.tuningEngine(eTuningMgrFunc_LTMTC_R1_CURVE,( bEnable &&(rRawIspCamInfo.u4Id == 1) ), i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if( bEnable &&(rRawIspCamInfo.u4Id == 1) ){

        MUINT32 u4LTM_CLIP_TH=0;

        ISP_NVRAM_LTM_CURVE_T defaultCurve = {0};

        if(rRawIspCamInfo.rCCU_Result.LTM.LTM_Valid == CCU_UNVALID){
        ALOGD("LTM ccu unvalid");
            MUINT32 defaultValue[33] = { 0,     412,   1016,  1524,  2044,  2560,  3073,  3589,  4037,  4597,
                                         5093,  5629,  6137,  6649,  7165,  7681,  8194,  8702,  9214,  9726,
                                         10238, 10746, 11262, 11774, 12282, 12798, 13310, 13827, 14335, 14847,
                                         15359, 15871, 16383};
            u4LTM_CLIP_TH = 4095;

            for (int numy = 0; numy < LTM_BLOCK_YNUM; numy++)
                for (int numx = 0; numx < LTM_BLOCK_XNUM; numx++){

                    int Bin_Pos= (numy * 12 + numx);

                    for (int k = 0; k < (33 - 1); k++){
                        if((k%2) == 0){
                            defaultCurve.lut[Bin_Pos * 16 + (k/2)].bits.LTMTC_TONECURVE_LUT_L = defaultValue[k];
                        }
                        else{
                            defaultCurve.lut[Bin_Pos * 16 + (k/2)].bits.LTMTC_TONECURVE_LUT_H = defaultValue[k];
                        }
                    }
                    //special case (colltct k=32(in each bin) on the final part of 1-dim array)
                    if((Bin_Pos % 2) == 0)
                        defaultCurve.lut[(LTM_BLOCK_YNUM * LTM_BLOCK_XNUM ) * 16 + (Bin_Pos / 2)].bits.LTMTC_TONECURVE_LUT_L = defaultValue[(33 - 1)];
                    else
                        defaultCurve.lut[(LTM_BLOCK_YNUM * LTM_BLOCK_XNUM ) * 16 + (Bin_Pos / 2)].bits.LTMTC_TONECURVE_LUT_H = defaultValue[(33 - 1)];
                }

            put(SubModuleIndex, defaultCurve);
        }
        else{
            u4LTM_CLIP_TH = rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH;
            put(SubModuleIndex, rRawIspCamInfo.rCCU_Result.LTM.LTM_Curve);
        }

        MUINT32 ct_alpha_base_shift_bit;
        MUINT32 ct_alpha_base;
        MUINT32 tmp = (u4LTM_CLIP_TH + 512) >> 10;
        if (tmp > 0)
        {
            ct_alpha_base_shift_bit = 5;
            while (tmp != 0)
            {
                tmp = (tmp >> 1);
                ct_alpha_base_shift_bit++;
            }
        }
        else
        {
            ct_alpha_base_shift_bit = 0;
        }
        if (u4LTM_CLIP_TH != 0){
            ct_alpha_base = ((1 << (10 + ct_alpha_base_shift_bit)) + (u4LTM_CLIP_TH >> 1)) / u4LTM_CLIP_TH;
        }
        else{
            ct_alpha_base = 1<<(10 + ct_alpha_base_shift_bit);
        }
        ct_alpha_base = (ct_alpha_base<0)? 0 :((ct_alpha_base>1023)? 1023 : ct_alpha_base);
        reinterpret_cast<REG_LTM_R1_LTM_MAX_DIV*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, MAX_DIV))->Bits.LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT = ct_alpha_base_shift_bit;
        reinterpret_cast<REG_LTM_R1_LTM_MAX_DIV*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, MAX_DIV))->Bits.LTM_CLIP_TH_ALPHA_BASE = ct_alpha_base;

#if 0
        CAM_LOGD("isp_mgr_ltm ltm curve print, LTM_Valid %d, launch num %d", rRawIspCamInfo.rCCU_Result.LTM.LTM_Valid, coldLaunchSerialNum);
        MUINT32 jj = 0;
        for(jj=0;jj<LTM_CURVE_SIZE;jj+=10)
        {
            CAM_LOGD("%d %d %d %d %d, %d %d %d %d %d",
                m_rIspRegInfo_CURVE[SubModuleIndex][jj  ].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+1].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+2].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+3].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+4].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+5].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+6].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+7].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+8].val,
                m_rIspRegInfo_CURVE[SubModuleIndex][jj+9].val);
        }
#endif

        AAA_TRACE_DRV(DRV_LTM);
        //Special case: only update 1 Reg, ERegInfo_MAX_DIV
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][ERegInfo_MAX_DIV])),
                1, i4SubsampleIdex);

        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo_CURVE[SubModuleIndex][0])),
                LTM_CURVE_SIZE, i4SubsampleIdex);
        AAA_TRACE_END_DRV;

        dumpRegInfoP1("LTM", SubModuleIndex);

#if 0
        //Check dump
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.dump.ltmcurve", value, "0");
        MBOOL m_dumpEnable = atoi(value);
        if(m_dumpEnable)
        {
            char filename[512];
            sprintf(filename, "/data/vendor/camera_dump/Debug_ltm_curve_%d", coldLaunchSerialNum++);
            ISP_NVRAM_LTM_CURVE_T ltm_curve = (!rRawIspCamInfo.rCCU_Result.LTM.LTM_Valid)?defaultCurve:rRawIspCamInfo.rCCU_Result.LTM.LTM_Curve;
            android::sp<IFileCache> fidLtmCurve;
            fidLtmCurve = IFileCache::open(filename);
            if (fidLtmCurve->write(&ltm_curve, sizeof(ISP_NVRAM_LTM_CURVE_T)) != sizeof(ISP_NVRAM_LTM_CURVE_T))
                CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);
        }
#endif
    }

#if 0
    CAM_LOGD("isp_mgr_ltm LTM_Valid %d, frame id %d, curve val %d, CLIP_TH %d",
        rRawIspCamInfo.rCCU_Result.LTM.LTM_Valid, rRawIspCamInfo.u4Id,
        rRawIspCamInfo.rCCU_Result.LTM.LTM_Curve.lut[150].val,
        rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH);
#endif

    return  MTRUE;
}

MBOOL
ISP_MGR_LTM_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case ELTM_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LTM_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){

        reinterpret_cast<LTM_REG_D1A_LTM_BLK_SZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_SZ))->Bits.LTM_BLK_WIDTH =
            ((rRawIspCamInfo.rCropRzInfo.sRRZout.w / LTM_BLOCK_XNUM) + ((rRawIspCamInfo.rCropRzInfo.sRRZout.w % LTM_BLOCK_XNUM) ? 1 : 0));
        reinterpret_cast<LTM_REG_D1A_LTM_BLK_SZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_SZ))->Bits.LTM_BLK_HEIGHT =
            ((rRawIspCamInfo.rCropRzInfo.sRRZout.h / LTM_BLOCK_YNUM) + ((rRawIspCamInfo.rCropRzInfo.sRRZout.h % LTM_BLOCK_YNUM) ? 1 : 0));

        MUINT32 tmpW = (rRawIspCamInfo.rCropRzInfo.sRRZout.w + 256) >> 9;
        MUINT32 x_alpha_base_shift_bit = 4;
        while (tmpW != 0)
        {
              tmpW = (tmpW >> 1);
              x_alpha_base_shift_bit++;
        }
        reinterpret_cast<LTM_REG_D1A_LTM_BLK_DIVX*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_DIVX))->Bits.LTM_X_ALPHA_BASE_SHIFT_BIT = x_alpha_base_shift_bit;
        reinterpret_cast<LTM_REG_D1A_LTM_BLK_DIVX*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_DIVX))->Bits.LTM_X_ALPHA_BASE =
            ((LTM_BLOCK_XNUM << (x_alpha_base_shift_bit + 9)) + (rRawIspCamInfo.rCropRzInfo.sRRZout.w >> 1)) / rRawIspCamInfo.rCropRzInfo.sRRZout.w;

        MUINT32 tmpH = (rRawIspCamInfo.rCropRzInfo.sRRZout.h + 256) >> 9;
        MUINT32 y_alpha_base_shift_bit = 4;
        while (tmpH != 0)
        {
              tmpH = (tmpH >> 1);
              y_alpha_base_shift_bit++;
        }
        reinterpret_cast<LTM_REG_D1A_LTM_BLK_DIVY*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_DIVY))->Bits.LTM_Y_ALPHA_BASE_SHIFT_BIT = y_alpha_base_shift_bit;
        reinterpret_cast<LTM_REG_D1A_LTM_BLK_DIVY*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BLK_DIVY))->Bits.LTM_Y_ALPHA_BASE =
            ((LTM_BLOCK_YNUM << (y_alpha_base_shift_bit + 9)) + (rRawIspCamInfo.rCropRzInfo.sRRZout.h >> 1)) / rRawIspCamInfo.rCropRzInfo.sRRZout.h;

        MUINT32 tmp = (rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH + 512) >> 10;
        MUINT32 ct_alpha_base_shift_bit;
        MUINT32 ct_alpha_base;
        if (tmp > 0)
        {
            ct_alpha_base_shift_bit = 5;
            while (tmp != 0)
            {
                tmp = (tmp >> 1);
                ct_alpha_base_shift_bit++;
            }
        }
        else
        {
            ct_alpha_base_shift_bit = 0;
        }
        if (rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH != 0){
            ct_alpha_base = ((1 << (10 + ct_alpha_base_shift_bit)) + (rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH >> 1)) / rRawIspCamInfo.rCCU_Result.LTM.CLIP_TH;
        }
        else{
            ct_alpha_base = 1<<(10 + ct_alpha_base_shift_bit);
        }
        ct_alpha_base = (ct_alpha_base<0)? 0 :((ct_alpha_base>1023)? 1023 : ct_alpha_base);
        reinterpret_cast<LTM_REG_D1A_LTM_MAX_DIV*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, MAX_DIV))->Bits.LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT = ct_alpha_base_shift_bit;
        reinterpret_cast<LTM_REG_D1A_LTM_MAX_DIV*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, MAX_DIV))->Bits.LTM_CLIP_TH_ALPHA_BASE = ct_alpha_base;

        put(SubModuleIndex, rRawIspCamInfo.rCCU_Result.LTM.LTM_Curve);

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);
        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_CURVE[SubModuleIndex]), LTM_CURVE_SIZE, pReg);

        dumpRegInfoP2("LTM", SubModuleIndex);

    }

    return  MTRUE;
}



}
