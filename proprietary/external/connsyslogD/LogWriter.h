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

/******************************************************************************
 *
 * Filename:
 * ---------
 *   logwriter.h
 *
 * Description:
 * ------------
 *   Header file of logwriter
 *
 *   The main class for  logger SD logging
 *
 *****************************************************************************/
#ifndef __LOG_WRITER_H__
#define __LOG_WRITER_H__

#include <stdio.h>
#include <pthread.h>
#include "GlbDefine.h"
#include "Engine.h"



#include <list>
#include <string>

using namespace std;
using ::std::list;
typedef ::std::list<string> strList;
typedef ::std::list<string>::iterator strListIter;


//extern void recyleLogPath(const char *path, int folderSize);
bool setLogRecycleSize(int size);
int getLogRecycleSize();

typedef bool (*LISTENER) (int msgid, const char * msg);

namespace consyslogger {

class BaseFile {
protected:
    static const int MAX_NUMBER_FAILED = 3;
    int m_pFile; //File Descriptor
    long m_lSize; //File size
public:
    static const int PATH_LEN = 256;
    int m_nBufId; //Buffer ID
    char m_szPath[PATH_LEN]; //File path
    int m_nFailedNumber;
    BaseFile(int nBufId = 3);
    virtual ~BaseFile() {};
    long closedSize();
    long size();
    virtual bool openFile(const char *path);
    virtual bool closeFile();
    virtual bool cleanCloseFile(); // Close and check if file is empty, if true, remove it
    virtual unsigned int writeFile(const char *ptr, unsigned int len);
    virtual bool flushFile();
    virtual bool isExceeded();
    virtual bool isExist();
};

class LogFile: public BaseFile {
private:
    //declare static varible here, however, this static variable
    //should be initialzied outside of class.
    static long m_capacity;
public:
    LogFile(int nBufId = 3);
    ~LogFile() {};
    bool openFile(const char *path);
    bool cleanCloseFile();
    bool isExceeded();
    static bool setCapacity(long size);
    static long getCapacity();
};


#define FOLDER_NAME  "%s/CsLog_%s"
#define LOG_FILE_NAME  "%s/%s_FW_%s.%s"
//#define VERSION_FILE_NAME  "%s/version_info.txt"
//#define VERSION_CONTENT  "Version: %s"
#define FILE_TREE_NAME  "%s/file_tree.txt"
#define MAX_BUFFER_COUNT  12
#define SD_RESERVED_SPACE  (10 * 1048576)
//#define WRITER_BUF_SIZE  (1024 * 1024)

typedef enum {
    RET_SDREADY = 0, // 0
    ERR_GETSDSYSTEM, // 1
    ERR_SDNOTREADY, // 2
    ERR_SDFULL, // 3
    RET_STOPCHECK, // 4
    RET_TIMEOUT,
} RETURN_NUM;

typedef enum {
    LOG_ON_NONE = 0,
    LOG_ON_DATA,
    LOG_ON_SD
} LOG_LOCATION;

class LogWriter : public CommHandler {
    friend void * thrWaitingSDReady(void *arg);
    friend void * thrLoggingRecycle(void *arg);
    friend void * thrClearBootupLog(void *arg);

protected:
    unsigned long long m_nIOBlkSize;
    unsigned long m_nReservedSpace;
    char m_bootuplogFile[BaseFile::PATH_LEN];
    //Log folder to  logger
    char m_szFolderPath[BaseFile::PATH_LEN];

    char m_szCurrentLogFile[BaseFile::PATH_LEN];
     strList bootuploglist;

    //log buffer count,
    //at present, it always be 4.
    int m_nBufCnt;

    //File pointer array, the array size is m_nBufCnt.
    // Here uses m_ppNandFiles/m_ppSDFiles to keep file pointer
    // when files are created, and then it would require a lock
    // and swap m_ppNandFiles/m_ppSDFiles for m_ppFiles
    BaseFile **m_ppFiles;
    //File pointers on /data/connsyslog
    BaseFile **m_ppNandFiles;
    //File pointers on sdcard
    BaseFile **m_ppSDFiles;

