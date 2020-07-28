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
 *   HIDL client's implementation of ICopyDBFilter interface and its callbacks
 *
 * Author:
 * -------
 *   Bo.Shang (MTK80204) 04/18/2019
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

// Methods start
Return<bool> FileDataCallback::sendFileFDToClient(const hidl_handle& hd) {
    return mHidlClient->saveDataToFile(hd);
}

Return<bool> FileListCallback::sendFileListToClient(const hidl_vec<hidl_string>& data) {
    return mHidlClient->insertFileListData(data);
}

void DBFilterHidlServerDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    cookie = 0;
    wp<IBase> w = who;
    mHidlClient->handleHidlDeath();
}

CopyModemDBFilterClient::CopyModemDBFilterClient(const char* name) {
    mode_t pre_mask = umask(000);
    ALOGI("2019-09-05.1 DB_HIDL client:[%s]. previous mask value %03o",name,pre_mask);
    strncpy(this->mServerName, name,strlen(name));
    mHidlServer = nullptr;
    mHidlServerDeathRecipient = nullptr;
    mFileListCB = nullptr;
    mFileDataCB = nullptr;
    mEnableFileKeyWord = false;
    mLogFD = -1;
    mNeedCopyDefaultFilter = true;
    mNeedCopyCustomFilter = true;
    mNeedCopyMDDB = true;
}

CopyModemDBFilterClient::~CopyModemDBFilterClient() {
    ALOGI("DB_HIDL client:[%s] destroy.",mServerName);
    if (mHidlServer != nullptr && mHidlServerDeathRecipient != nullptr) {
        mHidlServer->unlinkToDeath(mHidlServerDeathRecipient);
    }
    mHidlServer = nullptr;
    mHidlServerDeathRecipient = nullptr;
    mFileListCB = nullptr;
    mFileDataCB = nullptr;
}

bool CopyModemDBFilterClient::copyDBFilterFile() {
    bool errval = true;
    if (!isNeedDoCopy()) {
        ALOGI("DB_HIDL client:[%s] No need copy modem and filter",mServerName);
        return true;
    }
    if(!int_hidl()){
        ALOGE("DB_HIDL client:[%s]copy modem and filter for int_hidl fail",mServerName);
        return false;
    }
    waitVoldDecryptData();
    if (!createDataFolder()) {
        ALOGE("DB_HIDL client:[%s] create filter and db folder in data fail", mServerName);
        return false;
    }
    createClientStartFile();
    if(!copyFilter()) {
        ALOGE("DB_HIDL client:[%s] copyFilter modem filter fail",mServerName);
        errval = false;
    }
    if(!copyCustomFilter()) {
        ALOGE("DB_HIDL client:[%s] Customer modem  filter fail",mServerName);
        errval = false;
    }
    if(!copyModemDB()) {
        ALOGE("DB_HIDL client:[%s] copyModemDB modem db fail",mServerName);
        errval = false;
    }
    return true;

}

bool CopyModemDBFilterClient::insertFileListData(const hidl_vec<hidl_string>& data){
//    ALOGI("DB_HIDL client:[%s] insertFileListData()",mServerName);
    int mSize = data.size();
    for (int i=0;i < mSize;i++) {
        string s = data[i].c_str();
        if (mEnableFileKeyWord) {
            if (strstr(s.c_str(), mFileKeyWord) != NULL) {
                filesVector.push_back(s);
//                ALOGI("DB_HIDL client:insertFileListData %s ", s.c_str());
                char log[512];
                sprintf(log, "insert vendor file:[%s]",s.c_str());
                writeLogToFile(log);
            } else {
                ALOGW("DB_HIDL client:insertFileListData ingnore %s ", s.c_str());
                char log[512];
                sprintf(log, "ingnore vendor file:[%s]",s.c_str());
                writeLogToFile(log);
            }
        } else {
            filesVector.push_back(s);
 //           ALOGI("DB_HIDL client:insertFileListData %s ", s.c_str());
            char log[512];
            sprintf(log, "insert vendor file:[%s]",s.c_str());
            writeLogToFile(log);
        }
    }
    if (mSize > 0) {
        ALOGI("DB_HIDL client:[%s] return mSize = %d.insertFileListData: %lu ",mServerName,mSize,filesVector.size());
    } else {
        ALOGI("DB_HIDL client:[%s] return mSize = %d.",mServerName,mSize);
    }
    return true;
}

