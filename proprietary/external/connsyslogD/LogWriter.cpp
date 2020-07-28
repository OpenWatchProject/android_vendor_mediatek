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
 *   logwriter.cpp
 *
 * Description:
 * ------------
 *    Log writer
 *
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <dirent.h>

#include <sys/mman.h>
#include <fcntl.h>

#include "LogWriter.h"
#include "ConsysLogger.h"
#include "Utils.h"
#include "ConsysLog.h"
#include "Interface.h"


extern BOOT_MODE BootMode;


#define RESET_FILE_POINTER_ARRAY(file_list, count) \
    do { \
        if (file_list == NULL) { \
            file_list = new BaseFile*[count];\
        } \
        for(int i = 0; i < count; i++) { \
            file_list[i] = NULL; \
        } \
    } while(0)

#define CLEAR_FILE_POINTER_ARRAY(file_list, count) \
    do {\
        if (file_list != NULL) {\
            for (int i = 0; i< count; i++) {\
                if (file_list[i] != NULL) {\
                    delete file_list[i];\
                    file_list[i] = NULL;\
                }\
            }\
            delete [] file_list;\
            file_list = NULL;\
        }\
     }while(0)

char SD_FILE_SYSTEM[MAX_PATH_LEN] = {0};
char SD_LOG_ROOT[MAX_PATH_LEN] = {0};
//char storagePath[MAX_PATH_LEN] = {0};
mode_t create_mode = 0777; //S_IRWXU | S_IRWXG | S_IRWXO;
bool pathFromMTKLogger = false;
bool setStoragePath(char* path){
   if ((0 == strncmp(path, "/data", strlen("/data")))) {
       create_mode = 0770; //S_IRWXU | S_IRWXG;
   } else {
       create_mode = 0777;
   }
   LOGD("create log folder mode,0%03o",create_mode);
   struct statfs fileStat;
   memset(&fileStat,0,sizeof(fileStat));
    /*check whether the storage path is exist or not.
      if not exist, create it firstly.
    */
    if (statfs(path, &fileStat) == -1) {
        if (makeDirWithPermission(path, create_mode) != false) {
            LOGD("create log folder OK,%s\n",path);
        } else {
            LOGE("fail to create log folder %s: %s\n",path, strerror(errno));
        }
    }
    /* 'sdcard/log/' should be changed to 'sdcard/log' for other reference.
       otherwise ,it will show like this sdcard/log//mtklog.
     */
    if ('/' == *(path + strlen(path)-1)) {
       LOGD("find seperator in last char of the path :%s",path);
       *(path + strlen(path)-1) = '\0';
       LOGD("revise log folder,%s",path);
    }
    pathFromMTKLogger = true;
    property_set(PROP_LOG_SAVE_PATH, path);
    return pathFromMTKLogger;
}


namespace consyslogger {

bool LogWriter::notifyMTKLogger(int msgid) {
    bool ret = true;
    switch (msgid) {
    //fail write file
    case MSG_FAIL_WRITE_FILE: {
        LOGV("Inform UI failed to write file");
        ret = m_listener(MSG_FAIL_WRITE_FILE, "Fail write file.");
        break;
    }
    //SD not ready
    case MSG_SDCARD_NO_EXIST: {
        LOGV("Inform UI the SD card does not exist");
        if (isBootCompleted()) {
            ret = m_listener(MSG_SDCARD_NO_EXIST,
                    "SD card no exist.");
        } else {
            LOGD("System not ready then can not send SD card does not exist intent.");
        }
        break;
    }
    //SD full
    case MSG_SDCARD_IS_FULL: {
        LOGV("Inform UI SD card is full");
        mSDcardFull = true;
        ret = m_listener(MSG_SDCARD_IS_FULL, "SD card is full.");
        break;
    }
    //SD file is not exist, file is removed
    case MSG_SDCARD_NO_LOG_FILE: {
        if (false == m_bInformNoLogFile) {
            LOGV("Inform UI the log file does not exist");
            ret = m_listener(MSG_SDCARD_NO_LOG_FILE,
                    "File in SD card is disappeared.");
            m_bInformNoLogFile = true;
        } else {
            ret = true;
        }
        break;
    }
   
    default: {
        ret = false;
        break;
    }
    }
    return ret;
}

void* thrClearBootupLog(void* arg) {
    LogWriter *pLogWriter = static_cast<LogWriter *>(arg);
    LOGV("recycle data partition Log");
    DIR* dp = NULL;
    char srcname[256];
    dp = opendir(CONSYSLOGGER_BOOTUP_LOG_FOLDER);
    if (dp == NULL) {
        LOGE("recycle data partition Log: %s is not exist!",CONSYSLOGGER_BOOTUP_LOG_FOLDER);
        pLogWriter->threadClearDataLog = 0;
        return NULL;
    }
    struct timespec ts;
    int ret = -1;
    while (pLogWriter->m_bLog2SD == LOG_ON_DATA) {
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            LOGE("boot recycle Get clock time error");
        }
        ts.tv_sec += 10;
        pthread_mutex_lock(&pLogWriter->mutexRecycle);
        ret = pthread_cond_timedwait(&pLogWriter->condRecycle,&pLogWriter->mutexRecycle, &ts);
        //ret = pthread_cond_wait(&pLogWriter->condRecycle,&pLogWriter->mutexRecycle);
        if (ret == ETIMEDOUT) {
            LOGD("data recycle:time out");
        } else if (ret != 0){
            LOGD("data recycle return,status=%s,ret=%d",strerror(errno),ret);
        }
        pthread_mutex_unlock(&pLogWriter->mutexRecycle);
        if (pLogWriter->m_bLog2SD == LOG_ON_SD || pLogWriter->m_bPaused == true) {
            break;
        }
        if (pLogWriter->bootuploglist.size() < 10) {
            LOGE("Bootup Log: %d log files! ",pLogWriter->bootuploglist.size());
            continue;
        } else {
            LOGE("Bootup Log before deleted: %d log files!",pLogWriter->bootuploglist.size());
             memset(srcname, '\0', 256);
             strncpy(srcname,pLogWriter->bootuploglist.front().c_str(), sizeof(srcname) - 1);
             srcname[sizeof(srcname) - 1] = '\0';
             LOGE("remove Log: %s in data!",srcname);
             if (remove(srcname) == -1) {
                LOGE("remove file in data :%s failed: %s",srcname,strerror(errno));
             }
            pLogWriter->bootuploglist.pop_front();
            LOGE("data partition Log after deleted: %d log files!",pLogWriter->bootuploglist.size());
        }
    }
    closedir(dp);

