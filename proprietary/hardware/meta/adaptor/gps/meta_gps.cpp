/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_gps.cpp
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Implement GPS interface for META mode.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * Mar 20 2009 mtk80143
 * [DUMA00111323] [GPS] modify for GPS META
 * Add for GPS meta
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <strings.h>

//for read NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"

#ifdef GPS_PROPERTY
#undef GPS_PROPERTY
#endif
#define GPS_PROPERTY "/data/misc/GPS_CHIP_FTM.cfg"
//static  ap_nvram_gps_config_struct stGPSReadback;
#define MNL_ATTR_PWRCTL  "/sys/class/gpsdrv/gps/pwrctl"
#define MNL_ATTR_SUSPEND "/sys/class/gpsdrv/gps/suspend"
#define MNL_ATTR_STATE   "/sys/class/gpsdrv/gps/state"
#define MNL_ATTR_PWRSAVE "/sys/class/gpsdrv/gps/pwrsave"
#define MNL_ATTR_STATUS  "/sys/class/gpsdrv/gps/status"
#define BUF_TAIL_LENGTH   3
#define ACK_BUF_SIZE      1024
static int mnld_terminated = 0;

enum {
    GPS_PWRCTL_UNSUPPORTED  = 0xFF,
    GPS_PWRCTL_OFF          = 0x00,
    GPS_PWRCTL_ON           = 0x01,
    GPS_PWRCTL_RST          = 0x02,
    GPS_PWRCTL_OFF_FORCE    = 0x03,
    GPS_PWRCTL_RST_FORCE    = 0x04,
    GPS_PWRCTL_MAX          = 0x05,
};

#define C_INVALID_PID  (-1)   /*invalid process id*/
#define C_INVALID_TID  (-1)   /*invalid thread id*/
#define C_INVALID_FD   (-1)   /*invalid file handle*/
#define C_INVALID_SOCKET (-1)  /*invalid socket id*/

#define MND_ERR META_LOG
#define MND_MSG META_LOG
#define META_LOG ALOGD

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

//// meta test
#define META_TO_MNLD_SOCKET        "mtk_meta2mnld"
#include <hidl/LegacySupport.h>
#include <vendor/mediatek/hardware/lbs/1.0/ILbs.h>
#include "meta_gps_para.h"

//#include "Meta2MnldInterface.h"
#define META2MNLD_INTERFACE_PROTOCOL_TYPE 303
#define META2MNLD_INTERFACE_BUFF_SIZE 12
/**
 * The interface from Meta to Mnld
 */
typedef enum {
    META2MNLD_INTERFACE_REQ_GNSS_LOCATION = 0,
    META2MNLD_INTERFACE_CANCEL_GNSS_LOCATION = 1,
} Meta2MnldInterface_message_id;
using ::android::sp;
using ::android::wp;
using ::android::status_t;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::Return;
using ::android::hidl::base::V1_0::IBase;
using vendor::mediatek::hardware::lbs::V1_0::ILbs;
struct LbsDeathRecipient : virtual public hidl_death_recipient
{
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
      // TODO(gomo): implement a better death recovery mechanism without
      // crashing system server process as described in go//treble-gnss-death
      ALOGE("Abort due to ILBS hidl service failure.%zd", sizeof(who));
      UNUSED(cookie);
    }
};
sp<ILbs> lbsHal = nullptr;
sp<LbsDeathRecipient> lbsHalDeathRecipient = nullptr;
static hidl_vec<uint8_t> convertByteArrayToArrayList(char data[], int size) {
    hidl_vec<uint8_t> ret;
    ret.resize(size);
    for (int i = 0; i < size; i++) {
        ret[i] = data[i];
    }
    return ret;
}

void mtk_socket_put_char(char* buff, int* offset, char input) {
    *((char*)&buff[*offset]) = input;
    *offset += 1;
}

void mtk_socket_put_short(char* buff, int* offset, short input) {
    mtk_socket_put_char(buff, offset, input & 0xff);
    mtk_socket_put_char(buff, offset, (input >> 8) & 0xff);
}

void mtk_socket_put_int(char* buff, int* offset, int input) {
    mtk_socket_put_short(buff, offset, input & 0xffff);
    mtk_socket_put_short(buff, offset, (input >> 16) & 0xffff);
}