bool CopyModemDBFilterClient::saveDataToFile(const hidl_handle& hd){
    ALOGI("DB_HIDL client:[%s] saveDataToFile(%s)",mServerName,mFilePathToSave);
    if (hd == nullptr) {
        ALOGE("DB_HIDL client:[%s] hd is null", mServerName);
        return false;
    }
    int fd = hd->data[0];
    if (fd < 0) {
        ALOGE("DB_HIDL client:[%s] fd is %d ",mServerName, fd);
        return false;
    }
    int mSavefd = openFile(mFilePathToSave);
    if(mSavefd < 0) {
        ALOGE("DB_HIDL server:%s.[%s] open fail:%s", mServerName, mFilePathToSave, strerror(errno));
        return false;
    }

    if (fd >= 0) {
        int len = 4096 * 1024;
        char buffer[len];
        while (1) {
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer, len));
            if (bytes_read == 0) {
                 break;
            } else if (bytes_read < 0) {
                ALOGE("DB_HIDL client:[%s] read fd[%d] < 0. %s",mServerName,fd,strerror(errno));
                break;
            }

            if (!android::base::WriteFully(mSavefd, buffer, bytes_read)) {
                ALOGE("DB_HIDL client:[%s] write[%s] fail. %s",mServerName,mFilePathToSave,strerror(errno));
                break;
            }
        }
    }
    close(mSavefd);
    return true;
}

void CopyModemDBFilterClient::handleHidlDeath() {
    ALOGE("DB_HIDL client:[%s] died. ", mServerName);
}

bool CopyModemDBFilterClient::isModemSingleBinLoad() {
    return cmpProperty(PROP_SINGLE_BIN_FEATURE, "1", "0");
}

bool CopyModemDBFilterClient::cmpProperty(const char * propName,
        const char * expectValue, const char * defaultValue) {
    char prop[128];
    unsigned int propLen = 0;
    property_get(propName, prop, defaultValue);
    if ((propLen = strlen(prop)) == 0) {
        return false;
    }
    if (propLen != strlen(expectValue)) {
        return false;
    }
    return strncmp(prop, expectValue, propLen) == 0;
}

bool CopyModemDBFilterClient::isNeedDoCopy() {
    if(isModemSingleBinLoad()) {
        ALOGW("DB_HIDL client:[%s] is modem single bin load.Won't do copy",mServerName);
        return false;
    } else {
        ALOGI("DB_HIDL client:[%s] is NOT modem single bin load.",mServerName);
    }
    if (isExist(DEFAULT_FILTER_LOG_FILE_OK, F_OK)) {
        mNeedCopyDefaultFilter = false;
        ALOGI("DB_HIDL client:[%s] default filter alreay copied.",mServerName);
    }
    if (isExist(CUSTOM_FILTER_LOG_FILE_OK, F_OK)) {
        mNeedCopyCustomFilter = false;
        ALOGI("DB_HIDL client:[%s] Custom filter alreay copied.",mServerName);
    }
    if (isExist(MDDB_LOG_FILE_OK, F_OK)) {
        ALOGI("DB_HIDL client:[%s] modem db alreay copied.",mServerName);
        mNeedCopyMDDB = false;
    }
    return false;//mNeedCopyDefaultFilter || mNeedCopyCustomFilter || mNeedCopyMDDB;
}

void CopyModemDBFilterClient::clearVectorAndKeyword() {
    mEnableFileKeyWord = false;
    mFileKeyWord[0] = '\0';
    filesVector.clear();
}

void CopyModemDBFilterClient::enableKeyword(const char * key) {
    mEnableFileKeyWord = true;
    strncpy(mFileKeyWord, key, sizeof(mFileKeyWord) - 1);
    mFileKeyWord[sizeof(mFileKeyWord) - 1] = '\0';
}