    pLogWriter->threadClearDataLog = 0;
    LOGD("recycle data partitoin Log thread exit");
    return NULL;
}


void* thrLoggingRecycle(void *arg) {
    LogWriter *pLogWriter = static_cast<LogWriter *>(arg);
    if (pLogWriter->m_bLogRecycle) {
        LOGI("Logging recycle thread is already started.");
        pLogWriter->threadStartLogRecycle = 0;
        return NULL;
    }
    pLogWriter->m_bLogRecycle = true;
    if (isEngineerBuild()) {
        LOGI("start Logging Recycle Thread");
    }
    while (!pLogWriter->checkReceivedStartCommands()) {
        
        if (pLogWriter->m_bLogRecycle == false) {
            break;
        }
        sleep(2);
    }
    
    LOGI("start Logging Recycle.................");
    struct timespec ts;
    int ret = -1;
    while (pLogWriter->m_bLogRecycle) {
        if (pLogWriter->m_bLogRecycle == false) {
            break;
        }
        int recycleSize = getLogRecycleSize();
        pLogWriter->recyleLogPath(SD_LOG_ROOT, recycleSize);
        LOGD("clearLog :%s,foldersize:%d",SD_LOG_ROOT,recycleSize);
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            LOGE("Get clock time error");
        }
        ts.tv_sec += 30;
        if (pLogWriter->m_bLogRecycle == false) {
            break;
        }
        pthread_mutex_lock(&pLogWriter->mutexRecycle);
        ret = pthread_cond_timedwait(&pLogWriter->condRecycle,
                    &pLogWriter->mutexRecycle, &ts);
        if (ret == ETIMEDOUT) {
            LOGD("recycle:time out");
        } else if (ret != 0){
            LOGD("recycle return,status=%s,ret=%d",strerror(errno),ret);
        }
        pthread_mutex_unlock(&pLogWriter->mutexRecycle);
      }
    pLogWriter->m_bLogRecycle = false;
    pLogWriter->threadStartLogRecycle = 0;

    LOGI("Logging Recycle Thread exit");
    return NULL;
}

/**
 * Create a new thread to work for log recycle.
 * At present, only MODE_SD can start log recycle thread.
 * m_bLogRecycle, is used to control log recycle thread,
 * true if log recycle thread is running.
 * Two lock variables are used to do thread control:
 *  condRecycle, mutexRecycle
 */
void LogWriter::startLogRecycleThread() {
    //check logging mode firstly
    if (threadStartLogRecycle == 0) {
        LOGI("Need create log recycle thread.");
        int err = PTHREAD_CREATE("recycle", &threadStartLogRecycle, NULL,
                thrLoggingRecycle, this);
        if (err != 0) {
            m_bLogRecycle = false;
        }
    }
}


void LogWriter::startClearDataLogThread() {
    if (threadClearDataLog == 0) {
        LOGI(" create log recycle thread for data section logs.");
        PTHREAD_CREATE("data_recycle", &threadClearDataLog, NULL,
                 thrClearBootupLog, this);
    }
}


/**
 * Stop Log recycle thread
 */
void LogWriter::stopLogRecycleThread() {
    m_bLogRecycle = false;
    if (isPthreadAlive(threadStartLogRecycle)) {
        LOGV("LogWriter: log recycle thread is alive");
        m_bLogRecycle = false;
        int retry = 50;
        do {
            onReceiveSignal();
            usleep(100000); //wait 0.1s, total 5s
        } while (--retry > 0 && isPthreadAlive(threadStartLogRecycle));
        LOGD("LogWriter: wait for log recycle thread quit. retry:%d", retry);
    }
    threadStartLogRecycle = 0;
}


bool LogWriter::checkReceivedStartCommands() {
    if(isEngineerBuild()){
        LOGD("Check if receive start cmd");
    }
    
    if (BootMode == FACTORY_BOOT_MODE|| BootMode == ATE_FACTORY_BOOT_MODE || 
        BootMode == META_BOOT_MODE){
        LOGD("not normal mode, return true");
        return true;
    }
    return getReceiveStartCommand();
}

void *thrWaitingSDReady(void *arg) {
    LOGI("Started Waiting SD card thread.");
    LogWriter *pLogWriter = static_cast<LogWriter *>(arg);
    //If SD card is already ok, we should check if any log files exist on /data.
    //If exists, move them to data
    if (pLogWriter->m_bLog2SD == LOG_ON_SD) {
        // stop clear data logs
        if (pLogWriter->m_bNeedMoveBootupLog) {
            pLogWriter->moveBootupLog();
            pLogWriter->m_bNeedMoveBootupLog = false;
            sleep(1);//prevent thread return quickly, otherwise,maybe the thread's 
            // tid will be re-use again
        }
        pLogWriter->m_thrWaitingSD = 0;
        LOGV("m_bLog2SD = LOG_ON_SD,move bootup logs then return");
        return NULL;
    }
    pLogWriter->startClearDataLogThread();
    //If SD card is not OK, monitor SD card status.
    int ret = pLogWriter->startCheckSD(900);
    while (ret == RET_TIMEOUT) {
        LOGD("waiting SD card timeout");
        if (!pathFromMTKLogger) {
            LOGD("waiting SD card:not receive log path,keep in data partition");
            ret = pLogWriter->startCheckSD(900);
        }
    }

    if (ret == ERR_GETSDSYSTEM || ret == ERR_SDFULL || ret == ERR_SDNOTREADY) {
        LOGI("Waiting SD card: Time out, SD card is not ready");
        
        while (pLogWriter->isWritingLog()) {
            sleep(1);
            continue;
        }

        sleep(2);
        if (ret == ERR_GETSDSYSTEM || ret == ERR_SDNOTREADY) {
            pLogWriter->notifyMTKLogger(MSG_SDCARD_NO_EXIST);
        } else if (ret == ERR_SDFULL) {
            pLogWriter->notifyMTKLogger(MSG_SDCARD_IS_FULL);
        }
        return NULL;
    } else if (ret == RET_SDREADY) {
        LOGI("Waiting SD card: SD card is ready");
        // stop clear data logs
        int repeat = 3;
        int mSdFolderCreated = false;
        if (pLogWriter->m_bPaused == true) {
            LOGI("Waiting SD card: SD card is ready,but log stoped,return");
            pLogWriter->m_thrWaitingSD = 0;
            return NULL;
        }
        while (repeat-- > 0) {
            if (pLogWriter->createLogFilesOnSD()) {
                mSdFolderCreated = true;
                break;
            } else {
                sleep(2);
            }
        }
        if (mSdFolderCreated) {
            property_set(PROP_CONSYSLOGGER_RUNNING_STATUS, "1");
            if(isEngineerBuild()){
            LOGD("thrWaitingSDReady setproperty %s: 1.", PROP_CONSYSLOGGER_RUNNING_STATUS);
            }
            sleep(3);
            while (pLogWriter->isWritingLog()) {
                sleep(1);
                continue;
            }
            if (pLogWriter->m_bNeedMoveBootupLog) {
                pLogWriter->moveBootupLog();
                pLogWriter->m_bNeedMoveBootupLog = false;
            }
        } else {
            LOGE("Create file on sd failed.");
        }

    } else if (ret == RET_STOPCHECK) {
        LOGD("Waiting SD card: Stop checking SD card status");
    }
    pLogWriter->m_thrWaitingSD = 0;
    LOGI("Waiting SD card thread return.");
    return NULL;
}

