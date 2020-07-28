/******************************************************************************
 *
 * Filename:
 * ---------
 *   socketconnection.cpp
 *
 * Description:
 * ------------
 *   socketconnection implementation
 *
 *****************************************************************************/

#include "SocketConnection.h"
#include "ConsysLog.h"
#include "ConsysLogger.h"
#include "Utils.h"

const char *ACK_OK = "cmd_ack_ok";
const char *ACK_FAIL = "cmd_ack_fail";

socketconnection::socketconnection() {
    int i;
    for (i = 0; i < 10; i++) {
        client[i].fd = -1;
    }
    clientConnect = -1;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    pthread_mutex_init(&mlock, NULL);
    pthread_mutex_init(&mMsglock, NULL);

    shouldtimeout = false;
    mThread = -1;
    mSockListenId = -1;
}

socketconnection::~socketconnection() {
    pthread_mutex_destroy(&mlock);
    pthread_mutex_destroy(&mMsglock);
}

void socketconnection::startListening() {
    int ret;
    do {
        mSockListenId = socket_local_server(SOCKET_SERVER_NAME,
                ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);

        if (mSockListenId < 0) {
            LOGE("mSockListenId<0 ,errno=%d",errno);
            break;
        }
        LOGE("mSockListenId ok\n");

        ret = listen(mSockListenId, 4);
        if (ret < 0) {
            LOGE("listen error.errno=%d",errno);
            break;
        }
        int err = PTHREAD_CREATE("csl_sock_host", &mThread, NULL,
                socketconnection::threadStart, this);
        if (err != 0) {
            LOGE("PTHREAD_CREATE err=%d",err);
            break;
        }
        LOGD("Success to setup socket local server");
        return;
    } while (0);
    LOGD("Fail to setup socket local server, exit");
    if (mSockListenId > 0) {
        shutdown(mSockListenId, 2);
        close(mSockListenId);
        mSockListenId = -1;
    }
    exit(2);
}

void *socketconnection::threadStart(void *obj) {
    socketconnection *sc = reinterpret_cast<socketconnection *>(obj);
    sc->runListener();

    pthread_exit(NULL);
    return NULL;
}

void socketconnection::runListener() {
    if(isEngineerBuild()){
        LOGD("runListener");
    }
    int maxFd = 0;
    int ret = 0;
    fd_set read_fds;
    while (1) {
     FD_ZERO(&read_fds);
      if (mSockListenId > 0) {
          FD_SET(mSockListenId, &read_fds);
      }
      if (clientConnect > 0) {
          FD_SET(clientConnect, &read_fds);
      }
      
      maxFd= (mSockListenId > clientConnect) ? mSockListenId : clientConnect;
      
        if (isEngineerBuild()){
          LOGD("consyslogger socket select start");
       }
        if ((ret= select(maxFd+ 1, &read_fds, NULL, NULL,
                shouldtimeout ? &timeout : NULL)) < 0) {
            LOGE("select failed (%s), errno = %d", strerror(errno), errno);
            sleep(1);
            continue;
        } else if (!ret) {
            LOGE("select timeout");
            shouldtimeout = false;
            continue;
        }
        LOGD("consyslogger select done");
        shouldtimeout = false;
        // deal with new connection
        if (FD_ISSET(mSockListenId, &read_fds)) {
            struct sockaddr addr;
            socklen_t alen = sizeof(addr);
            int connectfd = -1;
            if (isEngineerBuild()) {
                LOGD("consyslgger begin to accept connection");
            }

            if ((connectfd = accept(mSockListenId, &addr, &alen)) < 0) {
                LOGE("accept failed (%s) , errno = %d", strerror(errno), errno);
                sleep(1);
                continue;
            }

            LOGD("consyslgger accept connection done connectfd=%d",connectfd);

            if (connectfd != clientConnect) {
                clientConnect = connectfd;
            }
            continue;
        }
        // socket channel command
        if (FD_ISSET(clientConnect, &read_fds)) {
            if (!commandHandler(clientConnect)) {
                close(clientConnect);
                FD_CLR(clientConnect, &read_fds);
                clientConnect = -1;
                continue;
             }
            FD_CLR(clientConnect, &read_fds);
            continue;
        }
    }
}

