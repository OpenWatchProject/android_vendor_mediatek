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
#define PIPE_TRACE TRACE_AutoAddFrame
#define PIPE_CLASS_TAG "Suite_AutoAddFrame_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/featurePipe/SFPIO.h>
#include <featurePipe/streaming/StreamingFeature_Common.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/log/ILogger.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <mtkcam/utils/std/DebugTimer.h>


#include <cstdlib>
#include <vector>
#include <unordered_map>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

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
namespace AutoAddFrameTest{

enum AAFType
{
    NOT_EXIST,
    UNKNOWN,
    FULL,
    DISP,
    DISP_R, // size(5,5)
    REC,
    REC_R,
    EXTRA,
    EXTRA_1_R,
    EXTRA_2, // size(10,10)
    PHYSICAL,
    NEXT_FULL,
    AAF_TYPE_COUNT
};

MSize typeToSize(MUINT32 type)
{
    return MSize(type+1, type+1);
}

MBOOL needRotate(MUINT32 type)
{
    return (type == DISP_R || type == REC_R || type == EXTRA_1_R);
}

MBOOL needPQ(MUINT32 type)
{
    return (type >= DISP && type < PHYSICAL);
}

MSize toMSize(const MSizeF size)
{
    return MSize(size.w, size.h);
}

class AAFOut
{
public:
    AAFOut(){}

