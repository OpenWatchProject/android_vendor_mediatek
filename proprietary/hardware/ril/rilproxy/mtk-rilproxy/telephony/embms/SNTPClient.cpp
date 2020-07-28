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

#include "SNTPClient.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <utils/Timers.h>
#include <pthread.h>

#define LOG_TAG "eMBMS-SNTPClient"
#include <log/log.h>

pthread_condattr_t s_cond_attr;
pthread_cond_t s_cond;
pthread_mutex_t s_mutex;

namespace android {

SNTPClient::SNTPClient() {
    pthread_condattr_init(&s_cond_attr);
    pthread_condattr_setclock(&s_cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&s_cond, &s_cond_attr);
    pthread_mutex_init(&s_mutex, NULL);
}

void sig_handler(int signo) {
    if (signo == SIGUSR1) {
        RLOGI("received SIGUSR1\n");
        pthread_exit(NULL);
    }
}

void * asyn_getv4addrinfo(void *ptr) {
    char *host;
    int ret;
    struct sockaddr_in *sin;
    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
    };
    unsigned int* ret_val = NULL;

    RLOGI("asyn_getv4addrinfo start");
    signal(SIGUSR1, sig_handler);
    host = (char *) ptr;
    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret != 0) {
        RLOGE("getaddrinfo error!: %s", gai_strerror(ret));
        pthread_mutex_lock(&s_mutex);
        pthread_cond_signal(&s_cond);
        pthread_mutex_unlock(&s_mutex);
        pthread_exit((void *)ret_val);
    } else {
        sin = (struct sockaddr_in *) res[0].ai_addr;

        RLOGI("asyn_getv4addrinfo %d.%d.%d.%d",
            sin->sin_addr.s_addr & 0xFF,
            (sin->sin_addr.s_addr & 0x0000FF00)>>8,
            (sin->sin_addr.s_addr & 0x00FF0000)>>16,
            (sin->sin_addr.s_addr & 0xFF000000)>>24);

        ret_val = (unsigned int*)malloc(sizeof(unsigned int));
        if (ret_val == NULL) {
            RLOGE("malloc fail");
        } else {
            *ret_val = sin->sin_addr.s_addr;
        }

        freeaddrinfo(res);
        pthread_mutex_lock(&s_mutex);
        pthread_cond_signal(&s_cond);
        pthread_mutex_unlock(&s_mutex);
        RLOGI("asyn_getv4addrinfo exit");
        pthread_exit((void *)ret_val);
    }
}

status_t SNTPClient::requestTime(const char *host) {
    struct hostent *ent;
    int64_t requestTimeNTP, requestTimeMs;
    ssize_t n;
    int64_t responseTimeMs, responseTimeNTP;
    int64_t originateTimeNTP, receiveTimeNTP, transmitTimeNTP;
    int64_t roundTripTimeNTP, clockOffsetNTP;

    struct sockaddr_in hostAddr;
    status_t err = UNKNOWN_ERROR;
    RLOGE("requestTime starts");

    int s = socket(AF_INET, SOCK_DGRAM, 0);

    if (s < 0) {

       goto bail;
    }

    // asyn getaddrinfo call, to avoid bad network block the ril-proxy main thread.
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != -1) {
        ts.tv_sec += 3;  // Set DNS timeout to 3 seconds
        ts.tv_nsec += 0;
        pthread_t thread1;
        unsigned int* p_addr = NULL;
        int r, kill_rc;
        RLOGE("requestTime pthread_create");

        pthread_mutex_lock(&s_mutex);
        pthread_create(&thread1, NULL , asyn_getv4addrinfo , (void*) host);
        r = pthread_cond_timedwait(&s_cond, &s_mutex, &ts);
        pthread_mutex_unlock(&s_mutex);
        if (r == ETIMEDOUT) {
             RLOGE("Time out! Force quit child thread\n");
             pthread_kill(thread1 , SIGUSR1);
        }

        RLOGI("bf pthread_join\n");
        r = pthread_join(thread1, (void**)&p_addr);
        if (r != 0 || p_addr == NULL) {
            RLOGE("pthread_join error or result null, error:%d", r);
            goto bail2;
        } else {
            unsigned int addr = *p_addr;
            RLOGI("asyn_getv4addrinfo %d.%d.%d.%d",
                addr & 0xFF,
                (addr & 0x0000FF00)>>8,
                (addr & 0x00FF0000)>>16,
                (addr & 0xFF000000)>>24);

            memset(hostAddr.sin_zero, 0, sizeof(hostAddr.sin_zero));
            hostAddr.sin_family = AF_INET;
            hostAddr.sin_port = htons(kNTPPort);
            hostAddr.sin_addr.s_addr = addr;
        }

        if (p_addr != NULL) {
            free(p_addr);
        }
    } else {
        RLOGI("Can't call asyn_getv4addrinfo due to clock_gettime error!");
        goto bail2;
    }

    uint8_t packet[kNTPPacketSize];
    memset(packet, 0, sizeof(packet));

    packet[0] = kNTPModeClient | (kNTPVersion << 3);
    // systemTime return 10^-9 nano seconds
    requestTimeNTP = systemTime(CLOCK_REALTIME) / 1000000ll;
    RLOGI("requestTimeNTP:%llu", (unsigned long long)requestTimeNTP);
    requestTimeMs = systemTime(CLOCK_MONOTONIC) / 1000000ll;
    writeTimeStamp(packet, kNTPTransmitTimeOffset, requestTimeNTP);
    RLOGI("requestTimeMs:%lld", (long long)requestTimeMs);
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    err = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (void *)&tv, sizeof(tv));
    if(err < 0)
    {
        RLOGE("set send timeout fail.");
        goto bail2;
    }
    err =setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(tv));
    if(err < 0)
    {
        RLOGE("set recv timeout fail.");
        goto bail2;
    }

    RLOGI("Before ntp send");
    n = sendto(
            s, packet, sizeof(packet), 0,
            (const struct sockaddr *)&hostAddr, sizeof(hostAddr));

    if (n < 0) {
        goto bail2;
    }

    memset(packet, 0, sizeof(packet));
    RLOGI("Before ntp recv");
    do {
        n = recv(s, packet, sizeof(packet), 0);
    } while (n < 0 && errno == EINTR);

    if (n < 0) {
        goto bail2;
    }
    RLOGI("After ntp recv");
    responseTimeMs = systemTime(CLOCK_MONOTONIC) / 1000000ll;
    RLOGI("responseTimeUs:%lld", (long long)responseTimeMs);
    responseTimeNTP = requestTimeNTP + (responseTimeMs - requestTimeMs);
    RLOGI("responseTimeNTP:%llu", (unsigned long long)responseTimeNTP);

    uint8_t leap;
    leap = (uint8_t) ((packet[0] >> 6) & 0x3);
    uint8_t mode;
    mode = (uint8_t) (packet[0] & 0x7);
    int stratum;
    stratum = (int) (packet[1] & 0xff);

    originateTimeNTP = readTimeStamp(packet, kNTPOriginateTimeOffset);
    RLOGI("originateTimeNTP:%llu", (unsigned long long)originateTimeNTP);
    receiveTimeNTP = readTimeStamp(packet, kNTPReceiveTimeOffset);
    RLOGI("receiveTimeNTP:%llu", (unsigned long long)receiveTimeNTP);
    transmitTimeNTP = readTimeStamp(packet, kNTPTransmitTimeOffset);
    RLOGI("transmitTimeNTP:%llu", (unsigned long long)transmitTimeNTP);

    roundTripTimeNTP =
        (responseTimeMs - requestTimeMs)
        - (transmitTimeNTP - receiveTimeNTP);

    if (originateTimeNTP <= 0) {
        RLOGI("originateTime:%lld", (unsigned long long)originateTimeNTP);
        clockOffsetNTP = ((receiveTimeNTP - requestTimeNTP) + (transmitTimeNTP - responseTimeNTP)) / 2;
    } else {
        clockOffsetNTP = ((receiveTimeNTP - originateTimeNTP) + (transmitTimeNTP - responseTimeNTP)) / 2;
    }

    mTimeReferenceNTP = responseTimeNTP + clockOffsetNTP;
    mTimeReferenceUs = responseTimeMs;
    mRoundTripTimeNTP = roundTripTimeNTP;

    err = OK;