LogWriter::LogWriter(LISTENER listener) {
    m_nBufCnt = 4;
    m_ppSDFiles = NULL;
    m_ppNandFiles = NULL;
    m_ppFiles = NULL;
    m_nIOBlkSize = 0;
    m_nReservedSpace = SD_RESERVED_SPACE;
    m_bInformNoLogFile = false;
    m_bPaused = true;
    m_bLog2SD = LOG_ON_NONE;
    m_bNeedMoveBootupLog = true;
    m_bCheckingSD = false;
    for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
        m_bWritingLog[i] = false;
    }
    m_listener = listener;
    ASSERT(m_listener != NULL);
    pthread_mutex_init(&m_mutexCheckLogFile, NULL);
    m_bLogRecycle = false;
    pthread_mutex_init(&mutexRecycle, NULL);
    pthread_cond_init(&condRecycle, NULL);
    m_thrWaitingSD = 0;
    threadStartLogRecycle = 0;
    threadClearDataLog = 0;

    bootuploglist.clear();
    memset(m_bootuplogFile,'\0',BaseFile::PATH_LEN);
    mSDcardFull = false;
    mClearLogSize = 0;
    mLogNeedToBeClearSize = 0;
    memset(m_externalSDPath,0,MAX_PATH_LEN);
}

LogWriter::~LogWriter() {
    m_thrWaitingSD = 0;
    bootuploglist.clear();
    mSDcardFull = false;
    memset(m_externalSDPath,0,MAX_PATH_LEN);
    pthread_mutex_destroy(&m_mutexCheckLogFile);
    pthread_mutex_destroy(&mutexRecycle);
    pthread_cond_destroy(&condRecycle);

}

bool LogWriter::init(int nBufCnt) {
    m_nBufCnt = nBufCnt;
    mSDcardFull = false;
    LOGV("Init LogWriter: Buffer count: %d", m_nBufCnt);
    m_ppFiles = new BaseFile*[m_nBufCnt];
    m_ppNandFiles = new BaseFile*[m_nBufCnt];
    m_ppSDFiles = new BaseFile*[m_nBufCnt];
    RESET_FILE_POINTER_ARRAY(m_ppFiles, m_nBufCnt);
    RESET_FILE_POINTER_ARRAY(m_ppNandFiles, m_nBufCnt);
    RESET_FILE_POINTER_ARRAY(m_ppSDFiles, m_nBufCnt);




    return true;
}

bool LogWriter::deinit() {
    if(isEngineerBuild()){
        LOGD("Deinit LogWriter");
    }
    onReceiveSignal();
    pthread_mutex_lock(&m_mutexCheckLogFile);
    CLEAR_FILE_POINTER_ARRAY(m_ppFiles, m_nBufCnt);
    CLEAR_FILE_POINTER_ARRAY(m_ppNandFiles, m_nBufCnt);
    CLEAR_FILE_POINTER_ARRAY(m_ppSDFiles, m_nBufCnt);
    pthread_mutex_unlock(&m_mutexCheckLogFile);

    m_nBufCnt = 0;
    m_nIOBlkSize = 0;
    m_nReservedSpace = SD_RESERVED_SPACE;
    return true;
}

int LogWriter::startCheckSD(int timeout) {
    m_bCheckingSD = true;
    return checkSDStatus(timeout);
}

void LogWriter::stopCheckSD() {
    m_bCheckingSD = false;
}

// Check SD card is ready, including the SD card is full or not
int LogWriter::checkSDStatus(int retryNumber) {
    LOGD("Check SD card status");
    struct statfs fileStat;
    memset(&fileStat,0,sizeof(fileStat));
    while (m_bCheckingSD) {
        //call here because SD related property value may be changed
        //1. when device is booting, some properties may be not set.
        initLogPath();

        if (statfs(SD_FILE_SYSTEM, &fileStat) == -1) {
            if (retryNumber-- > 3) {
                sleep(2);
                continue;
            }
            LOGE("ERR_GETSDSYSTEM, SD card status: %s, errno=%d", SD_FILE_SYSTEM, errno);
            return ERR_GETSDSYSTEM;
        } else {
            LOGV("SDCard: Waiting for SD card ready...");
            if (0 == fileStat.f_blocks) {
                if (0 == --retryNumber) {
                    LOGE("ERR_SDNOTREADY SD card status: unavailable.");
                    return ERR_SDNOTREADY;
                } else {
                    sleep(2);
                    continue;
                }
            }
            // m_nBufCnt * m_nIOBlkSize bytes space is reserved for closing files.
            m_nIOBlkSize = fileStat.f_bsize;
            if (fileStat.f_bavail * fileStat.f_bsize / 1024
                    <= (SD_RESERVED_SPACE + m_nBufCnt * m_nIOBlkSize) / 1024) {
                LOGE("ERR_SDFULL SD card status: no enough space left.");
                return ERR_SDFULL;
            } else {
                if (retryNumber-- > 5) {
                    if(isEngineerBuild()){
                        LOGD("Check  boot completed status and receive start command");
                    }
                    if (isBootCompleted()) {
                        if (checkReceivedStartCommands()) {
                            LOGD("Check SD card status: SD card is ready.");
                            return RET_SDREADY;
                        }
                     }
                     sleep(2);
                     continue;
                } else if (retryNumber-- > 0) {
                    if (checkReceivedStartCommands()) {
                        if(isEngineerBuild()){
                            LOGD("Check SD card status: received command.RET_SDREADY");
                        }
                        return RET_SDREADY;
                    } else {
                        usleep(500000);
                        continue;
                    }
                } else {
                    LOGD("SD card status: try RET_TIMEOUT");
                    return RET_TIMEOUT;
                }
            }
        }
    }
    LOGD("Check SD card status: Stop checking.");
    return RET_STOPCHECK;
}

