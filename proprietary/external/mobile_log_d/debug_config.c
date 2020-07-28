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

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <log/log.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include "global_var.h"
#include "mlog.h"
#include "aee.h"

#define PROP_PRDEBUG  "vendor.MB.prdebug"

static char build_type[PROPERTY_VALUE_MAX];
static char aee_info[PROPERTY_VALUE_MAX];

void *handle;
extern AEE_SYS_FUNC *aee_sys_exp;
extern AEE_SYS_FUNC *aee_sys_war;

/* load dynamic shared libs -- libaed.so */
void load_dynamic_shared_libs() {
    handle = dlopen(LIB_AED, RTLD_NOW);

    if (handle == NULL) {
        MLOGE_DATA("dlopen libaed.so error: %s", dlerror());
    } else {
        // clear error
        dlerror();
        aee_sys_exp = (AEE_SYS_FUNC *)dlsym(handle, "aee_system_exception");
        if (aee_sys_exp == NULL)
            MLOGE_DATA("can not find aee_system_exception, %s", dlerror());

        aee_sys_war = (AEE_SYS_FUNC *)dlsym(handle, "aee_system_warning");
        if (aee_sys_war == NULL)
            MLOGE_DATA("can not find aee_system_warning, %s", dlerror());
    }
}

// Init the system debug configuration
void InitDebugConfig(void) {
    property_get("ro.build.type", build_type, "");  // treat userdebug build same as user build
    property_get("ro.vendor.aee.build.info", aee_info, "customer");

    if (build_type[0] != '\0') {
        if (!strcmp(build_type, "userdebug") || !strcmp(build_type, "user")) {
            property_set(PROP_PRDEBUG, "1");
            if (aee_info[0] != '\0')
                system(!strcmp(aee_info, "customer") ? "aee -m 3" : "aee -m 1");
            else
                MLOGE_BOTH("can not get property value of 'ro.aee.build.info'");
        }
    } else {
        if (aee_sys_war)
            aee_sys_war("mobile_log_d", NULL, DB_OPT_DEFAULT, "mobile_log_d getprop 'ro.build.type' fail");
        MLOGE_BOTH("can not get property value of 'ro.build.type'");
    }
}

// DeInit the system debug configuration
void DeInitDebugConfig(void) {
    if (!strcmp(build_type, "userdebug") || !strcmp(build_type, "user")) {
        property_set(PROP_PRDEBUG, "0");
        system(!strcmp(aee_info, "customer") ? "aee -m 4" : "aee -m 2");
    }
}
