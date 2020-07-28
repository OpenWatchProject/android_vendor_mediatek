    /*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "FingerprintHal"
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IInterface.h>
#include "IGoodixFingerprint.h"
#include "GFDevice.h"
using namespace android;

static IGoodixFingerprintBase *gGoodixFingerprintBase = GFDevice::getInstance()->getFingerprintBase();
static int fingerprint_close(hw_device_t *dev) {
    ALOGE("fingerprint_close");
    if (dev != NULL) {
        free(dev);
        dev = NULL;
    }

    GFDevice::getInstance()->closeHal();
    return 0;

}

static uint64_t fingerprint_pre_enroll(struct fingerprint_device __unused *dev) {
    ALOGE("fingerprint_pre_enroll");
    return gGoodixFingerprintBase->preEnroll();
}

static int fingerprint_enroll(struct fingerprint_device __unused *dev,
                                const hw_auth_token_t *hat,
                                uint32_t gid,
                                uint32_t timeout_sec) {
    ALOGE("fingerprint_enroll");
    return gGoodixFingerprintBase->enroll((const void *)hat, gid, timeout_sec);
}

static int fingerprint_post_enroll(struct fingerprint_device __unused *dev) {
    ALOGE("fingerprint_post_enroll");
    return gGoodixFingerprintBase->postEnroll();
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device __unused *dev) {
    ALOGE("fingerprint_get_auth_id");
    return gGoodixFingerprintBase->getAuthenticatorId();
}

static int fingerprint_cancel(struct fingerprint_device __unused *dev) {
    ALOGE("fingerprint_cancel");
    return gGoodixFingerprintBase->cancel();;
}

static int fingerprint_remove(struct fingerprint_device __unused *dev,
                                uint32_t gid, uint32_t fid) {
    ALOGE("fingerprint_remove");
    return gGoodixFingerprintBase->remove(gid, fid);
}

static int fingerprint_set_active_group(struct fingerprint_device __unused *dev,
                                        uint32_t __unused gid, const char __unused *store_path) {
    ALOGE("fingerprint_set_active_group");

    gGoodixFingerprintBase->setActiveGroup(gid);
	return 0;
}

static int fingerprint_authenticate(struct fingerprint_device __unused *dev,
                                    uint64_t operation_id, uint32_t gid) {
    ALOGE("fingerprint_authenticate");
    return gGoodixFingerprintBase->authenticate(operation_id, gid);
}

static int fingerprint_enumerate(struct fingerprint_device __unused *dev) {
    ALOGE("fingerprint_enumerate");
    return gGoodixFingerprintBase->enumerate();
}

static int set_notify_callback(struct fingerprint_device *dev, fingerprint_notify_t notify) {
    ALOGE("set_notify_callback");
    /* Decorate with locks */
    gGoodixFingerprintBase->setNotifyCallback((void*)notify);
    return 0;
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{
    ALOGE("fingerprint_open");
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    fingerprint_device_t *dev = (fingerprint_device_t *)malloc(sizeof(fingerprint_device_t));
    memset(dev, 0, sizeof(fingerprint_device_t));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = FINGERPRINT_MODULE_API_VERSION_2_1;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = fingerprint_close;
    dev->pre_enroll = fingerprint_pre_enroll;
    dev->enroll = fingerprint_enroll;
    dev->post_enroll = fingerprint_post_enroll;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->authenticate = fingerprint_authenticate;
    dev->set_notify = set_notify_callback;
    dev->notify = NULL;
    dev->enumerate = fingerprint_enumerate;
    *device = (hw_device_t*) dev;

    if(GF_SUCCESS != GFDevice::getInstance()->openHal())
    {
        ALOGE("open goodix hal failed");
        free(dev);
        dev = NULL;
        return -EINVAL;
    }

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_1,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
#ifdef SUPPORT_MULTI_FINGERPRINT_VENDOR
        .id                 = "gf_fingerprint",
#else
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
#endif
        .name               = "Goodix Fingerprint HAL",
        .author             = "Goodix",
        .methods            = &fingerprint_module_methods,
        .dso                = NULL,
        .reserved = { 0 }
    },
};
