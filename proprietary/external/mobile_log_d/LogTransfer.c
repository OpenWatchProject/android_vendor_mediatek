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
#include <errno.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#include <log/log.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdbool.h>

#include <cutils/properties.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "global_var.h"
#include "mlog.h"
#include "libfunc.h"

union ui2char {
    unsigned int d;
    char c[4];
};
typedef union ui2char ui2char_t;

struct file_header {
    char name[64];  // file name
    unsigned int len;  // file size
    struct file_header *next;
};
typedef struct file_header header_t;

static void insert_to_list(header_t *node, header_t *list) {
    node->next = list->next;
    list->next = node;
}

int port_fd = -1;  // for send log to meta usb com
int transfer_mode = 0;  // wifi or USB com

#define PROP_META_CONN_TYPE "persist.vendor.meta.connecttype"
#define PROP_WIFI_ADDR      "persist.vendor.atm.ipaddress"

bool m_bConnect = false;
int m_nSockFd = -1;
int m_nClientFd = -1;
static const int WIFI_SOCKET_PORT = 10119;
static const int BACKLOG = 32;


signed int createSocket(char* serverIP __attribute__((__unused__))) {
    int sock_opt = 1;
    int enable = 1;
    char ipAddr[PROPERTY_VALUE_MAX];

    if (m_nSockFd != -1) {
        MLOGI_BOTH("mobileLog Wifi Socket already created.");
        return m_nSockFd;
    }

    if ((m_nSockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        MLOGE_BOTH("mobileLog Wifi Socket created fail. errno=%d", errno);
        return -1;
    }

    MLOGI_BOTH("mobileLog Wifi Socket created success m_nSockFd:%d", m_nSockFd);

    if (setsockopt(m_nSockFd, SOL_SOCKET, SO_REUSEADDR, (void*)&sock_opt, sizeof(sock_opt)) < 0) {
        MLOGE_BOTH("mobileLog wifi Socket setsockopt failed. errno=%d", errno);
        return -1;
    }

    if (setsockopt(m_nSockFd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable)) < 0) {
        MLOGE_BOTH("mobileLog Wifi Socket setsockopt TCP_NODELAY failed. errno=%d", errno);
    }
    struct sockaddr_in* serverAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    if (serverAddr == NULL) {
        MLOGE_BOTH("mobileLog Wifi Socket new server addr failed. errno=%d", errno);
        close(m_nSockFd);
        m_nSockFd =  -1;
        return -1;
    }

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(WIFI_SOCKET_PORT);

    property_get(PROP_WIFI_ADDR, ipAddr, "0.0.0.0");
    serverAddr->sin_addr.s_addr = inet_addr(ipAddr);
    MLOGI_BOTH("Wifi IP Address: %s", ipAddr);

    if (bind(m_nSockFd, (struct sockaddr*)serverAddr, sizeof(struct sockaddr)) < 0) {
        MLOGE_BOTH("mobileLog Wifi Socket bind failed. errno=%d", errno);
        goto errout;
    }
    MLOGI_BOTH("mobileLog Wifi Socket bind done");

    if (listen(m_nSockFd, BACKLOG) == -1) {
        MLOGE_BOTH("mobileLog Wifi Socket Failed to listen Socket port, errno=%d", errno);
        goto errout;
    }
    MLOGI_BOTH("mobileLog Wifi Socket listen done");

    free(serverAddr);
    serverAddr = NULL;
    return m_nSockFd;

errout:
    free(serverAddr);
    serverAddr = NULL;
    close(m_nSockFd);
    m_nSockFd = -1;
    return -1;
}

int connectWithClient() {
    if (-1 == m_nSockFd) {
        return -1;
    }

    if (m_bConnect) {
        return 0;
    }

    struct sockaddr_in* clientAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    if (clientAddr == NULL) {
        MLOGE_BOTH("mobileLog wifi Socket new client addr failed. errno=%d", errno);
        m_bConnect = false;
        return -1;
    }
    memset(clientAddr, 0, sizeof(struct sockaddr_in));
    socklen_t alen = sizeof(struct sockaddr);

    while (1) {
        MLOGI_BOTH("mobileLog wifi Socket connect, accept the connection........");
        if ((m_nClientFd = accept(m_nSockFd, (struct sockaddr*)clientAddr, &alen)) == -1) {
            MLOGE_BOTH("mobileLog Socket accept error, errno=%d", errno);

            if (errno == EAGAIN) {
                usleep(200*1000);
                continue;
            }
            m_bConnect = false;
            free(clientAddr);
            clientAddr = NULL;
            return -1;
        } else {
            m_bConnect = true;
            MLOGI_BOTH("mobileLog Socket connect, Received a connection from %s, m_nClientFd = %d",
                (char*)inet_ntoa(clientAddr->sin_addr), m_nClientFd);
            free(clientAddr);
            clientAddr = NULL;
            return 0;
        }
    }
}


