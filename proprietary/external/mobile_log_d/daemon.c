/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <log/log.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <unistd.h>

#include "global_var.h"
#include "config.h"
#include "mlog.h"
#include "daemon.h"
#include "bootmode.h"
#include "libfunc.h"
#include "size_control.h"
#include "config.h"
#include "logging.h"
#include "dump.h"

extern void* packlog_and_send(void*);
extern int checkConnectType();
extern void disconnect();
extern void socketClose();
extern void waitForConnect(void*);
extern signed int createSocket(char*);
extern bool m_bConnect;

uint64_t STORAGE_FULL_EVENT = 1001;
uint64_t FOLDER_REMOVED_EVENT = 1002;
uint64_t SIG_STOPLOG_EVENT = 1003;
uint64_t AUTO_COPY_EVENT = 1004;

int event_fd;

#define  GET_REALPATH      "get_run_mbfolder"
#define  GET_PULL_STATE    "get_pullmblog_state"
#define  DELETE_FILE       "delete_file"
#define  DELETE_FOLDER     "delete_folder"
#define  PATH_CHECKER      "debuglogger/mobilelog"
#define  MAX_MSG_LEN       512

struct client {
    int fd;
    struct sockaddr addr;
};

struct client_msg {
    int fd;
    char msg[MAX_MSG_LEN];
};

int g_redirect_flag = 0;
int g_copy_wait = 0;  // default is Can_Do_Copy

pthread_mutex_t delay_timer_lock = PTHREAD_MUTEX_INITIALIZER;
int delay_timer_created = -1;
timer_t delay_timer;


// lineNo is from the end line, 1,2,3,...
int fgetsReverse(FILE* fp, char item[], int len, int lineNo) {
    int totalLine = 0;
    int linePos;

    if (fp == NULL || item == NULL || len <= 0 || lineNo > 10000 || lineNo <= 0) {
        MLOGE_BOTH("Require Item with invalid args.");
        return -1;
    }

    while (fgets(item, len, fp) != NULL) {
        totalLine++;
    }
    MLOGI_BOTH("totalLine = %d", totalLine);

    if ((linePos = totalLine - lineNo + 1) > 0) {
        if (fseek(fp, 0L, SEEK_SET) != 0) {
            MLOGE_BOTH("Error fseeking start of file, %s", strerror(errno));
            return -1;
        }
        MLOGI_BOTH("linePos = %d", linePos);
        int i = linePos;
        while (fgets(item, len, fp) != NULL) {
            i--;
            if (i == 0)
                break;
        }
    } else {
        MLOGE_BOTH("lineNo(%d) > totalLine(%d), Over flow!", lineNo, totalLine);
        return -1;
    }

    return 1;
}

int symbol_link(const char* path, const char* symblkPath) {
    char cmd[512];

    if (access(symblkPath, F_OK) != 0) {
        MLOGI_DATA("%s Not exists.", symblkPath);
    } else {
        MLOGI_DATA("%s exists, and unlink it first.", symblkPath);
        if (remove(symblkPath) < 0) {
            MLOGE_DATA("remove %s fail(%s)", symblkPath, strerror(errno));
            return -1;
        }
    }

    snprintf(cmd, sizeof(cmd), "ln -s %s %s", path, symblkPath);
    MLOGI_DATA("%s", cmd);
    system(cmd);
    return 1;
}

