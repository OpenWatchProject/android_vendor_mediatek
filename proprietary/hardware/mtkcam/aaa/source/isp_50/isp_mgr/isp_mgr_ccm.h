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
#ifndef _ISP_MGR_CCM_H_
#define _ISP_MGR_CCM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CCM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CCM    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    //MBOOL m_bCFCEnable;
    //MBOOL m_bCFCCCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0x0920
    MBOOL m_bRWBSensor; //default: MFALSE
    MINT32 m_MdpCCM[3*3];
    float m_MdpCCM_Y2R[3*3];
    MUINT32 m_u4ISO;
    MBOOL m_bMono;
    MBOOL m_bManual;
    enum
    {
        ERegInfo_DIP_X_G2G_CNV_1,
        ERegInfo_DIP_X_G2G_CNV_2,
        ERegInfo_DIP_X_G2G_CNV_3,
        ERegInfo_DIP_X_G2G_CNV_4,
        ERegInfo_DIP_X_G2G_CNV_5,
        ERegInfo_DIP_X_G2G_CNV_6,
        ERegInfo_DIP_X_G2G_CTRL,
        ERegInfo_DIP_X_G2G_CFC,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CCM(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        //, m_bCFCEnable(MTRUE)
        //, m_bCFCCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_G2G_CNV_1))
        , m_bRWBSensor(MFALSE)
        , m_u4ISO(0)
        , m_bMono(MFALSE)
        , m_bManual(MFALSE)
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_1); // CAM+0x0920
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_2); // CAM+0x0924
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_3); // CAM+0x0928
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_4); // CAM+0x092C
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_5); // CAM+0x0930
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CNV_6); // CAM+0x0934
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CTRL);  // CAM+0x0938
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G_CFC);
    }

    virtual ~ISP_MGR_CCM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }
/*
    MBOOL
    isCFCEnable()
    {
        return m_bCFCEnable;
    }
*/
    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }
/*
    MVOID
    setCFCEnable(MBOOL bEnable)
    {
        m_bCFCEnable = bEnable;
    }
*/
    MVOID
    setMono(MBOOL bEnable)
    {
        m_bMono = bEnable;
    }

    MVOID
    setManual(MBOOL bEnable)
    {
        m_bManual = bEnable;
    }

    MVOID
    setRWBSensorInfo(MBOOL bRWBSensor)
    {
        m_bRWBSensor = bRWBSensor;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }
/*
    MBOOL
    isCFCCCTEnable()
    {
        return m_bCFCCCTEnable;
    }
*/
    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }
/*
    MVOID
    setCFCCCTEnable(MBOOL bEnable)
    {
        m_bCFCCCTEnable = bEnable;
    }
*/
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_CCM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCM2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CCM2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CCM2    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    //MBOOL m_bCFCEnable;
    //MBOOL m_bCFCCCTEnable;
    MUINT32 m_u4StartAddr;
    MBOOL m_bRWBSensor; //default: MFALSE
    MINT32 m_MdpCCM2[3*3];
    float m_MdpCCM2_Y2R[3*3];
    MUINT32 m_u4ISO;
    MBOOL m_bMono;

    enum
    {
        ERegInfo_DIP_X_G2G2_CNV_1,
        ERegInfo_DIP_X_G2G2_CNV_2,
        ERegInfo_DIP_X_G2G2_CNV_3,
        ERegInfo_DIP_X_G2G2_CNV_4,
        ERegInfo_DIP_X_G2G2_CNV_5,
        ERegInfo_DIP_X_G2G2_CNV_6,
        ERegInfo_DIP_X_G2G2_CTRL,
        ERegInfo_DIP_X_G2G2_CFC,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CCM2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        //, m_bCFCEnable(MTRUE)
        //, m_bCFCCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_G2G2_CNV_1))
        , m_bRWBSensor(MFALSE)
        , m_u4ISO(0)
        , m_bMono(MFALSE)
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_1); // CAM+0x0920
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_2); // CAM+0x0924
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_3); // CAM+0x0928
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_4); // CAM+0x092C
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_5); // CAM+0x0930
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CNV_6); // CAM+0x0934
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CTRL);  // CAM+0x0938
        INIT_REG_INFO_ADDR_P2(DIP_X_G2G2_CFC);
    }

    virtual ~ISP_MGR_CCM2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

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

    MVOID
    setMono(MBOOL bEnable)
    {
        m_bMono = bEnable;
    }

    MVOID
    setRWBSensorInfo(MBOOL bRWBSensor)
    {
        m_bRWBSensor = bRWBSensor;
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

    MVOID
    setISO(MUINT32 iso)
    {
        m_u4ISO = iso;
    }

    MyType& put(ISP_NVRAM_CCM_T const& rParam);

    MyType& get(ISP_NVRAM_CCM_T & rParam);

    MBOOL get(ISP_NVRAM_CCM_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_CCM2_T;

#endif

