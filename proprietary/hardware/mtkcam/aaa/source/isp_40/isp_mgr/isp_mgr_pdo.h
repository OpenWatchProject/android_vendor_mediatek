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
#ifndef _ISP_MGR_PDO_H_
#define _ISP_MGR_PDO_H_

#include <isp_mgr.h>
#include <pd_buf_common.h>

using namespace NS3Av3;

namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                PDO config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_PDO_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_PDO_CONFIG    MyType;

private:
    mutable Mutex m_Lock;
    MUINT32      m_bDebugEnable;
    MBOOL         m_bUpdateEngine;
    SPDOHWINFO_T m_sPDOHWInfo;
    MUINT32       m_u4StartAddr;
    MINT32        m_i4SensorIdx;

    enum
    {
        //ERegInfo_CAM_BPCI_BASE_ADDR,
        //ERegInfo_CAM_BPCI_XSIZE,
        //ERegInfo_CAM_BPCI_YSIZE,
        //ERegInfo_CAM_BPCI_STRIDE,
        ERegInfo_CAM_PDO_XSIZE,
        ERegInfo_CAM_PDO_YSIZE,
        ERegInfo_CAM_PDO_STRIDE,
        ERegInfo_CAM_PBN_TYPE,
        ERegInfo_CAM_PBN_LST,

        ERegInfo_CAM_PDE_TBLI1,
        ERegInfo_CAM_PDI_BASE_ADDR,
        ERegInfo_CAM_PDI_XSIZE,
        ERegInfo_CAM_PDI_YSIZE,

        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_PDO_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P1(CAM_AFO_XSIZE))
    {
        //INIT_REG_INFO_ADDR_P1(CAM_BPCI_BASE_ADDR); /* 0x1A004370 */
        //INIT_REG_INFO_ADDR_P1(CAM_BPCI_XSIZE);     /* 0x1A004380 */
        //INIT_REG_INFO_ADDR_P1(CAM_BPCI_YSIZE);     /* 0x1A004384 */
        //INIT_REG_INFO_ADDR_P1(CAM_BPCI_STRIDE);    /* 0x1A004388 */
        INIT_REG_INFO_ADDR_P1(CAM_PDO_XSIZE);      /* 0x1A004350 */
        INIT_REG_INFO_ADDR_P1(CAM_PDO_YSIZE);      /* 0x1A004354 */
        INIT_REG_INFO_ADDR_P1(CAM_PDO_STRIDE);     /* 0x1A004358 */
        INIT_REG_INFO_ADDR_P1(CAM_PBN_TYPE);       /* 0x1A004BB0 */
        INIT_REG_INFO_ADDR_P1(CAM_PBN_LST);        /* 0x1A004BB4 */

        INIT_REG_INFO_ADDR_P1(CAM_PDE_TBLI1);      /* 0x1A004CF0 */
        INIT_REG_INFO_ADDR_P1(CAM_PDI_BASE_ADDR);  /* 0x1A004D50 */
        INIT_REG_INFO_ADDR_P1(CAM_PDI_XSIZE);      /* 0x1A004D5C */
        INIT_REG_INFO_ADDR_P1(CAM_PDI_YSIZE);      /* 0x1A004D60 */

        m_i4SensorIdx   = 0;
        m_bDebugEnable = 0;
        m_bUpdateEngine = MFALSE;
    }

    virtual ~ISP_MGR_PDO_CONFIG() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //Interfaces

    /**
     * @ command for af_mgr.
     * @ initial parameters for hand shake.
     * @param :
     *        [ in] NA
     * @Return :
     *        [out] NA.
     */
    MVOID start(MINT32 i4SensorIdx);

    /**
     * @ command for af_mgr.
     * @ uninitial parameters for hand shake.
     * @param :
     *        [ in] sensor device index.
     * @Return :
     *        [out] NA.
     */
    MVOID stop();

    /**
     * @ command for pd_mgr.
     * @ configure HW setting..
     * @param :
     *        [ in] sInPdoCfg - PDO setting.
     * @Return :
     *        [out] NA
     */
    MVOID config( SPDOHWINFO_T &sInPdohwCfg);

    /**
     * @ command for isp_tuning_mgr.
     * @ apply HW setting
     * @param :
     *        [ in] rTuning
     *        [ in] i4Magic - magic number of request
     * @Return :
     *        [out] MTRUE-HW is ready, MFALSE-HW is not ready.
     */
    MBOOL apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex=0);

} ISP_MGR_PDO_CONFIG_T;

}
#endif
