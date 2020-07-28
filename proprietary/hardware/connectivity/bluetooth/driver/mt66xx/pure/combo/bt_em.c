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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "bt_em.h"


/* LOG_TAG must be defined before log.h */
#define LOG_TAG         "BT_EM "
#include <log/log.h>

#define BT_EM_DEBUG     1
#define ERR(f, ...)     ALOGE("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define WAN(f, ...)     ALOGW("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#if BT_EM_DEBUG
#define DBG(f, ...)     ALOGD("%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define TRC(f)          ALOGW("%s #%d", __FUNCTION__, __LINE__)
#else
#define DBG(...)        ((void)0)
#define TRC(f)          ((void)0)
#endif

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static int bt_fd = -1;

/* Used to read serial port */
static pthread_t rxThread;
static BOOL fgKillThread = FALSE;

/* mtk bt library */
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*DEINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buf, unsigned int len);
typedef int (*READ)(int fd, unsigned char *buf, unsigned int len);
typedef void (*SETIO)(int flags);
typedef int (*GETIO)(void);

INIT    em_bt_init = NULL;
DEINIT  em_bt_restore = NULL;
WRITE   em_bt_send_data = NULL;
READ    em_bt_receive_data = NULL;
SETIO   em_bt_set_io_attr = NULL;
GETIO   em_bt_get_io_attr = NULL;

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