bool CopyModemDBFilterClient::copyFilter() {
    if (!mNeedCopyDefaultFilter) {
        return true;
    }
    mLogFD = openFile(DEFAULT_FILTER_LOG_FILE);
    hidl_string path = DEFAULT_FILTER_FOLDER;
    clearVectorAndKeyword();
    enableKeyword(FILTER_KEY_WORD);
    if (!mHidlServer->getFileListAndSize(path,mFileListCB)) {
        path.clear();
        ALOGE("DB_HIDL client:[%s] copyFilter fail",mServerName);
        closeLogFD();
        return false;
    }
    path.clear();
    bool errval = copyVectorFiles(DEFAULT_FILTER_FOLDER,DEFAULT_FILTER_DATA_PATH);
    closeLogFD();
    if (errval) {
        setFileNameFlag(DEFAULT_FILTER_LOG_FILE,DEFAULT_FILTER_LOG_FILE_OK);
    }
    return true;
}

bool CopyModemDBFilterClient::copyCustomFilter() {
    if (!mNeedCopyCustomFilter) {
        return true;
    }
    mLogFD = openFile(CUSTOM_FILTER_LOG_FILE);
    hidl_string path = CUSTOM_FILTER_FOLDER;
    clearVectorAndKeyword();
    enableKeyword(FILTER_KEY_WORD);
    if (!mHidlServer->getFileListAndSize(path,mFileListCB)) {
        path.clear();
        ALOGE("DB_HIDL client:[%s] copyCustomFilter fail",mServerName);
        closeLogFD();
        return false;
    }
    path.clear();
    bool errval = copyVectorFiles(CUSTOM_FILTER_FOLDER,CUSTOM_FILTER_DATA_PATH);
    closeLogFD();
    if (errval) {
        setFileNameFlag(CUSTOM_FILTER_LOG_FILE,CUSTOM_FILTER_LOG_FILE_OK);
    }
    return true;
}

bool CopyModemDBFilterClient::copyModemDB() {
    if (!mNeedCopyMDDB) {
        return true;
    }
    mLogFD = openFile(MDDB_LOG_FILE);
    hidl_string path = MODEM_DB_FOLDER;
    clearVectorAndKeyword();
    if (!mHidlServer->getFileListAndSize(path,mFileListCB)) {
        path.clear();
        closeLogFD();
        ALOGE("DB_HIDL client:[%s] copyModemDB fail",mServerName);
        return false;
    }
    path.clear();
    bool errval = copyVectorFiles(MODEM_DB_FOLDER,MODEM_DB_DATA_PATH);
    closeLogFD();
    if (errval) {
        setFileNameFlag(MDDB_LOG_FILE,MDDB_LOG_FILE_OK);
    }
    return true;
}

bool CopyModemDBFilterClient::int_hidl() {
    ALOGI("DB_HIDL client:[%s] InitHidl",mServerName);
    if(mHidlServer == nullptr)
    {
        mHidlServer = ICopyDBFilter::getService(mServerName);
        if(mHidlServer != nullptr)
        {
            ALOGD("DB_HIDL client:[%s] start hidl service ok", mServerName);
        } else {
            ALOGE("DB_HIDL client:[%s] start hidl service fail", mServerName);
        }
    }
    if(mHidlServer == nullptr) {
        return false;
    }

    if (mHidlServerDeathRecipient == nullptr) {
        mHidlServerDeathRecipient = new DBFilterHidlServerDeathRecipient(this);
        mHidlServer->linkToDeath(mHidlServerDeathRecipient, 0);
    } else {
        mHidlServer->linkToDeath(mHidlServerDeathRecipient, 0);
    }

    if (mFileListCB == nullptr) {
        mFileListCB = new FileListCallback(this);
    }

    if (mFileDataCB == nullptr) {
        mFileDataCB = new FileDataCallback(this);
    }
    return true;

}

