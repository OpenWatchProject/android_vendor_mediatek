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
#define LOG_TAG "isp_cctop"
#define MTK_LOG_ENABLE 1
//
#include <utils/Errors.h>
#include <cutils/log.h>

#include "cct_feature.h"
#include "cct_main.h"
#include "cct_imp.h"

#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include <lsc/ILscMgr.h>
#include <ILscNvram.h>
#include <nvbuf_util.h>
#include <cutils/properties.h>


/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    ALOGD(fmt, ##arg)
//#define MY_ERR(fmt, arg...)    ALOGE("Err: %5d: " fmt, __LINE__, ##arg)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CUSTOM_NVRAM_REG_INDEX comes from isp_tuning_idx.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    UINT16  DBS;
    UINT16  OBC;
    UINT16  BNR_BPC;
    UINT16  BNR_NR1;
    UINT16  BNR_PDC;
    //UINT16  RMM;
    UINT16  RNR;
    UINT16  UDM;
    UINT16  ANR;
    UINT16  ANR2;
    UINT16  CCR;
    //UINT16  BOK;
    UINT16  HFG;
    UINT16  EE;
    UINT16  NR3D;
    UINT16  MFB;
    UINT16  MIXER3;

    UINT16  SL2F;
    UINT16  SL2J;
    UINT16  SL2;
    UINT16  CCM;
    UINT16  GGM;
    UINT16  IHDR_GGM;
    UINT16  COLOR;

    //UINT16 PCA;
    //UINT16 ANR_TBL;
} CUSTOM_NVRAM_REG_INDEX_T;


using namespace NSIspTuningv3;
using namespace NS3Av3;

#define FIXME 0
/*******************************************************************************
*
********************************************************************************/
//#define MY_LOG(fmt, arg...)    { XLOGD(fmt, ##arg);printf(fmt, ##arg);}
//#define MY_LOG(fmt, arg...)    XLOGD(fmt, ##arg)
//#define MY_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)


/*******************************************************************************
*
********************************************************************************/
MBOOL  CctCtrl::updateIspRegs(MUINT32 const /*u4Category*//*= 0xFFFFFFFF*/, MUINT32 const /*u4Index*//*= 0xFFFFFFFF*/)
{
#if 0
    MBOOL fgRet = MFALSE;

    MBOOL fgIsDynamicISPEnabled = false;
    MBOOL fgIsDynamicCCMEnabled = false;
    MBOOL fgDisableDynamic = false; //  Disable Dynamic Data.

    //  (1) Save all index.
    ISP_NVRAM_REG_INDEX_T BackupIdx = m_rISPRegsIdx;

    //  (2) Modify a specific index.
#define MY_SET_ISP_REG_INDEX(_category)\
    case EIspReg_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  MFALSE;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch (u4Category)
    {
    MY_SET_ISP_REG_INDEX(LSC);
    MY_SET_ISP_REG_INDEX(OBC);
    MY_SET_ISP_REG_INDEX(CFA);
    MY_SET_ISP_REG_INDEX(BPC);
    MY_SET_ISP_REG_INDEX(NR1);
    MY_SET_ISP_REG_INDEX(ANR);
    MY_SET_ISP_REG_INDEX(EE);
    //MY_SET_ISP_REG_INDEX(Saturation);
    //MY_SET_ISP_REG_INDEX(Contrast);
    //MY_SET_ISP_REG_INDEX(Hue);
    MY_SET_ISP_REG_INDEX(CCM);
    MY_SET_ISP_REG_INDEX(GGM);
    default:
        break;
    }

    //  (3) Save the current dynamic ISP flag.
    NSIspTuningv3::CmdArg_T cmd_GetDynamicISP;
    cmd_GetDynamicISP.eCmd        = NSIspTuningv3::ECmd_GetDynamicTuning;
    cmd_GetDynamicISP.pOutBuf     = &fgIsDynamicISPEnabled;
    cmd_GetDynamicISP.u4OutBufSize= sizeof(MBOOL);
    //  (4) Save the current dynamic CCM flag.
    NSIspTuningv3::CmdArg_T cmd_GetDynamicCCM;
    cmd_GetDynamicCCM.eCmd        = NSIspTuningv3::ECmd_GetDynamicCCM;
    cmd_GetDynamicCCM.pOutBuf     = &fgIsDynamicCCMEnabled;
    cmd_GetDynamicCCM.u4OutBufSize= sizeof(MBOOL);
    //  (5) Disable the dynamic ISP.
    NSIspTuningv3::CmdArg_T cmd_DisableDynamicISP;
    cmd_DisableDynamicISP.eCmd       = NSIspTuningv3::ECmd_SetDynamicTuning;
    cmd_DisableDynamicISP.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (6) Disable the dynamic CCM.
    NSIspTuningv3::CmdArg_T cmd_DisableDynamicCCM;
    cmd_DisableDynamicCCM.eCmd       = NSIspTuningv3::ECmd_SetDynamicCCM;
    cmd_DisableDynamicCCM.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicCCM.u4InBufSize= sizeof(MBOOL);
    //  (8) Restore the dynamic ISP flag.
    NSIspTuningv3::CmdArg_T cmd_RestoreDynamicISP;
    cmd_RestoreDynamicISP.eCmd       = NSIspTuningv3::ECmd_SetDynamicTuning;
    cmd_RestoreDynamicISP.pInBuf     = &fgIsDynamicISPEnabled;
    cmd_RestoreDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (9) Restore the dynamic CCM flag.
    NSIspTuningv3::CmdArg_T cmd_RestoreDynamicCCM;
    cmd_RestoreDynamicCCM.eCmd       = NSIspTuningv3::ECmd_SetDynamicCCM;
    cmd_RestoreDynamicCCM.pInBuf     = &fgIsDynamicCCMEnabled;
    cmd_RestoreDynamicCCM.u4InBufSize= sizeof(MBOOL);


    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicISP))     //(3)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicCCM))     //(4)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicISP)) //(5)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicCCM)) //(6)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_VALIDATE_FRAME, true)                                  //(7) Validate
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicISP)) //(8)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicCCM)) //(9)
        )
    {
        goto lbExit;
    }

    MY_LOG("dynamic flags:(isp, ccm)=(%d, %d)", fgIsDynamicISPEnabled, fgIsDynamicCCMEnabled);

    fgRet = MTRUE;

lbExit:
    //  (10) Restore all index.
    m_rISPRegsIdx = BackupIdx;
#endif

    return  MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPReadIspReg(MVOID * /*puParaIn*/, MVOID * /*puParaOut*/, MUINT32 * /*pu4RealParaOutLen*/)
{
#if 0
    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_READ_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = 0xFFFFFFFF;

    //if (err < 0)
    if (!err) //enter if false
  {
        MY_ERR("[CCTOPReadIspReg] readIspRegs() error");
        return err;
    }

    pIspRegInfoOut->RegData = isp_reg.Data;
    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);
    MY_LOG("[CCTOPReadIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    //return err;
    return CCTIF_NO_ERROR;
#endif
    MY_LOG("[CCTOPReadIspReg] not supported");
    return CCTIF_UNKNOWN_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPWriteIspReg(MVOID * /*puParaIn*/)
{
#if 0
    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_WRITE_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = pIspRegInfoIn->RegData;

    if (err < 0) {
        MY_ERR("[CCTOPWriteIspReg]writeRegs() error");
        return err;
    }

    MY_LOG("[CCTOPWriteIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    return err;
#endif
    MY_LOG("[CCTOPWriteIspReg] not supported");
    return CCTIF_UNKNOWN_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    if  ( sizeof(MUINT32) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT32 id = (MUINT32)PLATFORM_ID;
    MUINT32 ver = 0x00;
    if(CAM_SW_VERSION == 2) ver = 0xAA;

    // convert id from hex to decimal
    id = (id/1000) << 28 \
             | ((id%1000)/100) << 24 \
             | ((id%100)/10) << 20 \
             | ((id%10)) << 16 \
             | ver << 8;

    *reinterpret_cast<MUINT32 *>(puParaOut) = id; // 00 means 32bit; FF means 64bit
    *pu4RealParaOutLen = sizeof(MUINT32);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID ) done, PLATFORM_ID(%d), CAM_SW_VERSION(%d), 0x%08X\n", (MUINT32)PLATFORM_ID, (MUINT32)CAM_SW_VERSION, *reinterpret_cast<MUINT32 *>(puParaOut));
    return CCTIF_NO_ERROR;
}

#if CAM3_ISP_CCT_EN
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG )
{
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) pu4RealParaOutLen;
    (void) u4ParaOutLen;

    return CCTOPReadIspReg((MVOID *)puParaIn, (MVOID *)puParaOut, pu4RealParaOutLen);
}