bail2:
    close(s);
    s = -1;

bail:
    return err;
}

/**
 * Reads an unsigned 32 bit big endian number from the given offset in the buffer.
 */
uint64_t read32(uint8_t* buffer, int offset) {
    uint8_t b0 = buffer[offset];
    uint8_t b1 = buffer[offset+1];
    uint8_t b2 = buffer[offset+2];
    uint8_t b3 = buffer[offset+3];

    // convert signed bytes to unsigned values
    int i0 = ((b0 & 0x80) == 0x80 ? (b0 & 0x7F) + 0x80 : b0);
    int i1 = ((b1 & 0x80) == 0x80 ? (b1 & 0x7F) + 0x80 : b1);
    int i2 = ((b2 & 0x80) == 0x80 ? (b2 & 0x7F) + 0x80 : b2);
    int i3 = ((b3 & 0x80) == 0x80 ? (b3 & 0x7F) + 0x80 : b3);

    return ((int64_t)i0 << 24) + ((int64_t)i1 << 16) + ((int64_t)i2 << 8) + (int64_t)i3;
}

/**
 * Reads the NTP time stamp at the given offset in the buffer and returns
 * it as a system time (milliseconds since January 1, 1970).
 */
int64_t SNTPClient::readTimeStamp(uint8_t* buffer, int offset) {
    int64_t seconds = read32(buffer, offset);
    int64_t fraction = read32(buffer, offset + 4);
    // Special case: zero means zero.
    if (seconds == 0 && fraction == 0) {
        return 0;
    }
    return ((seconds - OFFSET_1900_TO_1970) * 1000) + ((fraction * 1000LL) / 0x100000000LL);
}

/**
 * Writes system time (milliseconds since January 1, 1970) as an NTP time stamp
 * at the given offset in the buffer.
 */
void SNTPClient::writeTimeStamp(uint8_t* buffer, int offset, uint64_t _time) {
    // Special case: zero means zero.
    if (_time == 0) {
        RLOGI("time = 0");
        memset((uint8_t*)&buffer[offset], 0, 8);
        return;
    }
    RLOGI("time = %llu", (unsigned long long)_time);
    uint64_t seconds = _time / 1000L;
    uint64_t milliseconds = _time - seconds * 1000L;
    seconds += OFFSET_1900_TO_1970;

    // write seconds in big endian format
    buffer[offset++] = (uint8_t)(seconds >> 24);
    buffer[offset++] = (uint8_t)(seconds >> 16);
    buffer[offset++] = (uint8_t)(seconds >> 8);
    buffer[offset++] = (uint8_t)(seconds >> 0);

    uint64_t fraction = milliseconds * 0x100000000L / 1000L;
    // write fraction in big endian format
    buffer[offset++] = (uint8_t)(fraction >> 24);
    buffer[offset++] = (uint8_t)(fraction >> 16);
    buffer[offset++] = (uint8_t)(fraction >> 8);
    // low order bits should be random data
    srand(time(NULL));
    buffer[offset++] = (uint8_t)(rand()%255);
}

}  // namespace android
