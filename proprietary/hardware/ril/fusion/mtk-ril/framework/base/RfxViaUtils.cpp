 /*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include "RfxViaUtils.h"
#include "ratconfig.h"

/*****************************************************************************
 * Class RfxViaUtils
 *****************************************************************************/

#define RFX_LOG_TAG "RfxViaUtils"
#define VIA_RIL_PATH "libvia-ril.so"

ViaBaseHandler* RfxViaUtils::m_viaHandler = NULL;
void* RfxViaUtils::m_dlHandler = NULL;

void RfxViaUtils::initViaHandler() {
    if (RatConfig_isC2kSupported() == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] not support C2K", __FUNCTION__);
        return;
    }

    if (m_dlHandler != NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] return, m_dlHandler = %p", __FUNCTION__, m_dlHandler);
        return;
    }

    m_dlHandler = dlopen(VIA_RIL_PATH, RTLD_NOW);
    if (m_dlHandler == NULL) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] dlopen failed in %s: %s",
                __FUNCTION__, VIA_RIL_PATH, dlerror());
        return;
    }

    // reset errors
    dlerror();

    create_t* createViaHandler = (create_t*) dlsym(m_dlHandler, "create");
    const char* dlsym_error = dlerror();
    if (createViaHandler == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] create not defined or exported in %s: %s",
                __FUNCTION__, VIA_RIL_PATH, dlsym_error);
        return;
    }

    // create an instance of ViaHandler
    m_viaHandler = createViaHandler();
    RFX_LOG_D(RFX_LOG_TAG, "[%s] completed", __FUNCTION__);
}

ViaBaseHandler* RfxViaUtils::getViaHandler() {
    return m_viaHandler;
}

void RfxViaUtils::deInitViaHandler() {
    if (m_dlHandler == NULL) {
        RFX_LOG_I(RFX_LOG_TAG, "[%s] return, m_dlHandler == NULL", __FUNCTION__);
        return;
    }

    destroy_t* destroyViaHandler = (destroy_t*) dlsym(m_dlHandler, "destroy");
    const char* dlsym_error = dlerror();
    if (destroyViaHandler == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] destroy not defined or exported in %s: %s",
                __FUNCTION__, VIA_RIL_PATH, dlsym_error);
        return;
    }

    destroyViaHandler(m_viaHandler);
    m_viaHandler = NULL;
    dlclose(m_dlHandler);
    m_dlHandler = NULL;
    RFX_LOG_D(RFX_LOG_TAG, "[%s] completed", __FUNCTION__);
}
