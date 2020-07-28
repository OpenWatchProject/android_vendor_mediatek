/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdlib.h>
#include <cutils/jstring.h>
#include <sys/epoll.h>
#include <poll.h>
#include "RtstUtils.h"
#include "RtstLog.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define TAG "RTF"

/*****************************************************************************
 * Class RtstUtils
 *****************************************************************************/
void RtstUtils::writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}


char *RtstUtils::getStringFromParcel(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}


int RtstUtils::pollWait(int fd, int timeout) {
    struct pollfd poll_fd[1];
    int ret;

    memset(poll_fd, 0, sizeof(poll_fd));
    poll_fd[0].fd      = fd;
    poll_fd[0].events  = POLLIN;
    poll_fd[0].revents = 0;

    ret = poll(poll_fd, 1, timeout);
    if(ret > 0) {
        if(poll_fd[0].revents & POLLIN ||
            poll_fd[0].revents & POLLERR) {
            ret = 1 << 0;
        }
    }
    return ret;
}


void RtstUtils::dumpToHexFormatLog(const char* prefix, const char* data, int len) {
    int i = 0;
    int offset = 0;
    char buff[1024] = {0};

    RTST_LOG_V(TAG, "%s  len=%d\n", prefix, len);

    for(i = 0; i < len; i++) {
        if(i%16 == 0) {
            sprintf(buff + offset, "%04x0 ", i/16);
            offset += 6;
        }

        sprintf(buff + offset, "%02x ", data[i]&0xff);
        offset += 3;

        if(i%16 == 15) {
            sprintf(buff + offset, "\r\n");

            RTST_LOG_V(TAG, "%s %s ", prefix, buff);

            memset(buff, 0, sizeof(buff));
            offset = 0;
        }
    }

    if(offset > 0) {
        RTST_LOG_V(TAG, "%s %s ", prefix, buff);
    }
}


char RtstUtils::char2Int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        RTST_ASSERT(0);
        return 0;
    }
}


void RtstUtils::hex2Bin(const char *src, char *target, int len) {
    int srcLen = strlen(src);
    RTST_ASSERT(srcLen % 2 == 0);
    RTST_ASSERT((srcLen / 2) == len);
    for (int i = 0; i < len; i++) {
        target[i] = char2Int(*src);
        src++;
        target[i] = target[i] << 4 | char2Int(*src);
        src++;
    }
}


void RtstUtils::cleanSocketBuffer(const RtstFd &s, int timeout) {
    while (1) {
        int ret = RtstUtils::pollWait(s.getFd(), timeout);
        if (ret <=0 ) {
            break;
        }
        unsigned char buf[1024];
        s.read(buf, 1024);
    }
}

int RtstUtils::bufferedRead(const RtstFd &fd,char * buf) {
     static int readCnt = 0;
     static char* readPtr;
     static char readBuf[2048];
     if (readCnt <= 0) {
         if ((readCnt = fd.read(readBuf, sizeof(readBuf))) < 0) {
             return -1;
         } else if (readCnt == 0) {
             return 0;
         }
         readPtr = readBuf;
     }
     readCnt--;
     *buf = *readPtr++;
     return 1;
}


int RtstUtils::readLine(const RtstFd & fd,void *buf, int len) {
    int rc;
    char c;
    char *ptr = (char* )buf;
    int i;
    for (i = 1; i < len; i++) {
        if ((rc = bufferedRead(fd, &c)) == 1) {
            *ptr++ = c;
            if (c=='\r') {
                break;
            }
        } else if (rc == 0) {
            if (i == 1) {
                return 0;
            } else {
                break;
            }
        } else {
            return -1;
        }
    }
    *ptr = 0;
    return i;
}