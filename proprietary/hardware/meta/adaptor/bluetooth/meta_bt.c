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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

#include "meta_bt.h"


/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static int  bt_fd = -1;
static BOOL bt_enabled = FALSE;
static BT_CNF_CB cnf_cb = NULL;
static BT_CNF bt_cnf;

/* Used to read serial port */
static pthread_t rxThread;
static BOOL fgKillThread = FALSE;

/* mtk bt library */
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*DEINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buf, unsigned int len);
typedef int (*READ)(int fd, unsigned char *buf, unsigned int len);
typedef int (*GETID)(unsigned int *pChipId);

INIT    meta_bt_init = NULL;
DEINIT  meta_bt_restore = NULL;
WRITE   meta_bt_send_data = NULL;
READ    meta_bt_receive_data = NULL;
GETID   meta_bt_get_combo_id = NULL;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BOOL BT_Send_HciCmd(BT_HCI_CMD *pHciCmd);
static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *pHciEvent);
static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData);
static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pAclData);

static void* BT_Meta_Thread(void* ptr);

/**************************************************************************
  *                         F U N C T I O N S                             *
***************************************************************************/

static void bt_send_resp(BT_CNF *cnf, unsigned short size, void *buf, unsigned short len)
{
    if (cnf_cb)
        cnf_cb(cnf, buf, len);
    else
        WriteDataToPC(cnf, size, buf, len);
}

void META_BT_Register(BT_CNF_CB callback)
{
    cnf_cb = callback;
}

BOOL META_BT_init(void)
{
    const char *errstr;

    TRC();

    glib_handle = dlopen("libbluetooth_mtk_pure.so", RTLD_LAZY);
    if (!glib_handle) {
        ERR("%s\n", dlerror());
        goto error;
    }

    dlerror(); /* Clear any existing error */

    meta_bt_init = dlsym(glib_handle, "bt_init");
    meta_bt_restore = dlsym(glib_handle, "bt_restore");
    meta_bt_send_data = dlsym(glib_handle, "bt_send_data");
    meta_bt_receive_data = dlsym(glib_handle, "bt_receive_data");
    meta_bt_get_combo_id = dlsym(glib_handle, "bt_get_combo_id");

    if ((errstr = dlerror()) != NULL) {
        ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }

    bt_fd = meta_bt_init();
    if (bt_fd < 0)
        goto error;

    DBG("BT is enabled success\n");

    /* Create RX thread */
    fgKillThread = FALSE;
    pthread_create(&rxThread, NULL, BT_Meta_Thread, (void*)&bt_cnf);

    bt_enabled = TRUE;
    sched_yield();

    return TRUE;

error:
    if (glib_handle) {
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    return FALSE;
}

void META_BT_deinit(void)
{
    TRC();

    /* Stop RX thread */
    fgKillThread = TRUE;
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
    }
    else {
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
        }
        else {
            meta_bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }

    bt_enabled = FALSE;
    return;
}

