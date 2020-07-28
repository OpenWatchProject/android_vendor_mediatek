/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* vprintf, vsnprintf */
#include <stdio.h>

/* exit(), atoi() */
#include <stdlib.h>

/* getopt, optind */
#include <unistd.h>

/* sysenv */
#include <sysenv_utils.h>

/* strcmp */
#include <string.h>

#include <getopt.h>
/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_status.h"
#include "mrdump_support_ext4.h"
#include "mrdump_support_mpart.h"

static void usage(const char *prog) __attribute__((noreturn));
static void usage(const char *prog)
{
    printf("Usage\n"
        "\t%1$s is-supported\n\n"
        "\t%1$s status-get\n"
        "\t%1$s status-log\n"
        "\t%1$s status-clear\n\n"
        "\t%1$s file-setup\n"
        "\t%1$s file-allocate n\n"
        "\t\tn is 0(disable file output) 1(halfmem) 2(fullmem) >256\n"
        "\t%1$s file-extract-core [-r] filename\n"
        "\t\t-r\tre-init pre-allocated file\n"
        "\t%1$s file-info\n\n"
        "\t%1$s mem-size-set n\n"
        "\t\tn is between 64 ~ 16384(m), 0 is output total-mem-size\n\n"
        "\t%1$s output-set output\n"
        "\t\tsetting mrdump output device\n"
        "\t\t  none\n"
        "\t\t  null\n"
        "\t\t  usb\n"
        "\t\t  partition: mrdump partition\n"
        "\t\t  internal-storage: ext4, f2fs\n"
        "\t%1$s output-get\n"
        "\t\tgetting mrdump output device\n",
        prog);
    exit(-1);
}

static void dump_status_ok(const struct mrdump_status_result *result)
{
    printf("Ok\n");
    printf("\tMode: %s\n\tOutput: ", result->mode);

    switch (result->output) {
    case MRDUMP_OUTPUT_NULL:
        printf("null\n");
        break;
    case MRDUMP_OUTPUT_USB:
        printf("usb\n");
        break;
    case MRDUMP_OUTPUT_DATA_FS:
        printf("ext4/data partition\n");
        break;
    case MRDUMP_OUTPUT_PARTITION:
        printf("dynamic mrdump partition\n");
        break;
    default:
        printf("not supported\n");
        break;
    }
}

static int file_setup_command(int argc, char * __attribute__((unused)) argv[])
{
    if (!mrdump_is_supported()) {
        error("file-setup not allowed in this mode\n");
    }
    if (argc != 1) {
        error("Invalid file-setup command argument\n");
    }
    mrdump_file_setup(false);
    return 0;
}

static void file_allocate_command(int argc, char *argv[])
{
    if (!mrdump_is_supported()) {
        error("file-allocate not allowed in this mode\n");
    }
    if (argc != 2) {
        error("Invaid file-allocate command argument\n");
    }
    int size_m = atoi(argv[1]);

    if (size_m < 0)
        size_m = 0;

    if ((size_m <= 2) || (size_m >= MRDUMP_EXT4_MIN_ALLOCATE)) {
        // enable condition: only 0, 1, 2, >256
        mrdump_file_set_maxsize(size_m);
    }
    else {
        error("Invalid dump size %d\n", size_m);
    }
}

static int file_extract_core_command(int argc, char *argv[])
{
    int opt;
    bool reinit = false;

    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r':
            reinit = true;
            break;
        default:
            error("Invalid file-extract-core parameter\n");
        }
    }
    if (optind >= argc) {
        error("Expected filename after options\n");
    }

    const char *fn = argv[optind];
    const char *output_dev = sysenv_get("mrdump_output");

    MD_LOGI("%s: mrdump_output= %s\n",__func__, output_dev);

    if (output_dev) {
        if (reinit && !mrdump_status_clear()) {
            error("MT-RAMDUMP Status clear failed\n");
        }
        if (!strncmp(output_dev, "partition", 9)) {
            MD_LOGI("%s: partition solution\n",__func__);
            if (mrdump_file_fetch_zip_coredump_partition(fn)) {
                return 0;
            }
        }
        else if (!strncmp(output_dev, "internal-storage", 21)) {
            MD_LOGI("%s: ext4 solution\n",__func__);
            if (mrdump_file_fetch_zip_coredump(fn)) {
                mrdump_file_setup(reinit);
                return 0;
            }
        }
    }

    error("Fetching Coredump data failed\n");
}

