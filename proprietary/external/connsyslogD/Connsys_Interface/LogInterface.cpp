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
 *   interface.cpp
 *
 * Description:
 * ------------
 *   
 *
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/poll.h>
#include <dirent.h>

#include "../ConsysLog.h"
//#include "../Utils.h"
#include "../Interface.h"
#include "../ConsysLogger.h"
#include "IoCtrl_Define.h"
#include "../GlbDefine.h"



#define UNUSED(x) (void)(x)

//char CONSYSLOGGER_LOG_PORT[64] = {0};
//char CONSYSLOGGER_CTRL_PORT[64] = {0};

static bool log_started = false;

static int consyslog_port[CONSYS_END]={-1,-1,-1,-1};
static char consyslog_node[CONSYS_END][24]= {WIFI_FW_LOG_PORT,
                                             BT_FW_LOG_PORT,
                                             GPS_FW_LOG_PORT,
                                             MCU_FW_LOG_PORT};

static int maxfd = 0;
static fd_set read_fds;
static int pfd[2] = {-1,-1};

bool open_logport()
{
    // wait WMT driver ready, then open device
    char wmt_ready[PROPERTY_VALUE_MAX];
    int retry_count = 0;
    do {
        memset(wmt_ready,0,PROPERTY_VALUE_MAX);
        int ret = property_get(PROP_WMT_DRIVER_READY, wmt_ready, "");
        if (0 >= ret) {
            LOGE("open_logport: get property:vendor.connsys.driver.ready failed ret:%d", ret);
        } else {
            if (!strncmp(wmt_ready, "yes",strlen("yes"))) {
                LOGI("open_logport: get property:vendor.connsys.driver.ready = %s", wmt_ready);
                break;
            }
        }
        usleep(300000);
    } while (1);

    for (int port_idx = 0; port_idx <CONSYS_END; port_idx++) {
        if (consyslog_port[port_idx] != -1) {
            LOGI("open_logport: port already open! idx=%d",port_idx);
            continue;
        }
        consyslog_port[port_idx] = open(consyslog_node[port_idx], O_RDWR | O_NONBLOCK);
        /* Polling 10 times and wait 500ms each run. */
        retry_count = 0;
        while (retry_count < 10 && consyslog_port[port_idx] == -1) {
            usleep(500*1000);
            consyslog_port[port_idx] = open(consyslog_node[port_idx], O_RDWR | O_NONBLOCK);
            retry_count ++;
            LOGI("open_logport: Open log port (node = %s) failed (errno = %d), times = %d",
                consyslog_node[port_idx], errno, retry_count);
        }
        if (consyslog_port[port_idx] == -1) {
            LOGE("open_logport: open fail, idx = %d,node = %s err=%d.",port_idx,consyslog_node[port_idx],errno);
           // return false;
        } else {
            LOGD("open_logport: consyslog_port[%d]=%d",port_idx,consyslog_port[port_idx]);

        }
    }
   return true;
}

int port_read(int port, const void *read_buf, int read_len, int timeout_first, int timeout_gap)
{
    LOGV("read logs: port=%d, len=%d, to=%d,%d", port, read_len, timeout_first, timeout_gap);
    int bytes_read = 0;
    int read_size;
    char *data_ptr = (char *)read_buf;
    int to = timeout_first;

    do {
        read_size = read(port, data_ptr, read_len-bytes_read);

        if (read_size == -1) {
            if (errno == EAGAIN) {
                struct pollfd event;
                memset(&event,0,sizeof(event));
                event.fd=port;
                event.events=POLLIN;
                int ret_poll = poll((struct pollfd *)&event, 1, (to == -1? -1 : to*1000)); //wait for 5000 msec
                if (ret_poll == 0) {
                    LOGV("port_read: timeout (%d sec), read_byte=%d", to, bytes_read);
                    return bytes_read;
                }
                if (ret_poll < 0) {
                    LOGI("port_read: poll error, err=%d", errno);
                    return -1;
                }
                if (event.revents & POLLERR) {
                    LOGI("port_read: got POLLERR");
                    return -2;
                }
                if (!(event.revents & POLLIN)) {
                    LOGW("port_read: unexpected poll revent %d", event.revents);
                    return -1;
                }

                continue;
            }
            LOGV("port_read: read error,port = %d, read_len=%d, bytes_read=%d, err=%d",port, read_len, bytes_read, errno);
            return -1;
        }

        bytes_read += read_size;
        data_ptr += read_size;
       // usleep(100000);
        to = timeout_gap;
    } while (bytes_read < read_len && to != 0);
    if (read_size != 0){
    LOGV("port_read: read done, read_size=%d, ,port=%d", read_size,port);
    }
    return read_size;
}