int make_full_path() {
    int len;
    char path[255], curTime[128], curFolderName[128];
    struct tm *tmp;
    time_t lt;

    FILE *fp;
    int fd = -1;
    char fileTreePath[255];
    char symblkPath[255];
    int symblkPath_prefix_len;
    unsigned long index;  // folder index
    sd_type_t logSd;  // sdcard type used to store log

    // get mobilelog path
    memset(path, 0x00, sizeof(path));
    snprintf(path, sizeof(path), "%s", sdContext.sdpath);

    len = strlen(path);
    if (path[len-1] != '/') {
        path[len] = '/';
        path[len+1] = '\0';
    }
    strncat(path, PATH_SUFFIX, sizeof(path) - 1 - len);  // ../debuglogger/mobilelog/
    snprintf(fileTreePath, sizeof(fileTreePath), "%s", path);  // copy to file tree path
    snprintf(symblkPath, sizeof(symblkPath), "%s", path);  // copy to file symblk path
    symblkPath_prefix_len = strlen(symblkPath);  // record symblkPath_prefix

    // create current APlog folder, Name Format: APLog_index__timestamp
    lt = time(NULL);
    tmp = localtime(&lt);
    memset(curTime, 0x0, sizeof(curTime));
    if (tmp != NULL) {
        strftime(curTime, sizeof(curTime), "%Y_%m%d_%H%M%S", tmp);
    } else {
        curTime[0] = 'E';
    }

    logSd = sdContext.location;
    index = read_folder_index(logSd);
    if (index < 10000)
        index++;
    else {
        MLOGE_BOTH("index is %lu, reset to 0", index);
        index = 0;
    }
    snprintf(curFolderName, sizeof(curFolderName), "APLog_%s__%lu/", curTime, index);
    len = strlen(path);
    strncat(path, curFolderName, sizeof(path) - 1 - len);

    if (!create_dir(path))
        return 0;

    /*  file_tree.txt */
    strncat(fileTreePath, "file_tree.txt", sizeof(fileTreePath) - 1 - strlen(fileTreePath));

    // for spec customer
    if (customId == CUSTOME_ID_GOD_A) {
        /* Soft symbol link the current logging folder -- SYMLINK "APLog_0"*/
        strncat(symblkPath, "APLog_0", sizeof(symblkPath) - 1 - strlen(symblkPath));
        MLOGI_DATA("symblkPath: %s", symblkPath);

        symbol_link(path, symblkPath);

        // create symbol link for recent logging path
        if (access(fileTreePath, F_OK) == 0) {
            FILE* f_1;
            char recentPath[255];
            char* p;
            if ((f_1 = fopen(fileTreePath, "r")) &&
                    (fgetsReverse(f_1, recentPath, sizeof(recentPath), 1) > 0)) {
                if ((p = strchr(recentPath, '\n')))
                    *p = '\0';
                symblkPath[symblkPath_prefix_len] = '\0';
                strncat(symblkPath, "APLog_1", sizeof(symblkPath) - 1 - strlen(symblkPath));
                MLOGI_DATA("symblkPath: %s", symblkPath);

                // SYMLINk "APLog_1"
                symbol_link(recentPath, symblkPath);
                fclose(f_1);
            } else
                MLOGE_BOTH("fail to open fileTree, (%s)", strerror(errno));
        } else
            MLOGI_DATA("%s Not exists.", fileTreePath);
    }

    /* Record the current logging folder 'path' to file_tree.txt */
    fd = mb_open(fileTreePath);
    if (fd >= 0 && (fp = fdopen(fd, "a+"))) {
        fputs(path, fp);
        fputs("\n", fp);
        fclose(fp);
    } else
        MLOGI_BOTH("open %d(%s) fail (%s)", fd, fileTreePath, strerror(errno));

    MLOGI_BOTH("log dir: %s", path);

    // update folder list
    curFolderName[strlen(curFolderName)-1] = '\0';
    update_folder_list(curFolderName, index, logSd);

    set_cur_logpath(path);
    return 1;
}


void maybe_config_msg(char *msg) {
    char *p = NULL;
    char key[128];

    snprintf(key, sizeof(key), "%s", msg);
    if (strlen(key) >= sizeof(key) - 1) {
        MLOGE_BOTH("msg[%s] is too long!", key);
        return;
    }
    if ((p = strchr(key, '=')) != NULL) {
        *p = '\0';
        update_config_value(key, p + 1);
    } else if ((p = strchr(key, ',')) != NULL &&
        !strncmp(key, STORAGE_PATH, strlen(STORAGE_PATH))) {
        update_sd_context(p + 1);
        property_set(PROP_OLDPATH, p + 1);
    } else {
        MLOGE_BOTH("%s not a config msg", key);
        return;
    }
}

int set_redirect_flag() {
    g_redirect_flag = 1;
    g_copy_wait = 1;  // Not Can_Do_Copy

    return 1;
}