void hidl_request_gps(bool turnOn) {
    char _buff[META2MNLD_INTERFACE_BUFF_SIZE] = {0};
    int _offset = 0;
    int src = 0;
    int msg = turnOn ? META2MNLD_INTERFACE_REQ_GNSS_LOCATION: META2MNLD_INTERFACE_CANCEL_GNSS_LOCATION;

    if (lbsHal == nullptr) {
        lbsHal = ILbs::getService(META_TO_MNLD_SOCKET);

        if (lbsHal != nullptr) {
          lbsHalDeathRecipient = new LbsDeathRecipient();
          android::hardware::Return<bool> linked = (android::hardware::Return<bool>)lbsHal->linkToDeath(
              lbsHalDeathRecipient, /*cookie*/ 0);
            if (!linked.isOk()) {
                META_LOG("Transaction error in linking to LbsHAL death: %s",
                        linked.description().c_str());
            } else if (!linked) {
                META_LOG("Unable to link to LbsHal death notifications");
            } else {
                META_LOG("Link to death notification successful");
            }
        }
    }

    if (lbsHal != nullptr) {
        mtk_socket_put_int(_buff, &_offset, META2MNLD_INTERFACE_PROTOCOL_TYPE);
        mtk_socket_put_int(_buff, &_offset, msg);
        mtk_socket_put_int(_buff, &_offset, src);
        int size = (_offset < (int)(sizeof(_buff)/sizeof(_buff[0])))? _offset : sizeof(_buff)/sizeof(_buff[0]);
        META_LOG("Send req gnss location to LBS service offset: %d size: %d", _offset, size);
        hidl_vec<uint8_t> data = convertByteArrayToArrayList(_buff, size);
        lbsHal->sendToServer(data);
    } else {
        META_LOG("lbsHal=nullptr, getService failed");
    }
}
//// end



static GPS_CNF gps_cnf1;
static GPS_CNF gps_cnf2;

pid_t mnl_pid = C_INVALID_PID;
int sockfd = C_INVALID_SOCKET;
pthread_t gps_meta_thread_handle = C_INVALID_TID;

