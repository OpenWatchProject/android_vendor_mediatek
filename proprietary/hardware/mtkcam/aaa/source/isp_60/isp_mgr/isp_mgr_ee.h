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
#ifndef _ISP_MGR_EE_H_
#define _ISP_MGR_EE_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_EE_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(EEE_D1 ,reg, EE_D1A_EE_)

typedef class ISP_MGR_EE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_EE  MyType;
public:

    enum
    {
        EEE_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    //MBOOL m_bSEEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;
    enum
    {
        //ERegInfo_CTRL,
        ERegInfo_TOP_CTRL,
        ERegInfo_BLND_CTRL_1,
        ERegInfo_BLND_CTRL_2,
        ERegInfo_CORE_CTRL,
        ERegInfo_GN_CTRL_1,
        ERegInfo_LUMA_CTRL_1,
        ERegInfo_LUMA_CTRL_2,
        ERegInfo_LUMA_SLNK_CTRL,
        ERegInfo_GLUT_CTRL_1,
        ERegInfo_GLUT_CTRL_2,
        ERegInfo_GLUT_CTRL_3,
        ERegInfo_GLUT_CTRL_4,
        ERegInfo_GLUT_CTRL_5,
        ERegInfo_GLUT_CTRL_6,
        ERegInfo_ARTIFACT_CTRL,
        ERegInfo_CLIP_CTRL,
        ERegInfo_GN_CTRL_2,
        ERegInfo_ST_CTRL_1,
        ERegInfo_ST_CTRL_2,
        ERegInfo_CE_CTRL,
        ERegInfo_CE_SL_CTRL,
        ERegInfo_CBOOST_CTRL_1,
        ERegInfo_CBOOST_CTRL_2,
        ERegInfo_PBC1_CTRL_0,
        ERegInfo_PBC1_CTRL_1,
        ERegInfo_PBC1_CTRL_2,
        ERegInfo_PBC1_CTRL_3,
        ERegInfo_PBC2_CTRL_0,
        ERegInfo_PBC2_CTRL_1,
        ERegInfo_PBC2_CTRL_2,
        ERegInfo_PBC2_CTRL_3,
        ERegInfo_PBC3_CTRL_0,
        ERegInfo_PBC3_CTRL_1,
        ERegInfo_PBC3_CTRL_2,
        ERegInfo_PBC3_CTRL_3,
        //ERegInfo_SE_Y_SPECL_CTRL,
        //ERegInfo_SE_EDGE_CTRL_1,
        //ERegInfo_SE_EDGE_CTRL_2,
        //ERegInfo_SE_CORE_CTRL_1,
        //ERegInfo_SE_CORE_CTRL_2,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_EE(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            //m_bSEEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        //INIT_EE_ADDR(CTRL);
        INIT_EE_ADDR(TOP_CTRL);
        INIT_EE_ADDR(BLND_CTRL_1);
        INIT_EE_ADDR(BLND_CTRL_2);
        INIT_EE_ADDR(CORE_CTRL);
        INIT_EE_ADDR(GN_CTRL_1);
        INIT_EE_ADDR(LUMA_CTRL_1);
        INIT_EE_ADDR(LUMA_CTRL_2);
        INIT_EE_ADDR(LUMA_SLNK_CTRL);
        INIT_EE_ADDR(GLUT_CTRL_1);
        INIT_EE_ADDR(GLUT_CTRL_2);
        INIT_EE_ADDR(GLUT_CTRL_3);
        INIT_EE_ADDR(GLUT_CTRL_4);
        INIT_EE_ADDR(GLUT_CTRL_5);
        INIT_EE_ADDR(GLUT_CTRL_6);
        INIT_EE_ADDR(ARTIFACT_CTRL);
        INIT_EE_ADDR(CLIP_CTRL);
        INIT_EE_ADDR(GN_CTRL_2);
        INIT_EE_ADDR(ST_CTRL_1);
        INIT_EE_ADDR(ST_CTRL_2);
        INIT_EE_ADDR(CE_CTRL);
        INIT_EE_ADDR(CE_SL_CTRL);
        INIT_EE_ADDR(CBOOST_CTRL_1);
        INIT_EE_ADDR(CBOOST_CTRL_2);
        INIT_EE_ADDR(PBC1_CTRL_0);
        INIT_EE_ADDR(PBC1_CTRL_1);
        INIT_EE_ADDR(PBC1_CTRL_2);
        INIT_EE_ADDR(PBC1_CTRL_3);
        INIT_EE_ADDR(PBC2_CTRL_0);
        INIT_EE_ADDR(PBC2_CTRL_1);
        INIT_EE_ADDR(PBC2_CTRL_2);
        INIT_EE_ADDR(PBC2_CTRL_3);
        INIT_EE_ADDR(PBC3_CTRL_0);
        INIT_EE_ADDR(PBC3_CTRL_1);
        INIT_EE_ADDR(PBC3_CTRL_2);
        INIT_EE_ADDR(PBC3_CTRL_3);
        //INIT_EE_ADDR(SE_Y_SPECL_CTRL);
        //INIT_EE_ADDR(SE_EDGE_CTRL_1);
        //INIT_EE_ADDR(SE_EDGE_CTRL_2);
        //INIT_EE_ADDR(SE_CORE_CTRL_1);
        //INIT_EE_ADDR(SE_CORE_CTRL_2);

    }

    virtual ~ISP_MGR_EE() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

#if 0
    MBOOL
    isSEEnable(MUINT8 SubModuleIndex)
    {
        return m_bSEEnable[SubModuleIndex];
    }

    MVOID
    setSEEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bSEEnable[SubModuleIndex] = bEnable;
    }
#endif

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

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_EE_T;



#endif