bool LogWriter::saveLogFile(const char *data, unsigned int len,
        unsigned char bufId, LOG_LOCATION logLocation) {
    static bool bNotifyFull = false;
    LOGV("saveLogFile(%d):Log %d file", logLocation, bufId);
    //save log file should check log file pointer log m_mutexCheckLogFile,
    //in order to ensure file pointer is not changed.
    pthread_mutex_lock(&m_mutexCheckLogFile);
    bool ret = false;
    do {
        if (LOG_ON_NONE == logLocation) {
            //Don't break, just add log to see if this case may happen or not.
            //Maybe add break if find case happens and cause errors.
            LOGE("saveLogFile(): Need to create file on SD or /data firstly");
        }
        if (NULL == m_ppFiles || NULL == m_ppFiles[bufId]) {
            LOGE("saveLogFile(%d):Log %d file pointer is Empty",
                    logLocation, bufId);
            break;
        }

        if (m_ppFiles[bufId]->isExist() != true) {
            LOGD("saveLogFile(%d): The %d log file doesn't exist.",
                    logLocation, bufId);
            notifyMTKLogger(MSG_SDCARD_NO_LOG_FILE);
            break;
        }

        // Check if a single file exceeds size limit
        if (m_ppFiles[bufId]->isExceeded()) {
            closeLogFile(m_ppFiles[bufId]);
            bool tmp = createLogFile(m_ppFiles[bufId], (int) bufId);
            if (tmp != true) {
                LOGE("saveLogFile(%d): Failed to create log files,Notify UI",
                        logLocation);
                notifyMTKLogger(MSG_SDCARD_NO_LOG_FILE);
                break;
            }
            if (logLocation == LOG_ON_SD) {
                size_t path_len = strlen(m_ppFiles[bufId]->m_szPath);
                generateLogTreeFile(m_ppFiles[bufId]->m_szPath, path_len, false, false);
            }
        }
        // Check if SD card is full
        unsigned long sdSpace = getStorageLeftSpace(logLocation);
        if (sdSpace >= (m_nReservedSpace + len) / 1024) {
            bNotifyFull = false;
            LOGV("saveLogFile(%d): Write %d bytes to log file %s",
                    logLocation, len, m_ppFiles[bufId]->m_szPath);
            ret = writeLogFile(m_ppFiles[bufId], data, len);
        } else {
            if (!bNotifyFull) {
                bNotifyFull = true;
                LOGE("saveLogFile(%d): Storage is full, Pause logger, and Notify",
                        logLocation);
                notifyMTKLogger(MSG_SDCARD_IS_FULL);
            }
            ret = false;
        }
    } while(0);
    pthread_mutex_unlock(&m_mutexCheckLogFile);
    return ret;
}

bool LogWriter::isWritingLog() {
    bool ret = true;
    int mMax = MAX_BUFFER_COUNT;
    if (MAX_BUFFER_COUNT > m_nBufCnt) {
        mMax = m_nBufCnt;
    }
    for (int i = 0; i < mMax; i++) {
        ret &= m_bWritingLog[i];
    }
    return ret;
}

bool LogWriter::onReceiveLogData(const char *data, unsigned int len,
        unsigned char bufId) {
    LOGV("LogWriter receive %u bytes from buffer %u", len, bufId);

    onReceiveSignal();
    if (mSDcardFull) {
        LOGD("onReceive: SD card full. pausing logging return true to drop log.");
        return true;
    }
    if (m_bPaused == true) {
        LOGD("onReceive: Logger is paused, can't save log");
        return false;
    }
    if (bufId >= m_nBufCnt) {
        LOGE("onReceive: Invalid buffer id %u, should be in 0~%d", bufId, m_nBufCnt - 1);
        return false;
    }
    bool ret = true;

          m_bWritingLog[bufId] = true;
        ret = saveLogFile(data, len, bufId, m_bLog2SD);
        if (m_bLog2SD == LOG_ON_DATA) {
            usleep(1000);
        }
        m_bWritingLog[bufId] = false;
        
        return ret;
    }


bool LogWriter::moveBootupLog() {
    if(isEngineerBuild()){
    LOGD("Move Bootup Log: Start to move bootup log");
    }
    DIR* dp = NULL;
    char srcname[512];
    char dstname[512];
    struct dirent* dirp;
    dp = opendir(CONSYSLOGGER_BOOTUP_LOG_FOLDER);
    if (dp == NULL) {
        LOGE("Move Bootup Log: %s is not exist!", CONSYSLOGGER_BOOTUP_LOG_FOLDER);
        return false;
    }


    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        memset(srcname, '\0', 512);
        sprintf(srcname,"%s/%s",CONSYSLOGGER_BOOTUP_LOG_FOLDER,dirp->d_name);
        memset(dstname, '\0', 512);
        sprintf(dstname,"%s/%s",m_szFolderPath,dirp->d_name);
        if (m_bPaused) {
            LOGE("Stop move bootup log for logging paused.");
            closedir(dp);
            dirp = NULL;
            return true;
        }
        if(isEngineerBuild()){
        LOGD("Move Bootup Log:  from %s to %s", srcname, dstname);
        }
        copyFile(srcname, dstname, false);
        if (remove(srcname) == -1) {
            LOGE("remove Bootup log :%s failed: %s",srcname,strerror(errno));
        }
        usleep(50000);
    }
    rmdir(CONSYSLOGGER_BOOTUP_LOG_FOLDER);
    closedir(dp);
    dirp = NULL;
    LOGD("Move Bootup Log: Move done");
    return true;
}

bool LogWriter::onStartLogging() {
    if(isEngineerBuild()){
    LOGD("LogWriter: Logging is started");
    }
    bool ret = true;
    m_bPaused = false;
    m_bInformNoLogFile = false;
    mSDcardFull = false;
    initLogPath();


    return ret;
}

bool LogWriter::onPostStartLogging() {
    return true;
}

bool LogWriter::onPreStopLogging() {
    return true;
}

