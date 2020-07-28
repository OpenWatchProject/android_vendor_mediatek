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
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <log/log.h>

#include "global_var.h"
#include "config.h"
#include "mlog.h"
#include "size_control.h"
#include "bootmode.h"

#define KEY  "index="

unsigned long read_folder_index(sd_type_t log_sd_type) {
    char folderRotFile[255], buf[255];
    FILE *fp;
    unsigned long index = 0;

    if (log_sd_type == INTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", INTER_DIR_FILE);
    else if (log_sd_type == EXTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", OUTER_DIR_FILE);
    else {
        MLOGE_BOTH("Can not know log_sd_type when get folder index");
        return 0;
    }
    // open and get index
    if ((fp = fopen(folderRotFile, "r")) == NULL) {
        MLOGE_BOTH("open %s faild %s", folderRotFile, strerror(errno));
        return 0;
    }

    char *p;
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        p = strstr(buf, KEY);
        if (p)
            index = strtol(p + strlen(KEY), NULL, 10);
    }
    fclose(fp);

    return index;
}
static struct folder_util *read_folder_file(sd_type_t log_sd_type) {
    struct folder_util *entry, *head;
    char folderRotFile[255], buf[255];
    FILE *fp;

    if (log_sd_type == INTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", INTER_DIR_FILE);
    else if (log_sd_type == EXTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", OUTER_DIR_FILE);
    else {
        MLOGE_BOTH("Can not know log_sd_type when get folder rotation info");
        return NULL;
    }
    // open and read
    if ((fp = fopen(folderRotFile, "r")) == NULL) {
        MLOGE_BOTH("open %s faild %s", folderRotFile, strerror(errno));
        return NULL;
    }

    entry = head = NULL;
    while (!feof(fp)) {
        memset(buf, 0x0, sizeof(buf));
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            break;
        }

        char *p;
        if ((p = strchr(buf, ' ')) == NULL) {
            if (!strstr(buf, KEY))
                MLOGE_BOTH("wrong format %s", buf);
            continue;
        }

        *p = '\0';
        struct folder_util *new_fu = (struct folder_util*)malloc(sizeof(struct folder_util));
        if (new_fu == NULL) {
            MLOGE_BOTH("malloc new_fu fail %s", strerror(errno));
            continue;
        }

        snprintf(new_fu->name, sizeof(new_fu->name), "%s", buf);
        new_fu->size = atoi(p+1);
        new_fu->next = NULL;

        if (entry) {
            entry->next = new_fu;
            entry = new_fu;
        } else {
            head = entry = new_fu;
        }
    }

    fclose(fp);

    return head;
}

static int write_folder_file(struct folder_util *head, char *new_folder, unsigned long idx, sd_type_t log_sd_type) {
    struct folder_util *entry;
    // unsigned long index;
    char folderRotFile[255], buf[255];
    FILE *fp;
    static int haschmod = 0;

    if (log_sd_type == INTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", INTER_DIR_FILE);
    else if (log_sd_type == EXTERNAL_SD)
        snprintf(folderRotFile, sizeof(folderRotFile), "%s", OUTER_DIR_FILE);
    else {
        MLOGE_BOTH("Can not know log_sd_type when write folder rotation info");
        return 0;
    }

    // open and write
    if ((fp = fopen(folderRotFile, "w")) == NULL) {
        MLOGE_BOTH("open %s faild %s", folderRotFile, strerror(errno));
        return 0;
    }

    // write 'index=idx'
    snprintf(buf, sizeof(buf), "index=%lu\n", idx);
    fputs(buf, fp);
    // write folder info
    for (entry = head; entry; entry=entry->next) {
        snprintf(buf, sizeof(buf), "%s %llu\n", entry->name, entry->size);
        fputs(buf, fp);
    }

    snprintf(buf, sizeof(buf), "%s 0\n", new_folder);
    fputs(buf, fp);
    fclose(fp);

    if (!haschmod) {
        if (chmod(folderRotFile, 0664) < 0) {
            MLOGE_BOTH("chmod %s fail(%s)", folderRotFile, strerror(errno));
            return 0;
        }
        haschmod = 1;
    }

    return 1;
}

static struct folder_util *get_cur_path_folder() {
    char whole_path[256];
    int len;
    struct folder_util *head, *fu_p;
    DIR *cur_dir;
    struct dirent *dir_info;

    // debuglogger/mobilelog/ path
    snprintf(whole_path, sizeof(whole_path), "%s", sdContext.sdpath);
    len =  strlen(whole_path);
    strncat(whole_path, PATH_SUFFIX, sizeof(whole_path) - 1 - len);

    if ((cur_dir = opendir(whole_path)) == NULL) {
        MLOGE_BOTH("open %s fail %s", whole_path, strerror(errno));
        return NULL;
    }

    fu_p = head = NULL;
    while ((dir_info=readdir(cur_dir))) {
        if (!strcmp(dir_info->d_name, ".") ||
                !strcmp(dir_info->d_name, ".."))
            continue;

        struct folder_util *fu =
            (struct folder_util *)malloc(sizeof(struct folder_util));
        if (fu == NULL) {
            MLOGE_BOTH("malloc fu fail %s", strerror(errno));
            continue;
        }
        strncpy(fu->name, dir_info->d_name, sizeof(fu->name) - 1);
        fu->next = NULL;

        if (fu_p) {
            fu_p->next = fu;
            fu_p = fu;
        } else {
            head = fu_p = fu;
        }
    }

    closedir(cur_dir);

    return head;
}

static long long get_folder_size(struct folder_util *fu_p) {
    long long size = 0;
    char tmpbuf[256], whole_path[256], whole_file[256];
    DIR *cur_dir;
    struct dirent *dir_info;
    struct stat file_info;

    // APLog foder full path
    snprintf(tmpbuf, sizeof(tmpbuf), "%s", sdContext.sdpath);
    snprintf(whole_path, sizeof(whole_path), "%s%s%s/", tmpbuf, PATH_SUFFIX, fu_p->name);

    if ((cur_dir = opendir(whole_path)) == NULL) {
        MLOGE_BOTH("open dir %s fail %s", whole_path, strerror(errno));
        return 0;
    }

    while ((dir_info=readdir(cur_dir))) {
        if (!strcmp(dir_info->d_name, ".") ||
                !strcmp(dir_info->d_name, ".."))
            continue;

        snprintf(whole_file, sizeof(whole_file), "%s%s", whole_path, dir_info->d_name);

        if (stat(whole_file, &file_info) < 0) {
            MLOGE_BOTH("stat %s fail(%s)", whole_file, strerror(errno));
        }
        size += file_info.st_size;
    }

    closedir(cur_dir);
    return size/1024;
}

static int resort_folder_list(struct folder_util **head) {
    struct folder_util *entry, **cur;

    for (cur = head; *cur;) {
        entry = *cur;
        if (entry->size == -1) {
            *cur = entry->next;
            MLOGE_BOTH("delete %s (no exist)", entry->name);
            free(entry);
        } else {
            cur = &entry->next;
        }
    }

    return 1;
}

static int control_folder_size(
    struct folder_util **head,
    unsigned int cur_size, unsigned int limit_size) {
    int len;
    struct folder_util *entry;
    char whole_path[256], cmd[512];

    // debuglogger/mobilelog/ path
    snprintf(whole_path, sizeof(whole_path), "%s", sdContext.sdpath);
    len =  strlen(whole_path);
    strncat(whole_path, PATH_SUFFIX, sizeof(whole_path) - 1 - len);

    MLOGE_BOTH("log folder total size overflow  ");
    entry = *head;
    while (entry && cur_size > limit_size) {
        memset(cmd, 0x0, sizeof(cmd));
        *head = entry->next;
        cur_size -= entry->size;
        MLOGE_BOTH("delete %s ", entry->name);
        snprintf(cmd, sizeof(cmd)-1, "rm -rf %s%s", whole_path, entry->name);
        system(cmd);
        free(entry);
        entry = *head;
    }

    return cur_size;
}

int update_folder_list(char *new_folder, unsigned long idx, sd_type_t log_sd_type) {
    int need_resort;
    long long total_sz, limit_sz;
    struct folder_util *cur_path_folder = NULL;
    struct folder_util *file_folder_list = NULL;

    file_folder_list = read_folder_file(log_sd_type);

    cur_path_folder = get_cur_path_folder();
    if (!cur_path_folder) {
        free(file_folder_list);
        MLOGE_BOTH("read cur path folder info fail");
        return 0;
    }

    need_resort = 0;
    total_sz = 0;
    struct folder_util *u, *v;
    for (u=file_folder_list; u ; u=u->next) {
        for (v=cur_path_folder; v ; v=v->next) {
            if (!strcmp(u->name, v->name)) {
                if (u->size == 0)
                    u->size = get_folder_size(u);
                total_sz += u->size;
                break;
            }
        }

        if (v == NULL) {
            MLOGE_BOTH("%s not in sd anymore", u->name);
            u->size = -1;
            need_resort = 1;
        }
    }

    if (need_resort) {
        resort_folder_list(&file_folder_list);
    }

    limit_sz = (long long)(atoi(read_bootmode_config(KEY_TOTAL_SIZE)) - atoi(read_bootmode_config(KEY_SIZE))) * 1024;
    if (limit_sz >= 0 && total_sz > limit_sz) {
        control_folder_size(&file_folder_list, total_sz, limit_sz);
    }

    write_folder_file(file_folder_list, new_folder, idx, log_sd_type);

    while (file_folder_list) {
        u = file_folder_list;
        file_folder_list = file_folder_list->next;
        free(u);
    }
    while (cur_path_folder) {
        u = cur_path_folder;
        cur_path_folder = cur_path_folder->next;
        free(u);
    }

    return 1;
}