    //Mutex lock to synchronize log file
    //lock any operations on m_ppFiles, including create, close and write
    pthread_mutex_t m_mutexCheckLogFile;


    //boolean flag array,
    //The array indicates which log buffer is writing log to file
    bool m_bWritingLog[MAX_BUFFER_COUNT];

  

    //Thread to monitor SD status.
    pthread_t m_thrWaitingSD;

 
    //Flag indicates log writer wants to write log on SD or /data/
    //Flag is set no matter the real file is created or not.
    LOG_LOCATION m_bLog2SD;

    //Flag indicates SD status monitor thread is running or not
    bool m_bCheckingSD;

    //to SD storage/sdcard/...
    bool m_bNeedMoveBootupLog;

    //Flag indicates whether SD card is full.
    bool mSDcardFull;

    //bool m_bDiskAvailable; Remove due to variable is never used.
    //Flag indicates whether it needs to send notification to UI to tell
    //the log file on disk is missing.
    bool m_bInformNoLogFile;

    //Flag to indicate log writer is paused or not.Use in 3 places
    //1. onReceiveData
    //2. checkAndNotifyFileExist(monitor if file is removed.
    bool m_bPaused;

    //tree file in log root folder,
    //this tree file records different log folders's info,
    //but not log file info. log file info is recorded in its
    //folder's tree file.
    char topTreeFile[MAX_PATH_LEN];
    char m_externalSDPath[MAX_PATH_LEN];

     //Listener to notify logger what happens in logwriter, so that logger
    //can execute its actions accordingly.
    LISTENER m_listener;

    //Log recycle thread,
    //It is better to do log recycle in logwriter, because log recycle only
    //exists when logging to SD card(including SD mode/PLS mode).
    pthread_t threadStartLogRecycle;
    pthread_t threadClearDataLog;

    //Flag indicate whether log recycle thread is running
    bool m_bLogRecycle;
    pthread_mutex_t mutexRecycle;
    pthread_cond_t condRecycle;
    /**
     * Clear old log file in folder BOOTUP_LOG_FOLDER
     */
    void clearOldLogFiles();

    /**
     * create log file folders and file handle on SD card.
     *
     */
    bool createLogFilesOnSD();

    /**
     * create log file folders and file handle on boot-up log folder
     *
     */
    bool createLogFilesOnNand();

    /*
     * create a single log file according log buf id
     */
    bool createLogFile(BaseFile* &pFile, int nBufId);


    bool closeLogFiles(BaseFile** &ppFiles);

    /**
     * close a single file
     */
    bool closeLogFile(BaseFile* &pFile);

    /**
     * TODO: check to remove this function.
     */
    bool flushFile(int nBufId);

    /**
     * write log file circularly
     * Parameter: pFile, BaseFile instance pointer which holds
     * the read file handle.
     * Parameter: ptr, the written buffer
     * parameter: len, the written length
     */
    bool writeLogFile(BaseFile* &pFile, const char *ptr, unsigned int len);

    /**
     * Save log content in log file.
     * It will call function "writeLogFile" to do the actual work.
     */
    bool saveLogFile(const char *data, unsigned int len,
            unsigned char bufId, LOG_LOCATION logLocation);

    /**
     * Get storage card left space
     */
    unsigned long getStorageLeftSpace(LOG_LOCATION logLocation);

    int startCheckSD(int timeout);

    void stopCheckSD();

    int checkSDStatus(int retryNumber);

    bool isWritingLog();

    bool checkReceivedStartCommands();

    bool CheckSDPlug();

