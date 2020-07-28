/**
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <linux/limits.h>
#include "bperf_util.h"

#include "common.h"

//---------------------------------------------------------------------------
#define VERSION     "6.0.18102901"
#define LOG_VERSION 0x100
#define BT_DEV      "/sys/kernel/debug/mtkbt/bt_dev"
#define FWLOG_DEV   "/dev/stpbtfwlog"

//---------------------------------------------------------------------------
static const uint64_t BTSNOOP_EPOCH_DELTA = 0x00dcddb30f2f8000ULL;
static uint64_t timestamp = 0;
static uint8_t buffer[1944] = {0};
static uint8_t cont = 1;    /** loop continue running */
static int file_size_remain_to_switch = 0;
static uint8_t bperf_start = 0;

//---------------------------------------------------------------------------
uint64_t btsnoop_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // Timestamp is in microseconds.
    timestamp = tv.tv_sec * 1000000ULL;
    timestamp += tv.tv_usec;
    timestamp += BTSNOOP_EPOCH_DELTA;
    return timestamp;
}

//---------------------------------------------------------------------------
void fillheader(unsigned char *header, int headerlen,
        unsigned short int dump_file_seq_num)
{
    int copy_hedare_len = 0;
    unsigned int logversion = htobe32(LOG_VERSION);
    memset(header, 0, headerlen);
    memcpy(header, &logversion, sizeof(logversion));
    copy_hedare_len += 4;   /** 4 byte for logversion */
    copy_hedare_len += 4;   /** 4 byte for chip id, not implement yet */
    dump_file_seq_num = htobe16(dump_file_seq_num);
    memcpy(header + copy_hedare_len, &dump_file_seq_num, sizeof(dump_file_seq_num));
    copy_hedare_len += 2;   /** 2 byte for sequence number */
    copy_hedare_len += 6;   /** first hci log length(2), zero(4) */

    btsnoop_timestamp();
    timestamp = htobe64(timestamp);
    memcpy(header + copy_hedare_len, &timestamp, sizeof(timestamp));
}

