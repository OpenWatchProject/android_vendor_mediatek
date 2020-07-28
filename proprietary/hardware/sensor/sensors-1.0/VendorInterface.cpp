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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <hardware/sensors.h>
#include <utils/Log.h>
#include <linux/hwmsensor.h>
#include "SensorList.h"

#include "VendorInterface.h"

#undef LOG_TAG
#define LOG_TAG "VendorInterface"

#define LIB_NAME_LENGTH 64

struct mag_libinfo_t {
    char libname[LIB_NAME_LENGTH];
    int32_t layout;
    int32_t deviceid;
};

VendorInterface *VendorInterface::VendorInterfaceInstance = nullptr;
VendorInterface *VendorInterface::getInstance() {
    if (VendorInterfaceInstance == nullptr) {
        VendorInterface *mInterface = new VendorInterface;
        VendorInterfaceInstance = mInterface;
    }
    return VendorInterfaceInstance;
}

// coverity[ +tainted_string_sanitize_content : arg-0 ]
static int isValidLibName(char *libname) {
    if (strlen(libname) > (LIB_NAME_LENGTH - 3 - 3 - 1)) { /* 3bytes "lib", 3bytes ".so", 1byte "\n" */
        ALOGE("invalid libname, length too long\n");
        return 0;
    } else {
        return 1;
    }
}

VendorInterface::VendorInterface() {
    int fd = -1;
    int len = 0;
    char buf[64] = {0};
    struct mag_libinfo_t libinfo;
    struct magChipInfo chipinfo;
    SensorList *mSensorList = nullptr;

    ALOGI("VendorInterface constructor.");

    lib_interface = NULL;
    lib_handle = NULL;
    memset(&libinfo, 0, sizeof(struct mag_libinfo_t));
    memset(&chipinfo, 0, sizeof(struct magChipInfo));

    fd = TEMP_FAILURE_RETRY(open("/sys/class/sensor/m_mag_misc/maglibinfo", O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, &libinfo, sizeof(struct mag_libinfo_t)));
        if (len <= 0) {
            ALOGE("read libinfo err, len = %d\n", len);
            close(fd);
            return;
        }
        close(fd);
    } else {
        ALOGE("open vendor libinfo fail\n");
        return;
    }
    libinfo.libname[LIB_NAME_LENGTH - 1] = '\0';
    if (!isValidLibName(libinfo.libname)) {
        ALOGE("invalid libname, length too long\n");
        return;
    }
    strlcpy(buf, "lib", sizeof(buf));
    strlcat(buf, libinfo.libname, sizeof(buf));
    strlcat(buf, ".so", sizeof(buf));

    lib_handle = dlopen(buf, RTLD_NOW);
    if (!lib_handle) {
        ALOGE("get lib_interface fail dlopen operation.\n");
        lib_handle = NULL;
        return;
    }
    dlerror();
    lib_interface = (struct mag_lib_interface_t *)dlsym(lib_handle, "MAG_LIB_API_INTERFACE");
    if (!lib_interface) {
        ALOGE("get lib_interface fail dlsym operation.\n");
        dlclose(lib_handle);
        lib_handle = NULL;
        return;
    }
    ALOGI("get lib_interface success.");

    mSensorList = SensorList::getInstance();
    chipinfo.hwGyro = mSensorList->hwGyroSupported();
    chipinfo.deviceid = libinfo.deviceid;
    chipinfo.layout = libinfo.layout;
    lib_interface->initLib(&chipinfo);
}

VendorInterface::~VendorInterface() {
    if (lib_handle) {
        dlclose(lib_handle);
        lib_handle = NULL;
        lib_interface = NULL;
    }
}
int VendorInterface::magInitLib() {
    return 0;
}

int VendorInterface::magEnableLib(int en) {
    if (!lib_interface)
        return -1;
    if (!lib_interface->enableLib)
        return -1;
    return lib_interface->enableLib(en);
}

int VendorInterface::setMagOffset(float offset[3]) {
    if (!lib_interface)
        return -1;
    if (!lib_interface->caliApiSetOffset)
        return -1;
    return lib_interface->caliApiSetOffset(offset);
}