IMP_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG )
{
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    return CCTOPWriteIspReg((MVOID *)puParaIn);
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;

#define MY_SET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch  (eCategory)
    {
        MY_SET_TUNING_INDEX(SL2F);
        MY_SET_TUNING_INDEX(DBS);
        MY_SET_TUNING_INDEX(OBC);
        MY_SET_TUNING_INDEX(BNR_BPC);
        MY_SET_TUNING_INDEX(BNR_NR1);
        MY_SET_TUNING_INDEX(BNR_PDC);
//        MY_SET_TUNING_INDEX(RMM);
        MY_SET_TUNING_INDEX(RNR);
        MY_SET_TUNING_INDEX(SL2);
        MY_SET_TUNING_INDEX(UDM);
        MY_SET_TUNING_INDEX(ANR);
        MY_SET_TUNING_INDEX(ANR2);
        MY_SET_TUNING_INDEX(CCR);
//        MY_SET_TUNING_INDEX(BOK);
        MY_SET_TUNING_INDEX(HFG);
        MY_SET_TUNING_INDEX(EE);
        MY_SET_TUNING_INDEX(NR3D);
        MY_SET_TUNING_INDEX(MFB);
        MY_SET_TUNING_INDEX(MIXER3);

        default:
            MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX ) Warning!! eCategory(%d) is not supported, u4Index=%d", eCategory, u4Index);
            break;//return  CCTIF_BAD_PARAM;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX ) done, (u4Index, eCategory) = (%d, %d)", u4Index, eCategory);
    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY   i_type;
    typedef MUINT32                     o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    o_type&      rIndex    = *reinterpret_cast<o_type*>(puParaOut);


#define MY_GET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        rIndex = m_rISPRegsIdx._category;\
        break

    switch  (eCategory)
    {
        MY_GET_TUNING_INDEX(SL2F);
        MY_GET_TUNING_INDEX(DBS);
        MY_GET_TUNING_INDEX(OBC);
        MY_GET_TUNING_INDEX(BNR_BPC);
        MY_GET_TUNING_INDEX(BNR_NR1);
        MY_GET_TUNING_INDEX(BNR_PDC);
//        MY_GET_TUNING_INDEX(RMM);
        MY_GET_TUNING_INDEX(RNR);
        MY_GET_TUNING_INDEX(SL2);
        MY_GET_TUNING_INDEX(UDM);
        MY_GET_TUNING_INDEX(ANR);
        MY_GET_TUNING_INDEX(ANR2);
        MY_GET_TUNING_INDEX(CCR);
//        MY_GET_TUNING_INDEX(BOK);
        MY_GET_TUNING_INDEX(HFG);
        MY_GET_TUNING_INDEX(EE);
        MY_GET_TUNING_INDEX(NR3D);
        MY_GET_TUNING_INDEX(MFB);
        MY_GET_TUNING_INDEX(MIXER3);

        default:
            MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX ) Warning!! eCategory(%d) is not supported, rIndex=%d", eCategory, rIndex);
            rIndex = -1;
            break;//return  CCTIF_BAD_PARAM;
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX ) done, (eCategory, rIndex) = (%d, %d)", eCategory, rIndex);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
#if 0
    typedef ISP_NVRAM_MIXER3_T o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MIXER3_T *rRegs = reinterpret_cast<o_type *>(puParaOut);

    ::memcpy(rRegs, (const void*)&m_rBuf_ISP.ISPMfbMixer, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);
#endif

    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
#if 0
    typedef ISP_NVRAM_MIXER3_T i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MIXER3_T *rRegs = reinterpret_cast<i_type *>(puParaIn);
    ::memcpy((void*)&m_rBuf_ISP.ISPMfbMixer, rRegs, sizeof(i_type));
#endif

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    typedef ISP_NVRAM_CCM_POLY22_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<o_type *>(puParaOut);

    //::memcpy(rRegs, &m_rBuf_ISP.ISPMulitCCM.Poly22, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF ) done\n");
    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ISP_NVRAM_CCM_POLY22_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<i_type *>(puParaIn);
    //::memcpy(&m_rBuf_ISP.ISPMulitCCM.Poly22, rRegs, sizeof(i_type));
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF ) done\n");
    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ACDK_CCT_ISP_GET_TUNING_PARAS
u4ParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    typedef ACDK_CCT_ISP_GET_TUNING_PARAS o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ACDK_CCT_ISP_NVRAM_REG& rRegs = reinterpret_cast<o_type*>(puParaOut)->stIspNvramRegs;

#define MY_GET_TUNING_PARAS(_category)\
    ::memcpy(rRegs._category, m_rISPRegs._category, sizeof(rRegs._category))

    MY_GET_TUNING_PARAS(SL2F);
    MY_GET_TUNING_PARAS(DBS);
    MY_GET_TUNING_PARAS(OBC);
    MY_GET_TUNING_PARAS(BNR_BPC);
    MY_GET_TUNING_PARAS(BNR_NR1);
    MY_GET_TUNING_PARAS(BNR_PDC);
    //MY_GET_TUNING_PARAS(RMM);
    MY_GET_TUNING_PARAS(RNR);
    MY_GET_TUNING_PARAS(SL2);
    MY_GET_TUNING_PARAS(UDM);
    MY_GET_TUNING_PARAS(ANR);
    MY_GET_TUNING_PARAS(ANR2);
    MY_GET_TUNING_PARAS(CCR);
    //MY_GET_TUNING_PARAS(BOK);
    MY_GET_TUNING_PARAS(HFG);
    MY_GET_TUNING_PARAS(EE);
    MY_GET_TUNING_PARAS(NR3D);
    MY_GET_TUNING_PARAS(MFB);
    MY_GET_TUNING_PARAS(MIXER3);

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS ) done\n");
    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_SET_TUNING_PARAS
u4ParaInLen
    sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_ISP_SET_TUNING_PARAS type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;
    ACDK_CCT_ISP_BUFFER_ACCESS_STRUCT const&   bufAccess       = reinterpret_cast<type const*>(puParaIn)->bufAccess;

#define CHECK_INDEX_RANGE(_category)\
    if (IspNvramRegMgr::NUM_##_category <= u4Index)\
        return CCTIF_BAD_PARAM;\
    m_rISPRegs._category[u4Index] = ((ISP_NVRAM_##_category##_T*)(bufAccess.pBuffer))[u4Index];\
    m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);



    switch(eCategory) {

        case EIsp_Category_SL2F:
            //CHECK_INDEX_RANGE(SL2F);
            if (IspNvramRegMgr::NUM_SL2F <= u4Index){
                return CCTIF_BAD_PARAM;
            }
            m_rISPRegs.SL2F[u4Index] = ((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index];
            m_rISPRegsIdx.SL2F = static_cast<MUINT8>(u4Index);
            ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_SL2G_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            break;

        case EIsp_Category_DBS:
            CHECK_INDEX_RANGE(DBS);
            ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_DBS_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_DBS2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_DBS_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            break;

        case EIsp_Category_OBC:
            CHECK_INDEX_RANGE(OBC);
            ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_OBC_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_OBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_OBC_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //Temp. Mark
            NSIspTuningv3::IspTuningMgr::getInstance().setPureOBCInfo((MINT32)m_eSensorEnum, &((ISP_NVRAM_OBC_T*)(bufAccess.pBuffer))[u4Index]);
            //NSIspTuningv3::ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_BNR_BPC:
            CHECK_INDEX_RANGE(BNR_BPC);
            ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_BPC_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_BPC_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_BNR_NR1:
            CHECK_INDEX_RANGE(BNR_NR1);
            ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_NR1_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_NR1_T*)(bufAccess.pBuffer))[u4Index]);
            //NSIspTuningv3::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_BNR_PDC:
            CHECK_INDEX_RANGE(BNR_PDC);
            ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_PDC_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BNR_PDC_T*)(bufAccess.pBuffer))[u4Index]);
            //NSIspTuningv3::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;
/*
        case EIsp_Category_RMM:
            CHECK_INDEX_RANGE(RMM);
            ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_RMM_T*)(bufAccess.pBuffer))[u4Index]);
            ISP_MGR_RMM2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_RMM_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;
*/

        case EIsp_Category_RNR:
            CHECK_INDEX_RANGE(RNR);
            ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_RNR_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_SL2:
            CHECK_INDEX_RANGE(SL2);
            ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index]);
