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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_BT

#ifndef FALSE
#define FALSE    0
#endif
#ifndef TRUE
#define TRUE     1
#endif
#ifndef BOOL
#define BOOL     bool
#endif

typedef unsigned char UCHAR;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;


#define TAG             "[BT] "

#define BT_FM_DEBUG     1
#define ERR(f, ...)     ALOGE(TAG "%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define WAN(f, ...)     ALOGW(TAG "%s: " f, __FUNCTION__, ##__VA_ARGS__)
#if BT_FM_DEBUG
#define DBG(f, ...)     ALOGD(TAG "%s: " f, __FUNCTION__, ##__VA_ARGS__)
#define TRC(f)          ALOGW(TAG "%s #%d", __FUNCTION__, __LINE__)
#else
#define DBG(...)        ((void)0)
#define TRC(f)          ((void)0)
#endif

/**************************************************************************
 *                       D E F I N I T I O N S                            *
***************************************************************************/

typedef struct _BT_HCI_CMD_T {
  UINT16 opcode;
  UINT8  len;
  UINT8  parms[256];
} BT_HCI_CMD_T;

typedef struct _BT_HCI_EVENT_T {
  UINT8 event;
  UINT8 len;
  UINT8 status;
  UINT8 parms[256];
} BT_HCI_EVENT_T;

/* Used to store inquiry result */
typedef struct _BT_INFO_T {
  struct _BT_INFO_T *pNext;
  UCHAR  btaddr[6];
  UCHAR  psr;
  UCHAR  cod[3];
  UCHAR  clkoffset[2];
  int    rssi;
  UCHAR  name[248];
} BT_INFO_T;

enum {
  TEST_RESULT_UNKNOWN = 0,
  TEST_RESULT_PASS,
  TEST_RESULT_FAIL,
};

typedef struct {
  int test_type;
  int test_result;
} TEST_INFO_T;

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static BT_INFO_T *g_pBtListHear = NULL;
static BOOL  g_scan_complete = FALSE;
static BOOL  g_inquiry_complete = FALSE;

static int   bt_fd = -1;

/* Used to read serial port */
static pthread_t rxThread;
static BOOL fgKillThread = FALSE;

/* mtk bt library */
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*DEINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buf, unsigned int len);
typedef int (*READ)(int fd, unsigned char *buf, unsigned int len);

INIT    bt_init = NULL;
DEINIT  bt_restore = NULL;
WRITE   bt_send_data = NULL;
READ    bt_receive_data = NULL;

extern  int test_result;
extern  sp_ata_data return_data;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BOOL BT_Send_HciCmd(BT_HCI_CMD_T *pHciCmd);
static BOOL BT_Recv_HciEvent(BT_HCI_EVENT_T *pHciEvent);
static BOOL BT_SetInquiryMode(UCHAR ucInquiryMode);
static BOOL BT_RemoteNameReq(BT_INFO_T *pTmpBtInfo);
static BOOL BT_Inquiry(void);

static void* BT_FM_Thread(void* ptr);

/**************************************************************************
  *                         F U N C T I O N S                             *
***************************************************************************/

