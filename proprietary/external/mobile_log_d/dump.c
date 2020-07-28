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
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <log/log.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <cutils/properties.h>
#include <cutils/iosched_policy.h>
#include <private/android_filesystem_config.h>

#include "global_var.h"
#include "mlog.h"
#include "dump.h"
#include "libfunc.h"
#include "bootmode.h"

#define CMD             "cmd:"
#define DATA_TMPFS_LOG  "/data/data_tmpfs_log/"
#define ENCRY_TEMP_LOG  DATA_TMPFS_LOG"log_temp/"
#define ENCRY_CONFIG    DATA_TMPFS_LOG"misc/mblog/"
#define PHONE_INFO_FILE     "properties"
#define DUMP_FILES          "/system/etc/dump_files"
#define LXEND   "================ END of FILE ===============\n\n"

static void get_sys_prop(const char *key, const char *name, void *cookies) {
    char temp[1024];
    memset(temp, 0x0, 1024);
    snprintf(temp, 1023, "[%s]: [%s]\n", key, name);
    write(*(int *)cookies, temp, strlen(temp));
}

void dump_sys_prop(struct dump_util *du_p) {
    int fd;
    int result = -1;

    fd = open(du_p->des_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        MLOGE_BOTH("open %s fail (%s). Skip dump sys property", du_p->des_file, strerror(errno));
        return;
    } else {
        char buf[100];
        snprintf(buf, sizeof(buf), "<< system properties >>\n");
        write(fd, buf, strlen(buf));
        result = property_list(get_sys_prop, &fd);
    }
    write(fd, LXEND, strlen(LXEND));
    close(fd);

    if (result < 0)
        MLOGE_BOTH("dump sys property fail");
    else
        MLOGI_BOTH("dump sys property.");
    return;
}

void dump_file(struct dump_util *du_p) {
    size_t n;
    FILE *des_fp, *src_fp;
    char temp[8192];
    bool dumpCmd = false;
    int fd = -1;
    int ret = 0;

    if (!strncmp(du_p->src_file, CMD, strlen(CMD)))
        dumpCmd = true;

    if (dumpCmd) {
        if ((src_fp = popen(du_p->src_file + strlen(CMD), "r")) == NULL) {
            MLOGE_BOTH("Fail to exec %s, %s(%d)", du_p->src_file, strerror(errno), errno);
            return;
        }
    } else {
        if ((src_fp = fopen(du_p->src_file, "r")) == NULL) {
            return;
        }
    }

    fd = mb_open(du_p->des_file);
    if (fd < 0) {
        MLOGE_BOTH("open %s failed! (%s)", du_p->des_file, strerror(errno));
        dumpCmd ? pclose(src_fp) : fclose(src_fp);
        return;
    } else if ((des_fp = fdopen(fd, "a+")) == NULL) {
        MLOGE_BOTH("fdopen %d(%s) failed! (%s)", fd, du_p->des_file, strerror(errno));
        dumpCmd ? pclose(src_fp) : fclose(src_fp);
        return;
    }

    snprintf(temp, 1023, "<< %s >>:\n", du_p->src_file);
    fputs(temp, des_fp);
    while (!feof(src_fp)) {
        if (g_mblog_status == STOPPED) {
            MLOGE_BOTH("status is topped");
            pthread_exit(0);
        }

        memset(temp, 0x0, sizeof(temp));
        n = fread(temp, sizeof(char), sizeof(temp), src_fp);
        if (n != fwrite(temp, sizeof(char), n, des_fp)) {
            MLOGE_BOTH("fwrite error! %s(%d)", strerror(errno), errno);
            continue;
        }
    }
    fputs(LXEND, des_fp);

    MLOGI_BOTH("dump %s", du_p->src_file);
    dumpCmd ? pclose(src_fp) : fclose(src_fp);
    fclose(des_fp);

    ret = usleep(100000);
    if (ret < 0) {
        MLOGE_BOTH("usleep error %s", strerror(errno));
    }
    return;
}

struct func_util {
    char key[128];
    void (*func_name)(struct dump_util *du_p);
};

struct func_util dump_funcs[] = {
    {"default", dump_file},
    {"sys_prop", dump_sys_prop}
};
void set_func(struct dump_util *du_p, char *key) {
    if (key == NULL) {
        goto lxend;
    }

    int i, num;
    num = sizeof(dump_funcs)/sizeof(struct func_util);
    for (i=0; i < num; i++) {
        if (!strcmp(key, dump_funcs[i].key)) {
            du_p->dump = dump_funcs[i].func_name;
            return;
        }
    }

lxend:
    du_p->dump = dump_file;
}

struct dump_util *init_dump_list_from_file(char *file, char *path) {
    struct dump_util *du_list, *du_p;
    FILE *fp;
    char buf[255];
    char *src, *des, *func;

