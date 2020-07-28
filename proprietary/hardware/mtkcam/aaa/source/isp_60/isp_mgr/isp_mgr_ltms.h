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
#ifndef _ISP_MGR_LTMS_H_
#define _ISP_MGR_LTMS_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LTMS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LTMS_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELTMS_R1, reg, LTMS_R1_LTMS_);\

typedef class ISP_MGR_LTMS : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LTMS    MyType;
private:

    enum
    {
        ELTMS_R1, //Pass1 Normal
        ESubModule_NUM
    };

    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        ERegInfo_CTRL,
        ERegInfo_BLK_NUM,
        ERegInfo_BLK_SZ,
        ERegInfo_BLK_AREA,
        ERegInfo_DETAIL,
        ERegInfo_HIST,
        ERegInfo_FLTLINE,
        ERegInfo_FLTBLK,
        ERegInfo_CLIP,
        ERegInfo_MAX_DIV,
        ERegInfo_CFG,
        ERegInfo_RESET,
        ERegInfo_INTEN,
        ERegInfo_INTSTA,
        ERegInfo_STATUS,
        ERegInfo_INPUT_COUNT,
        ERegInfo_OUTPUT_COUNT,
        ERegInfo_CHKSUM,
        ERegInfo_IN_SIZE,
        ERegInfo_OUT_SIZE,
        ERegInfo_ACT_WINDOW_X,
        ERegInfo_ACT_WINDOW_Y,
        ERegInfo_OUT_DATA_NUM,
        ERegInfo_DUMMY_REG,
        ERegInfo_SRAM_CFG,
        ERegInfo_ATPG,
        ERegInfo_SHADOW_CTRL,
        ERegInfo_HIST_R,
        ERegInfo_HIST_B,
        ERegInfo_HIST_C,
        ERegInfo_FLATLINE_R,
        ERegInfo_FLATBLK_B,
        ERegInfo_BLK_R_AREA,
        ERegInfo_BLK_B_AREA,
        ERegInfo_BLK_C_AREA,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_LTMS(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_LTMS_ADDR(CTRL);
        INIT_LTMS_ADDR(BLK_NUM);
        INIT_LTMS_ADDR(BLK_SZ);
        INIT_LTMS_ADDR(BLK_AREA);
        INIT_LTMS_ADDR(DETAIL);
        INIT_LTMS_ADDR(HIST);
        INIT_LTMS_ADDR(FLTLINE);
        INIT_LTMS_ADDR(FLTBLK);
        INIT_LTMS_ADDR(CLIP);
        INIT_LTMS_ADDR(MAX_DIV);
        INIT_LTMS_ADDR(CFG);
        INIT_LTMS_ADDR(RESET);
        INIT_LTMS_ADDR(INTEN);
        INIT_LTMS_ADDR(INTSTA);
        INIT_LTMS_ADDR(STATUS);
        INIT_LTMS_ADDR(INPUT_COUNT);
        INIT_LTMS_ADDR(OUTPUT_COUNT);
        INIT_LTMS_ADDR(CHKSUM);
        INIT_LTMS_ADDR(IN_SIZE);
        INIT_LTMS_ADDR(OUT_SIZE);
        INIT_LTMS_ADDR(ACT_WINDOW_X);
        INIT_LTMS_ADDR(ACT_WINDOW_Y);
        INIT_LTMS_ADDR(OUT_DATA_NUM);
        INIT_LTMS_ADDR(DUMMY_REG);
        INIT_LTMS_ADDR(SRAM_CFG);
        INIT_LTMS_ADDR(ATPG);
        INIT_LTMS_ADDR(SHADOW_CTRL);
        INIT_LTMS_ADDR(HIST_R);
        INIT_LTMS_ADDR(HIST_B);
        INIT_LTMS_ADDR(HIST_C);
        INIT_LTMS_ADDR(FLATLINE_R);
        INIT_LTMS_ADDR(FLATBLK_B);
        INIT_LTMS_ADDR(BLK_R_AREA);
        INIT_LTMS_ADDR(BLK_B_AREA);
        INIT_LTMS_ADDR(BLK_C_AREA);
    }

    virtual ~ISP_MGR_LTMS() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    MyType& put(MUINT8 SubModuleIndex, ISP_NVRAM_LTMS_T const& rParam);

    MyType& get(MUINT8 SubModuleIndex, ISP_NVRAM_LTMS_T & rParam);

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

} ISP_MGR_LTMS_T;


#endif
