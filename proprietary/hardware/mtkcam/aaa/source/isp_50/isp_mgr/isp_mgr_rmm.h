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
#ifndef _ISP_MGR_RMM_H_
#define _ISP_MGR_RMM_H_

#include <cutils/properties.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef class ISP_MGR_RMM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_RMM    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MINT32 debugDump;
#if 0
    MFLOAT mAeExpRatio;

    MINT32 m_iAwbRgain;
    MINT32 m_iAwbGgain;
    MINT32 m_iAwbBgain;
#endif

    MUINT32 m_u4StartAddr; // Debug address

    enum
    {
        ERegInfo_CAM_RMM_OSC,
        ERegInfo_CAM_RMM_MC,
        ERegInfo_CAM_RMM_REVG_1,
        ERegInfo_CAM_RMM_REVG_2,
        ERegInfo_CAM_RMM_LEOS,
        ERegInfo_CAM_RMM_MC2,
        ERegInfo_CAM_RMM_DIFF_LB,
        ERegInfo_CAM_RMM_MA,
        ERegInfo_CAM_RMM_TUNE,
        ERegInfo_CAM_RMM_IDX,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_RMM(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , m_bCCTEnable(MTRUE)
        , debugDump(0)
#if 0
        , mAeExpRatio(1.0)
        , m_iAwbRgain(1024)
        , m_iAwbGgain(1024)
        , m_iAwbBgain(1024)
#endif
        , m_u4StartAddr(REG_ADDR_P1(CAM_RMM_OSC))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_RMM_OSC);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_MC);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_REVG_1);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_REVG_2);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_LEOS);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_MC2);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_DIFF_LB);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_MA);
        INIT_REG_INFO_ADDR_P1(CAM_RMM_TUNE);
        //INIT_REG_INFO_ADDR_P1(CAM_RMM_IDX);

        INIT_REG_INFO_VALUE(CAM_RMM_OSC, 3060 | (4 << 12) | (9 << 16) | (9 << 20) );
        INIT_REG_INFO_VALUE(CAM_RMM_MC, 1 | (1 << 2) | (6 << 4) | (4 << 8) | (4 << 16));
        INIT_REG_INFO_VALUE(CAM_RMM_REVG_1, 1024 | (1024 << 16));
        INIT_REG_INFO_VALUE(CAM_RMM_REVG_2, 1024 | (1024 << 16));
        INIT_REG_INFO_VALUE(CAM_RMM_LEOS, 512);
        INIT_REG_INFO_VALUE(CAM_RMM_MC2, 383 | (5 << 16));
        INIT_REG_INFO_VALUE(CAM_RMM_DIFF_LB, 0);
        INIT_REG_INFO_VALUE(CAM_RMM_MA, 8 | (9 << 8) | (8 << 12 ) | (8 << 16 ) | ( 8 << 24));
        INIT_REG_INFO_VALUE(CAM_RMM_TUNE, 1 | (1 << 1) | (1 << 4) | (765 << 16 ) | (1 << 28) | ( 1<< 29));
        //INIT_REG_INFO_VALUE(CAM_RMM_IDX, 255 | (64 << 8) | ( 32 << 16));

    }

    virtual ~ISP_MGR_RMM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set RMM enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    MUINT32
    getRMM_SWIndex()
    {
        return REG_INFO_VALUE(CAM_RMM_IDX);
    }
#if 0
    /*==== Set AE ratio ===*/
    MVOID setAeLeSeRatio(MINT32 aeRatio); // aeRatio = (LE/SE) * 100

    /*==== Set AWB Gain ===*/
    MVOID
    setAwbGain(AWB_GAIN_T awbGain)
    {
        m_iAwbRgain = 512 * 1024 / awbGain.i4R;
        m_iAwbGgain = 512 * 1024 / awbGain.i4G;
        m_iAwbBgain = 512 * 1024 / awbGain.i4B;
    }
