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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "CommandProcessor.h"
#include "Mdmi_utils.h"
#include <unistd.h>



// Init static const member variable in CommandProcessor
const uint8_t CommandProcessor::REDIRECTOR_CMD_HEADER_MAGIC[4] = {0xF7, 0xDC, 0xE2, 0xB9};

#undef TAG
#define TAG "MDMI-CommandProcessor"

CommandProcessor::CommandProcessor(const redirectorCmdEntry *pCmdTable, unsigned int size) {
    m_pCmdTable = pCmdTable;
    mCmdTableSize = size;
}

int CommandProcessor::ParseCommand(DataQueue &dataQ) {
    size_t i = 0;
    int parseCnt = 0;

    MDMI_LOGD(TAG, "CommandProcessor::ParseCommand");

    /* Search for REDIRECTOR_CMD_HEADER_MAGIC(4) + CMD_CODE(2) + ARGS_LEN(4) */
    while (NULL != dataQ.Front() && dataQ.Size() >= sizeof(REDIRECTOR_CMD_HEADER_MAGIC) + 2 + 4) {
        if (0 != memcmp(dataQ.Front(), REDIRECTOR_CMD_HEADER_MAGIC, sizeof(REDIRECTOR_CMD_HEADER_MAGIC))) {
            /* Not REDIRECTOR_CMD_HEADER_MAGIC. Shift one byte and try again */
            MDMI_LOGE(TAG, "Not magic header. Pop(1).");
            dataQ.Pop(1);
            continue;
        }
        MDMI_LOGD(TAG, "Magic header is found.");

        /* Check CMD_CODE */
        const uint16_t *pCmdCode = (const uint16_t *)(dataQ.Front() + sizeof(REDIRECTOR_CMD_HEADER_MAGIC));
        if (NULL == pCmdCode) {
            MDMI_LOGE(TAG, "Incorrent command code. Pop(4).");
            dataQ.Pop(4);
            continue;
        }
        MDMI_LOGD(TAG, "Command code is found. [%" PRIu16 "]", *pCmdCode);

        /* Check ARGS_LEN */
        const uint32_t *pArgsLen = (const uint32_t *)(dataQ.Front() + sizeof(REDIRECTOR_CMD_HEADER_MAGIC) + 2);
        if (NULL == pArgsLen || COMMAND_ARGS_LEN_MAX < *pArgsLen) {
            MDMI_LOGE(TAG, "Incorrent command args len. Pop(4).");
            dataQ.Pop(4);
            continue;
        }
        MDMI_LOGD(TAG, "Argument length is found. [%" PRIu32 "]", *pArgsLen);

        if (dataQ.Size() < sizeof(REDIRECTOR_CMD_HEADER_MAGIC) + 2 + 4 + *pArgsLen) {
            /* Data in Q is not enough, we should wait for more data. */
            break;
        }

        for (i = 0; i < mCmdTableSize; ++i) {
            if (*pCmdCode == m_pCmdTable[i].cmdCode) {
                m_pCmdTable[i].func(*pArgsLen, dataQ.Front() + sizeof(REDIRECTOR_CMD_HEADER_MAGIC) + 2 + 4);
                break;
            }
        }
        if (i == mCmdTableSize) {
            // No such command, something wrong maybe.
            MDMI_LOGE(TAG, "No such command code [%" PRIu16 "]. Pop(4).", *pCmdCode);
            dataQ.Pop(4);
            continue;
        }

        dataQ.Pop(sizeof(REDIRECTOR_CMD_HEADER_MAGIC) + 2 + 4 + *pArgsLen);
        ++parseCnt;
    }
    return parseCnt;
}

bool CommandProcessor::DataIn(SocketConnection &connection) {
    MDMI_LOGD(TAG, "CommandProcessor::DataIn");

    for (int i = 0; i <= 100; ++i) { // Retry 100 times (1 second) at most if no commands are processed.
        if (!connection.ReadToBuffer()) {
            MDMI_LOGD(TAG, "Failed to refresh buffer. [%d] (connection is lost)", i);
            return false;
        }
        DataQueue & dataQ = connection.GetDataQueue();
        if (0 == dataQ.Size()) {
            break;
        }
        if (0 < ParseCommand(dataQ)) {
            // Receive at least one command
            break;
        }
        usleep(10000); // sleep 0.01 second
    }
    return true;
}