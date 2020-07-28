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

#include <pthread.h>
#include "commandlistening.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/reboot.h>
//#include <selinux/android.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "utils.h"

#define TIMEOUT_STOP 14
#define IS_ROHC_SUPPORT "is_rohc_compression_support"
#define ENABLE_ROHC "enable_rohc_compression,"
#define SET_ROHC_TOTAL_FILE "set_rohc_total_file,"

/*
extern "C" int find_pid(pid_t * tcpdump_pid_str);
extern "C" void clear_begin(int fd, char *local);
extern "C" void tcpdump_data(int fd, char *local, char * NTLog_name,
        char * file_size);
extern "C" int shell(char * cmd);
extern "C" void full_test(char *local, char *action, char * NTLog_name,
        char * ping_flag);
extern "C" void stop_tcpdump(bool setStopProp);
*/

extern int apply_rohc_compress;
extern int rohc_total_file;

extern int find_pid(pid_t * tcpdump_pid_str);
extern void clear_begin(int fd, char *local);
extern void tcpdump_data(int fd, char *local, char * NTLog_name,
        char * file_size);
extern int shell(char * cmd);
extern void full_test(char *local, char *action, char * NTLog_name,
        char * ping_flag);
extern void stop_tcpdump(bool setStopProp);
extern int support_rohc();
extern int makeDir(const char *szPath);


/**
 * Used to record current connected client FD
 */
int global_fd;
void cmd_exec_sig_handler(int s);
char location[128];
bool handle_setprop_command(char* cmd);

/**
 * Storage change for M version
 */
char mStoragePath[128] = {0};
char mStrSetPath [32] = "set_storage_path,";
int remaining_time = 10;
bool mTestSaveLogInData = false;
extern bool mEnableSaveLogInData;

FILE * fp_runshell = NULL;

int test_pthread(pthread_t tid) /*ESRCH: pthread has exit; EINVAL:signal illegal*/
{
    int pthread_kill_err;
    LOGD("mListenThread :%ld", tid);
    pthread_kill_err = pthread_kill(tid, 0);

    if (pthread_kill_err == ESRCH) {
        LOGE("Pthread ID0x%x has exited", (unsigned int) tid);
        return -1;
    } else if (pthread_kill_err == EINVAL) {
        LOGE("signal illegal\n");
        return -1;
    } else {
        LOGE("Pthread ID0x%x is still alive", (unsigned int) tid);
        return 0;
    }
}

commandlistening::commandlistening(const char *service) {
    sock_listen_id = 0;
    sockname = service;
    mlisten = true;
    mThread_listenevent = 0;
    mThread = 0;

    int i;
    for (i = 0; i < 10; i++) {
        client[i].fd = -1;
    }

    pthread_mutex_init(&mlock, NULL);
    pthread_mutex_init(&mlock_location, NULL);
    mcommands = new commandcollection();

    registercmd(new tcpdump_cmd());
    registercmd(new runshell_cmd());
    registercmd(new clear_cmd());
    registercmd(new kill_cmd());
    mIsSupportROHC = support_rohc();

}

void commandlistening::registercmd(command *cmd) {
    mcommands->push_back(cmd);
}

void commandlistening::startlistening() {
//    int flag = 1;
    LOGD("commandlistening sockname=%s", sockname);
    sock_listen_id = socket_local_server(sockname,
                ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    // android_get_control_socket(sockname);
    if (sock_listen_id < 0) {
        LOGE("Failed to get socket %s", strerror(errno));
        exit(1);
    }
  //  setsockopt(sock_listen_id, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    int ret;
    ret = listen(sock_listen_id, 10);

    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s", sock_listen_id,
                strerror(errno));
        exit(-1);
    }
    LOGD("commandlistening::startlistening() ret:%d,sock_listen_id:%d", ret,
            sock_listen_id);

    if (pthread_create(&mThread, NULL, commandlistening::threadStart, this)) {
        LOGE("pthread_create mThread(%s)", strerror(errno));
        exit(-1);
    }
    pthread_join(mThread, NULL);

}

void *commandlistening::threadStart(void *obj) {

    LOGI("commandlistening threadStart doing");
    commandlistening *me = reinterpret_cast<commandlistening *>(obj);

    me->runListener();
    pthread_exit (NULL);
    return NULL;
}

bool commandlistening::isSDCardReady() {
    struct statfs fileStat;
    if (statfs(VZW_VALUE_EXTERNAL_SD_PATH, &fileStat) == -1) {
        return false;
    } else {
        if (0 == fileStat.f_blocks) {
            return false;
        }
        // m_nBufCnt * m_nIOBlkSize bytes space is reserved for closing files.
        if (fileStat.f_bavail * fileStat.f_bsize / 1024
            <= (SD_RESERVED_SPACE) / 1024) {
            return false;
        } else {
            return true;
        }
    }
}

bool commandlistening::isChipMode() {
    char chiptest[256];
    property_get(PROP_CHIPTEST_ENABLE,chiptest,"0");
    LOGD("cheptest mode = %s",chiptest);
    if (!strncmp("1",chiptest,strlen("1"))) {
        return true;
    }
    return false;
}

