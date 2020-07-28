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

#include <gtest/gtest.h>

#include "DebugControl.h"
#define PIPE_TRACE TRACE_EIS_QCONTROL
#define PIPE_CLASS_TAG "Suite_EIS_QCONTROL_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <featurePipe/streaming/EISQControl.h>
#include <mtkcam3/feature/eis/eis_ext.h>
#include <camera_custom_eis.h>

#include <cstdlib>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define EISMODE_30          (1<<EIS_MODE_EIS_30)
#define EISMODE_QUEUE       (1<<EIS_MODE_EIS_QUEUE)

#define USE_EISQ  MTRUE
#define NO_EISQ   MFALSE
#define USE_TSQ   MTRUE
#define NO_TSQ    MFALSE
#define EISQ_READY     MTRUE
#define EISQ_NOT_READY MFALSE

#define APP_PHOTO_PREVIEW IStreamingFeaturePipe::APP_PHOTO_PREVIEW
#define APP_VIDEO_PREVIEW IStreamingFeaturePipe::APP_VIDEO_PREVIEW
#define APP_VIDEO_RECORD IStreamingFeaturePipe::APP_VIDEO_RECORD
#define APP_VIDEO_STOP IStreamingFeaturePipe::APP_VIDEO_STOP
#define I1 IStreamingFeaturePipe::APP_VIDEO_PREVIEW
#define I2 IStreamingFeaturePipe::APP_VIDEO_RECORD
#define I3 IStreamingFeaturePipe::APP_VIDEO_STOP

#define O0 EISQ_ACTION_NO
#define O1 EISQ_ACTION_READY
#define O2 EISQ_ACTION_INIT
#define O3 EISQ_ACTION_PUSH
#define O4 EISQ_ACTION_RUN
#define O5 EISQ_ACTION_STOP

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class Input
{
public:
    Input() {}
    Input(MUINT32 mode, MUINT32 recNo, MBOOL isReady)
        : mMode(mode)
        , mRecNo(recNo)
        , mIsReady(isReady)
    {
    }

    MUINT32 mMode = 0;
    MUINT32 mRecNo = 0;
    MBOOL mIsReady = MFALSE;
};

class TestCase
{
public:
    TestCase() {}
    TestCase(MBOOL eisQ, MBOOL tsQ, MUINT32 start, MUINT32 qSize)
        : mEISQ(eisQ)
        , mTSQ(tsQ)
        , mStart(start)
        , mQSize(qSize)
    {
    }

    MVOID append(MUINT32 mode, MUINT32 recNo, MBOOL isReady)
    {
        mInputs.push_back(Input(mode, recNo, isReady));
    }

    MVOID append(const Input &input)
    {
        mInputs.push_back(input);
    }

    std::vector<Input> mInputs;
    MBOOL mEISQ = MFALSE;
    MBOOL mTSQ = MFALSE;
    MUINT32 mStart = 0;
    MUINT32 mQSize = 0;
};

const char* toString(IStreamingFeaturePipe::eAppMode mode)
{
    const char *str = "UNKNOWN";
    switch(mode)
    {
        case APP_PHOTO_PREVIEW: str = "PHOTO_PREVIEW";  break;
        case APP_VIDEO_PREVIEW: str = "VIDEO_PREVIEW";  break;
        case APP_VIDEO_RECORD:  str = "VIDEO_RECORD";   break;
        case APP_VIDEO_STOP:    str = "VIDEO_STOP";     break;
        default:                str = "UNKNOWN";        break;
    }
    return str;
}

const char* toString(EISQ_ACTION action)
{
    const char *str = "UNKNOWN";
    switch(action)
    {
        case EISQ_ACTION_NO:    str = "NO";       break;
        case EISQ_ACTION_READY: str = "READY";    break;
        case EISQ_ACTION_INIT:  str = "INIT";     break;
        case EISQ_ACTION_PUSH:  str = "PUSH";     break;
        case EISQ_ACTION_RUN:   str = "RUN";      break;
        case EISQ_ACTION_STOP:  str = "STOP";     break;
        default:                str = "UNKNOWN";  break;
    }
    return str;
}