    AAFOut(MUINT32 _type, const sp<IIBuffer> &buf)
    : type(_type)
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



static PortID toPortID(MUINT32 index)
{
    switch(index)
    {
        case 0:
            return PORT_IMG2O;
        case 1:
            return PORT_WDMAO;
        case 2:
            return PORT_WROTO;
        default:
            MY_LOGE("unknown array index %d return IMGI!!", index);
            return PORT_IMGI;

    }
}

static MUINT32 indexToCropGID(MUINT32 index)
{
    switch(index)
    {
        case 0:
            return IMG2O_CROP_GROUP;
        case 1:
            return WDMAO_CROP_GROUP;
        case 2:
            return WROTO_CROP_GROUP;
        default:
            MY_LOGE("unknown array index %d for crop gid!!", index);
            return 999;

    }
}

static MINT32 toCropGID(PortID& port)
{
    switch(port.index)
    {
        case EPortIndex_IMG2O:
            return IMG2O_CROP_GROUP;
        case EPortIndex_WDMAO:
            return WDMAO_CROP_GROUP;
        case EPortIndex_WROTO:
            return WROTO_CROP_GROUP;
        default:
            MY_LOGE("unknown port index %d for crop gid!!", port.index);
            return 999;

    }
}

class AAFTestCase
{
public:
    std::vector<std::vector<MUINT32>> initQParam; // {Frame0 : IMG2O , WDMAO, WROTO},{Frame1 : IMG2O , WDMAO, WROTO}....
    std::vector<MUINT32> addInMDPOut;
    MUINT32 refFrameInd;
    std::vector<std::vector<MUINT32>> result;
    std::vector<std::vector<MUINT32>> addOneFrameResult;
    std::vector<MUINT32> addOneFrameLeftList;
};

// =====================================
// ====== Test 1 ==============

//==End of Cases ===============================

// ==============================
// ===========  Initialize Functions ===============
// =-==================================

MVOID updatePQ(PQParam*& pPQ, P2IO& out, MUINT32 index)
{
    if((pPQ) == NULL)
        (pPQ) = static_cast<NSCam::NSIoPipe::PQParam*>(out.mPqParam);

    PortID id = toPortID(index);
    if(id.index == EPortIndex_WDMAO && pPQ != NULL)
        (pPQ)->WDMAPQParam = out.mDpPqParam;
    else if(id.index == EPortIndex_WROTO & pPQ != NULL)
        (pPQ)->WROTPQParam = out.mDpPqParam;
}

MVOID createTestBuffers(std::map<MUINT32, AAFOut> &bufMap, sp<IBufferPool>& pool)
{
    pool->allocate(AAF_TYPE_COUNT);
    for(MUINT32 i = 0 ; i < AAF_TYPE_COUNT ; i++)
    {
        sp<IIBuffer> buf = pool->requestIIBuffer();
        bufMap[i] = AAFOut(i, buf);
        bufMap[i].init(); // pq pointer need point to obj stored in map
    }
}

QParams createInitQParam(const std::vector<std::vector<MUINT32>>& initQParam, std::map<MUINT32, AAFOut>& bufMap)
{
    QParams qparam;
    for(MUINT32 i = 0; i < initQParam.size() ; i++)
    {
        FrameParams f;
        PQParam *pPQ = NULL;
        for(MUINT32 index = 0; index <  initQParam[i].size() ; index++)
        {
            MUINT32 type = initQParam[i][index];
            if(type == NOT_EXIST)
                continue;
            PortID portID = toPortID(index);
            MINT32 cropGroup = indexToCropGID(index);
            P2IO &out = bufMap[type].p2Out;
            Feature::P2Util::push_out(f, portID, out);
            Feature::P2Util::push_crop(f, cropGroup, out.mCropRect, out.mCropDstSize, out.mDMAConstrain);
            updatePQ(pPQ, out, index);
        }

        if(pPQ != NULL)
        {
            ExtraParam extra;
            extra.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
            extra.moduleStruct = pPQ;
            f.mvExtraParam.push_back(extra);
        }

        qparam.mvFrameParams.push_back(f);
    }
    return qparam;
}

std::vector<P2IO> createTestIns(const std::vector<MUINT32> &InList, std::map<MUINT32, AAFOut>& bufMap)
{
    std::vector<P2IO> list;
    for(MUINT32 type : InList)
        list.push_back(bufMap[type].p2Out);
    return list;
}

// ==============================
// ===========  Check Functions ===============
// =-==================================

MUINT32 countOut(FrameParams &frame, PortID &portID)
{
    MUINT32 count = 0;
    for(auto&& out : frame.mvOut)
    {
        if(out.mPortID.index == portID.index)
            ++count;
    }
    return count;
}

MBOOL findOut(const FrameParams &frame, const PortID &portID, Output &output)
{
    for(auto&& out : frame.mvOut)
    {
        if(out.mPortID.index == portID.index)
        {
            output = out;
            return MTRUE;
        }
    }
    return MFALSE;
}

MUINT32 countCrop(FrameParams &frame, PortID &portID)
{
    MUINT32 count = 0;
    MINT32 gid = toCropGID(portID);
    for(auto&& cropInfo : frame.mvCropRsInfo)
    {
        if(cropInfo.mGroupID == gid)
            ++count;
    }
    return count;
}

MBOOL findCrop(FrameParams &frame, PortID &portID, MCrpRsInfo &outCrop)
{
    MINT32 gid = toCropGID(portID);
    for(auto&& crop : frame.mvCropRsInfo)
    {
        if(crop.mGroupID == gid)
        {
            outCrop = crop;
            return MTRUE;
        }
    }
    return MFALSE;
}

MVOID checkOutput(FrameParams &frame, PortID &portID, AAFOut& aafOut)
{
    EXPECT_EQ(aafOut.isExist() ? (MUINT32)1 : 0 , countOut(frame, portID));
    if(aafOut.isExist())
    {
        Output output;
        EXPECT_EQ(MTRUE, findOut(frame, portID, output));
        EXPECT_EQ(output.mBuffer , aafOut.p2Out.mBuffer);
    }
}

MVOID checkCrop(FrameParams &frame, PortID &portID, AAFOut& aafOut)
{
    EXPECT_EQ(aafOut.isExist() ? (MUINT32)1 : 0 , countCrop(frame, portID));
    if(aafOut.isExist())
    {
        MCrpRsInfo crop;
        EXPECT_EQ(MTRUE, findCrop(frame, portID, crop));
        EXPECT_EQ(crop.mCropRect.s , toMSize(aafOut.p2Out.mCropRect.s));
        EXPECT_EQ(crop.mResizeDst , aafOut.p2Out.mCropDstSize);
    }
}

MVOID checkPQParam(const FrameParams &frame, const std::vector<MUINT32> &expectResult, const std::map<MUINT32, AAFOut> &bufMap)
{
    MBOOL pqExist = MFALSE;
    for(auto&& type : expectResult)
    {
        pqExist |= needPQ(type);
    }
    EXPECT_EQ(pqExist ? 1 : 0 , (int)(frame.mvExtraParam.size()));
    if(frame.mvExtraParam.size())
    {
        ExtraParam extraParam = frame.mvExtraParam.itemAt(0);
        EXPECT_EQ(extraParam.CmdIdx , EPIPE_MDP_PQPARAM_CMD);
        ASSERT_NE(extraParam.moduleStruct, (void*)NULL);
        PQParam *pPQ = static_cast<NSCam::NSIoPipe::PQParam*>(extraParam.moduleStruct);

        MBOOL pqAddrCorrect = MFALSE;
        for(MUINT32 index = 0; index < expectResult.size() ; index++)
        {
            MUINT32 type = expectResult[index];
            PortID port = toPortID(index);
            const P2IO& p2Out = bufMap.at(type).p2Out;
            printf("Check PQ(%p). index(%d), type(%d), p2Out.pq/dppq(%p/%p)\n", pPQ, index, type, p2Out.mPqParam, p2Out.mDpPqParam);
            if(port.index == EPortIndex_WDMAO)
            {
                EXPECT_EQ(pPQ->WDMAPQParam , p2Out.mDpPqParam);
                pqAddrCorrect |= (extraParam.moduleStruct == p2Out.mPqParam);
            }
            else if(port.index == EPortIndex_WROTO)
            {
                EXPECT_EQ(pPQ->WROTPQParam , p2Out.mDpPqParam);
                pqAddrCorrect |= (extraParam.moduleStruct == p2Out.mPqParam);
            }
        }
        EXPECT_EQ(MTRUE, pqAddrCorrect);
    }
}

MVOID checkQParamContent(QParams &qParam, const std::vector<std::vector<MUINT32>> &result, std::map<MUINT32, AAFOut> &bufMap)
{
    EXPECT_EQ(qParam.mvFrameParams.size() , result.size());

    for(MUINT32 i = 0; i < qParam.mvFrameParams.size() ; i++)
    {
        printf("Start Check frame (%d/%zu)\n", i , qParam.mvFrameParams.size());
        FrameParams& frame = qParam.mvFrameParams.editItemAt(i);
        for(MUINT32 index = 0; index < result[i].size() ; index++)
        {
            printf("Check Out/Crop. (index %d) :\n", index);
            MUINT32 type = result[i][index];
            PortID port = toPortID(index);
            checkOutput(frame, port, bufMap[type]);
            checkCrop(frame, port, bufMap[type]);
        }

        checkPQParam(frame, result[i], bufMap);
    }
}

MVOID runTest(const AAFTestCase &test)
{

    DebugTimer timer;
    timer.start();

    std::map<MUINT32, AAFOut> bufMap;
    sp<IBufferPool> pool = ImageBufferPool::create("testBuf", 80, 80, eImgFmt_YV12, ImageBufferPool::USAGE_SW );
    createTestBuffers(bufMap, pool);
    QParams qParam = createInitQParam(test.initQParam, bufMap);
    QParams oneFrameTestParam = qParam;

    std::vector<P2IO> inBufList = createTestIns(test.addInMDPOut, bufMap);

    Feature::ILog log = Feature::makeSensorLogger("AutoAddFrame", 0,0);
    Feature::P2Util::printQParams(log, qParam);
    timer.stop();
    printf("prepare done cost time = %d us\n", timer.getElapsedU());

    // Do prepareMDPFrame  Test
    timer.start();
    Feature::P2Util::prepareMDPFrameParam(qParam, test.refFrameInd, inBufList);
    timer.stop();

    printf("prepareMDPFrameParam calculation done cost time = %d us\n", timer.getElapsedU());
    MY_LOGD("========= prepareMDPFrameParam Result ========");
    Feature::P2Util::printQParams(log, qParam);
    checkQParamContent(qParam, test.result, bufMap);


    // Do prepareOneMDPFrame  Test
    timer.start();
    FrameParams &f = oneFrameTestParam.mvFrameParams.editItemAt(test.refFrameInd);
    std::vector<P2IO> oneFrameLeftList;
    Feature::P2Util::prepareOneMDPFrameParam(f, inBufList, oneFrameLeftList);
    timer.stop();

    printf("prepareOneMDPFrameParam calculation done cost time = %d us\n", timer.getElapsedU());
    MY_LOGD("========= prepareOneMDPFrameParam Result ========");
    Feature::P2Util::printQParams(log, oneFrameTestParam);
    checkQParamContent(oneFrameTestParam, test.addOneFrameResult, bufMap);

    std::vector<P2IO> oneFrameLeftResult = createTestIns(test.addOneFrameLeftList, bufMap);
    EXPECT_EQ(oneFrameLeftList.size() , oneFrameLeftResult.size());
    for(size_t i = 0 ; i < oneFrameLeftList.size() ; i++)
    {
        MY_LOGD("Left[%d] , buf(%p), size(%dx%d)", i, oneFrameLeftList[i].mBuffer,
                oneFrameLeftList[i].mCropDstSize.w, oneFrameLeftList[i].mCropDstSize.h);
        MBOOL match = MFALSE;
        for(auto&& result : oneFrameLeftResult)
        {
            if(result.mBuffer == oneFrameLeftList[i].mBuffer)
            {
                match = MTRUE;
                break;
            }
        }
        EXPECT_EQ(match, MTRUE);
    }

}

TEST(AutoAddFrame, Disp)
{
    AAFTestCase testCase;
    testCase.initQParam = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       NOT_EXIST,      NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addInMDPOut = { DISP};
    testCase.refFrameInd = 0;
    testCase.result = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       DISP,        NOT_EXIST},
            {UNKNOWN,       UNKNOWN,    NOT_EXIST}
        };