void waitForConnect(void * arg __attribute__((__unused__))) {
    fd_set rfds;

    if (m_bConnect == true) {
        MLOGE_BOTH("already connected, no need waitForConnect");
        return;
    }

    if (m_nSockFd < 0) {
        MLOGE_BOTH("server not created, waitForConnect Fail");
        return;
    }

    while (1) {
        FD_ZERO(&rfds);
        if (m_nSockFd > 0) {
            FD_SET(m_nSockFd, &rfds);
        }
        if (m_nClientFd > 0) {
            FD_SET(m_nClientFd, &rfds);
        }

        int maxfd = 0;
        int rc = 0;
        maxfd = (m_nSockFd > m_nClientFd) ? (m_nSockFd+1) : (m_nClientFd+1);

        if ((rc = select(maxfd, &rfds, NULL, NULL, NULL)) < 0) {
            MLOGE_BOTH("waitForConnect, select failed (%s), errno = %d", strerror(errno), errno);
            sleep(1);
            continue;
        }

        if (FD_ISSET(m_nSockFd, &rfds)) {
            MLOGI_BOTH("begin to accept client connection");
            if (connectWithClient() == -1) {
                MLOGE_BOTH("accept client connection failed.");
                sleep(1);
                continue;
            }
            MLOGI_BOTH("accept client connection done.");
            break;
        }
    }

    return;
}

void disconnect() {
    if (m_bConnect) {
        MLOGI_BOTH("disconnect ... close clientFd");
        if (m_nClientFd != -1) {
            close(m_nClientFd);
            m_nClientFd = -1;
        }
        m_bConnect = false;
    }
}

void socketClose() {
    disconnect();
    if (m_nSockFd != -1) {
        close(m_nSockFd);
        m_nSockFd = -1;
        MLOGI_BOTH("disconnect ... close server");
    }
}

int checkConnectType() {
    char connectType[PROPERTY_VALUE_MAX];
    property_get(PROP_META_CONN_TYPE, connectType, "");
    if (!strncmp("wifi", connectType, strlen("wifi"))) {
        return 0;
    }
    return 1;
}

int initPort() {
    /*   ::Test::
    #define dst  "/data/misc/mblog/aplog-com"
    dstFd = TEMP_FAILURE_RETRY(open(dst, O_WRONLY | O_CREAT | O_APPEND, 0664));
    if (dstFd < 0) {
        MLOGE_BOTH("open %s fail (%s)", dst, strerror(errno));
        return -1;
    }
    */

    if (port_fd != -1) {
        close(port_fd);
    }

    const char* com_dev_name;
    com_dev_name = "/dev/ttyGS3";
    port_fd = open(com_dev_name, O_RDWR | O_NOCTTY);
    if (port_fd == -1) {
        MLOGE_BOTH("Open com device: %s fail (%s)", com_dev_name, strerror(errno));
        return -1;
    } else
        MLOGI_BOTH("Open com device: %s successfully, port_fd:%d", com_dev_name, port_fd);

    struct termios termOptions;

    if (fcntl(port_fd, F_SETFL, 0) == -1)
        MLOGE_BOTH("fcntl port_fd fail (%s)", strerror(errno));

    // Get its current options
    tcgetattr(port_fd, &termOptions);

    // raw mode
    termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
    termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*raw input*/
    termOptions.c_oflag &= ~OPOST; /*raw output*/

    tcflush(port_fd, TCIFLUSH); /* clear input buffer */
    termOptions.c_cc[VTIME] = 1; /* inter-character timer unused */
    termOptions.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */

    tcsetattr(port_fd, TCSANOW, &termOptions);
    return 0;
}