bool LogWriter::onStopLogging() {
    m_bPaused = true;
    LOGD("LogWriter: Logging is stoped");
    return true;
}

void LogWriter::onReceiveSignal() {
    pthread_mutex_lock(&mutexRecycle);
    pthread_cond_broadcast(&condRecycle);
    pthread_mutex_unlock(&mutexRecycle);
}

bool LogWriter::CheckSDPlug() {
    char sdRoot[256];
    memset(sdRoot,0,256);
    strncpy(sdRoot,"storage",sizeof(sdRoot) - 1);
    sdRoot[sizeof(sdRoot) - 1] = '\0';
    
    DIR* dp = NULL;
    struct dirent* dirp = NULL;
    dp = opendir(sdRoot);
    if (dp == NULL) {
        LOGE("can not access: %s is not exist,errno=%d!", sdRoot,errno);
        return false;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        if (strstr(dirp->d_name, "-") == NULL) {
            continue;
        }else{
          LOGD("SD card is inserted. detect it:%s!",dirp->d_name);
          sprintf(m_externalSDPath,"%s/%s",sdRoot,dirp->d_name);
          closedir(dp);
          dirp = NULL;
          return true;
        }
    }
    closedir(dp);
    dirp = NULL;
    LOGD("not detect SD card is inserted.")
    return false;
}

/**
 * Initialize log path.
 */
    void LogWriter::initLogPath() {
    if(isEngineerBuild()){
       LOGI("init log Path.");
    }
    strncpy(SD_FILE_SYSTEM, PROP_VALUE_LOG_PATH_DEFAULT, sizeof(SD_FILE_SYSTEM) - 1);
    SD_FILE_SYSTEM[sizeof(SD_FILE_SYSTEM) - 1] = '\0';
    sprintf(SD_LOG_ROOT, SD_LOG_ROOT_FORMAT, PROP_VALUE_LOG_PATH_DEFAULT);

    char sdRootTemp[MAX_PATH_LEN];
    memset(sdRootTemp, '\0', MAX_PATH_LEN);
    property_get(PROP_LOG_SAVE_PATH, sdRootTemp, "none");

    if (BootMode == ATE_FACTORY_BOOT_MODE || BootMode == FACTORY_BOOT) {
        bool pluged = CheckSDPlug();
        if (true == pluged) {
            strncpy(sdRootTemp, m_externalSDPath, sizeof(sdRootTemp) - 1);
            sdRootTemp[sizeof(sdRootTemp) - 1] = '\0';
        } else {
            strncpy(sdRootTemp, "mnt/sdcard2", sizeof(sdRootTemp) - 1);
            sdRootTemp[sizeof(sdRootTemp) - 1] = '\0';
        }
    }

    if(BootMode == META_BOOT) {
        if (CheckSDPlug() == true) {
           LOGD("SD card be pluged in,change log path.");
           strncpy(sdRootTemp, m_externalSDPath, sizeof(sdRootTemp) - 1);
           sdRootTemp[sizeof(sdRootTemp) - 1] = '\0';
         }
    }

    
    if (strncmp("none", sdRootTemp, strlen("none"))) {
        sprintf(SD_LOG_ROOT, SD_LOG_ROOT_FORMAT, sdRootTemp);
        strncpy(SD_FILE_SYSTEM, sdRootTemp, sizeof(SD_FILE_SYSTEM) - 1);
        SD_FILE_SYSTEM[sizeof(SD_FILE_SYSTEM) - 1] = '\0';
        pathFromMTKLogger = true;
        LOGI("SD_LOG_ROOT is init from property %s", PROP_LOG_SAVE_PATH);
    }
    LOGD("SD_LOG_ROOT is %s.", SD_LOG_ROOT);
}

bool LogWriter::generateLogTreeFile(const char *data, size_t len,
        bool isfoldertree, bool isnewcreate) {
    LOGV("Generate log folder tree file, isFolderTrue:%d, isNewCreat:%d",
            isfoldertree, isnewcreate);
    char filePath[BaseFile::PATH_LEN];
    if (isfoldertree) {
        sprintf(filePath, FILE_TREE_NAME, SD_LOG_ROOT);
        strncpy(topTreeFile, filePath, sizeof(topTreeFile) - 1);
        topTreeFile[sizeof(topTreeFile) - 1] = '\0';
    } else {
        sprintf(filePath, FILE_TREE_NAME, m_szFolderPath);
    }

    FILE *pFile = NULL;
    if (isnewcreate) {
        pFile = fopen(filePath, "wb+");
    } else {
        pFile = fopen(filePath, "ab+");
    }

    if (NULL == pFile) {
        LOGE("Failed to open/create tree file %s, errno=%d", filePath, errno);
        return false;
    }
    LOGV("File %s open.", filePath);

    size_t res = fwrite(data, sizeof(char), len, pFile);

    if (res != len) {
        fclose(pFile);
        pFile = NULL;
        return false;
    }

    size_t length = strlen("\r");
    res = fwrite("\r", sizeof(char), length, pFile);
    if (res != length) {
        fclose(pFile);
        pFile = NULL;
        return false;
    }
    if (isEngineerBuild()){
    LOGD("Success to write file tree file.");
    }
    if (fclose(pFile) == EOF) {
        LOGE("Failed to close file tree file %s, errno=%d", filePath, errno);
        pFile = NULL;
        return false;
    }
    pFile = NULL;
    return true;
}

void LogWriter::clearOldLogFiles() {
    LOGD("Clear old log files on data partition");

    DIR* dp = NULL;
    char *pathname = (char*) malloc(256);
    struct dirent* dirp;
    dp = opendir(CONSYSLOGGER_BOOTUP_LOG_FOLDER);
    if (dp == NULL) {
        LOGD("Log folder %s is not exist!", CONSYSLOGGER_BOOTUP_LOG_FOLDER);
        free(pathname);
        return;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
            continue;
        }
        sprintf(pathname,"%s/%s",CONSYSLOGGER_BOOTUP_LOG_FOLDER,dirp->d_name);
        LOGD("Delete data partition log file: %s.", pathname);
        if (remove(pathname) == -1) {
           LOGE("Delete data partition log file:%s failed: %s",pathname,strerror(errno));
        }
    }
    rmdir(CONSYSLOGGER_BOOTUP_LOG_FOLDER);
    closedir(dp);
    free(pathname);
    pathname = NULL;
    dirp = NULL;
}

