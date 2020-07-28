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
//#include <cutils/properties.h>
//#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include "hardware/ccci_intf.h"
#include "ccci_common.h"
#include "sysenv_utils.h"

static char sys_boot_mode = 0;
#define BOOT_MODE_FILE            "/sys/class/BOOT/BOOT/boot/boot_mode" // controlled by mtxxxx_boot.c
#define META_MODE            '1'
#define FACTORY_MODE        '4'
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 64
#endif

static char get_sys_boot_mode(void)
{
    int fd, ret;
    size_t s;
    volatile char data[20];

    if (!sys_boot_mode) {
        fd = open(BOOT_MODE_FILE, O_RDONLY);
        if (fd < 0) {
            CCCI_LOGE("fail to open %s: err_no=%d", BOOT_MODE_FILE, errno);
            return 0;
        }

        s = read(fd, (void *)data, sizeof(char) * 3);
        if (s <= 0) {
            CCCI_LOGE("fail to read %s err_no=%d", BOOT_MODE_FILE, errno);
            sys_boot_mode = '0';
        } else {
            sys_boot_mode = data[0];
        }

        close(fd);
    }
    CCCI_LOGD("system boot Mode: %d\n", sys_boot_mode);
    return sys_boot_mode;
}

int is_factory_mode(void)
{
    if (get_sys_boot_mode() == FACTORY_MODE)
        return 1;
    else
        return 0;
}

int is_meta_mode(void)
{
    int fd, ret;
    size_t s;
    volatile char data[20];

    fd = open(BOOT_MODE_FILE, O_RDONLY);
    if (fd < 0) {
        CCCI_LOGE("fail to open %s: err_no=%d", BOOT_MODE_FILE, errno);
        return 0;
    }

    s = read(fd, (void *)data, sizeof(char) * 3);
    if (s <= 0) {
        CCCI_LOGE("fail to read %s err_no=%d", BOOT_MODE_FILE, errno);
        ret = 0;
    } else {
        if (data[0] == META_MODE)
            ret = 1;
        else
            ret = 0;
    }

    close(fd);
    return ret;
}

/*------------------------------------------*/
/* Env variable need to store support       */
static int ccci_md_env_cfg_folder_init(void)
{
    int fd;
    int has_read;
    struct stat buf;
    umask(0007);
    if (stat("/data/vendor/ccci_cfg",&buf)<0) {
        CCCI_LOGD("No /data/vendor/ccci_cfg dir.\n");
        if (mkdir("/data/vendor/ccci_cfg",0700) < 0) {
            CCCI_LOGD("mkdir for ccci_cfg failed.\n");
            return -1;
        }
    }

    return 0;
}

static int md_env_val_get(const char name[], char buf[], int size)
{
    const char *tmp_buf;

    CCCI_LOGD("md env r\n");
    if (ccci_md_env_cfg_folder_init() < 0)
        return -1;

    tmp_buf = sysenv_get(name);
    snprintf(buf, size, "%s", tmp_buf);
    CCCI_LOGI("get %s %s", name, buf);
    return 1;
}

static int md_env_val_set(char name[], char val[])
{
    int ret = 0;

    CCCI_LOGD("md env w\n");
    if (ccci_md_env_cfg_folder_init() < 0)
        return -1;

    ret = sysenv_set(name, val);
    if (ret) {
        CCCI_LOGE("set %s fail", name);
        return -1;
    }
    return ret;
}


/*------------------------------------------*/
/* RAT configure setting */
/* Bit map defination at AP side            */
/* 0 | 0 | C | Lf | Lt | W | T | G          */
#define MD_CAP_ENHANCE        (0x5A<<24)
#define MD_CAP_GSM        (1<<0)
#define MD_CAP_TDS_CDMA        (1<<1)
#define MD_CAP_WCDMA        (1<<2)
#define MD_CAP_TDD_LTE        (1<<3)
#define MD_CAP_FDD_LTE        (1<<4)
#define MD_CAP_CDMA2000        (1<<5)
#define MD_CAP_MASK        (MD_CAP_GSM|MD_CAP_TDS_CDMA|MD_CAP_WCDMA|MD_CAP_TDD_LTE|MD_CAP_FDD_LTE|MD_CAP_CDMA2000)
#define MD_CAP_BIT_NUM        (6)

static unsigned int get_capability_bit(char cap_str[])
{
    if (cap_str == NULL)
        return 0;
    if ((strcmp(cap_str, "LF") == 0) || (strcmp(cap_str, "Lf") == 0) || (strcmp(cap_str, "lf") == 0))
        return MD_CAP_FDD_LTE;
    if ((strcmp(cap_str, "LT") == 0) || (strcmp(cap_str, "Lt") == 0) || (strcmp(cap_str, "lt") == 0))
        return MD_CAP_TDD_LTE;
    if ((strcmp(cap_str, "W") == 0) || (strcmp(cap_str, "w") == 0))
        return MD_CAP_WCDMA;
    if ((strcmp(cap_str, "C") == 0) || (strcmp(cap_str, "c") == 0))
        return MD_CAP_CDMA2000;
    if ((strcmp(cap_str, "T") == 0) || (strcmp(cap_str, "t") == 0))
        return MD_CAP_TDS_CDMA;
    if ((strcmp(cap_str, "G") == 0) || (strcmp(cap_str, "g") == 0))
        return MD_CAP_GSM;

    return 0;
}