void handle_client_msg(void *cli_msg) {
    int i, ret, fd;
    char msg[MAX_MSG_LEN];
    char ans[128];
    struct client_msg climsg;

    climsg = *(struct client_msg*)cli_msg;
    free(cli_msg);

    fd = climsg.fd;
    memset(msg, 0x0, sizeof(msg));
    snprintf(msg, sizeof(msg), "%s", climsg.msg);

    if (!strncmp(msg, "deep_start", 10)) {
        if (is_mblog_prepare()) return;
        if (is_mblog_running()) {
            if (g_mblog_status == SAVE_TO_BOOTUP) { /* running and saving to /data/log_temp */
                set_status(SAVE_TO_SDCARD);
                ret = make_full_path();
                if (ret == 0) {
                    MLOGE_DATA("make full path failed");
                    notify_client(fd, "die");
                    return;
                }
                set_redirect_flag();
                MLOGI_BOTH("redirect to sdcard");
                copy_and_dump();
            } else {
                MLOGE_BOTH("mblog already running ,why send deep_start?");
            }
            notify_client(fd, "deep_start,1");
            return;
        }

        MLOGI_BOTH("'deepstart' mblog");
        set_status(SAVE_TO_SDCARD);
        ret = make_full_path();
        if (ret == 0) {
            MLOGE_DATA("make full path failed");
            notify_client(fd, "die");
            return;
        }

        copy_and_dump();
        prepare_logging();

        update_config_value(KEY_BOOT, ENABLE);
    } else if (!strncmp(msg, "meta_start", 10)) {
        pthread_mutex_lock(&delay_timer_lock);
        if (delay_timer_created == 0) {
            if (timer_delete(delay_timer) == -1) {
                MLOGE_BOTH("meta_start timer_delete error %s", strerror(errno));
            }
            delay_timer_created = -1;
        }
        pthread_mutex_unlock(&delay_timer_lock);
        if (is_mblog_prepare()) return;
        /* vendor.MB.packed = NULL, clear status */
        property_set(PROP_PACKED, "");
        if (is_mblog_running()) {
            if (g_mblog_status == SAVE_TO_BOOTUP) { /* running and saving to /data/log_temp */
                set_status(SAVE_TO_SDCARD);
                ret = make_full_path();
                if (ret == 0) {
                    MLOGE_DATA("make full path failed");
                    notify_client(fd, "die");
                    return;
                }
                set_redirect_flag();
                MLOGI_BOTH("redirect to sdcard");
                copy_and_dump();
            } else {
                MLOGE_BOTH("mblog already running ,why send meta_start?");
            }
            notify_client(fd, "meta_start,1");
            return;
        } else {
            if (g_mblog_status != STOPPED) { /* not run, but preparing on boot up stage */
                MLOGE_BOTH("mblog not run, but is preparing.");
                notify_client(fd, "meta_start,1");
                return;
            }
            MLOGI_BOTH("'meta_start' mblog");
            // start logging
            set_status(SAVE_TO_SDCARD);
            ret = make_full_path();
            if (ret == 0) {
                MLOGE_DATA("make full path failed");
                notify_client(fd, "die");
                return;
            }
            copy_and_dump();
            prepare_logging();
        }
    } else if (!strncmp(msg, "start", 5)) {
        if (is_mblog_prepare()) return;
        if (is_mblog_running()) {
            if (g_mblog_status == SAVE_TO_BOOTUP) { /* running and saving to /data/log_temp */
                set_status(SAVE_TO_SDCARD);
                ret = make_full_path();
                if (ret == 0) {
                    MLOGE_DATA("make full path failed");
                    notify_client(fd, "die");
                    return;
                }
                set_redirect_flag();
                MLOGI_BOTH("redirect to sdcard");
                copy_and_dump();
            } else {
                MLOGE_BOTH("mblog already running ,why send start?");
            }
            notify_client(fd, "start,1");
            return;
        }

        MLOGI_BOTH("'start' mblog");
        set_status(SAVE_TO_SDCARD);
        ret = make_full_path();
        if (ret == 0) {
            MLOGE_DATA("make full path failed");
            notify_client(fd, "die");
            return;
        }

        copy_and_dump();
        prepare_logging();
    } else if (!strncmp(msg, "deep_stop", 9)) {
        if (!is_mblog_running()) {
            MLOGE_BOTH("mblog already stop ,why send deep_stop?");
            notify_client(fd, "deep_stop,1");
            return;
        }

        set_status(STOPPED);
        g_redirect_flag = 0;
        update_config_value(KEY_BOOT, DISABLE);
        for (i=0; i < 15; i++) {
            if (is_mblog_running()) {
                sleep(1);
            } else {
                break;
            }
        }
        if (i == 15) {
            return;
        }
    } else if ((!strncmp(msg, "meta_pause", 10)) || (!strncmp(msg, "stop", 4))) {
        if (!is_mblog_running()) {
            MLOGE_BOTH("mblog already stop ,why send stop?");
            snprintf(ans, sizeof(ans), "%s,1", msg);
            notify_client(fd, ans);
            return;
        }

        set_status(STOPPING);
        g_redirect_flag = 0;
        for (i=0; i < 15; i++) {
            if (is_mblog_running()) {
                sleep(1);
            } else {
                break;
            }
        }
        if (i == 15)
            return;
    } else if (!strncmp(msg, "copy", 4)) {
        if (is_mblog_prepare()) return;
        if (g_mblog_status == SAVE_TO_SDCARD) {
            MLOGE_BOTH("alreay save to sdcard, why send copy again?");
            notify_client(fd, "copy,1");
            return;
        }
        set_status(SAVE_TO_SDCARD);
        ret = make_full_path();
        if (ret == 0) {
            MLOGE_DATA("make full path failed");
            notify_client(fd, "die");
            set_status(STOPPED);
            return;
        }
        if (!is_mblog_running()) {
            MLOGI_BOTH("start log when copy");
            prepare_logging();
        } else {
            set_redirect_flag();
        }

        copy_and_dump();
    } else if (!strncmp(msg, "ext_autopull", 12)) {
        if (is_mblog_running()) {
            MLOGI_BOTH("mobileLog is running, stop logging first...");
            set_status(STOPPED);
            g_redirect_flag = 0;
            for (i=0; i < 15; i++) {
                if (is_mblog_running())
                    sleep(1);
                else {
                    break;
                }
            }
            if (i == 15) {
                MLOGE_BOTH("Can not stop, failed to 'ext_autopull'");
                return;
            }
        }
        // if pull log by wifi, here we need to create wifi connection.
        if (checkConnectType() == 0) {
            MLOGI_BOTH("Meta log transfer will use wifi.");
            createSocket(NULL);
            waitForConnect(NULL);
            if (!m_bConnect) {
                socketClose();
                MLOGE_BOTH("Create wifi connection fail.");
                notify_client(fd, "ext_autopull,0");
                return;
            }
        }
        // pack log and send
        packlog_and_send(NULL);
    }
    else if (!strncmp(msg, "ext_autostop", 12)) {
        /* vendor.MB.packed = NULL, clear status */
        property_set(PROP_PACKED, "");
        /* if still connected, disconnect */
        if (!m_bConnect) {
            socketClose();
        }
    } else if (!strncmp(msg, GET_REALPATH, strlen(GET_REALPATH))) {
        char respon[300];
        snprintf(respon, sizeof(respon), "%s,%s", GET_REALPATH, cur_logging_path);
        notify_client(fd, respon);
        return;
    } else if (!strncmp(msg, GET_PULL_STATE, strlen(GET_PULL_STATE))) {
        char respon[200];
        char prop_buf[PROPERTY_VALUE_MAX];
        property_get(PROP_PACKED, prop_buf, "");
        snprintf(respon, sizeof(respon), "%s,%s", GET_PULL_STATE, prop_buf);
        notify_client(fd, respon);
        return;
    } else if (!strncmp(msg, DELETE_FILE, strlen(DELETE_FILE))) {
        char *p = strchr(msg, ',');
        if (p != NULL) {
            if (!access(p + 1, F_OK)) {
                if (strstr(p + 1, PATH_CHECKER) != NULL) {
                    unlink(p + 1);
                } else {
                    MLOGE_BOTH("not mobile log path");
                }
            } else {
                MLOGE_BOTH("access %s fail, error is %s", p + 1, strerror(errno));
            }
        }
    } else if (!strncmp(msg, DELETE_FOLDER, strlen(DELETE_FOLDER))) {
        char *p = strchr(msg, ',');
        if (p != NULL) {
            if (!access(p + 1, F_OK)) {
                if (strstr(p + 1, PATH_CHECKER) != NULL) {
                    char cmd[MAX_MSG_LEN];
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "rm -rf %s", p + 1);
                    MLOGE_BOTH("delete folder, %s", cmd);
                    system(cmd);
                } else {
                    MLOGE_BOTH("not mobile log path");
                }
            } else {
                MLOGE_BOTH("access %s fail, error is %s", p + 1, strerror(errno));
            }
        }
    }
    else {
        maybe_config_msg(msg);
    }

    memset(ans, 0x0, sizeof(ans));
    snprintf(ans, sizeof(ans), "%s,1", msg);
    notify_client(fd, ans);
}

