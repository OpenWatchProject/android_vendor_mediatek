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

/**
* @file ResultPool.cpp
* @brief Implementation of Result Pool Manager
*/

#define LOG_TAG "Entry"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Log.h>
#include <string>

#include "Entry.h"
#include "aaa_utils.h"
#include <cutils/properties.h>

// ResultPoolBase.h define in Entry.h
#include <ResultPool4LSCConfig.h>
#include <ResultPool4Meta.h>
#include <isp_config/isp_af_config.h>
#include <isp_config/isp_pdo_config.h>
#include <isp_config/isp_ae_config.h>
#include <isp_config/isp_awb_config.h>
#include <isp_config/isp_flicker_config.h>
#include <ResultPool4Module.h>

using namespace NS3Av3;

/*******************************************************************************
*
*   utilities
*
********************************************************************************/
#define GET_PROP(prop, dft, val)\
    {\
       val = property_get_int32(prop,dft);\
    }

inline MVOID* enumToCreate(E_PARTIAL_RESULT_OF_MODULE_T ModuleEnum)
{
    switch (ModuleEnum)
    {
        case E_HAL_RESULTTOMETA:
        {
            HALResultToMeta_T* pObj = new HALResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AE_RESULTTOMETA:
        {
            AEResultToMeta_T* pObj = new AEResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AF_RESULTTOMETA:
        {
            AFResultToMeta_T* pObj = new AFResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AWB_RESULTTOMETA:
        {
            AWBResultToMeta_T* pObj = new AWBResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_ISP_RESULTTOMETA:
        {
            ISPResultToMeta_T* pObj = new ISPResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_LSC_RESULTTOMETA:
        {
            LSCResultToMeta_T* pObj = new LSCResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_FLASH_RESULTTOMETA:
        {
            FLASHResultToMeta_T* pObj = new FLASHResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_FLK_RESULTTOMETA:
        {
            FLKResultToMeta_T* pObj = new FLKResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_NUM_RESULTTOMETA:
            return NULL;
        case E_LCSO_RESULTTOMETA:
        {
            LCSOResultToMeta_T* pObj = new LCSOResultToMeta_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_LSC_CONFIGRESULTTOISP:
        {
            LSCConfigResult_T* pObj = new LSCConfigResult_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AF_CONFIGRESULTTOISP:
        {
            AFResultConfig_T* pObj = new AFResultConfig_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_FLK_CONFIGRESULTTOISP:
        {
            FLKResultConfig_T* pObj = new FLKResultConfig_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AE_CONFIGRESULTTOISP:
        {
            AEResultConfig_T* pObj = new AEResultConfig_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AWB_CONFIGRESULTTOISP:
        {
            AWBResultConfig_T* pObj = new AWBResultConfig_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_CCU_RESULTINFO4OVERWRITE:
        {
            CCUResultInfo_T* pObj = new CCUResultInfo_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_AWB_RESULTINFO4ISP:
        {
            AWBResultInfo_T* pObj = new AWBResultInfo_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_PDO_CONFIGRESULTTOISP:
        {
            isp_pdo_cfg_t* pObj = new isp_pdo_cfg_t();
            return static_cast<MVOID*> (pObj);
        }
        case E_AE_RESULTINFO:
        {
            AEResultInfo_T* pObj = new AEResultInfo_T();
            return static_cast<MVOID*> (pObj);
        }
        case E_NUM_RESULT_END:
        default:
            return NULL;
    }
}

inline MINT32 enumToDelete(E_PARTIAL_RESULT_OF_MODULE_T ModuleEnum, MVOID* pObj)
{
    switch (ModuleEnum)
    {
        case E_HAL_RESULTTOMETA:
        {
            delete (static_cast<HALResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_AE_RESULTTOMETA:
        {
            delete (static_cast<AEResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_AF_RESULTTOMETA:
        {
            delete (static_cast<AFResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_AWB_RESULTTOMETA:
        {
            delete (static_cast<AWBResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_ISP_RESULTTOMETA:
        {
            delete (static_cast<ISPResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_LSC_RESULTTOMETA:
        {
            delete (static_cast<LSCResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_FLASH_RESULTTOMETA:
        {
            delete (static_cast<FLASHResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_FLK_RESULTTOMETA:
        {
            delete (static_cast<FLKResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_NUM_RESULTTOMETA:
            return -1;
        case E_LCSO_RESULTTOMETA:
        {
            delete (static_cast<LCSOResultToMeta_T*> (pObj));
            return MTRUE;
        }
        case E_LSC_CONFIGRESULTTOISP:
        {
            delete (static_cast<LSCConfigResult_T*> (pObj));
            return MTRUE;
        }
        case E_AF_CONFIGRESULTTOISP:
        {
            delete (static_cast<AFResultConfig_T*> (pObj));
            return MTRUE;
        }
        case E_FLK_CONFIGRESULTTOISP:
        {
            delete (static_cast<FLKResultConfig_T*> (pObj));
            return MTRUE;
        }
        case E_AE_CONFIGRESULTTOISP:
        {
            delete (static_cast<AEResultConfig_T*> (pObj));
            return MTRUE;
        }
        case E_AWB_CONFIGRESULTTOISP:
        {
            delete (static_cast<AWBResultConfig_T*> (pObj));
            return MTRUE;
        }
        case E_CCU_RESULTINFO4OVERWRITE:
        {
            delete (static_cast<CCUResultInfo_T*> (pObj));
            return MTRUE;
        }
        case E_AWB_RESULTINFO4ISP:
        {
            delete (static_cast<AWBResultInfo_T*> (pObj));
            return MTRUE;
        }
        case E_PDO_CONFIGRESULTTOISP:
        {
            delete (static_cast<isp_pdo_cfg_t*> (pObj));
            return MTRUE;
        }
        case E_AE_RESULTINFO:
        {
            delete (static_cast<AEResultInfo_T*> (pObj));
            return MTRUE;
        }
        case E_NUM_RESULT_END:
        default:
            return -1;
    }
}

Entry::Entry()
    : m_Lock()
    , m_pModuleAddr(NULL)
    , m_bIsValidate(MFALSE)
    , m_u4LogEn(0)
{
    CAM_LOGD("[%s] Constructor", __FUNCTION__);
}

Entry::Entry(const std::string strUserName, const E_PARTIAL_RESULT_OF_MODULE_T ModuleEnum)
    : m_Lock()
    , m_pModuleAddr(NULL)
    , m_bIsValidate(MFALSE)
    , m_u4LogEn(0)
{
    GET_PROP("vendor.debug.EntryPool.log", 0, m_u4LogEn);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ Constructor pModuleAddr:%p", __FUNCTION__, m_pModuleAddr);

#if 1
    m_pModuleAddr = enumToCreate(ModuleEnum);
    if(m_pModuleAddr == NULL)
        CAM_LOGE("[%s] Add Enum does not create object", __FUNCTION__);
#else
    if(m_pModuleAddr == NULL)
        m_pModuleAddr = (MVOID*)malloc(size);
#endif
    CAM_LOGD_IF(m_u4LogEn, "[%s]- Constructor UserName:%s pModuleAddr:%p", __FUNCTION__, strUserName.c_str(), m_pModuleAddr);
}

Entry::~Entry()
{
    CAM_LOGD_IF(m_u4LogEn, "[%s] Destructor", __FUNCTION__);
}

MVOID
Entry::
free(const E_PARTIAL_RESULT_OF_MODULE_T ModuleEnum)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s]+", __FUNCTION__);
#if 1

    MINT32 i4Ret = MFALSE;
    if(m_pModuleAddr)
    {
        i4Ret = enumToDelete(ModuleEnum, m_pModuleAddr);
        m_pModuleAddr = NULL;
    }
#else
    ::free(m_pModuleAddr);
    m_pModuleAddr = NULL;
#endif
    CAM_LOGD_IF(m_u4LogEn, "[%s]- Ret:%d", __FUNCTION__, i4Ret);
}


 MVOID*
 Entry::
read()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_pModuleAddr:%p", __FUNCTION__, m_pModuleAddr);

    CAM_LOGD_IF(m_u4LogEn, "[%s]- bIsValidate:%d", __FUNCTION__, m_bIsValidate);
    return m_pModuleAddr;
}

MVOID
Entry::
write(const MVOID* pData)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ pData = %p", __FUNCTION__, pData);
    //===== copy data to local member from module =====
#if 1
    if(m_pModuleAddr)
    {
        ResultPoolBase_T* pBaseDst = reinterpret_cast<ResultPoolBase_T*>(m_pModuleAddr);
        const ResultPoolBase_T* pBaseSrc = reinterpret_cast<const ResultPoolBase_T*>(pData);
        (*pBaseDst) = (*pBaseSrc);
#else
        ::memcpy(m_pModuleAddr, pData, size);
#endif
        m_bIsValidate = MTRUE;
    }
    CAM_LOGD_IF(m_u4LogEn, "[%s]- IsValidate:%d", __FUNCTION__, m_bIsValidate);
}

MVOID
Entry::
reset()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_pModuleAddr:%p", __FUNCTION__, m_pModuleAddr);
#if 1
    if(m_pModuleAddr)
    {
        ResultPoolBase_T* pBase = static_cast<ResultPoolBase_T*>(m_pModuleAddr);
        pBase->clearMember();
    }
#else // vector can't not use
    memset(m_pModuleAddr, 0, m_i4Size);
#endif
    m_bIsValidate = MFALSE;
    CAM_LOGD_IF(m_u4LogEn, "[%s]-", __FUNCTION__);
}

MBOOL
Entry::
isValidate()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_bIsValidate:%d", __FUNCTION__, m_bIsValidate);

    CAM_LOGD_IF(m_u4LogEn, "[%s]-", __FUNCTION__);
    return m_bIsValidate;
}