BOOL FM_BT_init(void)
{
    const char *errstr;

    TRC();

    glib_handle = dlopen("libbluetooth_mtk_pure.so", RTLD_LAZY);
    if (!glib_handle) {
        ERR("%s\n", dlerror());
        goto error;
    }

    dlerror(); /* Clear any existing error */

    bt_init = dlsym(glib_handle, "bt_init");
    bt_restore = dlsym(glib_handle, "bt_restore");
    bt_send_data = dlsym(glib_handle, "bt_send_data");
    bt_receive_data = dlsym(glib_handle, "bt_receive_data");

    if ((errstr = dlerror()) != NULL) {
        ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }

    int bt_open_count = 0;
    do
    {
        usleep(1000*1000*2);
        if (bt_open_count++ > 60)
            goto error;
        bt_fd = bt_init();
    } while(bt_fd < 0);

    DBG("BT is enabled success\n");

    /* Enable inquiry result with RSSI */
    if (!BT_SetInquiryMode(0x01)) {
        ERR("Can't set BT inquiry mode\n");
        goto error;
    }

    /* Create RX thread */
    g_scan_complete = FALSE;
    g_inquiry_complete = FALSE;
    fgKillThread = FALSE;
    pthread_create(&rxThread, NULL, BT_FM_Thread, (void*)NULL);

    sched_yield();
    return TRUE;

error:
    if (bt_fd >= 0) {
        bt_restore(bt_fd);
        bt_fd = -1;
    }

    if (glib_handle) {
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return FALSE;
}

void FM_BT_deinit(void)
{
    BT_INFO_T *pBtInfo = NULL;

    TRC();

    /* Stop RX thread */
    fgKillThread = TRUE;
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);

    if (!glib_handle)
        ERR("mtk bt library is unloaded!\n");
    else {
        if (bt_fd < 0)
            ERR("bt driver fd is invalid!\n");
        else {
            bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    /* Clear globals */
    while (g_pBtListHear) {
        pBtInfo = g_pBtListHear;
        g_pBtListHear = g_pBtListHear->pNext;
        free(pBtInfo);
    }

    g_inquiry_complete = FALSE;
    g_scan_complete = FALSE;

    return;
}

static BOOL BT_Send_HciCmd(BT_HCI_CMD_T *pHciCmd)
{
    UINT8 ucHciCmd[256+4];

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    ucHciCmd[0] = 0x01;
    ucHciCmd[1] = (pHciCmd->opcode) & 0xFF;
    ucHciCmd[2] = (pHciCmd->opcode >> 8) & 0xFF;
    ucHciCmd[3] = pHciCmd->len;

    DBG("OpCode 0x%04x len %d\n", pHciCmd->opcode, (int)pHciCmd->len);

    if (pHciCmd->len)
        memcpy(&ucHciCmd[4], pHciCmd->parms, pHciCmd->len);

    if (bt_send_data(bt_fd, ucHciCmd, pHciCmd->len + 4) < 0) {
        ERR("Write HCI command fails errno %d\n", errno);
        return FALSE;
    }

    return TRUE;
}

static BOOL BT_Recv_HciEvent(BT_HCI_EVENT_T *pHciEvent)
{
    pHciEvent->status = FALSE;

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_receive_data(bt_fd, &pHciEvent->event, 1) < 0) {
        ERR("Read event code fails errno %d\n", errno);
        return FALSE;
    }

    DBG("Read event code: %02x\n", pHciEvent->event);

    if (bt_receive_data(bt_fd, &pHciEvent->len, 1) < 0) {
        ERR("Read event length fails errno %d\n", errno);
        return FALSE;
    }

    DBG("Read event length: %02x\n", pHciEvent->len);

    if (pHciEvent->len) {
        if (bt_receive_data(bt_fd, pHciEvent->parms, pHciEvent->len) < 0) {
            ERR("Read event param fails errno %d\n", errno);
            return FALSE;
        }
    }

    pHciEvent->status = TRUE;

    return TRUE;
}

static BOOL BT_SetInquiryMode(UCHAR ucInquiryMode)
{
    UINT8 HCI_INQUIRY_MODE[] = {0x01, 0x45, 0x0C, 0x01, 0x00};
    UINT8 ucAckEvent[7];
    UINT8 ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x45, 0x0C, 0x00};

    HCI_INQUIRY_MODE[4] = ucInquiryMode;

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_INQUIRY_MODE, sizeof(HCI_INQUIRY_MODE)) < 0) {
        ERR("Send inquiry mode command fails errno %d\n", errno);
        return FALSE;
    }

    if (bt_receive_data(bt_fd, ucAckEvent, sizeof(ucAckEvent)) < 0) {
        ERR("Receive command complete event fails errno %d\n", errno);
        return FALSE;
    }

    if (memcmp(ucAckEvent, ucEvent, sizeof(ucEvent))) {
        ERR("Receive unexpected event\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL BT_RemoteNameReq(BT_INFO_T *pTmpBtInfo)
{
    UINT8 ucHeader = 0;
    int   count = 0;
    BOOL  bRet = FALSE;

    BT_HCI_EVENT_T hci_event;
    UINT8 HCI_REMOTE_NAME_REQ[] =
        {0x01, 0x19, 0x04, 0x0A, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0xCC, 0xCC};

    HCI_REMOTE_NAME_REQ[4]  = pTmpBtInfo->btaddr[5];
    HCI_REMOTE_NAME_REQ[5]  = pTmpBtInfo->btaddr[4];
    HCI_REMOTE_NAME_REQ[6]  = pTmpBtInfo->btaddr[3];
    HCI_REMOTE_NAME_REQ[7]  = pTmpBtInfo->btaddr[2];
    HCI_REMOTE_NAME_REQ[8]  = pTmpBtInfo->btaddr[1];
    HCI_REMOTE_NAME_REQ[9]  = pTmpBtInfo->btaddr[0];
    HCI_REMOTE_NAME_REQ[10] = pTmpBtInfo->psr;
    HCI_REMOTE_NAME_REQ[12] = pTmpBtInfo->clkoffset[1];
    HCI_REMOTE_NAME_REQ[13] = pTmpBtInfo->clkoffset[0];

    TRC();

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_REMOTE_NAME_REQ, sizeof(HCI_REMOTE_NAME_REQ)) < 0) {
        ERR("Send remote name req command fails errno %d\n", errno);
        return FALSE;
    }

    while (!fgKillThread) {

        if (bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
            count ++;
            if (count < 5)
                continue;
            else
                break;
        }

        /* If not event */
        if (ucHeader != 0x04) {
            ERR("Unexpected BT packet header %02x\n", ucHeader);
            return FALSE;
        }

        if (BT_Recv_HciEvent(&hci_event)) {
            /* Wait for remote name request complete event */
            if (hci_event.event == 0x07) {
                /* success */
                if (hci_event.parms[0] == 0) {
                    /* FIX ME need convert UTF8 -> ASCII */
                    memcpy(pTmpBtInfo->name, &hci_event.parms[7], 248);
                    //pTmpBtInfo->name[hci_event.len] = '\0';
                    DBG("Remote name %s\n", pTmpBtInfo->name);
                    bRet = TRUE;
                    break;
                } else { /* faliure such as page time out */
                    DBG("Unexpected result event %02x status %02x\n",
                        hci_event.event, hci_event.parms[0]);
                    /* FIX ME need convert UTF8 -> ASCII */
                    pTmpBtInfo->name[0] = 'U';
                    pTmpBtInfo->name[1] = 'N';
                    pTmpBtInfo->name[2] = 'K';
                    pTmpBtInfo->name[3] = 'N';
                    pTmpBtInfo->name[4] = 'O';
                    pTmpBtInfo->name[5] = 'W';
                    pTmpBtInfo->name[6] = 'N';
                    pTmpBtInfo->name[7] = '\0';
                    bRet = TRUE;
                    break;
                }
            }
        } else {
            ERR("Receive event fails errno %d\n", errno);
            return FALSE;
        }
    }

    return bRet;
}

