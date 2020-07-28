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
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cutils/log.h>
#include <cutils/sockets.h>

#include <cutils/properties.h>
#include <sys/system_properties.h>

#include "../utils.h"

#define MAX 128
#define MAX_BUFFER 1024

extern int shell(char * cmd);
extern int check_dir(char *root, const char *target);

#define _PATH_PROCNET_ROUTE "/proc/net/route"
#define _PATH_PROCNET_DEV "/proc/net/dev"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DIR_MODE (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
#define DATA_FILE_MODE (S_IRWXU | S_IRWXG)

#define maxfd 10

typedef struct {
    int fd;
    pid_t pid;
} fd_pid;

fd_pid * childpid = NULL;
static int num_file = 0;
char buffer_read[MAX_BUFFER] = { 0 };

void sig_handler_test(int s);

static void read_write(char *root, int fd) {
    int len = strlen(root);
    char newbuffer[16] = { 0 };
    int from_fd;
    int bytes_read, bytes_write;
    char *ptr = NULL;
    struct stat statbuf;

    snprintf(newbuffer, 16, "[%d]:", num_file);
    write(fd, newbuffer, strlen(newbuffer));
    write(fd, root, len);
    write(fd, ":\n", strlen(":\n"));
    stat(root, &statbuf);

    if (statbuf.st_mode & S_IRUSR) {
        if ((from_fd = open(root, O_RDONLY)) == -1) {
            LOGD("open %s error. error: %d[%s]\n", root, errno, strerror(errno));
            return;
        }

        while ((bytes_read = read(from_fd, buffer_read, MAX_BUFFER))) {
            if ((bytes_read == -1) && (errno != EINTR)) {
                break;
            } else if (bytes_read > 0) {
                ptr = buffer_read;
                while ((bytes_write = write(fd, ptr, bytes_read))) {
                    if ((bytes_write == -1) && (errno != EINTR)) {
                        break;
                    } else if ((bytes_write > 0)
                            && (bytes_read == bytes_write)) {
                        break;
                    } else if ((bytes_write > 0)
                            && (bytes_read != bytes_write)) {
                        ptr += bytes_write;
                        bytes_read -= bytes_write;
                    }
                }
            }

        }
        write(fd, "\n---------------------------------------------\n", strlen("\n---------------------------------------------\n"));
        close(from_fd);
    } else {
        LOGD("path %s :statbuf.st_mode ! S_IRUSR not read",root);
    }
    return;
}

static int dump_proc_net_file(const char *root, int fd) {
    char path[MAX] = {0};
    int result = 0;
    DIR *dir = NULL;
    struct dirent *ptr;
    struct stat statbuf;
    dir = opendir(root);
    if (dir == NULL) {
        LOGE("fail to open dir");
        return -1;
    }
    while ((ptr = readdir(dir)) != NULL) {
        if (strlen(ptr->d_name) + strlen(root) > MAX - 1) {
            LOGE("dump_proc_net_file above MAX");
            continue;
        
        }
        snprintf(path, MAX - 1, "%s/%s", root, ptr->d_name);
        path[MAX - 1] = '\0';
        if (lstat(path, &statbuf) < 0) {
            LOGE("lstat error");
            if (dir) {
                closedir(dir);
            }
            return -1;
        }

        if (S_ISDIR(statbuf.st_mode) && ptr->d_name[0] != '.') { // it is the file,dump information
            result = dump_proc_net_file(path, fd);
            if (-1 == result) {
                LOGE("dump_proc_net_file fail");
               // closedir(dir);
            }
        } else {

            if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
                continue;
            }
            num_file++;
            read_write(path, fd);
        }
    }
    closedir(dir);
    return 0;
}

