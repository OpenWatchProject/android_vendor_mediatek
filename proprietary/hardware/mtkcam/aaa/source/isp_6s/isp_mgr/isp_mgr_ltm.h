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
#ifndef _ISP_MGR_LTM_H_
#define _ISP_MGR_LTM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LTM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LTM_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELTM_R1, reg, LTM_R1_LTM_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ELTM_D1, reg, LTM_D1A_LTM_);\

#define LTM_BLOCK_XNUM (12)
#define LTM_BLOCK_YNUM (9)

typedef class ISP_MGR_LTM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LTM    MyType;
public:
    enum
    {
        ELTM_R1,
        ELTM_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CTRL,
        ERegInfo_BLK_NUM,
        ERegInfo_BLK_SZ,
        ERegInfo_BLK_DIVX,
        ERegInfo_BLK_DIVY,
        ERegInfo_MAX_DIV,
        ERegInfo_CLIP,
        ERegInfo_CLIP_TH,
        ERegInfo_CFG,
        ERegInfo_TILE_CROP,
        ERegInfo_GAIN_MAP,
        ERegInfo_CVNODE_GRP0,
        ERegInfo_CVNODE_GRP1,
        ERegInfo_CVNODE_GRP2,
        ERegInfo_CVNODE_GRP3,
        ERegInfo_CVNODE_GRP4,
        ERegInfo_CVNODE_GRP5,
        ERegInfo_CVNODE_GRP6,
        ERegInfo_CVNODE_GRP7,
        ERegInfo_CVNODE_GRP8,
        ERegInfo_CVNODE_GRP9,
        ERegInfo_CVNODE_GRP10,
        ERegInfo_CVNODE_GRP11,
        ERegInfo_CVNODE_GRP12,
        ERegInfo_CVNODE_GRP13,
        ERegInfo_CVNODE_GRP14,
        ERegInfo_CVNODE_GRP15,
        ERegInfo_CVNODE_GRP16,
        ERegInfo_OUT_STR,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_CURVE[ESubModule_NUM][LTM_CURVE_SIZE];

protected:
    ISP_MGR_LTM(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }

        MUINT32 u4StartAddr[ESubModule_NUM];
        u4StartAddr[ELTM_R1]= REG_ADDR_P1(LTMTC_R1_LTMTC_CURVE[0]);
        u4StartAddr[ELTM_D1]= REG_ADDR_P2(LTMTC_D1A_LTMTC_CURVE[0]);

        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
            for (MINT32 j = 0; j < LTM_CURVE_SIZE; j++) {
                m_rIspRegInfo_CURVE[i][j].val = 0 ;
                m_rIspRegInfo_CURVE[i][j].addr = u4StartAddr[i] + 4*j;
            }
        }
        INIT_LTM_ADDR(CTRL);
        INIT_LTM_ADDR(BLK_NUM);
        INIT_LTM_ADDR(BLK_SZ);
        INIT_LTM_ADDR(BLK_DIVX);
        INIT_LTM_ADDR(BLK_DIVY);
        INIT_LTM_ADDR(MAX_DIV);
        INIT_LTM_ADDR(CLIP);
        INIT_LTM_ADDR(CLIP_TH);
        INIT_LTM_ADDR(GAIN_MAP);
        INIT_LTM_ADDR(CVNODE_GRP0);
        INIT_LTM_ADDR(CVNODE_GRP1);
        INIT_LTM_ADDR(CVNODE_GRP2);
        INIT_LTM_ADDR(CVNODE_GRP3);
        INIT_LTM_ADDR(CVNODE_GRP4);
        INIT_LTM_ADDR(CVNODE_GRP5);
        INIT_LTM_ADDR(CVNODE_GRP6);
        INIT_LTM_ADDR(CVNODE_GRP7);
        INIT_LTM_ADDR(CVNODE_GRP8);
        INIT_LTM_ADDR(CVNODE_GRP9);
        INIT_LTM_ADDR(CVNODE_GRP10);
        INIT_LTM_ADDR(CVNODE_GRP11);
        INIT_LTM_ADDR(CVNODE_GRP12);
        INIT_LTM_ADDR(CVNODE_GRP13);
        INIT_LTM_ADDR(CVNODE_GRP14);
        INIT_LTM_ADDR(CVNODE_GRP15);
        INIT_LTM_ADDR(CVNODE_GRP16);
        INIT_LTM_ADDR(OUT_STR);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<LTM_REG_D1A_LTM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, CTRL))->Bits.LTM_LUM_MAP_MODE = 1;
            reinterpret_cast<LTM_REG_D1A_LTM_BLK_NUM*>(REG_INFO_VALUE_PTR_MULTI(i, BLK_NUM))->Bits.LTM_BLK_X_NUM = LTM_BLOCK_XNUM;
            reinterpret_cast<LTM_REG_D1A_LTM_BLK_NUM*>(REG_INFO_VALUE_PTR_MULTI(i, BLK_NUM))->Bits.LTM_BLK_Y_NUM = LTM_BLOCK_YNUM;
            reinterpret_cast<LTM_REG_D1A_LTM_CLIP*>(REG_INFO_VALUE_PTR_MULTI(i, CLIP))->Bits.LTM_GAIN_TH = 8;

            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_LTM_ENGINE_EN = 1;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_LTM_CG_DISABLE = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_CHKSUM_EN = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_CHKSUM_SEL = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_TILE_CROP*>(REG_INFO_VALUE_PTR_MULTI(i, TILE_CROP))->Bits.LTM_CROP_H = 2;
            reinterpret_cast<LTM_REG_D1A_LTM_TILE_CROP*>(REG_INFO_VALUE_PTR_MULTI(i, TILE_CROP))->Bits.LTM_CROP_V = 2;
            reinterpret_cast<LTM_REG_D1A_LTM_OUT_STR*>(REG_INFO_VALUE_PTR_MULTI(i, OUT_STR))->Bits.LTM_OUT_STR = 16;

    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_MAP_LOG_EN         = 0 ;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_WGT_LOG_EN         = 0 ;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_NONTRAN_MAP_TYPE   = 1 ;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_TRAN_MAP_TYPE      = 5 ;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_TRAN_WGT_TYPE      = 0 ;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_TRAN_WGT           = 16;
    reinterpret_cast<LTM_REG_D1A_LTM_GAIN_MAP*>(REG_INFO_VALUE_PTR_MULTI(i, GAIN_MAP))->Bits.LTM_RANGE_SCL          = 4 ;
    reinterpret_cast<LTM_REG_D1A_LTM_CLIP_TH*>(REG_INFO_VALUE_PTR_MULTI(i, CLIP_TH))->Bits.LTM_CLIP_TH   = 4096;
    reinterpret_cast<LTM_REG_D1A_LTM_CLIP_TH*>(REG_INFO_VALUE_PTR_MULTI(i, CLIP_TH))->Bits.LTM_WGT_BSH   = 0;

     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP0*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP0))->Bits.LTM_CVNODE_0     = 0   ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP0*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP0))->Bits.LTM_CVNODE_1     = 1   ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP1*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP1))->Bits.LTM_CVNODE_2     = 9   ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP1*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP1))->Bits.LTM_CVNODE_3     = 22  ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP2*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP2))->Bits.LTM_CVNODE_4     = 42  ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP2*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP2))->Bits.LTM_CVNODE_5     = 68  ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP3*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP3))->Bits.LTM_CVNODE_6     = 103 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP3*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP3))->Bits.LTM_CVNODE_7     = 144 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP4*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP4))->Bits.LTM_CVNODE_8     = 193 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP4*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP4))->Bits.LTM_CVNODE_9     = 251 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP5*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP5))->Bits.LTM_CVNODE_10    = 316 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP5*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP5))->Bits.LTM_CVNODE_11    = 390 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP6*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP6))->Bits.LTM_CVNODE_12    = 473 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP6*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP6))->Bits.LTM_CVNODE_13    = 564 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP7*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP7))->Bits.LTM_CVNODE_14    = 664 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP7*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP7))->Bits.LTM_CVNODE_15    = 773 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP8*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP8))->Bits.LTM_CVNODE_16    = 891 ;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP8*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP8))->Bits.LTM_CVNODE_17    = 1018;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP9*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP9))->Bits.LTM_CVNODE_18    = 1154;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP9*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP9))->Bits.LTM_CVNODE_19    = 1300;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP10*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP10))->Bits.LTM_CVNODE_20   = 1456;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP10*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP10))->Bits.LTM_CVNODE_21   = 1621;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP11*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP11))->Bits.LTM_CVNODE_22   = 1795;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP11*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP11))->Bits.LTM_CVNODE_23   = 1980;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP12*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP12))->Bits.LTM_CVNODE_24   = 2174;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP12*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP12))->Bits.LTM_CVNODE_25   = 2378;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP13*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP13))->Bits.LTM_CVNODE_26   = 2593;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP13*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP13))->Bits.LTM_CVNODE_27   = 2817;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP14*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP14))->Bits.LTM_CVNODE_28   = 3052;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP14*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP14))->Bits.LTM_CVNODE_29   = 3297;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP15*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP15))->Bits.LTM_CVNODE_30   = 3552;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP15*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP15))->Bits.LTM_CVNODE_31   = 3818;
     reinterpret_cast<LTM_REG_D1A_LTM_CVNODE_GRP16*>(REG_INFO_VALUE_PTR_MULTI(i, CVNODE_GRP16))->Bits.LTM_CVNODE_32   = 4095;
        }

    }

    virtual ~ISP_MGR_LTM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public:

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        return m_bEnable[SubModuleIndex];
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_LTM_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_LTM_DEV : public ISP_MGR_LTM_T
{
public:
    static
    ISP_MGR_LTM_T&
    getInstance()
    {
        static ISP_MGR_LTM_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_LTM_DEV()
        : ISP_MGR_LTM_T(eSensorDev)
    {}

    virtual ~ISP_MGR_LTM_DEV() {}

};
#endif
