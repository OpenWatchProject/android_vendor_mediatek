/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/*******************************************************************************
* Dependency
*******************************************************************************/
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>  // For 'O_RDWR' & 'O_EXCL'
#include <poll.h>
#include <android/log.h>
#include "mnld_utile.h"
#include <termios.h>
#include "CFG_GPS_File.h"
#include "mnl_agps_interface.h"
#include <cutils/android_filesystem_config.h>

#include "mtk_lbs_utility.h"

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#if 0
#define LOGD(...) tag_log(1, "[mnld_uti]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnld_uti] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnld_uti] ERR: ", __VA_ARGS__);
#else


#define LOG_TAG "mnld_uti"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

extern int mtk_gps_sys_init();
#if MTK_GPS_NVRAM
extern ap_nvram_gps_config_struct stGPSReadback;
#endif
extern int in_meta_factory;

/*****************************************************************************/
int read_NVRAM() {
    int ret = 0;
    ret = mtk_gps_sys_init();
    #if MTK_GPS_NVRAM
    if (strcmp(stGPSReadback.dsp_dev, DSP_DEV) == 0) {
        LOGE("not 3332 UART port\n");
        return 1;
    }
    #endif
    return ret;
}

int init_3332_interface(const int fd) {
    struct termios termOptions;
    // fcntl(fd, F_SETFL, 0);

    // Get the current options:
    tcgetattr(fd, &termOptions);

    // Set 8bit data, No parity, stop 1 bit (8N1):
    termOptions.c_cflag &= ~PARENB;
    termOptions.c_cflag &= ~CSTOPB;
    termOptions.c_cflag &= ~CSIZE;
    termOptions.c_cflag |= CS8 | CLOCAL | CREAD;

    LOGD("GPS_Open: c_lflag=%x,c_iflag=%x,c_oflag=%x\n", termOptions.c_lflag, termOptions.c_iflag,
                  termOptions.c_oflag);
    // termOptions.c_lflag

    // Raw mode
    termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
    termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*raw input*/
    termOptions.c_oflag &= ~OPOST;  /*raw output*/

    tcflush(fd, TCIFLUSH);  // clear input buffer
    termOptions.c_cc[VTIME] = 10;  /* inter-character timer unused, wait 1s, if no data, return */
    termOptions.c_cc[VMIN] = 0;  /* blocking read until 0 character arrives */

     // Set baudrate to 38400 bps
    cfsetispeed(&termOptions, B115200);  /*set baudrate to 115200, which is 3332 default bd*/
    cfsetospeed(&termOptions, B115200);

    tcsetattr(fd, TCSANOW, &termOptions);

    return 0;
}

