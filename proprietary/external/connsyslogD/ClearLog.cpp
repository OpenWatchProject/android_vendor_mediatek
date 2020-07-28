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
 *   ClearLog.cpp
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    Support Log recycle in native layer
 *
 *
 *------------------------------------------------------------------------------
 *
 *
 *******************************************************************************/

#include<algorithm>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <dirent.h>
#include "LogWriter.h"
#include "ConsysLog.h"


#define FILE_TREE "/file_tree.txt"
#define SUB_FILE_TREE "/file_tree.txt"
namespace consyslogger {
long LogWriter::fileSize(const char *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) == -1)
    {
         LOGE("get the file %s status, errno=%d",path, errno);
        return 0;
    }
    else
    {
    //    LOGD("get the file %s status, size=%lu",path, file_stat.st_size);
        return (long)(file_stat.st_size / (1024 * 102));
    }
}

unsigned long LogWriter::calculateFolderSize(const char *path) {

    DIR *dp;
    struct dirent *dirp;
    char filepath[512] = {0};
    dp = opendir(path);
    unsigned long mSize = 0;
    if (dp == NULL) {
        return fileSize(path);
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        memset(filepath, '\0', 512);
        if (strlen(path) + strlen(dirp->d_name) > 254) {
            LOGE("calculateFolderSize length above 254");
            LOGE("opendir path overflow! Path: %s, dirname: %s", path, dirp->d_name);
            continue;
        }

        sprintf(filepath,"%s/%s",path,dirp->d_name);
        mSize += calculateFolderSize(filepath);
        if (dirp->d_type == DT_DIR) {
            // push all folders in fw/ to mNormalFolderArray.
            // include the folder which is not created by logger,
            // such as create test folder,
            // copy other folder.etc.
            mNormalFolderArray.push_back(filepath);
        }
    }
    closedir(dp);
    dirp = NULL;
    return mSize;
}

long LogWriter::checkFolderSize(const char *path) {
    mNormalFolderArray.clear();
    unsigned long mTotal_Size = 0;
    mTotal_Size = calculateFolderSize(path) / 10;
    return mTotal_Size;
}
/*
    filter the folder that is not recorded in folder file tree.
    ex: test folder, other folder that is not recorded in file tree
*/
int LogWriter::checkNeedRemoveFolder(){
    strListIter mTreePathIterator;
    strListIter mNoramlPathIterator;
    string ws;
    const char* _Source;
    char temp[128]={0};
    // remove folders that recorded in folder file tree
    for (mTreePathIterator = mTreeFolderArray.begin();
            mTreePathIterator != mTreeFolderArray.end(); mTreePathIterator++) {
        mNormalFolderArray.remove(*mTreePathIterator);
    }
    mTreePathIterator--;
    mNormalFolderArray.remove(*mTreePathIterator); // make sure remove current running
    // folder from delete array
    unsigned int mFolderInTXT = mTreeFolderArray.size(); // all log folders
    mRunningArray.push_back(*mTreePathIterator); // keep current running folder
    ws = *mTreePathIterator;
    _Source = ws.c_str();
    strncpy(temp, _Source, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    mTreeFolderArray.remove(*mTreePathIterator);
    LOGI("mTreeFolderArray size %u.Running folder %s",mFolderInTXT, temp);
    if(mTreeFolderArray.size() >=  mFolderInTXT) {
        return 0;
    }
    return 1;
}
/*
 parser the file_tree.txt in fw/. that recorded all created log folder
*/
int LogWriter::readFolderTxtFile(char *path) {
    mTreeFolderArray.clear();
    FILE *fd = NULL;
    int retValue = 0;
    if ((fd = fopen(path, "rb")) == NULL) {
        LOGE("open file: %s .error %d",path, errno);
        return retValue;
    }
    fseek(fd, 0, SEEK_END);
    int buffer_len = ftell(fd);
    if (buffer_len == -1) {
     LOGE("ftell file error: %s",strerror(errno));
     fclose(fd);
     return retValue;
    }
    fseek(fd, 0, SEEK_SET);

    char buffer[buffer_len];
    char strPath[256] = {0};
    int lenRead = 0;

   memset(buffer, '\0', buffer_len );
   lenRead = fread(buffer, sizeof(char), (buffer_len), fd);
    buffer[buffer_len-1] = '\0';
    char *pToken = strtok(buffer, "\r");
        while (pToken != NULL) {
            memset(strPath, '\0', 256);
            strncpy(strPath, pToken, sizeof(strPath) - 1);
            strPath[sizeof(strPath) - 1] = '\0';
            int len = strlen(strPath);
            if (len > 0) {
                mTreeFolderArray.push_back(strPath);
            }
            pToken = strtok(NULL, "\r");
        }

    fclose(fd);
    retValue = mTreeFolderArray.size();
    LOGI("Parser total folders %d from folder file_tree.txt",retValue);

    return retValue;
}

bool LogWriter::checkRemoveFileOneByOne(bool mRunningFolder) {
    int nCount = 1;
    int nTotalFile = 4;
   
    if (mCurrentFileArray.size() > 4) {
        nCount++;
        nTotalFile += mCurrentFileArray.size();
    }
    nTotalFile = nTotalFile / nCount;
    if (removeLogByProtectFileNum(nTotalFile)) {
        return true;
    }
    if (mRunningFolder) {
        if (nTotalFile > 4) {
            if (removeLogByProtectFileNum(4)) {
                return true;
            }
        }
    }

    return false;
}

bool LogWriter::deleteFileAndcheckSize(string str) {
    mClearLogSize += removeFileAndGetSize(str.c_str());
    if (checkDeleteSizeIfEnough()) {
        return true;
    }
    return false;
}

bool LogWriter::removeLogByProtectFileNum(int nFileRemained) {
    bool removeFind = false;
    if (nFileRemained < 0) {
        nFileRemained = 0;
    }

    strListIter iter;
    while (true) {
        removeFind = false;
   
        int mArraySize = mCurrentFileArray.size();
        if(mArraySize > nFileRemained)  {
            iter = mCurrentFileArray.begin();
           removeFind = true;
            if (deleteFileAndcheckSize(*iter)) {
                return true;
            }
            mCurrentFileArray.remove(*iter);
        }
        if (!removeFind) {
            break;
        }
    }
    return false;
}

int LogWriter::removeFileAndGetSize(const char *path) {
    char filePath[256];
    memset(filePath, '\0', 256);
    strncpy(filePath, path, strlen(path) - strlen(".curf"));
    filePath[sizeof(filePath) - 1] = '\0';
    if (access(filePath, F_OK) == 0) {
        LOGD("removeFileAndGetSize Remove File:%s",filePath);
    } else if (access(path, F_OK) == 0) {
        strncpy(filePath, path, sizeof(filePath) - 1);
        filePath[sizeof(filePath) - 1] = '\0';
        LOGD("removeFileAndGetSize Remove File:%s",filePath);
    } else {
    //        LOGE("File is not exist:%s",filePath);
        return 0;
    }
    int mSize = fileSize(filePath);
    if (remove(filePath) == -1) {
       LOGE("removeFileAndGetSize path :%s failed: %s",filePath,strerror(errno));
    }
    usleep(500000);
    return mSize;
}

bool LogWriter::checkDeleteSizeIfEnough() {
    int mSizeDeleted = mClearLogSize / 10;
    if (mSizeDeleted > 10) {
        if (mSizeDeleted >= mLogNeedToBeClearSize) {
            LOGV("Clean log size is %d M", mSizeDeleted);
            return true;
        }
    } else {
        if (mSizeDeleted != 0) {
            LOGV("Clean log size is %d M", mSizeDeleted);
        }
    }
    return false;
}

bool LogWriter::isFolderEmpty(const char * path){
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(path);
    if (dp == NULL) {
        return false;
    }
    long mCountFolderNum = 0;
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        mCountFolderNum++;
    }
    closedir(dp);
    dirp = NULL;
    return mCountFolderNum <= 0;
}

