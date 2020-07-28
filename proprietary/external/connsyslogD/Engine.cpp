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
 *   engine.cpp
 *
 * Description:
 * ------------
 *   Class CommEngine implementation
 *
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <time.h>

#include "Engine.h"
#include "ConsysLog.h"
#include "Utils.h"
#include "LogWriter.h"
#include "ConsysLogger.h"

namespace consyslogger {

CommEngine::CommEngine() {
    m_bTerminate = true;
    m_bPaused = true;
    m_bInited = false;
    m_pHandler = NULL;
    pthread_mutex_init(&m_mutexLogHandler, NULL);
    pthread_cond_init(&m_condPaused, NULL);
    pthread_cond_init(&m_condWriteLog, NULL);

    for (int idx = 0; idx < CONSYS_END; idx++){
        pthread_mutex_init(&(m_mutexLogRecv[idx]), NULL);
    }
    pthread_mutex_init(&m_mutexWriteLog, NULL);
    
    memset(m_LogBuf,0,sizeof(m_LogBuf));
    for (int idx = CONSYS_WIFI; idx < CONSYS_END; idx++) {
        m_nLogBufRIdx[idx] = m_nLogBufWIdx[idx] = 0;
    }
    
  /*  for (int idx = CONSYS_WIFI; idx < CONSYS_END; idx++) {
        log_buffer[idx] = (struct Log_Buffer*)malloc(sizeof(struct Log_Buffer));
        log_buffer[idx]->buf_Full = 0;
	log_buffer[idx]->nLogBufRIdx= 0;
	memset(log_buffer[idx]->buf, 0x0, sizeof(log_buffer[idx]->buf));
        log_buffer[idx]->next= NULL;
    }*/
    
    m_thrLogWriter = 0;
    m_thrLogReader = 0;
}

CommEngine::~CommEngine() {
    pthread_mutex_destroy(&m_mutexLogHandler);
    pthread_cond_destroy(&m_condPaused);
    pthread_cond_destroy(&m_condWriteLog);

    for (int idx = 0; idx < CONSYS_END; idx++){
        pthread_mutex_destroy(&(m_mutexLogRecv[idx]));
    }
    pthread_mutex_destroy(&m_mutexWriteLog);
}

void* thrLogReader(void *arg) {
    CommEngine *pEngine = static_cast<CommEngine *>(arg);

    LOGI("log reader thread started");
    int radio_index[CONSYS_END]={-1,-1,-1,-1};
    while (!pEngine->m_bTerminate) {
        ListenLogPort(radio_index);
        pEngine->readLogs(radio_index);
        pthread_cond_signal(&pEngine->m_condWriteLog);

    }

    pEngine->m_thrLogReader = 0;

    LOGI("Exit reader thread");

    return NULL;
}

#define LOG_WRITER_TIMEOUT_INTERVAL_SEC       (2)

void* thrLogWriter(void *arg) {
    CommEngine *pEngine = static_cast<CommEngine *>(arg);

    LOGI("log writer thread started");
    int retValue = -1;
    struct timespec ts;

    while (!pEngine->m_bTerminate) {
       // wait until log available
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        retValue = pthread_cond_wait(&pEngine->m_condWriteLog,
            &pEngine->m_mutexWriteLog);
       // LOGI("begin check buffer errno=%d,ret=%d",errno,retValue);

        for (int log_index = CONSYS_WIFI; log_index < CONSYS_END;log_index++) {
            if (pEngine->m_nLogBufWIdx[log_index]>= (LOG_BUFFER_SIZE-LOG_MAX_READ_UNIT)) {
                pthread_mutex_lock(&(pEngine->m_mutexLogRecv[log_index]));
                pthread_mutex_lock(&pEngine->m_mutexLogHandler);
                pEngine->m_pHandler->onReceiveLogData(pEngine->m_LogBuf[log_index],pEngine->m_nLogBufWIdx[log_index],log_index);

                //pEngine->m_nLogBufWIdx[log_index] = pEngine->m_nLogBufWIdx[log_index] & (LOG_BUFFER_SIZE-1);
                LOGI("thrLogWriter: LogBuf=%fKB,idx=%d",(float)(pEngine->m_nLogBufWIdx[log_index])/1024,log_index);
                pEngine->m_nLogBufWIdx[log_index] = 0;
                pthread_mutex_unlock(&pEngine->m_mutexLogHandler);
                pthread_mutex_unlock(&(pEngine->m_mutexLogRecv[log_index]));
               // LOGI("writer buffer full, write file, log_index = %d",log_index);

            }
        }
    }
    LOGI("begin empty buffer....");
    pthread_mutex_lock(&pEngine->m_mutexLogHandler);
    pEngine->emptyBuffer();
    pEngine->m_thrLogWriter = 0;
    pthread_mutex_unlock(&pEngine->m_mutexLogHandler);
    LOGI("Exit writer thread");

    return NULL;
}

#define COMM_MONITOR_INTERVAL_SEC       (5)

bool CommEngine::init() {
    LOGD("Init Engine: Init done");
    memset(m_LogBuf,0,sizeof(m_LogBuf));
    for (int idx = CONSYS_WIFI; idx < CONSYS_END; idx++) {
        m_nLogBufRIdx[idx] = m_nLogBufWIdx[idx] = 0;
    }
    return true;
}

bool CommEngine::deinit() {
 
    LOGD("Deinit Engine: Deinit done");
    return true;
}

bool CommEngine::start(int mode) {
    LOGV("CommEngine.start: mode=%d", mode);

    if (m_pHandler == NULL) {
        LOGE("CommEngine.start: handler not regsitered!");
        return false;
    }
    if (m_bTerminate==false) {
        LOGD("Start Engine: Engine already started.");
        return true;
    }
    bool ret = false;
    int m_nRetryACK = 8;
    if (isBootCompleted()) {
        m_nRetryACK = 1;
    }
    LOGD("Start Engine: start, logging mode %d", mode);
    m_bPaused = false;
    m_bTerminate = false;
     for (int idx = CONSYS_WIFI; idx < CONSYS_END; idx++) {
        m_nLogBufRIdx[idx] = m_nLogBufWIdx[idx] = 0;
    }

    if (m_pHandler == NULL || m_pHandler->onStartLogging() == false) {
        LOGE("CommEngine.start: handler start failed");
        return false;
    }

    while (m_nRetryACK-- > 0) {
        RETCODE result = OnLoggingStart(mode);
        if (result == OK) {
            ret = true;
            break;
        } else if (result == ERROR) {
            LOGE("CommEngine.start: interface start failed, mode=%d", mode);
            ret = false;
            sleep(2);// total 14s. MTKLogger wait 15s.
        } else if (result == NOT_PAUSED) {
            ret = true;
            break;
        } else if (result == LOG_MODE_NOT_SUPPORT) {
            LOGE("CommEngine.start: MAL start failed, mode=%d not support", mode);
            ret = false;
            break;
        } else {
            LOGE("CommEngine.start: MAL start failed, unknow MAL result(%d)", result);
            sleep(2);
        }
    }

    if (ret == false) {
        return false;
    }
    int err = PTHREAD_CREATE("cs_log_rd", &m_thrLogReader,NULL, thrLogReader, this);
    if (err != 0) {
        LOGE("start: failed to start LOG reader thread, err=%d", err);
    } else {
        if(isEngineerBuild()){
            LOGI("LOG reader thread created");
        }
    }
    err = PTHREAD_CREATE("cs_log_wt", &m_thrLogWriter, NULL,thrLogWriter, this);
    if (err != 0) {
        LOGE("CommEngine.start: failed to start LOG writer thread, err=%d", err);
    } else {
        if(isEngineerBuild()){
            LOGI("LOG writer thread created");
        }
    }


    if (ret == false) {
        LOGE("CommEngine.start: start failed, revert operation!");
        bool rt = stop();
        LOGE("CommEngine.start: start failed, revert operation %d", rt);
    }
    m_pHandler->onPostStartLogging();
    LOGD("Start Engine: done. mode=%d", mode);
    return ret;
}

bool CommEngine::stop() {
    if (m_bTerminate) {
        LOGW("Stop Engine:Engine already stopped.");
        return true;
    }
    if (m_bPaused == true) {
        LOGW("Engine stop logging: already paused.");
        return true;
    }
    m_bPaused = true;
    if(isEngineerBuild()){
    LOGD("Stop Engine: stop");
    }
 
    if (m_pHandler != NULL) {
        m_pHandler->onPreStopLogging();
    }

    OnLoggingStop();
    m_bTerminate = true;

    SignalThreadExit();
    setRunningStatus(false);


    if (0 != m_thrLogReader) {
        LOGI("notify reader thread %lx,terminated",m_thrLogReader);
    }
    
    if (isPthreadAlive(m_thrLogReader)) {
         int retry = 100;
         do {
              usleep(3000);
              SignalThreadExit();
            } while (--retry > 0 && isPthreadAlive(m_thrLogReader));
         LOGD("Stop Engine: wait for m_thrLogReader thread quit. retry:%d", retry);
    }
    m_thrLogReader = 0;

    if (0 != m_thrLogWriter) {
            LOGI("notify LOG writer thread %lx terminated",m_thrLogWriter);
    }

    if (isPthreadAlive(m_thrLogWriter)) {
        int retry = 100;
        do {
            usleep(12000);
            pthread_cond_signal(&m_condWriteLog);
           } while (--retry > 0 && isPthreadAlive(m_thrLogWriter));
        LOGD("Stop Engine: wait for m_thrLogWriter thread quit. retry:%d", retry);
        // if logwriter thread be blocked or not exit in time,
        //should empty buffer here
        if (retry == 0) {
            LOGE("LogWriter thread not exit in time");
            emptyBuffer();
        }
    }
    m_thrLogWriter = 0;

    //stop handler
    if (m_pHandler != NULL) {
        m_pHandler->onStopLogging();
    }

    LOGD("Stop Engine: done.");
    return true;
}

void CommEngine::registerHandler(CommHandler *pHandler) {
    pthread_mutex_lock(&m_mutexLogHandler);
    m_pHandler = pHandler;
    pthread_mutex_unlock(&m_mutexLogHandler);
}

void CommEngine::unregisterHandler() {
    pthread_mutex_lock(&m_mutexLogHandler);
    m_pHandler = NULL;
    pthread_mutex_unlock(&m_mutexLogHandler);
}

void CommEngine::setRunningStatus(bool bRunning) {
    if (bRunning == true) {
        property_set(PROP_CONSYSLOGGER_RUNNING_STATUS, "1");
    } else {
        property_set(PROP_CONSYSLOGGER_RUNNING_STATUS, "0");
    }
    LOGD("setproperty %s: %d.", PROP_CONSYSLOGGER_RUNNING_STATUS, bRunning);
}

void CommEngine::emptyBuffer() {
    for (int log_index = CONSYS_WIFI; log_index < CONSYS_END;log_index++) {
        if(m_nLogBufWIdx[log_index] > 0) {
            if (m_pHandler != NULL) {
                m_pHandler->onReceiveLogData(m_LogBuf[log_index],m_nLogBufWIdx[log_index],log_index);
                LOGD("stop logging:empty buffer %d", log_index);
                m_nLogBufWIdx[log_index] = 0;
            } else {
                LOGE("emptyBuffer: m_pHandler is NULL");
                return;
            }
        }
    }
}

int CommEngine::getBufCnt() {
    return CONSYS_END;
}

bool CommEngine::isPaused() {
    return m_bPaused;
}

void CommEngine:: readLogs(int *radio_index) {
    for (int log_index = 0;log_index<CONSYS_END;log_index++) {
        if (radio_index[log_index] != -1) {
           // LOGD("readLogs buffer %d", log_index);
            pthread_mutex_lock(&(m_mutexLogRecv[log_index]));
            int read_size = ReadFromLogPort(
            m_LogBuf[log_index] + m_nLogBufWIdx[log_index],
            LOG_MAX_READ_UNIT, (0 ? 0 : -1),log_index);
     
            if (read_size == -1) {
                LOGE("reader: read error log_indx=%d",log_index);
                usleep(200000);
                pthread_mutex_unlock(&(m_mutexLogRecv[log_index]));
                radio_index[log_index]= -1;
                continue;
            } else if (read_size == 0) {
                LOGV("reader: read %d bytes", read_size);
                pthread_mutex_unlock(&(m_mutexLogRecv[log_index]));
                radio_index[log_index]= -1;
                continue;
            }
         m_nLogBufWIdx[log_index] += read_size;
         pthread_mutex_unlock(&(m_mutexLogRecv[log_index]));
         radio_index[log_index]= -1;
     }
}
}

}