    if ((fp=fopen(file, "r")) == NULL) {
        MLOGE_BOTH("open %s fail,%s", file, strerror(errno));
        return NULL;
    }

    du_list = NULL;
    while (!feof(fp)) {
        if (fgets(buf, sizeof(buf), fp) == NULL)
            continue;
        if (buf[0] == '#' || buf[0] == '\n')
            continue;

        char *p = strchr(buf, '\n');
        if (p) *p = '\0';

        des = func = NULL;
        src = strtok(buf, " ");
        if (src != NULL) {
            des = strtok(NULL, " ");
            if (des != NULL)
                func =  strtok(NULL, " ");
        }

        du_p = (struct dump_util*)malloc(sizeof(struct dump_util));
        if (du_p == NULL) {
            MLOGE_BOTH("malloc() fail for %s", buf);
            continue;
        }
        memset(du_p, 0x0, sizeof(struct dump_util));
        if (src != NULL)
            strncpy(du_p->src_file, src, sizeof(du_p->src_file)-1);
        if (des != NULL)
            snprintf(du_p->des_file, sizeof(du_p->des_file), "%s%s", path, des);
        else
            snprintf(du_p->des_file, sizeof(du_p->des_file), "%s%s", path, PHONE_INFO_FILE);

        set_func(du_p, func);

        du_p->next = du_list;
        du_list = du_p;
    }
    fclose(fp);
    return du_list;
}

struct dump_util *add_dump_file(char *src, char *des, char *path, void* func) {
    struct dump_util *du_p;
    du_p = (struct dump_util*)malloc(sizeof(struct dump_util));
    if (du_p == NULL)
        return NULL;

    strncpy(du_p->src_file, src, sizeof(du_p->src_file) - 1);
    du_p->src_file[sizeof(du_p->src_file) - 1] = '\0';
    snprintf(du_p->des_file, sizeof(du_p->des_file), "%s%s", path, des);
    du_p->dump = func;
    du_p->next = NULL;

    return du_p;
}

void add_to_dump_list(struct dump_util **du_list, struct dump_util *item) {
    if (item == NULL)
        return;
    item->next = *du_list;
    *du_list = item;
}

struct dump_util *default_init_dump_list(char *des_path) {
    struct dump_util *du_list, *du_p;

