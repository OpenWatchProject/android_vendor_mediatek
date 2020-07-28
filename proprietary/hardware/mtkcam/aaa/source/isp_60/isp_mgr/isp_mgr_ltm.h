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
#ifndef _ISP_MGR_LTM_H_
#define _ISP_MGR_LTM_H_




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LTM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LTM_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELTM_R1 ,reg, LTM_R1_LTM_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ELTM_D1 ,reg, LTM_D1A_LTM_)

#define LTM_BLOCK_XNUM (12)
#define LTM_BLOCK_YNUM (9)

typedef class ISP_MGR_LTM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LTM  MyType;
public:

    enum
    {
        ELTM_R1, //Pass1 Normal
        ELTM_D1, //Pass2
        ESubModule_NUM
    };

private:

    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        ERegInfo_CTRL,
        ERegInfo_BLK_NUM,
        ERegInfo_BLK_SZ,
        ERegInfo_BLK_DIVX,
        ERegInfo_BLK_DIVY,
        ERegInfo_MAX_DIV,
        ERegInfo_CLIP,
        //ERegInfo_TILE_NUM,
        //ERegInfo_TILE_CNTX,
        //ERegInfo_TILE_CNTY,
        ERegInfo_CFG,
        //ERegInfo_RESET,
        //ERegInfo_INTEN,
        //ERegInfo_INTSTA,
        //ERegInfo_STATUS,
        //ERegInfo_INPUT_COUNT,
        //ERegInfo_OUTPUT_COUNT,
        //ERegInfo_CHKSUM,
        //ERegInfo_TILE_SIZE,
        //ERegInfo_TILE_EDGE,
        ERegInfo_TILE_CROP,
        //ERegInfo_DUMMY_REG,
        //ERegInfo_SRAM_CFG,
        //ERegInfo_SRAM_STATUS,
        //ERegInfo_ATPG,
        //ERegInfo_SHADOW_CTRL,
        //ERegInfo_SELRGB_GRAD0,
        //ERegInfo_SELRGB_GRAD1,
        //ERegInfo_SELRGB_GRAD2,
        //ERegInfo_SELRGB_GRAD3,
        ERegInfo_SELRGB_TH0,
        ERegInfo_SELRGB_TH1,
        ERegInfo_SELRGB_TH2,
        ERegInfo_SELRGB_TH3,
        //ERegInfo_SELRGB_SLP0,
        //ERegInfo_SELRGB_SLP1,
        //ERegInfo_SELRGB_SLP2,
        //ERegInfo_SELRGB_SLP3,
        //ERegInfo_SELRGB_SLP4,
        //ERegInfo_SELRGB_SLP5,
        //ERegInfo_SELRGB_SLP6,
        ERegInfo_OUT_STR,
        //ERegInfo_SRAM_PINGPONG,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_CURVE[ESubModule_NUM][LTM_CURVE_SIZE];

