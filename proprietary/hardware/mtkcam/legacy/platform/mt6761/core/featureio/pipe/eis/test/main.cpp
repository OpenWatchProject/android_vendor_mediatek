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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

#define LOG_TAG "testGyro"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <utils/threads.h>
#include <semaphore.h>
#include <sys/prctl.h>

#include <cutils/log.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>

#include <ALooper.h>
#include <ASensorEventQueue.h>
#include <ASensorManager.h>

using namespace android;
using std::string;

#include <Log.h>
#include <common.h>
#include <v1/camutils/CamMisc.h>

//#include <utils/GyroListener.h>

#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
using android::frameworks::sensorservice::V1_0::ISensorManager;
using android::frameworks::sensorservice::V1_0::Result;
using android::hardware::sensors::V1_0::SensorType;
using android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using android::hardware::Return;
using android::hardware::sensors::V1_0::Event;
using android::frameworks::sensorservice::V1_0::IEventQueue;

pthread_t     g_TestThread;
sem_t         g_semTestThreadStart;
sem_t         g_semTestThreadEnd;
Condition     GyroTestCondMain;
Mutex         GyroTestLockMain;

float              gGyroTestVec[3] = {0};
unsigned long long gGyroTestPrevTime = 0;
unsigned long long gGyroTestTimeStamp = 0;
int                gPeriod = 5;
int                gFrameNum = 0;
int                gGyroEventNum = 0;

#if 0
long long getTimeStamp()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp =((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000;
    return (timestamp); // from nano to ms
}
#endif

class GyroTestCallback : public IEventQueueCallback {
  public:
    Return<void> onEvent(const Event &e) {
        sensors_event_t sensorEvent;
        android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);

        gGyroTestTimeStamp = sensorEvent.timestamp;
        gGyroTestVec[0] = sensorEvent.gyro.x;
        gGyroTestVec[1] = sensorEvent.gyro.y;
        gGyroTestVec[2] = sensorEvent.gyro.z;

        gGyroEventNum++;

        return android::hardware::Void();
    }
};

void* TestThreadFunc(void *arg)
{
    ::prctl(PR_SET_NAME,"TestThread", 0, 0, 0);
    //MY_LOGD("thread TestThreadFunc +");

    // sensor HIDL interface
    sp<ISensorManager> manager = ISensorManager::getService();
    int32_t sensorHandle;
    manager->getDefaultSensor(SensorType::GYROSCOPE,
        [&](const auto &sensor, Result result) {
            if (result != Result::OK) {
                MY_LOGE("getDefaultSensor FAIL!");
                exit(0);
            }
            sensorHandle = sensor.sensorHandle;
        });
    sp<IEventQueue> queue;
    Result res;
    manager->createEventQueue(new GyroTestCallback(),
        [&] (const auto &q, Result result) {
            if (result != Result::OK) {
                MY_LOGE("createEventQueue FAIL!");
                exit(0);
            }
            queue = q;
        });
    if (queue->enableSensor(sensorHandle, gPeriod * 1000 /* sample period */, 0 /* latency */) != Result::OK) {
            MY_LOGE("enableSensor FAIL!");
            exit(0);
    }

    while (1)
    {
        usleep(33333); //33ms
        if (!(gGyroTestTimeStamp > gGyroTestPrevTime))
        {
            MY_LOGE("timestamp check FAILED!");
            exit(0);
        }
        //MY_LOGD("TestThreadFunc loop %llu:(%f, %f, %f)", gGyroTestTimeStamp,
        //    gGyroTestVec[0], gGyroTestVec[1], gGyroTestVec[2]);

        gGyroTestPrevTime = gGyroTestTimeStamp;

        gFrameNum++;
        if (gFrameNum % 150 == 0) //5 sec
        {
            GyroTestCondMain.signal();
        }
    }

    return NULL;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    MY_LOGD("GyroTest start");
    gGyroEventNum = 0;

    sem_init(&g_semTestThreadStart, 0, 0);
    sem_init(&g_semTestThreadEnd, 0, 0);
    pthread_create(&g_TestThread, NULL, TestThreadFunc, (void*)0);

    GyroTestCondMain.wait(GyroTestLockMain);

    MY_LOGD("GyroTest passed, %d Gyro events in 5 sec (period = %d ms)", gGyroEventNum, gPeriod);

    return 0;
}