    /**
     * Generate file_tree.txt file in top-level log file and common log folder.
     * In top-level log folder, file_tree.txt records log folder name, and
     * should be folder tree record;
     * in common log folder, file_tree.txt records log file name.
     *
     * Parameter: data, file/folder name that will be recorded file_tree.txt
     * Parameter: len, file/folder name length.
     * Parameter: isfoldertree, indicate data will be record in folder tree in
     *                          top-level log folder.
     * Parameter: isnewcreate, indicate open file with open flag "r" or "a".
     *                         if true, use "r"; else use "a".
     */
    bool generateLogTreeFile(const char *data, size_t len,bool isfoldertree, bool isnewcreate);

    /**
     * move bootup log /data/consyslog/bootupLog(see golbeconfig.h) to SD card
     */
    bool moveBootupLog();
    /**
     * copy files(bootup log file) from one place to another.
     * Parameter: srcFile, char *, source file path
     * Parameter: destFile, char * , target file path
     * Parameter: cancelable, bool, indicates whetcher the copy action
     *                                 can be canceled.
     * By default, copy action will be cancelled when logger is paused/stopped.
     * But if cancelable is setted false, it will ingore logger
     * paused/stopped control, and do copy until task complete or write fail.
     */
    void copyFile(char * srcFile, char * destFile,bool cancelable = true);

    /**
     * This function will use call-back function pointer "m_listener"
     */
    bool notifyMTKLogger(int msgid);

    /*
     * Start log recycle thread
     */
    void startLogRecycleThread();

    /**
     * Stop log recycle thread
     */
    void stopLogRecycleThread();

    /**
    * Start clear data logs
    */
    void startClearDataLogThread();

    /**
     * Initialize log path.
     */
    void initLogPath();

    bool startLogging();
    bool stopLogging();
	/*
	clear log
	*/

    strList mNormalFolderArray; // save all folder in fw/
    strList mTreeFolderArray;  // record all log folders that created orderly in fw/
    strList mRunningArray;     // current running folder.
    strList mCurrentFileArray; // file in current running folder

    long mClearLogSize;
    int mLogNeedToBeClearSize;
    void clearLogFolder(const char *path, int mRecycleSize);
    void clearArray();
    long checkFolderSize(const char *path);
    unsigned long calculateFolderSize(const char *path);
    long fileSize(const char *path);
    int readFolderTxtFile(char *path);
    bool checkRemoveFileOneByOne(bool mRunningFolder = false);
    int removeFileAndGetSize(const char *path);
    bool removeAllFileInFolder(const char *path, bool mNeedClearLeftLog = true);
    int clearLogs(const char *path, bool mNeedcheckSize = true);
    bool isFolderEmpty(const char * path);
    bool deleteFileAndcheckSize(string str);
    bool checkDeleteSizeIfEnough();
    int readCurrentLogFileTree(const char *path);
    void recyleLogPath(const char *path, int folderSize);
    int checkNeedRemoveFolder();
    bool removeLogByProtectFileNum(int nFileRemained);

public:
    LogWriter(LISTENER listener);
    virtual ~LogWriter();
    bool init(int nBufCnt);
    bool deinit();
    bool onReceiveLogData(const char *data, unsigned int len, unsigned char bufId);
    bool onStartLogging();
    bool onPostStartLogging();
    bool onPreStopLogging();
    bool onStopLogging();
    void onReceiveSignal();

    /**
     * Set value mTagLogRunning, mTagLogTimeOutOneHour for tag log
     * Parameter: runflag, true, tag log is running; false, tag log is stopped;
     * Parameter: timeout, time out value for tag log
     */
    void setTagLogStatus(bool runflag, int timeout);

};

class SdLogWriter : public LogWriter {
public:
    SdLogWriter(LISTENER listener) : LogWriter(listener) {}
    virtual ~SdLogWriter() {};
    bool onStartLogging();
    bool onPostStartLogging();
    bool onPreStopLogging();
    bool onStopLogging();
};


}

#endif