#define MAX_CAP_STR_LENGTH    16
static unsigned int ccci_rat_str_to_bitmap(char str[])
{
    char tmp_str[MAX_CAP_STR_LENGTH];
    int tmp_str_curr_pos = 0;
    unsigned int capability_bit_map = 0;
    int str_len;
    int i;

    if (str == NULL)
        return 0;

    str_len = strlen(str);
    for (i = 0; i < str_len; i++) {
        if (str[i] == ' ')
            continue;
        if (str[i] == '\t')
            continue;
        if ((str[i] == '/') || (str[i] == '_')) {
            if (tmp_str_curr_pos) {
                tmp_str[tmp_str_curr_pos] = 0;
                capability_bit_map |= get_capability_bit(tmp_str);
            }
            tmp_str_curr_pos = 0;
            continue;
        }
        if (tmp_str_curr_pos < (MAX_CAP_STR_LENGTH-1)) {
            tmp_str[tmp_str_curr_pos] = str[i];
            tmp_str_curr_pos++;
        } else
            break;
    }
    if (tmp_str_curr_pos) {
        tmp_str[tmp_str_curr_pos] = 0;
        capability_bit_map |= get_capability_bit(tmp_str);
    }

    return capability_bit_map;
}

static unsigned int legacy_rat_map[] = {
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ultg */
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_WCDMA|MD_CAP_GSM), /* ulwg */
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_WCDMA|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ulwtg */
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_WCDMA|MD_CAP_CDMA2000|MD_CAP_GSM), /* ulwcg */
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_WCDMA|MD_CAP_CDMA2000|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ulwctg */
    (MD_CAP_TDD_LTE|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ulttg */
    (MD_CAP_FDD_LTE|MD_CAP_WCDMA|MD_CAP_GSM), /* ulfwg */
    (MD_CAP_FDD_LTE|MD_CAP_WCDMA|MD_CAP_CDMA2000|MD_CAP_GSM), /* ulfwcg */
    (MD_CAP_FDD_LTE|MD_CAP_TDD_LTE|MD_CAP_CDMA2000|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ulctg */
    (MD_CAP_TDD_LTE|MD_CAP_CDMA2000|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ultctg */
    (MD_CAP_TDD_LTE|MD_CAP_WCDMA||MD_CAP_GSM), /*ultwg */
    (MD_CAP_TDD_LTE|MD_CAP_WCDMA|MD_CAP_CDMA2000|MD_CAP_GSM), /* ultwcg */
    (MD_CAP_FDD_LTE|MD_CAP_TDS_CDMA|MD_CAP_GSM), /* ulftg */
    (MD_CAP_FDD_LTE|MD_CAP_CDMA2000|MD_CAP_TDS_CDMA|MD_CAP_GSM)/* ulfctg */
};

/* Rat value description           */
/* 0: invalid                      */
/* 1: 2g                           */
/* 2: 3g                           */
/* 3: wg                           */
/* 4: tg                           */
/* 5: lwg                          */
/* 6: ltg                          */
/* 7: sglte (phase out)            */
/* 8~21: legacy ubin wm_id         */
/* 0x5A000000+xx : true rat bitmap */

static unsigned int legacy_md_support_id_to_rat(int md_support_id)
{
    if (md_support_id < 0) /* Invalid case */
        return 0;
    if (md_support_id < 8) /* Legacy modem support value */
        return md_support_id;
    if (md_support_id <= 21) /* Legacy ubin modem support value */
        return legacy_rat_map[md_support_id - 8];
    if ((md_support_id & MD_CAP_ENHANCE) == MD_CAP_ENHANCE)
        return md_support_id;

    return 0;
}

unsigned int parse_sys_env_rat_setting(int md_id)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int retpropget = 0;
    unsigned int rsc_value = 0;
    unsigned int saved_id_or_rat = 0;

    retpropget = get_rsc_protol_value(md_id, value, sizeof(value));
    if (retpropget > 0)
    	rsc_value = atoi(value);

    if (md_id == 0) {
        retpropget = md_env_val_get("md_type", value, sizeof(value));
        if (retpropget > 0)
            saved_id_or_rat = atoi(value);
    }
    CCCI_LOGD("rsc_value: 0x%x(@prj), 0x%x(@store)", rsc_value, saved_id_or_rat);

    /* Priority: persist > ro.boot > ro. */
    if (saved_id_or_rat > 0)
        return saved_id_or_rat;

    return rsc_value;
}

int get_stored_modem_type_val(int md_id)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int retpropget = 0;
    int ret = 0;

    if (md_id != 0)
        return 0; /* Only support modem 1 */

    retpropget = md_env_val_get("md_type", value, sizeof(value));
    if (retpropget > 0)
        ret = legacy_md_support_id_to_rat(atoi(value));
    else
        ret = -1;

    CCCI_LOGD("get_save_modem_type_val: 0x%x", ret);

    return ret;
}

int store_modem_type_val(int md_id, int new_val)
{
    char value[PROPERTY_VALUE_MAX];

    if (md_id != 0)
        return 0; /* Only support modem 1 */
    if (new_val == 0)
        return 0;

    snprintf(value, sizeof(value), "%d", new_val);
    return md_env_val_set("md_type", value);
}
