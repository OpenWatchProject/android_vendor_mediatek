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

#include "gtest/gtest.h"
#include <utils/RefBase.h>
#include <string>
#include <iostream>

#include "../P2_DispatchProcessor.h"
#include "TestP2_Common.h"

#include "DebugControl.h"
#define PIPE_TRACE TRACE_DISPATCH_PROCESSOR
#define PIPE_CLASS_TAG "Suite_DispatchProcessor"
#include <featurePipe/core/include/PipeLog.h>

using android::sp;
using android::RefBase;

using namespace NSCam;
using namespace NSCam::v3;

namespace P2
{
//********************************************
// Test Utils
//********************************************

//********************************************
// Imitating P2_Node::parseInitParam
//********************************************
void parseInitParam(ILog& log, Config& conf, sp<P2InfoObj>& p2Info){
    p2Info->mConfigInfo.mMainSensorID = conf.main1Idx;
    p2Info->mConfigInfo.mLog = log;
    p2Info->mLog = log;

    p2Info->addSensorInfo(log, conf.main1Idx);

    if( conf.isDualCam )
    {
        ILog sensorLog = makeSensorLogger(log, conf.main2Idx);
        p2Info->addSensorInfo(sensorLog, conf.main2Idx);
    }
}

//********************************************
// Imitating P2_Node::updateConfigInfo
//********************************************
void updateConfigInfo(sp<P2InfoObj>& p2Info){
    // TODO: get a real world setting
    p2Info->mConfigInfo.mBurstNum = 0;
    p2Info->mConfigInfo.mCustomOption = 0;
    p2Info->mConfigInfo.mSupportClearZoom = false;
    p2Info->mConfigInfo.mSupportDRE = false;
}

//********************************************
// Test Cases
//********************************************
TEST(DispatchProcessor, Basic)
{
    MY_UTLOG("+");
    Config config;

    // - construct
    MY_UTLOG("[construct]");
    MUINT32 logLevel = property_get_int32(KEY_P2_LOG, VAL_P2_LOG);
    ILog log = NSCam::Feature::makeLogger("P2_Node_test", logLevel);

    sp<P2InfoObj> p2Info = new P2InfoObj(log);
    // TODO: Removing P2FetureNode has to modify the test code
    const P2StreamingNode::ePass2Type pass2Type = P2StreamingNode::ePass2Type::PASS2_STREAM;
    P2StreamingNode::UsageHint usageHint = makeUsageHint(config);
    p2Info->mConfigInfo.mP2Type = toP2Type(pass2Type, usageHint);
    p2Info->mConfigInfo.mUsageHint = toP2UsageHint(usageHint);
    p2Info->mConfigInfo.mLogLevel = logLevel;

    DispatchProcessor mDispatcher;

    // - init
    {
        MY_UTLOG("[init]");
        ILog sensorLog = NSCam::Feature::makeSensorLogger(log, config.main1Idx);
        MBOOL needThread = MFALSE;
        EXPECT_TRUE( mDispatcher.setNeedThread(needThread) == needThread ) << "setNeedThread failed" << std::endl;

        parseInitParam(sensorLog, config, p2Info);
        EXPECT_TRUE( mDispatcher.init(P2InitParam(P2Info(p2Info, sensorLog, p2Info->mConfigInfo.mMainSensorID))) ) << "init failed" << std::endl;
    }

    // - config
    {
        MY_UTLOG("[config]");
        updateConfigInfo(p2Info);
        EXPECT_TRUE( mDispatcher.config(P2ConfigParam(P2Info(p2Info, p2Info->mLog, p2Info->mConfigInfo.mMainSensorID))) ) << "config failed" << std::endl;
    }

    // - queue
    sp<P2FrameRequestCreater> pP2FrameRequestCreater = new P2FrameRequestCreater();
    {
        set<MWRequestPath> vReqPaths;
        if( g_argc < 2 )
        {
            vReqPaths.insert(MWRequestPath::eGeneral);
        }
        else
        {
            for(int i=1 ; i<g_argc ; ++i)
            {
                string arg(g_argv[i]);
                if(arg == "-g")
                {
                    vReqPaths.insert(MWRequestPath::eGeneral);
                }
                else if(arg == "-p1")
                {
                    vReqPaths.insert(MWRequestPath::ePhysic_1);
                }
                else if(arg == "-p2")
                {
                    vReqPaths.insert(MWRequestPath::ePhysic_2);
                }
                else if(arg == "-l1")
                {
                    vReqPaths.insert(MWRequestPath::eLarge_1);
                }
                else if(arg == "-l2")
                {
                    vReqPaths.insert(MWRequestPath::eLarge_2);
                }
                else
                {
                    MY_UTLOG("unknown path(%s)", arg.c_str());
                }
            }
        }

        for(const auto& path : vReqPaths)
        {
            MY_UTLOG("enque path(%d)(%s)", path, pathToChar(path));
        }
        sp<P2FrameRequest> pRequest = pP2FrameRequestCreater->createRequest(log, p2Info, vReqPaths);
        mDispatcher.enque(pRequest);
    }

    // - wait for callback
    {
        MY_UTLOG("[wait]");
        std::chrono::microseconds maxWaitTimeUS(std::chrono::milliseconds(500));
        usleep(maxWaitTimeUS.count());
    }

    // - flush
    {
        MY_UTLOG("[flush]");
        mDispatcher.flush();
    }

    // - uninit
    {
        MY_UTLOG("[uninit]");
        mDispatcher.uninit();
    }

    MY_UTLOG("-");
}

} // namespace P2
