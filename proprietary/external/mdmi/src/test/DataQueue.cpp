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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>     /* offsetof */
#include <string.h>
#include "DataQueue.h"

DataQueue::DataQueue():m_size(0), m_unusedSize(0) {
}

DataQueue::~DataQueue() {
}

void DataQueue::Clear() {
    m_size = 0;
    m_unusedSize = 0;
}

size_t DataQueue::Size() const {
    return m_size;
}

size_t DataQueue::SpaceSize() const {
    return sizeof(m_data) - m_size - m_unusedSize;
}

const uint8_t *DataQueue::Front() const {
    return m_data + m_unusedSize;
}

void DataQueue::Pop(size_t size) {
    if (m_size <= size) {
        m_size = 0;
        m_unusedSize = 0;
    } else {
        m_unusedSize += size;
        m_size -= size;
        if (0 != m_unusedSize && (0 == size || m_unusedSize + m_size > (sizeof(m_data) >> 1))) {
            memmove(m_data, m_data + m_unusedSize, m_size);
            m_unusedSize = 0;
        }
    }
}

bool DataQueue::Push(const uint8_t *data, size_t nLen) {
    if (NULL == data) {
        return false;
    }
    if (0 == nLen) {
        return true;
    }
    if (nLen + m_size > sizeof(m_data)) {
        return false;
    }
    if (nLen + m_unusedSize + m_size > sizeof(m_data)) {
        memmove(m_data, m_data + m_unusedSize, m_size);
        m_unusedSize = 0;
    }
    memcpy(m_data + m_unusedSize + m_size, data, nLen);
    m_size += nLen;
    return true;
}

uint8_t *DataQueue::SpaceStart() {
    return m_data + m_unusedSize + m_size;
}

void DataQueue::IncSize(size_t size) {
    if (size > SpaceSize()) {
        size = SpaceSize();
    }
    m_size += size;
}
