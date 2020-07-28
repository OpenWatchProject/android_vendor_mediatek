/* Copyright Statement:
 *
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

#define MTK_LOG_ENABLE 1
#include "PplAgent.h"

#include <errno.h>
#include <hwbinder/IPCThreadState.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>

#include "libfile_op.h"

/* Has 3 type to store the ppl data:
 * 1. [NVRAM_MODE]: /data/nvram/dm/ppl_config:
 *   The origianl type, use nvram partition without write protection
 *   to write/read file.
 * 2. [RAW_DATA_MODE]: block device supplied by fs_mgr:
 *   For nvram partition with write protection on M MT6755, use this
 *   block to write/read raw data directly.
 * 3. [FILE_MODE]: /nvcfg/ppl/ppl_config:
 *   For nvram partition with write protection on & after M MT6797,
 *   use this ext4 partition to write/read file.
 */

using android::pplagent::PplAgent;

#ifdef MTK_WRITE_ON_PROTECTION
#ifndef RAW_DATA_PARTITION
#define CONTROL_DATA_ROOT_PATH "/mnt/vendor/nvcfg/ppl/"
#endif
#endif

#ifndef CONTROL_DATA_ROOT_PATH
#define CONTROL_DATA_ROOT_PATH "/mnt/vendor/nvdata/dm/"
#endif

#define CONTROL_DATA_FILE_PATH CONTROL_DATA_ROOT_PATH "ppl_config"

#define PPL_MNT_POINT "/ppl"

/* hard limit */
#define MAX_FILE_SIZE (4*1024)

#define STATUS_ENABLED 0x2
#define STATUS_LOCKED 0x4
#define STATUS_SIM_LOCKED 0x8

#define STORAGE_MODE_NVRAM      0x01
#define STORAGE_MODE_RAW_DATA   0x02
#define STORAGE_MODE_FILE       0x03

/// SECURITY:ALPS04103751 @{
#define SYSTEM_UID 1000
#define PHONE_UID  1001
/// @}

void convertVector2Array(std::vector<uint8_t> in, char* out) {
    int size = in.size();
    for(int i = 0; i < size; i++) {
        out[i] = in.at(i);
    }
}

void convertArray2Vector(const char* in, int len, std::vector<uint8_t>& out) {
    out.clear();
    for(int i = 0; i < len; i++) {
        out.push_back(in[i]);
    }
}

PplAgent::PplAgent() {
    initStorageMode();
    if (isRawDataPartition()) {
        readFstab();
        rawDataPath = getRawDataPath();
    }
    ALOGI("PplAgent created");
}

PplAgent::~PplAgent() {
    if (NULL != rawDataPath) {
        free(rawDataPath);
        rawDataPath = NULL;
    }
    ALOGI("PplAgent destroied");
}

int PplAgent::isWriteOnProtection() {
    #ifdef MTK_WRITE_ON_PROTECTION
        return 1;
    #endif
    return 0;
}

int PplAgent::isRawDataPartition() {
    #ifdef RAW_DATA_PARTITION
        return 1;
    #endif
    return 0;
}

void PplAgent::initStorageMode() {
    const char *str;
    if (isWriteOnProtection() && isRawDataPartition()) {
        storageMode = STORAGE_MODE_RAW_DATA;
        str = "STORAGE_MODE_RAW_DATA";
    } else if (isWriteOnProtection()) {
        storageMode = STORAGE_MODE_FILE;
        str = "STORAGE_MODE_FILE";;
    } else {
        storageMode = STORAGE_MODE_NVRAM;
        str = "STORAGE_MODE_NVRAM";
    }
    ALOGI("initStorageMode: %s", str);
}