//---------------------------------------------------------------------------
static void picus_sig_handler(int signum)
{
    DBGPRINT(MT_DEBUG_SHOW, "%s: %d\n", __func__, signum);
    system("echo 01 be fc 01 00 > "FWLOG_DEV);      // disable picus log
    cont = 0;                                       // stop loop
    if (bperf_start) {
        system("echo bperf=0 > "FWLOG_DEV);         // disable bperf
        bperf_uninit();
    }
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    FILE *fscript = 0;
    FILE *fw_dump_fscript = 0;
    int nRead = 0;
    int fd = 0;
    int ret = 0;
    int opt;
    int dump_name_index = 0;
    int writetofilelength = 0;
    char dump_file_name[64] = {0};
    int fw_dump_writetofilelength = 0;
    char fw_dump_file_name[64] = {0};
    int retry_file_open = 0;
    int file_number = 6;
    int file_size = FW_LOG_SWITCH_SIZE;     /** default file size is 20 MB */
    char *log_path = DEFAULT_PATH;
    char command[200] = {0};
    unsigned char padding[8] = {0};
    unsigned char header[24] = {0};
    int fw_log_len = 0;
    unsigned short int dump_file_seq_num = 0;
    char timestamp_buffer[24];
    fd_set rset;                            /** For select */
    struct timeval tv;
    time_t local_timestamp;
    uint8_t logmore = 0;                    /** Default log level */
    struct sigaction sigact;
    struct flock fl;
    char rssi_per_package = 0;
    char get_afh = 0;
    char picus_fullname[PATH_MAX + NAME_MAX + 2] = {0};
    uint16_t handle = 0;

    DBGPRINT(MT_DEBUG_SHOW, "%s Version: %s", argv[0], VERSION);
    sigact.sa_handler = picus_sig_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGKILL, &sigact, NULL);
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_pid = getpid();
    fl.l_start = 0;
    fl.l_len = 0;

    while ((opt = getopt(argc, argv, "d:c:p:n:s:f")) != -1) {
        struct stat sb;
        switch (opt) {
        /* debug */
        case 'd':
            if (strcmp(optarg, "kill") == 0) {
                system("echo 01 be fc 01 00 > "FWLOG_DEV);     // disable picus log firstly
                system("killall picus");
                DBGPRINT(MT_DEBUG_SHOW, "Kill all picus process.\n");
                goto done;
            } else if (strcmp(optarg, "trigger") == 0) {
                system("echo 01 be fc 01 00 > "FWLOG_DEV);     // disable picus log firstly
                DBGPRINT(MT_DEBUG_SHOW, "Manual Trigger FW Assert.\n");
                system("echo 01 6f fc 05 01 02 01 00 08 > "FWLOG_DEV);
                return 0;
            } else if (strcmp(optarg, "rssi") == 0) {
                DBGPRINT(MT_DEBUG_SHOW, "Send read rssi command.\n");
                int i;
                for (i = 0; i < 6; i++) {
                    char command[64] = {0};
                    int default_bredr_handle = 32;
                    /* Send Read RSSI command for bredr, handle is 0x0032 ~ 0x0037 */
                    sprintf(command, "echo 01 61 FC 02 %d 00 > /dev/stpbtfwlog", default_bredr_handle + i);
                    system(command);
                    usleep(10000);
                    /* Send Read RSSI command for LE, handle is 0x0200 ~ 0x0205 */
                    sprintf(command, "echo 01 61 FC 02 %02d 02 > /dev/stpbtfwlog", i);
                    system(command);
                    usleep(10000);
                }
                return 0;
            } else if (strcmp(optarg, "per") == 0) {
                DBGPRINT(MT_DEBUG_SHOW, "Send read per command.\n");
                system("echo 01 11 FD 00 > "FWLOG_DEV);
                return 0;
            } else if (strcmp(optarg, "bperf") == 0) {
                DBGPRINT(MT_DEBUG_SHOW, "Start bperf.\n");
                system("echo bperf=1 > "FWLOG_DEV);             // enable bperf
                bperf_init();
                bperf_start = 1;
            } else if (strcmp(optarg, "afh") == 0) {
                get_afh = 1;
                break;
            } else if (strcmp(optarg, "en_rssi") == 0) {
                rssi_per_package = 1;
                break;
            } else if (strcmp(optarg, "dis_rssi") == 0) {
                rssi_per_package = 2;
                break;
            }
            break;
        /* send command */
        case 'c':
            snprintf(command, sizeof(command), "echo %s > "FWLOG_DEV, optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Send command = %s from users.\n",command);
            system(command);
            return 0;
        /* change path */
        case 'p':
            if (stat(optarg, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                log_path = optarg;
                DBGPRINT(MT_DEBUG_SHOW, "Log path is %s\n", log_path);
            } else {
                DBGPRINT(MT_DEBUG_SHOW, "Directory is invalid");
                goto done;
            }
            break;
        /* change file number*/
        case 'n':
            file_number = atoi(optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Change the number of file to %d.\n", file_number);
            break;
        /* change file size*/
        case 's':
            file_size = atoi(optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Change the size of file to %d.\n", file_size);
            break;
        /* full log */
        case 'f':
            logmore = 1;
            break;
        /* command Usage */
        case '?':
        default:
            DBGPRINT(MT_DEBUG_SHOW, "Usage: picus [option] [path | command]");
            DBGPRINT(MT_DEBUG_SHOW, "[option]");
            DBGPRINT(MT_DEBUG_SHOW, "\t-d [command]\tSend debug command");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"kill\" command to kill all picus");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"trigger\" command to trigger fw assert");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"rssi\" command to read rssi");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"afh\" command to read afh table");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"per\" command to read per");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"en_rssi\" command to enable read rssi/channel every package");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"dis_rssi\" command to disable read rssi/channel every package");
            DBGPRINT(MT_DEBUG_SHOW, "\t-c [command]\tsend command");
            DBGPRINT(MT_DEBUG_SHOW, "\t-p [path]\tOutput the file to specific dictionary");
            DBGPRINT(MT_DEBUG_SHOW, "\t-n [NO]\t\tChange the output file number");
            DBGPRINT(MT_DEBUG_SHOW, "\t-s [bytes]\tChange the output file size");
            DBGPRINT(MT_DEBUG_SHOW, "\t-f\t\tLog level: More");
            goto done;
            break;
        }
    }
    if (rssi_per_package > 0) {
        char command[64] = {0};
        printf("%s", argv[optind]);
        handle = (uint16_t)strtoul(argv[optind], NULL, 16);
        if (rssi_per_package == 1) {
            sprintf(command, "echo 01 72 FD 03 01 %02X %02X > %s", (uint8_t)(handle & 0x00FF), (uint8_t)((handle & 0xFF00) >> 8), FWLOG_DEV);
            DBGPRINT(MT_DEBUG_SHOW, "Send enable rssi/afh log command.\n");
            system(command);
        }
        else {
            sprintf(command, "echo 01 72 FD 03 00 %02X %02X > %s", (uint8_t)(handle & 0x00FF), (uint8_t)((handle & 0xFF00) >> 8), FWLOG_DEV);
            DBGPRINT(MT_DEBUG_SHOW, "Send disable rssi/afh log command.\n");
            system(command);
        }
        return 0;
    }
    if (get_afh > 0) {
        char command[64] = {0};
        handle = (uint16_t)strtoul(argv[optind], NULL, 16);
        sprintf(command, "echo 01 06 14 02 %02X %02X > %s", (uint8_t)(handle & 0x00FF), (uint8_t)((handle & 0xFF00) >> 8), FWLOG_DEV);
        DBGPRINT(MT_DEBUG_SHOW, "Send get afh command.\n");
        system(command);
        return 0;
    }

    /* stpbtfwlog */
    fd = open(CUST_BT_FWLOG_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <= 0) {
        DBGPRINT(MT_DEBUG_ERROR, "Can't open device node %s, fd: %d", CUST_BT_FWLOG_PORT, fd);
        goto done;
    } else {
        DBGPRINT(MT_DEBUG_SHOW, "Open device node successfully");
    }

    /* flock the device node */
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        DBGPRINT(MT_DEBUG_SHOW, "lock device node failed, picus already running.");
        goto done;
    }

    /* log level */
    if (logmore) {
        ret = system("echo 01 5f fc 2e 50 01 0A 00 00 00 01 00 00 E0 00 00 00 00 00 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00 01 01 01 00 01 00 00 00 01 00 00 00 00 00 00 00 > "FWLOG_DEV);    // Log More
        ;
    } else {
        ret = system("echo 01 5f fc 2e 50 01 0A 00 00 00 00 00 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 > "FWLOG_DEV);    // Default Level
        ;
    }
    if (ret == -1)
        DBGPRINT(MT_DEBUG_ERROR, "Set log level fail");

    /* enable it firstly */
    ret = system("echo 01 be fc 01 05 > "FWLOG_DEV);
    if (ret == -1)
        DBGPRINT(MT_DEBUG_ERROR, "Enable fail");
    DBGPRINT(MT_DEBUG_SHOW, "Log %slevel set and enabled", logmore ? "more " : "");
    usleep(10000);

    /* check already exist file under log_path */
    char temp_picus_zero_filename[36] = {0};
    DIR *p_dir = opendir(log_path);
    if (p_dir != NULL) {
        struct dirent *p_file;
        while ((p_file = readdir(p_dir)) != NULL) {
            /* ignore . and .. directory */
            if (strncmp(p_file->d_name, "..", 2) == 0
                || strncmp(p_file->d_name, ".", 1) == 0) {
                continue;
            }
            /* Remove the old log */
            if (strstr(p_file->d_name, DUMP_PICUS_NAME_EXT) != NULL) {
                snprintf(temp_picus_zero_filename, sizeof(temp_picus_zero_filename), "%s", p_file->d_name);
                memset(picus_fullname, 0, sizeof(picus_fullname));
                snprintf(picus_fullname, sizeof(picus_fullname), "%s/%s", log_path, temp_picus_zero_filename);
                if (remove(picus_fullname)) {
                    DBGPRINT(MT_DEBUG_SHOW, "The old log:%s can't remove", temp_picus_zero_filename);
                } else {
                    DBGPRINT(MT_DEBUG_SHOW, "The old log:%s remove", temp_picus_zero_filename);
                }
            }
        }
        closedir(p_dir);
    }

    /* get current timestamp */
    time(&local_timestamp);
    strftime(timestamp_buffer, 24, "%Y%m%d%H%M%S", localtime(&local_timestamp));
    snprintf(dump_file_name, sizeof(dump_file_name), "%s/" DUMP_PICUS_NAME_PREFIX "%s_%d" DUMP_PICUS_NAME_EXT, log_path, timestamp_buffer, dump_name_index);

    /* combine file path and file name */
    snprintf(fw_dump_file_name, sizeof(fw_dump_file_name), "%s/" FW_DUMP_PICUS_NAME, log_path);

    /* dump file for picus log */
    if ((fscript = fopen(dump_file_name, "wb")) == NULL) {
        DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d", dump_file_name, strerror(errno), errno);
        goto done;
    } else {
        DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", dump_file_name);
    }

    fillheader(header, sizeof(header), dump_file_seq_num);
    dump_file_seq_num++;
    fwrite(header, 1, sizeof(header), fscript);
    fwrite(padding, 1, sizeof(padding), fscript);

    ret = 0;
    retry_file_open = 0;
    file_size_remain_to_switch = file_size;

    do {
        FD_ZERO(&rset);
        FD_SET(fd,&rset);
        tv.tv_sec = 10;
        tv.tv_usec = 0;   /* timeout is 10s for select method */
        if (select(fd + 1, &rset, NULL, NULL, &tv) == 0) {
            DBGPRINT(MT_DEBUG_ERROR, "Read data timeout from stpbtfwlog, timeout is 10s");
            continue;
        }
        if (!FD_ISSET(fd, &rset))
            continue;

        /* Read packet header and length from driver fwlog queue
        ret = read(fd, buffer, 2);
        nRead = ret;
        DBGPRINT(MT_DEBUG_ERROR, "Read data nRead = %d",nRead);
        Read payload from driver fwlog queue
        ret = read(fd, &buffer[2], buffer[1]);
        nRead += ret;
        DBGPRINT(MT_DEBUG_ERROR, "Read data nRead = %d, %02x",nRead, buffer[1]);*/

        /* Read all packet from driver fwlog queue */
        ret = read(fd, buffer, sizeof(buffer));
        nRead = ret;

        if (nRead >= 3) {
            if ((buffer[0] == 0xFF && buffer[2] == 0x50) ||
                (buffer[0] == 0xFF && buffer[1] == 0x05)) {
                /* Picus Event format : FF xx 50 */
                /* Picus ACL format : FF 50 xx xx */
                /* xx is length */
                /* process multiple packet from fwlog queue */
                int index = 0;
                while (index  < nRead) {
                    if (buffer[0] == 0xFF && buffer[2] == 0x50) {
                        writetofilelength = buffer[1 + index] - 1;
                        fw_log_len = buffer[1 + index] - 1;
                        fwrite(&buffer[PICUS_EVENT_LEN + index], 1, writetofilelength, fscript);
                    } else if (buffer[0] == 0xFF && buffer[1] == 0x05) {
                        writetofilelength = (buffer[3 + index] << 8) + buffer[2 + index];
                        fw_log_len = (buffer[3 + index] << 8) + buffer[2 + index];
                        fwrite(&buffer[PICUS_ACL_LEN + index], 1, writetofilelength, fscript);
                    }

                    file_size_remain_to_switch -= writetofilelength;

                    if (writetofilelength % 8) {
                        fwrite(padding, 1, 8 - (fw_log_len % 8), fscript);
                        file_size_remain_to_switch -= (8 - (fw_log_len % 8));
                    }

                    /* switch file name if file size is over file_size */
                    if (file_size_remain_to_switch <= 0) {
                        file_size_remain_to_switch = file_size;
                        fclose(fscript);
                        if (file_number - 1 > dump_name_index) {
                            dump_name_index++;
                        } else {
                            dump_name_index = 0;
                        }
                        /* remove the file before creating */
                        DIR *p_dir = opendir(log_path);
                        if (p_dir != NULL) {
                            struct dirent *p_file;
                            while ((p_file = readdir(p_dir)) != NULL) {
                                if (strncmp(p_file->d_name, "..", 2) == 0
                                    || strncmp(p_file->d_name, ".", 1) == 0) {
                                    continue;
                                }
                                char temp_picus_filename[24] = {0};
                                snprintf(temp_picus_filename, sizeof(temp_picus_filename), "_%d.picus", dump_name_index);
                                if (strstr(p_file->d_name, temp_picus_filename) != NULL) {
                                    memset(picus_fullname, 0, sizeof(picus_fullname));
                                    snprintf(picus_fullname, sizeof(picus_fullname), "%s/%s", log_path, p_file->d_name);
                                    if (remove(picus_fullname)) {
                                        DBGPRINT(MT_DEBUG_SHOW, "%s can't remove", picus_fullname);
                                    } else {
                                        DBGPRINT(MT_DEBUG_SHOW, "%s remove", picus_fullname);
                                    }
                                }
                            }
                            closedir(p_dir);
                        }
                        time(&local_timestamp);
                        strftime(timestamp_buffer, 24, "%Y%m%d%H%M%S", localtime(&local_timestamp));
                        snprintf(dump_file_name, sizeof(dump_file_name), "%s/" DUMP_PICUS_NAME_PREFIX "%s_%d" DUMP_PICUS_NAME_EXT, log_path, timestamp_buffer, dump_name_index);

                        while(1) {
                            if ((fscript = fopen(dump_file_name, "wb")) == NULL) {
                                DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d",
                                        dump_file_name, strerror(errno), errno);
                                if (retry_file_open >= RETRY_COUNT)
                                    goto done;
                            } else {
                                DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", dump_file_name);
                                retry_file_open = 0;
                                break;
                            }
                            ++retry_file_open;
                        }

                        fillheader(header, sizeof(header), dump_file_seq_num);
                        dump_file_seq_num++;
                        fwrite(header, 1, sizeof(header), fscript);
                    }
                    fflush(fscript);
                    if (buffer[0] == 0xFF && buffer[2] == 0x50) {
                        /* Add Picus event header len and payload */
                        /* EVENT Header : 3 bytes, payload : 240 bytes */
                        index += PICUS_EVENT_LEN + fw_log_len;
                    } else if (buffer[0] == 0xFF && buffer[1] == 0x05) {
                        /* Add Picus ACL header len and payload */
                        /* ACL Header : 4 bytes, payload : 240 bytes */
                        index += PICUS_ACL_LEN + fw_log_len;
                    }
                }
            } else if (buffer[0] == 0x6F && buffer[1] == 0xFC) {
                /* dump file for fw dump */
                if (fw_dump_fscript == NULL) {
                    while(1) {
                        if ((fw_dump_fscript = fopen(fw_dump_file_name, "wb")) == NULL) {
                            DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d", fw_dump_file_name,
                                    strerror(errno), errno);
                            if (retry_file_open >= RETRY_COUNT)
                                goto done;
                        } else {
                            DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", fw_dump_file_name);
                            retry_file_open = 0;
                            break;
                        }
                        ++retry_file_open;
                    }
                }
                fw_dump_writetofilelength = nRead - 4;
                if (buffer[nRead - 6] == ' ' &&
                    buffer[nRead - 5] == 'e' &&
                    buffer[nRead - 4] == 'n' &&
                    buffer[nRead - 3] == 'd') {
                    DBGPRINT(MT_DEBUG_SHOW, "FW dump end");
                }
                fwrite(&buffer[4], 1, fw_dump_writetofilelength, fw_dump_fscript);
                fflush(fw_dump_fscript);
            } else if (buffer[4] == 0x61 && buffer[5] == 0xFC) {
                int rssi = (int)(buffer[9]);
                if (rssi) {
                    rssi = 256 - rssi;
                    DBGPRINT(MT_DEBUG_ERROR, "read = %d, Packet header is RSSI, handle is 0x%02X%02X, RSSI is -%d",
                            nRead, buffer[8], buffer[7], rssi);
                }
            } else if (buffer[4] == 0x11 && buffer[5] == 0xFD) {
                int link_number = (int)(buffer[15]);
                int i;

                DBGPRINT(MT_DEBUG_SHOW, "link_number = %d", link_number);
                DBGPRINT(MT_DEBUG_SHOW, "BT Tx Count = %d and Rx Count = %d , LE Tx Count = %d and Rx Count = %d",
                        buffer[7] + (buffer[8] << 8), buffer[9] + (buffer[10] << 8), buffer[11] + (buffer[12] << 8), buffer[13] + (buffer[14] << 8));
                for ( i = 0; i < link_number; i++) {
                    int index = 16 + 26 * i;
                    uint32_t per_link_tx_count = 0;
                    uint32_t per_link_tx_total_count = 0;
                    uint32_t per_link_tx_error_count = 0;
                    uint32_t per_link_tx_per = 0;
                    uint32_t per_link_rx_count = 0;
                    uint32_t per_link_rx_total_count = 0;
                    uint32_t per_link_rx_error_count = 0;
                    uint32_t per_link_rx_per = 0;
                    double l2cap_avg = 0;
                    double l2cap_max = 0;
                    DBGPRINT(MT_DEBUG_SHOW, "BD_ADDRESS = %02X:%02X:%02X:%02X:%02X:%02X",
                            buffer[index + 5], buffer[index + 4], buffer[index + 3],
                            buffer[index + 2], buffer[index + 1], buffer[index]);
                    DBGPRINT(MT_DEBUG_SHOW, "Type of Link = %s",
                            buffer[index + 6]==(uint8_t)0?"BT Master":
                            buffer[index + 6]==(uint8_t)1?"BT Slave":
                            buffer[index + 6]==(uint8_t)2?"BLE Master":
                            buffer[index + 6]==(uint8_t)3?"BLE SLave":"Unknown");

                    per_link_tx_count = buffer[index + 7] + (buffer[index + 8] << 8);
                    per_link_tx_total_count = buffer[index + 9] + (buffer[index + 10] << 8);
                    per_link_tx_error_count = buffer[index + 11] + (buffer[index + 12] << 8);
                    if (per_link_tx_total_count)
                        per_link_tx_per = ((per_link_tx_error_count*100) / per_link_tx_total_count);

                    DBGPRINT(MT_DEBUG_SHOW, "Packet Tx Count of Link = %d", per_link_tx_count);
                    DBGPRINT(MT_DEBUG_SHOW, "TX Per = %d (%d/%d)", per_link_tx_per, per_link_tx_error_count, per_link_tx_total_count);

                    per_link_rx_count = buffer[index + 13] + (buffer[index + 14] << 8);
                    per_link_rx_total_count = buffer[index + 15] + (buffer[index + 16] << 8);
                    per_link_rx_error_count = buffer[index + 16] + (buffer[index + 18] << 8);
                    if (per_link_rx_total_count)
                        per_link_rx_per = ((per_link_rx_error_count*100) / per_link_rx_total_count);

                    DBGPRINT(MT_DEBUG_SHOW, "Packet Rx Count of Link = %d", per_link_rx_count);
                    DBGPRINT(MT_DEBUG_SHOW, "RX PER  = %d (%d/%d)", per_link_rx_per, per_link_rx_error_count, per_link_rx_total_count);

                    DBGPRINT(MT_DEBUG_SHOW, "Tx Power Lower Bound Index = %d, Last Used Tx Power Index = %d",
                            buffer[index + 19], buffer[index + 20]);
                    DBGPRINT(MT_DEBUG_SHOW, "Last Used Tx Power in dBm = %d dBm",
                            (int8_t)buffer[index + 21]);
                    l2cap_avg =  buffer[index + 22] + (buffer[index + 23] << 8);
                    DBGPRINT(MT_DEBUG_SHOW, "Average L2CAP Out latency = %.f, %.9f ms",
                            l2cap_avg, l2cap_avg * 0.3125);
                    l2cap_max =  buffer[index + 24] + (buffer[index + 25] << 8);
                    DBGPRINT(MT_DEBUG_SHOW, "Maximum L2CAP Out latency = %.f, %.9f ms\n",
                            l2cap_max, l2cap_max * 0.3125);
                }
            } else {
                switch (buffer[0]) {
                    case DATA_TYPE_COMMAND:
                        /* it's for hci command */
                        bperf_notify_cmd(&buffer[1], nRead - 1);
                        DBGPRINT(MT_DEBUG_TRACE, "read = %d, Packet header is cmd %02X %02X %02X %02X %02X %02X",
                                nRead, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                        break;
                    case DATA_TYPE_ACL:
                        bperf_notify_data(&buffer[1], nRead - 1);
                        DBGPRINT(MT_DEBUG_TRACE, "read = %d, Packet header is acl %02X %02X %02X %02X %02X %02X",
                                nRead, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                        break;
                    case DATA_TYPE_EVENT:
                        /* it's for hci event */
                        bperf_notify_event(&buffer[1], nRead - 1);
                        DBGPRINT(MT_DEBUG_TRACE, "read = %d, Packet header is event %02X %02X %02X %02X %02X %02X",
                                nRead, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                        if (buffer[1] == HCE_CONNECTION_COMPLETE) {
                            if (buffer[12] == 0x01) {
                                DBGPRINT(MT_DEBUG_SHOW, "(Connection_Complete)(ConnHandle:0x%04X)(ACL)", (((buffer[5] << 8) & 0xff00) | buffer[4]));
                            } else if(buffer[12] == 0x00) {
                                DBGPRINT(MT_DEBUG_SHOW, "(Connection_Complete)(ConnHandle:0x%04X)(SCO)", (((buffer[5] << 8) & 0xff00) | buffer[4]));
                            } else {
                                DBGPRINT(MT_DEBUG_SHOW, "(Connection_Complete)(ConnHandle:0x%04X)", ((buffer[5] << 8 & 0xff00) | buffer[4]));
                            }
                        }
                        else if (buffer[1] == HCE_COMMAND_COMPLETE) {
                            if (buffer[4] == 0xFF) {
                                DBGPRINT(MT_DEBUG_SHOW, "RSSI:%d Channel:%d", (int8_t)buffer[5], buffer[6]);
                            }
                            else if (buffer[4] == 0x06 &&  buffer[5] == 0x14) {
                                DBGPRINT(MT_DEBUG_SHOW, "Connection_Handle:0x%02X%02X AFH_Mode:%d AFH:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", buffer[8], buffer[7], buffer[9],
                                        buffer[10], buffer[11], buffer[12], buffer[13],buffer[14],
                                        buffer[15], buffer[16], buffer[17], buffer[18],buffer[19]);
                            }
                        }
                        break;
                    default:
                        DBGPRINT(MT_DEBUG_ERROR, "read = %d, Packet header is not not fw log %02X %02X %02X %02X %02X %02X",
                                nRead, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                        break;
                }
            }
            ret = 0;
        } else {
            ++ret;
        }
    } while (cont);

done:
    if (fd) close(fd);
    if (fscript) {
        DBGPRINT(MT_DEBUG_SHOW, "release %s", dump_file_name);
        fclose(fscript);
    }
    if (fw_dump_fscript) {
        DBGPRINT(MT_DEBUG_SHOW, "release %s", fw_dump_file_name);
        fclose(fw_dump_fscript);
    }
    /* unlock the device node */
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fcntl(fd, F_SETLKW, &fl);

    return 0;
}

//---------------------------------------------------------------------------
