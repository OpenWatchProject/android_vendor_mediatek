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
#ifndef _ISP_MGR_YNRS_H_
#define _ISP_MGR_YNRS_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YNRS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_YNRS_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EYNRS_R1 ,reg, YNRS_R1_YNRS_)


typedef class ISP_MGR_YNRS : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_YNRS  MyType;
public:
    enum
    {
        EYNRS_R1, //Pass1 Normal
        ESubModule_NUM
    };

private:
    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        ERegInfo_CON1,
        ERegInfo_CON2,
        ERegInfo_YAD2,
        ERegInfo_Y4LUT1,
        ERegInfo_Y4LUT2,
        ERegInfo_Y4LUT3,
        ERegInfo_C4LUT1,
        ERegInfo_C4LUT2,
        ERegInfo_C4LUT3,
        ERegInfo_A4LUT2,
        ERegInfo_A4LUT3,
        ERegInfo_L4LUT1,
        ERegInfo_L4LUT2,
        ERegInfo_L4LUT3,
        ERegInfo_PTY0V,
        ERegInfo_CAD,
        ERegInfo_SL2,
        ERegInfo_PTY0H,
        ERegInfo_T4LUT1,
        ERegInfo_T4LUT2,
        ERegInfo_T4LUT3,
        ERegInfo_ACT1,
        ERegInfo_PTCV,
        ERegInfo_ACT4,
        ERegInfo_PTCH,
        ERegInfo_HF_COR,
        ERegInfo_HF_ACT0,
        ERegInfo_HF_ACT1,
        ERegInfo_ACTC,
        ERegInfo_YLAD,
        ERegInfo_HF_ACT2,
        ERegInfo_HF_ACT3,
        ERegInfo_HF_LUMA0,
        ERegInfo_HF_LUMA1,
        ERegInfo_Y4LUT4,
        ERegInfo_Y4LUT5,
        ERegInfo_Y4LUT6,
        ERegInfo_Y4LUT7,
        ERegInfo_A4LUT1,
        //ERegInfo_SRAM_PINGPONG,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][YNRS_TBL_NUM + ERegInfo_NUM];

public:
    ISP_MGR_YNRS(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, (YNRS_TBL_NUM + ERegInfo_NUM), eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)* (ERegInfo_NUM + YNRS_TBL_NUM));
        }
        // register info addr init
        INIT_YNRS_ADDR(CON1);
        INIT_YNRS_ADDR(CON2);
        INIT_YNRS_ADDR(YAD2);
        INIT_YNRS_ADDR(Y4LUT1);
        INIT_YNRS_ADDR(Y4LUT2);
        INIT_YNRS_ADDR(Y4LUT3);
        INIT_YNRS_ADDR(C4LUT1);
        INIT_YNRS_ADDR(C4LUT2);
        INIT_YNRS_ADDR(C4LUT3);
        INIT_YNRS_ADDR(A4LUT2);
        INIT_YNRS_ADDR(A4LUT3);
        INIT_YNRS_ADDR(L4LUT1);
        INIT_YNRS_ADDR(L4LUT2);
        INIT_YNRS_ADDR(L4LUT3);
        INIT_YNRS_ADDR(PTY0V);
        INIT_YNRS_ADDR(CAD);
        INIT_YNRS_ADDR(SL2);
        INIT_YNRS_ADDR(PTY0H);
        INIT_YNRS_ADDR(T4LUT1);
        INIT_YNRS_ADDR(T4LUT2);
        INIT_YNRS_ADDR(T4LUT3);
        INIT_YNRS_ADDR(ACT1);
        INIT_YNRS_ADDR(PTCV);
        INIT_YNRS_ADDR(ACT4);
        INIT_YNRS_ADDR(PTCH);
        INIT_YNRS_ADDR(HF_COR);
        INIT_YNRS_ADDR(HF_ACT0);
        INIT_YNRS_ADDR(HF_ACT1);
        INIT_YNRS_ADDR(ACTC);
        INIT_YNRS_ADDR(YLAD);
        INIT_YNRS_ADDR(HF_ACT2);
        INIT_YNRS_ADDR(HF_ACT3);
        INIT_YNRS_ADDR(HF_LUMA0);
        INIT_YNRS_ADDR(HF_LUMA1);
        INIT_YNRS_ADDR(Y4LUT4);
        INIT_YNRS_ADDR(Y4LUT5);
        INIT_YNRS_ADDR(Y4LUT6);
        INIT_YNRS_ADDR(Y4LUT7);
        INIT_YNRS_ADDR(A4LUT1);
        //INIT_YNRS_ADDR(SRAM_PINGPONG);

        MUINT32 u4StartAddr[ESubModule_NUM];
        u4StartAddr[0]= REG_ADDR_P1(YNRS_R1_YNRS_TBL[0]);
        for(int i=0; i<ESubModule_NUM; i++){
            for (MINT32 j = 0; j < YNRS_TBL_NUM; j++) {
                m_rIspRegInfo[i][ERegInfo_NUM + j].val = 0 ;
                m_rIspRegInfo[i][ERegInfo_NUM + j].addr = u4StartAddr[i] + 4*j;
            }
        }

    }

    virtual ~ISP_MGR_YNRS() {}

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

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_YNRS_T;



#endif