MVOID generateInput(TestCase &test, MUINT32 count)
{
    MUINT32 max = test.mQSize * 3 + 10;
    for( unsigned i = 0; i < count; ++i )
    {
        MUINT32 len = 0;
        len = rand()%max;
        for( unsigned j = 0; j < len; ++j )
        {
            MBOOL isReady = rand()%2 ? EISQ_READY : EISQ_NOT_READY;
            test.append(I1, 0, isReady);
        }
        len = rand()%max;
        for( unsigned j = 0; j < len; ++j )
        {
            MBOOL isReady = rand()%2 ? EISQ_READY : EISQ_NOT_READY;
            test.append(I2, j, isReady);
        }
    }
}

EISQActionInfo getActionInfo(const Input &in)
{
    EISQActionInfo info;
    info.mAppMode = in.mMode;
    info.mRecordCount = in.mRecNo;
    info.mIsAppEIS = MTRUE;
    info.mIsReady = in.mIsReady;
    return info;
}

StreamingFeaturePipeUsage createUsage(MBOOL eisQ, MBOOL tsQ, MUINT32 startFrame, MUINT32 queueSize)
{
    const MUINT32 SENSOR_INDEX_0 = 0;
    IStreamingFeaturePipe::UsageHint hint;

    hint.mEISInfo.mode = eisQ ? (EISMODE_30 | EISMODE_QUEUE) : EISMODE_30;
    hint.mEISInfo.startFrame = startFrame;
    hint.mEISInfo.queueSize = queueSize;
    hint.mUseTSQ = tsQ;

    return StreamingFeaturePipeUsage(hint, SENSOR_INDEX_0);
}

class ValRange
{
public:
    ValRange() {}
    ValRange(MUINT32 min, MUINT32 max)
        : mMin(min)
        , mMax(max)
    {
    }

    MUINT32 mMin = 0;
    MUINT32 mMax = 0;
};

typedef std::set<EISQ_ACTION> ActRange;

MBOOL isInRange(MUINT32 val, const ValRange &range)
{
    return (val >= range.mMin) && (val <= range.mMax);
}

MBOOL isInRange(EISQ_ACTION act, const ActRange &range)
{
    return range.count(act);
}

MBOOL is(const EISQState &state, EISQ_ACTION act, MUINT32 counter)
{
    return (state.mAction == act) && (state.mCounter == counter);
}

typedef MBOOL(*RuleFunc)(const TestCase&, const Input&, const EISQState&, const EISQState &cur, MUINT32 maxQ);