bool socketconnection::sendCommandToClient(const int msgid,
        const char *msgdata) {
    const char *msg;
    switch (msgid) {
    case MSG_FAIL_WRITE_FILE: {
        msg = "FAIL_WRITEFILE";
        break;
    }
    case MSG_SDCARD_NO_EXIST: {
        msg = "SDCARD_NOTEXIST";
        break;
    }
    case MSG_SDCARD_IS_FULL: {
        msg = "SDCARD_FULL";
        break;
    }
    case MSG_SDCARD_NO_LOG_FILE: {
        msg = "LOGFILE_NOTEXIST";
        break;
    }
    default: {
        ASSERT(msgdata != NULL);
        msg = msgdata;
        break;
    }
    }
    LOGV("Send Message %d to APK: %s, fd = %d.", msgid, msg, clientConnect);
    int len = strlen(msg);
    bool result = false;
    int retrycount = 5;
    pthread_mutex_lock(&mMsglock);
    do {
        if (clientConnect <= 0) {
            break;
        }
        int sendsize = write(clientConnect, msg, len);
        if (sendsize == len) {
            result = true;
            break;
        } else if (-1 == sendsize) {
            if (errno == EINTR) {
                continue;
            }
            LOGE("Failed to write socket, errno=%d", errno);
            result = false;
            break;
        }
        LOGE("incomplete write socket %d(%d:%d), and resend it.",
                clientConnect, sendsize, len);
        result = false;
    } while (retrycount-- > 0);
    pthread_mutex_unlock(&mMsglock);
    //SANITY TEST LOG LABEL. DONOT CHANGE
    LOGD("Send Message to client: %s, %d", msg, result);
    return result;
}

extern bool setStoragePath(char* path);
bool Meta_control = false;

