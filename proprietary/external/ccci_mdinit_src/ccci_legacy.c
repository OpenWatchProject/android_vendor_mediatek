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
#include "hardware/ccci_intf.h"


typedef enum {
    md_type_invalid = 0,
    modem_2g = 1,
    modem_3g,
    modem_wg,
    modem_tg,
    modem_lwg,
    modem_ltg,
    MAX_IMG_NUM = 16 /* this enum starts from 1 */
}modem_type_t;

typedef struct _md_img
{
    char file_name[32];
    int  type;
}md_img_t;

const char md_img_folder[32] = "/vendor/firmware/";
const char md_img_cip_folder[32] = "/custom/etc/firmware/";

static md_img_t md1_img_list[] = {
    { "modem_1_2g_n.img", modem_2g},
    { "modem_1_3g_n.img", modem_3g},
    { "modem_1_wg_n.img", modem_wg},
    { "modem_1_tg_n.img", modem_tg},
    { "modem_1_lwg_n.img", modem_lwg},
    { "modem_1_ltg_n.img", modem_ltg},
};

static md_img_t md2_img_list[] = {
    { "modem_2_2g_n.img", modem_2g},
    { "modem_2_3g_n.img", modem_3g},
    { "modem_2_wg_n.img", modem_wg},
    { "modem_2_tg_n.img", modem_tg},
    { "modem_2_lwg_n.img", modem_lwg},
    { "modem_2_ltg_n.img", modem_ltg},
};

static md_img_t md3_img_list[] = {
    { "modem_3_3g_n.img", modem_3g},
};

int md_image_exist_check(int fd,int md_id)
{
    char md_img_path[100];
    char md_img_cip_path[100];
    unsigned int md_img_index = 0;
    int err = 0;
    unsigned int md_img_exist[MAX_IMG_NUM] = {0};
    md_img_t *md_img_list;
    unsigned int scan_cnt = 0;
    int exist_idx = 0;

    if (md_id == 1) {
        md_img_list = md1_img_list;
        scan_cnt = sizeof(md1_img_list)/sizeof(md_img_t);
    } else if (md_id == 2) {
        md_img_list = md2_img_list;
        scan_cnt = sizeof(md2_img_list)/sizeof(md_img_t);
    } else if (md_id == 3) {
        md_img_list = md3_img_list;
        scan_cnt = sizeof(md3_img_list)/sizeof(md_img_t);
    }

    memset(md_img_exist, 0, sizeof(md_img_exist));

    while (md_img_index < scan_cnt){
        memset(md_img_path, '\0', sizeof(md_img_path));
        snprintf(md_img_path, sizeof(md_img_path), "%s%s", md_img_folder, md_img_list[md_img_index].file_name);
        snprintf(md_img_cip_path, sizeof(md_img_cip_path), "%s%s", md_img_cip_folder, md_img_list[md_img_index].file_name);
        //CCCI_LOGD("looking for %s...", md_img_cip_path);
        if ((err = access(md_img_cip_path, F_OK)) == 0){
            CCCI_LOGD("Found %s\n", md_img_list[md_img_index].file_name);
            md_img_exist[exist_idx] = md_img_list[md_img_index].type;
            exist_idx++;
        }else {
            //CCCI_LOGD("looking for %s...", md_img_path);
            if ((err = access(md_img_path, F_OK)) == 0){
                CCCI_LOGD("Found %s\n", md_img_list[md_img_index].file_name);
                md_img_exist[exist_idx] = md_img_list[md_img_index].type;
                exist_idx++;
            }
        }
        md_img_index++;
    }
    CCCI_LOGI("md_img_exist %d %d %d %d\n", md_img_exist[0], md_img_exist[1], md_img_exist[2], md_img_exist[3]);
    ioctl(fd, CCCI_IOC_SET_MD_IMG_EXIST, &md_img_exist);
    return 0;
}
