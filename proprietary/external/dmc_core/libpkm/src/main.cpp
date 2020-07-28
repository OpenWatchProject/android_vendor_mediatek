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

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "Logs.h"
#include "libpkm.h"

// PKM_Start_Service(void);
typedef int (*__PKM_Start_Service)(void);
// PKM_Start_Service(void);
typedef void (*__PKM_Stop_Service)(void);

bool load_PKM(const char *module);
void unload_PKM();

void* sRefLibPKM = 0;
__PKM_Start_Service PKM_Start_Service = NULL;
__PKM_Stop_Service PKM_Stop_Service = NULL;

/***************************************************************************************************
 * @method startService
 * @brief Start Packets Monitor Service
 **************************************************************************************************/
bool load_PKM(const char *module) {

    if (sRefLibPKM != 0) {
        unload_PKM();
    }

    sRefLibPKM = dlopen(module, RTLD_GLOBAL);
    if (sRefLibPKM == 0) {
        return false;
    }

    static const char *PKM_FN_NAME_START_SERVICE = "startService";
    static const char *PKM_FN_NAME_STOP_SERVICE = "stopService";

    PKM_Start_Service = (__PKM_Start_Service) dlsym(sRefLibPKM, PKM_FN_NAME_START_SERVICE);
    if (PKM_Start_Service == 0) {
        unload_PKM();
        return false;
    }

    PKM_Stop_Service = (__PKM_Stop_Service) dlsym(sRefLibPKM, PKM_FN_NAME_STOP_SERVICE);
    if (PKM_Stop_Service == 0) {
        unload_PKM();
        return false;
    }

    return true;
}

/***************************************************************************************************
 * @method stopService
 * @brief Stop Packets Monitor Service
 **************************************************************************************************/
void unload_PKM() {

    if (sRefLibPKM == 0) {
        // Do nothing
    } else {
        dlclose(sRefLibPKM);
        sRefLibPKM = 0;
    }

    PKM_Start_Service = 0;
    PKM_Stop_Service = 0;
}

// Log TAG
const static char* TAG = "PKM-Service";


int main(void) {

    const char name[] = "libpkm.so";
    int rc = -1;
    if(load_PKM(name)) {
        PKM_LOGD("PacketMonitorService starting ...");

        // It will block here until process being killed
        if((rc = PKM_Start_Service()) == PKM_OK) {
            PKM_LOGE("PacketMonitorService leave ...");
        }
        unload_PKM();
    }

    return 0;
}