    du_list = NULL;
    du_p = add_dump_file("property", PHONE_INFO_FILE, des_path, dump_sys_prop);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file("/proc/last_kmsg", "last_kmsg", des_path, dump_file);
    add_to_dump_list(&du_list, du_p);
    du_p = add_dump_file(CMD "logcat -L", "last_AndroidLog", des_path, dump_file);
    add_to_dump_list(&du_list, du_p);
    du_p = add_dump_file("/proc/bootprof", "bootprof", des_path, dump_file);
    add_to_dump_list(&du_list, du_p);
    du_p = add_dump_file("/proc/pl_lk", "pl_lk", des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(BOOTUP_FILE_TREE,     PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(MBLOG_HISTORY,     PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(CONFIG_FILE,       PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(INTER_DIR_FILE,    PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(OUTER_DIR_FILE,    PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    du_p = add_dump_file(DUMP_FILES,        PHONE_INFO_FILE, des_path, dump_file);
    add_to_dump_list(&du_list, du_p);

    return du_list;
}

struct dump_util *init_dump_list(char *des_path) {
    if (access(DUMP_FILES, F_OK) == 0)
        return init_dump_list_from_file(DUMP_FILES, des_path);
    else
        return default_init_dump_list(des_path);
}

void dump_system_info() {
    struct dump_util *du_list, *du_p;

    du_list = init_dump_list(cur_logging_path);

    if (du_list == NULL) {
        MLOGE_BOTH("init dump list fail");
        return;
    }

    while (du_list) {
        du_p = du_list;
        du_list = du_list->next;
        du_p->dump(du_p);
        free(du_p);
    }
}

void copy_to_sdcard() {
    char srcDir[ItemMaxLen * 4];
    char destDir[ItemMaxLen * 4];
    char cmd[500];

    char logF[ItemMaxLen + 2];  // Item + '\n' + '\0'
    char *end;
    int copyCnt = 0;


    FILE * fp = fopen(BOOTUP_FILE_TREE, "r");
    if (!fp) {
        MLOGE_BOTH("open bootFileTree fail, %s (%d)", strerror(errno), errno);
        goto copy_check;
    }

    for (int itemNo = 1; itemNo <= BOOTMAX; itemNo++) {
        if (getItemReverse(fp, logF, sizeof(logF), itemNo) > 0) {
            MLOGE_DATA("copy item: %s", logF);
            if (logF[0] == '\0' || logF[0] == '*')
                continue;
            // data/log_temp/boot_x__xxx/
            snprintf(srcDir, sizeof(srcDir), "%s%s", PATH_TEMP, logF);
            if (access(srcDir, F_OK) != 0) {
                continue;
            }

            // get the postfix, boot mode:  __xxxx/
            if ((end = strstr(logF, "__")) == NULL) {
                end = "/";
            }
            // ../mobilelog/APLog_.../boot + 'end'
            // ../mobilelog/APLog_.../Last_ + 'itemNo - 1' + _boot + 'end'
            if (itemNo - 1 > 0)
                snprintf(destDir, sizeof(destDir), "%s%s%d%s%s",
                    cur_logging_path, "Last_", itemNo - 1, "_boot", end);
            else
                snprintf(destDir, sizeof(destDir), "%s%s%s",
                    cur_logging_path, "boot", end);
            if (!create_dir(destDir)) {
                MLOGE_BOTH("creat %s fail (%s)", destDir, strerror(errno));
                continue;
            }

            snprintf(cmd, sizeof(cmd), "cp -r %s* %s", srcDir, destDir);
            system(cmd);
            copyCnt++;
            MLOGI_BOTH("copy_log: %s", cmd);
            if (strncmp(destDir, "/data/", strlen("/data/")) == 0) {
                DIR *d = opendir(destDir);
                if (d == NULL) {
                    MLOGE_BOTH("open dir %s(%s)", destDir, strerror(errno));
                } else {
                    struct dirent *entry = NULL;
                    while (1) {
                        errno = 0;
                        entry = readdir(d);
                        if (entry == NULL) {
                            if (errno != 0) {
                                MLOGE_BOTH("readdir error(%s)", strerror(errno));
                            }
                            break;
                        }
                        if (!strcmp(entry->d_name, ".") ||
                            !strcmp(entry->d_name, "..")) {
                            continue;
                        }
                        snprintf(cmd, sizeof(cmd), "%s%s", destDir, entry->d_name);
                        if (chown(cmd, AID_SHELL, AID_LOG) < 0) {
                            MLOGE_BOTH("chown fail %s, %s", cmd, strerror(errno));
                        }
                    }
                    if (closedir(d) < 0) {
                        MLOGE_BOTH("close dir error, %s", strerror(errno));
                    }
                }
            }
        } else {
            MLOGI_BOTH("Can not read valid item ang more!");
            break;
        }
    }

    if (fp != NULL) {
        fclose(fp);
    }

copy_check:
    if (copyCnt == 0) {
        snprintf(cmd, sizeof(cmd), "cp -r %s* %s", PATH_TEMP, cur_logging_path);
        system(cmd);
        MLOGI_BOTH("Copy all log directly: %s", cmd);
    }
    snprintf(cmd, sizeof(cmd), "rm -rf %s*", PATH_TEMP);
    system(cmd);
}

void do_copy_and_dump() {
    prctl(PR_SET_NAME, "mobile_log_d.cp");
    IoSchedClass mbISC;
    int pid, mb_io_prio;
    int ret = 0;

    DIR* dirp;
    struct dirent* entry;
    bool needCopy = false;
    int retryCnt = 10;

    MLOGI_BOTH("create thread %d do copy dump", gettid());

    pid = getpid();
    ret = android_get_ioprio(pid, &mbISC, &mb_io_prio);
    if (-1 != ret) {
        MLOGI_BOTH("change ioprio, default:%d,%d", mbISC, mb_io_prio);
        if (android_set_ioprio(pid, IoSchedClass_BE, 7))
            MLOGE_BOTH("set ioprio failed (%s)", strerror(errno));
    } else
        MLOGE_BOTH("get ioprio failed (%s)", strerror(errno));

    dump_system_info();

    if ((dirp = opendir(PATH_TEMP)) != NULL) {
        while ((entry = readdir(dirp))) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;
            if (entry->d_type == DT_DIR) {
                needCopy = true;
                MLOGI_BOTH("This time need copy~~");
                break;
            }
        }
        if (closedir(dirp)) {
            MLOGE_BOTH("Closedir %s fail, (%s)", PATH_TEMP, strerror(errno));
        }
    } else
        MLOGE_BOTH("Opendir %s fail, (%s)", PATH_TEMP, strerror(errno));

    while (needCopy && g_copy_wait && retryCnt--) {
        usleep(10000);  // sleep 10 ms
        MLOGI_BOTH("copy_thread: wait for change path done...");
    }
    if (needCopy)
        copy_to_sdcard();

    if (-1 != ret) {
        if (android_set_ioprio(pid, mbISC, mb_io_prio))
            MLOGE_BOTH("set ioprio back failed (%s)", strerror(errno));
    }

    MLOGI_BOTH("copydump over");
}



void copy_and_dump() {
    pthread_t cydp_t;
    pthread_attr_t attr;

    if (!pthread_attr_init(&attr)) {
        if (!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
            if (!pthread_create(&cydp_t, &attr, (void *)do_copy_and_dump, NULL))
                pthread_attr_destroy(&attr);
        }
    }
}