int sendData(const void *data, int len) {
    int total = 0;
    const char* buf = data;
    int fd = transfer_mode ? port_fd : m_nClientFd;

    while (len > 0) {
        int ret = TEMP_FAILURE_RETRY(write(fd, buf, len));

        if (ret < 0) {
            MLOGE_BOTH("Send data Fail (%s), send: %d, left: %d", strerror(errno), total, len);
            if ((errno == EIO || errno == EBADF) && transfer_mode) {
                initPort();
            }
            goto Exit;
        }

        total += ret;
        buf += ret;
        len -= ret;
    }
Exit:
    return total;
}

int sendLogFileData(const char* log_dir, header_t*  node) {
    char src[256];
    int srcFd;
    unsigned int len = 0;
    ui2char_t len_chars;
    int ret = 0;

    char buf[8192];  // buffer for read and write
    ssize_t readCount, writeCount;
    ssize_t total_len = 0;

    // max size for written
    len = node->len;
    len_chars.d = len;
    // source file
    snprintf(src, sizeof(src), "%s%s", log_dir, node->name);
    // open src
    srcFd = TEMP_FAILURE_RETRY(open(src, O_RDONLY, 0));
    if (srcFd < 0) {
        MLOGE_BOTH("open %s fail (%s)", src, strerror(errno));
        return -1;
    }

    /* write file header */
    sendData(node->name, sizeof(node->name));
    sendData(len_chars.c, sizeof(len_chars.c));

    /*
    *             Write file content
    *
    *  Read a chunk, write it, and repeat.
    */
    while (1) {
        readCount = TEMP_FAILURE_RETRY(read(srcFd, buf, sizeof(buf)));
        if (readCount < 0) {
            MLOGE_BOTH("Failed reading %s, %s", src, strerror(errno));
            ret = -1;
            break;
        }

        total_len += readCount;
        if (readCount > 0 && total_len > (ssize_t) len) {
            MLOGE_BOTH("read %s oversize, readCount: %zd, over: %zd", src, readCount, total_len - len);
            readCount -= total_len - len;
        }
        if (readCount > 0) {
            writeCount = sendData(buf, readCount);
            if (writeCount != readCount) {
                MLOGE_BOTH("Partial write to port, (%zd of %zd)", writeCount, readCount);
                ret = -1;
                break;
            }
        }

        if (total_len >= (ssize_t) len || readCount < (ssize_t) sizeof(buf)) {
            break;
        }
    }
    close(srcFd);
    return ret;
}

