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
    MBOOL m_bANR1_ENYEnable;
    MBOOL m_bANR1_ENCEnable;
    MBOOL m_bANR1_Ebable;
    MBOOL m_bCCTANR1Enable;
    MBOOL m_bANRTBLEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x0A20
    ISP_NBC_LTM_SW_PARAM_T m_SwAnrParam;
    MUINT16 m_SW_ANR_LTM_Algo_Ver;
    ISP_NVRAM_ANR_LUT_T m_ANR_TBL_Backup;

    enum
    {
        ERegInfo_DIP_X_NBC_ANR_CON1,
        ERegInfo_DIP_X_NBC_ANR_CON2,
        ERegInfo_DIP_X_NBC_ANR_YAD1,
        ERegInfo_DIP_X_NBC_ANR_YAD2,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT1,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT2,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT3,
        ERegInfo_DIP_X_NBC_ANR_C4LUT1,
        ERegInfo_DIP_X_NBC_ANR_C4LUT2,
        ERegInfo_DIP_X_NBC_ANR_C4LUT3,
        ERegInfo_DIP_X_NBC_ANR_A4LUT2,
        ERegInfo_DIP_X_NBC_ANR_A4LUT3,
        ERegInfo_DIP_X_NBC_ANR_L4LUT1,
        ERegInfo_DIP_X_NBC_ANR_L4LUT2,
        ERegInfo_DIP_X_NBC_ANR_L4LUT3,
        ERegInfo_DIP_X_NBC_ANR_PTY0V,
        ERegInfo_DIP_X_NBC_ANR_CAD,
        ERegInfo_DIP_X_NBC_ANR_PTY1V,
        ERegInfo_DIP_X_NBC_ANR_SL2,
        ERegInfo_DIP_X_NBC_ANR_PTY2V,
        ERegInfo_DIP_X_NBC_ANR_PTY3V,
        ERegInfo_DIP_X_NBC_ANR_PTY0H,
        ERegInfo_DIP_X_NBC_ANR_PTY1H,
        ERegInfo_DIP_X_NBC_ANR_PTY2H,
        ERegInfo_DIP_X_NBC_ANR_T4LUT1,
        ERegInfo_DIP_X_NBC_ANR_T4LUT2,
        ERegInfo_DIP_X_NBC_ANR_T4LUT3,
        ERegInfo_DIP_X_NBC_ANR_ACT1,
        ERegInfo_DIP_X_NBC_ANR_PTY3H,
        ERegInfo_DIP_X_NBC_ANR_PTCV,
        ERegInfo_DIP_X_NBC_ANR_ACT4,
        ERegInfo_DIP_X_NBC_ANR_PTCH,
        ERegInfo_DIP_X_NBC_ANR_YLVL0,
        ERegInfo_DIP_X_NBC_ANR_YLVL1,
        ERegInfo_DIP_X_NBC_ANR_HF_COR,
        ERegInfo_DIP_X_NBC_ANR_HF_ACT0,
        ERegInfo_DIP_X_NBC_ANR_HF_ACT1,
        ERegInfo_DIP_X_NBC_ANR_ACTC,
        ERegInfo_DIP_X_NBC_ANR_YLAD,
        ERegInfo_DIP_X_NBC_ANR_HF_ACT2,
        ERegInfo_DIP_X_NBC_ANR_HF_ACT3,
        ERegInfo_DIP_X_NBC_ANR_HF_LUMA0,
        ERegInfo_DIP_X_NBC_ANR_HF_LUMA1,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT4,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT5,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT6,
        ERegInfo_DIP_X_NBC_ANR_Y4LUT7,
        ERegInfo_DIP_X_NBC_ANR_A4LUT1,

        ERegInfo_DIP_X_NBC_ANR_LTM_GAIN1,
        ERegInfo_DIP_X_NBC_ANR_LTM_GAIN2,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTP1,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTP2,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTO1,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTO2,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTS1,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTS2,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTS3,
        ERegInfo_DIP_X_NBC_ANR_LTM_LUTS4,
        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_ANR_TBL[ANR_HW_TBL_NUM];

public:
    ISP_MGR_NBC(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bANR1_ENYEnable(MTRUE)
        , m_bANR1_ENCEnable(MTRUE)
        , m_bANR1_Ebable(MTRUE)
        , m_bCCTANR1Enable(MTRUE)
        , m_bANRTBLEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_NBC_ANR_CON1))
        , m_SW_ANR_LTM_Algo_Ver(0)
    {

        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_CON1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_CON2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_YAD1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_YAD2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_C4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_C4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_C4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_A4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_A4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_L4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_L4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_L4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY0V);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_CAD);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY1V);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_SL2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY2V);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY3V);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY0H);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY1H);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY2H);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_T4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_T4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_T4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_ACT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTY3H);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTCV);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_ACT4);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_PTCH);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_YLVL0);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_YLVL1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_COR);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_ACT0);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_ACT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_ACTC);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_YLAD);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_ACT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_ACT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_LUMA0);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_HF_LUMA1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT4);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT5);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT6);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_Y4LUT7);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_A4LUT1);

        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_GAIN1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_GAIN2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTP1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTP2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTO1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTO2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTS1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTS2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTS3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC_ANR_LTM_LUTS4);

        // CAM_GGM_RB_GMT
        MUINT32 u4StartAddr = REG_ADDR_P2(DIP_X_NBC_ANR_TBL[0]);
        for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
            m_rIspRegInfo_ANR_TBL[i].addr = u4StartAddr + 4*i;
        }

        memset(&m_SwAnrParam, 0, sizeof(ISP_NBC_LTM_SW_PARAM_T));
        memset(&m_ANR_TBL_Backup, 0, sizeof(ISP_NVRAM_ANR_LUT_T));
    }

    virtual ~ISP_MGR_NBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL
    isANR1Enable()
    {
        return m_bANR1_Ebable;
    }

    MBOOL
    isCCTANR1Enable()
    {
        return m_bCCTANR1Enable;
    }

    MBOOL
    isANRTBLEnable()
    {
        return m_bANRTBLEnable;
    }

    MVOID
    setANR1Enable(MBOOL bEnable)
    {
         m_bANR1_Ebable = bEnable;
    }

    MVOID
    setCCTANR1Enable(MBOOL bEnable)
    {
         m_bCCTANR1Enable = bEnable;
    }

    MVOID
    setANRTBLEnable(MBOOL bEnable)
    {
         m_bANRTBLEnable = bEnable;
    }

    inline MVOID
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

    inline MVOID
    putANR_TBL_Backup(ISP_NVRAM_ANR_LUT_T const& rParam)
    {

        if(m_bANRTBLEnable){
            m_ANR_TBL_Backup = rParam;
        }
        else{
            memset(&m_ANR_TBL_Backup, 0, sizeof(ISP_NVRAM_ANR_LUT_T));
        }
    }

    inline MVOID
    getANR_TBL(ISP_NVRAM_ANR_LUT_T& rParam, const dip_x_reg_t* pReg) const
    {
        if (pReg)
        {
            for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
                rParam.set[i] = pReg->DIP_X_NBC_ANR_TBL[i].Raw;
            }
        }
    }

    MVOID
    putSwParam(ISP_NBC_LTM_SW_PARAM_T const& rSwParam){

        m_SwAnrParam = rSwParam;

    }

    MVOID
    getSwParam(ISP_NBC_LTM_SW_PARAM_T & rSwParam, MUINT16& SwVer){

        rSwParam = m_SwAnrParam;
        SwVer = m_SW_ANR_LTM_Algo_Ver;

    }

    MBOOL apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg);

} ISP_MGR_NBC_T;


#endif

