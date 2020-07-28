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
#define LOG_TAG "idx_cache"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>

#include <idx_cache.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>

#define IDX_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

using namespace NSIspTuning;

namespace NSIspTuning
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MUINT8 IdxCache::s_idxReMapping[IDXCACHE_VALTYPE_NUM] =
{
    0, //IDXCACHE_VALTYPE_CURRENT
    0, //IDXCACHE_VALTYPE_LOWERISO
    1, //IDXCACHE_VALTYPE_UPPERISO
    0, //IDXCACHE_VALTYPE_LOWERLV
    1, //IDXCACHE_VALTYPE_UPPERLV
    0, //IDXCACHE_VALTYPE_LOWERLV_LOWERCT
    1, //IDXCACHE_VALTYPE_LOWERLV_UPPERCT
    2, //IDXCACHE_VALTYPE_UPPERLV_LOWERCT
    3, //IDXCACHE_VALTYPE_UPPERLV_UPPERCT
};

IdxCache::IdxCache(MUINT32 eSensorDev)
: m_eSensorDev(eSensorDev), m_bEnabled(MTRUE)
{
    char *pScenario;
    m_bLogEnable = property_get_int32("vendor.debug.idxcache.log", 0);
    m_bDumpLogEnable = property_get_int32("vendor.debug.atms.dump", 0);
    ::memset((void*)m_cacheSameKey, 0, sizeof(m_cacheSameKey));
    ::memset((void*)m_cacheValid, 0, sizeof(m_cacheValid));
    ::memset((void*)m_cacheVal, 0, sizeof(m_cacheVal));
    ::memset((void*)m_scnIdx, 0, sizeof(m_scnIdx));
    m_scenarioNs = 0;
    IdxMgr::createInstance(m_eSensorDev)->getScenarioInfo(m_eSensorDev, &pScenario, m_scenarioNs);
    m_pScenarios = (char (*)[][64])pScenario;
    if (!m_pScenarios) {
        IDX_LOGE("[IdxCache::IdxCache] m_pScenarios(NULL)\n");
    }
}

MVOID IdxCache::setEnable(MBOOL bEnabled)
{
    m_bEnabled = bEnabled;
}

MVOID IdxCache::updateKey(CAM_IDX_QRY_COMB& sQry, IDXCACHE_STAGE eStage)
{
    if (::memcmp(&sQry, &m_key[eStage], sizeof(CAM_IDX_QRY_COMB))){
        ::memcpy(&m_key[eStage], &sQry, sizeof(CAM_IDX_QRY_COMB));
        m_cacheSameKey[eStage] = MFALSE;
    } else {
        m_cacheSameKey[eStage] = MTRUE;
    }
}

#define IDXCACHE_LOG(bHit, module, index, info, result) \
    CAM_LOGD_IF(m_bLogEnable, \
        "[IdxCache::From:%s] [Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d, ISO_GROUP %d)",\
        bHit ? "IdxCache":"IdxMgr", m_eSensorDev, strEModule[module], module, index, (*m_pScenarios)[u2ScnIdx], strEIspProfile[info.eIspProfile], strESensorMode[info.eSensorMode], info.eFrontBin, info.eSize, info.eFlash, strEApp[info.eApp], info.eFaceDetection, info.eZoom_Idx, info.eLV_Idx, info.eCT_Idx, info.eISO_Idx[u2IsoGrpIdx], u2IsoGrpIdx);

