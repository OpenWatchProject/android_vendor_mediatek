 /*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <dlfcn.h>
#include "RfxGwsdUtils.h"

/*****************************************************************************
 * Class RfxGwsdUtils
 *****************************************************************************/

#define RFX_LOG_TAG "RfxGwsdUtils"
#define GWSD_RIL_PATH "libgwsd-ril.so"


RfxGwsdSettingBaseHandler* RfxGwsdUtils::m_settingHandler = NULL;
RfxGwsdCallControlBaseHandler* RfxGwsdUtils::m_callcontrolHandler = NULL;
void* RfxGwsdUtils::m_dlHandler = NULL;

void RfxGwsdUtils::initHandler() {
    if(RfxRilUtils::isGwsdSupport() == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] not support gwsd", __FUNCTION__);
        return;
    }

    if (m_dlHandler != NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] return, m_dlHandler = %p", __FUNCTION__, m_dlHandler);
        return;
    }

    m_dlHandler = dlopen(GWSD_RIL_PATH, RTLD_NOW);
    if (m_dlHandler == NULL) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] dlopen failed in %s: %s",
                __FUNCTION__, GWSD_RIL_PATH, dlerror());
        return;
    }

    dlerror();

    createSettingHandler_t* createSetting =
            (createSettingHandler_t*) dlsym(m_dlHandler, "createSettingHandler");
    const char* dlsym_error = dlerror();
    if (createSetting == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] create not defined or exported in %s: %s",
                __FUNCTION__, GWSD_RIL_PATH, dlsym_error);
        return;
    }
    m_settingHandler = createSetting();

    createCallControlHandler_t* createCallControl =
            (createCallControlHandler_t*) dlsym(m_dlHandler, "createCallControlHandler");
    if (createCallControl == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] create not defined or exported in %s: %s",
                __FUNCTION__, GWSD_RIL_PATH, dlsym_error);
        return;
    }
    m_callcontrolHandler = createCallControl();

    RFX_LOG_D(RFX_LOG_TAG, "[%s] completed", __FUNCTION__);
}

RfxGwsdSettingBaseHandler* RfxGwsdUtils::getSettingHandler() {
    return m_settingHandler;
}

RfxGwsdCallControlBaseHandler* RfxGwsdUtils::getCallControlHandler() {
    return m_callcontrolHandler;
}

void RfxGwsdUtils::deInitHandler() {

    if (m_dlHandler == NULL) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] return, m_dlHandler == NULL", __FUNCTION__);
        return;
    }

    // release settingHandler resource
    if (m_settingHandler != NULL) {
        destroySettingHandler_t* destroyHandler =
                (destroySettingHandler_t*) dlsym(m_dlHandler, "destroySettingHandler");
        const char* dlsym_error = dlerror();
        if (destroyHandler == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%s] destroy not defined or exported in %s: %s",
                    __FUNCTION__, GWSD_RIL_PATH, dlsym_error);
        } else {
            destroyHandler(m_settingHandler);
            m_settingHandler = NULL;
        }
    }

    // release callcontrolHandler resource
    if (m_callcontrolHandler != NULL) {
        destroyCallControlHandler_t* destroyHandler =
                (destroyCallControlHandler_t*) dlsym(m_dlHandler, "destroyCallControlHandler");
        const char* dlsym_error = dlerror();
        if (destroyHandler == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%s] destroy not defined or exported in %s: %s",
                    __FUNCTION__, GWSD_RIL_PATH, dlsym_error);
        } else {
            destroyHandler(m_callcontrolHandler);
            m_callcontrolHandler = NULL;
        }
    }

    dlclose(m_dlHandler);
    m_dlHandler = NULL;
    RFX_LOG_D(RFX_LOG_TAG, "[%s] completed", __FUNCTION__);
}