//            ISP_MGR_SL2J_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_SL2_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;


        case EIsp_Category_UDM:
            CHECK_INDEX_RANGE(UDM);
            ISP_MGR_UDM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_UDM_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_ANR:
            CHECK_INDEX_RANGE(ANR);
            ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_ANR_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_ANR2:
            CHECK_INDEX_RANGE(ANR2);
            ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_ANR2_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_CCR:
            CHECK_INDEX_RANGE(CCR);
            ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_CCR_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;
/*
        case EIsp_Category_BOK:
            CHECK_INDEX_RANGE(BOK);
            ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_BOK_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;
*/
        case EIsp_Category_HFG:
            CHECK_INDEX_RANGE(HFG);
            ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_HFG_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_EE:
            CHECK_INDEX_RANGE(EE);
            ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_EE_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_NR3D:
            CHECK_INDEX_RANGE(NR3D);
            ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_NR3D_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_MFB:
            CHECK_INDEX_RANGE(MFB);
            //Temp. Mark
            ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_MFB_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        case EIsp_Category_MIXER3:
            CHECK_INDEX_RANGE(MIXER3);
            //Temp. Mark
            ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).put(((ISP_NVRAM_MIXER3_T*)(bufAccess.pBuffer))[u4Index]);
            //ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
            //NSIspTuningv3::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
            break;

        default:
            MY_ERR("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS ) eCategory(%d) is not supported, u4Index=%d\n", eCategory, u4Index);
            return  CCTIF_BAD_PARAM; // show error msg, no effect, don't return error

    }

    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS ) done, (u4Index, eCategory) = (%d, %d)\n", u4Index, eCategory);
    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    MINT32 bypass_en;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    bypass_en = NSIspTuningv3::IspTuningMgr::getInstance().getDynamicBypass((MINT32)m_eSensorEnum);
    if(bypass_en < 0)
    {
        return CCTIF_UNKNOWN_ERROR;
    }

    reinterpret_cast<o_type*>(puParaOut)->Enable = bypass_en;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF ) done, %d\n", bypass_en);
    return CCTIF_NO_ERROR;

}
#endif

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE )
{
    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    o_type*const pGammaAccessOut = reinterpret_cast<o_type*>(puParaOut);
    i_type*const pGammaAccessIn = reinterpret_cast<i_type*>(puParaIn);
    MUINT32 index;
    pGammaAccessOut->index = index = pGammaAccessIn->index;
    MBOOL bIHDR;
    pGammaAccessOut->bIHDR = bIHDR = pGammaAccessIn->bIHDR;
    //ISP_NVRAM_GGM_T* pGGM;
    //if (bIHDR) pGGM = &m_rISPToneMap.IHDR_GGM[index];
    //else pGGM = &m_rISPToneMap.GGM[index];

    int limit = 0;
    if (index >= (limit = bIHDR ? NVRAM_IHDR_GGM_TBL_NUM : NVRAM_GGM_TBL_NUM))
    {
        MY_ERR("index(%d) exceeds limit(%d), bIHDR(%d)\n", index, limit, bIHDR);
        return  CCTIF_BAD_PARAM;
    }
    MY_LOG("index(%d), limit(%d), bIHDR(%d) are checked ok\n", index, limit, bIHDR);

    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ggm);

    //for(int i=0;i<GAMMA_STEP_NO; i++) {
    //    pGammaAccessOut->gamma.r_tbl[i] = pGGM->lut.lut[i].GGM_R;//ggm.rb_gmt.lut[i].R_GAMMA;
    //    pGammaAccessOut->gamma.g_tbl[i] = pGGM->lut.lut[i].GGM_G;//ggm.g_gmt.lut[i].G_GAMMA;
    //    pGammaAccessOut->gamma.b_tbl[i] = pGGM->lut.lut[i].GGM_B;//ggm.rb_gmt.lut[i].B_GAMMA;
    //    MY_LOG("(r, g, b)[%d] = (%d, %d, %d)", i, pGammaAccessOut->gamma.r_tbl[i], pGammaAccessOut->gamma.g_tbl[i], pGammaAccessOut->gamma.b_tbl[i]);
    //}

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE ) done\n");

    return  CCTIF_NO_ERROR;

}
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MY_LOG("Enable Dynamic Bypass!!\n");

    if(NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MY_LOG("Disable Dynamic Bypass!!\n");

    if(NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}
/*
puParaIn
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_GAMMA_ACCESS_STRUCT i_type;
    int i;

    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pGammaAccess = reinterpret_cast<i_type*>(puParaIn);
    MUINT32 const setIndex = pGammaAccess->index;
    MBOOL bIHDR = pGammaAccess->bIHDR;
    if (bIHDR) m_rISPRegsIdx.IHDR_GGM = setIndex;
    else m_rISPRegsIdx.GGM = setIndex;
    ISP_NVRAM_GGM_T ggm;

    MY_LOG("[ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE] setIndex, bIHDR, m_rISPRegsIdx.IHDR_GGM, m_rISPRegsIdx.GGM = (%d, %d, %d, %d)\n"
        , setIndex, bIHDR, m_rISPRegsIdx.IHDR_GGM, m_rISPRegsIdx.GGM);


    for(i=0;i<GAMMA_STEP_NO; i++) {
/*
        m_rISPRegs.GGM[index].rb_gmt.lut[i].R_GAMMA = pGamma->r_tbl[i];
        m_rISPRegs.GGM[index].g_gmt.lut[i].G_GAMMA = pGamma->g_tbl[i];
        m_rISPRegs.GGM[index].rb_gmt.lut[i].B_GAMMA = pGamma->b_tbl[i];
*/
#if 0
        if (pGammaAccess->bIHDR)
        {
            ggm.lut.lut[i].GGM_R = m_rISPToneMap.IHDR_GGM[setIndex].lut.lut[i].GGM_R = pGammaAccess->gamma.r_tbl[i];
            ggm.lut.lut[i].GGM_G = m_rISPToneMap.IHDR_GGM[setIndex].lut.lut[i].GGM_G = pGammaAccess->gamma.g_tbl[i];
            ggm.lut.lut[i].GGM_B = m_rISPToneMap.IHDR_GGM[setIndex].lut.lut[i].GGM_B = pGammaAccess->gamma.b_tbl[i];
        }
        else
        {
            ggm.lut.lut[i].GGM_R = m_rISPToneMap.GGM[setIndex].lut.lut[i].GGM_R = pGammaAccess->gamma.r_tbl[i];
            ggm.lut.lut[i].GGM_G = m_rISPToneMap.GGM[setIndex].lut.lut[i].GGM_G = pGammaAccess->gamma.g_tbl[i];
            ggm.lut.lut[i].GGM_B = m_rISPToneMap.GGM[setIndex].lut.lut[i].GGM_B = pGammaAccess->gamma.b_tbl[i];
        }
#endif
        MY_LOG("(r, g, b)[%d] = (%d, %d, %d)", i, pGammaAccess->gamma.r_tbl[i], pGammaAccess->gamma.g_tbl[i], pGammaAccess->gamma.b_tbl[i]);
    }

    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(ggm);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //NSIspTuningv3::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE ) done\n");

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnable = reinterpret_cast<type*>(puParaIn)->Enable ? MFALSE : MTRUE;

    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgEnable);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS ) done, %d\n", fgEnable);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    reinterpret_cast<o_type*>(puParaOut)->Enable
        = (ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable()) ? MFALSE : MTRUE;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG ) done, %d\n", reinterpret_cast<o_type*>(puParaOut)->Enable);

    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/

#if CAM3_ISP_CCT_EN
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    MUINT32 u4Index
u4ParaInLen
    sizeof(MUINT32)
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef MUINT32             i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
#if 0
    i_type const index = *reinterpret_cast<i_type*>(puParaIn);
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    pDst->M11 = rSrc.cnv_1.bits.G2G_CNV_00;//conv0a.bits.G2G_CNV_00;
    pDst->M12 = rSrc.cnv_1.bits.G2G_CNV_01;//conv0a.bits.G2G_CNV_01;
    pDst->M13 = rSrc.cnv_2.bits.G2G_CNV_02;//conv0b.bits.G2G_CNV_02;
    pDst->M21 = rSrc.cnv_3.bits.G2G_CNV_10;//conv1a.bits.G2G_CNV_10;
    pDst->M22 = rSrc.cnv_3.bits.G2G_CNV_11;//conv1a.bits.G2G_CNV_11;
    pDst->M23 = rSrc.cnv_4.bits.G2G_CNV_12;//conv1b.bits.G2G_CNV_12;
    pDst->M31 = rSrc.cnv_5.bits.G2G_CNV_20;//conv2a.bits.G2G_CNV_20;
    pDst->M32 = rSrc.cnv_5.bits.G2G_CNV_21;//conv2a.bits.G2G_CNV_21;
    pDst->M33 = rSrc.cnv_6.bits.G2G_CNV_22;//conv2b.bits.G2G_CNV_22;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM ) done\n");
