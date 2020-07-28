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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <drm/DrmMtkDef.h>

#include <SecureTimerHelper.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef unsigned int uint32;
typedef long long32;
typedef long long long64;

using namespace android;

#define SEC_1900_TO_1970 0x83aa7e80U  // seconds between 1900/1/1 - 1970/1/1
                                      // which is 3600 * 24 * (365 * 70 + 17) = 2,208,988,800

#define NTP_TIMEOUT 10 // 10 seconds for each trial
#define NTP_RETRY 2    // 2 times trial for each server. totally 20 seconds for each server
#define NTP_PORT 123   // 123 PORT UDP

#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

#define NTPFRAC(x) (4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x)    (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

//#define _DRM_ST_SNTP_DEBUG

typedef struct timeval TimeValue;        // <sys/time.h>
typedef struct sockaddr_in SocketAddrIn; // <sys/socket.h>
typedef struct sockaddr SockAddr;

typedef struct
{
    uint32 coarse; // lower accuracy
    uint32 fine;   // higher accuracy
} NTPTimeStamp;

const char* NTP_SERVER_ADDR[NTP_SERVER_CNT] = {
        DrmSntpServer::NTP_SERVER_1,
        DrmSntpServer::NTP_SERVER_2,
        DrmSntpServer::NTP_SERVER_3,
        DrmSntpServer::NTP_SERVER_4,
        DrmSntpServer::NTP_SERVER_5};

long32 send_packet(int fd_socket)
{
    uint32 data[12];
    memset(data, 0, sizeof(data));

    data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1 << 16);
    data[2] = htonl(1 << 16);

    TimeValue now;
    gettimeofday(&now, NULL); // now.tv_sec is the seconds since 1970-1-1 Zulu
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("the current time value of device: tv_sec - [%d], tv_usec - [%d] \n", (int)now.tv_sec, (int)now.tv_usec);
    #endif

    data[10] = htonl(now.tv_sec + SEC_1900_TO_1970); //  Transmit Timestamp coarse, lower accuracy
    data[11] = htonl(NTPFRAC(0));                    //  Transmit Timestamp fine, higher accurary
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("the current time coarse in HEX format: [%x] \n", data[10]);
        printf("send data to SNTP server \n");
    #endif

    return send(fd_socket, data, sizeof(data), 0);
}

long32 get_time_diff(uint32* data, long32 ref, long32 ori)
{
    // ref: T4
    // ori: T1
    NTPTimeStamp recv_time; // T2
    NTPTimeStamp tran_time; // T3

    //  the delay for internet trans is:
    //  {delay} = ((T4 - T1) - (T3 - T2)) / 2
    //  the delta between the real time and device time should be:
    //  {delta} = (T2 - T1 + T3 - T4) / 2    {T_real} - {T_dev} = {delta}

    recv_time.coarse = ntohl(data[8]);
    recv_time.fine   = ntohl(data[9]);
    tran_time.coarse = ntohl(data[10]);
    tran_time.fine   = ntohl(data[11]);
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("reference time value: T4: [%d] seconds \n", (int)ref);
        printf(" original time value: T1: [%d] seconds \n", (int)ori);
    #endif

    #ifdef _DRM_ST_SNTP_DEBUG
        long32 delay = (ref - ori - tran_time.coarse + recv_time.coarse) / 2;
        printf("the transition delay on Internet: [%d] seconds \n", (int)delay);
    #endif

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("  receive time: T2 - [%x] \n", recv_time.coarse);
        printf(" transact time: T3 - [%x] \n", tran_time.coarse);
        printf("reference time: T4 - [%x] \n", (uint32)ref);
        printf(" original time: T1 - [%x] \n", (uint32)ori);
    #endif

    long32 diff = (long32)(((long64)recv_time.coarse
                            - (ori + SEC_1900_TO_1970)
                            + tran_time.coarse
                            - (ref + SEC_1900_TO_1970)) / 2);
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("the different between device time & real time is [%d] seconds \n", (int)diff);
    #endif

    return diff;
}

void print_curr_time(TimeValue tv __attribute__((unused)))
{
    #ifdef _DRM_ST_SNTP_DEBUG
        //  need root user
        if (0 != getuid() && 0 != geteuid())
        {
            printf("print_curr_time() : not root user \n");
            return;
        }
        printf("current time value: [%d] seconds \n", (int)tv.tv_sec);

        tm s_time = *(gmtime((const time_t*)&tv.tv_sec));
        printf("current time: [%d-%d-%d] [%d:%d.%d] \n",
                s_time.tm_year + 1900, s_time.tm_mon + 1, s_time.tm_mday,
                s_time.tm_hour, s_time.tm_min, s_time.tm_sec);
    #endif
}

const int SecureTimerHelper::NTP_SYNC_SUCCESS = 0;
const int SecureTimerHelper::NTP_SYNC_NETWORK_TIMEOUT = -1;
const int SecureTimerHelper::NTP_SYNC_SERVER_TIMEOUT = -2; // not used
const int SecureTimerHelper::NTP_SYNC_NETWORK_ERROR = -3;
const int SecureTimerHelper::NTP_SYNC_INVALID_OFFSET = -4;