bool socketconnection::commandHandler(int fd) {
    LOGV("consyslogger begin handle Socket command");
    char buffer[256] = {0};
    char cmdAck[512] = {0};
    memset(buffer, '\0', 256);
    memset(cmdAck, '\0', 512);
    int len;
    if ((len = read(fd, buffer, sizeof(buffer) - 1)) < 0) {
        LOGE("read() failed (%s), errno = %d", strerror(errno), errno);
        return false;
    } else if (!len) {
        LOGE("maybe client socket broken %d",errno);
        return false;
    }
    buffer[len] = '\0';
    LOGI("consyslogger receive command %s", buffer);
    int ret = 1;
    if (!strncmp(buffer, "deep_stop", strlen("deep_stop"))) {
        if (Meta_control == false)
        ret = executeCommand(OP_DEEP_STOP_LOGGING);

    } else if (!strncmp(buffer, "deep_start", strlen("deep_start"))) {
       if (Meta_control == false) {
           ret = executeCommand(OP_DEEP_START_LOGGING);
           if (ret == false) {
               LOGE("OP_DEEP_START_LOGGING, errno:%d, %s", errno, strerror(errno));
           }
       }
    } else if (!strncmp(buffer, "pause", strlen("pause"))) {
        if (Meta_control == false)
            ret = executeCommand(OP_PAUSE_LOGGING);

    } else if (!strncmp(buffer, "start", strlen("start"))) {
        if (Meta_control == false){
           ret = executeCommand(OP_START_LOGGING);
           if (ret == false) {
               LOGE("OP_START_LOGGING, errno:%d, %s", errno, strerror(errno));
           }
        }
   } else if (!strncmp(buffer, "logsize=", strlen("logsize="))) {
       char temp[256];
       memset(temp, '\0', 256);
       strncpy(temp, &buffer[strlen("logsize=")], sizeof(temp) - 1);
       temp[sizeof(temp) - 1] = '\0';
       ret = executeCommand(OP_SET_RECYCLE_SIZE, atoi(temp));

   } else if (!strncmp(buffer, "logfilesize=", strlen("logfilesize="))) {
       char temp[256];
       memset(temp, '\0', 256);
       strncpy(temp, &buffer[strlen("logfilesize=")], sizeof(temp) - 1);
       temp[sizeof(temp) - 1] = '\0';
       ret = executeCommand(OP_SET_LOGFILE_CAPACITY, atoi(temp));

   } else if (!strncmp(buffer, "autostart=", strlen("autostart="))) {
       ret = executeCommand(OP_SET_AUTOSTART_LOGGING_MODE,
                buffer[strlen("autostart=")] - '0');

   } else if (!strncmp(buffer,"set_storage_path", strlen("set_storage_path"))) {
       char temp[256];
       memset(temp, '\0', 256);
       strncpy(temp, &buffer[strlen("set_storage_path,")], sizeof(temp) - 1);
       temp[sizeof(temp) - 1] = '\0';
       // if storage path is empty or same with /data/connsyslog/bootupLog, no need to change path
       int len = strlen(temp);
       LOGI("storage path length :%s,len=%d", temp,len);
       if (len != 0) {
          if (strncmp(temp,CONSYSLOGGER_BOOTUP_FOLDER, strlen(CONSYSLOGGER_BOOTUP_FOLDER))) {
              ret = setStoragePath(temp);
              LOGI("update storage path");
          }
       }
   } else if (!strncmp(buffer,"set_btfw_log_level,",strlen("set_btfw_log_level,"))) {
       ret = executeCommand(OP_SET_DEBUG_LEVEL,buffer[strlen("set_btfw_log_level,")] - '0');

   } else if (!strncmp(buffer, "meta_connsys_start", strlen("meta_connsys_start"))) {
       Meta_control = true;
       ret = executeCommand(OP_META_START_LOGGING);
       if (ret == false) {
           LOGE("OP_DEEP_START_LOGGING, errno:%d, %s", errno, strerror(errno));
       }

   } else if (!strncmp(buffer, "meta_connsys_stop", strlen("meta_connsys_stop"))) {
       Meta_control = false;
       ret = executeCommand(OP_META_STOP_LOGGING);

   } else if (!strncmp(buffer, "pull_FWlog_start", strlen("pull_FWlog_start"))) {
       Meta_control = true;
       ret = executeCommand(OP_PULL_LOG_START);
   } else if (!strncmp(buffer, "pull_FWlog_stop", strlen("pull_FWlog_stop"))) {
       Meta_control = false;
       ret = executeCommand(OP_PULL_LOG_STOP);
   } else if (!strncmp(buffer,"delete_file,",strlen("delete_file,"))) {
       char temp[256];
       memset(temp, '\0', 256);
       strncpy(temp, &buffer[strlen("delete_file,")], sizeof(temp) - 1);
       temp[sizeof(temp) - 1] = '\0';
       if (strstr(temp,"..") != NULL) {
           LOGD("find special char in %s",temp);
           return ret;
       }
       ret = deleteFiles(temp);

   } else if (!strncmp(buffer,"delete_folder,",strlen("delete_folder,"))) {
       char temp[256];
       memset(temp, '\0', 256);
       strncpy(temp, &buffer[strlen("delete_folder,")], sizeof(temp) - 1);
       temp[sizeof(temp) - 1] = '\0';
       if (strstr(temp,"..") != NULL) {
           LOGD("find special char in %s",temp);
           return ret;
       }
       ret = deleteFiles(temp);

   } else {
       LOGE("unkown socket cmds %s", buffer);
       return true;
   }
   sprintf(cmdAck, "%s,%d", buffer, ret);
   write(fd, cmdAck, strlen(cmdAck));
   if (isEngineerBuild()){
       LOGI("consyslogger send ACK %s", cmdAck);
   }
   return true;
}

void socketconnection::stop() {
    if (mSockListenId > 0) {
        shutdown(mSockListenId, 2);
        close(mSockListenId);
    }
}