int hw_test_3332(const int fd) {
    ssize_t bytewrite, byteread;
    char buf[6] = {0};
    char cmd[] = {0xAA, 0xF0, 0x6E, 0x00, 0x08, 0xFE, 0x1A, 0x00, 0x00, 0x00, 0x00,
                0x00, 0xC3, 0x01, 0xA5, 0x02, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x45, 0x00,
                0x80, 0x04, 0x80, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
                0x96, 0x00, 0x6F, 0x3C, 0xDE, 0xDF, 0x8B, 0x6D, 0x04, 0x04, 0x00, 0xD2, 0x00,
                0xB7, 0x00, 0x28, 0x00, 0x5D, 0x4A, 0x1E, 0x00, 0xC6, 0x37, 0x28, 0x00, 0x5D,
                0x4A, 0x8E, 0x65, 0x00, 0x00, 0x01, 0x00, 0x28, 0x00, 0xFF, 0x00, 0x80, 0x00,
                0x47, 0x00, 0x64, 0x00, 0x50, 0x00, 0xD8, 0x00, 0x50, 0x00, 0xBB, 0x00, 0x03,
                0x00, 0x3C, 0x00, 0x6F, 0x00, 0x89, 0x00, 0x88, 0x00, 0x02, 0x00, 0xFB, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x7A, 0x16, 0xAA, 0x0F};
    char ack[] = {0xaa, 0xf0, 0x0e, 0x00, 0x31, 0xfe};

    bytewrite = write(fd, cmd, sizeof(cmd));
    if (bytewrite == sizeof(cmd)) {
        usleep(500*000);
        byteread = read(fd, buf, sizeof(buf));
        LOGD("ack:%02x %02x %02x %02x %02x %02x\n",
                 buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        if ((byteread == sizeof(ack)) && (memcmp(buf, ack, sizeof(ack)) == 0)) {
            LOGD("it's 3332\n");
            return 0;   /*0 means 3332, 1 means other GPS chips*/
        }
        return 1;
    } else {
        LOGE("write error, write API return is %d, error message is %s\n", bytewrite, strerror(errno));
        return 1;
    }
}

int hand_shake() {
#if MTK_GPS_NVRAM
    int fd;
#endif
    int ret = -1;
    int nv;
    nv = read_NVRAM();

    if (nv == 1)
        return 1;
    else if (nv == -1)
        return -1;
    else
        LOGD("read NVRAM ok\n");
#if MTK_GPS_NVRAM
    fd = open(stGPSReadback.dsp_dev, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        LOGE("GPS_Open: Unable to open - %s, %s\n", stGPSReadback.dsp_dev, strerror(errno));
        return -1;
    }
    init_3332_interface(fd);   /*set UART parameter*/

    ret = hw_test_3332(fd);   /*is 3332?    0:yes   1:no*/
    close(fd);
#endif
    return ret;
}

int confirm_if_3332() {
    int ret;
    // power_on_3332();
    ret = hand_shake();
    // power_off_3332();
    return ret;
}
#if ANDROID_MNLD_PROP_SUPPORT
extern char chip_id[PROPERTY_VALUE_MAX];
#else
extern char chip_id[100];
#endif
void chip_detector() {
    int get_time = 20;
    int res;
#if ANDROID_MNLD_PROP_SUPPORT
    while ((get_time-- != 0) && ((res = property_get("persist.vendor.connsys.chipid", chip_id, NULL)) < 6)) {
        LOGE("get chip id fail, retry");
        usleep(200000);
    }

    // chip id is like "0xXXXX"
    if (res < 6) {
       LOGE("combo_chip_id error: %s\n", chip_id);
       return;
    }
#endif
    LOGD("combo_chip_id is %s\n", chip_id);

    if (in_meta_factory) {
        return;
    }
    /* detect if there is 3332, yes set GPS property to 3332,
    then else read from combo chip to see which GPS chip used */
    res = confirm_if_3332();    /* 0 means 3332, 1 means not 3332, other value means error */
    if (res == 0) {
        strncpy(chip_id, "0x3332",sizeof(chip_id));
        LOGD("we get MT3332\n");
    }
    return;
}

int buff_get_int(char* buff, int* offset) {
    int ret = *((int*)&buff[*offset]);
    *offset += 4;
    return ret;
}

int buff_get_string(char* str, char* buff, int* offset) {
    int len = *((int*)&buff[*offset]);
    *offset += 4;

    memcpy(str, &buff[*offset], len);
    *offset += len;
    return len;
}

void buff_put_int(int input, char* buff, int* offset) {
    *((int*)&buff[*offset]) = input;
    *offset += 4;
}

void buff_put_string(char* str, char* buff, int* offset) {
    int len = strlen(str) + 1;

    *((int*)&buff[*offset]) = len;
    *offset += 4;

    memcpy(&buff[*offset], str, len);
    *offset += len;
}

void buff_put_struct(void* input, int size, char* buff, int* offset) {
    memcpy(&buff[*offset], input, size);
    *offset += size;
}

void buff_get_struct(char* output, int size, char* buff, int* offset) {
    memcpy(output, &buff[*offset], size);
    *offset += size;
}

int buff_get_binary(void* output, char* buff, int* offset) {
    int len = *((int*)&buff[*offset]);
    *offset += 4;

    memcpy(output, &buff[*offset], len);
    *offset += len;
    return len;
}

int add_chksum(char *pBuf) {
    int i;
    unsigned char chksum;
    char ch;
    unsigned short Size;

    Size = strlen(pBuf);

    if (Size == 0) {
        return 0;
    }

    // will add six characters: '*', CHK1, CHK2, <CR>, <LF>, <ZERO>
    if (Size > (PMTK_MAX_PKT_LENGTH - 6)) {
        Size = PMTK_MAX_PKT_LENGTH - 6;
    }

    chksum = 0;
    for (i = 1; i < Size; i++) {
        if (pBuf[i] != '*') {
            chksum ^= pBuf[i];
        } else {
            Size = i;
            break;
        }
     }

    // Add symbol '*'
    pBuf[Size] = '*';

    // Add CHK1
    ch = (chksum & 0xF0) >> 4;
    if (ch >= 10) {
        ch = 'A' + ch - 10;
    } else {
        ch = '0' + ch;
    }
    pBuf[Size + 1] = ch;

    // Add CHK2
    ch = chksum & 0x0F;
    if (ch >= 10) {
        ch = 'A' + ch - 10;
    } else {
        ch = '0' + ch;
    }
    pBuf[Size + 2] = ch;

    pBuf[Size + 3] = '\r';
    pBuf[Size + 4] = '\n';
    pBuf[Size + 5] = '\0';

    return 1;
}



