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
#include <stdarg.h>
#include "meta_bt.h"

static void bt_info_callback(BT_CNF *cnf, void *buf, unsigned short len)
{
    char *type[] = {
        "BT_OP_HCI_SEND_COMMAND", "BT_OP_HCI_CLEAN_COMMAND", "BT_OP_HCI_SEND_DATA", "BT_OP_HCI_TX_PURE_TEST",
        "BT_OP_HCI_RX_TEST_START", "BT_OP_HCI_RX_TEST_END", "BT_OP_HCI_TX_PURE_TEST_V2", "BT_OP_HCI_RX_TEST_START_V2",
        "BT_OP_ENABLE_NVRAM_ONLINE_UPDATE", "BT_OP_DISABLE_NVRAM_ONLINE_UPDATE", "BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL", "BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL",
        "BT_OP_GET_CHIP_ID", "BT_OP_INIT", "BT_OP_DEINIT"};

    printf("[META_BT] <CNF> %s, bt_status: %d, result_type: %d, status: %d\n",
        type[cnf->op],
        cnf->bt_status, cnf->result_type, cnf->status);

    if (cnf->result_type == PKT_TYPE_EVENT) {
        printf("[META_BT] HCI event %02x, len %d\n", cnf->result.hcievent.event, (int)cnf->result.hcievent.len);
        printf("[META_BT] HCI event %02x-%02x-%02x-%02x\n",
            cnf->result.hcievent.parms[0], cnf->result.hcievent.parms[1],
            cnf->result.hcievent.parms[2], cnf->result.hcievent.parms[3]);

    }
    else if (cnf->result_type == PKT_TYPE_ACL) {
        printf("[META_BT] ACL con_hdl %d, len: %d\n", (int)cnf->result.hcibuffer.con_hdl, (int)cnf->result.hcibuffer.len);
    }
    else {
        printf("[META_BT] Unexpected result type\n");
    }
}

int main(int argc, const char** argv)
{
    BT_REQ req;

    memset(&req, 0, sizeof(BT_REQ));

    META_BT_Register(bt_info_callback);

    if (META_BT_init() == FALSE) {
        printf("BT init fails\n");
        return -1;
    }
#if 0
    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0x0c03;
    req.cmd.hcicmd.len = 0;
    req.cmd.hcicmd.parms[0] = 0;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0xfc72;
    req.cmd.hcicmd.len = 1;
    req.cmd.hcicmd.parms[0] = 0x23;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);
#endif
    sleep(1);
    /* 01,04,05,33,8B,9E,05,0A */
    req.op = BT_OP_HCI_SEND_COMMAND;
    req.cmd.hcicmd.opcode = 0x0401;
    req.cmd.hcicmd.len = 5;
    req.cmd.hcicmd.parms[0] = 0x33;
    req.cmd.hcicmd.parms[1] = 0x8B;
    req.cmd.hcicmd.parms[2] = 0x9E;
    req.cmd.hcicmd.parms[3] = 0x05;
    req.cmd.hcicmd.parms[4] = 0x0A;
    META_BT_OP(&req, NULL, 0);

    sleep(20);

    req.op = BT_OP_HCI_CLEAN_COMMAND;
    META_BT_OP(&req, NULL, 0);

    sleep(1);

    META_BT_deinit();
    META_BT_Register(NULL);

    return 0;
}