bool commandlistening::initDataFolder(){
    bool ret = makeDir("/data/debuglogger");
    LOGD("initDataFolder makeDir ret = %d",ret);
/*
    if(ret) {
        if (selinux_android_restorecon("/data/debuglogger", 0) == -1) {
            ALOGE("Netdiag restorecon /data/debuglogger fail. %s",strerror(errno));
            return false;
        }
    }*/
    return ret;
}

void commandlistening::runChipmode(){

    char command_name[128] = "tcpdump";
    char action[128] = "start";
    char address[128] = "200";
    strncpy(location, "sdcard", sizeof(location) - 1);
    location[sizeof(location) - 1] = '\0';
    strncpy(mStoragePath, VZW_VALUE_EXTERNAL_SD_PATH, sizeof(mStoragePath) - 1);
    mStoragePath[sizeof(mStoragePath) - 1] = '\0';
    while(!isSDCardReady()) {
        LOGD("Netdiag wait for sdcard ready");
        sleep(3);
    }
    LOGD("SD card ready");
    commandcollection::iterator it;
    for (it = mcommands->begin(); it != mcommands->end(); ++it) {
        command *cc = *it;
        if (strcmp(cc->getcommand(), command_name) == 0) {
            cc->runcommand(0, location, action, address);
            break;
        }

        LOGE("cc->getcommand() is %s,Location is %s", cc->getcommand(),
                location);

    }
}

void commandlistening::runListener() {
    LOGD("commandlistening runListener doing\n");

    int max = 0;
    int rc = 0;
    int i = 0;
//    struct timeval tv;
    struct stat buf_size;
    //socketclients::iterator it;

    LOGD("Set net log running flag to 0 at the very beginning");
    property_set(_KEY_FLAG_LOG_RUNNING, "0");

    if (isChipMode()) {
        LOGD("Enter Chip Mode");
        runChipmode();
        return;
    }

    while (1) {

        FD_ZERO(&read_fds);
        if (mlisten) {
            max = sock_listen_id;
            LOGD("Add listener on socket server whose id=%d", sock_listen_id);
            FD_SET(sock_listen_id, &read_fds);
        }
        pthread_mutex_lock(&mlock);

        for (i = 0; i < 10; i++) {
            if (client[i].fd > 0) {
                LOGD("Add listener on former received socket client, fd=%d",
                        client[i].fd);
                FD_SET(client[i].fd, &read_fds);
                if (client[i].fd > max) {
                    max = client[i].fd;
                }
            }
        }

        pthread_mutex_unlock(&mlock);
        LOGD("commandlistening unlock exit\n");
        if ((rc = select(max + 1, &read_fds, NULL, NULL, NULL)) < 0) {
            //Since errno may change at any time or may only be accessed just once, cache it here
            int cachedErrorNo = errno;
            LOGD("select failed (%s), errno=(%d), for compare, EINTR=(%d)",
                    strerror(cachedErrorNo), cachedErrorNo, EINTR);
            if (cachedErrorNo == EINTR) {
                LOGD("Ignore system interrupt event.");
                sleep(1);
                continue;
            }
            LOGE(
                    "Unexpected error happen when select(), errno=%d (%s), close all connected fd.",
                    errno, strerror(errno));
            for (i = 0; i < 10; i++) {
                if (client[i].fd > 0) {
                    fstat(client[i].fd, &buf_size);
                    LOGD("select fd wrong, client fd:%d, total buffer size:%ld",
                            client[i].fd, (long)buf_size.st_size);
                    close(client[i].fd);
                    client[i].fd = -1;
                } else {
                    break;
                }
            }
            //close(sock_listen_id);
            sleep(1);
            continue;
        } else if (!rc) {
            LOGE("select timeout");
            continue;
        }
        if (mlisten && FD_ISSET(sock_listen_id, &read_fds)) {
            LOGD("commandlistening sock_listen_id:%d has some read event ",
                    sock_listen_id);
            struct sockaddr addr;
            socklen_t alen = sizeof(addr);
            int connectfd;

            if ((connectfd = accept(sock_listen_id, &addr, &alen)) < 0) {
                LOGE("accept failed (%s)", strerror(errno));
                sleep(1);
                continue;
            }
            pthread_mutex_lock(&mlock);
            LOGD("select out for sock_listen_id:%u,connectfd:%u ",
                    sock_listen_id, connectfd);
            //mclients->push_back(new socketclients(connectfd));
            for (i = 0; i < 10; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connectfd;
                    client[i].addr = addr;
                    LOGD(
                            "[CommandListening] Change current connected FD from %d to %d ",
                            global_fd, connectfd);
                    global_fd = connectfd;
                    break;
                }
            }
            pthread_mutex_unlock(&mlock);
        }

        do {
            pthread_mutex_lock(&mlock);

            for (i = 0; i < 10; i++) {
                int fd = client[i].fd;
                if (fd < 0) {
                    continue;
                }
                if (FD_ISSET(fd, &read_fds)) {
                    LOGE("select out for fd=%d,i=%d  ", fd, i);
                    if (!handle_message(fd)) {
                        if (global_fd == fd) {
                            LOGE(
                                    "read message from current fd[%d] fail, reset it.",
                                    fd);
                            global_fd = -1;
                        }
                        close(fd);
                        LOGE("close  fd=%d,i:%d", fd, i);
                        //FD_CLR(fd , &read_fds);
                        client[i].fd = -1;
                        memset((void *) &(client[i].addr), 0,
                                sizeof(struct sockaddr));
                    }
                    LOGE("FD_CLR fd=%d,index i:%d", fd, i);
                    FD_CLR(fd, &read_fds);
                    continue;
                }
            }

            pthread_mutex_unlock(&mlock);

        } while (0);
    }
}