static BOOL BT_Inquiry(void)
{
    UINT8 HCI_INQUIRY[] =
        {0x01, 0x01, 0x04, 0x05, 0x33, 0x8B, 0x9E, 0x05, 0x0A};
    char *config_val = NULL;

    TRC();

    config_val = ftm_get_prop("BT.DeviceNumber");
    if (config_val != NULL) {
        HCI_INQUIRY[8] = (UINT8)atoi(config_val);
    }

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (bt_send_data(bt_fd, HCI_INQUIRY, sizeof(HCI_INQUIRY)) < 0) {
        ERR("Send inquiry command fails errno %d\n", errno);
        return FALSE;
    }

    return TRUE;
}

BOOL FM_BT_inquiry(void)
{
    return BT_Inquiry();
}

void updateTextInfo(TEST_INFO_T *pTest, char *output_buf, int buf_len)
{
    char cBuf[1024];
    static int loop = 0;
    BT_INFO_T *pBtInfo = NULL;

    memset(cBuf, 0, sizeof(cBuf));

    if (g_pBtListHear == NULL) {
        if (g_inquiry_complete == FALSE)
            snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_inquiry_start);
        else {
            /* Can not find any device */
            snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_no_dev);
            if (FTM_MANUAL_ITEM != pTest->test_type)
                pTest->test_result = TEST_RESULT_FAIL;
        }
    } else if (g_scan_complete == FALSE) {

        if (g_inquiry_complete == FALSE) {
            if (loop == 0) {
                snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_inquiry_1);
                loop = 1;
            } else {
                snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_inquiry_2);
                loop = 0;
            }
        } else {
            if (loop == 0) {
                snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_scan_1);
                loop = 1;
            } else {
                snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_scan_2);
                loop = 0;
            }
        }
    } else
        snprintf(cBuf, sizeof(cBuf), "%s", uistr_info_bt_scan_complete);

    pBtInfo = g_pBtListHear;
    return_data.bt.num = 0;

    while (pBtInfo) {
        if (strlen((const char*)pBtInfo->name)) {
            int str_len = 0;
            str_len = strlen((const char*)pBtInfo->name);

            strncpy(cBuf  + strlen(cBuf), (const char*)pBtInfo->name, 12);
            if (str_len < 12)
                strncpy(cBuf  + strlen(cBuf), "            ", 12 - str_len);

            snprintf(cBuf  + strlen(cBuf), sizeof(cBuf) - strlen(cBuf), " %d\n", pBtInfo->rssi);
            strncpy(return_data.bt.bt[return_data.bt.num].bt_name, pBtInfo->name, 31);
        }
        else {
            /* Inquiry result */
            snprintf(cBuf  + strlen(cBuf), sizeof(cBuf) - strlen(cBuf), "%02x%02x%02x%02x%02x%02x %d\n",
                     pBtInfo->btaddr[0], pBtInfo->btaddr[1], pBtInfo->btaddr[2],
                     pBtInfo->btaddr[3], pBtInfo->btaddr[4], pBtInfo->btaddr[5],
                     pBtInfo->rssi);

            return_data.bt.bt[return_data.bt.num].bt_rssi = pBtInfo->rssi;
            sprintf(return_data.bt.bt[return_data.bt.num].bt_mac, "%02x%02x%02x%02x%02x%02x\n",
                    pBtInfo->btaddr[0], pBtInfo->btaddr[1], pBtInfo->btaddr[2],
                    pBtInfo->btaddr[3], pBtInfo->btaddr[4], pBtInfo->btaddr[5]);
        }

        pBtInfo = pBtInfo->pNext;
        (return_data.bt.num)++;
    }

    if (g_pBtListHear) {
        if (g_scan_complete == FALSE) {
            if (g_inquiry_complete == TRUE)
                snprintf(cBuf + strlen(cBuf), sizeof(cBuf) - strlen(cBuf), "%s", uistr_info_bt_dev_list_end);
        } else {
            snprintf(cBuf + strlen(cBuf), sizeof(cBuf) - strlen(cBuf), "%s", uistr_info_bt_scan_list_end);
            if (FTM_MANUAL_ITEM != pTest->test_type)
                pTest->test_result = TEST_RESULT_PASS;
        }
    }

    memcpy(output_buf, cBuf, strlen(cBuf) + 1);
    return;
}

