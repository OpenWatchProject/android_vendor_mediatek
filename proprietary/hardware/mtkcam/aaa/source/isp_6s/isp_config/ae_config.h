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
/**
 * @file ae_hw_config.h
 * @brief AE isp hardware config.
 */
#ifndef _AE_CONFIG_H_
#define _AE_CONFIG_H_
#include <isp_config/isp_ae_config.h>

/* AE HW configure*/
typedef struct
{
    MBOOL bEnableHDRYConfig;
    MBOOL bEnableOverCntConfig;
    MBOOL bEnableTSFConfig;
    MBOOL bDisablePixelHistConfig;
    MBOOL bEnableAEHSTDMAOut;
    MBOOL bEnableAELINEARStat;
    MUINT32 u4BlockNumW;
    MUINT32 u4BlockNumH;
    MUINT32 u4AEOverExpCntThr;
    MUINT32 u4AEOverExpCntShift;
    MUINT32 u4AEYCoefR;
    MUINT32 u4AEYCoefG;
    MUINT32 u4AEYCoefB;
    MUINT32 u4AAOmode;
    MINT32 i4R_IspAEPreGain2;
    MINT32 i4G_IspAEPreGain2;
    MINT32 i4B_IspAEPreGain2;
    MINT32 i4PSEXPMode;
    MINT32 i4PSHSTSepG;
    MINT32 i4PSCCUHSTEnd;
    eAETargetMODE eAETargetMode;
} AE_CONFIG_INFO_T;

typedef class ISP_AE_CONFIG
{
    typedef ISP_AE_CONFIG    MyType;
private:
    AWB_GAIN_T  m_rIspAEPreGain2; // 1x = 512
    MBOOL m_bEnableHDRYConfig;
    MBOOL m_bEnableOverCntConfig;
    MBOOL m_bEnableTSFConfig;
    MBOOL m_bDisablePixelHistConfig;
    MBOOL m_u4BlockNumW;
    MBOOL m_u4BlockNumH;
    MUINT32 m_u4AEOverExpCntThr;
    MUINT32 m_u4AEOverExpCntShift;
    MUINT32 m_u4AEYCoefR;
    MUINT32 m_u4AEYCoefG;
    MUINT32 m_u4AEYCoefB;
    MUINT32 m_u4AAOmode;
    MUINT32 m_bDebugEnable;
    MUINT32 m_u4ConfigNum;
    AERegInfo_T m_rAERegInfoPre[EAERegInfo_NUM];

protected:
    ISP_AE_CONFIG(MINT32 const eSensorDev)
    {
        m_rIspAEPreGain2.i4R = m_rIspAEPreGain2.i4G = m_rIspAEPreGain2.i4B = 512;
        m_bEnableHDRYConfig = 1;
        m_bEnableOverCntConfig = 1;
        m_bEnableTSFConfig = 1;
        m_bDisablePixelHistConfig = 0;
        m_u4BlockNumW = 120;
        m_u4BlockNumH = 90;
        m_u4AEOverExpCntThr = 255;
        m_u4AEOverExpCntShift = 0;
        m_u4AEYCoefR = 5;
        m_u4AEYCoefG = 9;
        m_u4AEYCoefB = 2;
        m_u4AAOmode = 0;
        m_bDebugEnable = 0;
        m_u4ConfigNum = 0;
    }

    virtual ~ISP_AE_CONFIG() {}

public:
    static MyType&  getInstance(MINT32 const eSensorDev);

public: //Interfaces
    //MVOID resetParam();
    MRESULT AEConfig(AE_STAT_PARAM_T *a_sAEConfig, AE_CONFIG_INFO_T *a_sInputAEInfo, AEResultConfig_T *p_sAERegInfo);
    MBOOL reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg, AE_STAT_PARAM_T & rAEStatConfig);
    MBOOL setAEconfigParam(MUINT32 u4NumW, MUINT32 u4NumH, MBOOL bHDR, MBOOL bOverCnt, MBOOL bTSF, MBOOL bPixel, MUINT32 u4overths, MUINT32 u4overshift, MUINT32 u4AAOmode);
    MBOOL setAEYCoefParam(MUINT32 u4AEYCoefR, MUINT32 u4AEYCoefG, MUINT32 u4AEYCoefB);

} ISP_AE_CONFIG_T;

template <MINT32 const eSensorDev>
class ISP_AE_CONFIG_DEV : public ISP_AE_CONFIG_T
{
public:
    static
    ISP_AE_CONFIG_T&
    getInstance()
    {
        static ISP_AE_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_AE_CONFIG_DEV()
        : ISP_AE_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_AE_CONFIG_DEV() {}

};

#endif // _AE_CONFIG_H_
