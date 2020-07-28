 /*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include "RilOpProxy.h"
#include <libmtkrilutils.h>


/*****************************************************************************
 * Define
 *****************************************************************************/
#undef LOG_TAG
#define LOG_TAG "RilOpProxy"


/*****************************************************************************
 * Class RilOpProxy
 *****************************************************************************/

#define OP_RIL_PATH "libmtk-rilop.so"
void* RilOpProxy::sDlOpHandler = NULL;

void RilOpProxy::initOpLibrary() {
    if (sDlOpHandler != NULL) {
        RLOGI("[%s] return, sDlOpHandler = %p", __FUNCTION__, sDlOpHandler);
        return;
    }

    sDlOpHandler = dlopen(OP_RIL_PATH, RTLD_NOW);
    if (sDlOpHandler == NULL) {
        RLOGI("[%s] dlopen failed in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlerror());
        return;
    }

    // reset errors
    dlerror();

    RLOGI("[%s] completed", __FUNCTION__);
}

void* RilOpProxy::getOpHandler() {
    if (isOMSupport()) {
        RLOGI("[%s] getOpHandler failed in %s: %s", __FUNCTION__, OP_RIL_PATH, dlerror());
        return NULL;
    }
    return sDlOpHandler;
}

void RilOpProxy::registerOpService(
        RIL_RadioFunctions *callbacks, android::CommandInfo *commands) {
    if (sDlOpHandler == NULL) {
        RLOGI("[%s] dlopen failed in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlerror());
        return;
    }

    // reset errors
    dlerror();

    RLOGI("[%s] completed", __FUNCTION__);

    void (*func)(RIL_RadioFunctions *, android::CommandInfo *);
    func = (void(*)(RIL_RadioFunctions *, android::CommandInfo *))
            dlsym(sDlOpHandler, "registerOpService");
    const char* dlsym_error = dlerror();
    if (func == NULL) {
        RLOGI("[%s] destroy not defined or exported in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlsym_error);
        return;
    }

    func(callbacks, commands);
    RLOGI("[%s] completed", __FUNCTION__);
}

android::CommandInfo *RilOpProxy::getOpCommandInfo(int request) {
    android::CommandInfo *pCi;
    if (sDlOpHandler == NULL || isOMSupport()) {
        RLOGI("[%s] dlopen failed in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlerror());
        return NULL;
    }

    android::CommandInfo *(*func)(int request);
    func = (android::CommandInfo *(*)(int request))
            dlsym(sDlOpHandler, "getOpCommandInfo");
    const char* dlsym_error = dlerror();
    if (func == NULL) {
        RLOGI("[%s] destroy not defined or exported in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlsym_error);
        return NULL;
    }

    pCi = func(request);
    RLOGI("[%s] completed", __FUNCTION__);

    return pCi;
}

android::UnsolResponseInfo *RilOpProxy::getOpUnsolResponseInfo(
        int unsolResponse) {
    if (sDlOpHandler == NULL || isOMSupport()) {
        RLOGI("[%s] dlopen failed in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlerror());
        return NULL;
    }

    android::UnsolResponseInfo *pUnsolResponseInfo = NULL;

    android::UnsolResponseInfo *(*func)(int unsolResponse);
    func = (android::UnsolResponseInfo *(*)(int unsolResponse))
            dlsym(sDlOpHandler, "getOpUnsolResponseInfo");
    const char* dlsym_error = dlerror();
    if (func == NULL) {
        RLOGI("[%s] destroy not defined or exported in %s: %s",
                __FUNCTION__, OP_RIL_PATH, dlsym_error);
        return NULL;
    }

    pUnsolResponseInfo = func(unsolResponse);
    RLOGI("[%s] completed", __FUNCTION__);

    return pUnsolResponseInfo;
}