commandlistening::runshell_cmd::runshell_cmd() :
        command("runshell", -1) {

}

void commandlistening::runshell_cmd::runcommand(int fd, char *location,
        char *action, char *address) {
    LOGD(" runshell_cmd::runcommand: fd = %d, local = %s", fd, location);
    pid_t pid;
    char * command = address;
    char buff[4] = "ro"; //r:run shell command ;o:OK
    int ret = 0, ret_kill = -1;
//    int status = 0;
    /*user mode not process shell command*/
    char build_type[256] = { 0 };
//    signal(SIGCHLD,SIG_IGN);
//    signal(SIGINT,SIG_IGN);
    property_get("ro.build.type", build_type, "eng");
    LOGD("Load build type: %s", build_type);
    if (strcmp(action, "start") == 0) {
        LOGD("run shell command starting, command=[%s]", command);
        /*feature option*/
        if (strcmp(command, "Log2sd") == 0 || strcmp(command, "log2sd") == 0) {
            if (property_set(_KEY_CONFIG_LOG_PATH, _VALUE_CONFIG_LOG_PATH_SD)
                    == 0) { //log must be located in external sdcard
                buff[1] = 'o';
                LOGE("Set log saving path to external SD successfully");
            } else {
                buff[1] = 'w';
                LOGE("Set log saving path to  external SD failed");
            }
            sleep(1);
            sync();
            LOGD("Adb shell command:%s has been finish, log path config key=%s",
                    command, _KEY_CONFIG_LOG_PATH);
            if (fd) {
                ret = write(fd, buff, strlen(buff));
            }
            return;
        } else if (strcmp(command, "Log2emmc") == 0
                || strcmp(command, "log2emmc") == 0) {
            if (property_set(_KEY_CONFIG_LOG_PATH, _VALUE_CONFIG_LOG_PATH_EMMC)
                    == 0) { //log must be located in internal sdcard
                LOGE("Set log saving path to internal SD successfully");
                buff[1] = 'o';
            } else {
                buff[1] = 'w';
                LOGE("Set log saving path to internal SD failed");
            }
            sleep(1);
            sync();
            if (fd) {
                ret = write(fd, buff, strlen(buff));
            }
            LOGD("Adb shell command:%s has been finish, log path config key=%s",
                    command, _KEY_CONFIG_LOG_PATH);
            return;
        }
        if (strstr(command, "setprop") != NULL) { //set property command, execute it directly
            if (handle_setprop_command(command)) {
                LOGI(
                        "setprop phase out, return true directly.");
                buff[1] = 'o';
                if (fd) {
                    ret = write(fd, buff, strlen(buff));
                }
                return;
            }
        }
        /*feature option*/
        if ((pid = fork()) < 0) {
            LOGE("fork failed %s", strerror(errno));
            buff[1] = 'w';
            if (fd) {
                ret = write(fd, buff, strlen(buff));
                LOGD("run shell command failed,write to connection %s ", buff);
            }
            return;
        }

        if (!pid) {
            setpgrp();
            LOGD("getpgrop:%d ", getpgrp());
            if (command == NULL)
                buff[1] = 'w';
            //LOGD("execl ret_execl:%s ",execl("/system/bin/sh", "sh", "-c", command, (char *)0));
            // int ret_execl = execl("/system/bin/sh", "sh", "-c", command, (char *)0);
            //LOGD("execl ret_execl:%d errno:%d",ret_execl,errno);
            //system(command);

            if (shell(command) == 0) {
                LOGD("run shell command return true directly.");
                buff[1] = 'o';
            } else {
                buff[1] = 'w';

            }
            if (fd) {
                ret = write(fd, buff, strlen(buff));
                LOGD("run shell command successfully,write to connection %s ",
                        buff);
            }
            /*kill oneself*/
            daemon_pid = getpid();
            if (daemon_pid != -1) {
                LOGE("kill run_shell_command pid = %d", daemon_pid);
                char command_killgrp_chld[32] = { 0 };
                sprintf(command_killgrp_chld, "kill -15 -%d", daemon_pid);
                LOGE("command_killgrp:%s", command_killgrp_chld);
                system(command_killgrp_chld);
                LOGE("kill -TERM processgroup successfully");
            }
            //return;
        } else {
            daemon_pid = pid;
            LOGE("run shell command service running pid=%d", daemon_pid);
            /*while(waitpid(0, &status, 0) < 0)
             {
             if(errno != EINTR)
             {
             status = -1;
             break;
             }
             } */

        }
    } else if (strcmp(action, "stop") == 0) {
        LOGE("run shell command ending");
        buff[0] = 's';
        if (fd) {
            ret = write(fd, buff, strlen(buff));
            LOGD("kill run shell command return, write to connection %s ",
                    buff);
        }
        if (daemon_pid != -1) {
            LOGE("kill run_shell_command pid = %d", daemon_pid);
            /*if(fp_runshell)
             pclose(fp_runshell);*/
            char command_killgrp[32] = { 0 };
            sprintf(command_killgrp, "kill -15 -%d", daemon_pid);
            LOGE("command_killgrp:%s", command_killgrp);
            system(command_killgrp);
            LOGE("kill -TERM processgroup successfully");
            /*ret_kill = kill(pgroup_id_neg , SIGKILL);
             if((ret_kill == 0)||((ret_kill == -1)&&(errno==ESRCH)))
             {
             buff[1]='o';
             }
             else
             buff[1]='w';*/
            ret_kill = kill(daemon_pid, SIGKILL);
            if ((ret_kill == 0) || ((ret_kill == -1) && (errno == ESRCH))) {
                buff[1] = 'o';
            } else
                buff[1] = 'w';

            return;
        } else {
            LOGE("daemon_pid = -1");
            buff[1] = 'o';
            if (fd) {
                ret = write(fd, buff, strlen(buff));
                LOGD(
                        " write to connection %s ,There is no run shell command running",
                        buff);
            }
            return;
        }
    }
    return;
}

