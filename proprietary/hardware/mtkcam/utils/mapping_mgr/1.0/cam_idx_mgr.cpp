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

#if MTK_CAM_NEW_NVRAM_SUPPORT
#define LOG_TAG "MtkCam/MappingMgr"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <aee.h>
#include <cutils/properties.h>
#include <nvbuf_util.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include "MyUtils.h"
#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>
#include <mtkcam/utils/std/Trace.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

// *******************
// ******* LOG ((*****
// *******************
#define LOG_QUERY_BASIC     (0x1u << 0)
#define LOG_QUERY_FULL       (0x1u << 1)

#define IS_LOG_BASIC    (s_bDebugEnable & LOG_QUERY_BASIC)
#define IS_LOG_FULL      (s_bDebugEnable & LOG_QUERY_FULL)

#define IDX_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define IDX_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define IDX_LOGV_IF(cond, ...)       do { if ( (cond) ) { IDX_LOGV(__VA_ARGS__); } }while(0)
#define IDX_LOGD_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGD(__VA_ARGS__); } }while(0)
#define IDX_LOGI_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGI(__VA_ARGS__); } }while(0)
#define IDX_LOGW_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGW(__VA_ARGS__); } }while(0)
#define IDX_LOGE_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGE(__VA_ARGS__); } }while(0)
#define IDX_LOGA_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGA(__VA_ARGS__); } }while(0)
#define IDX_LOGF_IF(cond, ...)          do { if ( (cond) ) { IDX_LOGF(__VA_ARGS__); } }while(0)

#define IDX_TRACE_BEGIN(...)        do { if (s_bTraceEnable) {CAM_TRACE_FMT_BEGIN(__VA_ARGS__);}}while(0)
#define IDX_TRACE_END()               do { if (s_bTraceEnable) {CAM_TRACE_FMT_END();}}while(0)

static MBOOL s_bDebugEnable = MFALSE;
static MBOOL s_bAeeEnable = MFALSE;
static MBOOL s_bTraceEnable =  MFALSE;

void IdxBase::mismatchHandling(EModule_T mod, const CAM_IDX_QRY_COMB& qry)
{
    if (s_bDebugEnable || s_bAeeEnable)

    {
        IDX_LOGE_IF(IS_LOG_FULL,
            "Mismatch : [Mod:%s(%d)] (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
            strEModule[mod], mod, strEIspProfile[qry.eIspProfile], strESensorMode[qry.eSensorMode], qry.eFrontBin, qry.eP2size, qry.eFlash, strEApp[qry.eApp], qry.eFaceDetection, qry.eZoom_Idx, qry.eLV_Idx, qry.eCT_Idx, qry.eISO_Idx);

        if (s_bAeeEnable)
        {
            aee_system_warning(LOG_TAG,
                NULL, DB_OPT_DEFAULT,
                "Mismatch : [Mod:%s(%d)] (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
                strEModule[mod], mod, strEIspProfile[qry.eIspProfile], strESensorMode[qry.eSensorMode], qry.eFrontBin, qry.eP2size, qry.eFlash, strEApp[qry.eApp], qry.eFaceDetection, qry.eZoom_Idx, qry.eLV_Idx, qry.eCT_Idx, qry.eISO_Idx);
        }
    }
}

// *******************
// ***** IdxMask *****
// *******************
IdxMask::IdxMask()
    : m_mod(EModule_NUM), dim_ns(0), entry_ns(0), key_sz(0), entry(NULL), acc_factor_ns(NULL), key(NULL), dims(NULL)
{
}

IdxMask::~IdxMask()
{
    if (key)
    {
        delete [] key;
    }

    if (acc_factor_ns)
    {
        delete [] acc_factor_ns;
    }
}