MUINT16 IdxCache::query(EModule_T eMod, IDXCACHE_STAGE eStage, IDXCACHE_VALTYPE eType, CAM_IDX_QRY_COMB& sQry, IDXCACHE_QRYMODE eMode)
{
    MUINT16 u2Idx = 0, u2ScnIdx = 0, u2IsoGrpIdx = 0;
    IDXMGR_QUERY_RESULT result;
    MBOOL bUseCache = (m_bEnabled && (eMode == IDXCACHE_QRYMODE_NORMAL));
    MBOOL bCacheHit = MFALSE;
    MUINT8 idxType = s_idxReMapping[eType];
#if 0
    if (m_bDumpLogEnable){
          char str_dump[500];
          sprintf(str_dump, "[debug](%s) u2Idx(%d), u2IsoGrpIdx(%d), u2ScnIdx(%d), bCacheHit(%d)\n",\
          strEModule[eMod],
          m_cacheVal[eStage][eMod].val[idxType],
          m_cacheIsoGroup[eStage][eMod].val[idxType],
          u2ScnIdx = m_scnIdx[eStage],
          (m_cacheSameKey[eStage] && (m_cacheValid[eStage][eMod] & ((MUINT8)0x1 << idxType))));

          FILE *pFile = fopen("/sdcard/ATMs.log", "a");
          if (pFile){
              fwrite(str_dump, strlen(str_dump), 1, pFile);
              fclose(pFile);
          }
    }
#endif
    if (bUseCache)
    {
        if (m_cacheSameKey[eStage] && (m_cacheValid[eStage][eMod] & ((MUINT8)0x1 << idxType)))
        {
            u2Idx = m_cacheVal[eStage][eMod].val[idxType];
            u2IsoGrpIdx = m_cacheIsoGroup[eStage][eMod].val[idxType];
            u2ScnIdx = m_scnIdx[eStage];
            bCacheHit = MTRUE;

            goto QUERY_EXIT;
        }
    }
    u2Idx = IdxMgr::createInstance(m_eSensorDev)->query(m_eSensorDev, eMod, sQry, result, __FUNCTION__);

    //if (bUseCache)
    {
        m_cacheVal[eStage][eMod].val[idxType] = u2Idx;
        m_scnIdx[eStage] = u2ScnIdx = result.scenario;
        m_cacheValid[eStage][eMod]  |= ((MUINT8)0x1 << idxType);
        m_cacheIsoGroup[eStage][eMod].val[idxType] = u2IsoGrpIdx = result.group;
    }

QUERY_EXIT:
    if (u2ScnIdx >= m_scenarioNs)
    {
        IDX_LOGE("[IdxCache::query] Scenario Index out of bounds, Scenario idx: %d, Total: %d", u2ScnIdx, m_scenarioNs);
        return 0;
    }

    if (m_pScenarios)
    {
        IDXCACHE_LOG(bCacheHit, eMod, u2Idx, sQry, result);
#if 0
        if (m_bDumpLogEnable){
              char str_dump[500];
              sprintf(str_dump, "[IdxCache::From:%s] [Dev:%d-Mod:%s(%d)] (Idx %d) (%s) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d, ISO_GROUP %d) \n",\
      bCacheHit ? "IdxCache":"IdxMgr", m_eSensorDev, strEModule[eMod], eMod, u2Idx, (*m_pScenarios)[u2ScnIdx], strEIspProfile[sQry.eIspProfile], strESensorMode[sQry.eSensorMode], sQry.eFrontBin, sQry.eSize, sQry.eFlash, strEApp[sQry.eApp], sQry.eFaceDetection, sQry.eZoom_Idx, sQry.eLV_Idx, sQry.eCT_Idx, sQry.eISO_Idx[u2IsoGrpIdx], u2IsoGrpIdx);
              FILE *pFile = fopen("/sdcard/ATMs.log", "a");
              if (pFile){
                  fwrite(str_dump, strlen(str_dump), 1, pFile);
                  fclose(pFile);
              }
        }
#endif
    }

    return u2Idx;
}

MBOOL IdxCache::isSameKey(IDXCACHE_STAGE eStage)
{
    return m_cacheSameKey[eStage];
}

MUINT16 IdxCache::getCacheValue(EModule_T eMod, IDXCACHE_STAGE eStage, IDXCACHE_VALTYPE eType) const
{
    MUINT8 idxType = s_idxReMapping[eType];

    return m_cacheVal[eStage][eMod].val[idxType];
}

MUINT16 IdxCache::getIsoGroupValue(EModule_T eMod, IDXCACHE_STAGE eStage, IDXCACHE_VALTYPE eType) const
{
    MUINT8 idxType = s_idxReMapping[eType];

    return m_cacheIsoGroup[eStage][eMod].val[idxType];
}

MUINT16 IdxCache::getSceneIdx(IDXCACHE_STAGE eStage) const
{
    return m_scnIdx[eStage];
}

MVOID IdxCache::setLogEnable()
{
    m_bLogEnable = property_get_int32("vendor.debug.idxcache.log", 0);
    m_bDumpLogEnable = property_get_int32("vendor.debug.atms.dump", 0);
}

}
