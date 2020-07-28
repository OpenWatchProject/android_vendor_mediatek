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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <log/log.h>
#include <string.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include "config.h"
#include "global_var.h"
#include "mlog.h"
#include "libfunc.h"
#include "bootmode.h"

sd_context_t sdContext = {INTERNAL_SD, PATH_DEFAULT};

pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

#define SUBLOG_MASK  0x0001DD81  // valid bit
const char* SUBLOG_NAME[MOBILOG_ID_MAX][2] = {
    [MOBILOG_ID_MAIN]     = {KEY_ANDROID, "1"},
    [MOBILOG_ID_RADIO]    = {"", ""},
    [MOBILOG_ID_EVENTS]   = {"", ""},
    [MOBILOG_ID_SYSTEM]   = {"", ""},
    [MOBILOG_ID_CRASH]    = {"", ""},
    [MOBILOG_ID_STATS]    = {"", ""},
    [MOBILOG_ID_SECURITY] = {"", ""},
    [MOBILOG_ID_KERNEL]   = {KEY_KERNEL, "1"},

    [MOBILOG_ID_ATF]      = {KEY_ATF, "1"},
    [MOBILOG_ID_GZ]       = {"", ""},
    [MOBILOG_ID_BSP]      = {KEY_BSP, "1"},
    [MOBILOG_ID_MMEDIA]   = {KEY_MMEDIA, "1"},
    [MOBILOG_ID_SCP]      = {KEY_SCP, "1"},
    [MOBILOG_ID_SCP_B]    = {"", ""},
    [MOBILOG_ID_SSPM]     = {KEY_SSPM, "1"},
    [MOBILOG_ID_ADSP]     = {KEY_ADSP, "1"},
    [MOBILOG_ID_MCUPM]    = {KEY_MCUPM, "1"},
};

bool gAllMode = true;  // if all mode use unified config
char spec_customer[16] = {0x00};  // special customer name
custom_id_t customId;

/*
 *  erase leading or trailing spaces
 */
char *strim(char *str) {
    size_t size;
    char *last_end, *end;

    size = strlen(str);
    if (!size)
        return str;

    end = str + size -1;
    last_end = end;
    while (end > str && ((*end == ' ') || (*end == '\n')))
        end--;
    if (last_end != end) {
        *(end+1) = '\0';
    }

    while (*str == ' ')
        str++;

    return str;
}

static const char *default_value(const char *key) {
    if (!strcmp(key, KEY_BOOT)) {
        char buf[PROPERTY_VALUE_MAX];
        property_get(PROP_BUILD_TYPE, buf, BUILD_TYPE_USER);
        if (!strcmp(buf, BUILD_TYPE_ENG)) {
            return ENABLE;
        } else {
            return DISABLE;
        }
    } else if (!strcmp(key, KEY_PATH)) {
        return PATH_INTERNAL;
    } else if (!strcmp(key, KEY_SPEC_CUSTOM)) {
        return CUSTOM_DEFAULT;
    } else if (!strcmp(key, KEY_SIZE)) {
        return SIZE_DEFAULT;
    } else if (!strcmp(key, KEY_TOTAL_SIZE)) {
        return TOTAL_SIZE_DEFAULT;
    } else {
        for (int i = MOBILOG_ID_MIN; i < MOBILOG_ID_MAX; i++)
            if (!strcmp(key, SUBLOG_NAME[i][0])) {
                return SUBLOG_NAME[i][1];
            }
    }

    MLOGE_DATA("not a valid key:%s", key);
    return NULL;
}

void init_custom_id(const char *name) {
    if (strstr(name, "GOD_A"))
        customId = CUSTOME_ID_GOD_A;
    else if (strstr(name, CUSTOM_DEFAULT))
        customId = CUSTOME_ID_MTK_IN;
    else
        MLOGE_DATA("No config for Customer: %s", name);
}