commandlistening::clear_cmd::clear_cmd() :
        command("clear", -1) {

}

void commandlistening::clear_cmd::runcommand(int fd, char *location,
        char *action, char *address) {
    LOGD("clear command starting, address = %s", address);
//    char buff[4] = "co"; //c:clear log command ;o:OK
    if (strcmp(action, "start") == 0) {
        LOGE("clear command starting");
        clear_begin(fd, location);

    } else {
        LOGE("clear parameters failed");
        _exit(-1);
    }

    return;
}

commandlistening::kill_cmd::kill_cmd() :
        command("kill", -1) {

}

void commandlistening::kill_cmd::runcommand(int fd, char *location,
        char *action, char *address) {
    LOGD("kill_cmd::runcommand, fd = %d, location = %s, address = %s", fd, location, address);
    if (strcmp(action, "start") == 0) {
        LOGE("kill the netdiag for restart command");
        kill(getpid(), SIGKILL);
    }
    return;
}

commandlistening::tcpdump_cmd::tcpdump_cmd() :
        command("tcpdump", -1) {

}
void netdiag_get_aid_and_cap() {
    LOGD("Warning: gid:%d,uid:%d,pid:%d !\n", getgid(), getuid(), getpid());
//    struct __user_cap_header_struct header;
//    struct __user_cap_data_struct cap;
//    header.version = _LINUX_CAPABILITY_VERSION;
//    header.pid = getpid();
//    capget(&header, &cap);
//    LOGD("Warning: permitted:%x,cap.effective:%x !\n",cap.permitted,cap.effective);
}
#define BUFSZ_RET 4
//Flag for stopping log process finished or not
int stop_finish = 0;
pid_t stop_test_pid;
pid_t stop_tcpdump_pid;