Return<void> PplAgent::readControlData(readControlData_cb _hidl_cb) {
    if (!isCallerUidHaveReadPermission()) {
        return Void();
    }

    if (storageMode == STORAGE_MODE_RAW_DATA) {
        ALOGI("[readControlData] read raw data: %s", rawDataPath);
    } else {
        ALOGI("[readControlData] read file path: %s", CONTROL_DATA_FILE_PATH);
    }

    int fd = -1;
    int32_t size = 0;
    char *buff = NULL;
    union sizeUnion {
        int size;
        char buf[4];
    } su;

    // raw data mode
    if (storageMode == STORAGE_MODE_RAW_DATA) {
        if ((fd = open(rawDataPath, O_RDONLY)) >= 0) {
            if (read(fd, su.buf, 4) >= 0) {
                size = su.size;
                buff = (char *) malloc(size);
                if (read(fd, buff, size) >= 0) {
                    close(fd);
                    std::vector<uint8_t> tmp(size);
                    convertArray2Vector(buff, size, tmp);
                    free(buff);
                    buff = NULL;
                    ALOGI("[readControlData] read raw data success. size=%d", size);
                    _hidl_cb(tmp);
                    return Void();
                }
            }
            ALOGE("[readControlData] read raw data fail ! size=%d, error:%s",
                    size, strerror(errno));
            close(fd);
        } else {
            ALOGD("[readControlData] open raw data fail ! error:%s", strerror(errno));
        }
    //file mode
    } else if ((fd = open(CONTROL_DATA_FILE_PATH, O_RDONLY)) >= 0) {
        struct stat file_stat;
        bzero(&file_stat, sizeof(file_stat));
        if (stat(CONTROL_DATA_FILE_PATH, &file_stat) >= 0) {
            size = file_stat.st_size;
            buff = (char *) malloc(size);
            if (read(fd, buff, size) >= 0) {
                close(fd);
                std::vector<uint8_t> tmp(size);
                convertArray2Vector(buff, size, tmp);
                free(buff);
                buff = NULL;
                ALOGI("[readControlData] read file success. size=%d", size);
                _hidl_cb(tmp);
                return Void();
            } else {
                ALOGE("[readControlData] read file fail ! size=%d, error:%s",
                        size, strerror(errno));
            }
        }
        close(fd);
    } else {
        ALOGD("[readControlData] open file fail ! error:%s", strerror(errno));
    }
    if (NULL != buff) {
        free(buff);
        buff = NULL;
    }
    _hidl_cb({});
    return Void();
}

Return<int32_t> PplAgent::writeControlData(const hidl_vec<uint8_t>& data, int32_t size) {
    if (!isCallerUidHaveWritePermission()) {
        return 0;
    }
    ALOGD("[writeControlData] enter. data size = %d", size);
    if (data == NULL || size <= 0 || size > MAX_FILE_SIZE) {
        return 0;
    }

    int fd = -1;
    char buff[size];
    convertVector2Array(data, buff);

    //raw data mode
    if (storageMode == STORAGE_MODE_RAW_DATA) {
        if ((fd = open(rawDataPath, O_WRONLY | O_TRUNC)) >= 0) {
            write(fd, &size , 4);
            write(fd, buff, size);
            fsync(fd);
            close(fd);
            ALOGD("[writeControlData] Write to raw data done.");
            //wirte to file for debug
            /*if ((fd = open(CONTROL_DATA_FILE_PATH, O_CREAT | O_WRONLY | O_TRUNC, 0775)) >= 0) {
                write(fd, &size , 4);
                write(fd, buff, size);
                fsync(fd);
                close(fd);
                ALOGD("writeControlData to backup file exit");
            }*/
            return 1;
        }
    // file mode
    }else if ((fd = open(CONTROL_DATA_FILE_PATH, O_CREAT | O_WRONLY | O_TRUNC, 0775)) >= 0) {
        write(fd, buff, size);
        fsync(fd);
        close(fd);
        if (storageMode == STORAGE_MODE_NVRAM) {
            FileOp_BackupToBinRegionForDM();
        }
        ALOGI("[writeControlData] write to file done.");
        return 1;
    }
    ALOGE("[writeControlData] write to file open failed = %s", strerror(errno));
    return 0;
}