void META_BT_OP(BT_REQ *req, UNUSED_ATTR char *buf, UNUSED_ATTR unsigned short len)
{
    TRC();

    if (NULL == req) {
        ERR("Invalid arguments or operation!\n");
        return;
    }

    memset(&bt_cnf, 0, sizeof(BT_CNF));
    bt_cnf.header.id = FT_BT_CNF_ID;
    bt_cnf.header.token = req->header.token;
    bt_cnf.op = req->op;

    if (bt_enabled == FALSE) {
        /*
           Initialize BT module when it is called first time
           to avoid the case that PC tool not send BT_OP_INIT
        */
        if (META_BT_init() == FALSE) {
            bt_cnf.bt_status = FALSE;
            bt_cnf.status = META_FAILED;
            bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
            return;
        }
    }

    switch (req->op)
    {
      case BT_OP_INIT:
        if ((bt_enabled == FALSE) && (META_BT_init() == FALSE)){
            bt_cnf.bt_status = FALSE;
            bt_cnf.status = META_FAILED;
        }
        else{
            bt_cnf.bt_status = TRUE;
            bt_cnf.status = META_SUCCESS;
        }

        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        break;

      case BT_OP_DEINIT:
        if (bt_enabled == TRUE)
            META_BT_deinit();

        bt_cnf.bt_status = TRUE;
        bt_cnf.status = META_SUCCESS;
        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        break;

      case BT_OP_GET_CHIP_ID:
      {
        UINT32 chipId;

        DBG("BT_OP_GET_CHIP_ID\n");
        if (meta_bt_get_combo_id(&chipId) < 0) {
            ERR("Get combo chip id fails\n");
            bt_cnf.bt_status = FALSE;
            bt_cnf.status = META_FAILED;
            bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
            break;
        }
        bt_cnf.result.dummy = chipId;
        bt_cnf.bt_status = TRUE;
        bt_cnf.status = META_SUCCESS;
        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        break;
      }
      case BT_OP_HCI_SEND_COMMAND:
        DBG("BT_OP_HCI_SEND_COMMAND\n");
        if (BT_Send_HciCmd(&req->cmd.hcicmd) == FALSE) {
            bt_cnf.bt_status = FALSE;
            bt_cnf.status = META_FAILED;
            bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        }
        break;

      case BT_OP_HCI_SEND_DATA:
        DBG("BT_OP_HCI_SEND_DATA\n");
        if (BT_Send_AclData(&req->cmd.hcibuf) == FALSE) {
            bt_cnf.bt_status = FALSE;
            bt_cnf.status = META_FAILED;
            bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        }
        break;

      case BT_OP_HCI_CLEAN_COMMAND:
      case BT_OP_HCI_TX_PURE_TEST:
      case BT_OP_HCI_RX_TEST_START:
      case BT_OP_HCI_RX_TEST_END:
      case BT_OP_HCI_TX_PURE_TEST_V2:
      case BT_OP_HCI_RX_TEST_START_V2:
      case BT_OP_ENABLE_NVRAM_ONLINE_UPDATE:
      case BT_OP_DISABLE_NVRAM_ONLINE_UPDATE:

      case BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL:
      case BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL:
        /* Need to confirm with CCCI driver buddy */
        DBG("Not implemented command %d\n", req->op);
        bt_cnf.status = META_FAILED;
        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        break;

      default:
        DBG("Unknown command %d\n", req->op);
        bt_cnf.status = META_FAILED;
        bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
        break;
    }

    return;
}

static BOOL BT_Send_HciCmd(BT_HCI_CMD *pHciCmd)
{
    UINT8 ucHciCmd[256+4];
    UINT8 i = 0;
    unsigned char str[100] = {""};
    unsigned char buf_str[6] = {""};

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

    if (pHciCmd->len) {
        memcpy(&ucHciCmd[4], pHciCmd->parms, pHciCmd->len);
    }

    if (meta_bt_send_data(bt_fd, ucHciCmd, pHciCmd->len + 4) < 0) {
        ERR("Write HCI command fails errno %d\n", errno);
        return FALSE;
    }

    /* Dump packet */
    for (i = 0; i < pHciCmd->len + 4; i++) {
        if ((i % 16 == 0) && (i != 0)) {
            DBG("%s\n", str);
            memset(str, 0, sizeof(str));
        }
        snprintf(buf_str, sizeof(buf_str), "0x%02x ", ucHciCmd[i]);
        strncat(str, buf_str, strlen(buf_str));
    }
    DBG("%s\n", str);

    return TRUE;
}

static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *pHciEvent)
{
    UINT8 i = 0;
    unsigned char str[100] = {""};
    unsigned char buf_str[6] = {""};
    pHciEvent->status = FALSE;

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (meta_bt_receive_data(bt_fd, &pHciEvent->event, 1) < 0) {
        ERR("Read event code fails errno %d\n", errno);
        return FALSE;
    }

    DBG("Read event code: %02x\n", pHciEvent->event);

    if (meta_bt_receive_data(bt_fd, &pHciEvent->len, 1) < 0) {
        ERR("Read event length fails errno %d\n", errno);
        return FALSE;
    }

    DBG("Read event length: %d\n", pHciEvent->len);

    if (pHciEvent->len) {
        if (meta_bt_receive_data(bt_fd, pHciEvent->parms, pHciEvent->len) < 0) {
            ERR("Read event param fails errno %d\n", errno);
            return FALSE;
        }
    }

    pHciEvent->status = TRUE;

    /* Dump packet */
    for (i = 0; i < pHciEvent->len; i++) {
        if ((i % 16 == 0) && (i != 0)) {
            DBG("%s\n", str);
            memset(str, 0, sizeof(str));
        }
        snprintf(buf_str, sizeof(buf_str), "0x%02x ", pHciEvent->parms[i]);
        strncat(str, buf_str, strlen(buf_str));
    }
    DBG("%s\n", str);

    return TRUE;
}

