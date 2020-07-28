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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include "TestP2_Common.h"
#include "TestP2_Mocks.h"

using namespace P2;

//********************************************
// Testing Environment
//********************************************
IHalSensor* P2Environment::HalSensorInit()
{
    Config config;
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    EXPECT_TRUE(pHalSensorList != nullptr) << "failed to get the HAL sensor list.";

    pHalSensorList->searchSensors();
    int sensorCount = pHalSensorList->queryNumberOfSensors();

    int main1Idx = config.main1Idx;
    int main2Idx = config.main2Idx;
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];
    IHalSensor *mpHalSensor = pHalSensorList->createSensor( PIPE_CLASS_TAG, 2, pIndex);
    EXPECT_TRUE(mpHalSensor != nullptr) << "failed to create HAL sensor.";

    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    MY_UTLOG("power on main1 sensor.");
    EXPECT_TRUE(mpHalSensor->powerOn(PIPE_CLASS_TAG, 1, &main1Index)) << "failed to open sensor power: " << main1Index;

    if( config.isDualCam )
    {
        MY_UTLOG("power on main2 sensor.");
        EXPECT_TRUE(mpHalSensor->powerOn(PIPE_CLASS_TAG, 1, &main2Index)) << "failed to open sensor power: " << main2Index;
    }
    return mpHalSensor;
}

void P2Environment::SetUp()
{
    //MY_UTSCOPEDTRACE();
    //Config config;
    //config.dump();
    //mpHalSensor = HalSensorInit();
}

void P2Environment::TearDown()
{
    //MY_UTSCOPEDTRACE();
    //HalSensorUninit();
}

void P2Environment::HalSensorUninit()
{
    Config globalConfig;
    int main1Idx = globalConfig.main1Idx;
    int main2Idx = globalConfig.main2Idx;

    MY_UTLOG("power off main1 sensor.");
    MUINT pIndex_main1[1] = { (MUINT)main1Idx};
    mpHalSensor->powerOff(PIPE_CLASS_TAG, 1, pIndex_main1);

    if( globalConfig.isDualCam )
    {
        MY_UTLOG("power off main2 sensor.");
        MUINT pIndex_main2[1] = { (MUINT)main2Idx};
        mpHalSensor->powerOff(PIPE_CLASS_TAG, 1, pIndex_main2);
    }
    mpHalSensor->destroyInstance(PIPE_CLASS_TAG);
}

//********************************************
// P2FrameRequestCreater
//********************************************
sp<P2FrameRequest> P2FrameRequestCreater::createRequest(const ILog &log, const sp<P2InfoObj> pP2Info, const set<MWRequestPath>& vRequestPaths)
{
    ILog frameLog = NSCam::Feature::makeFrameLogger(log, mFrameCount, mFrameCount, 0);
    sp<P2DataObj> p2Data = new P2DataObj(frameLog);
    p2Data->mFrameData.mP2FrameNo = frameLog.getLogFrameID();
    P2Pack p2Pack(frameLog, pP2Info, p2Data);

    Config globalConfig;
    std::vector<MUINT32> subIds;
    for(MUINT32 id : pP2Info->mConfigInfo.mAllSensorID)
    {
        if(id != globalConfig.main1Idx)
            subIds.push_back(id);
    }
    sp<P2InIDMap> inIdMap = new P2InIDMap( globalConfig.main1Idx, subIds);
    sp<P2FrameRequest> request = new P2::MWFrameRequestMock(frameLog, p2Pack, p2Data, inIdMap, vRequestPaths);

    ++mFrameCount;
    return request;
}

P2FrameRequestCreater::~P2FrameRequestCreater()
{
    //
}