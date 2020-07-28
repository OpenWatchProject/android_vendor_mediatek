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

#define INIT_REG_INFO_ADDR_PDO_MULTI(index, reg)\
    REG_INFO_ADDR_MULTI(index, reg) = REG_ADDR_P1(reg)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                PDO config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_PDO_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_PDO_CONFIG MyType;

private:
    MUINT32       m_bDebugEnable;
    ISP_PDO_CFG_T m_sPDOHWCfg;

    enum
    {
        ER1,
        ESUB_MODULE_NUM
    };

    enum
    {
        ERegInfo_PDO_R1_PDO_XSIZE,
        ERegInfo_PDO_R1_PDO_YSIZE,
        ERegInfo_PDO_R1_PDO_STRIDE,
        ERegInfo_PBN_R1_PBN_PBN_TYPE,
        ERegInfo_PBN_R1_PBN_PBN_LST,

        ERegInfo_PDE_R1_PDE_TBLI1,
        ERegInfo_PDE_R1_PDE_CTL,
        ERegInfo_PDI_R1_PDI_BASE_ADDR,
        ERegInfo_PDI_R1_PDI_XSIZE,
        ERegInfo_PDI_R1_PDI_YSIZE,

        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESUB_MODULE_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_PDO_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev)
    {
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDO_R1_PDO_XSIZE);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDO_R1_PDO_YSIZE);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDO_R1_PDO_STRIDE);
//        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PBN_R1_PBN_PBN_TYPE);
//        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PBN_R1_PBN_PBN_LST);

        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDE_R1_PDE_TBLI1);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDE_R1_PDE_CTL);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDI_R1_PDI_BASE_ADDR);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDI_R1_PDI_XSIZE);
        INIT_REG_INFO_ADDR_PDO_MULTI(ER1, PDI_R1_PDI_YSIZE);

        m_bDebugEnable  = 0;
    }

    virtual ~ISP_MGR_PDO_CONFIG() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //Interfaces

    /**
     * @ command for pd_mgr.
     * @ configure HW setting..
     * @param :
     *        [ in] sInPdoCfg - PDO setting.
     * @Return :
     *        [out] NA
     */
    MVOID configReg( ISP_PDO_CFG_T *sInPdohwCfg);

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

template <ESensorDev_T const eSensorDev>
class ISP_MGR_PDO_CONFIG_DEV : public ISP_MGR_PDO_CONFIG_T
{
public:
    static
    ISP_MGR_PDO_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_PDO_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_PDO_CONFIG_DEV()
        : ISP_MGR_PDO_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_MGR_PDO_CONFIG_DEV() {}

};

#endif
