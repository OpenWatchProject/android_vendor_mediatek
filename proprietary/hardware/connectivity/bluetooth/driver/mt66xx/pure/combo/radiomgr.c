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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>

#include "bt_kal.h"

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

extern ENUM_BT_STATUS_T GORM_Init(INT32 comPort, UINT32 chipId, PUCHAR pucNvRamData);

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

BOOL BT_InitDevice(INT32 comPort, UINT32 chipId, PUCHAR pucNvRamData)
{
    ENUM_BT_STATUS_T status;

    LOG_DBG("BT_InitDevice\n");

    /* Patch download is moved to WMT driver on combo chip */

    /* Invoke HCI transport entrance */
    status = GORM_Init(comPort, chipId, pucNvRamData);

    if (status != 0) {
        LOG_ERR("GORM fails return code %d\n", (INT32)status);
        return FALSE;
    }

    return TRUE;
}

BOOL BT_DeinitDevice(UNUSED_ATTR INT32 comPort)
{
    LOG_DBG("BT_DeinitDevice\n");
    return TRUE;
}

BOOL BT_SendHciCommand(INT32 comPort, HCI_CMD_T *pHciCommand)
{
    UINT8 ucHciCmd[256+4] = {0x01, 0x00, 0x00, 0x00};

    ucHciCmd[1] = (UINT8)pHciCommand->opCode;
    ucHciCmd[2] = (UINT8)(pHciCommand->opCode >> 8);
    ucHciCmd[3] = pHciCommand->len;

    LOG_DBG("OpCode 0x%04x len %d\n", pHciCommand->opCode, (INT32)pHciCommand->len);

    if (pHciCommand->len) {
        memcpy(&ucHciCmd[4], pHciCommand->parms, pHciCommand->len);
    }

    if (bt_send_data(comPort, ucHciCmd, pHciCommand->len + 4) < 0) {
        LOG_ERR("Write HCI command fails errno %d\n", errno);
        return FALSE;
    }

    return TRUE;
}

static BOOL BT_ReadPacketHeader(
    INT32   comPort,
    UINT8*  pPacketType, /* Command, Event, ACL data, SCO data */
    UINT16* pRemainLen, /* Remaining len for parameters */
    UINT16* pOpCode, /* Command OpCode */
    UINT16* pConnHandle, /* Connection handle */
    UINT8*  pEventCode /* Event code */
    )
{
    UINT8   ucCmdHdr[3];
    UINT8   ucAclHdr[4];
    UINT8   ucScoHdr[3];
    UINT8   ucEventHdr[2];
    UINT8   type = 0;

    /* Read UART header */
    if (bt_receive_data(comPort, &type, 1) < 0) {
        LOG_ERR("Read packet header fails\n");
        return FALSE;
    }

    *pPacketType = type;

    switch (type) {
      case 1: /* Command */
        if (bt_receive_data(comPort, ucCmdHdr, 3) < 0) {
            LOG_ERR("Read command header fails %d\n", errno);
            return FALSE;
        }

        *pOpCode = (((UINT16)ucCmdHdr[0]) | (((UINT16)ucCmdHdr[1]) << 8));
        *pRemainLen = ucCmdHdr[2];
        break;

      case 2: /* ACL data */
        if (bt_receive_data(comPort, ucAclHdr, 4) < 0) {
            LOG_ERR("Read ACL header fails %d\n", errno);
            return FALSE;
        }

        *pConnHandle = (((UINT16)ucAclHdr[0]) | (((UINT16)ucAclHdr[1]) << 8));
        *pRemainLen = (((UINT16)ucAclHdr[2]) | (((UINT16)ucAclHdr[3]) << 8));
        break;

      case 3: /* SCO data */
        if (bt_receive_data(comPort, ucScoHdr, 3) < 0) {
            LOG_ERR("Read SCO header fails %d\n", errno);
            return FALSE;
        }

        *pConnHandle = (((UINT16)ucScoHdr[0]) | (((UINT16)ucScoHdr[1]) << 8));
        *pRemainLen = ucScoHdr[2];
        break;

      case 4: /* Event */
        if (bt_receive_data(comPort, ucEventHdr, 2) < 0) {
            LOG_ERR("Read event header fails %d\n", errno);
            return FALSE;
        }

        *pEventCode = ucEventHdr[0];
        *pRemainLen = ucEventHdr[1];
        break;

      default: /* other */
        LOG_ERR("Unknown packet type %02x\n", type);
        return FALSE;
        break;
    }

    return TRUE;
}