bool Init()
{
    LOGV("Init");
    return true;
}

void setlogFD() {

    int port_idx = 0;
    FD_ZERO(&read_fds);
    maxfd = 0;

    for (port_idx = 0;port_idx < CONSYS_END;port_idx++){
        if (consyslog_port[port_idx] > 0) {
            FD_SET(consyslog_port[port_idx], &read_fds);
            if (consyslog_port[port_idx] > maxfd) {
                maxfd = consyslog_port[port_idx];
            }
        }
    }
    if (pfd[0] > 0) {
       FD_SET(pfd[0],&read_fds);
       maxfd = max(maxfd,pfd[0]);
    //   LOGE("add pipe fd to fdset");
    }
}
void close_logport() {
    
     for (int port_idx = 0;port_idx < CONSYS_END;port_idx++){
       if (consyslog_port[port_idx] == -1) {
           LOGI("port close: port%d not open!",port_idx);
           continue;
       }
       if (close(consyslog_port[port_idx]) == -1) {
           LOGE("port_close: close fail:%d, errno %d", port_idx,errno);
       }else {
           LOGI("radio %d port close!",port_idx);
       }
       consyslog_port[port_idx] = -1;
    }
}

void reopen_logport()
{
    close_logport();
    for (int port_idx = 0; port_idx <CONSYS_END; port_idx++) {
        if (!log_started) {
            LOGE("reopen_logport: log stopped clean up");
            close_logport();
            return;
        }
        if (consyslog_port[port_idx] != -1) {
            LOGI("reopen_logport: port already open! idx=%d",port_idx);
            continue;
        }
        consyslog_port[port_idx] = open(consyslog_node[port_idx], O_RDWR | O_NONBLOCK);
        if (consyslog_port[port_idx] == -1) {
            LOGE("reopen_logport: open fail, idx = %d,node = %s err=%d.",port_idx,consyslog_node[port_idx],errno);
        } else {
            LOGD("reopen_logport: consyslog_port[%d]=%d",port_idx,consyslog_port[port_idx]);
        }
    }
}

void Deinit()
{
    if(isEngineerBuild()) {
    LOGD("Deinit");
    }
}

int sendIOCtrl(CONSYS_TYPE radio_type,int cmd,int IoCtrl=FW_LOG_IOCTL_ON_OFF)
{
   int ret = 0;
   int ioret = ioctl(consyslog_port[radio_type],IoCtrl,cmd);
   if (ioret == -1) {
       LOGE("sendIOCtrl error consys type:%d: ioctl return error,error: %s",radio_type,strerror(errno));
       ret = -1;
       return ret;
   }
   LOGD("sendIOCtrl: %d,done.cmd=%d",radio_type,cmd);
   return ret;
}


RETCODE SendIOCtrlCmd(CONSYS_TYPE radio_type,int cmd,int IoCtrl)
{
    int ret = 0;
    int wifi_ret,bt_ret,gsp_ret,wcn_ret ;
    switch(radio_type){
        case CONSYS_WIFI:
            ret = sendIOCtrl(CONSYS_WIFI,cmd,IoCtrl);
            break;
        case CONSYS_BT:
            ret = sendIOCtrl(CONSYS_BT,cmd,IoCtrl);
            break;
        case CONSYS_GPS:
            ret = sendIOCtrl(CONSYS_GPS,cmd,IoCtrl);
            break;
        case CONSYS_MCU:
            ret = sendIOCtrl(CONSYS_MCU,cmd,IoCtrl);
            break;
        case CONSYS_END:
            wifi_ret = sendIOCtrl(CONSYS_WIFI,cmd,IoCtrl);
            bt_ret = sendIOCtrl(CONSYS_BT,cmd,IoCtrl);
            gsp_ret = sendIOCtrl(CONSYS_GPS,cmd,IoCtrl);
            wcn_ret = sendIOCtrl(CONSYS_MCU,cmd,IoCtrl);
          //  ret = wifi_ret + bt_ret + gsp_ret + wcn_ret;
            break;
       default:
           break;
         }
       if (ret == 0) {
          return OK;
       }
       return ERROR;
}
/*
the driver will unlock select() function only in fw logs are available.
it will not unlock select(), even if stop logging and device port is closed.
should create a pipe to signal select() function to unlock when logging is stopped.
*/
void CreatePipe() {
    if ((pfd[0] < 0) && (pfd[1] < 0)) {
        if (pipe(pfd) == -1) {
            LOGE("create pipe failed,err=%d",errno);
        }
        LOGD("new pipe fd:fd[0]=%d,fd[1]=%d",pfd[0],pfd[1]);
    } else {
        LOGD("pipe fd already generated:fd[0]=%d,fd[1]=%d",pfd[0],pfd[1]);
    }
}