MVOID IdxMask::init(EModule_T mod, IDX_BASE_T* pData, MUINT16* pFactorNs)
{
    if ((!pData) || (!pFactorNs))
    {
        assert(0 && "[IdxMask::init] invalid parameter");
        return;
    }

    IDX_LOGD_IF(IS_LOG_BASIC, "IdxMask::init() for mod[%d]", mod);

    IDX_MASK_T* pMask = (IDX_MASK_T*)pData;

    m_mod = mod;
    dim_ns = pMask->base.dim_ns;
    dims = pMask->base.dims;
    entry_ns = pMask->data.entry_ns;
    key_sz = pMask->data.key_sz;
    entry = pMask->data.entry;

    if (dim_ns)
    {
        acc_factor_ns = new (std::nothrow) MUINT16[dim_ns];

        if (!acc_factor_ns)
        {
            assert(acc_factor_ns && "[IdxMask::init] allocation acc_factor_ns fail");
            return;
        }

        acc_factor_ns[0] = 0;
        for (MINT32 i = 1; i < dim_ns; i++)
            acc_factor_ns[i] = pFactorNs[dims[i - 1]] + acc_factor_ns[i - 1];
    }

    if (key_sz)
    {
        key = new (std::nothrow) MUINT32[key_sz];
        if (!key)
        {
            assert(key && "[IdxMask::init] allocation key fail");
            return;
        }
    }
}

#define SET_BIT(i, b) (key[(b + acc_factor_ns[i])/32] |= (((MUINT32)0x1)<<((b + acc_factor_ns[i])%32)))

MVOID IdxMask::genKey(std::vector<MUINT16>& factor)
{
    memset(key, 0, sizeof(MUINT32) * key_sz);

    for (MINT32 i = 0; i < dim_ns; i++)
    {
        MUINT16 val = factor[dims[i]];

        if (((val + acc_factor_ns[i])/32) >= key_sz)
        {
            IDX_LOGE("[IdxMask::genKey] mod %d, key_sz %d, dims[%d] %d, factor %d", m_mod, key_sz, i, dims[i], factor[dims[i]]);
            assert(0 && "[IdxMask::genKey] access over-range");
            return;
        }
        SET_BIT(i, val);
    }
}

MINT32 IdxMask::compareKey(MUINT32* key_in, MUINT32* key_golden)
{
    for (MUINT32 i = 0; i < key_sz; i++)
    {
        if ((key_in[i] & key_golden[i]) != key_in[i])
        {
            return MFALSE;
        }
    }

    return MTRUE;
}

IDXMGR_QUERY_RESULT IdxMask::query(const CAM_IDX_QRY_COMB& qry)
{
    MUINT32 i;
    std::vector<MUINT16> factor;

    for (MINT32 i = 0; i < sizeof(qry)/sizeof(MINT32); i++)
    {
        MUINT16 val = (MUINT16)qry.query[i];
        factor.push_back(val);
    }

    genKey(factor);

    IDXMGR_QUERY_RESULT rtnVal = {0, INVALID_SCENARIO};
    for (i = 0; i < entry_ns; i++)
    {
        if (compareKey(key, entry[i].key))
        {
            rtnVal.idx = entry[i].value;
            rtnVal.scenario = entry[i].scenario;
            return rtnVal;
        }
    }

    mismatchHandling(m_mod, qry);

    return rtnVal;
}

// *******************
// ***** IdxDM *******
// *******************

IdxDM::IdxDM()
    : m_mod(EModule_NUM), dim_ns(0), acc_factor_ns(NULL), dims(NULL), idx_array(NULL), scenarios(NULL)
{
}

IdxDM::~IdxDM()
{
    if (acc_factor_ns)
    {
        delete [] acc_factor_ns;
    }
}

MVOID IdxDM::init(EModule_T mod, IDX_BASE_T* pData, MUINT16* pFactorNs)
{
    if ((!pData) || (!pFactorNs))
    {
        assert(0 && "[IdxDM::init] invalid parameter");
        return;
    }

    IDX_LOGD_IF(IS_LOG_BASIC, "IdxDM::init() for mod[%d]", mod);

    IDX_DM_T* pDM = (IDX_DM_T*)pData;

    m_mod = mod;
    dim_ns = pDM->base.dim_ns;
    dims = pDM->base.dims;
    idx_array = pDM->data.idx_array;
    scenarios = pDM->data.scenario_array;

    assert((idx_array != NULL) && "[IdxDM::init] idx_array is NULL");
    assert((scenarios != NULL) && "[IdxDM::init] scenarios is NULL");

    if (dim_ns)
    {
        acc_factor_ns = new (std::nothrow) MUINT16[dim_ns];

        if (!acc_factor_ns)
        {
            assert(acc_factor_ns && "[IdxDM::init] allocation acc_factor_ns fail");
            return;
        }

        for (MINT32 i = 0; i < dim_ns; i++)
        {
            acc_factor_ns[i] = 1;

            for (MINT32 j = i + 1; j < dim_ns; j++)
            {
                acc_factor_ns[i] *= pFactorNs[dims[j]];
            }
            IDX_LOGD_IF(IS_LOG_BASIC, "IdxDM::init() [Mod:%d] acc_factor_ns[%d]: %d", mod, i, acc_factor_ns[i]);
        }
    }
}

