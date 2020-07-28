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
#ifndef _ISP_MGR_H_
#define _ISP_MGR_H_

#include <stddef.h>
#include <utils/threads.h>
//
#include <drv/isp_reg.h>
#include <drv/tuning_mgr.h>
//
#include <ispif.h>
#include <camera_custom_nvram.h>
//
#include <aaa/awb_param.h>
#include <aaa/ae_param.h>
#include <aaa/pd_buf_common.h>
#include <aaa/flicker_param.h>
#include <aaa/shading_tuning_custom.h>

#include <isp_tuning/isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <private/aaa_hal_private.h>
#include <mtkcam/utils/std/ULog.h>

namespace NS3Av3
{
/**
 * @brief ISP register info
 */
typedef struct ISPREG_INFO
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
    ISPREG_INFO()
        :addr(0),val(0)
    {}
} ISPREG_INFO_T;
};


using namespace android;
using namespace NS3Av3;

#define ISP_WRITE_ENABLE_BITS(RegBase, RegName, FieldName, Value)   \
    do {                                                            \
        (RegBase->RegName.Bits.FieldName) = (Value);                \
    } while (0)


#define ISP_WRITE_ENABLE_REG(RegBase, RegName, Value)   \
    do {                                                \
        (RegBase->RegName.Raw) = (Value);               \
    } while (0)

#define ISP_LCS_OUT_WD (384)
#define ISP_LCS_OUT_HT (384)

#define MY_ASSERT(cond, ...) \
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, cond, "[%s] %s, check fail !", __FUNCTION__, #cond);

