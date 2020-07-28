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
#define PIPE_TRACE TRACE_MDPWrapper
#define PIPE_CLASS_TAG "Suite_MDPWrapper_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/featurePipe/SFPIO.h>
#include <featurePipe/streaming/StreamingFeature_Common.h>
#include <featurePipe/streaming/MDPWrapper.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/log/ILogger.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <mtkcam/utils/std/DebugTimer.h>


#include <cstdlib>
#include <vector>
#include <unordered_map>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);


#define TEST_MDP_RUN 1

using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;

using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;

using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;

using namespace NSCam::Utils;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

typedef MDPWrapper::OUTPUT_ARRAY OUTPUT_ARRAY;
typedef MDPWrapper::P2IO_OUTPUT_ARRAY P2IO_OUTPUT_ARRAY;
typedef MDPWrapper::MDPOutput MDPOutput;

namespace MDPWrapperTest{
enum AAFType
{
    NOT_EXIST,
    DUMMY,
    DISP,
    DISP_R, // size(5,5)
    REC,
    REC_R,
    EXTRA,
    EXTRA_1_R,
    EXTRA_2, // size(10,10)
    PHYSICAL,
    NEXT_FULL,
    FULL_INPUT,
    AAF_TYPE_COUNT
};

MSize typeToSize(MUINT32 type)
{
    return MSize((type+1)*16, (type+1)*16);
}

MBOOL needRotate(MUINT32 type)
{
    return (type == DISP_R || type == REC_R || type == EXTRA_1_R);
}

MBOOL needPQ(MUINT32 type)
{
    return (type >= DISP && type < PHYSICAL);
}

MSize toMSize(const MSizeF &size)
{
    return MSize(size.w, size.h);
}

class AAFOut
{
public:
    AAFOut(){}

    AAFOut(MUINT32 _type, sp<IIBuffer> buf)
    : type(_type)
    , pqParam()
    , dppq()
    , buffer(buf)
    {
        if(needRotate(type))
            p2Out.mTransform = eTransform_ROT_90;

        p2Out.mCropRect.s = typeToSize(type);
        p2Out.mCropDstSize = toMSize(p2Out.mCropRect.s);
        buffer->getImageBuffer()->setExtParam(p2Out.mCropDstSize);
        p2Out.mBuffer = buffer->getImageBufferPtr();
    }