BOOL EM_BT_init(void)
{
    const char *errstr;

    TRC();

    glib_handle = dlopen("libbluetooth_mtk_pure.so", RTLD_LAZY);
    DBG("load libbluetooth_mtk_pure.so\n");
    if (!glib_handle) {
        ERR("%s\n", dlerror());
        goto error;
    }

    dlerror(); /* Clear any existing error */

    em_bt_init = dlsym(glib_handle, "bt_init");
    em_bt_restore = dlsym(glib_handle, "bt_restore");
    em_bt_send_data = dlsym(glib_handle, "bt_send_data");
    em_bt_receive_data = dlsym(glib_handle, "bt_receive_data");
    em_bt_set_io_attr = dlsym(glib_handle, "bt_set_io_attr");
    em_bt_get_io_attr = dlsym(glib_handle, "bt_get_io_attr");

    if ((errstr = dlerror()) != NULL) {
        ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }

    bt_fd = em_bt_init();
    if (bt_fd < 0)
        goto error;

    DBG("BT is enabled success\n");

    return TRUE;

error:
    if (glib_handle) {
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return FALSE;
}

void EM_BT_deinit(void)
{
    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
    }
    else {
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
        }
        else{
            em_bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return;
}

BOOL EM_BT_write(unsigned char *buf, unsigned int len)
{
    UINT32 i;

    /* Print at most 16 numbers(HEX format) in each line
     * char str[] need to be larger than 82 bytes. */
    char str[100] = {""};
    char buf_str[6] = {""};

    TRC();

    if (buf == NULL) {
        ERR("NULL write buffer\n");
        return FALSE;
    }

    if ((buf[0] != 0x01) && (buf[0] != 0x02) && (buf[0] != 0x03) && (buf[0] != 0x05)) {
        ERR("Invalid packet type 0x%02x\n", buf[0]);
        return FALSE;
    }

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (em_bt_send_data(bt_fd, buf, len) < 0) {
        return FALSE;
    }

    /* Dump tx packet */
    DBG("write: len %d\n", len);
    for (i = 0; i < len; i++) {
        if ((i % 16 == 0) && (i != 0)) {
            DBG("%s\n", str);
            memset(str, 0, sizeof(str));
        }
        snprintf(buf_str, sizeof(buf_str), "0x%02x ", buf[i]);
        strncat(str, buf_str, strlen(buf_str));
    }
    DBG("%s\n", str);

    return TRUE;
}

BOOL EM_BT_read(unsigned char *buf, unsigned int len, unsigned int *pu4ResultLen)
{
    UINT8 ucHeader = 0;
    UINT32 u4Len = 0, pkt_len = 0;
    UINT32 i;

    /* Print at most 16 numbers(HEX format) in each line
     * char str[] need to be larger than 82 bytes. */
    char str[100] = {""};
    char buf_str[6] = {""};

    TRC();

    if (buf == NULL) {
        ERR("NULL read buffer\n");
        return FALSE;
    }

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

LOOP:
    if (em_bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
        *pu4ResultLen = u4Len;
        return FALSE;
    }

    buf[0] = ucHeader;
    u4Len ++;

    switch (ucHeader) {
      case 0x04:
        DBG("Receive HCI event\n");
        if (em_bt_receive_data(bt_fd, &buf[1], 2) < 0) {
            ERR("Read event header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 2;
        pkt_len = (UINT32)buf[2];
        if ((u4Len + pkt_len) > len) {
            ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (em_bt_receive_data(bt_fd, &buf[3], pkt_len) < 0) {
            ERR("Read event param fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x02:
        DBG("Receive ACL data\n");
        if (em_bt_receive_data(bt_fd, &buf[1], 4) < 0) {
            ERR("Read ACL header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 4;
        pkt_len = (((UINT32)buf[4]) << 8);
        pkt_len += (UINT32)buf[3]; /*little endian*/
        if ((u4Len + pkt_len) > len) {
            ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (em_bt_receive_data(bt_fd, &buf[5], pkt_len) < 0) {
            ERR("Read ACL data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x03:
        DBG("Receive SCO data\n");
        if (em_bt_receive_data(bt_fd, &buf[1], 3) < 0) {
            ERR("Read SCO header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 3;
        pkt_len = (UINT32)buf[3];
        if ((u4Len + pkt_len) > len) {
            ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (em_bt_receive_data(bt_fd, &buf[4], pkt_len) < 0) {
            ERR("Read SCO data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      case 0x05:
        DBG("Receive ISO data\n");
        if (em_bt_receive_data(bt_fd, &buf[1], 4) < 0) {
            ERR("Read ACL header fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += 4;
        pkt_len = (((UINT32)(buf[4] & 0x3F)) << 8);
        pkt_len += (UINT32)buf[3]; /*little endian*/
        if ((u4Len + pkt_len) > len) {
            ERR("Read buffer overflow! packet len %d\n", u4Len + pkt_len);
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        if (em_bt_receive_data(bt_fd, &buf[5], pkt_len) < 0) {
            ERR("Read ACL data fails\n");
            *pu4ResultLen = u4Len;
            return FALSE;
        }

        u4Len += pkt_len;
        *pu4ResultLen = u4Len;
        break;

      default:
        ERR("Unexpected BT packet header %02x\n", ucHeader);
        *pu4ResultLen = u4Len;
        return FALSE;
    }

    /* Dump rx packet */
    DBG("read: len %d\n", u4Len);
    for (i = 0; i < u4Len; i++) {
        if ((i % 16 == 0) && (i != 0)) {
            DBG("%s\n", str);
            memset(str, 0, sizeof(str));
        }
        snprintf(buf_str, sizeof(buf_str), "0x%02x ", buf[i]);
        strncat(str, buf_str, strlen(buf_str));
    }
    DBG("%s\n", str);

    /* If debug event, drop and retry */
    if ((buf[0] == 0x04) && (buf[1] == 0xE0)) {
        memset(buf, 0, len);
        u4Len = 0;
        goto LOOP;
    }

    return TRUE;
}

void EM_BT_set_io_attr(int flags)
{
    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return;
    }
    em_bt_set_io_attr(em_bt_get_io_attr() | flags);
}

void EM_BT_clear_io_attr(int flags)
{
    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return;
    }
    em_bt_set_io_attr(em_bt_get_io_attr() & ~flags);
}

static void thread_exit(UNUSED_ATTR int signo)
{
    pthread_t tid = pthread_self();
    DBG("Thread %lu prepare to exit\n", tid);
    fgKillThread = TRUE;
    EM_BT_set_io_attr(RD_INTERRUPTIBLE);
}

static void *BT_Rx_Thread(UNUSED_ATTR void *ptr)
{
    UINT8 ucRxBuf[512];
    UINT32 len = 512, u4ResultLen;

    DBG("Thread %lu starts\n", rxThread);
    while (!fgKillThread) {
        EM_BT_read(ucRxBuf, len, &u4ResultLen);
    }

    return 0;
}

void EM_BT_polling_start(void)
{
    DBG("Test start! Keep polling event from Controller\n");

    signal(SIGRTMIN, thread_exit);
    fgKillThread = FALSE;
    pthread_create(&rxThread, NULL, BT_Rx_Thread, NULL);
    return;
}

void EM_BT_polling_stop(void)
{
    DBG("Test about to end! Stop polling event from Controller\n");

    pthread_kill(rxThread, SIGRTMIN);
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);
    EM_BT_clear_io_attr(RD_INTERRUPTIBLE);
    signal(SIGRTMIN, SIG_DFL);
    return;
}

/* EM_BT_getPatchVersion
 *
 *    Connac20 use bt_dbg to get fw patch version
 *
 * Arguments:
 *    [IN] buf   - buffer for patch version string, please allocate a 256 bytes buffer
 *
 * Return Value:
 *     number of bytes write to buffer
 *
 */
unsigned int EM_BT_getPatchVersion(unsigned char *buf)
{
    int fd;
    int retval = 0;
    char procfs[] = "/proc/driver/bt_dbg";

    fd = open(procfs, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        DBG("Can't open %s, errno[%d][%s]", procfs, errno, strerror(errno));
    } else {
        retval = write(fd, "6 0 0", 5);
        if (retval > 0) {
            DBG("write: retval[%d]", retval);
        }
        retval = read(fd, buf, 256);
        if (retval > 0) {
            DBG("read: retval[%d], result:\n%s", retval, buf);
        }
    }
    close(fd);
    return retval;
}