    testCase.addOneFrameResult = {
             {UNKNOWN,       DISP,        NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addOneFrameLeftList = {};
    runTest(testCase);
}

TEST(AutoAddFrame, DispR_Rec)
{
    AAFTestCase testCase;
    testCase.initQParam = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       NOT_EXIST,      NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addInMDPOut = { DISP_R, REC};
    testCase.refFrameInd = 0;
    testCase.result = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       REC,        DISP_R},
            {UNKNOWN,       UNKNOWN,    NOT_EXIST}
        };

    testCase.addOneFrameResult = {
            {UNKNOWN,       REC,            DISP_R},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addOneFrameLeftList = {};
    runTest(testCase);
}

TEST(AutoAddFrame, DispR_Rec_NFull)
{
    AAFTestCase testCase;
    testCase.initQParam = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       NOT_EXIST,      NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addInMDPOut = { DISP_R, REC, NEXT_FULL };
    testCase.refFrameInd = 0;
    testCase.result = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       REC,        DISP_R},
            {UNKNOWN,       UNKNOWN,    NOT_EXIST},
            {NOT_EXIST,     NEXT_FULL,  NOT_EXIST}
        };

    testCase.addOneFrameResult = {
            {UNKNOWN,       REC,            DISP_R},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addOneFrameLeftList = {NEXT_FULL};
    runTest(testCase);
}