bool LogWriter::createLogFilesOnNand() {
    LOGD("Create log file on Nand");
    m_bLog2SD = LOG_ON_DATA;
    clearOldLogFiles();
    if (getStorageLeftSpace(LOG_ON_DATA) < m_nReservedSpace / 1024) {
        // SD has no space left for create a new file
        LOGE("Cannot create log files on Nand due to no enough space left.");
        return false;
    }
    //2. create log folder if needed
    if (mkdir(CONSYSLOGGER_BOOTUP_LOG_FOLDER, 0777) != -1) {
        if(isEngineerBuild()){
        LOGD("%s is created as log folder", CONSYSLOGGER_BOOTUP_LOG_FOLDER);
        }
        strncpy(m_szFolderPath, CONSYSLOGGER_BOOTUP_LOG_FOLDER, sizeof(m_szFolderPath) - 1);
        m_szFolderPath[sizeof(m_szFolderPath) - 1] = '\0';
    } else {
        if (errno == 17) {
            strncpy(m_szFolderPath, CONSYSLOGGER_BOOTUP_LOG_FOLDER, sizeof(m_szFolderPath) - 1);
            m_szFolderPath[sizeof(m_szFolderPath) - 1] = '\0';
            if(isEngineerBuild()){
            LOGD("Folder %s is exist, errno=%d", CONSYSLOGGER_BOOTUP_LOG_FOLDER, errno);
            }
        } else {
            LOGE("Failed to create folder %s on Nand, errno=%d", CONSYSLOGGER_BOOTUP_LOG_FOLDER, errno);
            m_szFolderPath[0] = '\0';
            return false;
        }
    }
    property_set(PROP_LOGGER_RUNNING_FOLDER,m_szFolderPath);
    //3. Create Log files
    RESET_FILE_POINTER_ARRAY(m_ppNandFiles, m_nBufCnt);
    bool ret = true;
    for(int i = 0; i < m_nBufCnt && (true == ret); i++) {
        ret = createLogFile(m_ppNandFiles[i], i);
    }
    if (ret) {
        if (isEngineerBuild()) {
        LOGD("Create log files: All %d log files have been created.", m_nBufCnt);
        }
    } else {
        LOGE("Create log files: Failed to create all %d log files.", m_nBufCnt);
    }

    if (true == ret) {
        pthread_mutex_lock(&m_mutexCheckLogFile);
        for (int j = 0; j < m_nBufCnt; j++) {
            if (m_ppFiles[j] != NULL) {
                closeLogFile(m_ppFiles[j]);
            }
            m_ppFiles[j] = m_ppNandFiles[j];
        }
        RESET_FILE_POINTER_ARRAY(m_ppNandFiles, m_nBufCnt);
        pthread_mutex_unlock(&m_mutexCheckLogFile);
    } else {
        closeLogFiles(m_ppNandFiles);
        RESET_FILE_POINTER_ARRAY(m_ppNandFiles, m_nBufCnt);
    }

    return ret;
}

bool LogWriter::createLogFilesOnSD() {
    if(isEngineerBuild()) {
    LOGD("Create log file on SD card");
    }
    //1. Check SD space
    size_t sdSpace = getStorageLeftSpace(LOG_ON_SD);
    if (sdSpace < m_nReservedSpace / 1024) {
        // SD has no space left for create a new file
        LOGE("Cannot create log files on SD due to no enough space left.");
        return false;
    }

    //2. Create Log folder if needed
    if (makeDirWithPermission(SD_LOG_ROOT,create_mode) == false) {
        LOGE("Failed to create root log folder %s",SD_LOG_ROOT);
        return false;
    }
    char szTime[17];
    //TODO: szFolderPath may out of range
    char szFolderPath[BaseFile::PATH_LEN];
    memset(szFolderPath,0,BaseFile::PATH_LEN);
    getCurrentTimeStr(szTime, NULL);
    //Make the path for log folder
    sprintf(szFolderPath, FOLDER_NAME, SD_LOG_ROOT, szTime);

    if (mkdir(szFolderPath, create_mode) != -1) {
        strncpy(m_szFolderPath, szFolderPath, sizeof(m_szFolderPath) - 1);
        m_szFolderPath[sizeof(m_szFolderPath) - 1] = '\0';
        LOGD("Success to create log folder %s", m_szFolderPath);
    } else {
        if (errno == 17) {
            strncpy(m_szFolderPath, szFolderPath, sizeof(m_szFolderPath) - 1);
            m_szFolderPath[sizeof(m_szFolderPath) - 1] = '\0';
            if(isEngineerBuild()){
            LOGD("Folder %s is exist, errno=%d", m_szFolderPath, errno);
            }
        } else {
            LOGE("Failed to create folder %s on SD, errno=%d", m_szFolderPath, errno);
            m_szFolderPath[0] = '\0';
            return false;
        }
    }
      property_set(PROP_LOGGER_RUNNING_FOLDER,m_szFolderPath);
    //3.Generate root tree file, version file
    size_t len = strlen(m_szFolderPath);
    generateLogTreeFile(m_szFolderPath, len, true, false);


    //TODO:to remove this block logic
    //4. Create Log file
    RESET_FILE_POINTER_ARRAY(m_ppSDFiles, m_nBufCnt);
    bool ret = true;
    for(int i = 0; i < m_nBufCnt && (true == ret); i++) {
        ret = createLogFile(m_ppSDFiles[i], i);
    }
    if (ret) {
        if(isEngineerBuild()){
        LOGD("Create log files: All %d log files have been created.", m_nBufCnt);
        }
    } else {
        LOGE("Create log files: Failed to create all %d log files.", m_nBufCnt);
    }
    if (true == ret) {
        pthread_mutex_lock(&m_mutexCheckLogFile);
        for (int j = 0; j < m_nBufCnt; j++) {
            if (m_ppFiles[j] != NULL) {
                closeLogFile(m_ppFiles[j]);
            }
            m_ppFiles[j] = m_ppSDFiles[j];
           // size_t len = strlen(m_ppSDFiles[j]->m_szPath);
          //  generateLogTreeFile(m_ppSDFiles[j]->m_szPath, len, false, false);
        }
        RESET_FILE_POINTER_ARRAY(m_ppSDFiles, m_nBufCnt);
        //This variable m_bLog2SD should be set with SD file pointer together,
        //because we this this variable to decide the logging location.
        m_bLog2SD = LOG_ON_SD;
        pthread_mutex_unlock(&m_mutexCheckLogFile);
    } else {
        closeLogFiles(m_ppSDFiles);
        RESET_FILE_POINTER_ARRAY(m_ppSDFiles, m_nBufCnt);
    }
    return ret;
}

