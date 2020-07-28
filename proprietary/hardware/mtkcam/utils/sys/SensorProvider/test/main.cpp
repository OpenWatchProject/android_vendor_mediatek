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
#define LOG_TAG "MtkCam/TestSensorProvider"
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <utils/threads.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <def/common.h>
#include <utils/std/Log.h>
#include <gtest/gtest.h>

using namespace android;

#include <utils/sys/SensorProvider.h>
#include "TestSensorProvider.h"

using namespace NSCam;
using namespace NSCam::Utils;
using std::vector;

#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

TEST(SensorProvider, getLatestSensorData)
{
    int frameNum = 0;
    int sensorInterval = 20, testTime = 5, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;
    sp<SensorProvider> mpProvider = SensorProvider::createInstance("getLatestSensorData");
    ASSERT_TRUE(mpProvider != NULL);

    MBOOL ret = mpProvider->enableSensor(SENSOR_TYPE_GYRO, sensorInterval);
    EXPECT_EQ(ret, true);
    ret = mpProvider->enableSensor(SENSOR_TYPE_ACCELERATION, sensorInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * testTime))
    {
        usleep(frameInterval);

        SensorData data;
        ret = mpProvider->getLatestSensorData(SENSOR_TYPE_GYRO, data);
        EXPECT_EQ(ret, true);
        if (data.timestamp == previousT && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_TRUE(data.timestamp > previousT);
        //MY_LOGD("sensorprovider test getLatestSensorData data.timestamp=%lld, preT=%lld", data.timestamp, previousT);

        ret = mpProvider->getLatestSensorData(SENSOR_TYPE_ACCELERATION, data);
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(data.timestamp > previousT);

        previousT = data.timestamp;
        ++frameNum;
        retry = 0;
    }
}

TEST(SensorProvider, getAllSensorData)
{
    usleep(100000); //100ms between cases

    int frameNum = 0, gyroDataNum = 0, accelDataNum = 0;
    int gyroInterval = 5, accelInterval = 10, testTime = 5, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;
    sp<SensorProvider> mpProvider = SensorProvider::createInstance("getAllSensorData");
    ASSERT_TRUE(mpProvider != NULL);

    MBOOL ret = mpProvider->enableSensor(SENSOR_TYPE_GYRO, gyroInterval);
    EXPECT_EQ(ret, true);
    ret = mpProvider->enableSensor(SENSOR_TYPE_ACCELERATION, accelInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * testTime))
    {
        usleep(frameInterval);

        vector<SensorData> data;
        ret = mpProvider->getAllSensorData(SENSOR_TYPE_GYRO, data);
        if (ret == false && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(data.size() > 0);
        //MY_LOGD("sensorprovider test getAllSensorData GYRO!! data.size=%d", data.size());
        EXPECT_TRUE(data[0].timestamp > previousT);
        gyroDataNum += data.size();

        ret = mpProvider->getAllSensorData(SENSOR_TYPE_ACCELERATION, data);
        //MY_LOGD("sensorprovider test getAllSensorData ACCEL!! data.size=%d", data.size());
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(data.size() > 0);
        EXPECT_TRUE(data[0].timestamp > previousT);
        accelDataNum += data.size();

        previousT = data[0].timestamp;
        ++frameNum;
        retry = 0;
    }
    int idealDataNum = ((float)1000 / gyroInterval) * testTime;
    int lowerBound = (float)idealDataNum * 0.95;
    int upperBound = (float)idealDataNum * 1.05;
    EXPECT_TRUE((gyroDataNum > lowerBound && gyroDataNum < upperBound));

    idealDataNum = ((float)1000 / accelInterval) * testTime;
    lowerBound = (float)idealDataNum * 0.95;
    upperBound = (float)idealDataNum * 1.05;
    EXPECT_TRUE((accelDataNum > lowerBound && accelDataNum < upperBound));
}

TEST(SensorProvider, multiUsers)
{
    usleep(100000); //100ms between cases

    sp<TestSensorProvider> userA = new TestSensorProvider();
    userA->test(33, 5, TestSensorProvider::USAGE_GET_LATEST);

    sp<TestSensorProvider> userB = new TestSensorProvider();
    userB->test(5, 10, TestSensorProvider::USAGE_GET_ALL);

    sp<TestSensorProvider> userC = new TestSensorProvider();
    userC->test(20, 10, TestSensorProvider::USAGE_GET_LATEST);

    userA->waitExit();
    userB->waitExit();
    userC->waitExit();
}