void* packlog_and_send(void* arg __attribute__((__unused__))) {
    DIR *dp;
    struct dirent *entry;
    struct stat st;
    char log_dir[256];
    char file_path[256];
    unsigned int file_count = 0;

    header_t *list = NULL;
    header_t *node = NULL;
    header_t *cur_node = NULL;
    header_t *free_node = NULL;

    char cmd[500];
    char tempBuf[256];
    char curDir[256];
    char curBase[128];

    // source path by pack
    snprintf(tempBuf, sizeof(tempBuf), "%s", cur_logging_path);
    snprintf(curDir, sizeof(curDir), "%s", dirname(tempBuf));
    snprintf(tempBuf, sizeof(tempBuf), "%s", cur_logging_path);
    snprintf(curBase, sizeof(curBase), "%s", basename(tempBuf));
    MLOGI_BOTH("Pack srcPath: %s/, %s", curDir, curBase);

    if (curBase[0] == '/' || curBase[0] == '\0') {
        MLOGI_BOTH("This may cause packing root path.");
        return NULL;
    }

    // dest path by pack
    snprintf(log_dir, sizeof(log_dir), "%s", LOG_DIR_PACKED);
    if (log_dir[strlen(log_dir) - 1] != '/')
        log_dir[strlen(log_dir)] = '/';
    MLOGI_BOTH("Pack Log to: %s...........", log_dir);

    if (access(log_dir, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s*", log_dir);
        MLOGI_BOTH("cmd: %s", cmd);
        system(cmd);
    } else if (!create_dir(log_dir)) {
        MLOGE_BOTH("creat %s fail (%s)", log_dir, strerror(errno));
        return NULL;
    }

    // pack by calling 'tar  -zcvf /.../xxx.tar.gz  filepath -C dir'
    snprintf(cmd, sizeof(cmd), "tar -zcf %s%s.tar.gz %s/ -C %s", log_dir, curBase, curBase, curDir);
    MLOGI_BOTH("cmd: %s", cmd);
    system(cmd);
    usleep(200000);


    /* vendor.MB.packed = 0, start pulling, but this not used by META tool */
    property_set(PROP_PACKED, "0");

    transfer_mode = checkConnectType();
    MLOGI_BOTH("Transfer mode: %s", transfer_mode ? "usb" : "wifi");
    // init the connection
    if (transfer_mode) {  // usb com mode
        if (initPort() == -1) {
            MLOGE_BOTH("Failed to init com port");
            return NULL;
        }
    }

    /* instead usb port for test
    int flags = O_RDWR | O_CREAT | O_APPEND;
    int mode = S_IRUSR | S_IWUSR;
    port_fd = TEMP_FAILURE_RETRY(open(TEST_PORT, flags, mode));
    if (port_fd < 0) {
        MLOGE_BOTH("Fail to open TEST_PORT, %s", strerror(errno));
        return NULL;
    }*/

    /* search AP log file to send */
    dp = opendir(log_dir);
    if (dp == NULL) {
        MLOGE_BOTH("Failed to opendir %s, (%s)", log_dir, strerror(errno));
        /* vendor.MB.packed = 2, No log */
        property_set(PROP_PACKED, "2");
        return NULL;
    }
    // init  file_header list
    list = (header_t *)malloc(sizeof(header_t));
    if (list == NULL) {
        MLOGE_BOTH("pack: malloc header_t list fail");
        closedir(dp);
        return NULL;
    }
    memset(list->name, 0x00, sizeof(list->name));
    list->len = 0;
    list->next = NULL;

    // Loop through the directory stream, create 'list'
    while ((entry = readdir(dp)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")
            || entry->d_type == DT_DIR) {
            continue;
        }

        // create header_t node
        node = (header_t *)malloc(sizeof(header_t));
        if (node == NULL) {
            MLOGE_BOTH("pack: malloc header_t fail");
            continue;
        }
        memset(node->name, 0x00, sizeof(node->name));
        snprintf(node->name, sizeof(node->name), "%s", entry->d_name);
        snprintf(file_path, sizeof(file_path), "%s%s", log_dir, node->name);
        if (stat(file_path, &st) == -1) {
            MLOGE_BOTH("Failed to stat %s (%s)", file_path, strerror(errno));
            node->len = 0;
        } else {
            file_count++;
            node->len = st.st_size;
        }
        node->next = NULL;
        // insert the 'node' to 'list'
        insert_to_list(node, list);
    }
    MLOGI_BOTH("Pack log file_count: %u", file_count);
    if (closedir(dp)) {
        MLOGE_BOTH("Failed to closedir %s, (%s)", log_dir, strerror(errno));
    }

    /**                                      --- send AP logs to com port ---
    *
    *
    *       16              4           64 + 4                           64 + 4                                   16 (Byte)
    *    PREFIX | file Count |file1 Header | file1 Data | file2 Header | file2 Data | ... | POSTFIX
    *
    *
    */

    #define PREFIX       "mobilelog_start"
    #define POSTFIX      "mobilelog_stop"
    char prefix[16];
    char postfix[16];
    memset(prefix, 0x00, sizeof(prefix));
    memset(postfix, 0x00, sizeof(postfix));
    snprintf(prefix, sizeof(prefix), "%s", PREFIX);
    snprintf(postfix, sizeof(postfix), "%s", POSTFIX);
    ui2char_t file_cnt;
    file_cnt.d = file_count;

    sendData(prefix, sizeof(prefix));  // Send PREFIX
    sendData(file_cnt.c, 4);  // Send File Count

    // Send File Header + File Data
    ///*
    cur_node = list->next;
    while (cur_node) {
        MLOGI_BOTH("pack log name: %s, len: %u", cur_node->name, cur_node->len);

        sendLogFileData(log_dir, cur_node);
        cur_node = cur_node->next;
    }
    //*/
    sendData(postfix, sizeof(postfix));  // Send POSTFIX

    usleep(50 * 1000);  // wait for Data flush

    if (port_fd != -1) {
        close(port_fd);
    }
    socketClose();

    /* vendor.MB.packed = 1, pull done! */
    property_set(PROP_PACKED, "1");

    /* free the whole 'list' */
    cur_node = list;
    while (cur_node) {
        free_node = cur_node;
        cur_node = cur_node->next;
        free(free_node);
    }

    return NULL;
}