IDXMGR_QUERY_RESULT IdxDM::query(const CAM_IDX_QRY_COMB& qry)
{
    IDXMGR_QUERY_RESULT rtnVal = {0, INVALID_SCENARIO};
    MUINT32 idx = 0;
    MUINT32 i;

    for (i = 0; i < dim_ns; i++)
    {
        MUINT32 val = qry.query[dims[i]];
        idx += (val * acc_factor_ns[i]);
    }

    rtnVal.idx = idx_array[idx];
    rtnVal.scenario = scenarios[idx];

    if (rtnVal.idx == (MUINT16)(-1))
    {
        mismatchHandling(m_mod, qry);
        rtnVal.idx = 0;
        rtnVal.scenario = INVALID_SCENARIO;
    }

    return rtnVal;
}

// *******************
// ***** IdxMgr ******
// *******************
IdxMgr* IdxMgr::createInstance(MUINT32 eSensorDev)
{
    static IdxMgr singleton;

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return &singleton;
    }

    if (singleton.m_pIdxMgrImpBase[idx] == NULL && MTRUE == singleton.m_flag[idx].load(std::memory_order_relaxed))
    {
        singleton.m_pIdxMgrImpBase[idx] = IdxMgrImplBase::getInstance(eSensorDev);
        IDX_LOGE_IF((singleton.m_pIdxMgrImpBase[idx] == NULL), "[Dev:%d] IdxMgrImplBase::getInstance fail, [mapped idx: %d]", eSensorDev, idx);
        {
            extern void nvram_ut(int);
            nvram_ut((int)eSensorDev);
        }
    }

    s_bDebugEnable  = property_get_int32("vendor.debug.mapping_mgr.enable", 0);
    s_bAeeEnable      = property_get_int32("vendor.debug.mapping_mgr.aee", 0);
    s_bTraceEnable   = property_get_int32("vendor.debug.mapping_mgr.trace", 0);

    IDX_LOGE_IF((singleton.m_pIdxMgrImpBase[idx] == NULL), "[Dev:%d] IdxMgrImplBase::getInstance fail", eSensorDev);
    IDX_LOGD_IF(IS_LOG_BASIC, "[Dev:%d] %p", eSensorDev, singleton.m_pIdxMgrImpBase[idx]);

    return  &singleton;
}


IdxMgr::IdxMgr() : m_pIdxMgrImpBase {0},
                   m_flag           {0}
{}

IdxMgr::~IdxMgr()
{
    for (MINT32 i = 0; i < IDXMGRIMP_SIZE; i++)
    {
        if (m_pIdxMgrImpBase[i])
        {
            delete m_pIdxMgrImpBase[i];
        }
    }
    memset(m_pIdxMgrImpBase, 0, sizeof(m_pIdxMgrImpBase));
}

MUINT16 IdxMgr::query(MUINT32 eSensorDev, EModule_T mod, MUINT32 FrmId)
{
    IDXMGR_QUERY_RESULT rtnVal = {0, 0, NULL};

    IDX_TRACE_BEGIN("IdxMgr_query");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return 0;
    }

        if (m_pIdxMgrImpBase[idx])
        {
        CAM_IDX_QRY_COMB info;
        if(!m_pIdxMgrImpBase[idx]->getMapping_List(FrmId, info)){
            if(!m_pIdxMgrImpBase[idx]->getMapping_List_Last(info)){
                IDX_LOGE("no Mapping Info");
            }
        }

        rtnVal = m_pIdxMgrImpBase[idx]->query(mod, info);

            assert (rtnVal.pScenario != NULL);

            IDX_LOGD_IF(IS_LOG_BASIC,
                "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s)",
                eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario);

            IDX_LOGD_IF(IS_LOG_FULL,
            "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
            eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario, strEIspProfile[info.eIspProfile], strESensorMode[info.eSensorMode], info.eFrontBin, info.eP2size, info.eFlash, strEApp[info.eApp], info.eFaceDetection, info.eZoom_Idx, info.eLV_Idx, info.eCT_Idx, info.eISO_Idx);
        }
        else
        {
            IDX_LOGE("[Dev:%d-Mod%d] Not initialized", eSensorDev, mod);
        }

    IDX_TRACE_END();

    return rtnVal.idx;
}

