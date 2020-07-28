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

#include <utils/Thread.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include "LogUtils.h"
#include "BufferUtils.h"
#include "ProcessingThread.h"

#define LOG_TAG "FB/ProcessingThread"

ProcessingThread::ProcessingThread():Thread(false) {
    MY_LOGD("ProcessingThread!!!!");
}

void ProcessingThread::sendRequest(sp<Request> request, sp<RequestCallback> callback) {
    FUNCTION_IN;
    mlock.lock();
    if (working_request != nullptr && working_request->status == PROCESSING) {
        mlock.unlock();
        return;
    }
    working_request = request;
    request_callback = callback;
    working_request->init();
    working_request->status = INIT;
    MY_LOGD("sendRequest working_request status = %d", working_request->status);
    mCondition.broadcast();
    mlock.unlock();
    FUNCTION_OUT;
}

bool ProcessingThread::threadLoop() {
    FUNCTION_IN;
    mlock.lock();
    MY_LOGD("threadLoop >>>>>>>>>>>>>>>>");

    if (working_request == nullptr) {
        MY_LOGW("threadLoop waiting !!!");
        mCondition.wait(mlock);
        mlock.unlock();
        return true;
    }
    MY_LOGD("threadLoop current status = %d", working_request->status);
    if (working_request->status != INIT) {
        MY_LOGW("threadLoop waiting !!!");
        mCondition.wait(mlock);
        mlock.unlock();
        return true;
    }
    if (working_request->status ==  INIT) {
        MY_LOGD("threadLoop processing >>>");
        working_request->status = PROCESSING;
    }
    mlock.unlock();

    bool completed = working_request->processing();
    MY_LOGD("threadLoop successed = %d", completed);
    //////////////////////////////////////////////////////

    if (request_callback != nullptr) {
        MY_LOGD("threadLoop  = %d", working_request->status);
        request_callback->onCompleted(working_request);
    }
    if (completed) {
            working_request->status = DONE;
    } else {
            working_request->status = FAILED;
    }
    MY_LOGD("threadLoop working_request->status = %d", working_request->status);
    FUNCTION_OUT;
    return true;
}

ProcessingThread::~ProcessingThread() {
    MY_LOGD("~~~ProcessingThread!!!!");
};