void full_test(char *local, char *action, char * NTLog_name, char * ping_flag) {

    LOGD("-->full_test,local=%s, log folder=%s, ping_flag=%s", local, NTLog_name, ping_flag);

    int fd, fpout;
    int need_ping_at_stop_time = 1;
    char dump_networking_path[MAX];
    char full_test_path[MAX];
    time_t t;
    struct tm *timenow;
    char net_addr[32];
    char iface[16];
    char real_iface[16];
//    char default_route[32] = "00000000";
//    int loss_rate = 0;
//    int transmitted = 0;
//    int received = 0;
//    int result = 0;
    char location_stop[256];
    memset(dump_networking_path, 0, MAX);
    memset(full_test_path, 0, MAX);
    /*char gateway[32];
     char real_gateway[32];
     memset(gateway,0,32);
     memset(real_gateway,0,32);*/
    memset(net_addr, 0, 32);
    memset(iface, 0, 16);
    memset(real_iface, 0, 16);
    //memset(buff,0,1024);
    //memset(result_buffer,0,1024);
    memset(location_stop, 0, 256);

    if (ping_flag == NULL || strlen(ping_flag) == 0) {
        LOGD("no ping_flag, default ping DNS.");
    } else if (strcmp(ping_flag, "noping") == 0) {
        need_ping_at_stop_time = 0;
        LOGD("No need to ping DNS at stop time");
    } else {
        LOGE("What is this ping_flag? [%s]", ping_flag);
    }

    struct sigaction act, oact;
    act.sa_handler = sig_handler_test;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGCHLD, &act, &oact) < 0) {
        LOGE("sigaction failed");
        return;
        //exit(1);
    }
