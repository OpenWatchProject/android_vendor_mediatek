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
#ifndef _ISP_MGR_LSC_H_
#define _ISP_MGR_LSC_H_

#include <lsc/ILscBuf.h>
#include <lsc/ILscTbl.h>

/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LSC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LSC    MyType;
private:
    MUINT32 m_u4StartAddr;
    MBOOL m_fgOnOff;
    MINT32 m_FD;
    MUINT32* m_bufVirAddr;

    enum
    {
        ERegInfo_CAM_LSCI_BASE_ADDR,
        ERegInfo_CAM_LSCI_OFST_ADDR,
        ERegInfo_CAM_LSCI_XSIZE,
        ERegInfo_CAM_LSCI_YSIZE,
        ERegInfo_CAM_LSCI_STRIDE,
        ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL1 = ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL2,
        ERegInfo_CAM_LSC_CTL3,
        ERegInfo_CAM_LSC_LBLOCK,
        ERegInfo_CAM_LSC_RATIO_0,
        ERegInfo_CAM_LSC_GAIN_TH,
        ERegInfo_CAM_LSC_RATIO_1,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

    //dynamic bin
    MBOOL m_isBin;
    ILscTbl::Config m_rawConfig;
    ILscTbl::Config m_binConfig;

public:
    ISP_MGR_LSC(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P1(CAM_LSCI_BASE_ADDR))
        , m_fgOnOff(MFALSE)
        , m_FD(0)
        , m_bufVirAddr(NULL)
        , m_isBin(MFALSE)
    {

    //for build pass
        INIT_REG_INFO_ADDR_P1(CAM_LSCI_BASE_ADDR); // 0x1500726c
        INIT_REG_INFO_ADDR_P1(CAM_LSCI_OFST_ADDR); // 0x15007270
        INIT_REG_INFO_ADDR_P1(CAM_LSCI_XSIZE); // 0x15007274
        INIT_REG_INFO_ADDR_P1(CAM_LSCI_YSIZE); // 0x15007278
        INIT_REG_INFO_ADDR_P1(CAM_LSCI_STRIDE); // 0x1500727c
        INIT_REG_INFO_ADDR_P1(CAM_LSC_CTL1);    // 0x15004530
        INIT_REG_INFO_ADDR_P1(CAM_LSC_CTL2);    // 0x15004534
        INIT_REG_INFO_ADDR_P1(CAM_LSC_CTL3);    // 0x15004538
        INIT_REG_INFO_ADDR_P1(CAM_LSC_LBLOCK);  // 0x1500453C
        INIT_REG_INFO_ADDR_P1(CAM_LSC_RATIO_0);   // 0x15004540
        INIT_REG_INFO_ADDR_P1(CAM_LSC_GAIN_TH); // 0x1500454C
        INIT_REG_INFO_ADDR_P1(CAM_LSC_RATIO_1);
    }

    virtual ~ISP_MGR_LSC() {}
public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MyType& putRatio(MUINT32 u4Ratio);
    MyType& putAddr(MUINT32 u4BaseAddr);
    MUINT32 getAddr();

    // override to do nothing
    MBOOL reset();

    MBOOL putBuf(NSIspTuning::ILscBuf& rBuf);
    MBOOL putBufAndRatio(NSIspTuning::ILscBuf& rBuf, MUINT32 ratio);

    MBOOL isBin();
    MBOOL setIsBin(MBOOL isBin);
    MBOOL setLSCconfigParam(ILscTbl::Config &rRawConfig, ILscTbl::Config &rBinConfig);
    ILscTbl::Config getRawConfig();
    ILscTbl::Config getBinConfig();
    MBOOL reconfig(MVOID* rDBinInfo, MVOID* rOutRegCfg);

    MBOOL apply(EIspProfile_T eIspProfile, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
    //MBOOL apply(EIspProfile_T eIspProfile, ESensorTG_T const eSensorTG, cam_reg_t* pReg);
    MVOID enableLsc(MBOOL enable);
    MBOOL isEnable(void);
} ISP_MGR_LSC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LSC2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LSC2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LSC2    MyType;
private:
    MUINT32 m_u4StartAddr;
    MBOOL m_fgOnOff;

    enum
    {
        ERegInfo_DIP_X_DEPI_BASE_ADDR,
        ERegInfo_DIP_X_DEPI_OFST_ADDR,
        ERegInfo_DIP_X_DEPI_XSIZE,
        ERegInfo_DIP_X_DEPI_YSIZE,
        ERegInfo_DIP_X_DEPI_STRIDE,
        ERegInfo_DIP_X_LSC2_START,
        ERegInfo_DIP_X_LSC2_CTL1 = ERegInfo_DIP_X_LSC2_START,
        ERegInfo_DIP_X_LSC2_CTL2,
        ERegInfo_DIP_X_LSC2_CTL3,
        ERegInfo_DIP_X_LSC2_LBLOCK,
        ERegInfo_DIP_X_LSC2_RATIO_0,
        ERegInfo_DIP_X_LSC2_GAIN_TH,
        ERegInfo_DIP_X_LSC2_RATIO_1,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_LSC2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_DEPI_BASE_ADDR))
        , m_fgOnOff(MFALSE)
    {
        ::memset(m_rIspRegInfo, 0, sizeof(RegInfo_T)*ERegInfo_NUM);

        INIT_REG_INFO_ADDR_P2(DIP_X_DEPI_BASE_ADDR); // 0x1500726c
        INIT_REG_INFO_ADDR_P2(DIP_X_DEPI_OFST_ADDR); // 0x15007270
        INIT_REG_INFO_ADDR_P2(DIP_X_DEPI_XSIZE); // 0x15007274
        INIT_REG_INFO_ADDR_P2(DIP_X_DEPI_YSIZE); // 0x15007278
        INIT_REG_INFO_ADDR_P2(DIP_X_DEPI_STRIDE); // 0x1500727c
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_CTL1);    // 0x15004530
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_CTL2);    // 0x15004534
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_CTL3);    // 0x15004538
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_LBLOCK);  // 0x1500453C
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_RATIO_0);   // 0x15004540
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_GAIN_TH); // 0x1500454C
        INIT_REG_INFO_ADDR_P2(DIP_X_LSC2_RATIO_1);   // 0x15004540

    }

    virtual ~ISP_MGR_LSC2() {}
public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MyType& putAddr(MUINT32 u4BaseAddr);
    MUINT32 getAddr();

    // override to do nothing
    MBOOL reset();

    MBOOL putBuf(NSIspTuning::ILscBuf& rBuf);

    MBOOL apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg);
    MVOID enableLsc(MBOOL enable);
    MBOOL isEnable(void);
} ISP_MGR_LSC2_T;


};
#endif

