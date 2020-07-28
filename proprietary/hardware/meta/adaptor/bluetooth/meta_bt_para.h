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

#ifndef __META_BT_PARA_H__
#define __META_BT_PARA_H__

#include "MetaPub.h"

#define UNUSED_ATTR __attribute__((unused))

#define FT_CNF_OK     0
#define FT_CNF_FAIL   1

#include <stdbool.h>
#ifndef FALSE
#define FALSE    0
#endif
#ifndef TRUE
#define TRUE     1
#endif
#ifndef BOOL
#define BOOL     bool
#endif

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

#ifdef __cplusplus
extern "C" {
#endif

#define PKT_TYPE_CMD        0
#define PKT_TYPE_EVENT      1
#define PKT_TYPE_SCO        2
#define PKT_TYPE_ACL        3

/*
* Test case enum defination for BT_module
*/
typedef enum {
  BT_OP_HCI_SEND_COMMAND = 0
  ,BT_OP_HCI_CLEAN_COMMAND
  ,BT_OP_HCI_SEND_DATA
  ,BT_OP_HCI_TX_PURE_TEST
  ,BT_OP_HCI_RX_TEST_START
  ,BT_OP_HCI_RX_TEST_END
  ,BT_OP_HCI_TX_PURE_TEST_V2
  ,BT_OP_HCI_RX_TEST_START_V2
  ,BT_OP_ENABLE_NVRAM_ONLINE_UPDATE
  ,BT_OP_DISABLE_NVRAM_ONLINE_UPDATE
  ,BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL
  ,BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL
  ,BT_OP_GET_CHIP_ID
  ,BT_OP_INIT
  ,BT_OP_DEINIT
  ,BT_OP_END
} BT_OP;

typedef enum {
  BT_CHIP_ID_MT6611 = 0
  ,BT_CHIP_ID_MT6612
  ,BT_CHIP_ID_MT6616
  ,BT_CHIP_ID_MT6620
  ,BT_CHIP_ID_MT6622
  ,BT_CHIP_ID_MT6626
} BT_CHIP_ID;

/*
* Structure Defination
*/
typedef struct _BT_HCI_CMD {
  UINT16 opcode;
  UINT8  len;
  UINT8  parms[256];
} BT_HCI_CMD;

typedef struct _BT_HCI_BUFFER {
  UINT16 con_hdl;
  UINT16 len;
  UINT8  buffer[1024];
} BT_HCI_BUFFER;

typedef union _BT_CMD {
  BT_HCI_CMD    hcicmd;
  BT_HCI_BUFFER hcibuf;
  UINT32        dummy;
} BT_CMD;

typedef struct _BT_HCI_EVENT {
  UINT8  event;
  UINT16 handle;
  UINT8  len;
  UINT8  status;
  UINT8  parms[256];
} BT_HCI_EVENT;

typedef union _BT_RESULT {
  BT_HCI_EVENT  hcievent;
  BT_HCI_BUFFER hcibuf;
  UINT32        dummy;
} BT_RESULT;

typedef struct _BT_REQ {
  FT_H        header;
  BT_OP       op;
  BT_CMD      cmd;
} BT_REQ;

typedef struct _BT_CNF {
  FT_H        header;
  BT_OP       op;
  UINT32      bt_status;
  UINT8       result_type; /* result type */
  BT_RESULT   result; /* result */
  META_RESULT status;
} BT_CNF;

BOOL META_BT_init(void);
void META_BT_deinit(void);
void META_BT_OP(BT_REQ *req, char *buf, unsigned short len);

#ifdef __cplusplus
};
#endif
#endif