/*
 * When recv a msg from client, create a new thread to handle it
 */
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
void wait_client(void *socket_id) {
    prctl(PR_SET_NAME, "mobile_log_d.wc");
#define MAX_CLIENT_NUM          10

    struct client clients[MAX_CLIENT_NUM];
    int sock_id, max_fd = -1, ret, i, e;
    int should_timeout;
    fd_set read_fds;
    struct stat fd_state;
    struct timeval timeout = {600, 0};

    for (i=0; i < MAX_CLIENT_NUM; i++) {
        clients[i].fd = -1;
    }

    event_fd = eventfd(0, EFD_NONBLOCK);

    sock_id = *(int*)socket_id;

    should_timeout = timeout_in_this_boot();
    while (1) {
        FD_ZERO(&read_fds);
        if (sock_id > 0) {
            FD_SET(sock_id, &read_fds);
            max_fd = sock_id;
        }

        if (event_fd > 0) {
            FD_SET(event_fd, &read_fds);
            if (event_fd > max_fd)
                max_fd = event_fd;
        }

        pthread_mutex_lock(&client_lock);
        for (i=0; i < MAX_CLIENT_NUM; i++) {
            if (clients[i].fd > 0) {
                FD_SET(clients[i].fd, &read_fds);
                if (max_fd < clients[i].fd)
                    max_fd = clients[i].fd;
            }
        }
        pthread_mutex_unlock(&client_lock);

        ret = select(max_fd+1, &read_fds, NULL, NULL, should_timeout? &timeout: NULL);
        e = errno;
        if (ret < 0) {
            MLOGE_BOTH("select fail(%s)", strerror(e));
            for (i=0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].fd > 0) {
                    if (fstat(clients[i].fd, &fd_state) < 0) {
                        e = errno;
                        clients[i].fd = -1;
                        MLOGE_BOTH("reset client %d to -1: %s", clients[i].fd, strerror(e));
                    }
                }
            }
            continue;
        } else if (!ret) {
            MLOGE_DATA("select timeout,try copy to sd");
            set_status(SAVE_TO_SDCARD);
            update_sd_context(PATH_DEFAULT);  // default path is /sdcard/ (internal sd path)
            ret = make_full_path();
            if (ret == 0) {
                MLOGE_DATA("make full path failed,stop logging");
                set_status(STOPPED);
            } else {
                set_redirect_flag();
                copy_and_dump();
            }
            should_timeout = 0;
            continue;
        }

        if (FD_ISSET(sock_id, &read_fds)) {
            struct sockaddr addr;
            socklen_t alen = sizeof(addr);
            int client_fd;

            client_fd = accept(sock_id, &addr, &alen);
            if (client_fd < 0) {
                MLOGE_BOTH("accept from %d failed(%s)", sock_id, strerror(errno));
                continue;
            }

            MLOGI_BOTH("new client(%d) conected!", client_fd);
            pthread_mutex_lock(&client_lock);
            for (i=0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = client_fd;
                    clients[i].addr = addr;
                    break;
                }
            }
            pthread_mutex_unlock(&client_lock);

            should_timeout = 0;

            if (i == MAX_CLIENT_NUM) {
                MLOGE_BOTH("clients full ,cant accept a new one.");
            }
        } else {
            for (i=0; i < MAX_CLIENT_NUM; i++) {
                int fd = clients[i].fd;
                int e;

                if (fd < 0 || !FD_ISSET(fd, &read_fds))
                    continue;

                char temp[MAX_MSG_LEN] = {0};
                ret = TEMP_FAILURE_RETRY(recv(fd, temp, sizeof(temp)-1, 0));
                e = errno;
                if (ret <= 0) {
                    if (ret == 0)
                        MLOGE_BOTH("recv returned 0 from client %d(%s)", fd, strerror(e));
                    if (ret < 0)
                        MLOGE_BOTH("read error from client %d(%s)", fd, strerror(e));
                    MLOGI_BOTH("close client %d", fd);
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    memset((void *)&(clients[i].addr), 0x0, sizeof(struct sockaddr));
                    continue;
                }

                MLOGI_BOTH("recv %s from %d", temp, fd);

                struct client_msg *cli_msg = (struct client_msg*)malloc(sizeof(struct client_msg));
                if (cli_msg == NULL) {
                    MLOGE_BOTH("create cli_msg fail, %s", strerror(errno));
                    continue;
                }
                memset(cli_msg, 0x0, sizeof(struct client_msg));
                cli_msg->fd = fd;
                strncpy(cli_msg->msg, temp, sizeof(cli_msg->msg)-1);
                handle_client_msg(cli_msg);
                /*
                pthread_t handle_client_t;
                pthread_create(&handle_client_t, NULL, (void *)handle_client_msg, cli_msg);
                pthread_join(handle_client_t, NULL);
                */
            }
        }

        if (FD_ISSET(event_fd, &read_fds)) {
            int e, ret;
            uint64_t u = 0;
            ret = TEMP_FAILURE_RETRY(read(event_fd, &u, sizeof(u)));
            e = errno;
            if (ret > 0) {
                MLOGI_BOTH("read %llu from event_fd", (unsigned long long)u);
                if (u == STORAGE_FULL_EVENT) {
                    for (i=0; i < MAX_CLIENT_NUM; i++) {
                        int fd = clients[i].fd;
                        if (fd > 0) {
                            notify_client(fd, "storage_full");
                            close(clients[i].fd);
                            clients[i].fd = -1;
                            memset((void *)&(clients[i].addr), 0x0, sizeof(struct sockaddr));
                        }
                    }
                } else if (u == FOLDER_REMOVED_EVENT) {
                    while (is_mblog_running()) {
                        int ret = usleep(100000);
                        if (ret < 0) {
                            MLOGE_BOTH("usleep error %s", strerror(errno));
                        }
                    }
                    MLOGI_BOTH("restoring mobilelog...");
                    set_status(SAVE_TO_SDCARD);
                    if (!make_full_path()) {
                        MLOGE_DATA("make full path failed");
                        for (i=0; i < MAX_CLIENT_NUM; i++) {
                            int fd = clients[i].fd;
                            if (fd > 0) {
                                notify_client(fd, "die");
                            }
                        }
                        continue;
                    }
                    copy_and_dump();
                    prepare_logging();
                } else if (u == SIG_STOPLOG_EVENT) {  // stop logging
                    MLOGI_BOTH("SIG_STOPLOG_EVENT handler...");
                    if (!is_mblog_running()) {
                        MLOGI_BOTH("MobiLog already stop, ignore SIG_EVENT.");
                        continue;
                    }
                    set_status(STOPPED);
                    g_redirect_flag = 0;
                    for (i=0; i < 15; i++) {
                        if (is_mblog_running()) {
                            sleep(1);
                        } else {
                            break;
                        }
                    }
                    if (i == 15) {
                        MLOGE_BOTH("Can not stop, handle SIG_EVENT Fail.");
                    } else {
                        for (i=0; i < MAX_CLIENT_NUM; i++) {
                            int fd = clients[i].fd;
                            if (fd > 0) {
                                notify_client(fd, "die");
                            }
                        }
                        MLOGI_BOTH("SIG_STOPLOG_EVENT handler...end...");
                    }
                } else if (u == AUTO_COPY_EVENT) {
                    // auto copy and dump onto sdcard
                    if (g_mblog_status == SAVE_TO_BOOTUP) {  // ensure no 'meta_start' coming
                        if (!make_full_path()) {
                            MLOGE_BOTH("auto copy, make full path failed!");
                            return;
                        }
                        set_status(SAVE_TO_SDCARD);
                        set_redirect_flag();
                        copy_and_dump();
                    }
                }
            } else {
                MLOGE_BOTH("read error from event_fd(%s)", strerror(e));
            }
        }
    }
}