void commandlistening::tcpdump_cmd::runcommand(int fd, char *location,
        char *action, char *address) {
    pid_t pid;
//    char tcpdump_name[16] = "tcpdump";
    char buffer[BUFSZ_RET] = "to"; //command has received
    char NTLog_name_arr[256];
    int ret = 0, ret_kill = -1;
//    int kill_ret;
    time_t t;
    struct tm *timenow;
//    int found_no = 0, j_run = 0;

    //Register a child process exit receiver, when stop test process finish, reset waiting flag in main process
    struct sigaction act, oact;
    act.sa_handler = cmd_exec_sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGCHLD, &act, &oact) < 0) {
        LOGE("sigaction in tcpdump_cmd failed");
        return;
    }

    //At operating log time, client side may already been closed, so the fd will not be writable
    // need to add error handler on this, since this signal will default quite process and generate NE
    LOGD("Ignore SIGPIPE to avoid possible NE");
    signal(SIGPIPE, SIG_IGN);

    if (strcmp(action, "stop") == 0) {
        buffer[0] = 'p';
        LOGE("trace ENDING:2019-07-26");
        char log_running_flag[256];
        property_get(_KEY_FLAG_LOG_RUNNING, log_running_flag, "0");
        if (strcmp(log_running_flag, "0") == 0) {
            LOGE("Stop flag is still 0, treat it as log is already stop");
            buffer[1] = 'o';
            if (fd) {
            	char resultBuffer[128] = "tcpdump_sdcard_stop";
            	        	int success = 1;
            	        	if (buffer[1] != 'o') {
            	        		success = 0;
            	        	}
            	        	if (address != NULL && strlen(address) > 0) {
            	        		sprintf(resultBuffer, "tcpdump_sdcard_stop_%s,%d", address, success);
            	        	} else {
            	        		sprintf(resultBuffer, "tcpdump_sdcard_stop,%d", success);
            	        	}
            	        	ret = write(fd, resultBuffer, strlen(resultBuffer));
            	        	LOGD("run trace command, write to connection %s", resultBuffer);
                LOGD("Network log is already stop, just return success");
            }
            return;
        }
        property_set(_KEY_FLAG_LOG_BREAKLOOP, "1"); //Breakloop:1,break loop and exit

        property_get(_KEY_LOG_SAVING_PATH, NTLog_name_arr, "");
        LOGD("checkdir stop NTLog_name:%s commandlistening full_test",
                NTLog_name_arr);
//        full_test(location,action,NTLog_name_arr);

        //put stop check operation into a separate process and wait at most TIMEOUT_STOP second for its finish.
        pid_t pid_stop;
        stop_finish = 0;
        stop_test_pid = 0;
        stop_tcpdump_pid = 0;
        if ((pid_stop = fork()) < 0) {
            LOGE("fork stop test process failed %s, cancel full_test.",
                    strerror(errno));
        } else {
            if (!pid_stop) {
                LOGD(" full_test in sub-process begin.");
                LOGD(" At stop time, parameter address=%s", address);
                full_test(location, action, NTLog_name_arr, address);
                LOGD(" full_test in sub-process finish.");
                _exit(127);
            } else {
                stop_test_pid = pid_stop;
                if ((daemon_pid != -1) && (daemon_pid != 0)) {
                    LOGE("kill trace pid = %d", daemon_pid);
                    stop_tcpdump_pid = daemon_pid;
                    ret_kill = kill(daemon_pid, SIGTERM);
                    if ((ret_kill == 0) || ((ret_kill == -1) && (errno == ESRCH))) {
                         buffer[1] = 'o';
                    } else
                        buffer[1] = 'w';
                } else {
                    LOGE("daemon_pid = -1");
                }
                remaining_time = TIMEOUT_STOP;
                stop_tcpdump(false);
                LOGI(" At stop time,kill trace pid");
                kill(stop_tcpdump_pid, SIGTERM);
                while (!stop_finish && (remaining_time > 0)) {
                    sleep(1);
                    LOGD(
                            " Waiting stop process finish, stop_finish = %i, remaining_time=%i",
                            stop_finish, remaining_time);
                    remaining_time--;
                }
                if (!stop_finish && (remaining_time <= 0)) {
                    LOGE("Stop process timeout, kill it directly.");
                    if (stop_test_pid != -1 && stop_test_pid != 0) {
                        int ret_force_stop = kill(stop_test_pid, SIGKILL);
                        LOGE(
                                "Force stop process test_pid(%d) terminate, return value=%i",
                                stop_test_pid, ret_force_stop);
                    }
                    if (stop_tcpdump_pid != -1 && stop_tcpdump_pid != 0) {
                        int ret_force_stop = kill(stop_tcpdump_pid, SIGKILL);
                        LOGE(
                                "Force stop process tcpdump_pid(%d) terminate, return value=%i",
                                stop_tcpdump_pid, ret_force_stop);
                    }
                }
            }
        }

        LOGD("At stop time, reset log running flag to 0");
        int retValue = property_set(_KEY_FLAG_LOG_RUNNING, "0");
        if (retValue < 0) {
            LOGE("At stop time, property_set reset log running flag to 0 failed. Return value %d", retValue);
        } else {

            char log_running_flag[256];
            int mRetryCount = 150;
            while (mRetryCount-- > 0) {
                property_get(_KEY_FLAG_LOG_RUNNING, log_running_flag, "0");
                if (strcmp(log_running_flag, "1") == 0) {
                    LOGE("At stop time, property_get log running flag and wait for 0. retry time %d", mRetryCount);
                    usleep(10000);
                } else {
                    LOGD("At stop time, property_get reset log running flag to 0 ok");
                    break;
                }

            }
            

        }
        if (fd) {
        	char resultBuffer[128] = "tcpdump_sdcard_stop";
        	int success = 1;
        	if (buffer[1] != 'o') {
        		success = 0;
        	}
        	if (address != NULL && strlen(address) > 0) {
        		sprintf(resultBuffer, "tcpdump_sdcard_stop_%s,%d", address, success);
        	} else {
        		sprintf(resultBuffer, "tcpdump_sdcard_stop,%d", success);
        	}
        	ret = write(fd, resultBuffer, strlen(resultBuffer));
        	LOGD("run trace command, write to connection %s", resultBuffer);
        }

        /*if(tcpdump_pid_arr )
         free(tcpdump_pid_arr );*/
        return;

    } else if (strcmp(action, "start") == 0) {
        LOGE("trace STARTING:2019-07-26");
        char log_running_flag[256];
        property_get(_KEY_FLAG_LOG_RUNNING, log_running_flag, "0");
        if (strcmp(log_running_flag, "1") == 0) {
            LOGE(
                    "Stop flag is still 1, treat it as log is already running, ignore other start command.");
            buffer[1] = 'o';
            if (fd) {
            	char resultBuffer[128] = "tcpdump_sdcard_start_";
            	int success = 1;
            	if (buffer[1] != 'o') {
            	    success = 0;
            	}
            	sprintf(resultBuffer, "tcpdump_sdcard_start_%s,%d", address, success);
            	ret = write(fd, resultBuffer, strlen(resultBuffer));
            	LOGD("run trace command, write to connection %s", resultBuffer);
                LOGD("Network log is already running, just return success");
            }
            return;
        }

        property_set(_KEY_FLAG_LOG_BREAKLOOP, "0"); //Breakloop:1,break loop and exit
        time(&t);
        timenow = localtime(&t);
        strftime(NTLog_name_arr, 100, "NTLog_%Y_%m%d_%H%M%S", timenow);
        LOGD("checkdir start NTLog_name:%s commandlistening", NTLog_name_arr);

        if ((pid = fork()) < 0) {
            LOGE("fork failed %s", strerror(errno));
            buffer[1] = 'w';
            if (fd) {
                ret = write(fd, buffer, strlen(buffer));
                LOGD("run start trace command failed,write to connection %s ",
                        buffer);
            }
            LOGD("Fork fail, reset log running flag to 0");
            property_set(_KEY_FLAG_LOG_RUNNING, "0");
            _exit(-1);
        }

        char stop_reason[256] = { 0 };
        property_set(_KEY_LOG_STOP_REASON, "");
        property_get(_KEY_LOG_STOP_REASON, stop_reason, "");
        LOGD(" At start time, stop reason=%s.\n", stop_reason);
        if (strlen(stop_reason) > 0) {
            LOGD(" At start time, reset stop reason to empty.");
            property_set(_KEY_LOG_STOP_REASON, "");
        }

        if (!pid) {
            tcpdump_data(fd, location, NTLog_name_arr, address);
            _exit(127);
        } else {
            daemon_pid = pid;
            LOGD("trace service running pid=%d", daemon_pid);
        }
    } else if (strcmp(action, "check") == 0) {
        LOGE("trace CHECKING");
        property_set(_KEY_FLAG_LOG_BREAKLOOP, "1"); //Breakloop:1,break loop and exit
        buffer[0] = 'k';
        stop_finish = 0;
        stop_tcpdump_pid = 0;

        if ((daemon_pid != -1) && (daemon_pid != 0)) {
            LOGE("kill trace pid = %d", daemon_pid);
            stop_tcpdump_pid = daemon_pid;
            ret_kill = kill(daemon_pid, SIGTERM);
            if ((ret_kill == 0) || ((ret_kill == -1) && (errno == ESRCH))) {
                buffer[1] = 'o';
            } else
                buffer[1] = 'w';
        } else {
            LOGE("daemon_pid = -1");
        }
        remaining_time = TIMEOUT_STOP;
        stop_tcpdump(false);
        while (!stop_finish && (remaining_time > 0)) {
           sleep(1);
           LOGD(
                " Waiting stop process finish, stop_finish = %i, remaining_time=%i",
                stop_finish, remaining_time);
                    remaining_time--;
        }
        if (!stop_finish && (remaining_time <= 0)) {
           LOGE("Stop process timeout, kill it directly.");
           if (stop_tcpdump_pid != -1 && stop_tcpdump_pid != 0) {
                int ret_force_stop = kill(stop_tcpdump_pid, SIGTERM);
                LOGE(
                     "Force stop process(%d) terminate, return value=%i",
                     stop_tcpdump_pid, ret_force_stop);
             }
        }
        if (fd) {
            ret = write(fd, buffer, strlen(buffer));
            LOGD("run stop trace check command, write to connection %s,'o'is ok,'w'is wrong",
                    buffer);
        }
        LOGD("At check time, reset log running flag to 0");
        property_set(_KEY_FLAG_LOG_RUNNING, "0");

        return;
    } else {
        buffer[1] = 'w';
        if (fd) {
            ret = write(fd, buffer, strlen(buffer));
            LOGD("run trace command failed,write to connection %s ", buffer);
        }

        LOGE("trace parameters failed");
        _exit(-1);
    }

    return;

}

