/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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

#include <utils/Log.h>
#include "SensorContext.h"
#include "SensorManager.h"
#include "VendorInterface.h"
#include "MtkInterface.h"
#include "SensorCalibration.h"
#include "SensorList.h"
#include "DirectChannelManager.h"

#undef LOG_TAG
#define LOG_TAG "Sensors"

static SensorManager *mSensorManager;
static SensorConnection *mNativeConnection;
static VendorInterface *mVendorInterface;
static MtkInterface *mMtkInterface;
static SensorCalibration *mSensorCalibration;
static SensorList *mSensorList;
static DirectChannelManager *mDirectChannelManager;

static int poll__close(struct hw_device_t *dev) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx)
        delete ctx;
    mSensorManager->removeSensorConnection(mNativeConnection);
    delete mSensorManager;
    delete mVendorInterface;
    delete mMtkInterface;
    delete mSensorCalibration;
    delete mSensorList;
    delete mDirectChannelManager;
    mSensorManager = nullptr;
    mVendorInterface = nullptr;
    mMtkInterface = nullptr;
    mSensorCalibration = nullptr;
    mSensorList = nullptr;
    mDirectChannelManager = nullptr;
    return 0;
}

static int poll__activate(struct sensors_poll_device_t * /*dev*/,
        int handle, int enabled) {
    return mSensorManager->activate(mNativeConnection, handle - ID_OFFSET, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t * /*dev*/,
        int handle, int64_t ns) {
    return mSensorManager->batch(mNativeConnection, handle - ID_OFFSET, ns, 0);
}

static int poll__poll(struct sensors_poll_device_t * /*dev*/,
        sensors_event_t* data, int count) {
    return mSensorManager->pollEvent(data, count);
}

static int poll__batch(struct sensors_poll_device_1 * /*dev*/,
        int handle, int /*flags*/, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    return mSensorManager->batch(mNativeConnection,
        handle - ID_OFFSET, samplingPeriodNs, maxBatchReportLatencyNs);
}

static int poll__flush(struct sensors_poll_device_1 * /*dev*/,
        int handle) {
    return mSensorManager->flush(mNativeConnection, handle - ID_OFFSET);
}

#ifdef SUPPORT_DIRECT_CHANNEL
static int poll__registerDirectChannel(struct sensors_poll_device_1 * /*dev*/,
        const struct sensors_direct_mem_t *mem, int channel_handle) {
    int ret = 0;

    if (mem) {
        ret = mDirectChannelManager->addDirectChannel(mem);
    } else {
        mDirectChannelManager->removeDirectChannel(channel_handle - ID_OFFSET);
        ret = 0;
    }
    return ret;
}

static int poll__configDirectReport(struct sensors_poll_device_1 * /*dev*/,
        int sensor_handle, int channel_handle,
        const sensors_direct_cfg_t *config) {
    int rate_level = config->rate_level;
    return mDirectChannelManager->configDirectReport(sensor_handle - ID_OFFSET,
        channel_handle, rate_level);
}
#endif

static int poll__injectSensorData(struct sensors_poll_device_1 * /*dev*/,
        const sensors_event_t * /*event*/) {
    return 0;
}

static int init_sensors(hw_module_t const* module, hw_device_t** device)
{
    sensors_poll_context_t *dev;
    size_t count = 0;
    const struct sensor_t *list = NULL;

    /* must initialize sensorlist first for hwgyrosupport */
    mSensorList = SensorList::getInstance();
    count = mSensorList->getSensorList(&list);

    dev = sensors_poll_context_t::getInstance();
    memset(&dev->device, 0, sizeof(sensors_poll_device_1));

    dev->device.common.tag      = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = SENSORS_DEVICE_API_VERSION_1_4;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;
    dev->device.batch           = poll__batch;
    dev->device.flush           = poll__flush;
    dev->device.inject_sensor_data = poll__injectSensorData;
#ifdef SUPPORT_DIRECT_CHANNEL
    dev->device.register_direct_channel = poll__registerDirectChannel;
    dev->device.config_direct_report = poll__configDirectReport;
#endif

    *device = &dev->device.common;

    mSensorManager = SensorManager::getInstance();
    mSensorManager->addSensorsList(list, count);
    mNativeConnection = mSensorManager->createSensorConnection(numFds);
    mSensorManager->setNativeConnection(mNativeConnection);
    mSensorManager->setSensorContext(dev);
    mVendorInterface = VendorInterface::getInstance();
    mMtkInterface = MtkInterface::getInstance();
    mSensorCalibration = SensorCalibration::getInstance();
    mDirectChannelManager = DirectChannelManager::getInstance();
    return 0;
}

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
   ALOGD("%s: name=%s!\n", __func__, name);
   return init_sensors(module, device);
}

static int sensors__get_sensors_list(struct sensors_module_t* /*module*/,
        struct sensor_t const** list) {
    size_t count = mSensorList->getSensorList(list);
    ALOGD("%s count=%zu!\n", __func__, count);
    return count;
}

static int set_operation_mode(unsigned int mode) {
    ALOGV("%s: mode=%u!", __func__, mode);
    return -EINVAL;
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "MTK SENSORS Module",
        .author = "Mediatek",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list,
    .set_operation_mode = set_operation_mode,
};