Return<int32_t> PplAgent::needLock() {
    int fd = -1;
    int offset;

    if (storageMode == STORAGE_MODE_RAW_DATA){
        if (-1 == (fd = open(rawDataPath, O_RDONLY))) {
            ALOGE("[needLock] Open raw data error = %s", strerror(errno));
            return 0;
        }
        offset = 5; //size offset 4
    } else {
        if (-1 == (fd = open(CONTROL_DATA_FILE_PATH, O_RDONLY))) {
            ALOGE("[needLock] Open file error = %s", strerror(errno));
            return 0;
        }
        offset = 1;
    }

    // get ControlData.status which is at the second byte
    if (-1 == lseek(fd, offset, SEEK_SET)) {
        ALOGE("[needLock] lseek %d byte error!", offset);
        close(fd);
        return 0;
    }
    char cstatus;
    if (-1 == read(fd, &cstatus, 1)) {
        ALOGE("[needLock] Get ControlData.status error = %s", strerror(errno));
        close(fd);
        return 0;
    }
    close(fd);

    int istatus = cstatus;
    ALOGI("[needLock] status = %d\n", istatus);
    if ((istatus & STATUS_ENABLED) == STATUS_ENABLED
        && ((istatus & STATUS_LOCKED) == STATUS_LOCKED
        || (istatus & STATUS_SIM_LOCKED) == STATUS_SIM_LOCKED)) {
        return 1;
    } else {
        return 0;
    }
}

/// SECURITY:ALPS04103751 @{
bool PplAgent::isCallerUidHaveReadPermission() {
    hardware::IPCThreadState* ipcState = hardware::IPCThreadState::selfOrNull();
    if (ipcState == NULL) {
        ALOGE("isCallerUidHaveReadPermission, ipcState is null");
        return false;
    }
    int uid = ipcState->getCallingUid();
    // check read UID permission
    if (uid == SYSTEM_UID || uid == PHONE_UID) {
        return true;
    }
    ALOGE("isCallerUidHaveReadPermission Fail!(uid:%d)", uid);
    return false;
}

bool PplAgent::isCallerUidHaveWritePermission() {
    hardware::IPCThreadState* ipcState = hardware::IPCThreadState::selfOrNull();
    if (ipcState == NULL) {
        ALOGE("isCallerUidHaveWritePermission, ipcState is null");
        return false;
    }
    int uid = ipcState->getCallingUid();
    // check wirte UID permission
    if (uid == SYSTEM_UID) {
        return true;
    }
    ALOGE("isCallerUidHaveWritePermission Fail! (uid:%d)", uid);
    return false;
}
/// @}

int PplAgent::readFstab() {
    if (!ReadDefaultFstab(&fstab)) {
        ALOGD("failed to open default");
        return -1;
    }
    return 0;
}

char* PplAgent::getRawDataPath() {
    char *source = NULL;

    auto rec = GetEntryForMountPoint(&fstab, PPL_MNT_POINT);
    if (rec == nullptr) {
        ALOGE("getRawDataPath failed for entry %s\n", PPL_MNT_POINT);
        return NULL;
    }
    asprintf(&source, "%s", rec->blk_device.c_str());
    ALOGD("getRawDataPath = %s\n", source);
    return source;
}

int main() {
    umask(000);
    ALOGD("CONTROL_DATA_ROOT_PATH = %s", CONTROL_DATA_ROOT_PATH);
    if (-1 == access(CONTROL_DATA_ROOT_PATH, F_OK)) {
        if (-1 == mkdir(CONTROL_DATA_ROOT_PATH, 0775)) {
            ALOGD("make control data path error = %s", strerror(errno));
            return 0;
        }
    }

    android::sp<IPplAgent> agent = new PplAgent();
    configureRpcThreadpool(1, true);
    status_t status = agent->registerAsService();
    if (status != android::OK) {
        ALOGW("Register FAILED. status=%d", status);
        return status;
    }

    joinRpcThreadpool();
    return 0;
}

