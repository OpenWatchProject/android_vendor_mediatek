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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <cutils/properties.h>

#include "bt_em.h"


/* LOG_TAG must be defined before log.h */
#define LOG_TAG           "BT_RELAYER "
#include <log/log.h>

#define BT_RELAYER_DEBUG  1
#define ERR(f, ...)       ALOGE("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define WAN(f, ...)       ALOGW("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#if BT_RELAYER_DEBUG
#define DBG(f, ...)       ALOGD("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define TRC(f)            ALOGW("%s #%d", __FUNCTION__, __LINE__)
#else
#define DBG(...)          ((void)0)
#define TRC(f)            ((void)0)
#endif

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static int serial_fd = -1;
static int serial_io_flags;

static pthread_t txThread; // PC->BT moniter thread
static pthread_t rxThread; // BT->PC moniter thread
static BOOL fgKillThread = FALSE;

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static int uart_speed(int s)
{
    switch (s) {
      case 9600:
        return B9600;
      case 19200:
        return B19200;
      case 38400:
        return B38400;
      case 57600:
        return B57600;
      case 115200:
        return B115200;
      case 230400:
        return B230400;
      case 460800:
        return B460800;
      case 500000:
        return B500000;
      case 576000:
        return B576000;
      case 921600:
        return B921600;
      default:
        return B57600;
    }
}

