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
#ifndef _ISP_MGR_NBC2_H_
#define _ISP_MGR_NBC2_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NBC2 (ANR2 + CCR + BOK)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NBC2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NBC2    MyType;
private:
    MBOOL m_bANR2_ENYEnable;
    MBOOL m_bANR2_ENCEnable;
    MBOOL m_bCCREnable;
    MBOOL m_bBOKEnable;
    MBOOL m_bCCREnBackup;
	MBOOL m_bANR2ENCBackup;
	MBOOL m_bANR2ENYBackup;
    MBOOL m_bCCTCCREnable;
    MBOOL m_bCCTANR2Enable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x0A20

    enum
    { 
		ERegInfo_CAM_ANR2_CON1, 	   
		ERegInfo_CAM_ANR2_CON2, 	   
		ERegInfo_CAM_ANR2_YAD1, 	   
		ERegInfo_CAM_ANR2_Y4LUT1,	   
		ERegInfo_CAM_ANR2_Y4LUT2,	   
		ERegInfo_CAM_ANR2_Y4LUT3,	   
		ERegInfo_CAM_ANR2_L4LUT1,	   
		ERegInfo_CAM_ANR2_L4LUT2,	   
		ERegInfo_CAM_ANR2_L4LUT3,	   
		ERegInfo_CAM_ANR2_CAD,		   
		ERegInfo_CAM_ANR2_PTC,		   
		ERegInfo_CAM_ANR2_LCE,		   
		ERegInfo_CAM_ANR2_MED1, 	   
		ERegInfo_CAM_ANR2_MED2, 	   
		ERegInfo_CAM_ANR2_MED3, 	   
		ERegInfo_CAM_ANR2_MED4, 	   
		ERegInfo_CAM_ANR2_ACTY, 	   
		ERegInfo_CAM_ANR2_ACTC, 	   
		ERegInfo_CAM_ANR2_RSV1, 	   
		ERegInfo_CAM_ANR2_RSV2,									   
		ERegInfo_CAM_CCR_CON,		   
		ERegInfo_CAM_CCR_YLUT,		   
		ERegInfo_CAM_CCR_UVLUT, 	   
		ERegInfo_CAM_CCR_YLUT2, 	   
		ERegInfo_CAM_CCR_SAT_CTRL,	   
		ERegInfo_CAM_CCR_UVLUT_SP,	   
		ERegInfo_CAM_CCR_HUE1,		   
		ERegInfo_CAM_CCR_HUE2,		   
		ERegInfo_CAM_CCR_HUE3,		   
		ERegInfo_CAM_CCR_RSV1,		   									   
		ERegInfo_CAM_BOK_CON,		   
		ERegInfo_CAM_BOK_TUN,		   
		ERegInfo_CAM_BOK_OFF,		   
		ERegInfo_CAM_BOK_RSV1,		   		
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_NBC2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bANR2_ENYEnable(MTRUE)
        , m_bANR2_ENCEnable(MTRUE)
        , m_bCCREnable(MTRUE)
        , m_bBOKEnable(MFALSE)
        , m_bCCREnBackup(MFALSE)
        , m_bANR2ENCBackup(MFALSE)
        , m_bANR2ENYBackup(MFALSE)
        , m_bCCTCCREnable(MTRUE)
        , m_bCCTANR2Enable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_ANR2_CON1))
    {
        // register info addr init
        INIT_REG_INFO_ADDR(CAM_ANR2_CON1);
        INIT_REG_INFO_ADDR(CAM_ANR2_CON2);
        INIT_REG_INFO_ADDR(CAM_ANR2_YAD1);
        INIT_REG_INFO_ADDR(CAM_ANR2_Y4LUT1);
        INIT_REG_INFO_ADDR(CAM_ANR2_Y4LUT2);
        INIT_REG_INFO_ADDR(CAM_ANR2_Y4LUT3);
        INIT_REG_INFO_ADDR(CAM_ANR2_L4LUT1);   
        INIT_REG_INFO_ADDR(CAM_ANR2_L4LUT2);   
        INIT_REG_INFO_ADDR(CAM_ANR2_L4LUT3);   
        INIT_REG_INFO_ADDR(CAM_ANR2_CAD); 
        INIT_REG_INFO_ADDR(CAM_ANR2_PTC); 
        INIT_REG_INFO_ADDR(CAM_ANR2_LCE); 
        INIT_REG_INFO_ADDR(CAM_ANR2_MED1);    
        INIT_REG_INFO_ADDR(CAM_ANR2_MED2);    
        INIT_REG_INFO_ADDR(CAM_ANR2_MED3);    
        INIT_REG_INFO_ADDR(CAM_ANR2_MED4);    
        INIT_REG_INFO_ADDR(CAM_ANR2_ACTY);
        INIT_REG_INFO_ADDR(CAM_ANR2_ACTC);
        INIT_REG_INFO_ADDR(CAM_ANR2_RSV1);    
        INIT_REG_INFO_ADDR(CAM_ANR2_RSV2);    
        INIT_REG_INFO_ADDR(CAM_CCR_CON);
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT);
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT); 
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT2);
        INIT_REG_INFO_ADDR(CAM_CCR_SAT_CTRL);
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT_SP); 
        INIT_REG_INFO_ADDR(CAM_CCR_HUE1);
        INIT_REG_INFO_ADDR(CAM_CCR_HUE2);
        INIT_REG_INFO_ADDR(CAM_CCR_HUE3);
        INIT_REG_INFO_ADDR(CAM_CCR_RSV1);
        INIT_REG_INFO_ADDR(CAM_BOK_CON);
        INIT_REG_INFO_ADDR(CAM_BOK_TUN);
        INIT_REG_INFO_ADDR(CAM_BOK_OFF);          		
    }

    virtual ~ISP_MGR_NBC2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isANR2_ENYEnable()
    {
        return m_bANR2_ENYEnable;
    }

    MBOOL
    isANR2_ENCEnable()
    {
        return m_bANR2_ENCEnable;
    }

    MBOOL
    isCCREnable()
    {
        return m_bCCREnable;
    }

    MBOOL
    isCCTCCREnable()
    {
        return m_bCCTCCREnable;
    }

    MBOOL
    isCCTANR2Enable()
    {
        return m_bCCTANR2Enable;
    }

    MVOID
    setANR2Enable(MBOOL bEnable)
    {
         setANR2_ENYEnable(bEnable);
         setANR2_ENCEnable(bEnable);
    }

    MVOID
    setANR2_ENYEnable(MBOOL bEnable)
    {
        m_bANR2_ENYEnable = bEnable;
    }

    MVOID
    setANR2_ENCEnable(MBOOL bEnable)
    {
        m_bANR2_ENCEnable = bEnable;
    }

    MVOID
    setCCREnable(MBOOL bEnable)
    {
        m_bCCREnable = bEnable;
    }

    MVOID
    setBOKEnable(MBOOL bEnable)
    {
        m_bBOKEnable = bEnable;
    }

    MVOID
    setCCTANR2Enable(MBOOL bEnable)
    {
         m_bCCTANR2Enable = bEnable;
    }

    MVOID
    setCCTCCREnable(MBOOL bEnable)
    {
        m_bCCTCCREnable = bEnable;
    }
     

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_NBC2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NBC2_DEV : public ISP_MGR_NBC2_T
{
public:
    static
    ISP_MGR_NBC2_T&
    getInstance()
    {
        static ISP_MGR_NBC2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NBC2_DEV()
        : ISP_MGR_NBC2_T(eSensorDev)
    {}

    virtual ~ISP_MGR_NBC2_DEV() {}

};


#endif