#endif
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_SET_NVRAM_CCM
u4ParaInLen
    sizeof(ACDK_CCT_SET_NVRAM_CCM)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_SET_NVRAM_CCM type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type& rInParam = *reinterpret_cast<type*>(puParaIn);
    MUINT32 const index = rInParam.u4Index;
#if 0
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];
/*
    rDst.conv0a.bits.G2G_CNV_00 = rInParam.ccm.M11;
    rDst.conv0a.bits.G2G_CNV_01 = rInParam.ccm.M12;
    rDst.conv0b.bits.G2G_CNV_02 = rInParam.ccm.M13;
    rDst.conv1a.bits.G2G_CNV_10 = rInParam.ccm.M21;
    rDst.conv1a.bits.G2G_CNV_11 = rInParam.ccm.M22;
    rDst.conv1b.bits.G2G_CNV_12 = rInParam.ccm.M23;
    rDst.conv2a.bits.G2G_CNV_20 = rInParam.ccm.M31;
    rDst.conv2a.bits.G2G_CNV_21 = rInParam.ccm.M32;
    rDst.conv2b.bits.G2G_CNV_22 = rInParam.ccm.M33;
*/
    rDst.cnv_1.bits.G2G_CNV_00 = rInParam.ccm.M11;
    rDst.cnv_1.bits.G2G_CNV_01 = rInParam.ccm.M12;
    rDst.cnv_2.bits.G2G_CNV_02 = rInParam.ccm.M13;
    rDst.cnv_3.bits.G2G_CNV_10 = rInParam.ccm.M21;
    rDst.cnv_3.bits.G2G_CNV_11 = rInParam.ccm.M22;
    rDst.cnv_4.bits.G2G_CNV_12 = rInParam.ccm.M23;
    rDst.cnv_5.bits.G2G_CNV_20 = rInParam.ccm.M31;
    rDst.cnv_5.bits.G2G_CNV_21 = rInParam.ccm.M32;
    rDst.cnv_6.bits.G2G_CNV_22 = rInParam.ccm.M33;

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to set: %d", index);
    for (MUINT32 i = 0; i < ISP_NVRAM_CCM_T::COUNT; i++)
    {
        MY_LOG("CCM: [%d] 0x%06X", i, rDst.set[i]);
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM ) done\n");
#endif
    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
pu4RealParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_NVRAM_CCM_PARA o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

#if 0

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        pDst->ccm[i].M11 = m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_00;//conv0a.bits.G2G_CNV_00;
        pDst->ccm[i].M12 = m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_01;//conv0a.bits.G2G_CNV_01;
        pDst->ccm[i].M13 = m_rISPRegs.CCM[i].cnv_2.bits.G2G_CNV_02;//conv0b.bits.G2G_CNV_02;
        pDst->ccm[i].M21 = m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_10;//conv1a.bits.G2G_CNV_10;
        pDst->ccm[i].M22 = m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_11;//conv1a.bits.G2G_CNV_11;
        pDst->ccm[i].M23 = m_rISPRegs.CCM[i].cnv_4.bits.G2G_CNV_12;//conv1b.bits.G2G_CNV_12;
        pDst->ccm[i].M31 = m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_20;//conv2a.bits.G2G_CNV_20;
        pDst->ccm[i].M32 = m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_21;//conv2a.bits.G2G_CNV_21;
        pDst->ccm[i].M33 = m_rISPRegs.CCM[i].cnv_6.bits.G2G_CNV_22;//conv2b.bits.G2G_CNV_22;
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA ) done\n");
#endif
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaInLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_NVRAM_CCM_PARA i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);

#if 0
    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
/*
        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_00 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_01 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].conv0b.bits.G2G_CNV_02 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_10 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_11 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].conv1b.bits.G2G_CNV_12 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_20 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_21 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].conv2b.bits.G2G_CNV_22 = pSrc->ccm[i].M33;
*/
        m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_00 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].cnv_1.bits.G2G_CNV_01 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].cnv_2.bits.G2G_CNV_02 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_10 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].cnv_3.bits.G2G_CNV_11 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].cnv_4.bits.G2G_CNV_12 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_20 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].cnv_5.bits.G2G_CNV_21 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].cnv_6.bits.G2G_CNV_22 = pSrc->ccm[i].M33;

    }
#endif
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA ) done\n");
    return  CCTIF_NO_ERROR;

}


/*
puParaIn
    ISP_NVRAM_CCM_AWB_GAIN_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_CCM_AWB_GAIN_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_WB )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

#if 0
    typedef ISP_NVRAM_CCM_AWB_GAIN_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);
#endif
#if 0
    m_rBuf_ISP.ISPMulitCCM.AWBGain = *pSrc;
    MY_LOG("pSrc->rA.i4R = %d, nvram buf = %d\n", pSrc->rA.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4R);
    MY_LOG("pSrc->rA.i4G = %d, nvram buf = %d\n", pSrc->rA.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4G);
    MY_LOG("pSrc->rA.i4B = %d, nvram buf = %d\n", pSrc->rA.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4B);
    MY_LOG("pSrc->rD65.i4R = %d, nvram buf = %d\n", pSrc->rD65.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4R);
    MY_LOG("pSrc->rD65.i4G = %d, nvram buf = %d\n", pSrc->rD65.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4G);
    MY_LOG("pSrc->rD65.i4B = %d, nvram buf = %d\n", pSrc->rD65.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4B);
#endif

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_WB ) done\n");
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ISP_NVRAM_CCM_AWB_GAIN_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_CCM_AWB_GAIN_STRUCT)
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_WB )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

#if 0
    typedef ISP_NVRAM_CCM_AWB_GAIN_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type* const pDst = reinterpret_cast<o_type*>(puParaOut);
#endif
#if 0
    *pDst = m_rBuf_ISP.ISPMulitCCM.AWBGain;
    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("pDst->rA.i4R = %d, nvram buf = %d\n", pDst->rA.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4R);
    MY_LOG("pDst->rA.i4G = %d, nvram buf = %d\n", pDst->rA.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4G);
    MY_LOG("pDst->rA.i4B = %d, nvram buf = %d\n", pDst->rA.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rA.i4B);
    MY_LOG("pDst->rD65.i4R = %d, nvram buf = %d\n", pDst->rD65.i4R, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4R);
    MY_LOG("pDst->rD65.i4G = %d, nvram buf = %d\n", pDst->rD65.i4G, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4G);
    MY_LOG("pDst->rD65.i4B = %d, nvram buf = %d\n", pDst->rD65.i4B, m_rBuf_ISP.ISPMulitCCM.AWBGain.rD65.i4B);
#endif

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_WB ) done\n");
    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    return  CCTIF_NO_ERROR;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(MUINT32);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef MUINT32 i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const u4Index = *reinterpret_cast<i_type const*>(puParaIn);

    if  ( IspNvramRegMgr::NUM_CCM <= u4Index )
    {
        return  CCTIF_BAD_PARAM;
    }

    m_rISPRegsIdx.CCM = static_cast<MUINT8>(u4Index);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE ) done, CCM Index: (old, new)=(%d, %d)", m_rISPRegsIdx.CCM, u4Index);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
u4ParaInLen
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef MUINT32 o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<o_type*>(puParaOut) = m_rISPRegsIdx.CCM;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("[ACDK_CCT_OP_GET_CCM_MODE] Current CCM Index: %d", m_rISPRegsIdx.CCM);
    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
#endif

IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);
    ISP_NVRAM_CCM_T ccm;

    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ccm);

    pDst->M11 = ccm.cnv_1.bits.G2G_CNV_00;//ccm.conv0a.bits.G2G_CNV_00;
    pDst->M12 = ccm.cnv_1.bits.G2G_CNV_01;//ccm.conv0a.bits.G2G_CNV_01;
    pDst->M13 = ccm.cnv_2.bits.G2G_CNV_02;//ccm.conv0b.bits.G2G_CNV_02;
    pDst->M21 = ccm.cnv_3.bits.G2G_CNV_10;//ccm.conv1a.bits.G2G_CNV_10;
    pDst->M22 = ccm.cnv_3.bits.G2G_CNV_11;//ccm.conv1a.bits.G2G_CNV_11;
    pDst->M23 = ccm.cnv_4.bits.G2G_CNV_12;//ccm.conv1b.bits.G2G_CNV_12;
    pDst->M31 = ccm.cnv_5.bits.G2G_CNV_20;//ccm.conv2a.bits.G2G_CNV_20;
    pDst->M32 = ccm.cnv_5.bits.G2G_CNV_21;//ccm.conv2a.bits.G2G_CNV_21;
    pDst->M33 = ccm.cnv_6.bits.G2G_CNV_22;//ccm.conv2b.bits.G2G_CNV_22;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM]\n");
    MY_LOG("M11 0x%03X\n", pDst->M11);
    MY_LOG("M12 0x%03X\n", pDst->M12);
    MY_LOG("M13 0x%03X\n", pDst->M13);
    MY_LOG("M21 0x%03X\n", pDst->M21);
    MY_LOG("M22 0x%03X\n", pDst->M22);
    MY_LOG("M23 0x%03X\n", pDst->M23);
    MY_LOG("M31 0x%03X\n", pDst->M31);
    MY_LOG("M32 0x%03X\n", pDst->M32);
    MY_LOG("M33 0x%03X\n", pDst->M33);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM ) done\n");

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_CCM_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_CCM_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_CCM_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM]\n");

    ISP_NVRAM_CCM_T rDst;
    type*const       pSrc = reinterpret_cast<type*>(puParaIn);
/*
    rDst.conv0a.bits.G2G_CNV_00 = pSrc->M11;
    rDst.conv0a.bits.G2G_CNV_01 = pSrc->M12;
    rDst.conv0b.bits.G2G_CNV_02 = pSrc->M13;
    rDst.conv1a.bits.G2G_CNV_10 = pSrc->M21;
    rDst.conv1a.bits.G2G_CNV_11 = pSrc->M22;
    rDst.conv1b.bits.G2G_CNV_12 = pSrc->M23;
    rDst.conv2a.bits.G2G_CNV_20 = pSrc->M31;
    rDst.conv2a.bits.G2G_CNV_21 = pSrc->M32;
    rDst.conv2b.bits.G2G_CNV_22 = pSrc->M33;
*/
    rDst.cnv_1.bits.G2G_CNV_00 = pSrc->M11;
    rDst.cnv_1.bits.G2G_CNV_01 = pSrc->M12;
    rDst.cnv_2.bits.G2G_CNV_02 = pSrc->M13;
    rDst.cnv_3.bits.G2G_CNV_10 = pSrc->M21;
    rDst.cnv_3.bits.G2G_CNV_11 = pSrc->M22;
    rDst.cnv_4.bits.G2G_CNV_12 = pSrc->M23;
    rDst.cnv_5.bits.G2G_CNV_20 = pSrc->M31;
    rDst.cnv_5.bits.G2G_CNV_21 = pSrc->M32;
    rDst.cnv_6.bits.G2G_CNV_22 = pSrc->M33;

    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rDst);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //NSIspTuningv3::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
  usleep(200000);
    MY_LOG("M11 0x%03X", pSrc->M11);
    MY_LOG("M12 0x%03X", pSrc->M12);
    MY_LOG("M13 0x%03X", pSrc->M13);
    MY_LOG("M21 0x%03X", pSrc->M21);
    MY_LOG("M22 0x%03X", pSrc->M22);
    MY_LOG("M23 0x%03X", pSrc->M23);
    MY_LOG("M31 0x%03X", pSrc->M31);
    MY_LOG("M32 0x%03X", pSrc->M32);
    MY_LOG("M33 0x%03X", pSrc->M33);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM ) done\n");
    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

