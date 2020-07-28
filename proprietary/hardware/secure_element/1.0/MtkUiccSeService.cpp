/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include "SecureElement.h"

#define LOG_TAG "MtkUiccSEService"
#include <log/log.h>

#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <stdio.h>
#include <string.h>

using android::OK;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::secure_element::V1_0::ISecureElement;
using android::hardware::secure_element::V1_0::implementation::SecureElement;
using android::sp;
using android::status_t;

bool IS_DEBUG = true; // control debug log

void checkDebug() {
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.build.type", prop_value, "");
    if (strcmp(prop_value, "user") == 0)
        IS_DEBUG = false;

    ALOGD("Debug = %x", IS_DEBUG);
}

int getNumberOfSim() {
    int numOfSim = 0;
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    char vsim_prop[PROPERTY_VALUE_MAX] = { 0 };

    property_get("persist.radio.multisim.config", prop_value, "dsds");
    if (strcmp(prop_value, "ss") == 0)
        numOfSim = 1;
    else if (strcmp(prop_value, "dsds") == 0)
        numOfSim = 2;
    else if (strcmp(prop_value, "dsda") == 0)
        numOfSim = 2;
    else if (strcmp(prop_value, "tsts") == 0) {
        // External SIM doesn't support
        property_get("ro.vendor.mtk_non_dsda_rsim_support", vsim_prop, "0");
        if (strcmp(vsim_prop, "1") == 0) {
            ALOGI("External SIM doesn't support secure element.");
            numOfSim = 2;
        } else {
            numOfSim = 3;
        }
    }
    else if (strcmp(prop_value, "qsqs") == 0)
        numOfSim = 4;

    ALOGD("getNumberOfSim() = %d", numOfSim);

    return numOfSim;
}

int main() {
    checkDebug();

    ALOGD("MTK UICC Secure Element Service is starting ...");
    configureRpcThreadpool(MAX_SE_BINDER_THREADS, true); /* one for myself, one for rild response */

    sp<ISecureElement> se_services[MAX_SIM_COUNT];
    int numOfSim = getNumberOfSim();
    if(numOfSim > MAX_SIM_COUNT) {
        LOG_ALWAYS_FATAL("SIM Number is bigger than Max SIM count");
        return -1;
    }

    for (int i=1; i <= numOfSim; ++i) {
        char name[8];
        sprintf(name, "SIM%d", i);

        se_services[i - 1] = new SecureElement(i);
        status_t status = se_services[i - 1]->registerAsService(name);
        if (status != OK) {
            LOG_ALWAYS_FATAL("Could not register service for UICC%d (%d)", i, status);
            return -1;
        }
    }

    ALOGD("MTK UICC Secure Element Service is ready.");
    joinRpcThreadpool();

    ALOGD("MTK UICC Secure Element Service exit.");
    return 1;
}