//    char sdcardpath[16];
//    char dump_networking[16] = "/debuglogger";
//    char full_test_log_path[16] = "/netlog";

    if (access(NTLog_name, 0) == 0) {
        LOGD("folder %s exists", NTLog_name);
    } else {
        LOGD("folder %s does not exists", NTLog_name);
        return;
    }
    strncpy(location_stop, NTLog_name, sizeof(location_stop) - 1);
    location_stop[sizeof(location_stop) - 1] = '\0';
    time(&t);
    timenow = localtime(&t);
    if (strcmp(action, "start") == 0)
        strftime(dump_networking_path, MAX,
                "/dump-networking-%Y_%m%d_%H%M%S_start.txt", timenow);
    else
        strftime(dump_networking_path, MAX,
                "/dump-networking-%Y_%m%d_%H%M%S_stop.txt", timenow);
    //change_name(dump_networking_path);

    strncat(location_stop, dump_networking_path,strlen(dump_networking_path));
    mode_t mode = FILE_MODE;
    if (0 == strncmp(location_stop, "/data", strlen("/data"))) {
        mode = DATA_FILE_MODE;
    }
    fd = open(location_stop, O_CREAT | O_RDWR, mode);
    if (fd == -1) {
        perror("open incorrect");
        LOGE("open location failed");
        return;
        //exit(1);
    }
    int temp_fd = dup(STDOUT_FILENO);
    LOGD("open dump_stop.txt %u,temp_fd:%u ", fd, temp_fd);
    if (temp_fd < 0) {
        LOGE("dup failed");
        close(fd);
        return;
    }
    int temp_fd2 = dup2(fd, STDOUT_FILENO);
    if (temp_fd2 != STDOUT_FILENO) {
        perror("dup2 incorrect");
        LOGE("dup2 failed");
        close(fd);
        close(temp_fd);
        return;
        //exit(0);
    }

    LOGD("after dup2 dump_stop.txt %u,temp_fd:%u,temp_fd2:%u", fd, temp_fd,
            temp_fd2);

    write(fd, "ifconfig:\n", 10);
    //print_ifconfig(fd);
    system("ip addr show");
    write(fd, "----------\n", 11);

    write(fd, "route:\n", 7);
    //print_route(fd);
    system("ip -4 route show table all");
    write(fd, "----------\n", 11);

    write(fd, "ip rule:\n", 10);
    //print_route(fd);
    system("ip rule list");
    write(fd, "----------\n", 11);

    write(fd, "ipv6 rule:\n", 11);
    system("ip -6 rule");
    write(fd, "----------\n", 11);

    write(fd, "ipv6_route:\n", 12);
    system("ip -6 route show table all");
    write(fd, "----------\n", 11);
    write(fd, "iptables -L -nv: filter table\n", strlen("iptables -L -nv: filter table\n"));
    system("iptables -L -nv");
    write(fd, "----------\n", 11);
    write(fd, "iptables -L -t nat: nat table\n", 30);
    system("iptables -L -nv -t nat");
    write(fd, "----------\n", 11);
    write(fd , "iptables -L -t mangle: mangle table\n", 36);
    system("iptables -L -nv -t mangle");
    write(fd , "----------\n" , 11);
    write(fd , "iptables -L -t raw: raw table\n", 30);
    system("iptables -L -nv -t raw");
    write(fd , "----------\n" , 11);

    write(fd, "ip6tables -L -nv: filter table\n", strlen("ip6tables -L -nv: filter table\n"));
    system("ip6tables -L -nv");
    write(fd, "----------\n", 11);
    write(fd, "ip6tables -L -t nat: nat table\n", 31);
    system("ip6tables -L -nv -t nat");
    write(fd, "----------\n", 11);
    write(fd , "ip6tables -L -t mangle: mangle table\n", 37);
    system("ip6tables -L -nv -t mangle");
    write(fd , "----------\n" , 11);
    write(fd , "ip6tables -L -t raw: raw table\n", 31);
    system("ip6tables -L -nv -t raw");
    write(fd , "----------\n" , 11); 

    //add for ip spec
    char build_type[256] = { 0 };
    property_get("ro.build.type", build_type, "user");
    LOGD("Load build type: %s", build_type);
    if (0 == strncmp("userdebug",build_type,strlen("userdebug"))
        || 0 == strncmp("eng",build_type,strlen("eng"))) {
        LOGD("build type not user, should output ip spec rule");
        write(fd , "ip xfrm policy list\n", 20);
        system("ip xfrm policy list");
        write(fd , "----------\n" , 11);
        write(fd , "ip xfrm state list\n", 19);
        system("ip xfrm state list");
        write(fd , "----------\n" , 11);
    }

    write(fd, "netcfg:\n", 8);
    system("netcfg");
    write(fd, "----------\n", 11);

    /////////////////////////////////////////////////////////////  networking information
    write(fd, "connectivity:\n", 14);
    system("dumpsys connectivity");
    write(fd , "----------\n" , 11);
    write(fd, "network_management:\n", strlen("network_management:\n"));
    system("dumpsys network_management");
    write(fd , "----------\n" , 11);
    /*
    write(fd, "netd:\n", strlen("netd:\n"));
    system("dumpsys netd");
    write(fd , "----------\n" , 11);*/
    write(fd, "connmetrics:\n", strlen("connmetrics:\n"));
    system("dumpsys connmetrics");
    write(fd , "----------\n" , 11);

    write(fd, "netstats detail:\n", strlen("netstats detail:\n"));
    system("dumpsys netstats detail");
    write(fd , "----------\n" , 11);
    write(fd, "netpolicy:\n", strlen("netpolicy:\n"));
    system("dumpsys netpolicy");
    write(fd , "----------\n" , 11);
    write(fd, "settings:\n", strlen("settings:\n"));
    system("dumpsys settings");
    write(fd , "----------\n" , 11);

    write(fd, "netd trafficcontroller:\n", strlen("netd trafficcontroller:\n"));
    system("dumpsys netd trafficcontroller");
    write(fd , "----------\n" , 11);

    if (strncmp("user",build_type,strlen("user"))) {
        write(fd, "getprop:\n", 9);
        system("getprop");
        write(fd, "----------\n", 11);
    }
    ///////////////////////////////////////////////////////////////  dump proc file
    num_file = 0;
    dump_proc_net_file("/proc/net", fd);
    dump_proc_net_file("/proc/sys/net/ipv4", fd);
    dump_proc_net_file("/proc/sys/net/ipv6", fd);
    dump_proc_net_file("/proc/sys/net/core", fd);
    dump_proc_net_file("/proc/sys/net/netfilter", fd);

    if (dup2(temp_fd, temp_fd2) < 0) {
        perror("dup2 incorrect");
        LOGE("dup2 failed");
        close(fd);
        close(temp_fd);
        return;
    }
    close(temp_fd);
    if (fd) {
        if (close(fd) == 0)
            LOGD(" close %u successfully,temp_fd:%u,temp_fd2:%u", fd, temp_fd,
                    temp_fd2);
        else
            LOGD(" close %u failed", fd);
    }

    if (need_ping_at_stop_time) {
        memset(location_stop, 0, 256);
        strncpy(location_stop, NTLog_name, sizeof(location_stop) - 1);
        location_stop[sizeof(location_stop) - 1] = '\0';
        time(&t);
        timenow = localtime(&t);
        if (strcmp(action, "start") == 0)
            strftime(full_test_path, MAX, "/ping-%Y_%m%d_%H%M%S_start.txt",
                    timenow);
        else
            strftime(full_test_path, MAX, "/ping-%Y_%m%d_%H%M%S_stop.txt",
                    timenow);

        strncat(location_stop, full_test_path,strlen(full_test_path));
        mode_t mode = FILE_MODE;
        if (0 == strncmp(location_stop, "/data", strlen("/data"))) {
            mode = DATA_FILE_MODE;
        }
        fpout = open(location_stop, O_CREAT | O_RDWR, mode);
        if (fpout == -1) {
            perror("fopen incorrect");
            LOGE("fopen ping.txt failed");
            return;
        }
        temp_fd = dup(STDOUT_FILENO);
        LOGD("open ping_stop.txt %u,temp_fd:%u ", fpout, temp_fd);
        if (temp_fd < 0) {
            LOGE("dup failed");
            close(fpout);
            return;
        }
        temp_fd2 = dup2(fpout, STDOUT_FILENO);
        if (temp_fd2 != STDOUT_FILENO) {
            perror("dup2 incorrect");
            LOGE("dup2 failed");
            close(fpout);
            close(temp_fd);
            return;
        }
        char baidu[24] = "www.baidu.com";
        char ping_dns[24] = "8.8.8.8";
        char ping_baidu[128];
        memset(ping_baidu, 0, 128);
        sprintf(ping_baidu, "ping -c 10 -i 0.5 -w 5 %s", baidu);
        write(fpout, "ping baidu:\n", 12);
        system(ping_baidu);
        LOGD("baidu ping:%s", ping_baidu);
        write(fpout, "----------\n", 11);
        memset(ping_baidu, 0, 128);
        sprintf(ping_baidu, "ping -c 10 -i 0.5 -w 5 %s", ping_dns);
        write(fpout, "ping dns:\n", 10);
        system(ping_baidu);
        LOGD("DNS ping:%s", ping_baidu);
        if (dup2(temp_fd, temp_fd2) < 0) {
            perror("dup2 incorrect");
            LOGE("dup2 failed");
            close(fpout);
            close(temp_fd);
            return;
        }
        close(temp_fd);
        if (fpout) {
            if (close(fpout) == 0)
                LOGD(" close %u successfully,temp_fd:%u,temp_fd2:%u", fpout,
                        temp_fd, temp_fd2);
            else
                LOGD(" close %u failed", fpout);
        }
    } else {
        LOGD("No need to ping DNS at stopping time");
    }

    return;

}

void sig_handler_test(int s) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        LOGE("sig_handler_test child %d terminated, receive SIGCHLD %d in full_test_data.\n", pid, s);
        return;
    }
}

