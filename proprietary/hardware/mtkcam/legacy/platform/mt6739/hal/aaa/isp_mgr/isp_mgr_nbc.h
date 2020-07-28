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
#ifndef _ISP_MGR_NBC_H_
#define _ISP_MGR_NBC_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NBC (ANR + CCR)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NBC    MyType;
private:
    MBOOL m_bANR_ENYEnable;
    MBOOL m_bANR_ENCEnable;
    MBOOL m_bCCREnable;
    MBOOL m_bCCREnBackup;
    MBOOL m_bANRENCBackup;
    MBOOL m_bANRENYBackup;
    MBOOL m_bCCTCCREnable;
    MBOOL m_bCCTANREnable;
    MBOOL m_bANRTBLEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x0A20

    enum
    {
        ERegInfo_CAM_ANR_CON1,
        ERegInfo_CAM_ANR_CON2,
        ERegInfo_CAM_ANR_YAD1,
        ERegInfo_CAM_ANR_YAD2,
        ERegInfo_CAM_ANR_Y4LUT1,
        ERegInfo_CAM_ANR_Y4LUT2,
        ERegInfo_CAM_ANR_Y4LUT3,
		ERegInfo_CAM_ANR_C4LUT1,
		ERegInfo_CAM_ANR_C4LUT2,
		ERegInfo_CAM_ANR_C4LUT3,
		ERegInfo_CAM_ANR_A4LUT2,
		ERegInfo_CAM_ANR_A4LUT3,
		ERegInfo_CAM_ANR_L4LUT1,
		ERegInfo_CAM_ANR_L4LUT2,
		ERegInfo_CAM_ANR_L4LUT3,
        ERegInfo_CAM_ANR_PTY,
        ERegInfo_CAM_ANR_CAD,
        ERegInfo_CAM_ANR_PTC,
        ERegInfo_CAM_ANR_LCE,
		ERegInfo_CAM_ANR_MED1,
		ERegInfo_CAM_ANR_MED2,
		ERegInfo_CAM_ANR_MED3,
		ERegInfo_CAM_ANR_MED4,
        ERegInfo_CAM_ANR_HP1,
        ERegInfo_CAM_ANR_HP2,
        ERegInfo_CAM_ANR_HP3,
		ERegInfo_CAM_ANR_ACT1,
		ERegInfo_CAM_ANR_ACT2,
		ERegInfo_CAM_ANR_ACT3,
        ERegInfo_CAM_ANR_ACTYH,
        ERegInfo_CAM_ANR_ACTC,
        ERegInfo_CAM_ANR_ACTYL,
        ERegInfo_CAM_ANR_YLAD,
        ERegInfo_CAM_ANR_PTYL,
        ERegInfo_CAM_ANR_LCOEF,

        ERegInfo_CAM_CCR_CON,
        ERegInfo_CAM_CCR_YLUT,
        ERegInfo_CAM_CCR_UVLUT,
        ERegInfo_CAM_CCR_YLUT2,
        ERegInfo_CAM_CCR_SAT_CTRL,
        ERegInfo_CAM_CCR_UVLUT_SP,
        ERegInfo_CAM_CCR_HUE1,
        ERegInfo_CAM_CCR_HUE2,
        ERegInfo_CAM_CCR_HUE3,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_ANR_TBL[ANR_HW_TBL_NUM];

protected:
    ISP_MGR_NBC(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bANR_ENYEnable(MTRUE)
        , m_bANR_ENCEnable(MTRUE)
        , m_bCCREnable(MTRUE)
        , m_bCCREnBackup(MFALSE)
        , m_bANRENCBackup(MFALSE)
        , m_bANRENYBackup(MFALSE)
        , m_bCCTCCREnable(MTRUE)
        , m_bCCTANREnable(MTRUE)
        , m_bANRTBLEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_ANR_CON1))
    {
        INIT_REG_INFO_ADDR(CAM_ANR_CON1);
        INIT_REG_INFO_ADDR(CAM_ANR_CON2);
        INIT_REG_INFO_ADDR(CAM_ANR_YAD1);
        INIT_REG_INFO_ADDR(CAM_ANR_YAD2);
        INIT_REG_INFO_ADDR(CAM_ANR_Y4LUT1);
        INIT_REG_INFO_ADDR(CAM_ANR_Y4LUT2);
        INIT_REG_INFO_ADDR(CAM_ANR_Y4LUT3);
		INIT_REG_INFO_ADDR(CAM_ANR_C4LUT1);
		INIT_REG_INFO_ADDR(CAM_ANR_C4LUT2);
		INIT_REG_INFO_ADDR(CAM_ANR_C4LUT3);
		INIT_REG_INFO_ADDR(CAM_ANR_A4LUT2);
		INIT_REG_INFO_ADDR(CAM_ANR_A4LUT3);
		INIT_REG_INFO_ADDR(CAM_ANR_L4LUT1);
		INIT_REG_INFO_ADDR(CAM_ANR_L4LUT2);
		INIT_REG_INFO_ADDR(CAM_ANR_L4LUT3);
        INIT_REG_INFO_ADDR(CAM_ANR_PTY);
        INIT_REG_INFO_ADDR(CAM_ANR_CAD);
        INIT_REG_INFO_ADDR(CAM_ANR_PTC);
        INIT_REG_INFO_ADDR(CAM_ANR_LCE);
		INIT_REG_INFO_ADDR(CAM_ANR_MED1);
		INIT_REG_INFO_ADDR(CAM_ANR_MED2);
		INIT_REG_INFO_ADDR(CAM_ANR_MED3);
		INIT_REG_INFO_ADDR(CAM_ANR_MED4);
        INIT_REG_INFO_ADDR(CAM_ANR_HP1);
        INIT_REG_INFO_ADDR(CAM_ANR_HP2);
        INIT_REG_INFO_ADDR(CAM_ANR_HP3);
		INIT_REG_INFO_ADDR(CAM_ANR_ACT1);
		INIT_REG_INFO_ADDR(CAM_ANR_ACT2);
		INIT_REG_INFO_ADDR(CAM_ANR_ACT3);
        INIT_REG_INFO_ADDR(CAM_ANR_ACTYH);
        INIT_REG_INFO_ADDR(CAM_ANR_ACTC);
        INIT_REG_INFO_ADDR(CAM_ANR_ACTYL);
        INIT_REG_INFO_ADDR(CAM_ANR_YLAD);
        INIT_REG_INFO_ADDR(CAM_ANR_PTYL);
        INIT_REG_INFO_ADDR(CAM_ANR_LCOEF);

        INIT_REG_INFO_ADDR(CAM_CCR_CON);   // CAM+0x0A80
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT);  // CAM+0x0A84
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT); // CAM+0x0A88
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT2); // CAM+0x0A8C
        INIT_REG_INFO_ADDR(CAM_CCR_SAT_CTRL); // CAM+0x0A90
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT_SP); // CAM+0x0A94
        INIT_REG_INFO_ADDR(CAM_CCR_HUE1);
        INIT_REG_INFO_ADDR(CAM_CCR_HUE2);
        INIT_REG_INFO_ADDR(CAM_CCR_HUE3);

        //ANR_TBL
        MUINT32 u4StartAddr = REG_ADDR(CAM_ANR_TBL[0]);
        for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
            m_rIspRegInfo_ANR_TBL[i].addr = u4StartAddr + 4*i;

        }
    }

    virtual ~ISP_MGR_NBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isANR_ENYEnable()
    {
        return m_bANR_ENYEnable;
    }

    MBOOL
    isANR_ENCEnable()
    {
        return m_bANR_ENCEnable;
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
    isCCTANREnable()
    {
        return m_bCCTANREnable;
    }

    MBOOL
    isANRTBLEnable()
    {
        return m_bANRTBLEnable;
    }

    MVOID
    setANREnable(MBOOL bEnable)
    {
         setANR_ENYEnable(bEnable);
         setANR_ENCEnable(bEnable);
    }

    MVOID
    setANR_ENYEnable(MBOOL bEnable)
    {
        m_bANR_ENYEnable = bEnable;
    }

    MVOID
    setANR_ENCEnable(MBOOL bEnable)
    {
        m_bANR_ENCEnable = bEnable;
    }

    MVOID
    setCCREnable(MBOOL bEnable)
    {
        m_bCCREnable = bEnable;
    }

    MVOID
    setCCTANREnable(MBOOL bEnable)
    {
         m_bCCTANREnable = bEnable;
    }

    MVOID
    setCCTCCREnable(MBOOL bEnable)
    {
        m_bCCTCCREnable = bEnable;
    }

    MVOID
    setANRTBLEnable(MBOOL bEnable)
    {
             m_bANRTBLEnable = bEnable;
    }

    MVOID
    putANR_TBL(ISP_NVRAM_ANR_LUT_T const& rParam)
    {

        if(m_bANRTBLEnable){
            for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
                m_rIspRegInfo_ANR_TBL[i].val = rParam.set[i];
            }
        }
        else{
            for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
                m_rIspRegInfo_ANR_TBL[i].val = 0;
            }
        }
    }

    MVOID
    putANR_LUT(ISP_NVRAM_ANR_LUT_T const& rParam)
    {

        if(m_bANRTBLEnable){
            for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
                m_rIspRegInfo_ANR_TBL[i].val = rParam.lut[i].val;
            }
        }
        else{
            for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
                m_rIspRegInfo_ANR_TBL[i].val = 0;
            }
        }
    }

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);


} ISP_MGR_NBC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NBC_DEV : public ISP_MGR_NBC_T
{
public:
    static
    ISP_MGR_NBC_T&
    getInstance()
    {
        static ISP_MGR_NBC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NBC_DEV()
        : ISP_MGR_NBC_T(eSensorDev)
    {}

    virtual ~ISP_MGR_NBC_DEV() {}

};


#endif