    MVOID init()
    {

        if(needPQ(type))
        {
            p2Out.mPqParam = &pqParam;
            p2Out.mDpPqParam = &dppq;
        }
    }
    MBOOL isExist(){return type != NOT_EXIST;}
    MUINT32 type;
    PQParam pqParam;
    DpPqParam dppq;
    P2IO p2Out;
    sp<IIBuffer> buffer;
};


class MDPTestCase
{
public:
    std::vector<MUINT32> addInMDPOut;
    std::vector<std::vector<MUINT32>> result;// {Frame0 : IMG2O , WDMAO, WROTO},{Frame1 : IMG2O , WDMAO, WROTO}....
};

// =====================================
// ====== Test 1 ==============

//==End of Cases ===============================

// ==============================
// ===========  Initialize Functions ===============
// =-==================================

MVOID createTestBuffers(std::map<MUINT32, AAFOut> &bufMap, sp<ImageBufferPool>& pool)
{
    pool->allocate(AAF_TYPE_COUNT);
    for(MUINT32 i = 0 ; i < AAF_TYPE_COUNT ; i++)
    {
        sp<IIBuffer> buf = pool->requestIIBuffer();
        bufMap[i] = AAFOut(i, buf);
        bufMap[i].init(); // pq pointer need point to obj stored in map
    }
}

MDPWrapper::P2IO_OUTPUT_ARRAY createTestIns(const std::vector<MUINT32> &InList, std::map<MUINT32, AAFOut>& bufMap)
{
    MDPWrapper::P2IO_OUTPUT_ARRAY list;
    for(MUINT32 type : InList)
        list.push_back(bufMap[type].p2Out);
    return list;
}

// ==============================
// ===========  Check Functions ===============
// =-==================================

MBOOL popMDPOutput(OUTPUT_ARRAY &dst, const P2IO &target, MUINT32 type, MDPOutput &output)
{
    auto it = dst.begin();
    while(it != dst.end())
    {
        if(it->mOutput.mBuffer == target.mBuffer
            || (it->mDummy && type == DUMMY))
        {
            output = *it;
            it = dst.erase(it);
            return MTRUE;
        }
        else
            it++;
    }
    return MFALSE;
}

MVOID removeNotExist(std::vector<MUINT32> &expectResult)
{
    auto it = expectResult.begin();
    while(it != expectResult.end())
    {
        if( *it == NOT_EXIST)
            it = expectResult.erase(it);
        else
            it++;
    }
}

MVOID checkSubDst(OUTPUT_ARRAY &dst, std::vector<MUINT32> expectResult, const std::map<MUINT32, AAFOut> &bufMap)
{
    removeNotExist(expectResult);
    EXPECT_EQ(dst.size(), expectResult.size());
    if(dst.size() != expectResult.size())
        return;

    for(auto type : expectResult)
    {
        const P2IO& expectOut = bufMap.at(type).p2Out;
        MDPOutput realOut;
        MBOOL get = popMDPOutput(dst, expectOut, type, realOut);
        EXPECT_EQ(get, MTRUE);

        if(get && type != DUMMY)
        {
            EXPECT_EQ(realOut.mCropRect.s , toMSize(expectOut.mCropRect.s));
            EXPECT_EQ(realOut.mOutput.mTransform , expectOut.mTransform);
            EXPECT_EQ(realOut.mOutput.mPortID.capbility , expectOut.mCapability);
        }
    }

    EXPECT_EQ(dst.size(), 0);
}

MBOOL prepareSubDst(const OUTPUT_ARRAY &dst, OUTPUT_ARRAY::const_iterator &start, MUINT32 count, OUTPUT_ARRAY& subDst)
{
    TRACE_FUNC_ENTER();

    if( start + count < dst.end() )
    {
        subDst = OUTPUT_ARRAY(start, start + count);
        start += count;
    }
    else
    {
        subDst = OUTPUT_ARRAY(start, dst.end());
        start = dst.end();
    }

    TRACE_FUNC_EXIT();

    return !subDst.empty();
}

MVOID checkNoDummy(OUTPUT_ARRAY &outArray)
{
    int dummyCnt = 0;
    for(auto&& out : outArray)
    {
        if(out.mDummy)
            dummyCnt++;
    }
    EXPECT_EQ(dummyCnt, 0);
}

MVOID ttt(AAFOut &output, std::map<MUINT32, AAFOut> &bufMap, MUINT32 type)
{
    output = bufMap[type];
}

MVOID runTest(const MDPTestCase &test)
{

    DebugTimer timer;
    MDPWrapper mMDP;
    MUINT32 maxDpPort = DpIspStream::queryMultiPortSupport(DpIspStream::ISP_ZSD_STREAM);

    timer.start();
    std::map<MUINT32, AAFOut> bufMap;
    sp<ImageBufferPool> pool = ImageBufferPool::create("testBuf", 16*AAF_TYPE_COUNT, 16*AAF_TYPE_COUNT, eImgFmt_YV12, ImageBufferPool::USAGE_HW );
    createTestBuffers(bufMap, pool);
    P2IO_OUTPUT_ARRAY inBufList = createTestIns(test.addInMDPOut, bufMap);
    timer.stop();
    printf("maxDpPort(%d), prepare done cost time = %d us\n", maxDpPort, timer.getElapsedU());

    timer.start();
    OUTPUT_ARRAY outArray;
    mMDP.generateOutArray(inBufList, outArray, MTRUE);
    timer.stop();
    printf("calculation done cost time = %d us\n", timer.getElapsedU());

    if(maxDpPort <= 1)
    {
        checkNoDummy(outArray);
    }
    else
    {
        if(maxDpPort != test.result[0].size())
        {
            EXPECT_EQ(maxDpPort, test.result[0].size());
            printf("test case expect result port size(%d) != max MDP port size(%d), exit!\n",test.result[0].size(), maxDpPort);
            return;
        }
        OUTPUT_ARRAY::const_iterator start = outArray.begin();
        OUTPUT_ARRAY subDst;
        int count = 0;

        while( prepareSubDst(outArray, start, maxDpPort, subDst) )
        {
            printf("=== Count %d ====\n", count);
            checkSubDst(subDst, test.result[count], bufMap);
            printf("=== Count %d check done ====\n", count);
            count++;
        }
        EXPECT_EQ(count, test.result.size());
    }

#if TEST_MDP_RUN
    timer.start();
    mMDP.process(bufMap[FULL_INPUT].p2Out.mBuffer, inBufList, MTRUE);
    timer.stop();
    printf("do mdp done cost time = %d us\n", timer.getElapsedU());
#endif

    bufMap.clear();
    ImageBufferPool::destroy(pool);

#if 0
    AAFOut ooo;
    ttt(ooo, bufMap, EXTRA_1_R);
    printf("before modi, ooo transform(%d), bufMap transofrm(%d)\n", ooo.p2Out.mTransform, bufMap[EXTRA_1_R].p2Out.mTransform);
    ooo.p2Out.mTransform = 100;
    printf("after modi, ooo transform(%d), bufMap transofrm(%d)\n", ooo.p2Out.mTransform, bufMap[EXTRA_1_R].p2Out.mTransform);
#endif
}


TEST(MDPWrapper_R1_NR2, DispR_Rec_NFull)
{
    MDPTestCase testCase;
    testCase.addInMDPOut = { DISP_R, REC, NEXT_FULL };
    testCase.result = { // each run has 2 output
            {REC,           DISP_R},
            {NEXT_FULL,     NOT_EXIST}
        };
    runTest(testCase);
}

TEST(MDPWrapper_R3_NR1, DispR_RecR_NFull_ExtR)
{
    MDPTestCase testCase;
    testCase.addInMDPOut = { DISP_R, REC_R, NEXT_FULL,  EXTRA_1_R};
    testCase.result = { // each run has 2 output
            {NEXT_FULL,     DISP_R},
            {DUMMY,     REC_R},
            {NOT_EXIST,     EXTRA_1_R},
        };
    runTest(testCase);
}


} // namespace MDPWrapperTest
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
