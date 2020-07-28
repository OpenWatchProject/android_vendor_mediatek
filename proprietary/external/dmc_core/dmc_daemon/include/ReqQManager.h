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
 * @file ReqQManager.h
 *
 * @author mtk33273 (Ankur Saxena)
 *
 * @brief The header file defines @c ReqQManager class for managing ReqThread
 * and enqueue/dequeue mechanism.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MDMI_REQ_Q_MANAGER_H__
#define __MDMI_REQ_Q_MANAGER_H__

#include <pthread.h>
#include "DmcDefs.h"
#include "KpiObj.h"

struct Node {
    int             kpiIndex;
    size_t          kpiSize;
    KPI_GROUP_TYPE  groupType;
    uint64_t        kpiTimestamp;
    uint8_t         *kpiData;
    Node            *next;
};

struct record_t {
    int         kpiIndex;
    size_t      kpiSize;
    uint64_t    kpiTimestamp;
    uint8_t     *kpiData;
};

typedef struct {
    size_t eventBufferInUse;
    size_t eventBufferCapacity;
    size_t droppedEvents;
} AllocInfo;

typedef DMC_RESULT_CODE (*EventHandler)(record_t *pRec, void *param);

class ReqQManager
{
public:
    ReqQManager();
    ~ReqQManager();
    void EnqueueRequest(int kpi_index, KPI_GROUP_TYPE group, void *kpi_data, int kpi_size, uint64_t timestamp);
    Node* DequeueRequest();
    void EmptyRequestQueue();
    bool IsQueueEmpty();
    void OnDeinit(void);
    void SetEventHandler(EventHandler handler, void *param);
    void initEventInfoBuffer(uint8_t groupSize);
    void freeEventInfoBuffer();
    AllocInfo GetAllocInfo(KPI_GROUP_TYPE groupType);

private:
    pthread_t m_thread;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    Node *volatile m_front, *volatile m_rear;
    bool m_runThread;
    EventHandler m_handler;
    void *m_param;
    uint8_t m_groupSize;
    uint32_t *m_pEventBufferInUse;
    uint32_t *m_pEventBufferCapacity;
    uint32_t *m_pDroppedEvents;

    static void *ThreadEntry(void *arg);
    void ProcessQueue(void *arg);
    void incDroppedEvents(KPI_GROUP_TYPE group);
    void incEventBufferInfo(KPI_GROUP_TYPE group, uint32_t allocSize);
    void decEventBufferInfo(KPI_GROUP_TYPE group, uint32_t freeSize);
};

#endif
