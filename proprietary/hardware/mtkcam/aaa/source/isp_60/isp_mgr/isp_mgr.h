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
#include <dip_reg.h>
#include <mfb_reg.h>
#include <drv/tuning_mgr.h>
//
#include <ispif.h>
#include <camera_custom_nvram.h>

#include "aaa/ae_param.h"
#include <aaa/shading_tuning_custom.h>

#include <isp_tuning/isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <private/aaa_hal_private.h>

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
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

//P2 Tuning Buffer Write
#define ISP_WRITE_ENABLE_BITS(RegBase, RegName, FieldName, Value)   \
    do {                                                            \
        (RegBase->RegName.Bits.FieldName) = (Value);                \
    } while (0)


#define ISP_WRITE_ENABLE_REG(RegBase, RegName, Value)   \
    do {                                                \
        (RegBase->RegName.Raw) = (Value);               \
    } while (0)

#define ISP_LCS_OUT_WD (510)
#define ISP_LCS_OUT_HT (510)

#define ISP_DCS_OUT_WD (128 * 20)  //*20 to workaround HW issue
#define ISP_DCS_OUT_HT (1)

#define MY_ASSERT(cond, ...) \
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, cond, "[%s] %s, check fail !", __FUNCTION__, #cond);

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BASE
{
protected:
    typedef ISP_MGR_BASE  MyType;
    typedef ISPREG_INFO_T RegInfo_T;

#define REG_ADDR_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

#define REG_ADDR_P2(reg)\
    ((MUINT32)offsetof(dip_x_reg_t, reg))

#define REG_ADDR_MFB(reg)\
    ((MUINT32)offsetof(mfb_reg_t, reg))

#define REG_INFO(reg)\
    (m_rIspRegInfo[ERegInfo_##reg])

#define REG_INFO_MULTI(index, reg)\
    (m_rIspRegInfo[index][ERegInfo_##reg])

#define REG_INFO_ADDR(reg)\
    (REG_INFO(reg).addr)

#define REG_INFO_ADDR_MULTI(index, reg)\
        (REG_INFO_MULTI(index,reg).addr)


#define REG_INFO_VALUE(reg)\
    (REG_INFO(reg).val)

#define REG_INFO_VALUE_MULTI(index, reg)\
    (REG_INFO_MULTI(index, reg).val)

#define REG_INFO_VALUE_PTR(reg)\
    (& REG_INFO_VALUE(reg))

#define REG_INFO_VALUE_PTR_MULTI(index,reg)\
    (& REG_INFO_VALUE_MULTI(index,reg))

#define INIT_REG_INFO_ADDR_P1(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_P1(reg)

#define INIT_REG_INFO_ADDR_P1_MULTI(index,reg, name)\
    REG_INFO_ADDR_MULTI(index,reg) = REG_ADDR_P1(name##reg)


#define INIT_REG_INFO_ADDR_P2(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_P2(reg)

#define INIT_REG_INFO_ADDR_P2_MULTI(index,reg, name)\
    REG_INFO_ADDR_MULTI(index,reg) = REG_ADDR_P2(name##reg)

#define INIT_REG_INFO_ADDR_MFB(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR_MFB(reg)

#define INIT_REG_INFO_ADDR_MFB_MULTI(index,reg, name)\
    REG_INFO_ADDR_MULTI(index,reg) = REG_ADDR_MFB(name##reg)

#define INIT_REG_INFO_VALUE(reg, val)\
    REG_INFO_VALUE(reg) = val

#define PUT_REG_INFO(dest, src)\
    REG_INFO_VALUE(dest) = rParam.src.val

#define PUT_REG_INFO_MULTI(index, dest, src)\
    REG_INFO_VALUE_MULTI(index, dest) = rParam.src.val

#define GET_REG_INFO(src, dest)\
    rParam.dest.val = REG_INFO_VALUE(src)

#define GET_REG_INFO_MULTI(index, src, dest)\
    rParam.dest.val = REG_INFO_VALUE_MULTI(index, src)

#define GET_REG_INFO_BUF(src, dest)\
    rParam.dest.val = pReg->src.Raw

protected:
    virtual ~ISP_MGR_BASE() {}
    ISP_MGR_BASE(MVOID*const pRegInfo, MUINT32 const u4RegInfoNum, MUINT32 const eSensorDev, MUINT32 u4SubModuleNUM =0)
     : m_bDebugEnable(MFALSE)
     , m_pRegInfo(pRegInfo)
     , m_u4RegInfoNum(u4RegInfoNum)
     , m_eSensorDev(eSensorDev)
     , m_u4SubModuleNum(u4SubModuleNUM)
    {
    }

#if 0
    switch (eSensorDev)\
    {\
        case ESensorDev_Main:\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_Main>::getInstance();\
        case ESensorDev_MainSecond:\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_MainSecond>::getInstance();\
        case ESensorDev_MainThird:\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_MainThird>::getInstance();\
        case ESensorDev_Sub:\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_Sub>::getInstance();\
        case ESensorDev_SubSecond:\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_SubSecond>::getInstance();\
        default:\
            CAM_LOGE("eSensorDev = %d", eSensorDev);\
            return  ISP_MGR_##module_name##_DEV<ESensorDev_Main>::getInstance();\
    }
#endif
#define ISP_MGR_MODULE_GET_INSTANCE(module_name)\
{\
        auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);\
        MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);\
        MY_ASSERT(i4SensorIdx >= 0);\
        MY_INST& rSingleton = gMultiton[i4SensorIdx];\
        std::call_once(rSingleton.onceFlag, [&] {\
            rSingleton.instance = std::make_unique<ISP_MGR_##module_name##_T>(eSensorDev);\
        } );\
        return *(rSingleton.instance);\
}
#define ISP_MGR_MODULE_GET_INSTANCE1(module_name)\
{\
        auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);\
        MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);\
        MY_ASSERT(i4SensorIdx >= 0);\
        MY_INST_CTL_EN_P1& rSingleton = gMultitonP1[i4SensorIdx];\
        std::call_once(rSingleton.onceFlag, [&] {\
            rSingleton.instance = std::make_unique<ISP_MGR_##module_name##_T>(eSensorDev);\
        } );\
        return *(rSingleton.instance);\
}
#define ISP_MGR_MODULE_GET_INSTANCE2(module_name)\
{\
        auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);\
        MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);\
        MY_ASSERT(i4SensorIdx >= 0);\
        MY_INST_CTL_EN_P2& rSingleton = gMultitonP2[i4SensorIdx];\
        std::call_once(rSingleton.onceFlag, [&] {\
            rSingleton.instance = std::make_unique<ISP_MGR_##module_name##_T>(eSensorDev);\
        } );\
        return *(rSingleton.instance);\
}

protected:
    MBOOL                  m_bDebugEnable;
    MVOID*const        m_pRegInfo;
    MUINT32 const      m_u4RegInfoNum;
    MUINT32 const      m_eSensorDev;
    MUINT32 const      m_u4SubModuleNum;

//==============================================================================
protected:

    inline
    MBOOL
    writeRegs2Exif(RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum, cam_reg_t* pReg, MUINT32 u4size)
    {
        if (pReg)
        {
            MUINT32 i;
            MUINT8* pRegStart = (MUINT8*)pReg;
            for (i = 0; i < u4RegInfoNum; i++)
            {
                if (pRegInfo[i].addr < u4size)
                    *((MUINT32*)(pRegStart + pRegInfo[i].addr)) = pRegInfo[i].val;
            }
        }
        return MFALSE;
    }

#if 0
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
#endif
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
    writeRegs(RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum, mfb_reg_t* pReg)
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
    MVOID
    dumpRegInfoP2(char const*const pModuleName, MUINT8 SubModuleIndex=0);

    virtual
    MVOID
    dumpRegInfoP1(char const*const pModuleName, MUINT8 SubModuleIndex=0);

} ISP_MGR_BASE_T;

#include "isp_mgr_ctl.h"

#include "isp_mgr_obc.h"
#include "isp_mgr_bpc.h"
#include "isp_mgr_dgn.h"
#include "isp_mgr_wb.h"
#include "isp_mgr_hlr.h"
#include "isp_mgr_ltm.h"
#include "isp_mgr_lce.h"
#include "isp_mgr_slk.h"
#include "isp_mgr_flc.h"
#include "isp_mgr_ggm.h"
#include "isp_mgr_g2c.h"
#include "isp_mgr_ndg.h"
#include "isp_mgr_ynr.h"
#include "isp_mgr_dce.h"
#include "isp_mgr_ee.h"
#include "isp_mgr_cnr.h"
#include "isp_mgr_ldnr.h"
#include "isp_mgr_nr3d.h"
#include "isp_mgr_color.h"
#include "isp_mgr_lpcnr.h"
#include "isp_mgr_ccm.h"
#include "isp_mgr_dm.h"
#include "isp_mgr_mfb.h"
#include "isp_mgr_mix.h"

#include "isp_mgr_mobc.h"
#include "isp_mgr_ltms.h"
#include "isp_mgr_ynrs.h"
#include "isp_mgr_c2g.h"
#include "isp_mgr_g2cx.h"
#include "isp_mgr_iggm.h"
#include "isp_mgr_lces.h"
#include "isp_mgr_dces.h"

//#include "isp_mgr_fus.h"
//#include "isp_mgr_zfus.h"


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
//  Data member: please follow the order of member initialization list in constructor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 m_i4SensorIdx;
    MINT32 m_i4SensorDev;
};

};  //  namespace NSIspTuning

#include "isp_mgr_lsc.h"

#endif // _ISP_MGR_H_

