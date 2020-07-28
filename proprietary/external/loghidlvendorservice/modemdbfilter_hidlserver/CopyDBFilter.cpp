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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   CopyDBFilter.cpp
 *
 * Description:
 * ------------
 *   HIDL service's implementation of ICopyDBFilter interface
 *
 * Author:
 * -------
 *   Bo.Shang (MTK80204) 04/16/2019
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *******************************************************************************/

#include "CopyDBFilter.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace modemdbfilter {
namespace V1_0 {
namespace implementation {

// Methods begin.

ModemDBFilterService::ModemDBFilterService(const char* name) {
    ALOGI("DB_HIDL server:%s.Version date:2019-6-20.1",name);
    strncpy(this->mServerName, name,strlen(name));
}

ModemDBFilterService::~ModemDBFilterService() {
    ALOGI("DB_HIDL server:%s.~ModemDBFilterService()",mServerName);
    releaseVector();
}

bool ModemDBFilterService::isExist(const char * pathname) {
    if (TEMP_FAILURE_RETRY(access(pathname, F_OK)) == 0) {
        return true;
    }
    ALOGE("DB_HIDL server:%s.[%s] access fail:%s", mServerName, pathname, strerror(errno));
    return false;
}

int ModemDBFilterService::insertFileToVec(const char * folder, bool mRecursive){
    DIR *dir = opendir(folder);
    if (dir == NULL) {
        ALOGE("DB_HIDL server:[%s] searchFiles %s fail:%s", mServerName, folder, strerror(errno));
        return 0;
    }
    struct dirent *node = NULL;
    int num_file = 0;
    char path[PATH_MAX_LEN];
    while ((node = readdir(dir)) != NULL) {
        if(!strcmp(node->d_name,"..") || !strcmp(node->d_name,".")){
            continue;
        }
        if (strlen(node->d_name) + strlen(folder) > PATH_MAX_LEN - 1) {
            ALOGE("DB_HIDL server:%s. searchFiles path len above 256", mServerName);
            continue;
        }
        snprintf(path, PATH_MAX_LEN - 1, "%s%s", folder, node->d_name);
        path[PATH_MAX_LEN - 1] = '\0';
        if (isdir(path) && node->d_name[0] != '.') {
            if (mRecursive) {
                num_file += countFiles(path,mRecursive);
            }
        } else {
            fileListVector[num_file] = path;
            num_file++;
        }
    }
    closedir(dir);
    return num_file;
}

void ModemDBFilterService::releaseVector() {
    if(fileListVector.size() > 0) {
        delete [] fileListVector.releaseData();
    }
    hidl_vec<hidl_string> zero_Vec;
    fileListVector = zero_Vec;
}

int ModemDBFilterService::searchFiles(const char * folder, bool mRecursive) {
    int mSize = countFiles(folder, mRecursive);
    if (mSize > 0) {
        fileListVector.resize(mSize);
        insertFileToVec(folder, mRecursive);
    }
    ALOGI("DB_HIDL server:%s.getFileListAndSize number %lu",mServerName,fileListVector.size());
    return fileListVector.size();
}

int ModemDBFilterService::countFiles(const char * folder, bool mRecursive) {
    DIR *dir = opendir(folder);
    if (dir == NULL) {
        ALOGE("DB_HIDL server:[%s] countFiles %s fail:%s", mServerName, folder, strerror(errno));
        return 0;
    }
    struct dirent *node = NULL;
    int num_file = 0;
    char path[PATH_MAX_LEN];
    while ((node = readdir(dir)) != NULL) {
        if(!strcmp(node->d_name,"..") || !strcmp(node->d_name,".")){
            continue;
        }
        if (strlen(node->d_name) + strlen(folder) > PATH_MAX_LEN - 1) {
            ALOGE("DB_HIDL server:%s.count path len above 256", mServerName);
            continue;
        }
        snprintf(path, PATH_MAX_LEN - 1, "%s%s", folder, node->d_name);
        path[PATH_MAX_LEN - 1] = '\0';
        if (isdir(path) && node->d_name[0] != '.') {
            if (mRecursive) {
                num_file += countFiles(path,mRecursive);
            }
        } else {
            num_file++;
        }
    }
    closedir(dir);
    return num_file;
}

int ModemDBFilterService::isdir(const char * file) {
    struct stat s;
    if (stat(file,&s) != 0) {
        ALOGE("DB_HIDL server:[%s] stat %s fail:%s", mServerName, file, strerror(errno));
        return 0;
    }
    return S_ISDIR(s.st_mode);
}

unique_fd ModemDBFilterService::openFile(const char * pathname){
    unique_fd fd(TEMP_FAILURE_RETRY(open(pathname, O_RDONLY | O_NONBLOCK)));
    return fd;
}

void ModemDBFilterService::freeHidlHandle(const hidl_handle& hd) {
    if (hd != nullptr) {
        native_handle_t *handle = const_cast<native_handle_t*>(static_cast<const native_handle_t*>(hd));
        if (handle != nullptr) {
            native_handle_close(handle);
            native_handle_delete(handle);
            handle = nullptr;
        }
    }
}

Return<bool> ModemDBFilterService::getFileListAndSize(const hidl_string& path, const sp<IFileListCallback>& fileListCB) {
    ALOGI("DB_HIDL server:%s.getFileListAndSize path: %s",mServerName,path.c_str());
    bool errval = true;
    releaseVector();
    if (isExist(path.c_str())) {
        if (isdir(path.c_str())) {
            searchFiles(path.c_str());
        } else {
            fileListVector.resize(1);
            fileListVector[0] = path.c_str();
            ALOGI("DB_HIDL server:%s. path is file. return itself. Array number %lu",mServerName,fileListVector.size());
        }
    } else {
        errval = false;
        ALOGW("DB_HIDL server:%s.return empty vector for no this folder:%s",mServerName, path.c_str());
    }
    if (!fileListCB->sendFileListToClient(fileListVector)) {
        ALOGE("DB_HIDL server:%s.fileListCB sendFileListToClient return fail",mServerName);
        errval = false;
    }
    releaseVector();
    return errval;
}

Return<bool> ModemDBFilterService::getFileData(const hidl_string& path, const sp<IFileDataCallback>& fileDataCB) {
    // fileDataCB is null need client fixed.
/*
    if (fileDataCB == nullptr) {
        ALOGE("DB_HIDL server:%s.fileDataCB is null",mServerName);
        return false;
    }
*/
    ALOGI("DB_HIDL server:getFileData: %s",path.c_str());
    if (!isExist(path.c_str())) {
        ALOGE("DB_HIDL server:%s.fileDataCB %s is NOT exist",mServerName, path.c_str());
        return false;
    }
    unique_fd mUFd = openFile(path.c_str());
    if(mUFd.get() < 0) {
        ALOGE("DB_HIDL server:[%s] open fail:%s", path.c_str(), strerror(errno));
        return false;
    }
    native_handle_t* const nativeHandle = native_handle_create(1, 0);
    nativeHandle->data[0] = mUFd.release();
    hidl_handle hd_file;
    hd_file.setTo(nativeHandle, true);
    if (!fileDataCB->sendFileFDToClient(hd_file)) {
        ALOGE("DB_HIDL server:%s.sendFileListToClient return fail",mServerName);
        close(mUFd.release());
        freeHidlHandle(hd_file);
        return false;
    }
    close(mUFd.release());
    freeHidlHandle(hd_file);
    return true;
}


// Methods end
}  // namespace implementation
}  // namespace V1_0
}  // namespace modemdbfilter
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