TEST(AutoAddFrame, Disp_RecR_NFull_EXTRA1R)
{
    AAFTestCase testCase;
    testCase.initQParam = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       PHYSICAL,      NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addInMDPOut = { DISP, REC_R, EXTRA_1_R, NEXT_FULL };
    testCase.refFrameInd = 1;
    testCase.result = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       PHYSICAL,        NOT_EXIST},
            {UNKNOWN,       UNKNOWN,    REC_R},
            {NOT_EXIST,     DISP,  EXTRA_1_R},
            {NOT_EXIST,     NEXT_FULL,  NOT_EXIST}
        };

    testCase.addOneFrameResult = {
            {UNKNOWN,       PHYSICAL,        NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        REC_R}
        };
    testCase.addOneFrameLeftList = {DISP, EXTRA_1_R, NEXT_FULL};
    runTest(testCase);
}

TEST(AutoAddFrame, Disp_NFull_RecR_EXTRA_2_EXTRA1R)
{
    AAFTestCase testCase;
    testCase.initQParam = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       NOT_EXIST,      NOT_EXIST},
            {UNKNOWN,       UNKNOWN,        NOT_EXIST}
        };
    testCase.addInMDPOut = { DISP, NEXT_FULL, REC_R, EXTRA_2, EXTRA_1_R  };
    testCase.refFrameInd = 0;
    testCase.result = {
            // IMG2O        WDMAO           WROTO
            {UNKNOWN,       DISP,        REC_R},
            {UNKNOWN,       UNKNOWN,    NOT_EXIST},
            {NOT_EXIST,     NEXT_FULL,  EXTRA_1_R},
            {NOT_EXIST,     EXTRA_2,  NOT_EXIST}
        };
    testCase.addOneFrameResult = {
            {UNKNOWN,       DISP,        REC_R},
            {UNKNOWN,       UNKNOWN,    NOT_EXIST}
        };
    testCase.addOneFrameLeftList = { NEXT_FULL, EXTRA_2, EXTRA_1_R};
    runTest(testCase);
}


} // namespace AutoAddFrameTest
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
