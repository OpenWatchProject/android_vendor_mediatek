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
#ifndef _ISP_MGR_NR3D_H_
#define _ISP_MGR_NR3D_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INIT_NR3D_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(ENR3D_D1 ,reg, NR3D_D1A_NR3D_NR3D_)
typedef class ISP_MGR_NR3D : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NR3D    MyType;
public:

    enum
    {
        ENR3D_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];

    //configuration for post_apply
    MRect       m_onRegion; // region modified by GMV
    MRect       m_fullImg; // image full size for demo mode calculation
    MUINT32     m_vipiOffst;      // in byte
    MSize       m_vipiReadSize;   // image size for vipi in pixel

    //GMV calculation
    MINT32 m_GMV_level_th;

    //R2C calculation
    MUINT32 m_NR3D_r2cf_cnt1;
    MUINT32 m_NR3D_r2cf_cnt2;
    MUINT32 m_NR3D_r2cf_cnt3;
    MUINT32 m_NR3D_r2cf_cnt4;

    enum
    {
        ERegInfo_ENG_CON,
        ERegInfo_SIZ,
        //ERegInfo_TILE_XY,
        ERegInfo_ON_CON,
        ERegInfo_ON_OFF,
        ERegInfo_ON_SIZ,
        ERegInfo_TNR_ENABLE,
        ERegInfo_FLT_CONFIG,
        ERegInfo_FB_INFO1,
        ERegInfo_THR_1,
        ERegInfo_CURVE_1,
        ERegInfo_CURVE_2,
        ERegInfo_CURVE_3,
        ERegInfo_CURVE_4,
        ERegInfo_CURVE_5,
        ERegInfo_CURVE_6,
        ERegInfo_CURVE_7,
        ERegInfo_CURVE_8,
        ERegInfo_CURVE_9,
        ERegInfo_CURVE_10,
        ERegInfo_CURVE_11,
        ERegInfo_CURVE_12,
        ERegInfo_CURVE_13,
        ERegInfo_CURVE_14,
        ERegInfo_CURVE_15,
        ERegInfo_R2C_1,
        ERegInfo_R2C_2,
        ERegInfo_R2C_3,
        ERegInfo_DBG_6,
        ERegInfo_DBG_15,
        ERegInfo_DBG_16,
        ERegInfo_DEMO_1,
        ERegInfo_DEMO_2,
        //ERegInfo_ATPG,
        //ERegInfo_DMY_0,
        //ERegInfo_DBG_17,
        //ERegInfo_INTERR,
        //ERegInfo_FB_INFO2,
        //ERegInfo_FB_INFO3,
        //ERegInfo_FB_INFO4,
        //ERegInfo_DBFISH,
        //ERegInfo_DBG_9,
        //ERegInfo_DBG_10,
        //ERegInfo_DBG_11,
        //ERegInfo_DBG_12,
        //ERegInfo_DBG_7,
        //ERegInfo_DMY_1,
        //ERegInfo_DMY_2,
        //ERegInfo_SAVE_INFO1,
        //ERegInfo_SAVE_INFO2,
        ERegInfo_SNR_CURVE_1,
        ERegInfo_SNR_CURVE_2,
        ERegInfo_SNR_CURVE_3,
        ERegInfo_SNR_CURVE_4,
        ERegInfo_SNR_CURVE_5,
        ERegInfo_SNR_CURVE_6,
        ERegInfo_SNR_CURVE_7,
        ERegInfo_SNR_CONTROL_1,
        ERegInfo_SNR_THR_2,
        ERegInfo_SNR_THR_3,
        ERegInfo_SNR_THR_4,
        //ERegInfo_IN1_CNT,
        //ERegInfo_IN2_CNT,
        //ERegInfo_IN3_CNT,
        //ERegInfo_OUT_CNT,
        //ERegInfo_STATUS,
        //ERegInfo_TILE_LOSS,
        //ERegInfo_MCVP_STAT1,
        //ERegInfo_MCVP_STAT2,
        //ERegInfo_MCVP_STAT3,
        //ERegInfo_MCVP_STAT4,
        //ERegInfo_MCVP_STAT5,
        //ERegInfo_MCVP_STAT6,
        //ERegInfo_MCVP_STAT7,
        //ERegInfo_MCVP_STAT8,
        //ERegInfo_MCVP_STAT9,
        ERegInfo_VIPI_SIZE,
        ERegInfo_VIPI_OFFSET,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_NR3D(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_vipiOffst(0)
        , m_GMV_level_th(200)
        , m_NR3D_r2cf_cnt1(0)
        , m_NR3D_r2cf_cnt2(0)
        , m_NR3D_r2cf_cnt3(0)
        , m_NR3D_r2cf_cnt4(0)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_NR3D_ADDR(ENG_CON);
        INIT_NR3D_ADDR(SIZ);
        //INIT_NR3D_ADDR(TILE_XY);
        INIT_NR3D_ADDR(ON_CON);
        INIT_NR3D_ADDR(ON_OFF);
        INIT_NR3D_ADDR(ON_SIZ);
        INIT_NR3D_ADDR(TNR_ENABLE);
        INIT_NR3D_ADDR(FLT_CONFIG);
        INIT_NR3D_ADDR(FB_INFO1);
        INIT_NR3D_ADDR(THR_1);
        INIT_NR3D_ADDR(CURVE_1);
        INIT_NR3D_ADDR(CURVE_2);
        INIT_NR3D_ADDR(CURVE_3);
        INIT_NR3D_ADDR(CURVE_4);
        INIT_NR3D_ADDR(CURVE_5);
        INIT_NR3D_ADDR(CURVE_6);
        INIT_NR3D_ADDR(CURVE_7);
        INIT_NR3D_ADDR(CURVE_8);
        INIT_NR3D_ADDR(CURVE_9);
        INIT_NR3D_ADDR(CURVE_10);
        INIT_NR3D_ADDR(CURVE_11);
        INIT_NR3D_ADDR(CURVE_12);
        INIT_NR3D_ADDR(CURVE_13);
        INIT_NR3D_ADDR(CURVE_14);
        INIT_NR3D_ADDR(CURVE_15);
        INIT_NR3D_ADDR(R2C_1);
        INIT_NR3D_ADDR(R2C_2);
        INIT_NR3D_ADDR(R2C_3);
        INIT_NR3D_ADDR(DBG_6);
        INIT_NR3D_ADDR(DBG_15);
        INIT_NR3D_ADDR(DBG_16);
        INIT_NR3D_ADDR(DEMO_1);
        INIT_NR3D_ADDR(DEMO_2);
        //INIT_NR3D_ADDR(ATPG);
        //INIT_NR3D_ADDR(DMY_0);
        //INIT_NR3D_ADDR(DBG_17);
        //INIT_NR3D_ADDR(INTERR);
        //INIT_NR3D_ADDR(FB_INFO2);
        //INIT_NR3D_ADDR(FB_INFO3);
        //INIT_NR3D_ADDR(FB_INFO4);
        //INIT_NR3D_ADDR(DBFISH);
        //INIT_NR3D_ADDR(DBG_9);
        //INIT_NR3D_ADDR(DBG_10);
        //INIT_NR3D_ADDR(DBG_11);
        //INIT_NR3D_ADDR(DBG_12);
        //INIT_NR3D_ADDR(DBG_7);
        //INIT_NR3D_ADDR(DMY_1);
        //INIT_NR3D_ADDR(DMY_2);
        //INIT_NR3D_ADDR(SAVE_INFO1);
        //INIT_NR3D_ADDR(SAVE_INFO2);
        INIT_NR3D_ADDR(SNR_CURVE_1);
        INIT_NR3D_ADDR(SNR_CURVE_2);
        INIT_NR3D_ADDR(SNR_CURVE_3);
        INIT_NR3D_ADDR(SNR_CURVE_4);
        INIT_NR3D_ADDR(SNR_CURVE_5);
        INIT_NR3D_ADDR(SNR_CURVE_6);
        INIT_NR3D_ADDR(SNR_CURVE_7);
        INIT_NR3D_ADDR(SNR_CONTROL_1);
        INIT_NR3D_ADDR(SNR_THR_2);
        INIT_NR3D_ADDR(SNR_THR_3);
        INIT_NR3D_ADDR(SNR_THR_4);
        //INIT_NR3D_ADDR(IN1_CNT);
        //INIT_NR3D_ADDR(IN2_CNT);
        //INIT_NR3D_ADDR(IN3_CNT);
        //INIT_NR3D_ADDR(OUT_CNT);
        //INIT_NR3D_ADDR(STATUS);
        //INIT_NR3D_ADDR(TILE_LOSS);
        //INIT_NR3D_ADDR(MCVP_STAT1);
        //INIT_NR3D_ADDR(MCVP_STAT2);
        //INIT_NR3D_ADDR(MCVP_STAT3);
        //INIT_NR3D_ADDR(MCVP_STAT4);
        //INIT_NR3D_ADDR(MCVP_STAT5);
        //INIT_NR3D_ADDR(MCVP_STAT6);
        //INIT_NR3D_ADDR(MCVP_STAT7);
        //INIT_NR3D_ADDR(MCVP_STAT8);
        //INIT_NR3D_ADDR(MCVP_STAT9);
        INIT_NR3D_ADDR(VIPI_SIZE);
        INIT_NR3D_ADDR(VIPI_OFFSET);

        for(int i=0; i<ESubModule_NUM; i++){
            //Sub module always ENABLE
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, ENG_CON))->Bits.NR3D_yclnr_en = MTRUE;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, ENG_CON))->Bits.NR3D_nr3d_uv_signed = 1;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, ENG_CON))->Bits.NR3D_nr3d_lbit_mode = 0;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, ENG_CON))->Bits.NR3D_r2c_lbit_mode = 1;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, ENG_CON))->Bits.NR3D_in_rnd_mode = 0;
        }
    }

    virtual ~ISP_MGR_NR3D() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

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

    MVOID
    setConfig(MRect const& onRegion, MRect const& fullImg,
        MUINT32 const vipiOffst, MSize const& vipiReadSize)
    {
        m_onRegion = onRegion;
        m_fullImg = fullImg;
        m_vipiOffst = vipiOffst;
        m_vipiReadSize = vipiReadSize;
    }

    MVOID
    setGMVLevelTh(MINT32 GMVLevelTh)
    {
        m_GMV_level_th = GMVLevelTh;
    }

    MINT32
    getGMVLevelTh()
    {
        return m_GMV_level_th;
    }

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MVOID adaptive_NR3D_setting(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfos);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_NR3D_T;




#endif