TEST(SensorProvider, noDataAvailable)
{
    usleep(100000); //100ms between cases

    int frameNum = 0, dataNum = 0;
    int sensorInterval = 33, testTime = 5, fps = 30;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;
    sp<SensorProvider> mpProvider = SensorProvider::createInstance("noDataAvailable");
    ASSERT_TRUE(mpProvider != NULL);

    //---------------Get data before enable sensor---------------
    SensorData data;
    MBOOL ret = mpProvider->getLatestSensorData(SENSOR_TYPE_GYRO, data);
    EXPECT_EQ(ret, false);
    vector<SensorData> dataVec;
    ret = mpProvider->getAllSensorData(SENSOR_TYPE_GYRO, dataVec);
    EXPECT_EQ(ret, false);
    EXPECT_TRUE(dataVec.size() == 0);

    ret = mpProvider->enableSensor(SENSOR_TYPE_GYRO, sensorInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    //---------------Get data after enable sensor---------------
    ret = mpProvider->getAllSensorData(SENSOR_TYPE_GYRO, dataVec);
    EXPECT_EQ(ret, true);
    EXPECT_TRUE(dataVec.size() != 0);
    ret = mpProvider->getAllSensorData(SENSOR_TYPE_ACCELERATION, dataVec);
    EXPECT_EQ(ret, false);
    EXPECT_TRUE(dataVec.size() == 0);

    mpProvider->disableSensor(SENSOR_TYPE_GYRO);
    usleep(50000); //50ms

    //---------------Get data after disable sensor---------------
    ret = mpProvider->getLatestSensorData(SENSOR_TYPE_GYRO, data);
    EXPECT_EQ(ret, false);
    ret = mpProvider->getAllSensorData(SENSOR_TYPE_GYRO, dataVec);
    EXPECT_EQ(ret, false);
    EXPECT_TRUE(dataVec.size() == 0);
}

TEST(SensorProvider, userIndependent)
{
    usleep(100000); //100ms between cases

    int frameNum = 0, gyroDataNum = 0, userLeaved = 0;
    int sensorInterval = 5, testTime = 5, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;
    sp<SensorProvider> mpEarlyLeaver = SensorProvider::createInstance("EarlyLeaver");
    ASSERT_TRUE(mpEarlyLeaver != NULL);
    sp<SensorProvider> mpStayer = SensorProvider::createInstance("Stayer");
    ASSERT_TRUE(mpStayer != NULL);

    MBOOL ret = mpEarlyLeaver->enableSensor(SENSOR_TYPE_GYRO, sensorInterval);
    EXPECT_EQ(ret, true);
    ret = mpStayer->enableSensor(SENSOR_TYPE_GYRO, sensorInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * testTime))
    {
        usleep(frameInterval);

        if (frameNum == (fps * (testTime - 2)))
        {
            // EarlyLeaver leaves
            ret = mpEarlyLeaver->disableSensor(SENSOR_TYPE_GYRO);
            EXPECT_EQ(ret, true);
            userLeaved = 1;
        }

        if (!userLeaved)
        {
            SensorData data;
            ret = mpEarlyLeaver->getLatestSensorData(SENSOR_TYPE_GYRO, data);
            EXPECT_EQ(ret, true);
        }

        vector<SensorData> dataVec;
        ret = mpStayer->getAllSensorData(SENSOR_TYPE_GYRO, dataVec);
        if (ret == false && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(dataVec.size() > 0);
        EXPECT_TRUE(dataVec[0].timestamp > previousT);

        previousT = dataVec[0].timestamp;
        gyroDataNum += dataVec.size();

        ++frameNum;
        retry = 0;
    }

    int idealDataNum = ((float)1000 / sensorInterval) * testTime;
    int lowerBound = (float)idealDataNum * 0.95;
    int upperBound = (float)idealDataNum * 1.05;
    EXPECT_TRUE((gyroDataNum > lowerBound && gyroDataNum < upperBound));
}

TEST(SensorProvider, monkeyTest)
{
    usleep(100000); //100ms between cases
    int userCount = 10;

    vector<sp<TestSensorProvider> > providers(userCount);

    for (int i = 0; i < userCount; i++)
    {
        providers[i] = new TestSensorProvider();
        providers[i]->test(33, 5, TestSensorProvider::USAGE_MONKEY);
        providers[i]->test(33, 5, TestSensorProvider::USAGE_MONKEY);
        providers[i]->test(33, 5, TestSensorProvider::USAGE_MONKEY);
    }

    for (int i = 0; i < userCount; i++)
    {
        providers[i]->waitExit();
    }
}

TEST(SensorProvider, longTimeTest)
{
    usleep(100000); //100ms between cases

    int frameNum = 0, gyroDataNum = 0;
    int gyroInterval = 5, testTimeInMinute = 30, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;
    sp<SensorProvider> mpProvider = SensorProvider::createInstance("longTimeTest");
    ASSERT_TRUE(mpProvider != NULL);

    MBOOL ret = mpProvider->enableSensor(SENSOR_TYPE_GYRO, gyroInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * testTimeInMinute * 60))
    {
        usleep(frameInterval);

        vector<SensorData> data;
        ret = mpProvider->getAllSensorData(SENSOR_TYPE_GYRO, data);
        if (ret == false && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(data.size() > 0);
        //MY_LOGD("sensorprovider test getAllSensorData GYRO!! data.size=%d", data.size());
        EXPECT_TRUE(data[0].timestamp > previousT);
        gyroDataNum += data.size();

        previousT = data[0].timestamp;
        ++frameNum;
        retry = 0;
    }
    int idealDataNum = ((float)1000 / gyroInterval) * testTimeInMinute * 60;
    int lowerBound = (float)idealDataNum * 0.95;
    int upperBound = (float)idealDataNum * 1.05;
    EXPECT_TRUE((gyroDataNum > lowerBound && gyroDataNum < upperBound));
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();

    return 0;
}