#if 0
static int mnl_write_attr(const char *name, unsigned char attr)
{
    int err, fd = open(name, O_RDWR);
    char buf[] = {(char)('0' + attr)};

    if (fd == -1) {
        META_LOG("open %s err = %s\n", name, strerror(errno));
        return -errno;
    }
    do {err = write(fd, buf, sizeof(buf) );}
    while (err < 0 && errno == EINTR);

    if (err != sizeof(buf)) {
        META_LOG("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        META_LOG("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    META_LOG("write '%d' to %s okay\n", attr, name);
    return err;
}
/*****************************************************************************/
void power_on_3332()
{
    int err;
    err = mnl_write_attr(MNL_ATTR_PWRCTL, GPS_PWRCTL_RST_FORCE);
    if (err != 0) {
        MND_ERR("GPS_Open: GPS power-on error: %d\n", err);
        return;
    }
    usleep(1000*100);
    return;
}
/*****************************************************************************/
void power_off_3332()
{
    int err;
    err = mnl_write_attr(MNL_ATTR_PWRCTL, GPS_PWRCTL_OFF);
    if (err != 0) {
        MND_ERR("GPS_Open: GPS power-on error: %d\n", err);
        return;
    }
    usleep(1000*100);
    return;
}

/*******************************************************************/
static int read_NVRAM()
{
    return 0;
}

/*****************************************************************************/
static int init_3332_interface(const int fd) {
    struct termios termOptions;
    // Get the current options:
    tcgetattr(fd, &termOptions);

    // Set 8bit data, No parity, stop 1 bit (8N1):
    termOptions.c_cflag &= ~PARENB;
    termOptions.c_cflag &= ~CSTOPB;
    termOptions.c_cflag &= ~CSIZE;
    termOptions.c_cflag |= CS8 | CLOCAL | CREAD;

    MND_MSG("GPS_Open: c_lflag=%x,c_iflag=%x,c_oflag=%x\n", termOptions.c_lflag, termOptions.c_iflag,
                            termOptions.c_oflag);

    // Raw mode
    termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
    termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*raw input*/
    termOptions.c_oflag &= ~OPOST;  /*raw output*/

    tcflush(fd, TCIFLUSH);  // clear input buffer
    termOptions.c_cc[VTIME] = 10;  /* inter-character timer unused, wait 1s, if no data, return */
    termOptions.c_cc[VMIN] = 0;  /* blocking read until 0 character arrives */

    // Set baudrate to 38400 bps
    cfsetispeed(&termOptions, B115200); /*set baudrate to 115200, which is 3332 default bd*/
    cfsetospeed(&termOptions, B115200);

    tcsetattr(fd, TCSANOW, &termOptions);

    return 0;
}
/*****************************************************************************/
static int hw_test_3332(const int fd)
{
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
                0x01, 0x00, 0x00, 0x00, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x7A,
                0x16, 0xAA, 0x0F};
    char ack[] = {0xaa, 0xf0, 0x0e, 0x00, 0x31, 0xfe};

    bytewrite = write(fd, cmd, sizeof(cmd));
    if (bytewrite == sizeof(cmd)) {
        usleep(500*1000);
        byteread = read(fd, buf, sizeof(buf));
        MND_MSG("ack:%02x %02x %02x %02x %02x %02x\n",
                 buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        if ((byteread == sizeof(ack)) && (memcmp(buf, ack, sizeof(ack)) == 0)) {
            MND_MSG("it's 3332\n");
            return 0;   /*0 means 3332,   1 means other GPS chips*/
        }
        return 1;
    } else  {
        MND_ERR("write error, write API return error message is %s\n", strerror(errno));
        return 1;
    }
}

/*****************************************************************************/
static int hand_shake()
{
    int fd;
    int ret;
    int nv;
    nv = read_NVRAM();

    if (nv == 1)
        return 1;
    else if (nv == -1)
        return -1;
    else
        MND_MSG("read NVRAM ok\n");

    fd = open(stGPSReadback.dsp_dev, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        MND_ERR("GPS_Open: Unable to open - %s, %s\n", stGPSReadback.dsp_dev, strerror(errno));
        return -1;
    }
    init_3332_interface(fd);    /*set UART parameter*/

    ret = hw_test_3332(fd); /*is 3332?  0:yes   1:no*/
    close(fd);
    return ret;
}

/*****************************************************************************/
static int confirm_if_3332()
{
    int ret;
    power_on_3332();
    ret = hand_shake();
    power_off_3332();
    return ret;
}
#endif
/*****************************************************************************/
static void sighlr(int signo)
{
    //pthread_t self = pthread_self();

    META_LOG("Wait MNLD terminated");
    if (signo == SIGCHLD) {
        META_LOG("MNLD terminated");
        mnld_terminated = 1;
    }
}
/*****************************************************************************/
int setup_signal_handler(void)
{
    struct sigaction actions;
    int err;

    /*the signal handler is MUST, otherwise, the thread will not be killed*/
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = sighlr;

    if ((err = sigaction(SIGALRM, &actions, NULL))) {
        META_LOG("register signal handler for SIGALRM: %s\n", strerror(errno));
        return -1;
    }
    if ((err = sigaction(SIGCHLD, &actions, NULL))) {
        META_LOG("register signal handler for SIGALRM: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
/*****************************************************************************/
int META_GPS_Open(int ATM_test)
{
    //int err;
    pid_t pid;
    int portno;
    struct sockaddr_in serv_addr;
    META_LOG("META_GPS_Open() 1\n");
    if (setup_signal_handler()) {
        META_LOG("setup_signal_handler: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    // power on GPS chip
    //err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 4);
    //if (err != 0) {
    //    META_LOG("META_GPS_Open: GPS power-on error: %d\n", err);
    //}

    if(0 != ATM_test) {
        META_LOG("ATM test, hidl start gps\n");
        hidl_request_gps(true);
    } else {
        // run gps driver
        if ((pid = fork()) < 0) {
            META_LOG("META_GPS_Open: fork fails: %d (%s)\n", errno, strerror(errno));
            return (-2);
        }
        else if (pid == 0) {  /*child process*/
            int err;
            err = execl("vendor/bin/mnld", "mnld", "1Hz=y", "meta", NULL);
            if (err == -1) {
                MND_MSG("execl error: %s\n", strerror(errno));
                return -1;
            }
            return 0;
        } else {  /*parent process*/
            mnl_pid = pid;
            META_LOG("META_GPS_Open: mnl_pid = %d\n", pid);
        }
    }
    // create socket connection to gps driver
    portno = 7000;
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        META_LOG("META_GPS_Open: ERROR opening socket");
        return (-4);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    int try_time = 10;
    do {
      META_LOG("META_GPS_Open: try connecting,try_time = %d",try_time);
      sleep(1);  // sleep 10 sec for libmnl to finish initialization
    } while (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && --try_time);
    if (try_time == 0) {
      META_LOG("META_GPS_Open: ERROR connecting");
      return (-6);
    }

    // run GPS_MetaThread
    if (pthread_create(&gps_meta_thread_handle, NULL, GPS_MetaThread, NULL)) {
      META_LOG("META_GPS_Open: error creating dsp thread \n");
      return (-7);
    }
    META_LOG("META_GPS_Open() 2\n");

    return 0;
}

void META_GPS_Close(int ATM_test) {
    int err;
    int cnt = 0, max = 10;

    META_LOG("META_GPS_Close() 1\n");
    // disconnect to gps driver
    if (sockfd != C_INVALID_SOCKET) {
        close(sockfd);
        sockfd = C_INVALID_SOCKET;
    }
    META_LOG("META_GPS_Close() 4\n");

    if(0 != ATM_test) {
        META_LOG("ATM test, hidl stop gps\n");
        hidl_request_gps(false);
    } else {
        if (mnl_pid != C_INVALID_PID) {
            META_LOG("GPS_Close() 5\n");
            // kill(mnl_pid, SIGKILL);
            // usleep(500000);

            kill(mnl_pid, SIGTERM);
            while (!mnld_terminated) {
                if (cnt++ < max) {
                /*timeout: 1 sec;
                notice that libmnlp needs
                some sleep time after MTK_PARAM_CMD_RESET_DSP*/
                    usleep(100000);
                    continue;
                } else {
                    kill(mnl_pid, SIGKILL);
                    usleep(100000);
                }
            }
            META_LOG("waiting counts: %d\n", cnt);
            mnl_pid = wait(&err);

            if (mnl_pid == -1)
                MND_MSG("wait error: %s\n",strerror(errno));
            MND_MSG("mnld process : %d is killed\n", mnl_pid);
            mnld_terminated = 1;
        }
    }
    unlink("/data/misc/mtkgps.dat");
    // power off GPS chip
    //err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 0);
    //if (err != 0) {
    //    META_LOG("GPS power-off error: %d\n", err);
    //}
    META_LOG("META_GPS_Close() 6\n");

    unlink(GPS_PROPERTY);
    META_LOG("META_GPS_Close() 7\n");
    return;
}


void META_GPS_OP(GPS_REQ *req, char *peer_buff, unsigned short peer_len, int ATM_test)
{
    memset(&gps_cnf1, 0, sizeof(GPS_CNF));
    gps_cnf1.header.id = FT_GPS_CNF_ID;
    gps_cnf1.header.token = req->header.token;
    gps_cnf1.op = req->op;
    memset(&gps_cnf2, 0, sizeof(GPS_CNF));
    gps_cnf2.header.id = FT_GPS_CNF_ID;
    gps_cnf2.header.token = req->header.token;
    gps_cnf2.op = req->op;

    META_LOG("META_GPS_OP() 1, (%d),(%s),(%u)\n", req->op, peer_buff, peer_len);
    switch (req->op) {
        case GPS_OP_OPEN:
            META_LOG("META_GPS_OP(), GPS_OP_OPEN 1\n");
            if (META_GPS_Open(ATM_test) != 0) {   // open fail
                META_LOG("META_GPS_OP(), GPS_OP_OPEN fail\n");
                META_GPS_Close(ATM_test);
                META_LOG("Can't open gps driver \r\n");
                gps_cnf1.gps_status = FALSE;
                gps_cnf1.status = META_FAILED;
            }
            else {
                META_LOG("META_GPS_OP(), GPS_OP_OPEN OK\n");
                gps_cnf1.gps_status = TRUE;
                gps_cnf1.status = META_SUCCESS;
            }
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);
            META_LOG("META_GPS_OP(), GPS_OP_OPEN 2\n");
            break;

        case GPS_OP_CLOSE:
            META_LOG("META_GPS_OP(), GPS_OP_CLOSE 1\n");
            META_GPS_Close(ATM_test);
            gps_cnf1.gps_status = TRUE;
            gps_cnf1.status = META_SUCCESS;
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);
            META_LOG("META_GPS_OP(), GPS_OP_CLOSE 2\n");
            break;

        case GPS_OP_SEND_CMD:
            META_LOG("META_GPS_OP(), GPS_OP_SEND_CMD\n");
            if (sockfd != C_INVALID_SOCKET) {
                int n = write(sockfd, req->cmd.buff, req->cmd.len);
                if (n < 0) {
                     META_LOG("ERROR writing to socket\r\n");
                }
                META_LOG("META_GPS_OP(), GPS_OP_SEND_CMD: %s\r\n", req->cmd.buff);
            }

            gps_cnf1.gps_status = TRUE;
            gps_cnf1.status = META_SUCCESS;
            META_LOG("GPS_OP_SEND_CMD, gps_cnf.status:%d\r\n", gps_cnf1.status);
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);

            break;

        default:
            META_LOG("META_GPS_OP(), default 1\n");
            gps_cnf1.gps_status = FALSE;
            gps_cnf1.status = META_FAILED;
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);
            META_LOG("META_GPS_OP(), default 2\n");
            break;
    }
    META_LOG("META_GPS_OP() 2\n");
}

void *GPS_MetaThread(void *arg)
{
    int read_leng = 0;
    int cnt = 0;
    int cnt_tmp = 0;
    char *ptr;
    char buf[10240];
    UNUSED(arg);

    cnt = 0;

    while (1) {
        memset(buf, 0, sizeof(buf));
        read_leng = 0;
        read_leng = read(sockfd, buf, sizeof(buf) - 1);
        if (read_leng < 0) {
            META_LOG("ERROR reading from socket");
            return (void *)(-1);
        } else if (read_leng > 0) {
            ptr=strtok(buf, "\r\n");
            if (ptr == NULL) {
                continue;
            }

            do {
                cnt_tmp = strlen(ptr);
                if (cnt_tmp + BUF_TAIL_LENGTH > ACK_BUF_SIZE || ptr[0] == '#') {
                    continue;
                }
                META_LOG("GPS_MetaThread: [%d]/[%d]\n", cnt_tmp, cnt);
                if ((cnt != 0) && ((cnt + cnt_tmp + BUF_TAIL_LENGTH) >= ACK_BUF_SIZE)) {
                    gps_cnf2.gps_ack.len = cnt;
                    gps_cnf2.gps_status = TRUE;
                    gps_cnf2.status = META_SUCCESS;
                    META_LOG("GPS_MetaThread, status:%d, gps_cnf.gps_ack.len:%d\r\n",
                    gps_cnf2.status, gps_cnf2.gps_ack.len);
                    WriteDataToPC(&gps_cnf2, sizeof(GPS_CNF), NULL, 0);
                    cnt = 0;
                }
                META_LOG("GPS_MetaThread: %s", ptr);
                strncpy((char*)&gps_cnf2.gps_ack.buff[cnt], ptr, (sizeof(gps_cnf2.gps_ack.buff) - cnt));
                cnt += cnt_tmp;
                gps_cnf2.gps_ack.buff[cnt++] = '\r';
                gps_cnf2.gps_ack.buff[cnt++] = '\n';
            } while ((ptr = strtok(NULL, "\r\n")) != NULL);
            if (cnt > 0) {
                gps_cnf2.gps_ack.len = cnt;
                gps_cnf2.gps_status = TRUE;
                gps_cnf2.status = META_SUCCESS;
                META_LOG("GPS_MetaThread, status:%d, last gps_cnf.gps_ack.len:%d\r\n",
                gps_cnf2.status, gps_cnf2.gps_ack.len);
                WriteDataToPC(&gps_cnf2, sizeof(GPS_CNF), NULL, 0);
                cnt = 0;
            }
        }
    }

    return (void *)0;
}