const char *read_config_value(const char *key) {
    FILE *fp;
    char buf[255];
    static char value[255];

    if (access(CONFIG_FILE, F_OK) != 0) {
        MLOGE_DATA("config file not exist");
        if (create_config_file() == 0) {
            MLOGE_DATA("create config file failed");
            return default_value(key);
        }
    }

    pthread_mutex_lock(&config_lock);
    if ((fp = fopen(CONFIG_FILE, "r")) == NULL) {
        MLOGE_BOTH("open config file fail,%s", strerror(errno));
        pthread_mutex_unlock(&config_lock);
        return default_value(key);
    }

    memset(value, 0x0, sizeof(value));
    while (!feof(fp)) {
        char *p;
        memset(buf, 0x0, sizeof(buf));
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            MLOGE_BOTH("not find key:%s", key);
            create_config_file();
            snprintf(value, sizeof(value), "%s", default_value(key));
            break;
        }

        if (strstr(buf, key)) {
            int len = strlen(buf);
            if (buf[len-1] == '\n')
                buf[len-1] = '\0';
            p = strchr(buf, '=');
            if (p) {
                snprintf(value, sizeof(value), "%s", p+1);
            } else {
                MLOGE_BOTH("config file error %s", buf);
                create_config_file();
                snprintf(value, sizeof(value), "%s", default_value(key));
            }
            break;
        }
    }
    fclose(fp);
    pthread_mutex_unlock(&config_lock);
    if (*value == 0x00)
        MLOGE_BOTH("can't find key:%s", key);
    return value;
}

void update_config_value(const char *key, const char *new_value) {
    int i, num;
    FILE *fp;
    char temp[KEY_NUM*2][255];
    const char* real_key;
    int config_all_log = 0;
    long int mask;

    // MLOGI_BOTH("thread %d comes", gettid());
    pthread_mutex_lock(&config_lock);
    if ((fp = fopen(CONFIG_FILE, "r+")) == NULL) {
        MLOGE_BOTH("update config(%s=%s) fail(%s)", key, new_value, strerror(errno));
        pthread_mutex_unlock(&config_lock);
        return;
    }

    real_key = key;
    if (strstr(key, "sublog_ALL")) {
        real_key = NULL;
        config_all_log = 1;
        if (strncmp(new_value, "0x", 2) && strncmp(new_value, "0X", 2)) {
            MLOGE_BOTH("'sublog_ALL' with Invalid value: %s", new_value);
            goto close_and_unlock;
        }
        mask = strtol(new_value, NULL, 16);
        if (mask == LONG_MAX) {
            MLOGE_BOTH("'sublog_ALL' config Fail, the value >= LONG_MAX");
            goto close_and_unlock;
        }
        mask = mask & SUBLOG_MASK;
        MLOGI_BOTH("Config sublogs: 0x%08lX", mask);
    } else if (strstr(key, "sublog_")) {
        real_key = strchr(key, '_') + 1;
    }

    // update config items
    num = 0;
    memset(temp, 0x0, sizeof(temp));
    if (config_all_log) {
        while (!feof(fp)) {
            if (fgets(temp[num], sizeof(temp[num]), fp) == NULL)
                break;
            if (!strncmp(temp[num], KEY_ANDROID, strlen(KEY_ANDROID))) {
                memset(temp[num], 0x00, sizeof(temp[num]));
                break;
            }
            if (num++ > KEY_NUM) {
                MLOGE_BOTH("Too Many Configs");
                break;
            }
        }
        for (int i = MOBILOG_ID_MIN; i < MOBILOG_ID_MAX; i++) {
            if (strlen(SUBLOG_NAME[i][0])) {
                if (mask & 1 << i) {
                    snprintf(temp[num], sizeof(temp[num]), "%s=1\n", SUBLOG_NAME[i][0]);
                } else {
                    snprintf(temp[num], sizeof(temp[num]), "%s=0\n", SUBLOG_NAME[i][0]);
                }
                num++;
            }
        }
    } else {
        while (!feof(fp)) {
            if (fgets(temp[num], sizeof(temp[num]), fp) == NULL)
                break;
            if (!strncmp(temp[num], real_key, strlen(real_key))) {
                memset(temp[num], 0x00, sizeof(temp[num]));
                snprintf(temp[num], sizeof(temp[num]), "%s=%s\n", real_key, new_value);
            }
            num++;
            if (num >= KEY_NUM*2) {
                MLOGE_BOTH("too many config");
                break;
            }
        }
    }

    if (num > KEY_NUM) {
        MLOGE_DATA("why here");
    }
    fclose(fp);
    fp = fopen(CONFIG_FILE, "w");
    if (fp == NULL) {
        MLOGE_BOTH("update config(%s=%s) fail(%s)", key, new_value, strerror(errno));
        pthread_mutex_unlock(&config_lock);
        return;
    }
    for (i=0; i < num; i++) {
        if (fputs(temp[i], fp) == EOF)
            MLOGE_BOTH("update config item [%s], fputs fail, (%s)", temp[i], strerror(errno));
    }
    if (fflush(fp) != 0)
        MLOGE_BOTH("update config, fflush fail, (%s)", strerror(errno));
    if (fsync(fileno(fp)) < 0)
        MLOGE_BOTH("update config, fsync fail, (%s)", strerror(errno));

close_and_unlock:
    fclose(fp);
    pthread_mutex_unlock(&config_lock);
}