MBOOL O0Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O0 )
    {
        if( in.mMode == I2 && test.mEISQ && test.mQSize )
        {
            ret = is(cur, O1, 0);
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

MBOOL O1Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O1 )
    {
        if( in.mMode == I2 )
        {
            if( in.mRecNo > test.mStart && in.mIsReady )
            {
                ret = is(cur, O2, 0);
            }
            else
            {
                ret = is(cur, O1, 0);
            }
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

MBOOL O2Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O2 )
    {
        if( in.mMode == I2 )
        {
            ret = is(cur, O3, 1);
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

MBOOL O3Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O3 )
    {
        if( in.mMode == I2 )
        {
            if( old.mCounter >= test.mQSize )
            {
                ret = is(cur, O4, test.mQSize);
            }
            else
            {
                ret = is(cur, O3, old.mCounter+1);
            }
        }
        else if( in.mMode == I3 )
        {
            ret = is(cur, O5, maxQ-1);
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

MBOOL O4Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O4 )
    {
        if( in.mMode == I2 )
        {
            ret = is(cur, O4, maxQ);
        }
        else if( in.mMode == I3 )
        {
            ret = is(cur, O5, maxQ-1);
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

MBOOL O5Rule(const TestCase &test, const Input &in, const EISQState &old, const EISQState &cur, MUINT32 maxQ)
{
    (void)test;
    (void)in;
    (void)maxQ;
    MBOOL ret = MFALSE;
    if( old.mAction == O5 )
    {
        if( in.mMode == I3 )
        {
            ret = is(cur, O5, old.mCounter ? old.mCounter-1 : 0);
        }
        else
        {
            ret = is(cur, O0, 0);
        }
    }
    return ret;
}

class Policy
{
public:
    Policy() {}
    Policy(MUINT32 min, MUINT32 max, RuleFunc rule, const ActRange &prev, const ActRange &next)
        : mCounterRange(min, max)
        , mRule(rule)
        , mPrevRange(prev)
        , mNextRange(next)
    {
    }

    MBOOL isValid(const TestCase &test, const Input &in, const EISQState &old, EISQState &cur, MUINT32 maxQ)
    {
        return (mRule == NULL) || mRule(test, in, old, cur, maxQ);
    }

    ValRange mCounterRange;
    RuleFunc mRule = NULL;
    ActRange mPrevRange;
    ActRange mNextRange;
};

std::map<EISQ_ACTION, Policy> getPolicyMap(const TestCase &test)
{
    std::map<EISQ_ACTION, Policy> policy;
    ActRange validRange = { O0, O1, O2, O3, O4 };
    MUINT32 Q = test.mQSize;

    policy[O0] = Policy(0, 0,   O0Rule, {O0,O1,O2,O3,O4,O5},  {O0,O1});
    policy[O1] = Policy(0, 0,   O1Rule, {O0,O1},              {O0,O1,O2});
    policy[O2] = Policy(0, 0,   O2Rule, {O1},                 {O0,O3});
    policy[O3] = Policy(1, Q,   O3Rule, {O2,O3},              {O0,O3,O4,O5});
    policy[O4] = Policy(Q, Q,   O4Rule, {O3,O4},              {O0,O4,O5});
    policy[O5] = Policy(0, Q-1, O5Rule, {O3,O4,O5},           {O0,O5});

    return policy;
}

MBOOL isInRange(EISQ_ACTION act, const std::map<EISQ_ACTION, Policy> &policy)
{
    return policy.count(act);
}

MVOID print(const TestCase &test, const Input &in, const EISQState &old, EISQState &cur, MUINT32 maxQ)
{
    printf("========================================================\n");
    printf("========================================================\n");
    printf("eisQ=%d tsQ=%d pSize=%d qSize=%d maxQ=%d\n",
           test.mEISQ, test.mTSQ, test.mStart, test.mQSize, maxQ);
    printf("mode=%d recNo=%d (act,counter):(%d,%d)=>(%d,%d)\n",
           in.mMode, in.mRecNo, old.mAction, old.mCounter, cur.mAction, cur.mCounter);

    std::map<EISQ_ACTION, Policy> policy = getPolicyMap(test);

    printf("isInRange(old.mAction, validRange)=%d\n"
           "isInRange(cur.mAction, validRange)=%d\n"
           "isInRange(old.mCounter, policy[old.mAction].mCounterRange)=%d\n"
           "isInRange(cur.mCounter, policy[cur.mAction].mCounterRange)=%d\n"
           "isInRange(old.mAction, policy[cur.mAction].mPrevRange)=%d\n"
           "isInRange(cur.mAction, policy[old.mAction].mNextRange)=%d\n"
           "policy[old.mAction].isValid(test, in, old, cur, maxQ)=%d\n"
           "(maxQ <= test.mQSize)=%d\n",
            isInRange(old.mAction, policy),
            isInRange(cur.mAction, policy),
            isInRange(old.mCounter, policy[old.mAction].mCounterRange),
            isInRange(cur.mCounter, policy[cur.mAction].mCounterRange),
            isInRange(old.mAction, policy[cur.mAction].mPrevRange),
            isInRange(cur.mAction, policy[old.mAction].mNextRange),
            policy[old.mAction].isValid(test, in, old, cur, maxQ),
            maxQ <= test.mQSize);
    printf("========================================================\n");
    printf("========================================================\n");
}

MBOOL checkPolicy(const TestCase &test, const Input &in, const EISQState &old, EISQState &cur, MUINT32 maxQ)
{
    MBOOL ret = MTRUE;
    std::map<EISQ_ACTION, Policy> policy = getPolicyMap(test);

    ret = isInRange(old.mAction, policy) &&
          isInRange(cur.mAction, policy) &&
          isInRange(old.mCounter, policy[old.mAction].mCounterRange) &&
          isInRange(cur.mCounter, policy[cur.mAction].mCounterRange) &&
          isInRange(old.mAction, policy[cur.mAction].mPrevRange) &&
          isInRange(cur.mAction, policy[old.mAction].mNextRange) &&
          policy[old.mAction].isValid(test, in, old, cur, maxQ) &&
          maxQ <= test.mQSize;

    return ret;
}

MBOOL runTest(const TestCase &test)
{
    MBOOL ret = MFALSE;

    EISQControl control;
    StreamingFeaturePipeUsage usage = createUsage(test.mEISQ, test.mTSQ, test.mStart, test.mQSize);
    EISQState initState, oldState, curState;

    control.init(usage);
    oldState = initState = control.getCurrentState();

    if( initState.mAction == O0 && initState.mCounter ==  0 )
    {
        MUINT32 maxQ = 0;
        ret = MTRUE;
        for( const Input in : test.mInputs )
        {
            MBOOL policy = MFALSE;
            control.update(getActionInfo(in));
            curState = control.getCurrentState();
            if( curState.mAction == O1 )
            {
                maxQ = 0;
            }
            else if( curState.mAction == O3 )
            {
                ++maxQ;
            }
            policy = checkPolicy(test, in, oldState, curState, maxQ);
            if( !policy )
            {
                print(test, in, oldState, curState, maxQ);
                ret = MFALSE;
                break;
            }
            oldState = curState;
        }
    }

    return ret;
}

TEST(EISQControl, NoQ)
{
    for(unsigned i = 0; i < 100; ++i )
    {
        MBOOL eisq = NO_EISQ;
        MBOOL tsq = rand()%2 ? USE_TSQ : NO_TSQ;
        MUINT start = rand()%3;
        MUINT qsize = rand()%3;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 10);
        ASSERT_TRUE(runTest(test));
    }

    for(unsigned i = 0; i< 100; ++i )
    {
        MBOOL eisq = rand()%2 ? USE_EISQ : NO_EISQ;
        MBOOL tsq = rand()%2 ? USE_TSQ : NO_TSQ;
        MUINT start = rand()%3;
        MUINT qsize = 0;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 10);
        ASSERT_TRUE(runTest(test));
    }
}

TEST(EISQControl, TSQ)
{
    for(unsigned i = 0; i < 100; ++i )
    {
        MBOOL eisq = USE_EISQ;
        MBOOL tsq = USE_TSQ;
        MUINT start = rand()%3;
        MUINT qsize = rand()%3;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 100);
        ASSERT_TRUE(runTest(test));
    }
}

TEST(EISQControl, RequestQ)
{
    for(unsigned i = 0; i < 100; ++i )
    {
        MBOOL eisq = USE_EISQ;
        MBOOL tsq = NO_TSQ;
        MUINT start = rand()%3;
        MUINT qsize = rand()%3;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 100);
        ASSERT_TRUE(runTest(test));
    }
}

TEST(EISQControl, Random)
{
    for(unsigned i = 0; i < 100; ++i )
    {
        MBOOL eisq = USE_EISQ;
        MBOOL tsq = rand()%2 ? USE_TSQ : NO_TSQ;
        MUINT start = rand()%3;
        MUINT qsize = rand()%3;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 100);
        ASSERT_TRUE(runTest(test));
    }
}

TEST(EISQControl, LongTest)
{
    for(unsigned i = 0; i < 100; ++i )
    {
        MBOOL eisq = USE_EISQ;
        MBOOL tsq = rand()%2 ? USE_TSQ : NO_TSQ;
        MUINT start = 19;
        MUINT qsize = 25;
        TestCase test(eisq, tsq, start, qsize);
        generateInput(test, 100);
        ASSERT_TRUE(runTest(test));
    }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