static int file_info_command(int argc, char * __attribute__((unused)) argv[])
{
    if (argc != 1) {
        error("Invalid file-info command argument\n");
    }

    struct mrdump_pafile_info info;
    if (mrdump_file_get_info(MRDUMP_EXT4_ALLOCATE_FILE, &info)) {
        printf("\tInfo LBA :      %" PRIu32 "\n"
               "\tAddress LBA :   %" PRIu32 "\n"
               "\tFile Size :     %" PRIu64 "\n"
               "\tCoredump Size : %" PRIu64 "\n"
               "\tTimestamp :     %" PRIx64 "\n",
               info.info_lba, info.addr_lba,
               info.filesize, info.coredump_size,
               info.timestamp
              );
        return 0;
    }
    else {
        error("Cannot get pre-allocate file info\n");
    }
}

static void mem_size_set_command(int argc, char *argv[])
{
    char msize[5];

    if (argc != 2) {
        error("Invaid mem-size-set command argument\n");
    }

    int size_m = atoi(argv[1]);
    if ((size_m == 0) || ((size_m >= 64) && (size_m <= 16 * 1024))) {
        if (size_m != 0) {
            snprintf(msize, sizeof(msize), "%d", size_m);
            if (sysenv_set("mrdump_mem_size", msize) == 0) {
                MD_LOGI("mem-size-set done.\n");
            }
        }
        else {
            if (sysenv_set("mrdump_mem_size", "") == 0) {
                MD_LOGI("total-mem-size done.\n");
            }
            else {
                error("failed to set memory dump size, plz try again later.\n");
            }
        }
    }
    else {
        error("Invalid memory dump size\n");
    }
}

static void output_set_command(int argc, char *argv[])
{
    if(argc < 2) {
        error("Invalid output device, valid input [none, null, usb, partition, internal-storage]\n");
    }
    else {
        const char *output_dev = argv[1];
        int need_reboot = 0;
        const char *prev_output_dev = sysenv_get("mrdump_output");

        if (prev_output_dev) {
            if (strcmp(prev_output_dev, "partition") == 0) {
                if (mrdump_check_partition()) {
                    need_reboot = 1;
                }
            }
        }
        if (strcmp(output_dev, "partition") == 0) {
            if (!mrdump_check_partition()) {
                error("mrdump partition doesn't exist, cannot dump to partition.\n");
            }
            need_reboot = 1;
        }
        else if (strcmp(output_dev, "none") &&
                 strcmp(output_dev, "null") &&
                 strcmp(output_dev, "usb") &&
                 strcmp(output_dev, "internal-storage")) {
            error("Unknown output %s\n", output_dev);
        }

        if (sysenv_set("mrdump_output", output_dev) == 0) {
            MD_LOGI("mrdump_output = %s\n", output_dev);
        }
        else {
            error("output-set failed.(%s)\n", output_dev);
        }

        if (strcmp(output_dev, "internal-storage") == 0) {
            mrdump_file_set_maxsize(DEFAULT_FULLMEM);
        }
        else {
            mrdump_file_set_maxsize(DEFAULT_DISABLE);
        }

        if (need_reboot) {
            if (0 > execl("/system/bin/reboot", "reboot", NULL, NULL))
                error("%s: failed to reboot into LK for partition resize.\n", __func__);
        }
    }
}

static void status_get_command(int __attribute((unused)) argc,
                               char * __attribute ((unused)) argv[])
{
    struct mrdump_status_result result;
    if (mrdump_status_get(&result)) {
        printf("MT-RAMDUMP\n\tStatus:");
        switch (result.status) {
        case MRDUMP_STATUS_NONE:
            printf("None\n");
            break;
        case MRDUMP_STATUS_FAILED:
            printf("Failed\n");
            break;
        case MRDUMP_STATUS_OK:
            dump_status_ok(&result);
            break;
        }
    }
    else {
        error("MT-RAMDUMP get status failed\n");
    }
}

