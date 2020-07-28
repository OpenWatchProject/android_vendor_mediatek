/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2019. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
#ifndef TASKTHREAD_H
#define TASKTHREAD_H

#include <android/looper.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include "LogUtils.h"

#define LOG_TAG "FeatureUtils/TaskThread"

class Task {
public:
    virtual ~Task() {}
    virtual void run()=0;
};

template <typename T>
class TaskThread {
public:
    TaskThread() {
        pthread_create(&mThread, nullptr, pthread_main, this);
    }

    ~TaskThread() {
    }

    void postTaskAndWaitDone(const T& task) {
        android::AutoMutex lock(mTasklock);
        mTaskCompleted = false;
        int err = write(mWritePipe, &task, sizeof(task));
        if (err < 0) {
            MY_LOGE("Failed to write to pipe");
        }
        while (!mTaskCompleted) {
            mTaskCondition.wait(mTasklock);
        }
    }

    void exit() {
        mNeedExit = true;
    }

    void join() {
        MY_LOGD("join ++");
        pthread_join(mThread, NULL);
        MY_LOGD("join --");
    }
private:
    ALooper* mLooper;
    pthread_t mThread;
    int mReadPipe;
    int mWritePipe;

    mutable ::android::Mutex mTasklock;
    mutable ::android::Condition mTaskCondition;
    volatile bool mTaskCompleted = false;
    volatile bool mNeedExit = false;

    void run() {
        FUNCTION_IN;

        int msgpipe[2];
        if (pipe(msgpipe) < 0){
            MY_LOGE("Failed to create pipe");
            return;
        }
        mReadPipe = msgpipe[0];
        mWritePipe = msgpipe[1];

        mLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        ALooper_addFd(mLooper, mReadPipe, 1, ALOOPER_EVENT_INPUT, handleTask, this);

        while (true) {
            ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
            if (mNeedExit) {
                MY_LOGD("pthread_exit");
                FUNCTION_OUT;
                ALooper_removeFd(mLooper, mReadPipe);
                ::close(mReadPipe);
                ::close(mWritePipe);
                pthread_exit(0);
            }
        }
    }

    static void* pthread_main(void* arg) {
        FUNCTION_IN;
        auto handlerThread = (TaskThread*)arg;
        handlerThread->run();
        FUNCTION_OUT;
        return nullptr;
    }

    static int handleTask(int fd, int events, void *data) {
        FUNCTION_IN;
        auto handlerThread = (TaskThread<T>*)data;
        android::AutoMutex lock(handlerThread->mTasklock);

        T task = T();
        int err = read(handlerThread->mReadPipe, &task, sizeof(T));
        if (err < 0) {
            MY_LOGE("Failed to read from pipe");
        } else {
            task.run();
        }

        handlerThread->mTaskCompleted = true;
        handlerThread->mTaskCondition.broadcast();

        FUNCTION_OUT;
        return 1;
    }
};

#endif