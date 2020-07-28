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
 *   consyslogger.c
 *
 * Description:
 * ------------
 *   logger main function
 *
 *******************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <termios.h>
#include <signal.h>
#include <selinux/android.h>
#include "PropParser.h"
#include "ConsysLog.h"
#include "Utils.h"
#include "ConsysLogger.h"
#include "SocketConnection.h"

#define UNUSED(x) (void)(x)

typedef struct {
    int client_pid;
    OP op;
} REQ_H;

typedef union {
    int logging_mode;
    bool result;
} REQ_BODY;

typedef struct {
    REQ_H header;
    REQ_BODY body;
} REQ;

typedef struct {
    OP op;
} RSP_H;

typedef union {
    int logging_mode;
    bool result;
} RSP_BODY;

typedef struct {
    RSP_H header;
    RSP_BODY body;
} RSP;

//static bool g_pLogWriterRegister = false;
static bool g_bInited = false;

socketconnection *pConnection = NULL;
BOOT_MODE BootMode = NORMAL_BOOT_MODE;
bool mReceiveStartCMD = false;

int debugLevel = -1;
bool mIsEngLoad = true;

CommEngine *g_pEngine = NULL;
LogWriter *g_pLogWriter = NULL;


static bool g_bStopLogger = false;
static bool g_bLStarting = false;
static bool g_bLStoping = false;


//Function declaration
static bool startLogging(LOGGING_MODE mode);
static bool stopLogging();
static bool startSDLogging(LOGGING_MODE mode);
static bool stopSDLogging();


/**
 * Test Function, it will read fifo and do test accordingly.
 */
