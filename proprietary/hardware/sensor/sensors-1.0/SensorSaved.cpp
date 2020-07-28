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

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <log/log.h>

#include "file.h"
#include "JSONObject.h"
#include "SensorSaved.h"

#undef LOG_TAG
#define LOG_TAG "SensorSaved"

#define SENSOR_SAVED_NVCFG_ROOT_DIR         "/mnt/vendor/nvcfg"
#define SENSOR_SAVED_NVCFG_SENSOR_DIR       "/mnt/vendor/nvcfg/sensor"
#define SENSOR_SAVED_DATA_VENDOR_SENSOR_DIR "/data/vendor/sensor"

namespace android {

static bool vendorNvcfgPathValided() {
    static bool vendorNvcfgValid = true, firstEnter = true;

    if (!firstEnter)
        return vendorNvcfgValid;
    if (access(SENSOR_SAVED_NVCFG_ROOT_DIR, F_OK)) {
        ALOGE("(%s) not exist\n", SENSOR_SAVED_NVCFG_ROOT_DIR);
        vendorNvcfgValid = false;
        mkdir(SENSOR_SAVED_DATA_VENDOR_SENSOR_DIR, 0774);
    } else {
        ALOGE("(%s) exist\n", SENSOR_SAVED_NVCFG_ROOT_DIR);
        vendorNvcfgValid = true;
        mkdir(SENSOR_SAVED_NVCFG_SENSOR_DIR, 0774);
    }
    firstEnter = false;

    return vendorNvcfgValid;
}

SensorSaved::SensorSaved() {
    memset(sensorSavedPath, 0, sizeof(sensorSavedPath));
    if (vendorNvcfgPathValided())
        strlcpy(sensorSavedPath, SENSOR_SAVED_NVCFG_SENSOR_DIR, sizeof(sensorSavedPath));
    else
        strlcpy(sensorSavedPath, SENSOR_SAVED_DATA_VENDOR_SENSOR_DIR, sizeof(sensorSavedPath));
    strlcat(sensorSavedPath, "/", sizeof(sensorSavedPath));
    sensorSavedPathLength = strlen(sensorSavedPath);
}

SensorSaved::~SensorSaved() {
}

static sp<JSONObject> readSettings(File *file) {
    off64_t size = file->seekTo(0, SEEK_END);
    file->seekTo(0, SEEK_SET);

    sp<JSONObject> root;

    if (size > 0) {
        char *buf = (char *)malloc(size);

        if (buf != NULL) {
            CHECK_EQ(file->read(buf, size), (ssize_t)size);
            file->seekTo(0, SEEK_SET);

            sp<JSONCompound> in = JSONCompound::Parse(buf, size);
            free(buf);
            buf = NULL;

            if (in != NULL && in->isObject()) {
                root = (JSONObject *)in.get();
            }
        }
    }

    if (root == NULL) {
        root = new JSONObject;
    }

    return root;
}

static void loadSensorSettings(const char *setting_dir, sp<JSONObject>* settings) {
    File settings_file(setting_dir, "r");

    status_t err;
    if ((err = settings_file.initCheck()) != OK) {
        ALOGE("settings file(%s) open failed: %d (%s)",
              setting_dir,
              err,
              strerror(-err));

        *settings = new JSONObject;
    } else {
        *settings = readSettings(&settings_file);
    }
}

bool SensorSaved::getCalibrationInt32(
        const char *setting_dir, const char *key, int32_t *output,
        size_t numArgs) {
    Mutex::Autolock _l(mLock);
    sp<JSONArray> array;
    sp<JSONObject> settings;

    strlcpy(&sensorSavedPath[sensorSavedPathLength], setting_dir,
        sizeof(sensorSavedPath) - sensorSavedPathLength);
    loadSensorSettings(sensorSavedPath, &settings);
    if (!settings->getArray(key, &array)) {
        return false;
    } else {
        for (size_t i = 0; i < numArgs; i++) {
            if (!array->getInt32(i, &output[i])) {
                return false;
            }
        }
    }
    return true;
}

bool SensorSaved::getCalibrationFloat(
        const char *setting_dir, const char *key, float *output,
        size_t numArgs) {
    Mutex::Autolock _l(mLock);
    sp<JSONArray> array;
    sp<JSONObject> settings;

    strlcpy(&sensorSavedPath[sensorSavedPathLength], setting_dir,
        sizeof(sensorSavedPath) - sensorSavedPathLength);
    loadSensorSettings(sensorSavedPath, &settings);
    if (!settings->getArray(key, &array)) {
        return false;
    } else {
        for (size_t i = 0; i < numArgs; i++) {
            if (!array->getFloat(i, &output[i])) {
                return false;
            }
        }
    }
    return true;
}

void SensorSaved::saveCalibrationFloat(const char *setting_dir,
        const char *name, float *input, size_t numArgs) {
    if (!input)
        return;
    Mutex::Autolock _l(mLock);
    strlcpy(&sensorSavedPath[sensorSavedPathLength], setting_dir,
        sizeof(sensorSavedPath) - sensorSavedPathLength);
    File saved_settings_file(sensorSavedPath, "w");

    status_t err;
    if ((err = saved_settings_file.initCheck()) != OK) {
        ALOGE("saved settings(%s) file open failed %d (%s)",
              sensorSavedPath,
              err,
              strerror(-err));
        return;
    }

    // Build a settings object.
    sp<JSONArray> mArray = new JSONArray;

    for (size_t i = 0; i < numArgs; i++) {
        mArray->addFloat(input[i]);
    }

    sp<JSONObject> settingsObject = new JSONObject;
    settingsObject->setArray(name, mArray);

    // Write the JSON string to disk.
    AString serializedSettings = settingsObject->toString();
    size_t size = serializedSettings.size();
    if ((err = saved_settings_file.write(serializedSettings.c_str(), size)) != (ssize_t)size) {
        ALOGE("saved settings file write failed %d (%s)",
              err,
              strerror(-err));
    }
}

void SensorSaved::saveCalibrationInt32(const char *setting_dir,
        const char *name, int32_t *input, size_t numArgs) {
    if (!input)
        return;
    Mutex::Autolock _l(mLock);
    strlcpy(&sensorSavedPath[sensorSavedPathLength], setting_dir,
        sizeof(sensorSavedPath) - sensorSavedPathLength);
    File saved_settings_file(sensorSavedPath, "w");

    status_t err;
    if ((err = saved_settings_file.initCheck()) != OK) {
        ALOGE("saved settings(%s) file open failed %d (%s)",
              sensorSavedPath,
              err,
              strerror(-err));
        return;
    }

    // Build a settings object.
    sp<JSONArray> mArray = new JSONArray;

    for (size_t i = 0; i < numArgs; i++) {
        mArray->addInt32(input[i]);
    }

    sp<JSONObject> settingsObject = new JSONObject;
    settingsObject->setArray(name, mArray);

    // Write the JSON string to disk.
    AString serializedSettings = settingsObject->toString();
    size_t size = serializedSettings.size();
    if ((err = saved_settings_file.write(serializedSettings.c_str(), size)) != (ssize_t)size) {
        ALOGE("saved settings file write failed %d (%s)",
              err,
              strerror(-err));
    }
}

}
