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
#ifndef LOGGING_H
#define LOGGING_H

#include <cutils/list.h>
#include <log/log.h>

#define BUFFER_SIZE         (256 * 1024)  // 256kB

extern int is_mblog_prepare();

/*
 * log buffer util, used to cache log and control throughput
 * @buf_used, used size of this buf
 * @ok, whether the buf is full
 * @marked, whether the full buf marked
 * @owner, which log device does this buf belong to
 * @m_buf, save logs
 */
struct buf_util {
    int buf_used;
    int ok;
    bool marked;
    char m_buf[BUFFER_SIZE];
    struct log_dev_t *owner;
    struct listnode buflist;
};

/*
 * @android_dev, log dev is android dev
 * @direct_dev, logs can directly read and write, no need decode
 */
typedef enum {
    ANDROID_DEV = 0,
    DIRECT_DEV  = 1
} dev_type;

/*
 * Core struct of mobilelog, represents a log type
 *
 * @dev_name,          the name of log source device
 * @name,              the name of log file
 * @rfd,               fd of source device
 * @wfd,               fd of log file
 * @rotate_size,       max size of the log file
 * @total_write_size,  size writted to the log file
 * @buf_p,             pointer of struct buf_util,
 *
 */
struct log_dev_t {
    dev_type type;
    int id;
    char log_name[32];
    char wf_name[128];
    int rfd;
    int wfd;

    unsigned long rotate_size;
    unsigned long total_write_size;
    struct buf_util *buf_p;
    struct listnode devlist;
};

// fulled log file node which is ready for rotation
struct logfull_entry {
    struct listnode node;
    char name[128];
    long long size;
    char timestamp[64];
};
typedef struct logfull_entry logfull_entry_t;

int android_log_read(void);
int change_log_path(void);
int connect_to_logdr(int android_log, int kernel_log);
int direct_log_read(struct log_dev_t *ld_p);

void get_free_buf(struct log_dev_t *ld_p);
void insert_to_buf(struct log_dev_t *ld_p, struct log_msg *logmsg);

int open_logfile(struct log_dev_t *ld_p, const char *logpath);
void rename_logfile(struct log_dev_t *ld_p, const char* logPath);
int open_logger_dev(struct log_dev_t *ld_p);
int kernel_log_source(void);
int prepare_logging(void);
void read_thread(void);
int set_android_log_format(const char * str);
void set_rotate_size(void);
void stop_logging(void);
void write_thread(void);
int check_storage(void);
int enable_scp_poll(void);
int disable_scp_poll(void);
int enable_sspm_poll(void);
int disable_sspm_poll(void);
int enable_adsp_poll(void);
int disable_adsp_poll(void);
int enable_mcupm_poll(void);
int disable_mcupm_poll(void);

#endif
