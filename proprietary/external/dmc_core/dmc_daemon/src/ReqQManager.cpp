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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/**
 * @file ReqQManager.cpp
 *
 * @author mtk33273 (Ankur Saxena)
 *
 * @brief The source file maintains ReqThread and provides enqueue and dequeue mechanism.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include "ReqQManager.h"
#include "dmc_utils.h"

#undef TAG
#define TAG "DMC-ReqQManager"

ReqQManager::ReqQManager() {
    DMC_LOGD(TAG, "ReqQManager");
    m_groupSize = 0;
    m_runThread = true;
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
    m_front = m_rear = NULL;

    m_pEventBufferInUse = NULL;
    m_pEventBufferCapacity = NULL;
    m_pDroppedEvents = NULL;

    pthread_create(&m_thread, NULL, ThreadEntry, (void *)this);
}

ReqQManager::~ReqQManager() {
    DMC_LOGD(TAG, "~ReqQManager");

    pthread_mutex_lock(&m_mutex);

    EmptyRequestQueue();

    if (m_pEventBufferInUse != NULL) {
        free(m_pEventBufferInUse);
        m_pEventBufferInUse = NULL;
    }
    if (m_pEventBufferCapacity != NULL) {
        free(m_pEventBufferCapacity);
        m_pEventBufferCapacity = NULL;
    }
    if (m_pDroppedEvents != NULL) {
        free(m_pDroppedEvents);
        m_pDroppedEvents = NULL;
    }

    //signal waiting thread
    pthread_cond_signal(&m_cond);

    pthread_mutex_unlock(&m_mutex);

    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

void ReqQManager::initEventInfoBuffer(uint8_t groupSize) {

    pthread_mutex_lock(&m_mutex);

    if (m_pEventBufferInUse == NULL) {
        m_pEventBufferInUse = (uint32_t *)calloc(groupSize, sizeof(uint32_t));
    } else {
        memset(m_pEventBufferInUse, 0, groupSize*sizeof(uint32_t));
    }

    if (m_pEventBufferCapacity == NULL) {
        m_pEventBufferCapacity = (uint32_t *)calloc(groupSize, sizeof(uint32_t));
    } else {
        memset(m_pEventBufferCapacity, 0, groupSize*sizeof(uint32_t));
    }

    if (m_pDroppedEvents == NULL) {
        m_pDroppedEvents = (uint32_t *)calloc(groupSize, sizeof(uint32_t));
    } else {
        memset(m_pDroppedEvents, 0, groupSize*sizeof(uint32_t));
    }

    pthread_mutex_unlock(&m_mutex);
    DMC_LOGD(TAG, "initEventInfoBuffer(%" PRIu8")", groupSize);
}

void ReqQManager::freeEventInfoBuffer() {

    pthread_mutex_lock(&m_mutex);

    if (m_pEventBufferInUse != NULL) {
        free(m_pEventBufferInUse);
        m_pEventBufferInUse = NULL;
    }
    if (m_pEventBufferCapacity != NULL) {
        free(m_pEventBufferCapacity);
        m_pEventBufferCapacity = NULL;
    }
    if (m_pDroppedEvents != NULL) {
        free(m_pDroppedEvents);
        m_pDroppedEvents = NULL;
    }

    pthread_mutex_unlock(&m_mutex);
    DMC_LOGD(TAG, "freeEventInfoBuffer()");
}

void ReqQManager::EnqueueRequest(int kpi_index, KPI_GROUP_TYPE group, void *kpi_data, int kpi_size, uint64_t timestamp) {
    // DMC_LOGD(TAG, "EnqueueRequest: %s(%d) time = %" PRIu64"", KpiData::GetKpiName(kpi_index), kpi_index, timestamp);
    if (!m_runThread) {
        DMC_LOGW(TAG, "EnqueueRequest: index %d thread not running.", kpi_index);
        return;
    }

    int allocSize = 0;
    allocSize += sizeof(Node);
    struct Node* temp = new Node;
    if (!temp) {
        DMC_LOGE(TAG, "EnqueueRequest: index %d node MEM alloc fail", kpi_index);
        incDroppedEvents(group);
        return;
    }
    temp->kpiIndex = kpi_index;
    temp->kpiSize = kpi_size;
    temp->groupType = group;
    temp->kpiTimestamp = timestamp;

    temp->next = NULL;

    if (kpi_data) {
        allocSize += kpi_size;
        temp->kpiData = new uint8_t[kpi_size];
        if (!temp->kpiData) {
            DMC_LOGE(TAG, "EnqueueRequest: index %d data MEM alloc fail", kpi_index);
            delete temp;
            incDroppedEvents(group);
            return;
        }
        memcpy(temp->kpiData, kpi_data, kpi_size); //copy data
    }

    pthread_mutex_lock(&m_mutex);

    if(m_front == NULL && m_rear == NULL){
        m_front = m_rear = temp;
    } else {
        m_rear->next = temp;
        m_rear = temp;
    }

    incEventBufferInfo(group, allocSize);

    //DMC_LOGD(TAG, "EnqueueRequest: waking up thread");

    //signal waiting thread
    pthread_cond_signal(&m_cond);

    pthread_mutex_unlock(&m_mutex);
}


Node* ReqQManager::DequeueRequest() {
    if(m_front == NULL) {
        DMC_LOGE(TAG, "DequeueRequest: Queue is EMPTY");
        return NULL;
    }

    Node* temp = m_front;
    if(m_front == m_rear) {
        m_front = m_rear = NULL;
    } else {
        m_front = m_front->next;
    }

    decEventBufferInfo(temp->groupType, temp->kpiSize + sizeof(Node));

    return temp;
}


void ReqQManager::EmptyRequestQueue() {
    DMC_LOGD(TAG, "EmptyRequestQueue");

    if (m_front == NULL) {
        return;
    }

    struct Node* temp = NULL;

    while (m_front != NULL) {
        temp = m_front;
        m_front = m_front->next;

        KPI_GROUP_TYPE group = temp->groupType;
        if (group != KPI_GROUP_TYPE_UNDEFINED && m_pEventBufferInUse != NULL) {
            if (m_pEventBufferInUse[group] > (temp->kpiSize + sizeof(Node))) {
                m_pEventBufferInUse[group] -= (temp->kpiSize + sizeof(Node));
            } else {
                m_pEventBufferInUse[group] = 0;
            }
        }
        delete [] temp->kpiData;
        delete [] temp;
    }

    m_rear = NULL;
}

bool ReqQManager::IsQueueEmpty() {
    return m_front == NULL;
}

void *ReqQManager::ThreadEntry(void *arg) {
    ((ReqQManager *)arg)->ProcessQueue(arg);

    return NULL;
}


void ReqQManager::ProcessQueue(void *arg) {
    UNUSED(arg);

    // DMC_LOGD(TAG, "ProcessQueue");

    while (m_runThread) {
        //MLOGL2(LOG_DEBUG, TAG, "ThreadRun: in while");

        //acquire cond wait mutex
        pthread_mutex_lock(&m_mutex);

        //sleep thread if there is no more requests in queue
        while (IsQueueEmpty() && m_runThread) {
            // DMC_LOGD(TAG, "ProcessQueue: waiting");

            //wait
            pthread_cond_wait(&m_cond, &m_mutex);
        }

        if (!m_runThread) {
            //unlock cond wait mutex
            pthread_mutex_unlock(&m_mutex);
            break;
        }

        Node *request = DequeueRequest();//get queue node

        //unlock cond wait mutex
        pthread_mutex_unlock(&m_mutex);

        if (!m_runThread) {
            if (request) {
                delete [] request->kpiData;
                delete request;
            }
            break;
        }

        // DMC_LOGD(TAG, "ProcessQueue: running");

        if (request) {
            DMC_RESULT_CODE ret = DMC_RESULT_SUCCESS;
            record_t rec = {0};

            rec.kpiData = request->kpiData;
            rec.kpiSize = request->kpiSize;
            rec.kpiIndex = request->kpiIndex;
            rec.kpiTimestamp = request->kpiTimestamp;

            ret = m_handler(&rec, m_param);

            if (ret != DMC_RESULT_SUCCESS) {
                DMC_LOGE(TAG, "ProcessQueue: report error! KPI = %d, ret = %d",
                        request->kpiIndex, ret);
                KPI_GROUP_TYPE group = request->groupType;
                incDroppedEvents(group);
            }

            //free node contents
            delete [] request->kpiData;
            delete request;
        }
    }

    pthread_exit(NULL);
}

void ReqQManager::OnDeinit(void) {
    DMC_LOGD(TAG, "OnDeinit");

    if (pthread_equal(m_thread, pthread_self()) != 0) {
        DMC_LOGD(TAG, "thread is cancelled");
        return;
    }

    m_runThread = false;

    //acquire cond wait mutex
    pthread_mutex_lock(&m_mutex);

    EmptyRequestQueue();

    //signal waiting thread
    pthread_cond_signal(&m_cond);

    //unlock cond wait mutex
    pthread_mutex_unlock(&m_mutex);

    if (pthread_join(m_thread, NULL) != 0) {
        DMC_LOGE(TAG, "pthread_join failed. %s", strerror(errno));
    }

    m_thread = pthread_self();

    m_handler = NULL;
    m_param = NULL;
}

void ReqQManager::SetEventHandler(EventHandler handler, void *param) {
    m_handler = handler;
    m_param = param;
}

AllocInfo ReqQManager::GetAllocInfo(KPI_GROUP_TYPE groupType) {
    AllocInfo allocInfo = {
        .eventBufferInUse = 0,
        .eventBufferCapacity = 0,
        .droppedEvents = 0
    };

    pthread_mutex_lock(&m_mutex);

    allocInfo.eventBufferInUse = m_pEventBufferInUse[groupType];
    allocInfo.eventBufferCapacity = m_pEventBufferCapacity[groupType];
    allocInfo.droppedEvents = m_pDroppedEvents[groupType];

    pthread_mutex_unlock(&m_mutex);

    return allocInfo;
}

void ReqQManager::incDroppedEvents(KPI_GROUP_TYPE group) {
    if (group != KPI_GROUP_TYPE_UNDEFINED && m_pDroppedEvents != NULL) {
        if (m_pDroppedEvents[group] >= UINT32_MAX) {
            m_pDroppedEvents[group] = 0;
            DMC_LOGE(TAG, "m_pDroppedEvents reaches UINT32_MAX, reset!");
        }
        m_pDroppedEvents[group]++;
    }
}

void ReqQManager::incEventBufferInfo(KPI_GROUP_TYPE group, uint32_t allocSize) {

    if (group == KPI_GROUP_TYPE_UNDEFINED) {
        return;
    }
    if (m_pEventBufferInUse == NULL) {
        return;
    }
    if ((UINT32_MAX - allocSize) <= m_pEventBufferInUse[group]) {
        m_pEventBufferInUse[group] = 0;
        DMC_LOGE(TAG, "m_pEventBufferInUse reaches UINT32_MAX, reset!");
    }
    m_pEventBufferInUse[group] += allocSize;

    if (m_pEventBufferCapacity == NULL) {
        return;
    }
    if (m_pEventBufferInUse[group] > m_pEventBufferCapacity[group]) {
        m_pEventBufferCapacity[group] = m_pEventBufferInUse[group];
    }
}

void ReqQManager::decEventBufferInfo(KPI_GROUP_TYPE group, uint32_t freeSize) {
    if (group == KPI_GROUP_TYPE_UNDEFINED) {
        return;
    }
    if (m_pEventBufferInUse == NULL) {
        return;
    }
    if (m_pEventBufferInUse[group] > freeSize) {
        m_pEventBufferInUse[group] -= freeSize;
    } else {
        m_pEventBufferInUse[group] = 0;
    }
}

