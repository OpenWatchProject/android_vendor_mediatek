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
 * MediaTek Inc. (C) 2012. All rights reserved.
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

#ifdef MTK_USE_RESERVED_EXT_MEM

#include <sys/mman.h>
#include <ctype.h>
#include <sys/prctl.h>

#include "malloc_debug_mtk.h"


// behavior of "-a" and "-b" arguments
//#define ALL_OR_NONE_TO_DEFAULT

#define MAX_ARGS    256
#define BUF_SIZE    (4 * 1024)
#define NAME_LEN    28

struct DebugProcessConfig {
    char name[NAME_LEN];
    DebugConfig config;
};

struct DebugConfigTable {
    //pthread_mutex_t lock;
    unsigned long count;
    struct DebugProcessConfig configs[0];
};

static struct DebugConfigTable *gDebugConfigTable = NULL;

#define CONFIG_MAP_SIZE     (1 * 4096)
#define CONFIG_TABLE_SIZE   ((CONFIG_MAP_SIZE - sizeof(struct DebugConfigTable)) / sizeof(struct DebugProcessConfig))

#define ASHMEM_DEVICE       "/dev/ashmem"
#define ASHMEM_CONFIG_NAME  "malloc-debug-config"

#ifndef PR_SET_VMA
#define PR_SET_VMA   0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME    0
#endif

