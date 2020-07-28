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
#ifndef _ISP_MGR_YNR_H_
#define _ISP_MGR_YNR_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// YNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_YNR_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(EYNR_D1, reg, YNR_D1A_YNR_);\

#define FACE_ALPHA_MAP_SIZE          (30)

typedef class ISP_MGR_YNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_YNR    MyType;
public:
    enum
    {
        EYNR_D1,
        ESubModule_NUM
    };
private:
    struct FaceAlphaMap_T
    {
        IImageBuffer* pFaceAlphaBuf;
        MUINT32   i4FrmId;
        std::string strName;

        FaceAlphaMap_T()
            : pFaceAlphaBuf(NULL)
            , i4FrmId(0)
            , strName("default")
        {}
    };
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bSkinEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;
    MUINT32 m_i4MapHead[ESubModule_NUM]; //point to head for use
    FaceAlphaMap_T m_AlphaMap[ESubModule_NUM][FACE_ALPHA_MAP_SIZE];
    ISP_NVRAM_YNR_LUT_T m_YNR_TBL[ESubModule_NUM];
    enum
    {
        ERegInfo_CON1,
        ERegInfo_YAD1,
        ERegInfo_YAD2,
        ERegInfo_Y4LUT1,
        ERegInfo_Y4LUT2,
        ERegInfo_Y4LUT3,
        ERegInfo_C4LUT1,
        ERegInfo_C4LUT2,
        ERegInfo_C4LUT3,
        ERegInfo_CATH,
        ERegInfo_SM_EDGE,
        ERegInfo_YNLM,
        ERegInfo_YIMP,
        ERegInfo_A4LUT2,
        ERegInfo_A4LUT3,
        ERegInfo_L4LUT1,
        ERegInfo_L4LUT2,
        ERegInfo_L4LUT3,
        ERegInfo_PTY0V,
        ERegInfo_CAD,
        ERegInfo_PTY1V,
        ERegInfo_SL2,
        ERegInfo_PTY2V,
        ERegInfo_PTY3V,
        ERegInfo_PTY0H,
        ERegInfo_PTY1H,
        ERegInfo_PTY2H,
        ERegInfo_T4LUT1,
        ERegInfo_T4LUT2,
        ERegInfo_T4LUT3,
        ERegInfo_ACT1,
        ERegInfo_PTY3H,
        ERegInfo_PTCV,
        ERegInfo_ACT4,
        ERegInfo_PTCH,
        ERegInfo_YLVL0,
        ERegInfo_YLVL1,
        ERegInfo_HF_COR,
        ERegInfo_HF_ACT0,
        ERegInfo_HF_ACT1,
        ERegInfo_ACTC,
        ERegInfo_YLAD,
        ERegInfo_HF_ACT2,
        ERegInfo_HF_ACT3,
        ERegInfo_HF_LUMA0,
        ERegInfo_HF_LUMA1,
        ERegInfo_LCE_GAIN1,
        ERegInfo_LCE_GAIN2,
        ERegInfo_LCE_LUTP1,
        ERegInfo_LCE_LUTP2,
        ERegInfo_LCE_LUTO1,
        ERegInfo_LCE_LUTO2,
        ERegInfo_LCE_LUTS1,
        ERegInfo_LCE_LUTS2,
        ERegInfo_LCE_LUTS3,
        ERegInfo_LCE_LUTS4,
        ERegInfo_Y4LUT4,
        ERegInfo_Y4LUT5,
        ERegInfo_Y4LUT6,
        ERegInfo_Y4LUT7,
        ERegInfo_A4LUT1,
        ERegInfo_SKIN_CON,
        ERegInfo_SKIN1_Y,
        ERegInfo_SKIN1_U,
        ERegInfo_SKIN1_V,
        ERegInfo_SKIN2_Y,
        ERegInfo_SKIN2_U,
        ERegInfo_SKIN2_V,
        ERegInfo_SKIN3_Y,
        ERegInfo_SKIN3_U,
        ERegInfo_SKIN3_V,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_TBL[ESubModule_NUM][YNR_TBL_NUM];

protected:
    ISP_MGR_YNR(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            m_bSkinEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
            m_i4MapHead[i] = 0;
        }
        INIT_YNR_ADDR(CON1);
        INIT_YNR_ADDR(YAD1);
        INIT_YNR_ADDR(YAD2);
        INIT_YNR_ADDR(Y4LUT1);
        INIT_YNR_ADDR(Y4LUT2);
        INIT_YNR_ADDR(Y4LUT3);
        INIT_YNR_ADDR(C4LUT1);
        INIT_YNR_ADDR(C4LUT2);
        INIT_YNR_ADDR(C4LUT3);
        INIT_YNR_ADDR(A4LUT2);
        INIT_YNR_ADDR(A4LUT3);
        INIT_YNR_ADDR(L4LUT1);
        INIT_YNR_ADDR(L4LUT2);
        INIT_YNR_ADDR(L4LUT3);
        INIT_YNR_ADDR(PTY0V);
        INIT_YNR_ADDR(CAD);
        INIT_YNR_ADDR(PTY1V);
        INIT_YNR_ADDR(SL2);
        INIT_YNR_ADDR(PTY2V);
        INIT_YNR_ADDR(PTY3V);
        INIT_YNR_ADDR(PTY0H);
        INIT_YNR_ADDR(PTY1H);
        INIT_YNR_ADDR(PTY2H);
        INIT_YNR_ADDR(T4LUT1);
        INIT_YNR_ADDR(T4LUT2);
        INIT_YNR_ADDR(T4LUT3);
        INIT_YNR_ADDR(ACT1);
        INIT_YNR_ADDR(PTY3H);
        INIT_YNR_ADDR(PTCV);
        INIT_YNR_ADDR(ACT4);
        INIT_YNR_ADDR(PTCH);
        INIT_YNR_ADDR(YLVL0);
        INIT_YNR_ADDR(YLVL1);
        INIT_YNR_ADDR(HF_COR);
        INIT_YNR_ADDR(HF_ACT0);
        INIT_YNR_ADDR(HF_ACT1);
        INIT_YNR_ADDR(ACTC);
        INIT_YNR_ADDR(YLAD);
        INIT_YNR_ADDR(HF_ACT2);
        INIT_YNR_ADDR(HF_ACT3);
        INIT_YNR_ADDR(HF_LUMA0);
        INIT_YNR_ADDR(HF_LUMA1);
        INIT_YNR_ADDR(LCE_GAIN1);
        INIT_YNR_ADDR(LCE_GAIN2);
        INIT_YNR_ADDR(LCE_LUTP1);
        INIT_YNR_ADDR(LCE_LUTP2);
        INIT_YNR_ADDR(LCE_LUTO1);
        INIT_YNR_ADDR(LCE_LUTO2);
        INIT_YNR_ADDR(LCE_LUTS1);
        INIT_YNR_ADDR(LCE_LUTS2);
        INIT_YNR_ADDR(LCE_LUTS3);
        INIT_YNR_ADDR(LCE_LUTS4);
        INIT_YNR_ADDR(Y4LUT4);
        INIT_YNR_ADDR(Y4LUT5);
        INIT_YNR_ADDR(Y4LUT6);
        INIT_YNR_ADDR(Y4LUT7);
        INIT_YNR_ADDR(A4LUT1);
        INIT_YNR_ADDR(SKIN_CON);
        INIT_YNR_ADDR(SKIN1_Y);
        INIT_YNR_ADDR(SKIN1_U);
        INIT_YNR_ADDR(SKIN1_V);
        INIT_YNR_ADDR(SKIN2_Y);
        INIT_YNR_ADDR(SKIN2_U);
        INIT_YNR_ADDR(SKIN2_V);
        INIT_YNR_ADDR(SKIN3_Y);
        INIT_YNR_ADDR(SKIN3_U);
        INIT_YNR_ADDR(SKIN3_V);
        INIT_YNR_ADDR(CATH);
        INIT_YNR_ADDR(SM_EDGE);
        INIT_YNR_ADDR(YNLM);
        INIT_YNR_ADDR(YIMP);

        MUINT32 u4StartAddr[ESubModule_NUM];
        u4StartAddr[0]= REG_ADDR_P2(YNR_D1A_YNR_TBL[0]);
        for(int i=0; i<ESubModule_NUM; i++){
            for (MINT32 j = 0; j < YNR_TBL_NUM; j++) {
                m_rIspRegInfo_TBL[i][j].val = 0 ;
                m_rIspRegInfo_TBL[i][j].addr = u4StartAddr[i] + 4*j;
            }
        }
    }

    virtual ~ISP_MGR_YNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public:

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            //CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

    MBOOL
    isSkinEnable(MUINT8 SubModuleIndex)
    {
        return m_bSkinEnable[SubModuleIndex];
    }

    MVOID
    setSkinEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bSkinEnable[SubModuleIndex] = bEnable;
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

    MVOID*    getAlphaMap(MUINT8 SubModuleIndex, MUINT32 i4FrmId){
        MUINT32 u4MapHead = m_i4MapHead[SubModuleIndex];
        if (m_AlphaMap[SubModuleIndex][u4MapHead].pFaceAlphaBuf != NULL){
            return m_AlphaMap[SubModuleIndex][u4MapHead].pFaceAlphaBuf;
        } else
            return NULL;
    }

    MVOID*    getAlphaMapForWrite(MUINT8 SubModuleIndex, MUINT32 i4FrmId){
        MUINT32 u4MapHead = (m_i4MapHead[SubModuleIndex]+1)%FACE_ALPHA_MAP_SIZE;
        if (m_AlphaMap[SubModuleIndex][u4MapHead].pFaceAlphaBuf != NULL){
            m_i4MapHead[SubModuleIndex] = u4MapHead;
            return m_AlphaMap[SubModuleIndex][u4MapHead].pFaceAlphaBuf;
        } else
            return NULL;
    }

    MBOOL createAlphaMap(MUINT8 SubModuleIndex, MUINT32 width, MUINT32 height);
    MVOID releaseAlphaMap();
    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_YNR_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_YNR_DEV : public ISP_MGR_YNR_T
{
public:
    static
    ISP_MGR_YNR_T&
    getInstance()
    {
        static ISP_MGR_YNR_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_YNR_DEV()
        : ISP_MGR_YNR_T(eSensorDev)
    {}

    virtual ~ISP_MGR_YNR_DEV() {}

};
#endif