#endif

    //MBOOL apply(EIspProfile_T eIspProfile, TuningMgr& rTuning);
    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_RMM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMM2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef class ISP_MGR_RMM2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_RMM2    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MINT32 debugDump;
#if 0
    MFLOAT mAeExpRatio;

    MINT32 m_iAwbRgain;
    MINT32 m_iAwbGgain;
    MINT32 m_iAwbBgain;
#endif

    MUINT32 m_u4StartAddr; // Debug address

    enum
    {
        ERegInfo_DIP_X_RMM2_OSC,
        ERegInfo_DIP_X_RMM2_MC,
        ERegInfo_DIP_X_RMM2_REVG_1,
        ERegInfo_DIP_X_RMM2_REVG_2,
        ERegInfo_DIP_X_RMM2_LEOS,
        ERegInfo_DIP_X_RMM2_MC2,
        ERegInfo_DIP_X_RMM2_DIFF_LB,
        ERegInfo_DIP_X_RMM2_MA,
        ERegInfo_DIP_X_RMM2_TUNE,
        ERegInfo_DIP_X_RMM2_IDX,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_RMM2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , m_bCCTEnable(MTRUE)
        , debugDump(0)
#if 0
        , mAeExpRatio(1.0)
        , m_iAwbRgain(1024)
        , m_iAwbGgain(1024)
        , m_iAwbBgain(1024)
#endif
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_RMM2_OSC))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_OSC);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_MC);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_REVG_1);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_REVG_2);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_LEOS);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_MC2);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_DIFF_LB);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_MA);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMM2_TUNE);
        //INIT_REG_INFO_ADDR_P1(DIP_X_RMM2_IDX);

        INIT_REG_INFO_VALUE(DIP_X_RMM2_OSC, 3060 | (4 << 12) | (9 << 16) | (9 << 20) );
        INIT_REG_INFO_VALUE(DIP_X_RMM2_MC, 1 | (1 << 2) | (6 << 4) | (4 << 8) | (4 << 16));
        INIT_REG_INFO_VALUE(DIP_X_RMM2_REVG_1, 1024 | (1024 << 16));
        INIT_REG_INFO_VALUE(DIP_X_RMM2_REVG_2, 1024 | (1024 << 16));
        INIT_REG_INFO_VALUE(DIP_X_RMM2_LEOS, 512);
        INIT_REG_INFO_VALUE(DIP_X_RMM2_MC2, 383 | (5 << 16));
        INIT_REG_INFO_VALUE(DIP_X_RMM2_DIFF_LB, 0);
        INIT_REG_INFO_VALUE(DIP_X_RMM2_MA, 8 | (9 << 8) | (8 << 12 ) | (8 << 16 ) | ( 8 << 24));
        INIT_REG_INFO_VALUE(DIP_X_RMM2_TUNE, 1 | (1 << 1) | (1 << 4) | (765 << 16 ) | (1 << 28) | ( 1<< 29));
        //INIT_REG_INFO_VALUE(DIP_X_RMM2_IDX, 255 | (64 << 8) | ( 32 << 16));

    }

    virtual ~ISP_MGR_RMM2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set RMM enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    MUINT32
    getRMM2_SWIndex()
    {
        return REG_INFO_VALUE(DIP_X_RMM2_IDX);
    }
#if 0
    /*==== Set AE ratio ===*/
    MVOID setAeLeSeRatio(MINT32 aeRatio); // aeRatio = (LE/SE) * 100

    /*==== Set AWB Gain ===*/
    MVOID
    setAwbGain(AWB_GAIN_T awbGain)
    {
        m_iAwbRgain = 512 * 1024 / awbGain.i4R;
        m_iAwbGgain = 512 * 1024 / awbGain.i4G;
        m_iAwbBgain = 512 * 1024 / awbGain.i4B;
    }
#endif

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_RMM2_T;


#endif


