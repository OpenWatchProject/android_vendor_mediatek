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

#include "SubscriptionBatchMdmRequest.h"
#include "dmc_utils.h"
#include <inttypes.h>

const char *TAG = "SubBatchMdmRequest";

void SubscriptionBatchMdmRequest::add(KPI_SOURCE_TYPE source, const SubscriptionRequest &request) {
    switch (source) {
        case KPI_SOURCE_TYPE_MDM_EM_MSG:
            mEmRequestList.push_back(request);
            break;
        case KPI_SOURCE_TYPE_MDM_EM_OTA:
            mOtaRequestList.push_back(request);
            break;
        case KPI_SOURCE_TYPE_MDM_ICD_EVENT:
            mIcdEventRequestList.push_back(request);
            break;
        case KPI_SOURCE_TYPE_MDM_ICD_RECORD:
            mIcdRecordRequestList.push_back(request);
            break;
        default:
            // Do nothing
            break;
    }
}

bool SubscriptionBatchMdmRequest::getMsgIds(KPI_SOURCE_TYPE source, KPI_MSGID **ppMsgIds) {
    std::vector<SubscriptionRequest> requestList;
    if (getRequestList(source, requestList)) {
        unsigned int size = requestList.size();
        if (size > 0) {
            *ppMsgIds = (KPI_MSGID *)calloc(size, sizeof(KPI_MSGID));
            if (*ppMsgIds == NULL) {
                DMC_LOGE(TAG, "getMsgIds() calloc failed!");
                return false;
            }

            for (unsigned int i = 0; i < size; i++) {
                KPI_MSGID msgId = requestList[i].mMsgId;
                (*ppMsgIds)[i] = msgId;
            }
            return true;
        }
    }
    return false;
}

bool SubscriptionBatchMdmRequest::getRequestList(
    KPI_SOURCE_TYPE source, std::vector<SubscriptionRequest> &requestList) {
    switch (source) {
        case KPI_SOURCE_TYPE_MDM_EM_MSG:
            requestList.assign(mEmRequestList.begin(), mEmRequestList.end());
            break;
        case KPI_SOURCE_TYPE_MDM_EM_OTA:
            requestList.assign(mOtaRequestList.begin(), mOtaRequestList.end());
            break;
        case KPI_SOURCE_TYPE_MDM_ICD_RECORD:
            requestList.assign(mIcdRecordRequestList.begin(), mIcdRecordRequestList.end());
            break;
        case KPI_SOURCE_TYPE_MDM_ICD_EVENT:
            requestList.assign(mIcdEventRequestList.begin(), mIcdEventRequestList.end());
            break;
        default:
            return false;
    }
    return true;
}

uint32_t SubscriptionBatchMdmRequest::size() const {
    uint32_t size =
            mEmRequestList.size() +
            mOtaRequestList.size() +
            mIcdEventRequestList.size() +
            mIcdRecordRequestList.size();
    return size;
}

void SubscriptionBatchMdmRequest::clear() {
    mEmRequestList.clear();
    mOtaRequestList.clear();
    mIcdEventRequestList.clear();
    mIcdRecordRequestList.clear();
}

void SubscriptionBatchMdmRequest::dump() {
    DMC_LOGD(TAG, "Dump SubBatchMdmRequest:");
    if (size() == 0) {
        DMC_LOGD(TAG, "No entry");
    } else {
        DMC_LOGD(TAG, "EM(%d):", mEmRequestList.size());
        for (unsigned int i = 0; i < mEmRequestList.size(); i++) {
                DMC_LOGD(TAG, "[%d|%" PRIu64"]", mEmRequestList[i].mObjType, mEmRequestList[i].mMsgId);
        }
        DMC_LOGD(TAG, "OTA(%d):", mOtaRequestList.size());
        for (unsigned int i = 0; i < mOtaRequestList.size(); i++) {
                DMC_LOGD(TAG, "[%d|%" PRIu64"]", mOtaRequestList[i].mObjType, mOtaRequestList[i].mMsgId);
        }
        DMC_LOGD(TAG, "ICD EVENT(%d):", mIcdEventRequestList.size());
        for (unsigned int i = 0; i < mIcdEventRequestList.size(); i++) {
                DMC_LOGD(TAG, "[%d|%" PRIu64"]", mIcdEventRequestList[i].mObjType, mIcdEventRequestList[i].mMsgId);
        }
        DMC_LOGD(TAG, "ICD RECORD(%d):", mIcdRecordRequestList.size());
        for (unsigned int i = 0; i < mIcdRecordRequestList.size(); i++) {
                DMC_LOGD(TAG, "[%d|%" PRIu64"]", mIcdRecordRequestList[i].mObjType, mIcdRecordRequestList[i].mMsgId);
        }
    }
}