#if 0
    MBOOL fgEnableDynamicCCM = true;
    NSIspTuningv3::CmdArg_T cmd = {
        eCmd:               NSIspTuningv3::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    MY_LOG("Enable Dynamic CCM!!\n");

    if(NSIspTuningv3::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MY_LOG("Disable Dynamic CCM!!\n");

    if(NSIspTuningv3::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    MINT32 en;
    CCTIF_ERROR_ENUM err_status;

    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS]\n");

    en = NSIspTuningv3::IspTuningMgr::getInstance().getDynamicCCM((MINT32)m_eSensorEnum);
    if(en < 0)
    {
        err_status = CCTIF_UNKNOWN_ERROR;
    }
    else {
        reinterpret_cast<o_type*>(puParaOut)->Enable = en;
        err_status = CCTIF_NO_ERROR;
    }

    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS ) done, getDynamicCCM() = %d\n", en);

    return err_status;

}

MVOID
CctCtrl::
dumpIspReg(MUINT32 const /*u4Addr*/) const
{
}

MVOID
CctCtrl::
setIspOnOff_SL2F(MBOOL const fgOn)
{
    ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_DBS(MBOOL const fgOn)
{
    ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    ISP_MGR_DBS2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_OBC(MBOOL const fgOn)
{
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    ISP_MGR_OBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_BNR_BPC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_BPC;

    //m_rISPRegs.BPC[u4Index].con.bits.BPC_ENABLE = fgOn;
    //m_rISPRegs.BNR_BPC[u4Index].con.bits.BPC_EN = fgOn; //definition change: CAM_BPC_CON CAM+0800H

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTBPCEnable(fgOn);
    ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTBPC2Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_BNR_NR1(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_NR1;

    //m_rISPRegs.BNR_NR1[u4Index].con.bits.NR1_CT_EN = fgOn;

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCTEnable(fgOn);
    ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCT2Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_BNR_PDC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_PDC;

    //m_rISPRegs.BNR_PDC[u4Index].con.bits.PDC_EN = fgOn;

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTPDCEnable(fgOn);
    ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTPDC2Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_RMM(MBOOL const fgOn)
{

    ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    ISP_MGR_RMM2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_HLR(MBOOL const /*fgOn*/)
{
#if 0
    ISP_MGR_HLR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    ISP_MGR_HLR2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
#endif
}


MVOID
CctCtrl::
setIspOnOff_RNR(MBOOL const fgOn)
{

    ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_SL2(MBOOL const fgOn)
{

    ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctCtrl::
setIspOnOff_UDM(MBOOL const fgOn)
{

    MUINT32 u4Index = m_rISPRegsIdx.UDM;

    ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_CCM(MBOOL const fgOn)
{
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_GGM(MBOOL const fgOn)
{
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_PCA(MBOOL const fgOn)
{
    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_COLOR(MBOOL const fgOn)
{
    ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTColorEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_ANR(MBOOL const fgOn)
{

    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTANR1Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_ANR2(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTANR2Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_CCR(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCCREnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_ABF(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTABFEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_BOK(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTBOKEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_HFG(MBOOL const fgOn)
{

    ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_EE(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;

// Choo
//    ISP_DIP_X_SEEE_SRK_CTRL_T &EECtrl_SRK = m_rISPRegs.EE[u4Index].srk_ctrl.bits;
//    ISP_DIP_X_SEEE_CLIP_CTRL_T &EECtrl_Clip = m_rISPRegs.EE[u4Index].clip_ctrl.bits;

//    EECtrl_Clip.SEEE_OVRSH_CLIP_EN = fgOn;//definition change: CAM_SEEE_CLIP_CTRL CAM+AA4H

    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEEEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctCtrl::
setIspOnOff_NR3D(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).setNr3dEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_MFB(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctCtrl::
setIspOnOff_MIXER3(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MBOOL
CctCtrl::
getIspOnOff_SL2F() const
{
    return ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_DBS() const
{
    return ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}


MBOOL
CctCtrl::
getIspOnOff_OBC() const
{
    return ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_BNR_BPC() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTBPCEnable();
}


MBOOL
CctCtrl::
getIspOnOff_BNR_NR1() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_BNR_PDC() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTPDCEnable();
}

MBOOL
CctCtrl::
getIspOnOff_RMM() const
{

    return  ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctCtrl::
getIspOnOff_HLR() const
{

    //return  ISP_MGR_HLR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
    return MFALSE;

}

MBOOL
CctCtrl::
getIspOnOff_RNR() const
{

    return  ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctCtrl::
getIspOnOff_SL2() const
{

    return  ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}


MBOOL
CctCtrl::
getIspOnOff_UDM() const
{

    return  ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctCtrl::
getIspOnOff_CCM() const
{
    return ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_GGM() const
{
    return  ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_PCA() const
{
    return  ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_COLOR() const
{
    return  ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTColorEnable();
}

MBOOL
CctCtrl::
getIspOnOff_ANR() const
{

        return (ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTANR1Enable());
}

MBOOL
CctCtrl::
getIspOnOff_ANR2() const
{

        return (ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTANR2Enable());
}


MBOOL
CctCtrl::
getIspOnOff_CCR() const
{

        return ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTCCREnable();

}

MBOOL
CctCtrl::
getIspOnOff_ABF() const
{

        return ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTABFEnable();

}

MBOOL
CctCtrl::
getIspOnOff_BOK() const
{

        return ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTBOKEnable();

}


MBOOL
CctCtrl::
getIspOnOff_HFG() const
{
    return ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_EE() const
{
    return ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEEEnable();
}

MBOOL
CctCtrl::
getIspOnOff_NR3D() const
{
    return ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).isNr3dEnable();
}

MBOOL
CctCtrl::
getIspOnOff_MFB() const
{
    //return MTRUE;
    //Temp. Mark
    return ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctCtrl::
getIspOnOff_MIXER3() const
{
    //return MTRUE;
    //Temp. Mark
    return ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MINT32
CctCtrl::
setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn)
{
#define SET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        setIspOnOff_##_category(fgOn);\
        MY_LOG("[setIspOnOff] < %s >", #_category);\
        break

    switch  ( u4Category )
    {
        SET_ISP_ON_OFF(SL2F);
        SET_ISP_ON_OFF(DBS);
        SET_ISP_ON_OFF(OBC);
        SET_ISP_ON_OFF(BNR_BPC);
        SET_ISP_ON_OFF(BNR_NR1);
        SET_ISP_ON_OFF(BNR_PDC);
        SET_ISP_ON_OFF(RMM);
        SET_ISP_ON_OFF(RNR);
        SET_ISP_ON_OFF(SL2);
        SET_ISP_ON_OFF(UDM);
        //SET_ISP_ON_OFF(CCM);
        SET_ISP_ON_OFF(ANR);
        SET_ISP_ON_OFF(ANR2);
        SET_ISP_ON_OFF(CCR);
        SET_ISP_ON_OFF(BOK);
        SET_ISP_ON_OFF(HFG);
        SET_ISP_ON_OFF(EE);
        SET_ISP_ON_OFF(NR3D);
        SET_ISP_ON_OFF(MFB);
        SET_ISP_ON_OFF(MIXER3);

        default:
            MY_ERR("[setIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOG("[%s] (u4Category, fgOn) = (%d, %d)", __FUNCTION__, u4Category, fgOn);
    return  CCTIF_NO_ERROR;
}


MINT32
CctCtrl::
getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const
{
#define GET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        MY_LOG("[getIspOnOff] < %s >", #_category);\
        rfgOn = getIspOnOff_##_category();\
        break

    switch  ( u4Category )
    {
        GET_ISP_ON_OFF(SL2F);
        GET_ISP_ON_OFF(DBS);
        GET_ISP_ON_OFF(OBC);
        GET_ISP_ON_OFF(BNR_BPC);
        GET_ISP_ON_OFF(BNR_NR1);
        GET_ISP_ON_OFF(BNR_PDC);
        GET_ISP_ON_OFF(RMM);
        GET_ISP_ON_OFF(RNR);
        GET_ISP_ON_OFF(SL2);
        GET_ISP_ON_OFF(UDM);
        //GET_ISP_ON_OFF(CCM);
        GET_ISP_ON_OFF(ANR);
        GET_ISP_ON_OFF(ANR2);
        GET_ISP_ON_OFF(CCR);
        GET_ISP_ON_OFF(BOK);
        GET_ISP_ON_OFF(HFG);
        GET_ISP_ON_OFF(EE);
        GET_ISP_ON_OFF(NR3D);
        GET_ISP_ON_OFF(MFB);
        GET_ISP_ON_OFF(MIXER3);

        default:
            MY_ERR("[getIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOG("[%s] (u4Category, rfgOn) = (%d, %d)", __FUNCTION__, u4Category, rfgOn);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 1);

    MY_LOG("[-ACDK_CCT_OP_SET_ISP_ON] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 0);

    MY_LOG("[ACDK_CCT_OP_SET_ISP_OFF] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY       i_type;
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    MBOOL&       rfgEnable = reinterpret_cast<o_type*>(puParaOut)->Enable;

    MINT32 const err = getIspOnOff(eCategory, rfgEnable);

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[-ACDK_CCT_OP_GET_ISP_ON_OFF] (eCategory, rfgEnable)=(%d, %d)", eCategory, rfgEnable);
    return  err;
}



/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )");

    void* pBuf; //temp used
    //int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_ISP read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    //
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM ) done");
    return  err;
}

IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MINT32 err = CCTIF_NO_ERROR;
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM )");

    void* pBuf; //temp used
    //int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_3A read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM ) done 3A");
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_eSensorEnum, (void*&)pBuf, 1);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_3A_LOAD_FROM_NVRAM] CAMERA_NVRAM_DATA_LENS read fail err=%d(0x%x)\n",err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM ) done LENS");
    return  err;
}



IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )");

    //int err;
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_ISP, m_eSensorEnum);
    if  ( 0 != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SAVE_TO_NVRAM] write fail err=%d(0x%x)\n", err, err);
        return  err;
    }
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM ) done");
    return  err;
}

#if 1
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
#if 0
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaIn);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;

    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_lo2[u4Offset];
        break;
    case 1:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_md2[u4Offset];
        break;
    case 2:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_hi2[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_fw, pBuf_pc, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    for (int i=0; i<(int)u4Count/10; i++)
    {
        MY_LOG("pBuf_fw[%d-%d] = (%x, %x, %x, %x, %x, %x, %x, %x, %x, %x)\n", i*10+0, i*10+9
            , pBuf_fw[i*10+0].val
            , pBuf_fw[i*10+1].val
            , pBuf_fw[i*10+2].val
            , pBuf_fw[i*10+3].val
            , pBuf_fw[i*10+4].val
            , pBuf_fw[i*10+5].val
            , pBuf_fw[i*10+6].val
            , pBuf_fw[i*10+7].val
            , pBuf_fw[i*10+8].val
            , pBuf_fw[i*10+9].val);
    }


    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).loadLut((MUINT32 *)pBuf_fw);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //NSIspTuningv3::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE ) done, (u4Offset, u4Count, u8ColorTemperature, u8Index) = (%d, %d, %d, %d)"
        , u4Offset, u4Count, u8ColorTemperature, pAccess->u8Index);
    return  CCTIF_NO_ERROR;
#else
    return CCTIF_UNKNOWN_ERROR;
#endif
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
#if 0
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaOut);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;

    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_lo[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_lo2[u4Offset];
        break;
    case 1:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_md[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_md2[u4Offset];
        break;
    case 2:
        pBuf_fw = (pAccess->u8Index == 0/*0: origianl PCA, 1 : HDR PCA */) ?
            &m_rISPPca.PCA_LUTS.lut_hi[u4Offset] : &m_rISPColorTbl.PCA_LUTS.lut_hi2[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_pc, pBuf_fw, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    for (int i=0; i<(int)u4Count/10; i++)
    {
        MY_LOG("pBuf_pc[%d-%d] = (%x, %x, %x, %x, %x, %x, %x, %x, %x, %x)\n", i*10+0, i*10+9
            , pBuf_pc[i*10+0].val
            , pBuf_pc[i*10+1].val
            , pBuf_pc[i*10+2].val
            , pBuf_pc[i*10+3].val
            , pBuf_pc[i*10+4].val
            , pBuf_pc[i*10+5].val
            , pBuf_pc[i*10+6].val
            , pBuf_pc[i*10+7].val
            , pBuf_pc[i*10+8].val
            , pBuf_pc[i*10+9].val);
    }

    *pu4RealParaOutLen = sizeof(type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE ) done, (u4Offset, u4Count, u8ColorTemperature, u8Index) = (%d, %d, %d, %d)"
        , u4Offset, u4Count, u8ColorTemperature, pAccess->u8Index);

    return  CCTIF_NO_ERROR;
#else
    return CCTIF_UNKNOWN_ERROR;
#endif
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_ACCESS_PCA_CONFIG i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pAccess = reinterpret_cast<i_type*>(puParaIn);

    setIspOnOff_PCA((MBOOL)pAccess->EN);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA ) done, %d", pAccess->EN);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_ACCESS_PCA_CONFIG o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pAccess = reinterpret_cast<o_type*>(puParaOut);

    pAccess->EN = getIspOnOff_PCA();
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA ) done, %d", pAccess->EN);

    return  CCTIF_NO_ERROR;

}

#endif

#if 0

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_SLIDER
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/

IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_SLIDER )
{

    typedef ACDK_CCT_ACCESS_PCA_SLIDER i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pca = reinterpret_cast<i_type*>(puParaIn);
    ::memcpy(&m_rISPPca.Slider, &pca->slider, sizeof(ACDK_CCT_ACCESS_PCA_SLIDER));
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_SLIDER ) done");

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_SLIDER
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_SLIDER)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_SLIDER )
{

    typedef ACDK_CCT_ACCESS_PCA_SLIDER o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pca = reinterpret_cast<o_type*>(puParaOut);
    ::memcpy(&pca->slider, &m_rISPPca.Slider, sizeof(ACDK_CCT_ACCESS_PCA_SLIDER));

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_SLIDER ) done");

    return  CCTIF_NO_ERROR;

}
#endif

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    MBOOL fgOnOff;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    if ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);

    fgOnOff = pShadingPara->Enable;
    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setOnOff(fgOnOff);
    pLscMgr->updateLsc();

    MY_LOG("[%s] SET_SHADING_ON_OFF(%s)", __FUNCTION__, (fgOnOff ? "On":"Off"));

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    MBOOL fgOnOff;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT o_type;
    if ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    fgOnOff = pLscMgr->getOnOff();
    pShadingPara->Enable = fgOnOff;

    MY_LOG("[%s] GET_SHADING_ON_OFF(%s)", __FUNCTION__, (fgOnOff ? "On":"Off"));

    return CCTIF_NO_ERROR;
}
/*******************************************************************************
* Because CCT tool is working on preview mode (Lsc_mgr.m_u4Mode = 0)
* 1.
*    Capture parameters will not be update to isp register
*    (since Lsc_mgr.m_u4Mode doesn't changed) till capture command
* 2.
*     Preivew parameters will be updated to reigster immediately at "prepareHw_PerFrame_Shading()"
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_SHADING_COMP_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);
    ESensorMode_T eSensorMode = (ESensorMode_T)pShadingPara->SHADING_MODE;

    MY_LOG("[%s +] SET_SHADING_PARA SensorMode(%d)", __FUNCTION__, eSensorMode);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
    ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();
    const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
    const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

    // only change grid number to NVRAM, do not let HW take effect immediately.
    pLscData->GridXNum = pShadingPara->pShadingComp->SHADINGBLK_XNUM + 1;
    pLscData->GridYNum = pShadingPara->pShadingComp->SHADINGBLK_YNUM + 1;
    pLscData->Width    = rCfg.i4ImgWd;
    pLscData->Height   = rCfg.i4ImgHt;

    // log nvram data
    MY_LOG("[%s -] SET_SHADING_PARA SensorMode(%d): Grid(%dx%d), WxH(%d,%d), BlockWidth(%d), BlockHeight(%d), LastBlockWidth(%d), LastBlockHeight(%d)", __FUNCTION__,
        eSensorMode, pLscData->GridXNum, pLscData->GridYNum, pLscData->Width, pLscData->Height,
        pShadingPara->pShadingComp->SHADINGBLK_WIDTH, pShadingPara->pShadingComp->SHADINGBLK_HEIGHT,
        pShadingPara->pShadingComp->SD_LWIDTH, pShadingPara->pShadingComp->SD_LHEIGHT);

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) pu4RealParaOutLen;

    MUINT8 *pCompMode=reinterpret_cast<MUINT8*>(puParaIn);
    MUINT32 u4Mode;
    typedef ACDK_CCT_SHADING_COMP_STRUCT o_type;
    if (! puParaIn || sizeof(o_type) != u4ParaOutLen  || ! puParaOut)
        return CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);
    ESensorMode_T eSensorMode = (ESensorMode_T)(*((UINT8*)puParaIn));

    MY_LOG("[%s +] GET_SHADING_PARA SensorMode(%d)", __FUNCTION__, eSensorMode);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    if (pLscMgr == NULL)
    {
        MY_ERR("GET_SHADING_PARA fail! NULL pLscMgr!");
        return CCTIF_BAD_PARAM;
    }
    const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
    if (pTbl == NULL)
    {
        MY_ERR("GET_SHADING_PARA fail to get table!");
        return CCTIF_BAD_PARAM;
    }
    const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

    pShadingPara->pShadingComp->SHADING_EN          = pLscMgr->getOnOff();
    pShadingPara->pShadingComp->SHADINGBLK_XNUM     = rCfg.rCfgBlk.i4BlkX+1;
    pShadingPara->pShadingComp->SHADINGBLK_YNUM     = rCfg.rCfgBlk.i4BlkY+1;
    pShadingPara->pShadingComp->SHADINGBLK_WIDTH    = rCfg.rCfgBlk.i4BlkW;
    pShadingPara->pShadingComp->SHADINGBLK_HEIGHT   = rCfg.rCfgBlk.i4BlkH;
    pShadingPara->pShadingComp->SHADING_RADDR       = 0;
    pShadingPara->pShadingComp->SD_LWIDTH           = rCfg.rCfgBlk.i4BlkLastW;
    pShadingPara->pShadingComp->SD_LHEIGHT          = rCfg.rCfgBlk.i4BlkLastH;
    pShadingPara->pShadingComp->SDBLK_RATIO00       = 32;
    pShadingPara->pShadingComp->SDBLK_RATIO01       = 32;
    pShadingPara->pShadingComp->SDBLK_RATIO10       = 32;
    pShadingPara->pShadingComp->SDBLK_RATIO11       = 32;

    // log nvram data
    MY_LOG("[%s -] GET_SHADING_PARA SensorMode(%d): (%dx%d, %d,%d,%d,%d)", __FUNCTION__, eSensorMode,
        rCfg.rCfgBlk.i4BlkX, rCfg.rCfgBlk.i4BlkY, rCfg.rCfgBlk.i4BlkW, rCfg.rCfgBlk.i4BlkH,
        rCfg.rCfgBlk.i4BlkLastW, rCfg.rCfgBlk.i4BlkLastH);

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX )
{
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    if ( ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 u4CCT = *reinterpret_cast<MUINT8*>(puParaIn);

    MY_LOG("[%s] SET_SHADING_INDEX(%d)", __FUNCTION__, u4CCT);

    IspTuningMgr::getInstance().enableDynamicShading(m_eSensorEnum, MFALSE);
    IspTuningMgr::getInstance().setIndex_Shading(m_eSensorEnum, u4CCT);

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    if  ( ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT8 *pShadingIndex = reinterpret_cast<MUINT8*>(puParaOut);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    *pShadingIndex = pLscMgr->getCTIdx();

    MY_LOG("[%s] GET_SHADING_INDEX(%d)", __FUNCTION__, *pShadingIndex);

    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    NSIspTuning::ESensorMode_T eLscScn = (NSIspTuning::ESensorMode_T)pShadingtabledata->Mode;
    MUINT8* pSrc = (MUINT8*)(pShadingtabledata->pBuffer);
    MUINT32 u4CtIdx = pShadingtabledata->ColorTemp;
    MUINT32 u4Size = pShadingtabledata->Length;

    MY_LOG("[%s +] SET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
        eLscScn, u4CtIdx, pSrc, u4Size);

    if (pSrc == NULL)
    {
        MY_ERR("SET_SHADING_TABLE: NULL pSrc");
        return CCTIF_BAD_PARAM;
    }

    if (u4CtIdx >= 4)
    {
        MY_ERR("SET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
        return CCTIF_BAD_PARAM;
    }

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
    const ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();

    // write to nvram buffer
    MUINT32* pDst = pLscNvram->getLut(ESensorMode_Capture, u4CtIdx);
    ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));
    // reset flow to validate ?
    pLscMgr->CCTOPReset();

    //NSIspTuning::ILscTbl rTbl(NSIspTuning::ILscTable::HWTBL);
    //rTbl.setConfig(pLscData->Width, pLscData->Height, pLscData->GridXNum, pLscData->GridYNum);
    //rTbl.setData(pSrc, u4Size*sizeof(MUINT32));
    //if (!pLscMgr->syncTbl(rTbl))
    //{
    //    MY_ERR("Fail to sync LUT to DRAM");
    //    return CCTIF_UNKNOWN_ERROR;
    //}
    MY_LOG("[%s -] SET_SHADING_TABLE", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    NSIspTuning::ESensorMode_T eLscScn = (NSIspTuning::ESensorMode_T)pShadingtabledata->Mode;
    MUINT8* pDst = (MUINT8*)(pShadingtabledata->pBuffer);
    MUINT32 u4CtIdx = pShadingtabledata->ColorTemp;
    MUINT32 u4Size = pShadingtabledata->Length;

    MY_LOG("[%s +] GET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
        eLscScn, u4CtIdx, pDst, u4Size);

    if (pDst == NULL)
    {
        MY_ERR("GET_SHADING_TABLE: NULL pDst");
        return CCTIF_BAD_PARAM;
    }

    if (u4CtIdx >= 4)
    {
        MY_ERR("GET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
        return CCTIF_BAD_PARAM;
    }

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
    const ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();

    // read from nvram buffer
    MUINT32* pSrc = pLscNvram->getLut(ESensorMode_Capture, u4CtIdx);
    ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));

    MY_LOG("[%s -] GET_SHADING_TABLE", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE )
{
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
    ACDK_CCT_FUNCTION_ENABLE_STRUCT*const cctEnable = (reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaIn));

    MY_LOG("[%s] + OnOff(%d)", __FUNCTION__, cctEnable->Enable);
//#warning "FIXME: LSC CCT"
    #if 0
    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setTsfForceAwb(cctEnable->Enable ? MTRUE : MFALSE);
    #endif
    MY_LOG("[%s] -", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSF_ONOFF )
{
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
    UINT32 u4OnOff = *(reinterpret_cast<UINT32*>(puParaIn));

    MY_LOG("[%s] + SET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    pLscMgr->setTsfOnOff(u4OnOff);

    MY_LOG("[%s] - SET_SHADING_TSF_ONOFF", __FUNCTION__);
    return CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TSF_ONOFF )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;
    UINT32 u4OnOff;

    MY_LOG("[%s] + GET_SHADING_TSF_ONOFF", __FUNCTION__);

    NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
    u4OnOff = pLscMgr->getTsfOnOff();
    *(reinterpret_cast<UINT32*>(puParaOut)) = u4OnOff;

    MY_LOG("[%s] - GET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);

    return CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA )
{
    (void) puParaIn;
    (void) u4ParaInLen;
#if 1
    typedef ACDK_CCT_NVRAM_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pCAMERA_NVRAM_DATA  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA]"
        "Mode is %d"
        , pCAMERA_NVRAM_DATA->Mode
        );


    MUINT8* p_cct_output_address = (MUINT8 *)(pCAMERA_NVRAM_DATA->pBuffer);

    switch (pCAMERA_NVRAM_DATA->Mode)
    {
        case CAMERA_NVRAM_DEFECT_STRUCT:
            break;
        case CAMERA_NVRAM_SHADING_STRUCT:
        {
            ::memcpy(p_cct_output_address, &m_rBuf_SD.Shading, sizeof(ISP_SHADING_STRUCT));
            *pu4RealParaOutLen = sizeof(ISP_SHADING_STRUCT);
            //MY_LOG("NVRAM Data :%d\n", m_rBuf_SD.Shading.PrvTable[0][0]);
        }
            break;
        case CAMERA_NVRAM_3A_STRUCT:
        {
            ::memcpy(p_cct_output_address, &m_rBuf_3A, sizeof(NVRAM_CAMERA_3A_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_CAMERA_3A_STRUCT);
      MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
      //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
      MY_LOG("*pu4RealParaOutLen = %d\n",  *pu4RealParaOutLen);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4R = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4R);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4G = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4G);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4B = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rUnitGain.i4B);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4R = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4R);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4G = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4G);
      //MY_LOG("cal data: m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4B = %d\n",  m_rBuf_3A.rAWBNVRAM.rCalData.rD65Gain.i4B);

      MY_LOG("cal data: m_rBuf_3A.AE.rDevicesInfo.u4MinGain = %d\n",  m_rBuf_3A.AE[0].rDevicesInfo.u4MinGain);
      MY_LOG("cal data: m_rBuf_3A.AE.rDevicesInfo.u4MaxGain = %d\n",  m_rBuf_3A.AE[0].rDevicesInfo.u4MaxGain);
      MY_LOG("cal data: m_rBuf_3A.AE.rDevicesInfo.u4MiniISOGain = %d\n",  m_rBuf_3A.AE[0].rDevicesInfo.u4MiniISOGain);
      MY_LOG("cal data: m_rBuf_3A.AE.rDevicesInfo.u4PreExpUnit = %d\n",  m_rBuf_3A.AE[0].rDevicesInfo.u4PreExpUnit);

        }
            break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT:
        {
            ::memcpy(p_cct_output_address, &m_rBuf_ISP, sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT);

#if 0
//1019 choo
      MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
      //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
      MY_LOG("*pu4RealParaOutLen = %d\n",  *pu4RealParaOutLen);
      MY_LOG("ISP reg: m_rBuf_ISP.SensorId = %d\n",  m_rBuf_ISP.SensorId);
      MY_LOG("ISP reg: m_rBuf_ISP.Version = %d\n",  m_rBuf_ISP.Version);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[0].offst0.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[0].offst0.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[1].offst1.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[1].offst1.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[2].gain0.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[2].gain0.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.OBC[3].gain1.val = %d\n",  m_rBuf_ISP.ISPRegs.OBC[3].gain1.val);

      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[0].con1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[0].con1.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[10].yad1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[10].yad1.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[20].lut1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[20].lut1.val);
      //MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[30].lce1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[30].lce1.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.ANR[40].hp1.val = %d\n",  m_rBuf_ISP.ISPRegs.ANR[40].hp1.val);

      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[0].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[0].uvlut.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[3].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[3].uvlut.val);
      MY_LOG("ISP reg: m_rBuf_ISP.ISPRegs.CCR[5].uvlut.val = %d\n",  m_rBuf_ISP.ISPRegs.CCR[5].uvlut.val);
#endif
        }
            break;
    case CAMERA_NVRAM_LENS_STRUCT:
        {
//      MINT32 err = CCTIF_NO_ERROR;
//        err = m_rBufIf_LN.refresh(m_eSensorEnum, m_u4SensorID);
//        if  ( CCTIF_NO_ERROR != err )
//        {
//            MY_ERR("[CAMERA_NVRAM_LENS_STRUCT] m_rBufIf_LN.refresh() fail (0x%x)\n", err);
//            return  err;
//        }
            ::memcpy(p_cct_output_address, &m_rBuf_LN, sizeof(NVRAM_LENS_PARA_STRUCT));
            *pu4RealParaOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);
      MY_LOG("m_eSensorEnum(sensor type) = %d\n",  (MUINT32)m_eSensorEnum);
      //MY_LOG("m_u4SensorID = %d\n",  (MUINT32)m_u4SensorID);
      //MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4Offset = %d\n",    m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4Offset);
      //MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum = %d\n",   m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum);
      //MY_LOG("m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum = %d\n",    m_rBuf_LN.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);

        }
      break;
        default:
            MY_LOG("[Get Camera NVRAM data]"
                "Not support NVRAM structure\n");
            return CCTIF_BAD_PARAM;
            break;
    }


    return  CCTIF_NO_ERROR;
#else
    return  CCTIF_NO_ERROR;
#endif

}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);

    if (pLscNvram->readNvramTbl(MTRUE))
    {
        //pLscMgr->loadLutToSysram();
        MY_LOG("[%s] SDTBL_LOAD_FROM_NVRAM OK", __FUNCTION__);
        return CCTIF_NO_ERROR;
    }
    else
    {
        MY_ERR("SDTBL_LOAD_FROM_NVRAM fail");
        return CCTIF_UNKNOWN_ERROR;
    }
}


IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )
{
    (void) puParaIn;
    (void) u4ParaInLen;
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);

    if (pLscNvram->writeNvramTbl())
    {
        MY_LOG("[%s] SDTBL_SAVE_TO_NVRAM OK", __FUNCTION__);
        return CCTIF_NO_ERROR;
    }
    else
    {
        MY_ERR("SDTBL_SAVE_TO_NVRAM fail");
        return CCTIF_UNKNOWN_ERROR;
    }
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH )
{
    (void) puParaIn;
    (void) u4ParaInLen;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;
#if MTK_CAM_NEW_NVRAM_SUPPORT
#else
    reinterpret_cast<o_type*>(puParaOut)->Enable = m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method);
#endif
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT //1=Smooth CCM; 0=Dynamic CCM
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH )
{
    (void) puParaOut;
    (void) u4ParaOutLen;
    (void) pu4RealParaOutLen;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

#if MTK_CAM_NEW_NVRAM_SUPPORT
#else
    m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method = reinterpret_cast<i_type*>(puParaIn)->Enable ? 1 : 0;
    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method);
#endif
    return  CCTIF_NO_ERROR;

}

