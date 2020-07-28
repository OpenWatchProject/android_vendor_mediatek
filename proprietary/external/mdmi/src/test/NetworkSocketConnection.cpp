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

#include "NetworkSocketConnection.h"
#include <unistd.h>

#include "Mdmi_utils.h"

#undef TAG
#define TAG "MDMI-NetworkSocketConnection"

NetworkSocketConnection::NetworkSocketConnection():m_isValid(false), m_fd(-1) {
}
NetworkSocketConnection::NetworkSocketConnection(int fd):m_isValid(true), m_fd(fd) {
}

bool NetworkSocketConnection::IsValid() const {
    return m_isValid;
}

void NetworkSocketConnection::Reset() {
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
    m_isValid = false;
}

void NetworkSocketConnection::Drain() {
    uint8_t buffer[65536];
    while (Read(buffer, sizeof(buffer)) > 0);
}

ssize_t NetworkSocketConnection::Read(uint8_t *buffer, size_t bufferSize) {
    ssize_t readBytes = -1;
    int retry = 0;
#define READ_RETRY_TIMES 5
    MDMI_LOGD(TAG, "NetworkSocketConnection::Read");

    if (NULL == buffer || 0 == bufferSize) {
        MDMI_LOGE(TAG, "Bad parameters. buffer = [%p], bufferSize = [%zu]", buffer, bufferSize);
        return -2;
    }

    while (true) {
        if (0 > (readBytes = read(m_fd, buffer, bufferSize))) {
            if (errno == EAGAIN || errno == EINTR) {
                if (retry == READ_RETRY_TIMES) {
                    MDMI_LOGE(TAG, "No data to read from fd: [%d], retry [%d] times when err = [%s](%d)", m_fd, retry, strerror(errno), errno);
                    return 0;
                }
                ++retry;
                continue;
            }
            MDMI_LOGE(TAG, "Failed to read from fd: [%d], err = [%s](%d)", m_fd, strerror(errno), errno);
            return -1;
        } else if (0 == readBytes) {
            MDMI_LOGE(TAG, "Socket is closed. fd: [%d]", m_fd);
            m_isValid = false;
            return -1;
        }
        MDMI_LOGE(TAG, "NetworkSocketConnection::Read done! read [%zd] bytes.", readBytes);
        return readBytes;
    }
}

bool NetworkSocketConnection::Write(const uint8_t *data, size_t len, volatile const bool *bStopFlag) {
    ssize_t writtenBytes = -1;
    size_t totalBytes = 0;

    MDMI_LOGD(TAG, "NetworkSocketConnection::Write to fd: [%d] size=[%zu]", m_fd, len);

    if (data == NULL) {
        return false;
    }

    if (len == 0) {
        return true;
    }

    if (!IsValid()) {
        return false;
    }

    MDMI_LOGD(TAG, "write to fd: [%d]", m_fd);

    while (totalBytes < len) {
        if (NULL != bStopFlag && *bStopFlag) {
            return false;
        }
        if (0 > (writtenBytes = write(m_fd, data + totalBytes, len - totalBytes))) {
            if (errno == EAGAIN || errno == EINTR) {
                if (NULL != bStopFlag && *bStopFlag) {
                    return false;
                }
                continue;
            }
            MDMI_LOGE(TAG, "Failed to write to fd: [%d], err = [%s](%d)", m_fd, strerror(errno), errno);
            m_isValid = false;
            return false;
        }
        totalBytes += writtenBytes;
    }

    MDMI_LOGI(TAG, "NetworkSocketConnection::Send to fd: [%d] size=[%zu] successfully.", m_fd, len);
    return true;
}