static int parse_log_level(const char *log_level)
{
    if (strcmp(log_level, "debug") == 0)
        return LOG_DEBUG;
    else if (strcmp(log_level, "info") == 0)
        return LOG_INFO;
    else if (strcmp(log_level, "warn") == 0)
        return LOG_WARNING;
    else if (strcmp(log_level, "error") == 0)
        return LOG_ERR;
    return LOG_WARNING;
}

int main(int argc, char *argv[])
{
    int log_level = LOG_WARNING;
    int log_syslog = 0;
    bool reset_optind = false;

    static struct option long_options[]= {
        {"help", no_argument, 0, 0},
        {"log-level", required_argument, 0, 0},
        {"log-syslog", no_argument, 0, 0},
        {0, 0, 0, 0},
    };

    while (1) {
        int option_index = 0, c;
        c = getopt_long(argc, argv, "+", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            if (strcmp(long_options[option_index].name, "log-level") == 0) {
                log_level = parse_log_level(optarg);
                reset_optind = true;
            }
            else if (strcmp(long_options[option_index].name, "log-syslog") == 0) {
                log_syslog = 1;
                reset_optind = true;
            }
            else if (strcmp(long_options[option_index].name, "help") == 0) {
                usage(argv[0]);
            }
            break;

        default:
            error("Invalid arguments\n");
        }
    }

    mdlog_init(log_level, log_syslog);

    const int command_argc = argc - optind;
    if (command_argc < 1) {
        error("No command given\n");
    }
    const char *command = argv[optind];
    char **command_argv = &argv[optind];
    if(reset_optind)
        optind = 1;

    if (strcmp(command, "is-supported") == 0) {
        if (mrdump_is_supported()) {
            printf("MT-RAMDUMP support ok\n");
            return 0;
        }
        else {
            printf("MT-RAMDUMP not support\n");
            return 1;
        }
    }
    else if (strcmp(command, "status-get") == 0) {
        status_get_command(command_argc, command_argv);
    }
    else if (strcmp(command, "status-log") == 0) {
        struct mrdump_status_result result;
        if (!mrdump_status_get(&result)) {
            error("MT-RAMDUMP get status failed\n");
        }
        printf("=>status line:\n%s\n=>log:\n%s\n", result.status_line, result.log_buf);
    }
    else if (strcmp(command, "status-clear") == 0) {
        if (!mrdump_is_supported()) {
            error("MT-RAMDUMP not support\n");
        }
        if (!mrdump_status_clear()) {
            error("MT-RAMDUMP Status clear failed\n");
        }
    }
    else if (strcmp(command, "file-setup") == 0) {
        file_setup_command(command_argc, command_argv);
    }
    else if (strcmp(command, "file-allocate") == 0) {
        file_allocate_command(command_argc, command_argv);
    }
    else if (strcmp(command, "file-extract-core") == 0) {
        file_extract_core_command(command_argc, command_argv);
    }
    else if (strcmp(command, "file-info") == 0) {
        file_info_command(command_argc, command_argv);
    }
    else if (strcmp(command, "mem-size-set") == 0) {
        mem_size_set_command(command_argc, command_argv);
    }
    else if (strcmp(command, "output-set") == 0) {
        output_set_command(command_argc, command_argv);
    }
    else if (strcmp(command, "partition") == 0) {
        /* ignored "-Wunused-result" */
        if (1 == mrdump_check_partition()) {;}
    }
    else if (strcmp(command, "output-get") == 0) {
        const char *output_dev = sysenv_get("mrdump_output");
        if (!output_dev) {
            printf("no mrdump output device setting\n");
        }
        else {
            printf("%s\n", output_dev);
        }
    }
    else {
        error("Unknown command %s\n", command);
    }

    return 0;
}