static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData)
{
    UINT8 ucAclData[1029];

    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    ucAclData[0] = 0x02;
    ucAclData[1] = (pAclData->con_hdl) & 0xFF;
    ucAclData[2] = (pAclData->con_hdl >> 8) & 0xFF;
    ucAclData[3] = (pAclData->len) & 0xFF;
    ucAclData[4] = (pAclData->len >> 8) & 0xFF;

    if (pAclData->len) {
        memcpy(&ucAclData[5], pAclData->buffer, pAclData->len);
    }

    if (meta_bt_send_data(bt_fd, ucAclData, pAclData->len + 5) < 0) {
        ERR("Write ACL data fails errno %d\n", errno);
        return FALSE;
    }

    return TRUE;
}

static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pAclData)
{
    if (!glib_handle) {
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0) {
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

    if (meta_bt_receive_data(bt_fd, (UINT8*)&pAclData->con_hdl, 2) < 0) {
        ERR("Read connection handle fails errno %d\n", errno);
        return FALSE;
    }

    pAclData->con_hdl = ((pAclData->con_hdl & 0xFF) << 8) | ((pAclData->con_hdl >> 8) & 0xFF);

    if (meta_bt_receive_data(bt_fd, (UINT8*)&pAclData->len, 2) < 0) {
        ERR("Read ACL data length fails errno %d\n", errno);
        return FALSE;
    }

    pAclData->len = ((pAclData->len & 0xFF) << 8) | ((pAclData->len >> 8) & 0xFF);

    if (pAclData->len) {
        if (meta_bt_receive_data(bt_fd, pAclData->buffer, pAclData->len) < 0) {
            ERR("Read ACL data fails errno %d\n", errno);
            return FALSE;
        }
    }

    return TRUE;
}


static void *BT_Meta_Thread(void *ptr)
{
    BT_CNF *pBtCnf = (BT_CNF*)ptr;
    BT_HCI_EVENT hci_event;
    BT_HCI_BUFFER acl_data;
    UINT8  ucHeader = 0;

    TRC();

    while (!fgKillThread) {

        if (!glib_handle) {
            ERR("mtk bt library is unloaded!\n");
            break;
        }
        if (bt_fd < 0) {
            ERR("bt driver fd is invalid!\n");
            break;
        }

        if (meta_bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0) {
            ERR("Zero byte read\n");
            continue;
        }

        switch (ucHeader) {
          case 0x04:
            DBG("Receive HCI event\n");
            if (BT_Recv_HciEvent(&hci_event)) {
                pBtCnf->bt_status = TRUE;
                pBtCnf->result_type = PKT_TYPE_EVENT;
                memcpy(&pBtCnf->result.hcievent, &hci_event, sizeof(hci_event));
                pBtCnf->status = META_SUCCESS;
                bt_send_resp(pBtCnf, sizeof(BT_CNF), NULL, 0);
            }
            else {
                pBtCnf->bt_status = FALSE;
                pBtCnf->status = META_FAILED;
                bt_send_resp(pBtCnf, sizeof(BT_CNF), NULL, 0);
            }
            break;

          case 0x02:
            DBG("Receive ACL data\n");
            if (BT_Recv_AclData(&acl_data)) {
                pBtCnf->bt_status = TRUE;
                pBtCnf->result_type = PKT_TYPE_ACL;
                memcpy(&pBtCnf->result.hcibuf, &acl_data, sizeof(acl_data));
                pBtCnf->status = META_SUCCESS;
                bt_send_resp(pBtCnf, sizeof(BT_CNF), NULL, 0);
            }
            else {
                pBtCnf->bt_status = FALSE;
                pBtCnf->status = META_FAILED;
                bt_send_resp(pBtCnf, sizeof(BT_CNF), NULL, 0);
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