void handleRequestForTest(void *req) {
    char * request = (char*)req;
    const char * tmp;
    bool ret = false;
    if (!strncmp(request, "g_sz", strlen("g_sz"))) {
        ret = executeCommand(OP_GET_RECYCLE_SIZE);
        tmp = "OP_GET_RECYCLE_SIZE";
    } else if (!strncmp(request, "am", strlen("am"))) {
        ret = executeCommand(OP_SET_AUTOSTART_LOGGING_MODE,
                             (LOGGING_MODE)(request[strlen("am,")] - '0'));
        tmp = "P_SET_AUTOSTART_LOGGING_MODE";
    } else if (!strncmp(request, "sz", strlen("sz"))) {
        char temp[256];
        memset(temp, '\0', 256);
        strncpy(temp, &request[strlen("sz,")], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        ret = executeCommand(OP_SET_RECYCLE_SIZE, atoi(temp));
        tmp = "OP_SET_RECYCLE_SIZE";
    } else if (!strncmp(request, "dp", strlen("dp"))) {
        ret = executeCommand(OP_DEEP_STOP_LOGGING);
        tmp = "OP_DEEP_STOP_LOGGING";
    } else if (!strncmp(request, "ds", strlen("ds"))) {
        ret = executeCommand(OP_DEEP_START_LOGGING,
                             (LOGGING_MODE)(request[strlen("ds,")] - '0'));
        tmp = "OP_DEEP_START_LOGGING";
       
    } else if (!strncmp(request, "s", strlen("s"))) {
        ret = executeCommand(OP_START_LOGGING,
                             (LOGGING_MODE)(request[strlen("s,")] - '0'));
        tmp = "OP_START_LOGGING";
        if (ret == false) {
            LOGE("OP_START_LOGGING, errno:%d, %s", errno, strerror(errno));
        }
        
    } else {
        LOGE("unknown socket cmds %s", request);
        tmp = "Unknown";
    }
    LOGD("***********************************");
    LOGD("Execute Request: %s", request);
    LOGD("Execute Command: %s", tmp);
    LOGD("Execute Result: %d", ret);
    LOGD("***********************************");
    char response[128];
    sprintf(response, "handleRequestForTest:%s. result:%d", tmp, ret);
    int client_fd = -1;
    int retry = 10;
    while (1) {
        // Try to open client FIFO
        client_fd = open(CLIENT_FIFO, O_WRONLY | O_NONBLOCK);
        if (client_fd != -1) {
            LOGD("Handle request: Open client FIFO %d", client_fd);
            // Send response to client FIFO
            int bytes = write(client_fd, &response, sizeof(response));
            if (bytes != -1) {
                //LOGD("Wrote %d bytes to client FIFO, max %d", bytes, sizeof(RSP));
                LOGD("Handle request: response client %s", response);
            } else {
                LOGE("Handle request: Failed to write client FIFO!");
                ret = false;
            }
            close(client_fd);
            break;
        } else if (--retry > 0) {
            LOGD("Handle request: Retry %d to open client FIFO", retry);
            usleep(200000);
            continue;
        } else {
            LOGE("Handle request: Failed to open client FIFO errno=%d", errno);
            break;
        }
    }
}

/**
 * we can find some situations in non-main thread(e.g. log write thread)
 * that will case logger hang:
 *  MSG_SDCARD_NO_EXIST, no sd card;
 *  MSG_SDCARD_IS_FULL, sd card is full;
 *  MSG_SDCARD_NO_LOG_FILE, logging file is missing;
 *
 * In these case, the function call should return immediately, and it needs
 * to notify main thread to pause log from non-main thread.
 */

bool signalStopLogging() {
    REQ_H head = {(int)pthread_self(), OP_DEEP_STOP_LOGGING};
    REQ_BODY body = {MODE_IDLE};
    REQ req = {head, body};
    int mainFifoFd = open(SERVER_FIFO, O_WRONLY | O_NONBLOCK);
    bool ret = false;
    if (mainFifoFd != -1) {
        LOGD("signal send request: open main FIFO %d", mainFifoFd);
        // Send request to FIFO in main thread
        int bytes = write(mainFifoFd, &req, sizeof(REQ));
        if (bytes != -1) {
            ret = true;
            LOGD("signal send request: response client_pid=%d, op=%d",
                    req.header.client_pid, req.header.op);
        } else {
            LOGE("signal send request: failed to write main FIFO! errno:%d, %s",
                    errno, strerror(errno));
            ret = false;
        }
        close(mainFifoFd);
    } else {
        LOGE("signal Handle request: Failed to open client FIFO errno:%d, %s",
                errno, strerror(errno));
       LOGE("Exit Logger with error.can not find pipe");
       exit(1);
    }
    return ret;
}

/**
 * The function will notify UI what happens in logger,
 * then it should return immediately,
 * The other function like PausingLogger should be done in asynchronous thread,
 * in order to not block the other thread running.
 */
bool callbackLogHandlerListener(int controlcode, const char * data) {
    UNUSED(data);
    bool ret = false;
    LOGV("callbackLogHandlerListener receive %d", controlcode);
    switch(controlcode) {
    //FAIL_WRITE_FILE
    case MSG_FAIL_WRITE_FILE: {
        LOGD("Receive Notify: MSG_SDCARD_NO_EXIST");
        if ((g_pEngine != NULL) && (!g_pEngine->isPaused())) {
        ret = sendCommandToAPK(MSG_FAIL_WRITE_FILE);
        signalStopLogging();
        }else {
        LOGD("logging is paused or pausing ,no need to notify UI");
        }
        break;
    }
    //SDCARD_NOTEXIST
    case MSG_SDCARD_NO_EXIST: {
        LOGD("Receive Notify: MSG_SDCARD_NO_EXIST");
        if ((g_pEngine != NULL) && (!g_pEngine->isPaused())) {
        ret = sendCommandToAPK(MSG_SDCARD_NO_EXIST);
        signalStopLogging();
        }else {
        LOGD("logging is paused or pausing ,no need to notify UI");
        }
        break;
    }
    //SDCARD_FULL
    case MSG_SDCARD_IS_FULL: {
        LOGD("Receive Notify: MSG_SDCARD_IS_FULL");
        if ((g_pEngine != NULL) && (!g_pEngine->isPaused())) {
        ret = sendCommandToAPK(MSG_SDCARD_IS_FULL);
        signalStopLogging();
        }else {
        LOGD("logging is paused or pausing ,no need to notify UI");
        }
        break;
    }
    //LOGFILE_NOTEXIST
    case MSG_SDCARD_NO_LOG_FILE: {
        LOGD("Receive Notify: MSG_SDCARD_NO_LOG_FILE");
        if ((g_pEngine != NULL) && (!g_pEngine->isPaused())) {
        ret = sendCommandToAPK(MSG_SDCARD_NO_LOG_FILE);
        signalStopLogging();
        }else {
        LOGD("logging is paused or pausing ,no need to notify UI");
        }
        break;
    }
     default:
        break;
    }
    LOGI("callbackLogHandlerListener return %d", ret);
    return ret;
}
extern bool pathFromMTKLogger;

bool getReceiveStartCommand() {
   // return true;
    return (mReceiveStartCMD && pathFromMTKLogger);
}
/*
for BTLog debug level
*/
int getDebugLevel() {
    return debugLevel;
}

int setDebugLevel(int level) {
    debugLevel = level;
    return 1;
}

bool isSetRunningProperty() {
    return checkSysPropEq(PROP_CONSYSLOGGER_RUNNING_STATUS, "1", "0");
}


bool initEngine(void) {
    if (!g_bInited) {
        g_pEngine = new CommEngine();
        if (!g_pEngine->init()) {
            LOGE("Init Engine: Failed to init Engine");
            delete g_pEngine;
            g_pEngine = NULL;
            return false;
        }
        g_bInited = true;
        LOGD("Init Engine: OK");
        return true;
    } else {
        LOGW("Init Engine: Engine is already initialized before.");
        return true;
    }
}

bool deinitEngine(void) {
    if (g_bInited) {
        if (!g_pEngine) {
            LOGD("Deinit Engine: not initialzed");
            return true;
        }
        g_pEngine->deinit();
        delete g_pEngine;
        g_pEngine = NULL;
        g_bInited = false;
        LOGI("Deinit Engine: OK");
        return true;
    } else {
        LOGW("Deinit Engine: Engine is not initialized before.");
        return true;
    }
}

bool initLogWritter(void) {
    if (g_pLogWriter == NULL) {
        g_pLogWriter = new SdLogWriter(callbackLogHandlerListener);
        if (g_pLogWriter->init(g_pEngine->getBufCnt())) {
            LOGD("Init LogWritter: OK");
        } else {
            delete g_pLogWriter;
            g_pLogWriter = NULL;
            LOGE("Init LogWritter: Failed to init LogWriter.");
            return false;
        }
    }
    return true;
}

bool deinitLogWriter() {
    if (!g_pLogWriter) {
        LOGD("Deinit LogWriter: not initialzed");
        return true;
    }
    bool ret = true;
    if (g_pLogWriter->deinit()) {
        LOGD("Deinit LogWriter: OK");
    } else {
        LOGW("Deinit LogWriter: Failed to deinit LogWriter");
        ret = false;
    }
    delete g_pLogWriter;
    g_pLogWriter = NULL;
    return ret;
}

bool initSDMode(void) {
    if (!initEngine()) {
        return false;
    }
    if (!initLogWritter()) {
        return false;
    }
    g_pEngine->registerHandler(g_pLogWriter);
    LOGD("OKOKOK >Init SD logging mode< OKOKOK");
    return true;
}

bool deinitSDMode(void) {
    LOGI("------>Deinit SD logging mode<------");
    if (g_pEngine != NULL) {
        g_pEngine->unregisterHandler();
    }
    if (!deinitLogWriter()) {
        return false;
    }
    if (!deinitEngine()) {
        return false;
    }
    LOGD("OKOKOK >Deinit SD logging mode< OKOKOK");
    return true;
}


/**
 * Start SD Logging Process,
 * startSDLogging
 *     |-> initSDMode
 *         |-> initEngine, create g_pEngine
 *         |-> initLogWritter, create and init g_pLogWriter
 *         |-> g_pEngine->registerHandler(g_pLogWriter)
 *     |-> g_pEngine->Start
 *     |-> setCurrentLoggingMode(MODE_SD)
 */
bool startSDLogging(LOGGING_MODE mode) {
    bool ret = false;
    do {
        if (!initSDMode()) {
            break;
        }
        if (g_pEngine != NULL && !g_pEngine->start((int)mode)) {
            LOGE("Start SD Logging: Failed to start Engine in SD Mode!");
            break;
        }
        ret = true;
        LOGI("Success to start logging in SD Mode!");
    } while(0);
    if (false == ret) {
        LOGE("Fail to start logging in SD Mode! Logger will be stopped.");
        stopSDLogging();
    }
    return ret;
}

/**
 * Stop SD Logging Process,
 * stopSDLogging
 *     |-> g_pEngine->Stop
 *     |-> deinitSDMode
 *         |-> g_pEngine->unregisterHandler(g_pLogWriter)
 *         |-> deinitLogWritter, delete g_pLogWriter
 *         |-> deinitEngine, delete g_pEngine
 *     |-> setCurrentLoggingMode(MODE_IDLE)
 */
bool stopSDLogging() {
  //  int mode = getCurrentLoggingMode();

    if (g_pEngine != NULL && !g_pEngine->stop()) {
        LOGE("Stop SD Logging: Failed to stop Engine");
    } else {
        LOGD("Stop SD Logging: Engine stop OK");
    }

    if (!deinitSDMode()) {
        LOGE("Stop SD Logging: Failed to stop SD logging");
        return false;
    }
    return true;
}

bool startLogging(LOGGING_MODE mode) {
    LOGI("START Logging, mode=%d", mode);
    bool ret = true;
   if (g_bLStarting == true) {
        LOGI("startLogging: already trying to start");
        return true;
    }


    LOGV("g_bLStarting to TRUE");
    g_bLStarting = true;
    ret = startSDLogging(mode);
    LOGV("g_bLStarting to FALSE");
    g_bLStarting = false;
    return ret;
}

bool stopLogging() {
    LOGI("STOP Logging");
    if (g_bLStoping == true) {
        LOGI("stopLogging: already in stopping");
        return true;
    }
    g_bLStoping = true;
    bool ret = stopSDLogging();
    g_bLStoping = false;
    return ret;
}

bool isEngineerBuild() {
    return mIsEngLoad;
}

bool getStartFromCust() {
    PropParser *mPropParseHanlder = new PropParser();
    if (mPropParseHanlder == NULL) {
        LOGE("Prop handle is null");
        return false;
    }
    mPropParseHanlder->getPropFileConfig();

    bool mRetValue = mPropParseHanlder->getAutoStartConfigure();
    if (access(CONSYSLOGGER_RECYCLE_SIZE_FILE, F_OK) != 0) {
        int recycleSize = mPropParseHanlder->getRecycleSizeFromPropFile();
        setLogRecycleSize(recycleSize);
    }

    delete mPropParseHanlder;
    return mRetValue;
}

bool isLogPropFileExist() {
    if (access(LOGGER_CUST_FILE, F_OK) == 0) {
        if (isEngineerBuild()) {
            LOGI("system Prop file exist.");
        }
        return true;
    }
    LOGI("Customize Prop file doesn't exist.");

    return false;
}



LOGGING_MODE initialConfigIfNotExist() {
    
    if (access(CONSYSLOGGER_CONFIG_FILE, F_OK) == 0) {        
        return MODE_UNKNOWN;
    }
    // Configuration file not exist, create it
    int fd = creat(CONSYSLOGGER_CONFIG_FILE, 0664);
   // chown(CONSYSLOGGER_CONFIG_FILE, 2000, 1000);
    LOGGING_MODE defaultVal = MODE_IDLE;
    if (isLogPropFileExist()) {
        if (getStartFromCust()) {
            LOGI("Init config file: It's engineer build, set default auto start mode to SD mode");
            defaultVal = MODE_START;
        }
    } else {
        if (isEngineerBuild()) {
            LOGI("Init config file not exist: It's engineer build, set default auto start mode to SD mode");
            defaultVal = MODE_START;
        }
    }
    if (fd != -1) {
        LOGD("Init config file: Create Logger config file %s", CONSYSLOGGER_CONFIG_FILE);
        // Initialize the content
        if (write(fd, &(defaultVal), sizeof(LOGGING_MODE))
                == sizeof(LOGGING_MODE)) {
            LOGD("Init config file: Default mode is set to %d", defaultVal);
        }
        close(fd);
    } else {
        LOGE("Init config file: Failed to create Logger config file %s, errno=%d", CONSYSLOGGER_CONFIG_FILE, errno);
    }

    return defaultVal;
}

int readSys_int(char const * path) {
    int fd;

    if (path == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        char buffer[20];
        int amt = read(fd, buffer, sizeof(int));
        close(fd);
        return amt == -1 ? -errno : atoi(buffer);
    }
    LOGE("Fail to open facotry mode  %s\n", path);
    return -errno;
}
/*
factory mode
ATE factory mode
meta mode
nornal mode
*/
void getBootMode() {
    int boot_Mode = readSys_int(BOOTMODE_PATH);
    if (FACTORY_BOOT == boot_Mode) {
        LOGD("Factory mode boot!");
        BootMode = FACTORY_BOOT_MODE;
    } else if (ATE_FACTORY_BOOT == boot_Mode) {
        LOGD("ATE Factory mode boot!");
        BootMode = ATE_FACTORY_BOOT_MODE;
    }else if (META_BOOT == boot_Mode) {
        BootMode = META_BOOT_MODE;
    } else {
        BootMode = NORMAL_BOOT_MODE;
    }
}


LOGGING_MODE getSavedLoggingMode() {
    LOGGING_MODE ret = initialConfigIfNotExist();
    if (ret <= MODE_UNKNOWN || ret >= MODE_END) {
        int fd = open(CONSYSLOGGER_CONFIG_FILE, O_RDONLY);
        if (fd != -1) {
            // Read the logging mode set for next time
            int mode = 0;
            ssize_t retvalue = read(fd, &(mode), sizeof(int));
            if (-1 == retvalue) {
                LOGE("Get auto start mode: Failed to read logging mode from config file, errno=%d", errno);
            } else if (sizeof(int) == (size_t) retvalue) {
                ret = (LOGGING_MODE)mode;
            } else {
                LOGE("Get auto start mode: Failed to read  mode from config file, returned %ld", (long)retvalue);
            }
            close(fd);
        } else {
            LOGE("Get auto start mode: Failed to open Logger config file %s, errno=%d", CONSYSLOGGER_CONFIG_FILE, errno);
        }
    }
    LOGD("Get auto start mode: %d", ret);


    return ret;
}

int setSavedLoggingMode(LOGGING_MODE mode) {
    LOGGING_MODE ret = MODE_UNKNOWN;
    initialConfigIfNotExist();
    int fd = open(CONSYSLOGGER_CONFIG_FILE, O_RDWR | O_SYNC);
    if (fd != -1) {
        // Set the logging mode on next time booting up
        ssize_t retvalue = write(fd, &mode, sizeof(LOGGING_MODE));
        if (-1 == retvalue) {
            LOGE("Set auto start mode: Failed to write logging mode %d to config file, errno=%d", mode, errno);
        } else if (sizeof(LOGGING_MODE) == (size_t)retvalue) {
            ret = mode;
        } else {
            LOGE("Set auto start mode: Failed to write  mode %d to config file, returned %ld", mode, (long)retvalue);
        }
        close(fd);
    } else {
        LOGE("Set auto start mode: Failed to open Logger config file %s, errno=%d", CONSYSLOGGER_CONFIG_FILE, errno);
    }
    LOGD("Set auto start mode: %d", mode);
    return ret;
}

bool handleReq(REQ *pReq) {
    RSP rsp;
    int cliFifoFd = -1;
    int bytes = 0;;

    int retry = 0;
    int retryMax = 10;

    memset(&rsp, '\0', sizeof(RSP));

    if (pReq == NULL) {
        LOGE("Handle the request from : REQ pointer is NULL.");
        return false;
    }
    rsp.header.op = pReq->header.op;
    LOGD("Handle request: client_pid=%d, op=%d", pReq->header.client_pid, pReq->header.op);
    // Handle request
    bool ret = true;
    switch (pReq->header.op) {
    case OP_DEEP_STOP_LOGGING:
        LOGD("Handle request: Stop logging");
        rsp.body.result = stopLogging();
        break;
    default:
        LOGE("Handle request: Unknown OP!!!");
        handleRequestForTest(pReq);
        return false;
    }
    while (1) {
        // Try to open client FIFO
        cliFifoFd = open(CLIENT_FIFO, O_WRONLY | O_NONBLOCK);
        if (cliFifoFd != -1) {
            LOGD("Handle request: Open client FIFO %d", cliFifoFd);
            // Send response to client FIFO
            bytes = write(cliFifoFd, &rsp, sizeof(RSP));
            if (bytes != -1) {
                //LOGD("Wrote %d bytes to client FIFO, max %d", bytes, sizeof(RSP));
                LOGD("Handle request: response client_pid=%d, op=%d", pReq->header.client_pid, rsp.header.op);
            } else {
                LOGE("Handle request: Failed to write client FIFO!");
                ret = false;
            }
            close(cliFifoFd);
            break;
        } else if (retry++ < retryMax) {
            LOGD("Handle request: Retry %d to open client FIFO", retry);
            usleep(200000);
            continue;
        } else {
            LOGE("Handle request: Failed to open client FIFO errno=%d", errno);
            ret = false;
            break;
        }
    }
    return ret;
}

bool sendCommandToAPK(const int msgid, const char *msgdata) {
    bool ret = false;
    if (pConnection != NULL) {
        ret = pConnection->sendCommandToClient(msgid, msgdata);
    }
    return ret;
}

int executeCommand(int cmd, int parameter) {
    int ret = 1;
    switch (cmd) {
    case OP_SET_AUTOSTART_LOGGING_MODE:
        LOGD("Handle request: Set auto start logging mode");
        ret= setSavedLoggingMode((LOGGING_MODE)parameter);
        break;

    case OP_DEEP_START_LOGGING: {
        LOGD("Handle request: deep start and set auto start.");
        LOGGING_MODE mode = MODE_START;
        mReceiveStartCMD = true;
        setSavedLoggingMode(mode);
        ret = startLogging(mode);

        break;
    }
    case OP_DEEP_STOP_LOGGING:
        LOGD("executeCommand Handle request: deep Pause logging");
        setSavedLoggingMode(MODE_IDLE);
        ret = stopLogging();
        break;

    case OP_SET_RECYCLE_SIZE:
        LOGD("executeCommand Handle request: Set recycle size = %d", parameter);
        ret= setLogRecycleSize(parameter);
        break;

    case OP_SET_DEBUG_LEVEL:
        LOGD("executeCommand Handle request: Set debug level= %d", parameter);
        ret = setDebugLevel(parameter);
        break;

    case OP_SET_LOGFILE_CAPACITY:
        LOGD("executeCommand Handle request: Set log file capacity %d", parameter);
        ret= LogFile::setCapacity(parameter);
        break;
    case OP_META_START_LOGGING:{
        LOGGING_MODE mode = MODE_START;
        LOGD("executeCommand meta logging");
        mReceiveStartCMD = true;
        if (checkConnectType() == ATM_WIFI_MODE) {
            createSocket();
        }
        ret = startLogging(mode);
        break;
    }

    case OP_META_STOP_LOGGING:
        LOGD("executeCommand: meta Pause logging");
        ret = stopLogging();
        break;
    case OP_PULL_LOG_START: {
       property_set("vendor.pullFWlog","");
       char logPath[128];
       memset(logPath,0,128);
       if((g_pEngine != NULL) && (!g_pEngine->isPaused())) {
           stopLogging();
       }
       property_get(PROP_LOGGER_RUNNING_FOLDER, logPath, "");
       if (checkConnectType() == ATM_WIFI_MODE) {
           createSocket();
           sendSourceFile(logPath,ATM_WIFI_MODE);
       }else {
           sendSourceFile(logPath,ATM_USB_MODE);
       }
       break;
    }
    case OP_PULL_LOG_STOP:
       property_set("vendor.pullFWlog","");
       break;
    default:
        break;
    }
    return ret;
}

void sighand(int value) {
    if (value != SIGUSR1) {
        LOGE("Logger: exit by signal %d ", value);
    }
    if (value == SIGTERM || value == SIGINT) {
        LOGE("Logger: receive signal %d ,the killer mayber vold", value);
        stopLogging();
    }
}

int main(void) {
    mode_t pre_mask = umask(000);
    if (isEngineerBuild()) {
    LOGI("previous mask value is %03o",pre_mask);
    LOGI("Starting consyslogger. Version=W1622.p1");
    }
    int servFifoFd = -1;
    int dumpServerFD = -1;
    int dumpClientRead = -1;
    REQ req;
    int bytes = 0;
    // setup signal, signal will be used to notify threads for termination
    struct sigaction actions;
    actions.sa_flags = 0;
    sigemptyset(&actions.sa_mask);
    actions.sa_handler = sighand;
    sigaction(SIGUSR1, &actions, NULL);
    LOGV("signal ignore");
    signal(SIGPIPE,SIG_IGN);
    sigaction(SIGINT,&actions,NULL);
    sigaction(SIGTERM, &actions, NULL);
    mIsEngLoad = checkSysPropEq(PROP_BUILD_TYPE, "eng", "eng");
    getBootMode();
    memset(&req, '\0', sizeof(REQ));
    // create data partition folder
    if (makeDirWithPermission(CONSYSLOGGER_DATA_PATH, 0775) != false) {
        if (selinux_android_restorecon(CONSYSLOGGER_DATA_PATH, 0) == -1) {
            LOGE("restorecon failed for %s: %s\n", CONSYSLOGGER_DATA_PATH, strerror(errno));
        }
        int result = mkdir(CONSYSLOGGER_BOOTUP_FOLDER, 0755);
        if (result != 0) {
            LOGE("mkdir fail:directory [%s], errno=%d", CONSYSLOGGER_BOOTUP_FOLDER, errno);
        }
    } else {
        LOGE("fail to create log folder %s: %s\n", CONSYSLOGGER_DATA_PATH, strerror(errno));
    }
    property_set(PROP_CONSYSLOGGER_RUNNING_STATUS, "0");

    if (isBootCompleted()) {
        mReceiveStartCMD = true;
    }
    LOGGING_MODE mode = getSavedLoggingMode();
    LOGD("getSavedLoggingMode(): mode = %d",mode);
    property_set(PROP_LOGGER_RUNNING_FOLDER,"");
    property_set("vendor.pullFWlog","");
    if (BootMode == META_BOOT_MODE) {
        mode = MODE_IDLE;
        char meta_logging[128];
        memset(meta_logging,0,128);

        property_get(PROP_META_BOOT_FLAG,meta_logging,"none");
        LOGD("ro.boot.meta_log_disable = %s",meta_logging);
        if (strncmp(meta_logging,"0",strlen("0")) == 0)
        {
            LOGD("meta mode starting value is on,start it");
            mode = MODE_START;
        }
    }
    if (!checkSysPropEq(PROP_FEATURE_OPTION, "1", "")) {
        mode = MODE_IDLE;
        LOGD("CONNSYSLOG_FEATURE IS OFF");
    } else {
        pConnection = new socketconnection();
        pConnection->shouldtimeout = true;
        pConnection->startListening();
        LOGD("start logger : %d", mode);
    }
    if (mode > MODE_IDLE && mode < MODE_END) {
        if (Init() == false) {
            LOGE("Main: failed to init");
            goto errorout;
        }
        if (startLogging(mode) == true) {
            LOGD("startLogging success");
        }
    }

    // Create server FIFO
    LOGD("boot mode:%d", BootMode);
 //   if (BootMode == NORMAL_BOOT_MODE) {
        if (access(SERVER_FIFO, F_OK) == -1) {
            if (mkfifo(SERVER_FIFO, 0664) == 0) {
                chown(SERVER_FIFO, 2000, 1015);
                //  setgid
                LOGD("Main: Create server FIFO %s", SERVER_FIFO);
            } else {
                LOGE("Main: Failed to create server FIFO %s, errno=%d", SERVER_FIFO, errno);
                goto errorout;
            }
        }

        if (access(CLIENT_FIFO, F_OK) == -1) {
            if (mkfifo(CLIENT_FIFO, 0664) == 0) {
                chown(CLIENT_FIFO, 2000, 1015);
                LOGD("Handle request: Create client FIFO ok");
            } else {
                LOGE("Handle request: Failed to create client errno=%d", errno);
                goto errorout;
            }
        }
        LOGD("Open servFifoFd FIFO %d", servFifoFd);
        servFifoFd = open(SERVER_FIFO, O_RDONLY);
        LOGD("Open Dump server FIFO %d", dumpServerFD);
        dumpServerFD = open(SERVER_FIFO, O_WRONLY);
        LOGD("Open Dump Client RD FIFO %d", dumpClientRead);
        dumpClientRead = open(CLIENT_FIFO, O_RDONLY | O_NONBLOCK);
 //   }

    while (!g_bStopLogger) {
        LOGV("g_bStopLogger Normal boot %d", BootMode);
        if (servFifoFd == -1) {
            servFifoFd = open(SERVER_FIFO, O_RDONLY /*| O_NONBLOCK*/);
            LOGD("Open FIFO for close");
            sleep(5);
            continue;
        }

        bytes = read(servFifoFd, &req, sizeof(REQ));
        if (bytes > 0) {
            LOGD("Main: Read %d bytes from server FIFO", bytes);
            handleReq(&req);
        } else {
            LOGD("Main: colse servFifoFd :bytes=%d,errno=%d",bytes,errno);
            close(servFifoFd);
            servFifoFd = -1;
        }
    }
    if (servFifoFd != -1) {
        close(servFifoFd);
        servFifoFd = -1;
    }
    if (dumpServerFD != -1) {
        close(dumpServerFD);
        dumpServerFD = -1;
    }
    if (dumpClientRead != -1) {
        close(dumpClientRead);
        dumpClientRead = -1;
    }
    if (pConnection != NULL) {
        pConnection->stop();
        delete pConnection;
        pConnection = NULL;
    }

    Deinit();

    LOGD("Main: Exit Logger normally");
    exit(0);

errorout:
    LOGE("Main: Exit Logger with error.");
    exit(1);
}