RETCODE OnLoggingStart(int mode)
{
    if(isEngineerBuild()){
        LOGD("OnLoggingStart:%d",mode);
    }
    RETCODE ret = ERROR;
    if (log_started == false) {
        open_logport();
        CreatePipe();
         // set BT debug level firstly for BT flow
        int level = getDebugLevel();
        if (level != -1) {
            SetDebugLevel(CONSYS_BT,level);
        }
        int onoff = 1;
        ret = SendIOCtrlCmd(CONSYS_END,onoff,FW_LOG_IOCTL_ON_OFF);
        log_started = true;
    }

    if (ret != OK) {
        LOGD("OnLoggingStart: clean up failure");
        OnLoggingStop();
    }

    return ret;
}


void OnLoggingStop()
{
    LOGD("OnLoggingStop");
    int onoff = 0;
    SendIOCtrlCmd(CONSYS_END,onoff,FW_LOG_IOCTL_ON_OFF);
    close_logport();
    LOGD("OnLoggingStop: log port closed");

    log_started = false;
    
}
//pthread_mutex_t mlock;
int ListenLogPort(int* log_idx) {
    int port_idx = 0;
    int rc = 0;
    while (1) {
       setlogFD();
      // LOGD("begin to select log port...........");
        if( (rc = select(maxfd+1, &read_fds, NULL, NULL, NULL)) < 0) {
            LOGE("Wait log FD select failed (%s), errno = %d", strerror(errno), errno);
            if (errno == EBADF) {
                reopen_logport();
                break;
            } else if (errno == EINTR || errno == EAGAIN) {
                sleep(2);
                continue;
            }
        }
        if (FD_ISSET(pfd[0],&read_fds)) {
            //read pipe buffer
            int onoff = -1;
            if (read(pfd[0],&onoff,sizeof(onoff)) == -1) {
                LOGE("select unlock,read pipe error:%d",errno);
            }
            LOGD("log stop ,select unlock,read status:%d",onoff);
            break;
        }
     //   LOGD("accept fd:%d logs,return RC=%d",rc);
        for (port_idx = 0;port_idx < CONSYS_END;port_idx++){
            if (consyslog_port[port_idx] == -1){// some port maybe open
            // failed,and port maybe -1, we should not handle this radio
                //LOGE("port index:%d not opened",port_idx);
                continue;
            }
            if(FD_ISSET(consyslog_port[port_idx], &read_fds)){
              // LOGD("accept fd:%d logs,log_port=%d",port_idx,consyslog_port[port_idx]);
                log_idx[port_idx] = port_idx;
            }
         }
         break;
    }
    return port_idx;
}

int ReadFromLogPort(const void *data, int len, int timeout,int& log_idx)
{
    int read_size = port_read(consyslog_port[log_idx], data, len, timeout, 0);
    return read_size;
}

const char *GetKeyWord(int type)
{
    static const char *keyword[] = {"WIFI","BT", "GPS", "MCU"};
    return keyword[type];
}

int SetDebugLevel(CONSYS_TYPE connsys_radio,int level){
    return (int)SendIOCtrlCmd(connsys_radio,level,FW_LOG_IOCTL_SET_LEVEL);
}

/*
write any buffer to pipe, then the select() function will be unlocked
*/
int SignalThreadExit() {
    int onoff = 0;
    if (write(pfd[1],&onoff,sizeof(onoff)) < 0) {
      LOGE("OnLoggingStop: write pipe failed err=%d",errno);
      return 0;
    }
    return 1;
}