namespace NSIspTuningv3
{

MVOID convertCcmFormat(const ISP_NVRAM_CCM_T& rInCCM, MINT32* pOutCCM);
MVOID convertCcmFormat(const MINT32* pInCCM, ISP_NVRAM_CCM_T& rOutCCM);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BASE
{
public:
    static MVOID setPerFramePropEnable(MBOOL bEnable);

protected:
    typedef ISP_MGR_BASE  MyType;
    typedef ISPREG_INFO_T RegInfo_T;

#define REG_ADDR_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

#define REG_ADDR_P2(reg)\
    ((MUINT32)offsetof(dip_x_reg_t, reg))

#define REG_ADDR_P1_UNI(reg)\
    ((MUINT32)offsetof(cam_uni_reg_t, reg))

#define REG_INFO(reg)\
    (m_rIspRegInfo[ERegInfo_##reg])

#define REG_INFO_ADDR(reg)\
    (REG_INFO(reg).addr)

#define REG_INFO_VALUE(reg)\
    (REG_INFO(reg).val)

#define REG_INFO_VALUE_PTR(reg)\
    (& REG_INFO_VALUE(reg))

#define INIT_REG_INFO_ADDR_P1(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_P1(reg)

#define INIT_REG_INFO_ADDR_P2(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_P2(reg)

#define INIT_REG_INFO_ADDR_P1_UNI(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_P1_UNI(reg)

#define INIT_REG_INFO_VALUE(reg, val)\
    REG_INFO_VALUE(reg) = val

#define PUT_REG_INFO(dest, src)\
    REG_INFO_VALUE(dest) = setbits(REG_INFO_VALUE(dest), rParam.src)

#define GET_REG_INFO(src, dest)\
    rParam.dest.val = REG_INFO_VALUE(src)

#define GET_REG_INFO_BUF(src, dest)\
    rParam.dest.val = pReg->src.Raw

protected:
    virtual ~ISP_MGR_BASE() {}
    ISP_MGR_BASE(MVOID*const pRegInfo, MUINT32 const u4RegInfoNum, MUINT32& u4StartAddr, ESensorDev_T const eSensorDev)
     : m_bDebugEnable(MFALSE)
     , m_pRegInfo(pRegInfo)
     , m_u4RegInfoNum(u4RegInfoNum)
     , m_u4StartAddr(u4StartAddr)
     , m_eSensorDev(eSensorDev)
    {
    }

protected:
    static MBOOL       s_bPerFramePropEnable;
    MBOOL              m_bDebugEnable;
    MVOID*const        m_pRegInfo;
    MUINT32 const      m_u4RegInfoNum;
    MUINT32&           m_u4StartAddr;
    ESensorDev_T const m_eSensorDev;

//==============================================================================
protected:
    template <class _ISP_XXX_T>
    inline
    MUINT32
    setbits(MUINT32 const /*dest*/, _ISP_XXX_T const src)
    {
        //MUINT32 const u4Mask = 0xFFFFFFFF;
        //  (1) clear bits + (2) set bits
        //return  ((dest & ~u4Mask) | (src.val & u4Mask));
        return src.val ;
    }

    inline
    MBOOL
    writeRegs(RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum, cam_reg_t* pReg)
    {
        if (pReg)
        {
            MUINT32 i;
            MUINT8* pRegStart = (MUINT8*)pReg;
            for (i = 0; i < u4RegInfoNum; i++)
            {
                *((MUINT32*)(pRegStart + pRegInfo[i].addr)) = pRegInfo[i].val;
            }
        }
        return MFALSE;
    }

    inline
    MBOOL
    writeRegs(RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum, dip_x_reg_t* pReg)
    {
        if (pReg)
        {
            MUINT32 i;
            MUINT8* pRegStart = (MUINT8*)pReg;
            for (i = 0; i < u4RegInfoNum; i++)
            {
                *((MUINT32*)(pRegStart + pRegInfo[i].addr)) = pRegInfo[i].val;
            }
        }
        return MFALSE;
    }

    inline
    MBOOL
    IsDebugEnabled()
    {
         return m_bDebugEnable;
    }

public: // Interfaces
    virtual
    MBOOL
    reset()
    {
        //#warning "FIXME"
        #if 0
        addressErrorCheck("Before reset()");
        MBOOL err = readRegs(ISPDRV_MODE_ISP, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        addressErrorCheck("After reset()");
        return err;
        #else
        return MTRUE;
        #endif
    }

    virtual
    MVOID
    dumpRegInfo(char const*const pModuleName);

    virtual
    MVOID
    dumpRegInfoP1(char const*const pModuleName);


    virtual
    MVOID
    dumpRegs();

    virtual
    MVOID
    addressErrorCheck(char const*const ptestCastName);

} ISP_MGR_BASE_T;

#include "isp_mgr_ctl.h"
#include "isp_mgr_dbs.h"
#include "isp_mgr_obc.h"
#include "isp_mgr_bnr.h"
#include "isp_mgr_rpg.h"
#include "isp_mgr_pgn.h"
#include "isp_mgr_flc.h"
#include "isp_mgr_udm.h"
#include "isp_mgr_ccm.h"
#include "isp_mgr_ggm.h"
#include "isp_mgr_g2c.h"
#include "isp_mgr_nbc.h"
#include "isp_mgr_nbc2.h"
#include "isp_mgr_pca.h"
#include "isp_mgr_seee.h"
#include "isp_mgr_nr3d.h"
#include "isp_mgr_mfb.h"
#include "isp_mgr_mixer3.h"
#include "isp_mgr_awb_stat.h"

#include "isp_mgr_sl2.h"
#include "isp_mgr_sl2_sensor.h"
#include "isp_mgr_flk.h"
#include "isp_mgr_ae_stat.h"

#include "isp_mgr_rmm.h"
#include "isp_mgr_rmg.h"
#include "isp_mgr_lce.h"

#include "isp_mgr_rnr.h"
#include "isp_mgr_hfg.h"
#include "isp_mgr_ndg.h"
#include "isp_mgr_ndg2.h"


#include "isp_mgr_abf.h"

class IspDebug
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IspDebug(IspDebug const&);
    //  Copy-assignment operator is disallowed.
    IspDebug& operator=(IspDebug const&);

public:  ////
    IspDebug(MINT32 i4SensorIdx, MINT32 i4SensorDev);
    ~IspDebug() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL dumpRegs(MUINT32 u4MagicNum);
    #if 0
    MBOOL dumpIspDebugMessage();
    MVOID dumpIspDebugMessage_DMA();
    MVOID dumpIspDebugMessage_AWB();
    MVOID dumpIspDebugMessage_OBC();
    MVOID dumpIspDebugMessage_LSC();
    MVOID dumpIspDebugMessage_BNR();
    MVOID dumpIspDebugMessage_RPG();
    MVOID dumpIspDebugMessage_PGN();
    MVOID dumpIspDebugMessage_CFA();
    MVOID dumpIspDebugMessage_CCM();
    MVOID dumpIspDebugMessage_GGM();
    MVOID dumpIspDebugMessage_G2C();
    MVOID dumpIspDebugMessage_NBC();
    MVOID dumpIspDebugMessage_PCA();
    MVOID dumpIspDebugMessage_SEEE();
    MVOID dumpIspDebugMessage_NR3D();
    MVOID dumpIspDebugMessage_AE();
    MVOID dumpIspDebugMessage_MFB();
    MVOID dumpIspDebugMessage_MIX3();

    MUINT32 readLsciAddr(MUINT32 u4TgInfo);
    #endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member: please follow the order of member initialization list in constructor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 m_i4SensorIdx;
    MINT32 m_i4SensorDev;
};

};  //  namespace NSIspTuningv3

#include "isp_mgr_lsc.h"
//#include "isp_mgr_af_stat.h"
#include "isp_mgr_pdo.h"

#endif // _ISP_MGR_H_

