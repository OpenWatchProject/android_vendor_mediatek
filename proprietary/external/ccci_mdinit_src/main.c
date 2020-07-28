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


#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
//#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>

#include "ccci_common.h"

int curr_md_id;
int  system_ch_handle = 0;

#define BOOT_PROF_FILE      "/proc/bootprof"
static int boot_prof_fd;
int update_inf_to_bootprof(char str[])
{
    int ret = -1, size;
    char msg[64];

    if (boot_prof_fd >= 0) {
        size = snprintf(msg, 64, "ccci_md%d: %s", curr_md_id, str);
        ret = write(boot_prof_fd, msg, size);
    }

    return ret;
}

int main(int argc, char **argv)
{
    int drv_ver;

    curr_md_id = 0;

    if (argc < 2) {
        CCCI_LOGV("argc value invalid(%d), exit", argc);
        return 0;
    }
    if (strcmp(argv[1], "0") == 0)
        curr_md_id = 1;
    else if (strcmp(argv[1], "1") == 0)
        curr_md_id = 2;
    else if (strcmp(argv[1], "2") == 0)
        curr_md_id = 3;
    else {
        CCCI_LOGV("invalid md_id(%s), exit", argv[1]);
        return 0;
    }

    boot_prof_fd = open(BOOT_PROF_FILE, O_RDWR);
    update_inf_to_bootprof("md_init srv start");

    depends_so_prepare();

    if (check_lk_load_md_status(curr_md_id) < 0) {
        CCCI_LOGV("detect lk info fail, exit");
        return 0;
    }

    if (kernel_setting_prepare() < 0) {
        CCCI_LOGV("get kernel setting fail, exit");
        return 0;
    }

    if (is_current_md_en(curr_md_id) == 0) {
        CCCI_LOGV("md not enabled, exit");
        return 0;
    }

    drv_ver = get_ccci_drv_ver();
    if (drv_ver == 2)
        main_v2(curr_md_id, 0);
    else if(drv_ver == 1)
        main_v1(curr_md_id, 1);
    else
        main_v1(curr_md_id, 0);

    depends_so_free();

    if (boot_prof_fd >= 0)
        close(boot_prof_fd);
    return 0;
}



