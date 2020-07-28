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

using namespace android;
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

Entry::Entry()
    : m_Lock()
    , m_pModuleAddr(NULL)
    , m_bIsValidate(MFALSE)
    , m_i4Size(0)
    , m_u4LogEn(0)
{
    CAM_LOGD("[%s] Constructor", __FUNCTION__);
}

Entry::Entry(const std::string strUserName, const MINT32 size)
    : m_Lock()
    , m_pModuleAddr(NULL)
    , m_bIsValidate(MFALSE)
    , m_i4Size(size)
    , m_u4LogEn(0)
{
    GET_PROP("vendor.debug.EntryPool.log", 0, m_u4LogEn);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ Constructor pModuleAddr:%p, size:%d", __FUNCTION__, m_pModuleAddr, size);

    if(m_pModuleAddr == NULL)
        m_pModuleAddr = (MVOID*)malloc(size);
    CAM_LOGD_IF(m_u4LogEn, "[%s]- Constructor UserName:%s pModuleAddr:%p", __FUNCTION__, strUserName.c_str(), m_pModuleAddr);
}

Entry::~Entry()
{
    CAM_LOGD_IF(m_u4LogEn, "[%s] Destructor", __FUNCTION__);
    if(m_pModuleAddr)
        free();
}

MVOID
Entry::
free()
{
    CAM_LOGD_IF(m_u4LogEn, "[%s]+", __FUNCTION__);
    ::free(m_pModuleAddr);
    m_pModuleAddr = NULL;
    CAM_LOGD_IF(m_u4LogEn, "[%s]-", __FUNCTION__);
}


MVOID*
Entry::
read()
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_pModuleAddr:%p", __FUNCTION__, m_pModuleAddr);

    CAM_LOGD_IF(m_u4LogEn, "[%s]- bIsValidate:%d", __FUNCTION__, m_bIsValidate);
    return m_pModuleAddr;
}

MVOID
Entry::
write(const MINT32 size, const MVOID* pData)
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+", __FUNCTION__);
    //===== copy data to local member from module =====
    ::memcpy(m_pModuleAddr, pData, size);
    m_bIsValidate = MTRUE;
    CAM_LOGD_IF(m_u4LogEn, "[%s]- IsValidate:%d", __FUNCTION__, m_bIsValidate);
}

MVOID
Entry::
reset()
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_pModuleAddr:%p, size:%d", __FUNCTION__, m_pModuleAddr, m_i4Size);
    memset(m_pModuleAddr, 0, m_i4Size);
    m_bIsValidate = MFALSE;
    CAM_LOGD_IF(m_u4LogEn, "[%s]-", __FUNCTION__);
}

MBOOL
Entry::
isValidate()
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF(m_u4LogEn, "[%s]+ m_bIsValidate:%d", __FUNCTION__, m_bIsValidate);

    CAM_LOGD_IF(m_u4LogEn, "[%s]-", __FUNCTION__);
    return m_bIsValidate;
}