MUINT16 IdxMgr::query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, const char* caller)
{
    IDXMGR_QUERY_RESULT rtnVal = {0, NULL};

    IDX_TRACE_BEGIN("IdxMgr_query");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return 0;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        rtnVal = m_pIdxMgrImpBase[idx]->query(mod, qry);

        assert (rtnVal.pScenario != NULL);

        IDX_LOGD_IF(IS_LOG_BASIC,
            "[Dev:%d-Caller:%s-Mod:%s(%d)] (Idx %d) (%s)",
            eSensorDev, caller, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario);

        IDX_LOGD_IF(IS_LOG_FULL,
            "[Dev:%d-Caller:%s-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
            eSensorDev, caller, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario, strEIspProfile[qry.eIspProfile], strESensorMode[qry.eSensorMode], qry.eFrontBin, qry.eP2size, qry.eFlash, strEApp[qry.eApp], qry.eFaceDetection, qry.eZoom_Idx, qry.eLV_Idx, qry.eCT_Idx, qry.eISO_Idx);
    }
    else
    {
        IDX_LOGE("[Dev:%d-Caller:%s-Mod(%d)] Not initialized", eSensorDev, caller, mod);
    }

    IDX_TRACE_END();

    return rtnVal.idx;
}

MUINT16 IdxMgr::query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, IDXMGR_QUERY_RESULT& result)
{
    IDXMGR_QUERY_RESULT rtnVal = {0, 0, NULL};

    IDX_TRACE_BEGIN("IdxMgr_query");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return 0;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        rtnVal = m_pIdxMgrImpBase[idx]->query(mod, qry);

        assert (rtnVal.pScenario != NULL);

        IDX_LOGD_IF(IS_LOG_BASIC,
                "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s)",
                eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario);

        IDX_LOGD_IF(IS_LOG_FULL,
                "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
                eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario, strEIspProfile[qry.eIspProfile], strESensorMode[qry.eSensorMode], qry.eFrontBin, qry.eP2size, qry.eFlash, strEApp[qry.eApp], qry.eFaceDetection, qry.eZoom_Idx, qry.eLV_Idx, qry.eCT_Idx, qry.eISO_Idx);
    }
    else
    {
            IDX_LOGE("[Dev:%d-Mod%d] Not initialized", eSensorDev, mod);
    }

    IDX_TRACE_END();

    result.scenario = rtnVal.scenario;
	result.idx = rtnVal.idx;
	result.group = 0;

    return rtnVal.idx;
}

MUINT16 IdxMgr::query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, MUINT16& u2ScenarioIdx)
{
    IDXMGR_QUERY_RESULT rtnVal = {0, 0, NULL};

    IDX_TRACE_BEGIN("IdxMgr_query");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return 0;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        rtnVal = m_pIdxMgrImpBase[idx]->query(mod, qry);

        assert (rtnVal.pScenario != NULL);

        IDX_LOGD_IF(IS_LOG_BASIC,
                "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s)",
                eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario);

        IDX_LOGD_IF(IS_LOG_FULL,
                "[Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d,)",
                eSensorDev, strEModule[mod], mod, rtnVal.idx, rtnVal.pScenario, strEIspProfile[qry.eIspProfile], strESensorMode[qry.eSensorMode], qry.eFrontBin, qry.eP2size, qry.eFlash, strEApp[qry.eApp], qry.eFaceDetection, qry.eZoom_Idx, qry.eLV_Idx, qry.eCT_Idx, qry.eISO_Idx);
    }
    else
    {
            IDX_LOGE("[Dev:%d-Mod%d] Not initialized", eSensorDev, mod);
    }

    IDX_TRACE_END();

    u2ScenarioIdx = rtnVal.scenario;

    return rtnVal.idx;
}

MUINT16 IdxMgr::query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, IDXMGR_QUERY_RESULT& result, const char* caller)
{
    return 0; //not support
}

MVOID IdxMgr::setMappingInfo(MUINT32 eSensorDev, const CAM_IDX_QRY_COMB& output , MUINT32 FrmId)
{

    IDX_TRACE_BEGIN("IdxMgr_setInfo");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        m_pIdxMgrImpBase[idx]->updateMapping_List(FrmId, output);
    }
    else
    {
        IDX_LOGE("[Dev:%d] Idx_Mgr Not initialized", eSensorDev);
    }
    IDX_TRACE_END();

}