#define CUSTOM_NUM (4)
int create_config_file() {
    FILE *fp, *fp2;
    char *p, buf[512], c_buf[255];

    if (access(CONFIG_DIR, F_OK) != 0) {
        create_dir(CONFIG_DIR);
    }

    if ((fp = fopen(CONFIG_FILE, "w")) == NULL) {
        MLOGE_BOTH("open %s fail %s", CONFIG_FILE, strerror(errno));
        return 0;
    }

    if (access(CUSTOM_FILE, F_OK) == 0) {
        int i = 0, ret;
        char *q = buf;
        char customer[64] = {0x00};

        if ((fp2 = fopen(CUSTOM_FILE, "r")) == NULL) {
            MLOGE_BOTH("open %s fail %s", CUSTOM_FILE, strerror(errno));
            goto default_set;
        }
        memset(buf, 0x0, sizeof(buf));
        while (!feof(fp2)) {
            ret = 0;
            if (fgets(c_buf, sizeof(c_buf), fp2) == NULL) {
                break;
            }
            if (strstr(c_buf, CUSTOM_BOOT)) {
                if (strstr(c_buf, "true")) {
                    ret = snprintf(q, sizeof(buf), "%s=1\n", KEY_BOOT);
                } else {
                    ret = snprintf(q, sizeof(buf), "%s=0\n", KEY_BOOT);
                }
            } else if (strstr(c_buf, CUSTOM_PATH)) {
                p = strchr(c_buf, '=');
                ret = snprintf(q, sizeof(buf), "%s=%s\n", KEY_PATH, strim(p+1));
            } else if (strstr(c_buf, CUSTOM_SPEC_CUSTOM)) {
                p = strchr(c_buf, '=');
                // this config may not exist.
                snprintf(customer, sizeof(customer), "%s=%s\n", KEY_SPEC_CUSTOM, strim(p+1));
            } else if (strstr(c_buf, CUSTOM_SIZE)) {
                p = strchr(c_buf, '=');
                ret = snprintf(q, sizeof(buf), "%s=%s\n", KEY_SIZE, strim(p+1));
            } else if (strstr(c_buf, CUSTOM_TOTALSIZE)) {
                p = strchr(c_buf, '=');
                ret = snprintf(q, sizeof(buf), "%s=%s\n", KEY_TOTAL_SIZE, strim(p+1));
            } else if (strstr(c_buf, CUSTOM_ANDROID_LOG)) {
                if (strstr(c_buf, "true")) {
                    SUBLOG_NAME[MOBILOG_ID_MAIN][1] = "1";
                } else {
                    SUBLOG_NAME[MOBILOG_ID_MAIN][1] = "0";
                }
            } else if (strstr(c_buf, CUSTOM_KERNEL_LOG)) {
                if (strstr(c_buf, "true")) {
                    SUBLOG_NAME[MOBILOG_ID_KERNEL][1] = "1";
                } else {
                    SUBLOG_NAME[MOBILOG_ID_KERNEL][1] = "0";
                }
            }

            if (ret > 0) {
                q = q+ret;
                i++;
            }
        }
        fclose(fp2);
        if (i == CUSTOM_NUM) {
            if (customer[0] == 0x00) {
                snprintf(customer, sizeof(customer), "%s=%s\n",
                    KEY_SPEC_CUSTOM, default_value(KEY_SPEC_CUSTOM));
            }
            ALOGE("customer: %s", customer);
            fputs(customer, fp);
            fputs(buf, fp);
            goto left;
        } else {
            MLOGE_DATA("only get %d configs from custom file,use default", i);
        }
    } else {
        MLOGE_BOTH("acess %s fail", CUSTOM_FILE);
    }

default_set:
    memset(buf, 0x0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s=%s\n%s=%s\n%s=%s\n%s=%s\n%s=%s\n",
             KEY_SPEC_CUSTOM, default_value(KEY_SPEC_CUSTOM),
             KEY_BOOT, default_value(KEY_BOOT),
             KEY_SIZE, default_value(KEY_SIZE),
             KEY_TOTAL_SIZE, default_value(KEY_TOTAL_SIZE),
             KEY_PATH, default_value(KEY_PATH));
    if (fputs(buf, fp) == EOF)
        MLOGE_BOTH("fputs config global items fail, (%s)", strerror(errno));
    if (fflush(fp) != 0)
        MLOGE_BOTH("config global items, fflush fail, (%s)", strerror(errno));
    if (fsync(fileno(fp)) < 0)
        MLOGE_BOTH("config global items, fsync fail, (%s)", strerror(errno));

left:
    memset(buf, 0x0, sizeof(buf));
    int ret = 0;
    for (int i = MOBILOG_ID_MIN; i < MOBILOG_ID_MAX; i++) {
        if (strlen(SUBLOG_NAME[i][0])) {
            ret += snprintf(buf + ret, sizeof(buf) - ret, "%s=%s\n", SUBLOG_NAME[i][0], SUBLOG_NAME[i][1]);
        }
        if (sizeof(buf) < (size_t)ret) {
            MLOGE_BOTH("Some sublog defualt config may be missed.");
            break;
        }
    }

    if (fputs(buf, fp) == EOF)
        MLOGE_BOTH("fputs config sublog items fail, (%s)", strerror(errno));
    if (fflush(fp) != 0)
        MLOGE_BOTH("config sublog items, fflush fail, (%s)", strerror(errno));
    if (fsync(fileno(fp)) < 0)
        MLOGE_BOTH("config sublog items, fsync fail, (%s)", strerror(errno));
    fclose(fp);

    if (chmod(CONFIG_FILE, 0660) < 0)
        MLOGE_BOTH("chmod %s faile(%s)", CONFIG_FILE, strerror(errno));
    MLOGI_DATA("create configure file done");

    return 1;
}

