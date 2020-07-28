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

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LSC_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELSC_R1 ,reg, LSC_R1_LSC_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ELSC_D1A ,reg, LSC_D1A_LSC_)

#define INIT_LSCI_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELSCI_R1 ,reg, LSCI_R1_LSCI_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EIMGCI_D1A ,reg, IMGCI_D1A_IMGCI_)

#define INIT_TSFS_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ETSFS_R1 ,reg, TSFS_R1_TSFS_);

#define INIT_TSFSO_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ETSFSO_R1 ,reg, TSFSO_R1_);

#undef MAX
#define MAX(a, b) ((a) >= (b)) ? (a) : (b)

typedef class ISP_MGR_LSC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LSC    MyType;
private:
    enum
    {
        ELSC_R1,              //Pass1 Normal
        ELSCI_R1  =ELSC_R1,   //Pass1 shading table
        ETSFS_R1  =ELSC_R1,   //Pass1 TSFS
        ETSFSO_R1 =ELSC_R1,   //Pass1 TSFSO
        ELSC_D1A,             //Pass2 Normal
        EIMGCI_D1A=ELSC_D1A, // Pass2 shading table
        ESubModule_NUM
    };

    enum
    {
        //LSC
        ERegInfo_CTL1,
        ERegInfo_CTL2,
        ERegInfo_CTL3,
        ERegInfo_LBLOCK,
        ERegInfo_RATIO_0,
        ERegInfo_GAIN_TH,
        ERegInfo_RATIO_1,

        //LSCI
        ERegInfo_BASE_ADDR,
        ERegInfo_OFST_ADDR,
        ERegInfo_XSIZE,
        ERegInfo_YSIZE,
        ERegInfo_STRIDE,

        //TSFS
        ERegInfo_ORG,
        ERegInfo_SIZE,
        ERegInfo_PIT,
        ERegInfo_NUM,
        ERegInfo_PC0,
        ERegInfo_PC1,
        ERegInfo_PC2,

        //TSFSO
        ERegInfo_TSFSO_XSIZE,
        ERegInfo_TSFSO_YSIZE,

        ERegInfo_LSC_NUM
    };

private:
    MBOOL m_fgOnOff[ESubModule_NUM];
    MINT32 m_FD;
    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_LSC_NUM];

public:
    ISP_MGR_LSC(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_LSC_NUM, eSensorDev, ESubModule_NUM)
        , m_FD(0)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_fgOnOff[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*(ERegInfo_LSC_NUM));
        }

        INIT_LSC_ADDR(CTL1);
        INIT_LSC_ADDR(CTL2);
        INIT_LSC_ADDR(CTL3);
        INIT_LSC_ADDR(LBLOCK);
        INIT_LSC_ADDR(RATIO_0);
        INIT_LSC_ADDR(GAIN_TH);
        INIT_LSC_ADDR(RATIO_1);

        INIT_LSCI_ADDR(BASE_ADDR);
        INIT_LSCI_ADDR(OFST_ADDR);
        INIT_LSCI_ADDR(XSIZE);
        INIT_LSCI_ADDR(YSIZE);
        INIT_LSCI_ADDR(STRIDE);

        INIT_TSFS_ADDR(ORG);
        INIT_TSFS_ADDR(SIZE);
        INIT_TSFS_ADDR(PIT);
        INIT_TSFS_ADDR(NUM);
        INIT_TSFS_ADDR(PC0);
        INIT_TSFS_ADDR(PC1);
        INIT_TSFS_ADDR(PC2);

        INIT_TSFSO_ADDR(TSFSO_XSIZE);
        INIT_TSFSO_ADDR(TSFSO_YSIZE);
    }

    virtual ~ISP_MGR_LSC() {}
public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MyType& putRatio(MUINT8 SubModuleIndex, MUINT32 u4Ratio);
    MyType& putAddr(MUINT8 SubModuleIndex, MUINT32 u4BaseAddr);
    MUINT32 getAddr(MUINT8 SubModuleIndex);

    MBOOL putBuf(MUINT8 SubModuleIndex, NSIspTuning::ILscBuf& rBuf);
    MBOOL putBufAndRatio(MUINT8 SubModuleIndex, NSIspTuning::ILscBuf& rBuf, MUINT32 ratio);

    MBOOL configTSFS_Size(MUINT8 SubModuleIndex, MUINT32 ImgWidth, MUINT32 ImgHeight, MUINT32 tsfsH, MUINT32 tsfsV);

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

    MVOID enableLsc(MUINT8 SubModuleIndex, MBOOL enable);
    MBOOL isEnable(MUINT8 SubModuleIndex);
} ISP_MGR_LSC_T;



};
#endif
