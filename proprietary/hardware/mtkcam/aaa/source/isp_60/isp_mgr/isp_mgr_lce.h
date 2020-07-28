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
#ifndef _ISP_MGR_LCE_H_
#define _ISP_MGR_LCE_H_




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LCE_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(ELCE_D1 ,reg, LCE_D1A_LCE_)


typedef class ISP_MGR_LCE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LCE  MyType;
public:
    enum
    {
        ELCE_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        //ERegInfo_CON,
        ERegInfo_ZR,
        ERegInfo_SLM_SIZE,
        //ERegInfo_OFST,
        //ERegInfo_BIAS,
        //ERegInfo_IMAGE_SIZE,
        ERegInfo_GLOBAL,
        ERegInfo_CEN_PARA0,
        ERegInfo_CEN_PARA1,
        ERegInfo_CEN_PARA2,
        ERegInfo_BIL_TH0,
        ERegInfo_BIL_TH1,
        ERegInfo_TM_PARA0,
        ERegInfo_TM_PARA1,
        ERegInfo_TM_PARA2,
        ERegInfo_TM_PARA3,
        ERegInfo_TM_PARA4,
        ERegInfo_TM_PARA5,
        ERegInfo_TM_PARA6,
        ERegInfo_TM_PARA7,
        ERegInfo_HLR_PARA0,
        ERegInfo_HLR_PARA1,
        ERegInfo_HLR_PARA2,
        ERegInfo_HLR_PARA3,
        ERegInfo_HLR_PARA4,
        ERegInfo_HLR_PARA5,
        ERegInfo_HLR_PARA6,
        ERegInfo_HLR_PARA7,
        ERegInfo_TCHL_PARA0,
        ERegInfo_TCHL_PARA1,
        //ERegInfo_DUM,
        //Dummy for Tile
        ERegInfo_SLM,
        ERegInfo_OFFSET,
        ERegInfo_OUT,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];
    ISP_NVRAM_LCE_GLOBAL_T m_lce_glocal_param[ESubModule_NUM];

public:
    ISP_MGR_LCE(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        //INIT_LCE_ADDR(CON);
        INIT_LCE_ADDR(ZR);
        INIT_LCE_ADDR(SLM_SIZE);
        //INIT_LCE_ADDR(OFST);
        //INIT_LCE_ADDR(BIAS);
        //INIT_LCE_ADDR(IMAGE_SIZE);
        INIT_LCE_ADDR(GLOBAL);
        INIT_LCE_ADDR(CEN_PARA0);
        INIT_LCE_ADDR(CEN_PARA1);
        INIT_LCE_ADDR(CEN_PARA2);
        INIT_LCE_ADDR(BIL_TH0);
        INIT_LCE_ADDR(BIL_TH1);
        INIT_LCE_ADDR(TM_PARA0);
        INIT_LCE_ADDR(TM_PARA1);
        INIT_LCE_ADDR(TM_PARA2);
        INIT_LCE_ADDR(TM_PARA3);
        INIT_LCE_ADDR(TM_PARA4);
        INIT_LCE_ADDR(TM_PARA5);
        INIT_LCE_ADDR(TM_PARA6);
        INIT_LCE_ADDR(TM_PARA7);

        INIT_LCE_ADDR(HLR_PARA0);
        INIT_LCE_ADDR(HLR_PARA1);
        INIT_LCE_ADDR(HLR_PARA2);
        INIT_LCE_ADDR(HLR_PARA3);
        INIT_LCE_ADDR(HLR_PARA4);
        INIT_LCE_ADDR(HLR_PARA5);
        INIT_LCE_ADDR(HLR_PARA6);
        INIT_LCE_ADDR(HLR_PARA7);
        INIT_LCE_ADDR(TCHL_PARA0);
        INIT_LCE_ADDR(TCHL_PARA1);
        //INIT_LCE_ADDR(DUM);

        INIT_LCE_ADDR(SLM);
        INIT_LCE_ADDR(OFFSET);
        INIT_LCE_ADDR(OUT);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<LCE_REG_D1A_LCE_GLOBAL*>(REG_INFO_VALUE_PTR_MULTI(i, GLOBAL))->Bits.LCE_GLOB_POS     = 1;
            // DCE Top Control
            reinterpret_cast<LCE_REG_D1A_LCE_GLOBAL*>(REG_INFO_VALUE_PTR_MULTI(i, GLOBAL))->Bits.LCE_GLOB_TONE    = 1;
            // LCE Top Control
            reinterpret_cast<LCE_REG_D1A_LCE_GLOBAL*>(REG_INFO_VALUE_PTR_MULTI(i, GLOBAL))->Bits.LCE_LC_TONE      = 1;

            m_lce_glocal_param[i].val = reinterpret_cast<LCE_REG_D1A_LCE_GLOBAL*>(REG_INFO_VALUE_PTR_MULTI(i, GLOBAL))->Raw;

            reinterpret_cast<LCE_REG_D1A_LCE_BIL_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, BIL_TH0))->Bits.LCE_BIL_TH1 = 400;
            reinterpret_cast<LCE_REG_D1A_LCE_BIL_TH0*>(REG_INFO_VALUE_PTR_MULTI(i, BIL_TH0))->Bits.LCE_BIL_TH2 = 800;
            reinterpret_cast<LCE_REG_D1A_LCE_BIL_TH1*>(REG_INFO_VALUE_PTR_MULTI(i, BIL_TH1))->Bits.LCE_BIL_TH3 = 1200;
            reinterpret_cast<LCE_REG_D1A_LCE_BIL_TH1*>(REG_INFO_VALUE_PTR_MULTI(i, BIL_TH1))->Bits.LCE_BIL_TH4 = 1600;
        }

    }

    virtual ~ISP_MGR_LCE() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);


    MUINT32
    getGlobleTone(MUINT8 SubModuleIndex)
    {
        return reinterpret_cast<LCE_REG_D1A_LCE_GLOBAL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GLOBAL))->Bits.LCE_GLOB_TONE ;
    }

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

    MVOID transform_LCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_LCE_T;




#endif