bool ifAllModeConfigUnified() {
    FILE *fp;
    char buf[255];
    bool allModeConfigUnified = true;

    if (access(CUSTOM_FILE, F_OK) == 0) {
        if ((fp = fopen(CUSTOM_FILE, "r")) == NULL) {
            MLOGE_BOTH("open %s fail, %s", CUSTOM_FILE, strerror(errno));
            return true;
        }
        while (!feof(fp)) {
            if (fgets(buf, sizeof(buf), fp) == NULL) {
                break;
            }
            if (strstr(buf, CUSTOM_AllMode)) {
                if (strstr(buf, "true")) {
                    allModeConfigUnified = true;
                    MLOGI_BOTH("AllModeConfigUnified is true");
                } else {
                    allModeConfigUnified = false;
                    MLOGI_BOTH("AllModeConfigUnified is false");
                }
            }
        }
        fclose(fp);
    } else
        MLOGE_BOTH("acess %s fail, %s", CUSTOM_FILE, strerror(errno));

    return allModeConfigUnified;
}

/*
* Attention Please:
*
* This function may be invoked by root. Do not use MLOG API during this function,
* MLOGx may create a root:root mblog_history which cannot access later
*/
int get_custom_mobilog_path(char path[], int len) {
    FILE *fp;
    char buf[255];
    char *p;
    int ret = -1;

    if (access(CUSTOM_FILE, F_OK) == 0) {
        if ((fp = fopen(CUSTOM_FILE, "r")) == NULL) {
            ALOGE("open %s fail, %s", CUSTOM_FILE, strerror(errno));
            return -1;
        }
        while (!feof(fp)) {
            if (fgets(buf, sizeof(buf), fp) == NULL) {
                break;
            }
            if (strstr(buf, CUSTOM_MOBILOG_PATH)) {
                p = strchr(buf, '=');
                ret = snprintf(path, len, "%s", strim(p+1));
            }
        }
        fclose(fp);
    } else
        ALOGE("acess %s fail, %s", CUSTOM_FILE, strerror(errno));
    return ret;
}

int update_sd_context(const char* path) {
    #define ROOTDIR  PATH_EXTSD_PARENT  //  "/storage/"
    size_t len1 = strlen(ROOTDIR);
    size_t len2 = strlen(path);

    if (len2 < sizeof(sdContext.sdpath) - 1)
        snprintf(sdContext.sdpath, sizeof(sdContext.sdpath), "%s", path);
    else {
        MLOGE_DATA("Update sdpath fail, the length of path is too long");
        return 0;
    }

    if (sdContext.sdpath[len2 - 1] != '/') {
        sdContext.sdpath[len2] = '/';
        sdContext.sdpath[len2 + 1] = '\0';
    }

    // check sd path type
    if ((len1 + 9 + 1 == strlen(sdContext.sdpath)) && !strncmp(path, ROOTDIR, len1)) {  // ROOTDIR + XXXX-XXXX + /
        if (*(path + len1 + 4) == '-') {  // XXXX-XXXX (external sdcard uuid)
            sdContext.location = EXTERNAL_SD;
            update_config_value(KEY_PATH, PATH_EXTERNAL);
            return 1;
        }
    }

    sdContext.location = INTERNAL_SD;
    update_config_value(KEY_PATH, PATH_INTERNAL);
    return 1;
}