/*
* Attention Please:
*
* This function may be invoked by root. Do not use MLOG API during this function,
* MLOGx may create a root:root mblog_history which cannot access later
*/
int send_to_control(char* buf, size_t len) {
    int buf_size = len;
    char* cp = buf;

    int sock = socket_local_client("mobilelogd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (sock < 0) {
        ALOGE("setup client for mobilelogd fail (%s)", strerror(errno));
        return -1;
    }

    buf[buf_size - 1] = 0x00;
    if (TEMP_FAILURE_RETRY(write(sock, buf, strlen(buf) + 1)) <= 0) {
        ALOGE("send %s fail (%s)", buf, strerror(errno));
        return -1;
    } else {
        ssize_t ret = 0;
        int errno_save = 0;
        do {
            struct pollfd p = {
                .fd = sock,
                .events = POLLIN,
                .revents = 0
            };

            ret = TEMP_FAILURE_RETRY(poll(&p, 1, 20 * 1000));  // 20s timeout
            if (ret == 0) {
                ALOGE("poll timeout");
                break;
            }
            if (ret < 0) {
                ALOGE("poll error: %s", strerror(errno));
                break;
            }
            if (!(p.revents & POLLIN)) {
                ALOGE("poll not POLLIN: 0x%08x", p.revents);
                break;
            }

            ret = TEMP_FAILURE_RETRY(read(sock, cp, len));
            errno_save = errno;
            if (ret <= 0) {  // if peer closed, pollHUP and read return 0
                ALOGE("buf: %s, ret: %zd, (%s)", buf, ret, strerror(errno_save));
                break;
            } else {
                if ((size_t)ret == len)
                    break;
                len -= ret;
                cp += ret;
                buf[buf_size - len] = 0x00;
                ALOGI("response buf: %s", buf);
            }
        } while (0);
    }
    close(sock);
    return 0;
}