static void *BT_FM_Thread(void *ptr)
{
    BT_HCI_EVENT_T hci_event;
    UINT8 ucHeader = 0;
    BT_INFO_T *pBtInfo = NULL, *pBtInfoForList = NULL;
    int   rssi = 0;

    TRC();

    while (!fgKillThread) {

        if (g_scan_complete == TRUE) {
            DBG("Scan complete\n");
            break;
        }

        if (!glib_handle) {
            ERR("mtk bt library is unloaded!\n");
            break;
        }
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
            break;
        }

        if (bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
            ERR("Zero byte read\n");
            continue;
        }

        switch (ucHeader) {
          case 0x04:
            DBG("Receive HCI event\n");
            if (BT_Recv_HciEvent(&hci_event)) {
                if (hci_event.event == 0x0F) {
                    /* Command status event */
                    if (hci_event.len != 0x04) {
                        ERR("Unexpected command status event len %d", (int)hci_event.len);
                        goto CleanUp;
                    }

                    if (hci_event.parms[0] != 0x00) {
                        ERR("Unexpected command status %02x", hci_event.parms[0]);
                        goto CleanUp;
                    }
                } else if (hci_event.event == 0x01) {
                    /* Inquiry complete event */
                    if (hci_event.len != 0x01) {
                        ERR("Unexpected inquiry complete event len %d", (int)hci_event.len);
                        goto CleanUp;
                    }

                    if (hci_event.parms[0] != 0x00) {
                        ERR("Unexpected inquiry complete status %02x", hci_event.parms[0]);
                        goto CleanUp;
                    }

                    g_inquiry_complete = TRUE;
                    DBG("Inquiry complete\n");

                    pBtInfo = g_pBtListHear;
                    DBG("Start remote name request\n");
                    while (pBtInfo && !fgKillThread) {
                        BT_RemoteNameReq(pBtInfo);
                        pBtInfo = pBtInfo->pNext;
                    }

                    g_scan_complete = TRUE;
                } else if (hci_event.event == 0x02) {
                    /* Inquiry result event */
                    /* should not be received */
                    goto CleanUp;
                } else if (hci_event.event == 0x22) {
                    /* Inquiry result with RSSI */
                    pBtInfo = (BT_INFO_T *)malloc(sizeof(BT_INFO_T));
                    memset(pBtInfo, 0, sizeof(BT_INFO_T));

                    if (hci_event.len != 0x0F) {
                        ERR("Unexpected inquiry result event len %d\n", (int)hci_event.len);
                        goto CleanUp;
                    }

                    /* negative 2's complement */
                    rssi = hci_event.parms[14];
                    if (rssi >= 128)
                        rssi -= 256;

                    /* Update record */
                    pBtInfo->btaddr[0] = hci_event.parms[6];
                    pBtInfo->btaddr[1] = hci_event.parms[5];
                    pBtInfo->btaddr[2] = hci_event.parms[4];
                    pBtInfo->btaddr[3] = hci_event.parms[3];
                    pBtInfo->btaddr[4] = hci_event.parms[2];
                    pBtInfo->btaddr[5] = hci_event.parms[1];
                    pBtInfo->psr = hci_event.parms[7];
                    pBtInfo->cod[0] = hci_event.parms[11];
                    pBtInfo->cod[1] = hci_event.parms[10];
                    pBtInfo->cod[2] = hci_event.parms[9];
                    pBtInfo->clkoffset[0] = hci_event.parms[13];
                    pBtInfo->clkoffset[1] = hci_event.parms[12];
                    pBtInfo->rssi = rssi; //-120 - 20dbm

                    /* Insert into list */
                    if (g_pBtListHear == NULL)
                        g_pBtListHear = pBtInfo;
                    else {
                        pBtInfoForList = g_pBtListHear;
                        while(pBtInfoForList->pNext != NULL) {
                            pBtInfoForList = pBtInfoForList->pNext;
                        }
                        pBtInfoForList->pNext = pBtInfo;
                    }
                } else {
                    /* simply ignore it? */
                    DBG("Unexpected event %02x len %d %02x-%02x-%02x-%02x\n",
                        hci_event.event, (int)hci_event.len,
                        hci_event.parms[0], hci_event.parms[1],
                        hci_event.parms[2], hci_event.parms[3]);
                }
            } else {
                ERR("Read event fails errno %d\n", errno);
                goto CleanUp;
            }
            break;

          default:
            ERR("Unexpected BT packet header %02x\n", ucHeader);
            goto CleanUp;
        }
    }

CleanUp:
    return NULL;
}
#endif