static int init_config_table(void)
{
#if 0
    int fd, ret;

    // TODO: use shared memory
    fd = open(ASHMEM_DEVICE, O_RDWR);
    if (fd < 0)
        goto anon;

    ret = ioctl(fd, ASHMEM_SET_NAME, ASHMEM_CONFIG_NAME);
    if (ret < 0)
        goto error;

    ret = ioctl(fd, ASHMEM_SET_SIZE, CONFIG_MAP_SIZE);
    if (ret < 0)
        goto error;

    gDebugConfigTable = mmap(NULL, CONFIG_MAP_SIZE,
            PROT_READ | PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_PRIVATE, fd, 0);
    if (gDebugConfigTable == MAP_FAILED)
        goto error;

out:
    gDebugConfigTable->count = 1;
    return 0;

error:
    close(fd);
anon:
    gDebugConfigTable = mmap(NULL, CONFIG_MAP_SIZE,
            PROT_READ | PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (gDebugConfigTable == MAP_FAILED) {
        gDebugConfigTable = NULL;
        return -1;
    }

    goto out;
#else
    gDebugConfigTable = mmap(NULL, CONFIG_MAP_SIZE,
            PROT_READ | PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (gDebugConfigTable == MAP_FAILED) {
        gDebugConfigTable = NULL;
        return -1;
    }
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, gDebugConfigTable,
          CONFIG_MAP_SIZE, ASHMEM_CONFIG_NAME);
    gDebugConfigTable->count = 1;
    return 0;
#endif
}

static int get_config(const char *name, DebugConfig *config)
{
    unsigned long i;

    if (gDebugConfigTable == NULL)
        return 0;

    assert(gDebugConfigTable->count);
    assert(name && config);

    for (i = 1; i < gDebugConfigTable->count; i++) {
        if (strstr(name, gDebugConfigTable->configs[i].name)) {
         *config = gDebugConfigTable->configs[i].config;
            if (config->mDebugMspaceSize == (uint32_t)-1) {
                debug_log("%s is blocked\n", name);
                return -1;
            }
            else
                return 0;
        }
    }

    if (gDebugConfigTable->configs[0].name[0]) {
        *config = gDebugConfigTable->configs[0].config;
        debug_log("%s is not found; use default config, mspace size:0x%x\n", name, config->mDebugMspaceSize);
        if (config->mDebugMspaceSize == (uint32_t)-1) {
            debug_log("%s is blocked\n", name);
            return -1;
        }
        else
            return 0;
    }

    debug_log("%s is not found", name);
    return -1;
}

static void set_default_config(DebugConfig *config)
{
    assert(gDebugConfigTable && gDebugConfigTable->count);
    assert(config);

    strcpy(gDebugConfigTable->configs[0].name, "<ALL>");
    gDebugConfigTable->configs[0].config = *config;
}

static void reset_default_config(void)
{
    assert(gDebugConfigTable && gDebugConfigTable->count);
    memset(&gDebugConfigTable->configs[0], 0, sizeof(struct DebugProcessConfig));
}

static int set_config(char *name, DebugConfig *config)
{
    unsigned long i;

    assert(gDebugConfigTable && gDebugConfigTable->count);
    assert(name && config);

    for (i = 1; i < gDebugConfigTable->count; i++) {
        if (strncmp(name, gDebugConfigTable->configs[i].name, NAME_LEN - 1) == 0) {
            gDebugConfigTable->configs[i].config = *config;
            return 0;
        }
    }

    assert(i == gDebugConfigTable->count);
    if (i == CONFIG_TABLE_SIZE)
        return -1;

    strncpy(gDebugConfigTable->configs[i].name, name, NAME_LEN - 1);
    gDebugConfigTable->configs[i].name[NAME_LEN - 1] = '\0';
    gDebugConfigTable->configs[i].config = *config;
    gDebugConfigTable->count++;
    return 0;
}

static uint32_t config_to_uint(DebugConfig *cs)
{
    uint32_t config;
    assert(cs);
    config  = ((cs->mDebugMspaceSize / DEBUG_MSPACE_SIZE_UNIT)) & DEBUG_MSPACE_SIZE_MASK;
    config |= ((cs->mHistoricalBufferSize / HISTORICAL_BUFFER_SIZE_UNIT) << 12) & HISTORICAL_BUFFER_SIZE_MASK;
    config |= ((cs->mMaxBtDepth / BT_DEPTH_UNIT) << 24) & MAX_BT_DEPTH_MASK;
    config |= ((cs->mBtMethod << 29) & UNWIND_BT_MASK);
    config |= ((cs->mSig == 1) ? SIG_HANDLER_MASK : 0);
    config |= ((cs->mDebugMspaceSource == EXTERNAL_MEM) ? DEBUG_MSPACE_SOURCE_MASK : 0);
    return config;
}

static void dump_configs(void)
{
    unsigned long i;

    if (gDebugConfigTable == NULL)
        return;

    assert(gDebugConfigTable->count);

    for (i = 1; i < gDebugConfigTable->count; i++)
        info_log("%08X %s", config_to_uint(&gDebugConfigTable->configs[i].config), gDebugConfigTable->configs[i].name);

    if (gDebugConfigTable->configs[0].name[0])
        info_log("%08X %s", config_to_uint(&gDebugConfigTable->configs[0].config), gDebugConfigTable->configs[0].name);
}


static long get_var(char *str, char *varname, uint32_t *var)
{
    char *endptr;

    if (strncmp(str, varname, strlen(varname)) == 0) {
        if (strlen(str) > strlen(varname)) {
            errno = 0;
            *var = (uint32_t)strtoul(str + strlen(varname), &endptr, 0);

            if (errno < 0) {
                error_log("errno=%d", errno);
                return -1;
            }
            
            if (*var == (uint32_t)-1) {
                return 0;
            }

            if (endptr && endptr[0]) {
                if (endptr[0] == 'm' || endptr[0] == 'M')
                    *var *= 1024*1024;
                else if (endptr[0] == 'k' || endptr[0] == 'K')
                    *var *= 1024;
            }
            return 0;
        }
    }
    return -1;
}

static int parse_vars(int argc, char **argv, DebugConfig *config)
{
    DebugConfig cs = DEFAULT_DEBUG_CONFIG;
    int i;

    for (i = 0; i < argc; i++) {
#define PARSE_VAR(n, v)                         \
        if (get_var(argv[i], (n), &var) == 0) { \
            (v) = var;                          \
            continue;                           \
        }

        uint32_t var;
        PARSE_VAR("size=", cs.mDebugMspaceSize);
        PARSE_VAR("his=", cs.mHistoricalBufferSize);
        PARSE_VAR("btdepth=", cs.mMaxBtDepth);
        PARSE_VAR("sig=", cs.mSig);
        PARSE_VAR("btmethod=", cs.mBtMethod);
        PARSE_VAR("ext=", cs.mDebugMspaceSource);
        break;

#undef PARSE_VAR
    }
    if (i) {
        *config = cs;
    }

    return i;
}

static int add_process(int argc, char **argv)
{
    DebugConfig config;
    int nr_proc = 1;
    int ret = 0;
    int i;

#ifndef ALL_OR_NONE_TO_DEFAULT
    if (argc < 2)
        return -1;
#endif

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-')
            break;
        ret = parse_vars(argc - i, argv + i, &config);
        if (ret) {
            nr_proc = i;
            break;
        }
    }

    if (ret == 0)
        return -1;
#ifdef ALL_OR_NONE_TO_DEFAULT
    if (i == 1)
        set_default_config(&config);
#endif

    for (i = 1; i < nr_proc; i++) {
#ifdef ALL_OR_NONE_TO_DEFAULT
        if (strcmp(argv[i], "all") == 0)
            set_default_config(&config);
        else
#endif
        if (set_config(argv[i], &config))
            return -1;
    }

    return nr_proc + ret;
}