MVOID IdxMgr::patchMappingInfo(MUINT32 eSensorDev, CAM_IDX_QRY_COMB& output, EDim_T dim, MUINT32* value){
    output.query[dim] = *(MUINT32*)value;
}

MVOID IdxMgr::getMappingInfo(MUINT32 eSensorDev, CAM_IDX_QRY_COMB& output , MUINT32 FrmId)
{
    IDX_TRACE_BEGIN("IdxMgr_getInfo");

    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        if(!m_pIdxMgrImpBase[idx]->getMapping_List(FrmId, output)){
            if(!m_pIdxMgrImpBase[idx]->getMapping_List_Last(output)){
                IDX_LOGE("no Mapping Info");
            }
        }
    }
    else
    {
        IDX_LOGE("[Dev:%d] Idx_Mgr Not initialized", eSensorDev);
    }

    IDX_TRACE_END();
}

MVOID IdxMgr::getScenarioInfo(MUINT32 eSensorDev, char **p_scenarios, MUINT16 &scenarioNs)
{
    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return;
    }

    if (m_pIdxMgrImpBase[idx])
    {
        IdxMgrImpl* pIdxMgrImpl = (IdxMgrImpl*)m_pIdxMgrImpBase[idx];
        *p_scenarios = pIdxMgrImpl->getScenarioPointer();
        scenarioNs = pIdxMgrImpl->getScenarioNs();
    }
    else
    {
        IDX_LOGE("[Dev:%d] Idx_Mgr Not initialized", eSensorDev);
    }
}

MVOID IdxMgr::setValidFlag(NSIspTuning::ESensorDev_T eSensorDev)
{
    auto idx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
    if (idx == -1)
    {
        IDX_LOGE("get idx from mapSensorDevToIdx()using [Dev:%d] failed", eSensorDev);
        return;
    }
    if(idx >= SENSOR_IDX_MAX || idx < 0)
    {
        IDX_LOGE("idx: (%d) exceeds  SENSOR_IDX_MAX   [Dev:%d]", idx, eSensorDev);
        return;
    }
    m_flag[idx].store(MTRUE, std::memory_order_relaxed);
}

IdxMgrImplBase* IdxMgrImplBase::getInstance(MUINT32 eSensorDev)
{
    IdxMgrImplBase* pMgrImplBase = NULL;
    MVOID*  pNvram_Idx = NULL;
    MINT32 err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_IDX_TBL, eSensorDev, (MVOID*&)pNvram_Idx);
    if ((err != 0) || (pNvram_Idx == NULL))
    {
        IDX_LOGE("[Dev:%d] getBufAndRead fail", eSensorDev);
        return NULL;
    }

    pMgrImplBase = IdxMgrImpl::getInstance(eSensorDev, pNvram_Idx);

    IDX_LOGE_IF((pMgrImplBase == NULL), "[Dev:%d] IdxMgrImpl::getInstance fail", eSensorDev);

    return pMgrImplBase;
}

IdxMgrImplBase::IdxMgrImplBase()
{
}

IdxMgrImplBase::~IdxMgrImplBase()
{
}

#define MY_INST_IDX_MGR NS3Av3::INST_T<IdxMgrImpl>
static std::array<MY_INST_IDX_MGR, SENSOR_IDX_MAX> gMultitonIdxMgr;

IdxMgrImplBase* IdxMgrImpl::
getInstance(MUINT32 eSensorDev, MVOID* const pNvram_Idx)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        return nullptr;
    }

    MY_INST_IDX_MGR& rSingleton = gMultitonIdxMgr[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<IdxMgrImpl>();
        (rSingleton.instance)->init(pNvram_Idx);
    } );

    return rSingleton.instance.get();
}

IdxMgrImpl::IdxMgrImpl()
: m_pFactorNs(NULL), m_pScenarios(NULL), m_scenarioNs(0), m_pIspTuningCustom(NULL)
{
    memset(m_pMobObj, 0, sizeof(m_pMobObj));
    for ( int dim = 0; dim < EDim_NUM; dim++ )
        m_funcs[dim] = NULL;
}

