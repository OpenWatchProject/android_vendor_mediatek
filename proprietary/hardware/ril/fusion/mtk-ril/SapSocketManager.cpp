/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <cutils/sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <libmtkrilutils.h>
#include "SapSocketManager.h"

#define LOG_TAG "SapSocketMgr"

SapSocketManager::SapSocketManager() {

    RFX_LOG_D(LOG_TAG, "[SapSocketManager] constructor");
    for (int i = 0; i < getSimCount(); i++) {
        socket_sap_gsm_fds[i] = -1;
        socket_sap_c2k_fds[i] = -1;
        socket_sap_bt_fds[i] = -1;
        pthread_mutex_init(&sap_socket_gsm_mutex[i], NULL);
        pthread_mutex_init(&sap_socket_c2k_mutex[i], NULL);
        pthread_mutex_init(&sap_socket_bt_mutex[i], NULL);
    }
}

bool SapSocketManager::setSapSocket(RIL_SOCKET_ID socketId, RILD_RadioTechnology_Group group,
        int fd) {
    if (socketId >= getSimCount()) {
        RFX_LOG_D(LOG_TAG, "[SapSocketManager] setSapSocket parameter error: %d", socketId);
        return false;
    }

    switch (group) {
        case RADIO_TECH_GROUP_SAP_GSM:
            pthread_mutex_lock(&sap_socket_gsm_mutex[socketId]);
            socket_sap_gsm_fds[socketId] = fd;
            pthread_mutex_unlock(&sap_socket_gsm_mutex[socketId]);
            RFX_LOG_D(LOG_TAG, "[SapSocketManager] setSapSocket (socketId=%d, group=%d, fd=%d)",
                    socketId, group, fd);
            break;
        case RADIO_TECH_GROUP_SAP_C2K:
            pthread_mutex_lock(&sap_socket_c2k_mutex[socketId]);
            socket_sap_c2k_fds[socketId] = fd;
            pthread_mutex_unlock(&sap_socket_c2k_mutex[socketId]);
            RFX_LOG_D(LOG_TAG, "[SapSocketManager] setSapSocket (socketId=%d, group=%d, fd=%d)",
                    socketId, group, fd);
            break;
        case RADIO_TECH_GROUP_SAP_BT:
            pthread_mutex_lock(&sap_socket_bt_mutex[socketId]);
            socket_sap_bt_fds[socketId] = fd;
            pthread_mutex_unlock(&sap_socket_bt_mutex[socketId]);
            RFX_LOG_D(LOG_TAG, "[SapSocketManager] setSapSocket (socketId=%d, group=%d, fd=%d)",
                    socketId, group, fd);
            break;
    }
    return true;
}

bool SapSocketManager::sendSapMessageToRild(void *data, int datalen, RIL_SOCKET_ID socketId) {
    bool ret = true;
    int targetFd = -1;

    if (data == NULL) {
        RFX_LOG_D(LOG_TAG, "[SapSocketManager] sendSapMessageToRild arg check fail");
        return false;
    }

    RILD_RadioTechnology_Group dest = SapSocketManager::choiceDestViaCurrCardType(socketId);
    if (RADIO_TECH_GROUP_GSM == dest) {
        // send to gsm rild
        targetFd = socket_sap_gsm_fds[socketId];
        RFX_LOG_D(LOG_TAG, "[SapSocketManager] sendSapMessageToRild send to gsm, fd = %d", targetFd);
    } else if (RADIO_TECH_GROUP_C2K == dest) {
        // send to c2k rild
        targetFd = socket_sap_c2k_fds[socketId];
        RFX_LOG_D(LOG_TAG, "[SapSocketManager] sendSapMessageToRild send to c2k, fd = %d", targetFd);
    }
    if (targetFd == -1) {
        RFX_LOG_D(LOG_TAG, "[SapSocketManager] sendSapMessageToRild socket not ready (dest=%d, targetFd=%d)",
                dest, targetFd);
        return false;
    }

    if (RADIO_TECH_GROUP_GSM == dest) {
        pthread_mutex_lock(&sap_socket_gsm_mutex[socketId]);
    } else {
        pthread_mutex_lock(&sap_socket_c2k_mutex[socketId]);
    }
    ssize_t sent = 0;
    sent = send(targetFd , data , datalen, 0);
    if (sent < 0) {
        RFX_LOG_D(LOG_TAG, "[SapSocketManager]\
                sendSapMessageToRild send data fail (sent=%d, err=%d)(%d, %d, %d, dataSize=%d)",
                sent, errno, socketId, dest, targetFd, datalen);
        ret = false;
    }
    if (RADIO_TECH_GROUP_GSM == dest) {
        pthread_mutex_unlock(&sap_socket_gsm_mutex[socketId]);
    } else {
        pthread_mutex_unlock(&sap_socket_c2k_mutex[socketId]);
    }

    return ret;
}

RILD_RadioTechnology_Group SapSocketManager::choiceDestViaCurrCardType(int slotId) {
    RILD_RadioTechnology_Group dest = RADIO_TECH_GROUP_GSM;
    const char *fullUiccType = NULL;
    char tmp[MTK_PROPERTY_VALUE_MAX] = {0};

    do {
        if (slotId >= 0 && slotId < MAX_SIM_COUNT) {
            fullUiccType = PROPERTY_RIL_FULL_UICC_TYPE[slotId];
            rfx_property_get(fullUiccType, tmp, "");
        } else {
            RFX_LOG_D(LOG_TAG, "[SapSocketManager] choiceDestViaCurrCardType, slotId %d is wrong!", slotId);
            break;
        }

        if ((strncmp(tmp, "SIM", strlen("SIM")) == 0) || (strstr(tmp, "USIM") != NULL)) {
            // Send GSM RILD the request if there is SIM/USIM
            break;
        }

        if ((strstr(tmp, "CSIM") != NULL) || (strstr(tmp, "RUIM") != NULL)) {
            // Send C2K RILD the request if this is pure CDMA card
            dest = RADIO_TECH_GROUP_C2K;
            break;
        }

        RFX_LOG_D(LOG_TAG, "[SapSocketManager] choiceDestViaCurrCardType, No 3GPP and 3GPP2?!(slot %d)",
                slotId);
    } while (0);

    RFX_LOG_D(LOG_TAG, "[SapSocketManager] choiceDestViaCurrCardType, dest %d! (slot %d)", dest, slotId);
    return dest;
}