void CopyModemDBFilterClient::waitVoldDecryptData() {
    int time_out = 0;// wait for 20s
    char crypt[256] = {0};
    property_get("ro.crypto.state",crypt,"0");
    ALOGI("DB_HIDL client:[%s] ro.crypto.state: %s", mServerName, crypt);
    if (!strncmp(crypt,"encrypted",strlen("encrypted"))) {// encrypted
        memset(crypt,'\0',sizeof(crypt));
        property_get("ro.crypto.type",crypt,"0");
        ALOGI("DB_HIDL client:[%s] ro.crypto.type[%s]", mServerName, crypt);
        if (!strncmp(crypt,"block",strlen("block"))) {//block
            memset(crypt,'\0',sizeof(crypt));
            property_get("vold.decrypt",crypt,"0");
            ALOGI("DB_HIDL client:[%s] vold.decrypt[%s]", mServerName, crypt);
            do {
                if (!strncmp("trigger_restart_framework",crypt,strlen("trigger_restart_frawework"))) {
                    break;
                } else {
                    sleep(1);
                    property_get("vold.decrypt",crypt,"0");
                    continue;
                }
             } while (time_out++ < 20);
            ALOGI("DB_HIDL client:[%s] waiting vold ready for %d seconds", mServerName, time_out);
        }
    }
}

bool CopyModemDBFilterClient::copyVectorFiles(const char * srcRoot, const char * desRoot) {
    StringVectorIter mPathIterator;
    int errval = false;
    for (mPathIterator = filesVector.begin();
             mPathIterator != filesVector.end(); mPathIterator++) {
        string s = *mPathIterator;
        const char* _Source = s.c_str();
        sprintf(mFilePathToSave, FILE_PATH_FORMAT, desRoot, &_Source[strlen(srcRoot)]);
        hidl_string path = s;
        if (!mHidlServer->getFileData(path,mFileDataCB)) {
            path.clear();
            char log[512];
            sprintf(log, "Copy fail:[%s] to [%s]",s.c_str(), mFilePathToSave);
            writeLogToFile(log);
            ALOGE("DB_HIDL client:[%s] getFileData fail:[%s] to [%s]",mServerName,s.c_str(), mFilePathToSave);
            return false;
        }
        path.clear();
        errval = true;
        char log[512];
        sprintf(log, "Copy OK:[%s] to [%s]",s.c_str(), mFilePathToSave);
        writeLogToFile(log);
    }
    return errval;
}

void CopyModemDBFilterClient::createClientStartFile(){
    if (!isExist(HIDL_SERVICE_RUN, F_OK)) {
        int fd = openFile(HIDL_SERVICE_RUN);
        if (fd > 0) {
            close(fd);
            ALOGI("DB_HIDL client:[%s] start file created: %s", mServerName, HIDL_SERVICE_RUN);
        } else {
            ALOGE("DB_HIDL client:[%s] created:%s fail. %s", mServerName, HIDL_SERVICE_RUN, strerror(errno));
        }
    } else {
        ALOGI("DB_HIDL client:[%s] start file[ %s]already exist", mServerName, HIDL_SERVICE_RUN);
    }
}

bool CopyModemDBFilterClient::createDataFolder(){
    int mSleepInterval = 200 * 1000;
    int mOnSecond = 5;
    int mCountTotal = 20 * mOnSecond;
    bool mFindRoot = false;
    while(mCountTotal-- > 0) {
        if (isExist(SYSTEM_DATA_DE_PATH, F_OK | W_OK)) {
            mFindRoot = true;
            ALOGI("DB_HIDL client:[%s] data root path ok = %s ",mServerName, SYSTEM_DATA_DE_PATH);
            break;
        }
        usleep(mSleepInterval);
    }
    if (!mFindRoot) {
        ALOGE("DB_HIDL client:[%s] wait for data root path[%s] ready time out.",mServerName, SYSTEM_DATA_DE_PATH);
    }
    bool ret = makeDirs(CUSTOM_FILTER_DATA_PATH,S_IRWXU | S_IRWXG | S_IROTH)
            && makeDirs(MODEM_DB_DATA_PATH,S_IRWXU | S_IRWXG | S_IROTH);

    if(ret) {
        ALOGI("DB_HIDL client:[%s] makeDirs ok %s ",mServerName, CUSTOM_FILTER_DATA_PATH);
        ALOGI("DB_HIDL client:[%s] makeDirs ok %s ",mServerName, MODEM_DB_DATA_PATH);
/*
        if (selinux_android_restorecon(SELF_DB_FILTER_FOLDER_PATH, 0) == -1) {
            ALOGE("DB_HIDL client:[%s] restorecon[%s] fail. %s",mServerName,SELF_DB_FILTER_FOLDER_PATH,strerror(errno));
        }
        if (selinux_android_restorecon(MODEM_DB_DATA_PATH, 0) == -1) {
            ALOGE("DB_HIDL client:[%s] restorecon[%s] fail. %s",mServerName,MODEM_DB_DATA_PATH,strerror(errno));
        }
        if (selinux_android_restorecon(DEFAULT_FILTER_DATA_PATH, 0) == -1) {
            ALOGE("DB_HIDL client:[%s] restorecon[%s] fail. %s",mServerName,DEFAULT_FILTER_DATA_PATH,strerror(errno));
        }
        if (selinux_android_restorecon(CUSTOM_FILTER_DATA_PATH, 0) == -1) {
            ALOGE("DB_HIDL client:[%s] restorecon[%s] fail. %s",mServerName,CUSTOM_FILTER_DATA_PATH,strerror(errno));
        }
*/
    }
    return ret;
}

