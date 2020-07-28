/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#ifndef _I_AUTOMATION_MAIN_H_
#define _I_AUTOMATION_MAIN_H_

/////////////////////////////////////////////////////////////////////////
//! Typedefs
/////////////////////////////////////////////////////////////////////////
typedef unsigned char   MUINT8;
typedef unsigned short  MUINT16;
typedef unsigned int    MUINT32;
typedef signed char     MINT8;
typedef signed short    MINT16;
typedef signed int      MINT32;
typedef void            MVOID;
typedef int             MBOOL;

#include "cct_feature.h"


typedef struct
{
	DIP_X_REG_CTL_YUV_EN                IspReg_YuvEn;                                /* 0004, 0x15022004, DIP_A_CTL_YUV_EN */
	DIP_X_REG_CTL_YUV2_EN               IspReg_Yuv2En;                               /* 0008, 0x15022008, DIP_A_CTL_YUV2_EN */
	DIP_X_REG_CTL_RGB_EN                IspReg_RgbEn;                                /* 000C, 0x1502200C, DIP_A_CTL_RGB_EN */
	DIP_X_REG_CTL_DMA_EN                IspReg_DmaEn;                                /* 0010, 0x15022010, DIP_A_CTL_DMA_EN */    
} AUTOMATION_ISP_CTRL_SET;

typedef struct 
{
    ISP_NVRAM_PGN_T         sPGN;
    ISP_NVRAM_SL2_T         sSL2;
    ISP_NVRAM_UDM_T         sUDM;
    ISP_NVRAM_CCM_T         sCCM;     //G2G
//    ISP_NVRAM_CCM_CTL_T     sCCMCtl;  //G2G
    ISP_NVRAM_GGM_T         sGGM;
    ISP_NVRAM_DBS_T         sDBS;
    ISP_NVRAM_OBC_T         sOBC2;
    ISP_NVRAM_BNR_BPC_T     sBNRBpc;
    ISP_NVRAM_BNR_NR1_T     sBNRNr1;
    ISP_NVRAM_BNR_PDC_T     sBNRPdc;
    ISP_NVRAM_LSC_T         sLSC2;
    
    ISP_NVRAM_RNR_T         sRNR;
    ISP_NVRAM_SL2_T         sSL2G;
    ISP_NVRAM_G2C_T         sG2C;
//    ISP_NVRAM_G2C_SHADE_T   sG2CShade;
    ISP_NVRAM_ANR_T         sANR;     //NBC
    ISP_NVRAM_ANR_LUT_T     sANR_Tbl; //NBC ANR LUT
    ISP_NVRAM_ANR2_T        sANR2;    //NBC2
    ISP_NVRAM_CCR_T         sCCR;     //NBC2

    ISP_NVRAM_PCA_T         sPCA;
    MUINT32                 u4PCA_Lut[PCA_BIN_NUM*2];
    ISP_NVRAM_EE_T          sEE;      //SEEE
    ISP_NVRAM_SE_T          sSE;      //SEEE
} AUTOMATION_ISP_NVRAM_SET;

typedef struct
{
    MUINT32                 LscTable[6*1024];
} AUTOMATION_SHADING_TABLE;


typedef struct
{
    AUTOMATION_ISP_CTRL_SET         IspCtl;
    AUTOMATION_ISP_NVRAM_SET        IspNvram;
    AUTOMATION_SHADING_TABLE        IspLsc;
} AUTOMATION_ISP_PARAM_SET;



class IAutomationMain
{
public:
    static IAutomationMain* createInstance();
    virtual void destroyInstance() = 0;

    virtual MINT32 sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen
    ) = 0;
private:
    virtual MBOOL updateInfo(AUTOMATION_IMAGE_INFO* pImageInfo) = 0;
//    virtual MBOOL applyISPParams(AUTOMATION_NVRAM_ISP_PARAM* pIspParams) = 0;
    virtual MBOOL applyISPParams(AUTOMATION_ISP_PARAM_SET* pIspParams) = 0;
    virtual MBOOL run() = 0;

protected:  //    Ctor/Dtor.
                        IAutomationMain(){}
    virtual             ~IAutomationMain(){}

                        IAutomationMain(const IAutomationMain&);
                        IAutomationMain& operator=(const IAutomationMain&);
};

#endif // _I_PURE_TO_JPEG_H_