bool commandlistening::handle_message(int fd) {

    //int fd = client->getsocket();
    commandcollection::iterator it;

    LOGE("commandlistening handle_message");
    char buffer[128];
    char cmdAck[256];
    char temp[256];
    memset(buffer, '\0', sizeof(buffer));
    int len;
    int tcpdump_start_add = 1;
    if ((len = read(fd, buffer, sizeof(buffer) - 1)) < 0) {
        LOGE("read() failed (%s)", strerror(errno));
        return false;
    } else if (!len)
        return false;

    buffer[len - 1] = '\0';
    LOGE("commandlistening buffer %s", buffer);

    if (!strncmp(buffer, mStrSetPath, strlen(mStrSetPath))) {
        memset(mStoragePath, '\0', 128);
        if (!mTestSaveLogInData) {
            strncpy(mStoragePath, &buffer[strlen(mStrSetPath)], sizeof(mStoragePath) - 1);
        } else {
            strncpy(mStoragePath, "/data", sizeof(mStoragePath) - 1);
        }
        mStoragePath[sizeof(mStoragePath) - 1] = '\0';
        mEnableSaveLogInData = false;
        if (0 == strncmp(mStoragePath, "/data", strlen("/data"))) {
            mEnableSaveLogInData = true;
            initDataFolder();
        } else if (access(mStoragePath, F_OK) == -1) {
            if (makeDir(mStoragePath) != -1) {
                LOGD("Create log folder success : %s", mStoragePath);
            } else {
                LOGE("Create log folder fail : %s", mStoragePath);
            }
        }
        if ('/' == *(mStoragePath + strlen(mStoragePath) - 1)) {
            LOGD("find seperator in path : %s", mStoragePath);
            *(mStoragePath + strlen(mStoragePath) - 1) = '\0';
            LOGD("move seperator sepretor in path : %s", mStoragePath);
        }
        return true;
    } else if (!strncmp(buffer, IS_ROHC_SUPPORT, strlen(IS_ROHC_SUPPORT))) {
        memset(cmdAck, '\0', sizeof(cmdAck));
        sprintf(cmdAck, "%s,%d", buffer, mIsSupportROHC);
        write(fd, cmdAck, strlen(cmdAck));
        LOGI("Netdiag send ACK %s", cmdAck);
        return true;
    } else if (!strncmp(buffer, ENABLE_ROHC, strlen(ENABLE_ROHC))) {
        memset(temp, '\0', 256);
        memset(cmdAck, '\0', sizeof(cmdAck));
        strncpy(temp, &buffer[strlen(ENABLE_ROHC)], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        if (atoi(temp) >= 0) {
            apply_rohc_compress = atoi(temp);
            sprintf(cmdAck, "%s,1", buffer);
        } else {
            LOGE("Netdiag atoi fail %s", temp);
            sprintf(cmdAck, "%s,0", buffer);
        }
        write(fd, cmdAck, strlen(cmdAck));
        LOGI("Netdiag send ACK %s.apply_rohc_compress = %d", cmdAck, apply_rohc_compress);
        return true;
    } else if (!strncmp(buffer, SET_ROHC_TOTAL_FILE, strlen(SET_ROHC_TOTAL_FILE))) {
        memset(temp, '\0', 256);
        memset(cmdAck, '\0', sizeof(cmdAck));
        strncpy(temp, &buffer[strlen(SET_ROHC_TOTAL_FILE)], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        if (atoi(temp) > 0) {
            rohc_total_file = atoi(temp);
            sprintf(cmdAck, "%s,1", buffer);
        } else {
            LOGE("Netdiag atoi fail %s", temp);
            sprintf(cmdAck, "%s,0", buffer);
        }
        write(fd, cmdAck, strlen(cmdAck));
        LOGI("Netdiag send ACK %s.rohc_total_file = %d", cmdAck, rohc_total_file);
        return true;
    }
//////////////////////////////////////////////////////////////////////////////////// for parse the command
    char *c = &buffer[0];

    char command_name[128];

    char action[128];
    char address[128];
//    int kill_ret;
    int i = 0;
    char name[128];

    while ((*c != '_') && (i < len)) {
        if (*c == '\0') {
            LOGE("Wrong command position 1 and return true for not close socket fd");
            return true;
        }
        name[i] = *c;
        i++;
        c++;
    }
    if (i == len)/*wrong command,eg,xx(without'_')*/
        return false;
    c++;
    name[i] = '\0';
    strncpy(command_name, name, sizeof(command_name) - 1);
    command_name[sizeof(command_name) - 1] = '\0';
    memset(name, 0, sizeof(name));
    i = 0;
    LOGE("COMMAND = %s", command_name);

    if (strcmp(command_name, "runshell") == 0) //runshell_command_start_*****
            {
        pthread_mutex_lock(&mlock_location);
        while ((*c != '_') && ((unsigned long)i < (len - strlen(command_name)))) {
            if (*c == '\0') {
                pthread_mutex_unlock(&mlock_location);
                LOGE("Wrong command position 2 and return true for not close socket fd");
                return true;
            }
            name[i] = *c;
            i++;
            c++;
        }
        if ((unsigned long)i == (len - strlen(command_name)))/*wrong command,eg,runshell_xx(without'_')*/
        {
            pthread_mutex_unlock(&mlock_location);
            LOGE("Wrong command position 3 and return true for not close socket fd");
            return true;
        }
        c++;
        name[i] = '\0';
        strncpy(location, name, sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
        //commandlistening::setlocation(location);
        pthread_mutex_unlock(&mlock_location);
        memset(name, 0, sizeof(name));
        i = 0;
        LOGE("location = %s", location);
        while ((*c != '_')
                && ((unsigned long)i < (len - strlen(command_name) - strlen(location)))) {
            if (*c == '\0') {
                LOGE("Wrong command position 4 and return true for not close socket fd");
                return true;
            }
            name[i] = *c;
            i++;
            c++;
        }
        if ((unsigned long)i == (len - strlen(command_name) - strlen(location)))/*wrong command,eg,runshell_command_xx(without'_')*/
        {
            LOGE("Wrong command position 5 and return true for not close socket fd");
            return true;
        }
        name[i] = '\0';
        strncpy(action, name, sizeof(action) - 1);
        action[sizeof(action) - 1] = '\0';
        memset(name, 0, sizeof(name));
        LOGE("action = %s", action);
        i = 0;
        c++;
        memset(address, 0, sizeof(address));
        while ((*c != '\0')
                && ((unsigned long)i
                        < (len - strlen(command_name) - strlen(location)
                                - strlen(action)))) {
            name[i] = *c;
            i++;
            c++;
        }
        name[i] = '\0';
        strncpy(address, name, sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
        memset(name, 0, sizeof(name));
        i = 0;
        LOGE("command= %s", address);

    } else {
        pthread_mutex_lock(&mlock_location);
        while ((*c != '_') && ((unsigned long)i < (len - strlen(command_name)))) {
            if (*c == '\0') {
                pthread_mutex_unlock(&mlock_location);
                LOGE("Wrong command position 6 and return true for not close socket fd");
                return true;
            }
            name[i] = *c;
            i++;
            c++;
        }
        if ((unsigned long)i == (len - strlen(command_name)))/*wrong command,eg,xx_xx(without'_')*/
        {
            pthread_mutex_unlock(&mlock_location);
            LOGE("Wrong command position 7 and return true for not close socket fd");
            return true;
        }
        c++;
        name[i] = '\0';
        strncpy(location, name, sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
        //commandlistening::setlocation(location);
        pthread_mutex_unlock(&mlock_location);
        memset(name, 0, sizeof(name));
        i = 0;
        LOGE("location = %s", location);
        while (*c != '_') {
            if (*c == '\0') {
                tcpdump_start_add = 0;
                break;
            }
            name[i] = *c;
            i++;
            c++;
        }
        c++;
        name[i] = '\0';
        strncpy(action, name, sizeof(action) - 1);
        action[sizeof(action) - 1] = '\0';
        memset(name, 0, sizeof(name));
        i = 0;
        LOGE("action = %s", action);
        memset(address, 0, sizeof(address));
        if (tcpdump_start_add == 1) {
            while (*c != '\0') {
                name[i] = *c;
                i++;
                c++;
            }
            name[i] = '\0';
            strncpy(address, name, sizeof(address) - 1);
            address[sizeof(address) - 1] = '\0';
            memset(name, 0, sizeof(name));
            i = 0;
            LOGE("log size= %s", address);
        }
    }
    if ((strcmp(action, "start") == 0)
            && (strcmp(command_name, "tcpdump") == 0)) {

    } else if ((strcmp(action, "stop") == 0)
            && (strcmp(command_name, "tcpdump") == 0)) {

    }

///////////////////////////////////////////////////////////////////////////////////////////////

    for (it = mcommands->begin(); it != mcommands->end(); ++it) {
        command *cc = *it;
        if (strcmp(cc->getcommand(), command_name) == 0) {
            cc->runcommand(fd, location, action, address);
            break;
        }

        LOGE("cc->getcommand() is %s,Location is %s", cc->getcommand(),
                location);

    }
    /*if((strcmp(action , "stop") == 0)&&(strcmp(command_name,"tcpdump")==0))
     return false;
     else*/
    return true;

}

/**
 * Some command may need to be executed in sub process, use this to monitor command execute finished signal
 */
void cmd_exec_sig_handler(int s) {
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        char stop_reason[256] = { 0 };
        property_get(_KEY_LOG_STOP_REASON, stop_reason, "");
        LOGE(" At stop time, singal = %d, stop reason=%s.\n", s, stop_reason);
        LOGE(
                "[CommandListening] child %d terminated. Receive signal in commandlistening. "
                        "Sub thread stop_reason=%s\n", pid, stop_reason);
        if (global_fd >= 0 && strlen(stop_reason) > 0) {
            write(global_fd, stop_reason, strlen(stop_reason));
            LOGE(
                    "[CommandListening] Write response to client side, fd=%d, message=%s and reset flag\n",
                    global_fd, stop_reason);
            property_set(_KEY_LOG_STOP_REASON, "");
        }
        if (stop_test_pid == pid) {
            LOGD(
                    "Receive stop_test process exit signal, stop waiting in main process.");
            stop_test_pid = 0;
        }
        if (stop_tcpdump_pid == pid) {
            LOGD("Receive trace process exit signal, stop waiting in main process.");
            stop_tcpdump_pid = 0;
        }
        if (stop_tcpdump_pid == 0 && stop_test_pid == 0) {
           stop_finish = 1;
        }
        return;
    }
}

/**
 * Execute setprop command, by call property_set directly,
 * command should look like "setprop ** **"
 */
bool handle_setprop_command(char* cmd) {
    LOGI("-->handle_setprop_command(), cmd=%s", cmd);
// Remove and phase out this feature for security problem.
/*

    memset(initCmd, 0, sizeof(initCmd));
    memset(command, 0, sizeof(command));
    memset(key, 0, sizeof(key));
    memset(value, 0, sizeof(value));

    strncpy(initCmd, cmd);
    int length = strlen(initCmd);
    int index = 0;
    //parse setprop command head
    while (initCmd[index] == ' ' && index < length) { //Ignore white space at the begin
        index++;
    }
    int i = 0;
    while (index < length && initCmd[index] != ' ' && i < 20) {
        command[i++] = initCmd[index++];
    }
    if (index >= length || i >= 20) {
        LOGE("Wrong setprop command format, maybe not start with setprop");
        return false;
    }
    LOGI(" --command=%s, index=%d", command, index);
    //parse key string
    while (initCmd[index] == ' ' && index < length) { //Ignore white space, maybe more than one
        index++;
    }
    i = 0;
    while (index < length && initCmd[index] != ' ' && i < 100) {
        key[i++] = initCmd[index++];
    }
    if (index >= length || i >= 100) {
        LOGE("Wrong setprop command format, maybe key too long");
        return false;
    }
    LOGI(" --key=%s, index=%d", key, index);

    //parse value string
    while (initCmd[index] == ' ' && index < length) { //Ignore white space, maybe more than one
        index++;
    }
    i = 0;
    while (index < length && initCmd[index] != ' ' && i < 100) {
        value[i++] = initCmd[index++];
    }
    if (i == 0 || i >= 100) {
        LOGE("Wrong setprop command format, maybe no value or value too long");
        return false;
    }

    LOGI(" parse command finished, cmd=%s, key=%s, value=%s", command, key,
            value);
    property_set(key, value);*/
    return true;
}

