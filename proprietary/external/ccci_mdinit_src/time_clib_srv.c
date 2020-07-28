/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include "hardware/ccci_intf.h"
#include "ccci_mdinit_cfg.h"
#include "ccci_common.h"

#define CCCI_TIMEZONE_FILE    "/data/vendor/ccci_cfg/timezone"
#define CCCI_TIME_UPDATE_PORT    "/dev/ccci_ipc_5"

//extern int time_monitor_thd_quit;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int ipc_fd;

static int save_timezone(int tz)
{
    int fd;
    int ret;
    CCCI_LOGD("save_timezone++\n");
    fd = open(CCCI_TIMEZONE_FILE, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if(fd<0) {
        CCCI_LOGE("Open time zone setting fail:%d(%d)", fd, errno);
        return -1;
    }

    //Update timezone info to file
    ret = write(fd, &tz, sizeof(int));
    if(ret != sizeof(int)) {
        CCCI_LOGE("Write time zone setting fail:%d(%d)", ret, errno);
        close(fd);
        return -2;
    }

    close(fd);
    return 0;
}

int load_timezone(int *tz)
{
    int fd;
    int ret;
  CCCI_LOGD("load_timezone++\n");
    fd = open(CCCI_TIMEZONE_FILE, O_RDONLY);
    if(fd<0) {
        CCCI_LOGE("Open time zone setting fail(R):%d(%d)", fd, errno);
        return -1;
    }

    //Update timezone info to file
    ret = read(fd, tz, sizeof(int));
    if(ret != sizeof(int)) {
        CCCI_LOGE("Read time zone setting fail(R):%d(%d)", ret, errno);
        close(fd);
        return -2;
    }

    close(fd);
    return 0;
}

int time_srv_init(void)
{
    struct timezone tz;
    int    curr_tz;
    CCCI_LOGD("time_srv_init++\n");
    ipc_fd = open(CCCI_TIME_UPDATE_PORT, O_RDWR);
    if(ipc_fd<0) {
        CCCI_LOGE("Open ipc port %d fail", errno);
        return -1;
    }

    if(load_timezone(&curr_tz) < 0) {
        gettimeofday(NULL, &tz);
        curr_tz = tz.tz_minuteswest;
    }

    if(ioctl(ipc_fd, CCCI_IPC_UPDATE_TIMEZONE, curr_tz)<0) {
        CCCI_LOGI("Set default tz by ipc port fail(%d)", errno);
        return -2;
    }
    return 0;
}

void* monitor_time_update_thread(__attribute__((unused))void* arg)
{
    struct timezone tz;
    int    curr_tz;
    CCCI_LOGD("monitor_time_update_thread++\n");
    if(load_timezone(&curr_tz) < 0) {
        gettimeofday(NULL, &tz);
        curr_tz = tz.tz_minuteswest;
    }

    CCCI_LOGD("Umonitor_time_update_thread begin to run");

    while(1) {
        pthread_mutex_lock(&mtx);

        if(ioctl(ipc_fd, CCCI_IPC_WAIT_TIME_UPDATE, 0)==0) {
            gettimeofday(NULL, &tz);
            if(curr_tz != tz.tz_minuteswest) {
                curr_tz = tz.tz_minuteswest;
                save_timezone(curr_tz);
            }

            if(ioctl(ipc_fd, CCCI_IPC_UPDATE_TIME, curr_tz)<0)
                CCCI_LOGI("Update time to md by ipc port fail(%d)", errno);
        }

        pthread_mutex_unlock(&mtx);
    }

    close(ipc_fd);
    return NULL;
}