/* Initialize serial port to PC */
static int init_serial(int port, int speed)
{
    struct termios ti;
    int fd;
    int baudenum;
    char dev[20] = {0};
    char usb_prop[PROPERTY_VALUE_MAX];

    if (port < 4) { /* serial port UART */
        sprintf(dev, "/dev/ttyMT%d", port);
    }
    else { /* serial port USB */
        sprintf(dev, "/dev/ttyGS2");
    }

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        ERR("Can't open serial port %s\n", dev);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    if (tcgetattr(fd, &ti) < 0) {
        ERR("Can't get serial port setting\n");
        close(fd);
        return -1;
    }

    cfmakeraw(&ti);

    if (port < 4) { /* serial port UART */
        ti.c_cflag |= CLOCAL;
        ti.c_lflag = 0;

        ti.c_cflag &= ~CRTSCTS;
        ti.c_iflag &= ~(IXON | IXOFF | IXANY);

        /* Set baudrate */
        baudenum = uart_speed(speed);
        if ((baudenum == B57600) && (speed != 57600)) {
            ERR("Serial port baudrate not supported!\n");
            close(fd);
            return -1;
        }

        cfsetospeed(&ti, baudenum);
        cfsetispeed(&ti, baudenum);
    }
    else { /* serial port USB */
        /* Set USB property to acm_third: add 1 acm port to /dev/ttyGS2 */
        property_get("vendor.usb.config", usb_prop, NULL);
        if (0 != strcmp(usb_prop, "acm_third")) {
            if (property_set("vendor.usb.config", "acm_third") < 0) {
                ERR("Can't set USB property to open a VCOM\n");
                close(fd);
                return -1;
            }
            else {
                DBG("Set USB property to open a VCOM\n");
            }
        }
    }

    if (tcsetattr(fd, TCSANOW, &ti) < 0) {
        ERR("Can't set serial port setting\n");
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    serial_io_flags = 0;
    return fd;
}


static int write_data_to_pc(int fd, unsigned char *buf, unsigned int len)
{
    int bytesWritten = 0;
    unsigned int bytesToWrite = len;

    if (fd < 0)
        return -EINVAL;

    /* Try to send len bytes data in buffer */
    while (bytesToWrite > 0) {
        bytesWritten = write(fd, buf, bytesToWrite);
        if (bytesWritten < 0) {
            if (((errno == EINTR) && !(serial_io_flags & WR_INTERRUPTIBLE)) || (errno == EAGAIN))
                continue;
            else
                return -errno;
        }
        bytesToWrite -= bytesWritten;
        buf += bytesWritten;
    }

    return 0;
}

static int read_data_from_pc(int fd, unsigned char *buf, unsigned int len)
{
    int bytesRead = 0;
    unsigned int bytesToRead = len;

    int ret = 0;
    struct timeval tv;
    fd_set readfd;

    tv.tv_sec = 5; /* SECOND */
    tv.tv_usec = 0; /* USECOND */
    FD_ZERO(&readfd);

    if (fd < 0)
        return -EINVAL;

    /* Try to receive len bytes */
    while (bytesToRead > 0) {

        FD_SET(fd, &readfd);
        ret = select(fd + 1, &readfd, NULL, NULL, &tv);

        if (ret > 0) {
            bytesRead = read(fd, buf, bytesToRead);
            if (bytesRead < 0) {
                if (((errno == EINTR) && !(serial_io_flags & RD_INTERRUPTIBLE)) || (errno == EAGAIN))
                    continue;
                else
                    return -errno;
            }
            else{
                bytesToRead -= bytesRead;
                buf += bytesRead;
            }
        }
        else if (ret == 0) {
            return -ETIMEDOUT; /* Read com port timeout 5000ms */
        }
        else if ((ret == -1) && (errno == EINTR) && !(serial_io_flags & RD_INTERRUPTIBLE)) {
            ERR("select error ignore EINTR\n");
            continue;
        }
        else {
            ERR("select error %s(%d)!\n", strerror(errno), errno);
            return -errno;
        }
    }

    return 0;
}

static BOOL RELAYER_write(int fd, unsigned char *buf, unsigned int len)
{
    if (buf == NULL) {
        ERR("Write buffer is NULL!\n");
        return FALSE;
    }

    if ((buf[0] != 0x04) && (buf[0] != 0x02) && (buf[0] != 0x03) && (buf[0] != 0x05)){
        ERR("Invalid packet type %02x to PC\n", buf[0]);
        return FALSE;
    }

    if (write_data_to_pc(fd, buf, len) < 0) {
        return FALSE;
    }

    return TRUE;
}

static BOOL RELAYER_read(
    int fd,
    unsigned char *buf,
    unsigned int len,
    unsigned int *pu4ResultLen
    )
{
    UCHAR ucHeader = 0;
    unsigned int u4Len = 0, pkt_len = 0;

    if (buf == NULL) {
        ERR("Read buffer is NULL\n");
        return FALSE;
    }

    if (read_data_from_pc(fd, &ucHeader, sizeof(ucHeader)) < 0) {
        *pu4ResultLen = u4Len;
        return FALSE;
    }

    buf[0] = ucHeader;
    u4Len ++;

    switch (ucHeader) {
      case 0x01:
        /* HCI command */
        if (read_data_from_pc(fd, &buf[1], 3) < 0) {
            ERR("Read command header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 3;
        pkt_len = (unsigned int)buf[3];
        if ((u4Len + pkt_len) > len) {
            ERR("Too large packet from PC! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (read_data_from_pc(fd, &buf[4], pkt_len) < 0) {
            ERR("Read command param fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x02:
        /* ACL data */
        if (read_data_from_pc(fd, &buf[1], 4) < 0) {
            ERR("Read ACL header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 4;
        pkt_len = (((unsigned int)buf[4]) << 8);
        pkt_len += (unsigned int)buf[3]; /*little endian*/
        if ((u4Len + pkt_len) > len) {
            ERR("Too large packet from PC! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (read_data_from_pc(fd, &buf[5], pkt_len) < 0) {
            ERR("Read ACL data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x03:
        /* SCO data */
        if (read_data_from_pc(fd, &buf[1], 3) < 0) {
            ERR("Read SCO header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 3;
        pkt_len = (unsigned int)buf[3];
        if ((u4Len + pkt_len) > len) {
            ERR("Too large packet from PC! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (read_data_from_pc(fd, &buf[4], pkt_len) < 0) {
            ERR("Read SCO data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x05:
        /* ISO data */
        if (read_data_from_pc(fd, &buf[1], 4) < 0) {
            ERR("Read ACL header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 4;
        pkt_len = (((unsigned int)buf[4] & 0x3F) << 8);
        pkt_len += (unsigned int)buf[3]; /*little endian*/
        if ((u4Len + pkt_len) > len) {
            ERR("Too large packet from PC! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (read_data_from_pc(fd, &buf[5], pkt_len) < 0) {
            ERR("Read ACL data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      default:
        /* Filter PC garbage data */
        ERR("Invalid packet type %02x from PC\n", ucHeader);
        *pu4ResultLen = 0;
        return FALSE;
    }

    return TRUE;
}


static void thread_exit(UNUSED_ATTR int signo)
{
    pthread_t tid = pthread_self();
    DBG("Thread %lu prepare to exit\n", tid);
    fgKillThread = TRUE;
    if (tid == txThread) {
        EM_BT_set_io_attr(WR_INTERRUPTIBLE);
        serial_io_flags |= RD_INTERRUPTIBLE;
    }
    else if (tid == rxThread) {
        EM_BT_set_io_attr(RD_INTERRUPTIBLE);
        serial_io_flags |= WR_INTERRUPTIBLE;
    }
}

static void *bt_tx_monitor(UNUSED_ATTR void *ptr)
{
    /* From Spec, the largest packet "3-DH5" has 1026 bytes */
    UCHAR ucTxBuf[1026];
    unsigned int u4PktLen;

    DBG("Thread %lu starts\n", txThread);

    while (!fgKillThread) {
        memset(ucTxBuf, 0, sizeof(ucTxBuf));
        u4PktLen = 0;

        /* Receive HCI packet from PC */
        if (RELAYER_read(serial_fd, ucTxBuf, sizeof(ucTxBuf), &u4PktLen)) {
            DBG("Receive packet from PC\n");
            /* Send the packet to BT Controller */
            if (EM_BT_write(ucTxBuf, u4PktLen)) {
                sched_yield();
                continue;
            }
            else {
                ERR("Send packet to BT Controller fails\n");
                continue;
            }
        }
        else {
            if (u4PktLen == 0) {
                continue;
            }
            else {
                ERR("Receive packet from PC fails\n");
                continue;
            }
        }
    }
    return 0;
}

static void *bt_rx_monitor(UNUSED_ATTR void *ptr)
{
    /* From Spec, the largest packet "3-DH5" has 1026 bytes */
    UCHAR ucRxBuf[1026];
    unsigned int u4PktLen;

    DBG("Thread %lu starts\n", rxThread);

    while (!fgKillThread) {
        memset(ucRxBuf, 0, sizeof(ucRxBuf));
        u4PktLen = 0;

        /* Receive HCI packet from BT Controller */
        if (EM_BT_read(ucRxBuf, sizeof(ucRxBuf), &u4PktLen)) {
            DBG("Send packet to PC\n");
            /* Send the packet to PC */
            if (RELAYER_write(serial_fd, ucRxBuf, u4PktLen)) {
                sched_yield();
                continue;
            }
            else {
                ERR("Send packet to PC fails\n");
                continue;
            }
        }
        else {
            if (u4PktLen == 0) {
                continue;
            }
            else {
                ERR("Receive packet from BT Controller fails\n");
                continue;
            }
        }
    }
    return 0;
}

BOOL RELAYER_start(int serial_port, int serial_speed)
{
    TRC();

    if (EM_BT_init()) {
        DBG("BT device power on success\n");
    }
    else {
        ERR("BT device power on fails\n");
        return FALSE;
    }

    serial_fd = init_serial(serial_port, serial_speed);
    if (serial_fd < 0) {
        ERR("Initialize serial port to PC fails\n");
        EM_BT_deinit();
        return FALSE;
    }

    signal(SIGRTMIN, thread_exit);
    fgKillThread = FALSE;
    /* Create Tx monitor thread */
    pthread_create(&txThread, NULL, bt_tx_monitor, (void*)NULL);
    /* Create Rx monitor thread */
    pthread_create(&rxThread, NULL, bt_rx_monitor, (void*)NULL);

    DBG("BT Relayer mode start\n");

    return TRUE;
}

void RELAYER_exit(void)
{
    TRC();

    pthread_kill(txThread, SIGRTMIN);
    /* Wait until thread exit */
    pthread_join(txThread, NULL);
    pthread_kill(rxThread, SIGRTMIN);
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);

    /* Clear the previous set I/O attributes for return on signal */
    EM_BT_clear_io_attr(WR_INTERRUPTIBLE | RD_INTERRUPTIBLE);
    serial_io_flags &= ~(WR_INTERRUPTIBLE | RD_INTERRUPTIBLE);
    signal(SIGRTMIN, SIG_DFL);

    close(serial_fd);
    serial_fd = -1;

    EM_BT_deinit();
}