public:
    ISP_MGR_LTM(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
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
        // register info addr init
        INIT_LTM_ADDR(CTRL);
        INIT_LTM_ADDR(BLK_NUM);
        INIT_LTM_ADDR(BLK_SZ);
        INIT_LTM_ADDR(BLK_DIVX);
        INIT_LTM_ADDR(BLK_DIVY);
        INIT_LTM_ADDR(MAX_DIV);
        INIT_LTM_ADDR(CLIP);
        //INIT_LTM_ADDR(TILE_NUM);
        //INIT_LTM_ADDR(TILE_CNTX);
        //INIT_LTM_ADDR(TILE_CNTY);
        INIT_LTM_ADDR(CFG);
        //INIT_LTM_ADDR(RESET);
        //INIT_LTM_ADDR(INTEN);
        //INIT_LTM_ADDR(INTSTA);
        //INIT_LTM_ADDR(STATUS);
        //INIT_LTM_ADDR(INPUT_COUNT);
        //INIT_LTM_ADDR(OUTPUT_COUNT);
        //INIT_LTM_ADDR(CHKSUM);
        //INIT_LTM_ADDR(TILE_SIZE);
        //INIT_LTM_ADDR(TILE_EDGE);
        INIT_LTM_ADDR(TILE_CROP);
        //INIT_LTM_ADDR(DUMMY_REG);
        //INIT_LTM_ADDR(SRAM_CFG);
        //INIT_LTM_ADDR(SRAM_STATUS);
        //INIT_LTM_ADDR(ATPG);
        //INIT_LTM_ADDR(SHADOW_CTRL);
        //INIT_LTM_ADDR(SELRGB_GRAD0);
        //INIT_LTM_ADDR(SELRGB_GRAD1);
        //INIT_LTM_ADDR(SELRGB_GRAD2);
        //INIT_LTM_ADDR(SELRGB_GRAD3);
        INIT_LTM_ADDR(SELRGB_TH0);
        INIT_LTM_ADDR(SELRGB_TH1);
        INIT_LTM_ADDR(SELRGB_TH2);
        INIT_LTM_ADDR(SELRGB_TH3);
        //INIT_LTM_ADDR(SELRGB_SLP0);
        //INIT_LTM_ADDR(SELRGB_SLP1);
        //INIT_LTM_ADDR(SELRGB_SLP2);
        //INIT_LTM_ADDR(SELRGB_SLP3);
        //INIT_LTM_ADDR(SELRGB_SLP4);
        //INIT_LTM_ADDR(SELRGB_SLP5);
        //INIT_LTM_ADDR(SELRGB_SLP6);
        INIT_LTM_ADDR(OUT_STR);
        //INIT_LTM_ADDR(SRAM_PINGPONG);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<LTM_REG_D1A_LTM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, CTRL))->Bits.LTM_LUM_MAP_MODE = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_BLK_NUM*>(REG_INFO_VALUE_PTR_MULTI(i, BLK_NUM))->Bits.LTM_BLK_X_NUM = LTM_BLOCK_XNUM;
            reinterpret_cast<LTM_REG_D1A_LTM_BLK_NUM*>(REG_INFO_VALUE_PTR_MULTI(i, BLK_NUM))->Bits.LTM_BLK_Y_NUM = LTM_BLOCK_YNUM;
            reinterpret_cast<LTM_REG_D1A_LTM_CLIP*>(REG_INFO_VALUE_PTR_MULTI(i, CLIP))->Bits.LTM_GAIN_TH = 8;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_LTM_ENGINE_EN = 1;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_LTM_CG_DISABLE = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_CHKSUM_EN = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_CFG*>(REG_INFO_VALUE_PTR_MULTI(i, CFG))->Bits.LTM_CHKSUM_SEL = 0;
            reinterpret_cast<LTM_REG_D1A_LTM_TILE_CROP*>(REG_INFO_VALUE_PTR_MULTI(i, TILE_CROP))->Bits.LTM_CROP_H = 2;
            reinterpret_cast<LTM_REG_D1A_LTM_TILE_CROP*>(REG_INFO_VALUE_PTR_MULTI(i, TILE_CROP))->Bits.LTM_CROP_V = 2;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH0))->Bits.LTM_SELRGB_TH0 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH0))->Bits.LTM_SELRGB_TH1 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH0))->Bits.LTM_SELRGB_TH0 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH0))->Bits.LTM_SELRGB_TH1 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH1*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH1))->Bits.LTM_SELRGB_TH2 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH1*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH1))->Bits.LTM_SELRGB_TH3 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH2*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH2))->Bits.LTM_SELRGB_TH4 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH2*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH2))->Bits.LTM_SELRGB_TH5 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH3*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH3))->Bits.LTM_SELRGB_TH6 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_SELRGB_TH3*>(REG_INFO_VALUE_PTR_MULTI(i, SELRGB_TH3))->Bits.LTM_SELRGB_TH7 = 64;
            reinterpret_cast<LTM_REG_D1A_LTM_OUT_STR*>(REG_INFO_VALUE_PTR_MULTI(i, OUT_STR))->Bits.LTM_OUT_STR = 16;
        }
    }

    virtual ~ISP_MGR_LTM() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }
#if 0
    MBOOL
    isCCTEnable(MUINT8 SubModuleIndex)
    {
        return m_bCCTEnable[SubModuleIndex];
    }

    MVOID
    setCCTEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bCCTEnable[SubModuleIndex] = bEnable;
    }
#endif
    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_LTM_T;

template <MUINT32 const eSensorDev>
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