#define COPY_BLKSZ (1024 * 16 * 64)

#ifndef MAP_FAILED
#define MAP_FAILED ((void)-1)
#endif

void LogWriter::copyFile(char *srcFile, char*destFile, bool cancelable) {
    if (m_bPaused && cancelable == true) {
        LOGE("No need Copy File bootupLog for stop logging");
        return;
    }
    int fdSrcFile, fdDestFile;
    size_t size;
    struct stat sb;
    char *psrc = NULL;
    char *ptr = NULL;

    fdSrcFile = open(srcFile, O_RDONLY);
    if (fdSrcFile < 0) {
        LOGE("Can not open srcFile: %s", srcFile);
        return;
    }
    if (fdSrcFile >= 0) {
        char tmpDestFile[256];
        sprintf(tmpDestFile, "%s.tmp", destFile);
        fdDestFile = open(tmpDestFile, O_RDWR | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR);
        if (fdDestFile >= 0) {
            if (fstat(fdSrcFile, &sb) == 0) {
                size = sb.st_size;
                psrc = (char *) mmap(NULL, size, PROT_READ, MAP_PRIVATE,
                        fdSrcFile, 0);
                if (psrc != MAP_FAILED) {
                    for (ptr = psrc; size > COPY_BLKSZ;
                            size -= COPY_BLKSZ, ptr += COPY_BLKSZ) {
                        if (-1 == write(fdDestFile, ptr, COPY_BLKSZ)) {
                            close(fdSrcFile);
                            close(fdDestFile);
                            munmap(psrc, sb.st_size);

                            LOGE("Copy File error happened %s.", strerror(errno));
                            return;
                        }
                        if (m_bPaused && cancelable == true) {
                            LOGE("Copy File bootup log error pause for stop logging");
                            close(fdSrcFile);
                            close(fdDestFile);
                            munmap(psrc, sb.st_size);
                            return;
                        }
                        usleep(100000);
                        fdatasync(fdDestFile);
                    }
                    write(fdDestFile, ptr, size);
                }
            }
            fdatasync(fdDestFile);
            close(fdDestFile);
            int err = rename(tmpDestFile, destFile);
            if (err < 0) {
                LOGE("copy file -- rename file error. %d", err);
            }
        } else {
            LOGE("Can not open destFile: %s.", destFile);
        }
        if (psrc != NULL) {
            munmap(psrc, sb.st_size);
        }
        close(fdSrcFile);
    }
}

bool LogWriter::createLogFile(BaseFile* &pFile, int nBufId) {
    char filePath[BaseFile::PATH_LEN];
    if (NULL != pFile) {
        LOGD("Create log file: Log File is already created");
        return true;
    }

    int mRetry = 5;
    while (mRetry-- > 0) {
        if (0 != access(m_szFolderPath, F_OK)) {
            if(isEngineerBuild()){
            LOGD("createLogFile: log folder not exist, create a new one %s",
                    m_szFolderPath);
            }
            if (!makeDirWithPermission(m_szFolderPath, 0777)) {
                LOGE("Create log file: Failed to create folder %s when create file %d",
                        m_szFolderPath, errno);
            } else {
                sleep(1);
            }
        } else {
            break;
        }
    }

    if (mRetry <= 0) {
        return false;
    }

    char szTime[17];
    getCurrentTimeStr(szTime, NULL);
    char szMTBF_tag[8];
    memset(szMTBF_tag, '\0', sizeof(char) * 8);

    //Make the path for log file
    sprintf(filePath, LOG_FILE_NAME, m_szFolderPath, GetKeyWord(nBufId),szTime, "clog");
    pFile = new LogFile(nBufId);
    strncpy(szMTBF_tag, ".curf", sizeof(szMTBF_tag) - 1);
    szMTBF_tag[sizeof(szMTBF_tag) - 1] = '\0';
   
    strncat(filePath, szMTBF_tag,strlen(szMTBF_tag));
    //Create and open log file

    memset(m_szCurrentLogFile, '\0', BaseFile::PATH_LEN);
    strncpy(m_szCurrentLogFile, filePath, sizeof(m_szCurrentLogFile) - 1);
    m_szCurrentLogFile[sizeof(m_szCurrentLogFile) - 1] = '\0';
    
    bool ret = pFile->openFile(filePath);

    return ret;
}

bool LogWriter::closeLogFiles(BaseFile** &ppFiles) {
    if (ppFiles == NULL) {
        LOGE("Close log files: NULL pointer");
        return false;
    }

    bool ret = true;
    int i = 0;
    while (i < m_nBufCnt) {
        ret = closeLogFile(ppFiles[i]);
        i++;
    }

    if (ret) {
        if(isEngineerBuild()) {
        LOGD("Close log files: All %d log files have been closed.", m_nBufCnt);
        }
    } else {
        LOGE("Close log files: Failed to close all %d log files.", m_nBufCnt);
    }
    return ret;
}

bool LogWriter::closeLogFile(BaseFile* &pFile) {
    if (NULL == pFile) {
        LOGE("Close log file: File is not open yet");
        return true;
    }
    if(isEngineerBuild()){
    LOGD("Close log file and delete the pointer.");
    }
    if (m_bLog2SD == LOG_ON_DATA &&
        strstr(pFile->m_szPath,CONSYSLOGGER_BOOTUP_LOG_FOLDER) != NULL) {
        char bootuplogFile[BaseFile::PATH_LEN]={0};
        memset(bootuplogFile,'\0',BaseFile::PATH_LEN);
        strncpy(bootuplogFile, pFile->m_szPath, strlen(pFile->m_szPath) - strlen(".curf"));
        bootuplogFile[sizeof(bootuplogFile) - 1] = '\0';
        bootuploglist.push_back(bootuplogFile);
        LOGD("new boot up file : %s",bootuplogFile);
    } else {
        size_t len = strlen(pFile->m_szPath);
        generateLogTreeFile(pFile->m_szPath, len, false, false);
    }
    bool ret = pFile->cleanCloseFile();
    //make sure pFile is delete
    if (NULL != pFile) {
        delete pFile;
    }
    pFile = NULL;
    if (ret == false) {
        LOGE("Close false maybe File hanle is null.won't refree handle for causing seldom NE");
    }
    return ret;
}