int VendorInterface::getMagOffset(float offset[3]) {
    if (!lib_interface)
        return -1;
    if (!lib_interface->caliApiGetOffset)
        return -1;
    return lib_interface->caliApiGetOffset(offset);
}

int VendorInterface::setGyroData(struct sensorData *inputData) {
    struct magCaliDataInPut magInputData;

    memset(&magInputData, 0, sizeof(struct magCaliDataInPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->caliApiSetGyroData)
        return -1;
    magInputData.timeStamp = inputData->timeStamp;
    magInputData.x = inputData->data[0];
    magInputData.y = inputData->data[1];
    magInputData.z = inputData->data[2];
    magInputData.status = inputData->status;
    return lib_interface->caliApiSetGyroData(&magInputData);
}

int VendorInterface::setAccData(struct sensorData *inputData) {
    struct magCaliDataInPut magInputData;

    memset(&magInputData, 0, sizeof(struct magCaliDataInPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->caliApiSetAccData)
        return -1;
    magInputData.timeStamp = inputData->timeStamp;
    magInputData.x = inputData->data[0];
    magInputData.y = inputData->data[1];
    magInputData.z = inputData->data[2];
    magInputData.status = inputData->status;
    return lib_interface->caliApiSetAccData(&magInputData);
}

int VendorInterface::setMagData(struct sensorData *inputData) {
    struct magCaliDataInPut magInputData;

    memset(&magInputData, 0, sizeof(struct magCaliDataInPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->caliApiSetMagData)
        return -1;
    magInputData.timeStamp = inputData->timeStamp;
    magInputData.x = inputData->data[0];
    magInputData.y = inputData->data[1];
    magInputData.z = inputData->data[2];
    magInputData.status = inputData->status;
    return lib_interface->caliApiSetMagData(&magInputData);
}

int VendorInterface::magCalibration(struct sensorData *inputData,
    struct sensorData *outputData) {
    struct magCaliDataInPut magInputData;
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magInputData, 0, sizeof(struct magCaliDataInPut));
    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->doCaliApi)
        return -1;
    magInputData.timeStamp = inputData->timeStamp;
    magInputData.x = inputData->data[0];
    magInputData.y = inputData->data[1];
    magInputData.z = inputData->data[2];
    magInputData.status = inputData->status;
    ret = lib_interface->doCaliApi(&magInputData, &magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->data[3] = magOutputData.x_bias;
    outputData->data[4] = magOutputData.y_bias;
    outputData->data[5] = magOutputData.z_bias;
    outputData->status = magOutputData.status;
    return ret;
}
int VendorInterface::getGravity(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getGravity)
        return -1;
    ret = lib_interface->getGravity(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getRotationVector(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getRotationVector)
        return -1;
    ret = lib_interface->getRotationVector(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->data[3] = magOutputData.x_bias;
    outputData->data[4] = magOutputData.y_bias;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getOrientation(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getOrientation)
        return -1;
    ret = lib_interface->getOrientation(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getLinearaccel(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getLinearaccel)
        return -1;
    ret = lib_interface->getLinearaccel(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getGameRotationVector(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getGameRotationVector)
        return -1;
    ret = lib_interface->getGameRotationVector(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->data[3] = magOutputData.x_bias;
    outputData->data[4] = magOutputData.y_bias;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getGeoMagnetic(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getGeoMagnetic)
        return -1;
    ret = lib_interface->getGeoMagnetic(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->data[3] = magOutputData.x_bias;
    outputData->data[4] = magOutputData.y_bias;
    outputData->status = magOutputData.status;
    return ret;
}

int VendorInterface::getVirtualGyro(struct sensorData *outputData) {
    struct magCaliDataOutPut magOutputData;
    int ret = 0;

    memset(&magOutputData, 0, sizeof(struct magCaliDataOutPut));
    if (!lib_interface)
        return -1;
    if (!lib_interface->getVirtualGyro)
        return -1;
    ret = lib_interface->getVirtualGyro(&magOutputData);
    outputData->timeStamp = magOutputData.timeStamp;
    outputData->data[0] = magOutputData.x;
    outputData->data[1] = magOutputData.y;
    outputData->data[2] = magOutputData.z;
    outputData->status = magOutputData.status;
    return ret;
}