/*
 * Create server socket and start a thread to listen
 */
int setup_socket() {
    int ret;
    static int socket_id = 0;
    pthread_t thread_id;
    pthread_attr_t attr;

    socket_id = socket_local_server("mobilelogd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (socket_id < 0) {
        MLOGE_DATA("create server fail(%s).", strerror(errno));
        return 0;
    }

    if ((ret = listen(socket_id, MAX_CLIENT_NUM)) < 0) {
        MLOGE_DATA("listen socket fail(%s).", strerror(errno));
        close(socket_id);
        return 0;
    }

    if (!pthread_attr_init(&attr)) {
        if (!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
            if (!pthread_create(&thread_id, &attr, (void *)wait_client, (void *)&socket_id))
                pthread_attr_destroy(&attr);
            else {
                MLOGE_BOTH("create thread fail(%s)", strerror(errno));
                close(socket_id);
            }
        }
    }
    return 1;
}

void sig_timer_handler(int event) {
    MLOGI_BOTH("sig_timer_handler handle event %d", event);

    pthread_mutex_lock(&delay_timer_lock);
    if (delay_timer_created == 0) {
        if (timer_delete(delay_timer) == -1) {
            MLOGE_BOTH("sig_timer_handler timer_delete error %s", strerror(errno));
        }
        delay_timer_created = -1;
    }
    pthread_mutex_unlock(&delay_timer_lock);

    uint64_t u = AUTO_COPY_EVENT;
    int ret = TEMP_FAILURE_RETRY(write(event_fd, &u, sizeof(u)));
    if (ret < 0) {
        MLOGE_BOTH("sig_timer_handler, write %llu to event_fd error (%s)",
                (unsigned long long)u, strerror(errno));
    }
}