bool LogWriter::writeLogFile(BaseFile* &pFile, const char *ptr,
        unsigned int len) {
    bool ret = true;
    unsigned int bytesWriten = 0;
    unsigned int bytesLeft;
    int nCountRepeat = 0;
    while (bytesWriten < len) {
        if (NULL == pFile) {
            LOGE("Write log file: File is not opened yet, file pointer is null");
            return false;
        }
        if (nCountRepeat > 0) {
            LOGV("Write log to file repeat: %d", nCountRepeat);
        }
        nCountRepeat++;
        bytesLeft = len - bytesWriten;
        size_t res = pFile->writeFile(ptr + bytesWriten, bytesLeft);
        if (res == 0) {
            LOGE("Write log file: Only write %uB into file", bytesWriten);
            ret = false;
            break;
        } else if (res > 0) {
            bytesWriten += res;
        } else {
            LOGE("Write log file: Error: %d", errno);
            return false;
        }
    }
    return ret;
}

bool LogWriter::flushFile(int nBufId) {
    if (nBufId < 0 || nBufId >= m_nBufCnt || m_ppFiles == NULL) {
        LOGE("Flush log file: Buffer ID is not right or file pointer is null");
        return false;
    }
    if (m_ppFiles[nBufId] == NULL) {
        LOGE("Flush log file: m_ppFiles[nBufId] is null");
        return false;
    }
    if (isEngineerBuild()){
    LOGD("Flush log file: %s", m_ppFiles[nBufId]->m_szPath);
    }
    return m_ppFiles[nBufId]->flushFile();
}


unsigned long LogWriter::getStorageLeftSpace(LOG_LOCATION logLocation) {
    unsigned long ret;
    struct statfs fileStat;
    memset(&fileStat,0,sizeof(fileStat));
    char storage[255];
    memset(storage,0,255);
    
    if (logLocation == LOG_ON_DATA) {
        sprintf(storage,"%s","/data");
    } else {
       sprintf(storage,"%s",SD_FILE_SYSTEM);
    }
    RECHECK:
    if (statfs(storage, &fileStat) == -1) {
        LOGE("Get storage :%s ,left space: Failed to get the storage status, errno=%d", storage, errno);
        if (errno == EINTR) {
            usleep(200000);
            goto RECHECK;
        }
        return -1;
    } else if (0 == fileStat.f_blocks) {
        // Disk is not available
        LOGE("Get left space:%s not available",storage);
        return -1;
    }
    m_nIOBlkSize = fileStat.f_bsize;
    ret = (unsigned long) ((fileStat.f_bavail * fileStat.f_bsize) / 1024);
    LOGV("Get left space: %lu", ret);
    return ret;
}


bool SdLogWriter::onStartLogging() {
    bool ret = LogWriter::onStartLogging();
    int sdstatus = startCheckSD(3);
    if (sdstatus == ERR_GETSDSYSTEM || sdstatus == ERR_SDFULL || sdstatus == ERR_SDNOTREADY) {
        LOGD("Failed to init logwriter: SD card status can't be got or is full.");
    }
    if (sdstatus == RET_SDREADY) {
        if(isEngineerBuild()){
            LOGD("Init LogWriter: SD card is ready, record log on SD card");
        }
        ret = createLogFilesOnSD();
        property_set(PROP_CONSYSLOGGER_RUNNING_STATUS, "1");
        if(isEngineerBuild()) {
        LOGD("init setproperty %s: 1.", PROP_CONSYSLOGGER_RUNNING_STATUS);
        }
    } else {
        //we only record log on data/ when device does not boot up,
        //Otherwise, it may stop.
        if (sdstatus == ERR_GETSDSYSTEM ||sdstatus == ERR_SDNOTREADY || sdstatus == RET_TIMEOUT) {
            LOGD("Init LogWriter: SD card is not ready, record log on Nand");
            ret = createLogFilesOnNand();
            //Sometimes /data may not be created rightly due to system error, so retry it.
            if (ret == false) {
                ret = createLogFilesOnNand();
            }
        } else if (sdstatus == ERR_SDFULL) {
            ret = false;
        }
    }
    //If SD card is ready, we create a thead to move log in /data/ if needed
    //If SD card is not ready, we create a thead to monitor SD card status.
    if (ret == true) {
        m_bNeedMoveBootupLog = true;
        PTHREAD_CREATE("csl_sd_monitor", &m_thrWaitingSD, NULL,
                  thrWaitingSDReady, this);
    }
    return ret;
}

bool SdLogWriter::onPostStartLogging() {
    bool ret = LogWriter::onPostStartLogging();
    startLogRecycleThread();
    return ret;
}
bool SdLogWriter::onPreStopLogging() {
    bool ret = LogWriter::onPreStopLogging();
    stopLogRecycleThread();
    return ret;
}

bool SdLogWriter::onStopLogging() {
    bool ret = LogWriter::onStopLogging();
    pthread_mutex_lock(&m_mutexCheckLogFile);
    if (m_ppFiles != NULL) {
        ret = closeLogFiles(m_ppFiles);
        RESET_FILE_POINTER_ARRAY(m_ppFiles, m_nBufCnt);
    }
    
    pthread_mutex_unlock(&m_mutexCheckLogFile);
    stopCheckSD();

    if (LOG_ON_SD == m_bLog2SD) {
        if (isPthreadAlive(m_thrWaitingSD)) {
            int retry = 0;
            do {
                stopCheckSD();
                usleep(500000);//wait for 0.5s, total 60s
            } while (++retry < 20 && isPthreadAlive(m_thrWaitingSD));
            LOGD("LogWriter: wait for sd monitor thread quit. retry:%d", retry);
        }
    } else {
        //stop sdstate_monitor thread if exists
        if (isPthreadAlive(m_thrWaitingSD)) {
            int retry = 20;
            do {
                stopCheckSD();
                usleep(100000);//wait for 0.1s, totle 2s
            } while (--retry > 0 && isPthreadAlive(m_thrWaitingSD));
            LOGD("LogWriter: wait for sd monitor thread quit. retry:%d", retry);
        }
    }
    m_thrWaitingSD = 0;
    if (isPthreadAlive(threadClearDataLog)) {
            int retry = 0;
            do {
                onReceiveSignal();
                usleep(100000);
            } while (++retry < 20 && isPthreadAlive(threadClearDataLog));
            LOGD("LogWriter: data recycle quit. retry:%d", retry);
        }
    return ret;
}

}