IdxMgrImpl::~IdxMgrImpl()
{
    Mutex::Autolock lock(m_lock);

    for (MINT32 i = 0; i < EModule_NUM; i++)
    {
        if (m_pMobObj[i])
        {
            delete m_pMobObj[i];
        }
    }
    memset(m_pMobObj, 0, sizeof(m_pMobObj));

    clearMapping_List();
}

MVOID IdxMgrImpl::init(MVOID* const pNvram_Idx)
{
    Mutex::Autolock lock(m_lock);

    if (pNvram_Idx == NULL)
    {
        assert((pNvram_Idx != NULL) && "[IdxMgrImpl::init] invalid input parameter");
        return;
    }

    IDX_MODULE_ARRAY*  pModArray = (IDX_MODULE_ARRAY*)pNvram_Idx;

    for (MINT32 i = 0; i < EModule_NUM; i++)
    {
        IdxBase*pBase = classFactory((EModule_T)i, pModArray->modules[i], pModArray->idx_factor_ns);
        assert((pBase != NULL) && "[IdxMgrImpl::init] classFactory output NULL pointer");
        m_pMobObj[i] = pBase;
    }
    m_pFactorNs = pModArray->idx_factor_ns;
    m_scenarioNs = pModArray->scenario_ns;
    m_pScenarios = pModArray->scenarios;
    m_pIspTuningCustom = NULL;
}

IDXMGR_QUERY_RESULT IdxMgrImpl::query(EModule_T mod, const CAM_IDX_QRY_COMB& qry)
{
    Mutex::Autolock lock(m_lock);

    IDX_QUERY_RESULT rtnVal = {0, INVALID_SCENARIO, 0};
    IDXMGR_QUERY_RESULT rtnVal2 = {0, 0, NULL, 0, 0};

    for (MUINT32 i = 0; i < EDim_NUM; i++)
    {
        if (qry.query[i] >= m_pFactorNs[i])
        {
            assert(0 && "[IdxMgrImpl::query] query fail, dimension out of range");
            return rtnVal2;
        }
    }

    IdxBase* pMod = m_pMobObj[mod];

    if (pMod != NULL)
    {
        rtnVal2 = pMod->query(qry);
    }

    if (rtnVal2.scenario >= m_scenarioNs)
    {
        rtnVal2.scenario = m_scenarioNs - 1;
    }

    //rtnVal2.idx = rtnVal.idx;
    //rtnVal2.scenario = rtnVal.scenario;
    rtnVal2.pScenario = (*m_pScenarios)[rtnVal2.scenario];
    //rtnVal2.group = rtnVal.group;

    return rtnVal2;
}