int set_delay_timer() {
    struct sigaction sa;
    struct sigevent se;
    struct itimerspec its;
    memset(&sa, 0, sizeof(sa));
    memset(&se, 0, sizeof(se));
    memset(&its, 0, sizeof(its));
    memset(&delay_timer, 0, sizeof(delay_timer));

    sa.sa_handler = sig_timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        MLOGE_BOTH("set_delay_timer sigaction error %s", strerror(errno));
        return -1;
    }

    pthread_mutex_lock(&delay_timer_lock);

    se.sigev_signo = SIGUSR1;
    se.sigev_notify = SIGEV_SIGNAL;
    if (timer_create(CLOCK_REALTIME, &se, &delay_timer) == -1) {
        MLOGE_BOTH("set_delay_timer timer_create error %s", strerror(errno));
        pthread_mutex_unlock(&delay_timer_lock);
        return -1;
    }

    delay_timer_created = 0;

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = DELAY_TIME;
    its.it_value.tv_nsec = 0;

    if (timer_settime(delay_timer, 0, &its, NULL) == -1) {
        MLOGE_BOTH("set_delay_timer timer_settime error %s", strerror(errno));
        if (timer_delete(delay_timer) == -1) {
            MLOGE_BOTH("set_delay_timer timer_delete error %s", strerror(errno));
        }
        delay_timer_created = -1;
        pthread_mutex_unlock(&delay_timer_lock);
        return -1;
    }

    pthread_mutex_unlock(&delay_timer_lock);
    return 0;
}
