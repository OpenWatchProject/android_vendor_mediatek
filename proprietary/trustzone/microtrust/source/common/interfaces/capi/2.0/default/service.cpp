/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#define IMSG_TAG "vendor.microtrust.hardware.capi@2.0-service"

#include "ClientApiDevice.h"
#include <hidl/HidlTransportSupport.h>
#include <imsg_log.h>
#include <vendor/microtrust/hardware/capi/2.0/IClientApiDevice.h>
#include <cutils/properties.h>

using vendor::microtrust::hardware::capi::V2_0::IClientApiDevice;
using vendor::microtrust::hardware::capi::V2_0::implementation::ClientApiDevice;

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;
using android::status_t;
using android::OK;

int main()
{

    IMSG_ERROR("Client Api Hardware service is starting");
    android::sp<android::UtClientApiDaemonProxy> proxy = android::UtClientApiDaemonProxy::getInstance();
    android::sp<IClientApiDevice> service = new ClientApiDevice(proxy);

    configureRpcThreadpool(2, true /* callerWillJoin */);

    status_t status = service->registerAsService();
    if (status == OK)
    {
        IMSG_ERROR("CApi Daemon HAL Ready.");
        property_set("vendor.soter.teei.capi", "ready");
        joinRpcThreadpool();
    }
    else
    {
        IMSG_ERROR("Could not register service (%d).", status);
        property_set("vendor.soter.teei.capi", "fail");
    }
    // In normal operation, we don't expect the thread pool to exit
    IMSG_ERROR("Client Api Hardware service is shutting down");

    return 1;
}