int LogWriter::clearLogs(const char *path , bool mNeedcheckSize){
    if((access(path, F_OK) != 0)) {
        return 0;
    }
    DIR *dp;
    struct dirent *dirp;
    char filepath[512] = {0};
    dp = opendir(path);
    long mSize = 0;
    if (dp == NULL) {
        int length = fileSize(path);
        mSize += length;
        if (remove(path) == -1) {
           LOGE("remove path :%s failed: %s",path,strerror(errno));
        }
        mClearLogSize += length;
        LOGE("ClearLogs open dir failed: dir:%s.",path);
        usleep(500000);
        return mSize;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        if (m_bPaused) {
            LOGE("Stop do clear log for logging paused.");
            break;
        }
        if (strlen(path) + strlen(dirp->d_name) > 254) {
            LOGE("clearLogs length above 254");
            continue;
        }
        //clear all files include file_tree.txt
        /* if (strstr(dirp->d_name,"_FW_") == NULL) {
            continue;
        }*/
        memset(filepath, '\0', 512);
        sprintf(filepath,"%s/%s",path,dirp->d_name);
        if (dirp->d_type == DT_DIR) {
            mSize += clearLogs(filepath);
        } else {
            int length = fileSize(filepath);
            mSize += length;
            if (remove(filepath) == -1) {
               LOGE("remove File :%s failed: %s",filepath,strerror(errno));
            }

            mClearLogSize += length;
             LOGD("clearLogs Remove File:%s.",filepath);
            if(mNeedcheckSize && checkDeleteSizeIfEnough()){
                closedir(dp);
                dirp = NULL;
                if(isFolderEmpty(path)) {
                   rmdir(path);
                }
                return mSize;
            }
            usleep(500000);
        }

    }
    closedir(dp);
    rmdir(path);
    dirp = NULL;
    return mSize;
}