int CopyModemDBFilterClient::openFile(const char * pathname){
    int fd = (TEMP_FAILURE_RETRY(open(pathname, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,
            S_IRWXU|S_IXGRP|S_IRGRP|S_IROTH|S_IXOTH)));
    return fd;
}

int CopyModemDBFilterClient::closeLogFD(){
    if(mLogFD < 0){
        return 0;
    }
    int val = close(mLogFD);
    mLogFD = -1;
    return val;
}

bool CopyModemDBFilterClient::writeLogToFile(const char *log) {
    if(mLogFD < 0){
        ALOGE("DB_HIDL client:[%s] mLogFD < 0",mServerName);
        return false;
    }
    char outlog[512];
    sprintf(outlog, "%s%s", log, "\n");
    return android::base::WriteFully(mLogFD, outlog, strlen(outlog));
}

bool CopyModemDBFilterClient::makeDirs(const char *path, mode_t mode) {
    if (path == NULL) {
        return false;
    }
    size_t len = strlen(path) + 1;
    char currentLevelDir[len];
    char createdPath[len];
    char *pCreatedPath;
    char *pCurrentLevelDir;
    memset(createdPath,'\0' , len * sizeof(char));
    memset(currentLevelDir,'\0' , len * sizeof(char));
    pCreatedPath = createdPath;
    pCurrentLevelDir = currentLevelDir;
    for (unsigned long i = 0; i < len; i++) {
        *pCreatedPath = *path;
        *pCurrentLevelDir = *path;
        pCreatedPath++;
        path++;
        if ('/' == *pCurrentLevelDir || len - 1 == i) {
            *pCurrentLevelDir = '\0';
            if (strlen(currentLevelDir) != 0) {
                if (!makeDir(createdPath, mode)) {
                    ALOGE("DB_HIDL client:[%s] makeDir [%s], errno=%s",mServerName, createdPath, strerror(errno));
                    return false;
                }
            }
            pCurrentLevelDir = currentLevelDir;
            continue;
        }
        pCurrentLevelDir++;
    }
    return true;

}

bool CopyModemDBFilterClient::isExist(const char *path, mode_t mode) {
    return TEMP_FAILURE_RETRY(access(path, mode)) == 0;
}

bool CopyModemDBFilterClient::makeDir(const char *path, mode_t mode) {
    if (isExist(path, F_OK)) {
//        ALOGI("DB_HIDL client:[%s] makeDir file exists = %s ",mServerName, path);
        return true;
    }

    if (TEMP_FAILURE_RETRY(mkdir(path, mode)) == 0) {
//        ALOGI("DB_HIDL client:[%s] mkdir ok %s ",mServerName, path);
        return true;
    }

    if (errno == EEXIST) {
//        ALOGI("DB_HIDL client:[%s] mkdir file exists = %s ",mServerName, path);
        return true;
    }
    ALOGE("DB_HIDL client:[%s] makeDir [%s], errno=%s",mServerName, path, strerror(errno));
    return false;
}

bool CopyModemDBFilterClient::setFileNameFlag(const char * path, const char *newPath){
    if (TEMP_FAILURE_RETRY(rename(path, newPath)) == 0) {
        return true;
    }
    ALOGE("DB_HIDL client:[%s] rename %s fail.%s",mServerName, path, strerror(errno));
    return false;
}

// Methods end.

}  // namespace implementation
}  // namespace V1_0
}  // namespace modemdbfilter
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