static BOOL BT_ReadPacket(
    INT32   comPort,
    UINT8*  pPacket,
    UINT32  u4MaxBufSz,
    UINT32* pu4PktLen
    )
{
    UINT8   packetType;
    UINT16  remainLen;
    UINT16  opCode, connHandle;
    UINT8   eventCode;
    UINT32  u4PktLen = 0;

    if (u4MaxBufSz == 0) {
        LOG_ERR("Read buffer size is zero!\n");
        return FALSE;
    }

    if (BT_ReadPacketHeader(
          comPort,
          &packetType,
          &remainLen,
          &opCode,
          &connHandle,
          &eventCode) == FALSE) {

        LOG_ERR("Read packet header fails\n");
        return FALSE;
    }

    pPacket[0] = packetType;
    u4PktLen ++;

    /* Command packet */
    if (packetType == 1) {
        if (u4MaxBufSz < (4 + remainLen)) {
            LOG_ERR("Read command buffer is too short!\n");
            return FALSE;
        }

        pPacket[u4PktLen] = (UINT8)opCode;
        pPacket[u4PktLen + 1] = (UINT8)(opCode >> 8);
        u4PktLen += 2;

        pPacket[u4PktLen] = (UINT8)remainLen;
        u4PktLen ++;

        if (bt_receive_data(comPort, pPacket + u4PktLen, remainLen) < 0) {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }

        u4PktLen += remainLen;
        *pu4PktLen = u4PktLen;

        return TRUE;
    }

    /* ACL data */
    if (packetType == 2) {
        if (u4MaxBufSz < (5 + remainLen)) {
            LOG_ERR("Read ACL buffer is too short!\n");
            return FALSE;
        }

        pPacket[u4PktLen] = (UINT8)connHandle;
        pPacket[u4PktLen + 1] = (UINT8)(connHandle >> 8);
        u4PktLen += 2;

        pPacket[u4PktLen] = (UINT8)remainLen;
        pPacket[u4PktLen + 1] = (UINT8)(remainLen >> 8);
        u4PktLen += 2;

        if (bt_receive_data(comPort, pPacket + u4PktLen, remainLen) < 0) {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }

        u4PktLen += remainLen;
        *pu4PktLen = u4PktLen;

        return TRUE;
    }

    /* SCO data */
    if (packetType == 3) {
        if (u4MaxBufSz < (4 + remainLen)) {
            LOG_ERR("Read SCO buffer is too short!\n");
            return FALSE;
        }

        pPacket[u4PktLen] = (UINT8)connHandle;
        pPacket[u4PktLen + 1] = (UINT8)(connHandle >> 8);
        u4PktLen += 2;

        pPacket[u4PktLen] = (UINT8)remainLen;
        u4PktLen ++;

        if (bt_receive_data(comPort, pPacket + u4PktLen, remainLen) < 0) {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }

        u4PktLen += remainLen;
        *pu4PktLen = u4PktLen;

        return TRUE;
    }

    /* Event packet */
    if (packetType == 4) {
        if(u4MaxBufSz < (3 + remainLen)) {
            LOG_ERR("Read event buffer is too short!\n");
            return FALSE;
        }

        pPacket[u4PktLen] = eventCode;
        pPacket[u4PktLen + 1] = (UINT8)remainLen;
        u4PktLen += 2;

        if (bt_receive_data(comPort, pPacket + u4PktLen, remainLen) < 0) {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }

        u4PktLen += remainLen;
        *pu4PktLen = u4PktLen;

        return TRUE;
    }

    LOG_ERR("Unknown packet type\n");

    return FALSE;
}

BOOL BT_ReadExpectedEvent(
    INT32   comPort,
    UINT8*  pEventPacket,
    UINT32  u4MaxBufSz,
    UINT32* pu4PktLen,
    UINT8   ucExpectedEventCode,
    BOOL    fCheckCompleteOpCode,/* whether to check command OpCode */
    UINT16  u2ExpectedOpCode,
    BOOL    fCheckCommandStatus, /* whether to check command status */
    UINT8   ucExpectedStatus
    )
{
    UINT16  u2OpCode;
    UINT8   ucEventCode, ucStatus;
    UINT8   i;

RESTART:
    if (BT_ReadPacket(
          comPort,
          pEventPacket,
          u4MaxBufSz,
          pu4PktLen) == FALSE) {

        LOG_ERR("Read packet fails\n");
        return FALSE;
    }

    /* Expect Event only */
    if (pEventPacket[0] != 4) {
        LOG_ERR("Unexpected packet type\n");
        return FALSE;
    }

    ucEventCode = pEventPacket[1];

    if (ucEventCode == 0xFF) {
        /* Dump rx packet */
        LOG_DBG("Get vendor specific event, dump 11 bytes then drop it!\n");
        for (i = 0; i < *pu4PktLen && i < 11; i++)
            LOG_DBG("%02x\n", pEventPacket[i]);
        goto RESTART;
    }

    if (ucEventCode != ucExpectedEventCode) {
        LOG_ERR("Unexpected event code\n");
        return FALSE;
    }

    if (ucEventCode == 0x0E) {
        if (fCheckCompleteOpCode) {
            u2OpCode = ((UINT16)pEventPacket[4]) | (((UINT16)pEventPacket[5]) << 8);

            if (u2OpCode != u2ExpectedOpCode) {
                LOG_ERR("Unexpected OpCode\n");
                return FALSE;
            }
        }
        if (fCheckCommandStatus) {
            ucStatus = pEventPacket[6];

            if (ucStatus != ucExpectedStatus) {
                LOG_ERR("Unexpected status %02x\n", ucStatus);
                return FALSE;
            }
        }
    }

    if (ucEventCode == 0x0F) {
        if (fCheckCompleteOpCode) {
            u2OpCode = ((UINT16)pEventPacket[5]) | (((UINT16)pEventPacket[6]) << 8);

            if (u2OpCode != u2ExpectedOpCode) {
                LOG_ERR("Unexpected OpCode\n");
                return FALSE;
            }
        }

        if (fCheckCommandStatus) {
            ucStatus = pEventPacket[3];

            if (ucStatus != ucExpectedStatus) {
                LOG_ERR("Unexpected status %02x\n", ucStatus);
                return FALSE;
            }
        }
    }

    return TRUE;
}