MBOOL IdxMgrImpl::updateMapping_List(MUINT32 FrmId, const CAM_IDX_QRY_COMB& qry)
{
    Mutex::Autolock lock(m_lock);

    MINT32 i4Ret = MFALSE;
    MINT32 i4Pos = 0;
    MINT32 i4Size = MappingInfoQ.size();

    ISP_MAPPING_INFO_T temp;
    temp.u4FrmId = FrmId;
    temp.qry = qry;

    list<ISP_MAPPING_INFO_T>::iterator it = MappingInfoQ.begin();

    for (it = MappingInfoQ.begin(); it != MappingInfoQ.end(); it++, i4Pos++)
    {
        if (it->u4FrmId == FrmId)
        {
            //MY_LOGD("overwirte LCSList_Out FrmID: %d", FrmId);
            it->qry = qry;
            i4Ret = MTRUE;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        MappingInfoQ.push_back(temp);
        i4Ret = MTRUE;
    }

    // remove item
    if (MappingInfoQ.size() > MAPPING_INFO_LIST_SIZE)
    {
        MappingInfoQ.erase(MappingInfoQ.begin());
    }
    return i4Ret;
}

MUINT32 IdxMgrImpl::addMappingFunction(MUINT32 dim, MUINT32 (IspTuningCustom::*function)(MUINT32, MVOID*), IspTuningCustom& tuning){
    m_funcs[dim] = function;
    m_pIspTuningCustom = &tuning;
    return 0;
}

MBOOL IdxMgrImpl::updateMapping_List_By_Dim(MUINT32 FrmId, MUINT32 dim, MVOID* input)
{
    Mutex::Autolock lock(m_lock);

    MINT32 i4Ret = MFALSE;
    MINT32 i4Pos = 0;
    MINT32 i4Size = MappingInfoQ.size();
    ISP_MAPPING_INFO_T temp;
    MUINT32 value = 0;

    if (m_funcs[dim] != NULL && m_pIspTuningCustom != NULL)
        value = CALL_MEMBER_FN(*m_pIspTuningCustom ,m_funcs[dim])(dim ,input);
    else
        value = *(MUINT32*)input;

    list<ISP_MAPPING_INFO_T>::iterator it = MappingInfoQ.begin();

    for (it = MappingInfoQ.begin(); it != MappingInfoQ.end(); it++, i4Pos++)
    {
        if (it->u4FrmId == FrmId)
        {
#if BUILD_VERSION > 1
            if (dim == EDim_ISO){
                memcpy(&(it->qry.eISO_Idx), input, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
            } else {
                it->qry.query[dim] = value;
            }
#else
            it->qry.query[dim] = value;
#endif
            i4Ret = MTRUE;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        if (i4Size == 0)
        {
#if BUILD_VERSION > 1
            if (dim == EDim_ISO){
                memcpy(&(temp.qry.eISO_Idx), input, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
            } else {
                temp.qry.query[dim] = value;
            }
#else
            temp.qry.query[dim] = value;
#endif
            temp.u4FrmId = FrmId;
            MappingInfoQ.push_back(temp);
            i4Ret = MTRUE;
        }
        else
        {//copy last item in list to temp and push it into end of the list
            it = MappingInfoQ.end();
            it--;
            temp.qry = it->qry;
#if BUILD_VERSION > 1
            if (dim == EDim_ISO){
                memcpy(&(temp.qry.eISO_Idx), input, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
            } else {
                temp.qry.query[dim] = value;
            }
#else
            temp.qry.query[dim] = value;
#endif
            temp.u4FrmId = FrmId;
            MappingInfoQ.push_back(temp);
            i4Ret = MTRUE;
        }

    }

    // remove item
    if (MappingInfoQ.size() > MAPPING_INFO_LIST_SIZE)
    {
        MappingInfoQ.erase(MappingInfoQ.begin());
    }

    return i4Ret;
}

MBOOL IdxMgrImpl::getMapping_List(MUINT32 FrmId, CAM_IDX_QRY_COMB& qry)
{
    Mutex::Autolock lock(m_lock);

    MBOOL i4Ret = MFALSE;
    MINT32 i4Pos = 0;
    MINT32 i4Size = MappingInfoQ.size();
    list<ISP_MAPPING_INFO_T>::iterator it = MappingInfoQ.begin();
    for (; it != MappingInfoQ.end(); it++, i4Pos++)
    {
        if (it->u4FrmId == (MUINT32)FrmId)
        {
            qry = it->qry;
            i4Ret = MTRUE;
            //MY_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        //MY_LOGD("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = MFALSE;
    }

    return i4Ret;
}

MBOOL IdxMgrImpl::getMapping_List_Last(CAM_IDX_QRY_COMB& qry)
{
    Mutex::Autolock lock(m_lock);

    if (!MappingInfoQ.empty())
    {
        list<ISP_MAPPING_INFO_T>::iterator it = MappingInfoQ.end();
        it--;
        qry = it->qry;
        return MTRUE;
    }

    return MFALSE;
}

MVOID IdxMgrImpl::clearMapping_List()
{
    MappingInfoQ.clear();
}

IdxBase* IdxMgrImpl::classFactory(EModule_T mod, IDX_BASE_T* pBase, MUINT16* pFactorNs)
{
    if ((!pBase) || (!pFactorNs))
    {
        return NULL;
    }

    IdxBase* pOut = NULL;
    MUINT8 algo_type = pBase->type;

    switch (algo_type)
    {
    case IDX_ALGO_MASK:
        pOut = new IdxMask();
        if (pOut)
        {
            pOut->init(mod, pBase, pFactorNs);
        }
        break;

    case IDX_ALGO_DM:
        pOut = new IdxDM();
        if (pOut)
        {
            pOut->init(mod, pBase, pFactorNs);
        }
        break;

    case IDX_ALGO_TREE:
        pOut = NULL;
        break;

    case IDX_ALGO_HASH:
        pOut = NULL;
        break;

    default:
        pOut = NULL;
        break;
    }

    return pOut;
}

char* IdxMgrImpl::
getScenarioPointer()
{
    return (char *)m_pScenarios;
}

MUINT16 IdxMgrImpl::
getScenarioNs()
{
    return m_scenarioNs;
}

#endif