static int add_all_processes(int argc, char **argv)
{
    DebugConfig config;
    int ret;

    if (argc < 2)
        return -1;

    ret = parse_vars(argc - 1, argv + 1, &config);
    if (ret)
        set_default_config(&config);

    return 1 + ret;
}

static int block_process(int argc, char **argv)
{
    DebugConfig config = {
        .mDebugMspaceSize = (uint32_t)-1
    };
    int i;

#ifndef ALL_OR_NONE_TO_DEFAULT
    if (argc < 2)
        return -1;
#endif

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-')
            break;
#ifdef ALL_OR_NONE_TO_DEFAULT
        if (strcmp(argv[i], "all") == 0)
            reset_default_config();
        else
#endif
        if (set_config(argv[i], &config))
            return -1;
    }

#ifdef ALL_OR_NONE_TO_DEFAULT
    if (i == 1)
        reset_default_config();
#endif

    return i;
}

static int block_all_processes(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    reset_default_config();
    return 1;
}

static int process_config_file(char *filename);
static int add_config_file(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    if (process_config_file(argv[1]))
        return -1;

    return 2;
}

static long read_config_file(char *filename, char *buf, long buf_sz)
{
    int fd;

    assert(buf);

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        error_log("file %s is not found", filename);
        return -errno;
    }

    buf_sz = read(fd, buf, buf_sz - 1);
    close(fd);
    if (buf_sz < 0)
        return -errno;

    buf[buf_sz] = 0;
    return buf_sz;
}

static int buf_to_args(char *buf, long buf_sz, char **argv)
{
    int argc = 0;
    long last;
    long i;

    for (i = 0, last = 0; i < buf_sz; i++) {
        if (buf[i] == '\0' || isspace(buf[i])) {
            buf[i] = '\0';
            last = i + 1;

        } else if (last >= 0) {
            argv[argc] = &buf[last];
            argc++;
            last = -1;

            if (argc == MAX_ARGS) {
                assert(0);
                return argc;
            }
        }
    }
    return argc;
}

static void process_args(int argc, char **argv)
{
    int inc = 0;
    int i;

    for (i = 1; i < argc; i += inc) {
#define PROCESS_ARG(a, f)               \
        if (strcmp(argv[i], (a)) == 0) {    \
            inc = (f)(argc - i, &argv[i]);  \
            if (inc < 0)                    \
                break;                      \
            continue;                       \
        }

        PROCESS_ARG("-a", add_process);
        PROCESS_ARG("-all", add_all_processes);
        PROCESS_ARG("-b", block_process);
        PROCESS_ARG("-ball", block_all_processes);
        PROCESS_ARG("-f", add_config_file);
        error_log("unknown option %s", argv[i]);

#undef PROCESS_ARG
    }
}

static int process_config_file(char *filename)
{
    char buf[BUF_SIZE];
    long buf_sz;
    char *argv[MAX_ARGS];
    int argc;

    buf_sz = read_config_file(filename, (char *)&buf[0], BUF_SIZE);
    if (buf_sz < 0) {
        return -1;
    }
    else if(buf_sz == BUF_SIZE) {
        error_log("process_config_file buf full, need enlarge\n");
        return -1;
    }

    argv[0] = filename;
    argc = 1;
    argc += buf_to_args(buf, buf_sz, argv + 1);

    process_args(argc, argv);
    return 0;
}


void mtk_malloc_debug_read_config(char *filename)
{
    if (init_config_table())
        return;

    if (process_config_file(filename)) {
        munmap(gDebugConfigTable, CONFIG_MAP_SIZE);
        gDebugConfigTable = NULL;
    }
    //dump_configs();
}

int mtk_malloc_debug_set_config(const char *name)
{
    if (name == NULL)
        return -1;
    if (get_config(name, &gDebugConfig)) {
        munmap(gDebugConfigTable, CONFIG_MAP_SIZE);
        gDebugConfigTable = NULL;
        return -1;
    }
    return 0;
}

#endif /* MTK_USE_RESERVED_EXT_MEM */

