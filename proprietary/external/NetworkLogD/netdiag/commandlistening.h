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

#ifndef command_listening_h
#define command_listening_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <cutils/log.h>
#include <cutils/sockets.h>

#include <cutils/properties.h>
#include <sys/system_properties.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include <signal.h>
#include <utils/List.h>

#include "utils.h"

#define MAX 128

struct CLIENT {
    int fd;
    struct sockaddr addr;
};

class command {
    const char *command_name;
public:
    int daemon_pid;
    command();
    command(const char *cmd, int pid) {
        command_name = cmd;
        daemon_pid = pid;
    }
    virtual ~command() {
    }

    inline const char *getcommand() {
        return command_name;
    }
    virtual void runcommand(int fd, char *location, char *action,
            char *address) = 0;
    //virtual int runcommand(int fd , char *location);
    //void find_pid(char* process_name , int *real_pid);
};

typedef android::List<command *> commandcollection;

class commandlistening {

public:
    commandlistening(const char *service);
    virtual ~commandlistening() {

    }

    class tcpdump_cmd: public command {
    private:
    public:
        tcpdump_cmd();
        virtual ~tcpdump_cmd() {
        }
        virtual void runcommand(int fd, char *location, char *action,
                char *address);
    };

    class clear_cmd: public command {
    private:
    public:
        clear_cmd();
        virtual ~clear_cmd() {
        }
        virtual void runcommand(int fd, char *location, char *action,
                char *address);

    };
    class runshell_cmd: public command {
    private:
    public:
        runshell_cmd();
        virtual ~runshell_cmd() {
        }
        virtual void runcommand(int fd, char *location, char *action,
                char *address);

    };

    class kill_cmd: public command {
    private:
    public:
        kill_cmd();
        virtual ~kill_cmd() {
        }
        virtual void runcommand(int fd, char *location, char *action,
                char *address);
    };

    pthread_mutex_t mlock_location;

    void startlistening();
    static void *threadStart(void *obj);

    void runListener();
    bool handle_message(int fd);
    void registercmd(command *cmd);
    //static  pthread_t        mThread_listenevent;
private:
    const char *sockname;
    int sock_listen_id;
    bool mlisten;
    int mIsSupportROHC;
    pthread_t mThread;
    pthread_t mThread_listenevent;
    pthread_mutex_t mlock;
//    char * location_ptr;

    fd_set read_fds;

    /*
    * support Chip mode
    */
    bool isSDCardReady();
    bool isChipMode();
    void runChipmode();
    bool initDataFolder();
    commandcollection *mcommands;
    //socketclientcollection *mclients;
    struct CLIENT client[10];
};

#endif