bool LogWriter::removeAllFileInFolder(const char *path, bool mNeedClearLeftLog){
    clearLogs(path, mNeedClearLeftLog);
    if(checkDeleteSizeIfEnough()) {
        return true;
    }
    return false;
}

void LogWriter::clearArray(){
  
    mCurrentFileArray.clear();
}

void LogWriter::clearLogFolder(const char *path, int mRecycleSize) {
    unsigned long mFolderSize = checkFolderSize(path);
    mLogNeedToBeClearSize = mFolderSize - mRecycleSize;
    if (mLogNeedToBeClearSize <=0 ) {
        LOGV("clearLogFolder %s size is %ld M. Recycle size is %ld M,no need clear log", path,mFolderSize, mRecycleSize);
        return;
    }
    LOGV("clearLogFolder %s size is %lu M. Recycle size is %d M, need clear size is %d M.", path,mFolderSize, mRecycleSize, mLogNeedToBeClearSize);
    mClearLogSize = 0;
    char mTopTree[64] = {0};
    strncpy(mTopTree, path, sizeof(mTopTree) - 1);
    mTopTree[sizeof(mTopTree) - 1] = '\0';
    strncat(mTopTree, FILE_TREE,strlen(FILE_TREE));
    mRunningArray.clear();
    sleep(1); // wait for file tree txt updated.
    if (readFolderTxtFile(mTopTree) <= 0) {
         LOGE("Get nothing from Top tree txt. return then need check next time");
        return;
    }

    mNormalFolderArray.sort();
 
    if(!checkNeedRemoveFolder()){
        LOGE("Remove file File tree txt file error");
        return;
    }
    strListIter mPathIterator;

    LOGI("Begin to clear not recorded folder in mNormalFolderArray, size: %u", (unsigned int)mNormalFolderArray.size());
    for (mPathIterator = mNormalFolderArray.begin();
            mPathIterator != mNormalFolderArray.end(); mPathIterator++) {
        string s = *mPathIterator;
        const char* _Source = s.c_str();
        if (removeAllFileInFolder(_Source)) {
            return;
        }
    }

    LOGI("Begin to clear folder in TreeFolderArray ,size: %u", (unsigned int)mTreeFolderArray.size());
    for (mPathIterator = mTreeFolderArray.begin();
            mPathIterator != mTreeFolderArray.end(); mPathIterator++) {
        string s = *mPathIterator;
        const char* _Source = s.c_str();
        if (removeAllFileInFolder(_Source)) {
            return;
        }
    }

    LOGI("Begin to clear file in RunningArray, size: %u",(unsigned int) mRunningArray.size());
    for (mPathIterator = mRunningArray.begin();
            mPathIterator != mRunningArray.end(); mPathIterator++) {
        string ws = *mPathIterator;
        const char* _Source = ws.c_str();
        if (readCurrentLogFileTree(_Source)) {
            if (checkRemoveFileOneByOne(true)) {
                return;
            }
        }
    }
}

int LogWriter::readCurrentLogFileTree(const char *path) {
    
    char mTreeFile[256]= {0};
    char strPath[256]= {0};
    strncpy(mTreeFile, path, sizeof(mTreeFile) - 1);
    mTreeFile[sizeof(mTreeFile) - 1] = '\0';
    strncat(mTreeFile, SUB_FILE_TREE,strlen(SUB_FILE_TREE));
    FILE *fd = NULL;
    int retValue = 0;
    if ((fd = fopen(mTreeFile, "rb")) == NULL) {
        LOGE("open file: %s.error %d",mTreeFile, errno);
        return retValue;
    }
 
    clearArray();    
    fseek(fd, 0, SEEK_END);
    int buffer_len = ftell(fd);
    if (buffer_len == -1) {
     LOGE("ftell file error: %s",strerror(errno));
     fclose(fd);
     return retValue;
    }
    fseek(fd, 0, SEEK_SET);

    char buffer[buffer_len];
    int lenRead = 0;
    memset(buffer, '\0', buffer_len );
    lenRead = fread(buffer, sizeof(char), (buffer_len), fd);
    buffer[buffer_len-1] = '\0';
    char *pToken = strtok(buffer, "\r");
    while (pToken != NULL) {
        memset(strPath, '\0', 256);
        strncpy(strPath, pToken, sizeof(strPath) - 1);
        strPath[sizeof(strPath) - 1] = '\0';
        int len = strlen(strPath);
        if (len > 0) {
            if (strstr(strPath, "_FW_") != NULL) {
                //if (strstr(strPath, "curf") == NULL) {
                    mCurrentFileArray.push_back(strPath);
               // }
            }
        }
        pToken = strtok(NULL, "\r");
    }
    fclose(fd);
    retValue = mCurrentFileArray.size();
     LOGI("current files array size:%d from sub_file_tree.txt.",retValue);  
    return retValue;
}
static pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

void LogWriter::recyleLogPath(const char *path, int folderSize) {
    pthread_mutex_lock(&mlock);
    clearLogFolder(path, folderSize);
    clearArray();
    pthread_mutex_unlock(&mlock);
    return ;
}

}