int SecureTimerHelper::syncNTPTime(time_t& offset)
{
    int value = 0;
    int index = 0;
    do
    {
        #ifdef _DRM_ST_SNTP_DEBUG
            printf("\n============================================ \n");
            printf("sync with SNTP server: [%s] \n", NTP_SERVER_ADDR[index]);
        #endif
        value = syncNTPTime(offset, NTP_SERVER_ADDR[index], 1);
        if (NTP_SYNC_SUCCESS == value)
        {
            #ifdef _DRM_ST_SNTP_DEBUG
                printf("successfully synchronized with SNTP server: [%s]\n", NTP_SERVER_ADDR[index]);
                printf("============================================ \n\n");
            #endif
            break;
        }
        index++; // next one
    }
    while (index < NTP_SERVER_CNT);

    return value;
}

int SecureTimerHelper::syncNTPTime(time_t& offset, const char* svr_addr, int addr_type)
{
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("syncNTPTime() ----> \n");
    #endif

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("create socket @syncNTPTime() \n");
    #endif

    //  create socket. if create fail return.
    int sock = socket(PF_INET, SOCK_DGRAM, 0); // UDP mode
    if (sock < 0) {
        return NTP_SYNC_NETWORK_ERROR;
    }

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("setup socket @syncNTPTime() \n");
    #endif

    //  bind local address so as to listen to it.
    SocketAddrIn addr_src; // local
    memset(&addr_src, 0, sizeof(SocketAddrIn)); // local
    addr_src.sin_family = AF_INET;
    addr_src.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_src.sin_port = htons(0);
    bind(sock, (SockAddr*)&addr_src, sizeof(SocketAddrIn));

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("connect @syncNTPTime() \n");
    #endif

    //  connect to NTP server.
    SocketAddrIn addr_dst; // server
    memset(&addr_dst, 0, sizeof(SocketAddrIn));
    addr_dst.sin_family = AF_INET;
    struct hostent* host = NULL;
    if (addr_type == 0) // IP string like xxx.xxx.xxx.xxx
    {
        in_addr_t inaddr = inet_addr(svr_addr);
        host = gethostbyaddr((const char*)&inaddr, 4, AF_INET);
    }
    else if (addr_type == 1) // domain name address string
    {
        host = gethostbyname(svr_addr);
    }

    if (host == NULL)
    {
        #ifdef _DRM_ST_SNTP_DEBUG
            printf("failed to get host by name @syncNTPTime() \n");
        #endif
        close(sock);
        return NTP_SYNC_NETWORK_ERROR;
    }
    memcpy(&(addr_dst.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr_dst.sin_port = htons(NTP_PORT);
    connect(sock, (SockAddr*)&addr_dst, sizeof(SocketAddrIn));

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("send SNTP package to server @syncNTPTime() \n");
    #endif

    // begin sync, the first time, send the NTP package directly.
    long32 ori = (long32)time(NULL); // T1
    send_packet(sock);

    #ifdef _DRM_ST_SNTP_DEBUG
        printf("send and receive socket data; try sync [%d] times @syncNTPTime() \n", NTP_RETRY);
    #endif

    int cnt = NTP_RETRY;
    while (cnt >= 0)
    {
        cnt -= 1;

        //  Wait the socket to response until timeout
        TimeValue tv_timeout;
        tv_timeout.tv_sec = NTP_TIMEOUT;
        tv_timeout.tv_usec = 0;

        fd_set fds_read;
        FD_ZERO(&fds_read);
        FD_SET(sock, &fds_read);
        int ret = select(sock + 1, &fds_read, NULL, NULL, &tv_timeout);

        if (0 == ret || !FD_ISSET(sock, &fds_read))
        {
            #ifdef _DRM_ST_SNTP_DEBUG
                printf("NTP server response timeout. continue sending NTP package @syncNTPTime() \n");
            #endif
            ori = (long32)time(NULL); // T1
            send_packet(sock); // send NTP pakcage again
            continue;
        }

        #ifdef _DRM_ST_SNTP_DEBUG
            printf("NTP server response received successfully @syncNTPTime() \n");
        #endif
        long32 ref = (long32)time(NULL); // T4

        uint32 buf[12];
        memset(buf, 0, sizeof(buf));
        SockAddr server;
        socklen_t svr_len;
        recvfrom(sock, buf, sizeof(buf), 0, &server, &svr_len); // get data into buffer

        // calculate time different between device & server(realTime)
        offset = (time_t)get_time_diff(buf, ref, ori);

        #ifdef _DRM_ST_SNTP_DEBUG
            printf("NTP sync successfully, and close socket @syncNTPTime() \n");
        #endif
        close(sock);

        #ifdef _DRM_ST_SNTP_DEBUG
            TimeValue tx; // current time
            gettimeofday(&tx, NULL);
            printf("\n ----> original device time: ----> \n");
            print_curr_time(tx);
        #endif

        #ifdef _DRM_ST_SNTP_DEBUG
            tx.tv_sec += offset; // then adjust it
            printf("\n ---> adjusted time: ---> \n");
            print_curr_time(tx);
        #endif

        return NTP_SYNC_SUCCESS;
    }

    //  after NTP_RETRY times of trial, the synchronization fails
    #ifdef _DRM_ST_SNTP_DEBUG
        printf("NTP sync failed, and close socket @syncNTPTime() \n");
    #endif
    close(sock);
    return NTP_SYNC_NETWORK_TIMEOUT;
}

