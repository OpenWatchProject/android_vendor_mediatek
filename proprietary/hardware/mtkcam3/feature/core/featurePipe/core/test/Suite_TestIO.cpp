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
#include <featurePipe/core/include/IOUtil.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include "TestIO.h"

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{



#define TEST_IO_FULL            1
#define TEST_IO_FOV             0
#define TEST_IO_COMBINE         0
#define TEST_IO_VENDOR_INPLACE  0
#define TEST_IO_VENDOR_ALONE    0
#define TEST_IO_DEPTH           0

android::sp<IBufferPool> createIBufferPool(const char *name)
{
    return ImageBufferPool::create(name, 16, 16, eImgFmt_YV12, ImageBufferPool::USAGE_SW );
}

typedef std::list<TestIONode*> TestNodePath;

TestIONode nodeA(NODEA_NAME);
TestIONode nodeB(NODEB_NAME);
TestIONode nodeC(NODEC_NAME);
TestIONode nodeD(NODED_NAME);
TestIONode nodeE(NODEE_NAME);
TestIONode nodeDA(NODEDA_NAME);
TestIONode nodeDB(NODEDB_NAME);

void runIOTestCase(TestNodePath &recordPath, TestNodePath &displayPath, TestNodePath &nodes, TestCase *testCases, unsigned n, TestNodePath physicalPath = {})
{
    IOControl<TestIONode, TestReqInfo> control;

    std::vector<android::sp<IBufferPool>> fullPools;
    for(unsigned i=0;i<nodes.size();++i)
    {
        fullPools.push_back(createIBufferPool("Full"));
        fullPools[i]->allocate(1);
    }

    android::sp<IBufferPool> nodeCNextFullPool = createIBufferPool("CNextFull");
    nodeCNextFullPool->allocate(1);
    nodeC.setInputBufferPool(nodeCNextFullPool);

    android::sp<IBufferPool> nodeDNextFullPool = createIBufferPool("DNextFull");
    nodeDNextFullPool->allocate(1);
    nodeD.setInputBufferPool(nodeDNextFullPool);

    control.setRoot(*displayPath.begin());

    control.addStream(STREAMTYPE_PREVIEW, displayPath);
    control.addStream(STREAMTYPE_PREVIEW_CALLBACK, displayPath);
    control.addStream(STREAMTYPE_RECORD, recordPath);
    control.addStream(STREAMTYPE_PHYSICAL, physicalPath);

    for(unsigned i=0;i<n;++i)
    {
        TestCase& t = testCases[i];

        IORequest<TestIONode, TestReqInfo> outputRequest;
        IOControl<TestIONode, TestReqInfo>::StreamSet streams;

        if( t.hasOutput.display )
        {
            streams.insert(STREAMTYPE_PREVIEW);
        }
        if( t.hasOutput.record )
        {
            streams.insert(STREAMTYPE_RECORD);
        }
        if( t.hasOutput.phy)
        {
            streams.insert(STREAMTYPE_PHYSICAL);
        }

        TestReqInfo reqInfo(0, t.needNode.pack(), 0, 0);
        control.prepareMap(streams, reqInfo, outputRequest);
        control.dump(outputRequest);

        std::list<TestIONode*>::iterator it, end;
        unsigned j = 0;
        for(it = nodes.begin(), end = nodes.end(); it != end; ++it)
        {
            TestIONode *node = *it;
            TestNodeExpect &nodeExpect = testCases[i].nodeExpect[j];

            printf("case(%u/%u) node(%u/%zu:%s)\n", i, n, j, nodes.size(), node->getName());

            EXPECT_EQ(nodeExpect.record,  outputRequest.needRecord(node));
            EXPECT_EQ(nodeExpect.display, outputRequest.needPreview(node));
            EXPECT_EQ(nodeExpect.extra,   outputRequest.needPreviewCallback(node));
            EXPECT_EQ(nodeExpect.full,    outputRequest.needFull(node));
            //EXPECT_EQ(nodeExpect.DFull,   outputRequest.needDualFull(node));
            EXPECT_EQ(nodeExpect.QFull || nodeExpect.AFull,   outputRequest.needNextFull(node));
            EXPECT_EQ(nodeExpect.phy,   outputRequest.needPhysicalOut(node));

            if(outputRequest.needNextFull(node))
            {
                MSize resize;
                android::sp<IIBuffer> img = outputRequest.getNextFullImg(node, resize);
                EXPECT_TRUE(img != NULL);
            }
            if(outputRequest.needFull(node))
            {
                android::sp<IIBuffer> img = fullPools[j]->requestIIBuffer();
                EXPECT_TRUE(img != NULL);
            }

            printf("case(%u/%u) node(%u/%zu:%s) end\n", i, n, j, nodes.size(), node->getName());
            j++;
        }
    }
}

#define FULL_NODES          &nodeA, &nodeB, &nodeC, &nodeD
#define FULL_RECORD_PATH    &nodeA, &nodeB, &nodeC, &nodeD
#define FULL_DISPLAY_PATH   &nodeA, &nodeB, &nodeC
#define FULL_PHYSICAL_PATH  &nodeA

TestCase testCasesFull[] = {
    //       OutputConfig                          NodeConfig                                         NodeA Expect                                  NodeB Expect                                NodeC Expect                                NodeD Expect
#if 1
    { {.display=true, .phy=true},     {.A=true},                                                  { {.display=true, .phy=true},                 {},                                         {},                                         {}             } },
    { {.display=true},                  {.A=true, .B=true},                                         { {.full=true},                                 {.display=true},                            {},                                         {}             } },
    { {.display=true},                  {.A=true, .C=true},                                         { {.full=true},                                 {},                                         {.display=true},                            {}             } },
    { {.display=true},                  {.A=true, .B=true,   .C=true},                              { {.full=true},                                 {.full=true},                               {.display=true},                            {}             } },

    { {.display=true},                  {.A=true, .A_3=true},                                       { {.display=true, .full=true},                  {},                                         {},                                         {}             } },
    { {.display=true},                  {.A=true, .A_3=true, .B=true},                              { {.full=true},                                 {.display=true},                            {},                                         {}             } },
    { {.display=true},                  {.A=true, .A_3=true, .C=true},                              { {.full=true},                                 {},                                         {.display=true},                            {}             } },
    { {.display=true},                  {.A=true, .A_3=true, .B=true, .C=true},                     { {.full=true},                                 {.full=true},                               {.display=true},                            {}             } },

    { {.record=true, .display=true},    {.A=true, .D=true,   .D_Q=true},                            { {.display=true, .QFull=true},                 {},                                         {},                                         {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .B=true,   .D=true, .D_Q=true},                   { {.full=true},                                 {.display=true, .QFull=true },              {},                                         {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .C=true,   .D=true, .D_Q=true},                   { {.full=true},                                 {},                                         {.display=true, .QFull=true},               {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .B=true,   .C=true, .D=true, .D_Q=true},          { {.full=true},                                 {.full=true},                               {.display=true, .QFull=true},               {.record=true} } },

    { {.record=true, .display=true},    {.A=true, .A_3=true, .D=true, .D_Q=true},                   { {.display=true, .QFull=true},                 {},                                         {},                                         {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .A_3=true, .B=true, .D=true, .D_Q=true},          { {.full=true},                                 {.display=true, .QFull=true },              {},                                         {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .A_3=true, .C=true, .D=true, .D_Q=true},          { {.full=true},                                 {},                                         {.display=true, .QFull=true},               {.record=true} } },
#endif
    { {.record=true, .display=true
        ,.phy=true} ,                   {.A=true, .A_3=true, .B=true, .C=true, .D=true, .D_Q=true}, { {.full=true, .phy=true},                      {.full=true},                               {.display=true, .QFull=true},               {.record=true} } },

};

#if TEST_IO_FULL
TEST(FeatureIO, IO_FULL)
{
    TestNodePath recordPath = {FULL_RECORD_PATH}, displayPath = {FULL_DISPLAY_PATH}, nodes = {FULL_NODES};
    TestNodePath physicalPath = {FULL_PHYSICAL_PATH};
    unsigned n = sizeof(testCasesFull)/sizeof(testCasesFull[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesFull, n, physicalPath);
}
#endif

#define FOV_NODES          &nodeA, &nodeB, &nodeC
#define FOV_RECORD_PATH    &nodeA, &nodeB, &nodeC
#define FOV_DISPLAY_PATH   &nodeA, &nodeB, &nodeC

TestCase testCasesFOV[] = {
    //       OutputConfig                 NodeConfig                      NodeA Expect                     NodeB Expect                     NodeC Expect
    { {.record=true, .display=true},    {.A=true, },                    { {.display=true, .record=true},   {},                              {},                           } },
    { {.record=true, .display=true},    {.A=true, .B=true},             { {.full=true},                    {.display=true, .record=true},   {},                           } },
    { {.record=true, .display=true},    {.A=true, .C=true},             { {.full=true},                    {},                              {.display=true, .record=true} } },
    { {.record=true, .display=true},    {.A=true, .B=true, .C=true},    { {.full=true},                    {.full=true},                    {.display=true, .record=true} } },
};

#if TEST_IO_FOV
TEST(FeatureIO, IO_FOV)
{
    TestNodePath recordPath = {FOV_RECORD_PATH}, displayPath = {FOV_DISPLAY_PATH}, nodes = {FOV_NODES};
    unsigned n = sizeof(testCasesFOV)/sizeof(testCasesFOV[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesFOV, n);
}
#endif

#define COMBINE_NODES          &nodeA, &nodeB, &nodeD
#define COMBINE_RECORD_PATH    &nodeA, &nodeD
#define COMBINE_DISPLAY_PATH   &nodeA, &nodeB

TestCase testCasesCombine[] = {
    //       OutputConfig               NodeConfig                                NodeA Expect                      NodeB Expect       NodeD Expect
    { {.record=true, .display=true},    {.A=true, .D=true, .D_Q=true},          { {.display=true, .QFull=true},     {},                {.record=true} } },
    { {.record=true, .display=true},    {.A=true, .B=true, .D=true, .D_Q=true}, { {.QFull=true},                    {.display=true},   {.record=true} } },
};

#if TEST_IO_COMBINE
TEST(FeatureIO, IO_COMBINE)
{
    TestNodePath recordPath = {COMBINE_RECORD_PATH}, displayPath = {COMBINE_DISPLAY_PATH}, nodes = {COMBINE_NODES};
    unsigned n = sizeof(testCasesCombine)/sizeof(testCasesCombine[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesCombine, n);
}
#endif

#define FULL_VENDOR_NODES          &nodeA, &nodeB, &nodeC, &nodeD, &nodeE
#define FULL_VENDOR_RECORD_PATH    &nodeA, &nodeB, &nodeC, &nodeD
#define FULL_VENDOR_DISPLAY_PATH   &nodeA, &nodeB, &nodeC, &nodeE

TestCase testCasesVendorInplace[] = {
    //       OutputConfig                          NodeConfig                                                                    NodeA Expect    NodeB Expect   NodeC Expect    NodeD Expect    NodeE Expect
    { {.display=true},                  {.A=true, .C=true,   .E=true},                                                         { {.full=true},   {},            {.full=true},   {},             {.display=true}} },
    { {.display=true},                  {.A=true, .B=true,   .C=true,   .C_I=true,  .E=true},                                  { {.full=true},   {.full=true},  {.full=true},   {},             {.display=true}} },

    { {.display=true},                  {.A=true, .A_3=true, .C=true,   .C_I=true,  .E=true},                                  { {.full=true},   {},            {.full=true},   {},             {.display=true}} },
    { {.display=true},                  {.A=true, .A_3=true, .B=true,   .C=true,    .C_I=true,  .E=true},                      { {.full=true},   {.full=true},  {.full=true},   {},             {.display=true}} },

    { {.record=true, .display=true},    {.A=true, .C=true,   .C_I=true, .D=true,    .D_Q=true,  .E=true},                      { {.QFull=true},  {},            {.full=true},   {.record=true}, {.display=true}} },
    { {.record=true, .display=true},    {.A=true, .B=true,   .C=true,   .C_I=true,  .D=true,    .D_Q=true,  .E=true},          { {.full=true},   {.QFull=true}, {.full=true},   {.record=true}, {.display=true}} },

    { {.record=true, .display=true},    {.A=true, .A_3=true, .C=true,   .C_I=true,  .D=true,    .D_Q=true,  .E=true},          { {.QFull=true},  {},            {.full=true},   {.record=true}, {.display=true}} },
    { {.record=true, .display=true},    {.A=true, .A_3=true, .B=true,   .C=true,    .C_I=true,  .D=true, .D_Q=true,  .E=true}, { {.full=true},   {.QFull=true}, {.full=true},   {.record=true}, {.display=true}} },
};

#if TEST_IO_VENDOR_INPLACE
TEST(FeatureIO, IO_VENDOR_INPLACE)
{
    TestNodePath recordPath = {FULL_VENDOR_RECORD_PATH}, displayPath = {FULL_VENDOR_DISPLAY_PATH}, nodes = {FULL_VENDOR_NODES};
    unsigned n = sizeof(testCasesVendorInplace)/sizeof(testCasesVendorInplace[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesVendorInplace, n);
}
#endif

#define VENDOR_ALONE_NODES          &nodeA, &nodeB, &nodeC, &nodeE
#define VENDOR_ALONE_RECORD_PATH    &nodeA, &nodeB, &nodeC, &nodeE
#define VENDOR_ALONE_DISPLAY_PATH   &nodeA, &nodeB, &nodeC, &nodeE

TestCase testCasesVendorAlone[] = {
    //       OutputConfig                          NodeConfig                                                 NodeA Expect                  NodeB Expect    NodeC Expect    NodeE Expect
    { {.display=true},                  {.A=true, .C=true,   .C_A=true, .E=true},                           { {.AFull=true},                {},             {.full=true},   {.display=true}} },
    { {.display=true},                  {.A=true, .B=true,   .C=true,   .C_A=true,  .E=true},               { {.full=true},                 {.AFull=true},  {.full=true},   {.display=true}} },

    { {.display=true},                  {.A=true, .A_3=true, .C=true,   .C_A=true,  .E=true},               { {.full=true, .AFull=true},    {},             {.full=true},   {.display=true}} },
    { {.display=true},                  {.A=true, .A_3=true, .B=true,   .C=true,    .C_A=true,  .E=true},   { {.full=true},                 {.AFull=true},  {.full=true},   {.display=true}} },

    { {.record=true, .display=true},    {.A=true, .C=true,   .C_A=true, .E=true},                           { {.AFull=true},                {},             {.full=true},   {.display=true, .record=true}} },
    { {.record=true, .display=true},    {.A=true, .B=true,   .C=true,   .C_A=true,  .E=true},               { {.full=true},                 {.AFull=true},  {.full=true},   {.display=true, .record=true}} },

    { {.record=true, .display=true},    {.A=true, .A_3=true, .C=true,   .C_A=true,  .E=true},               { {.full=true, .AFull=true},    {},             {.full=true},   {.display=true, .record=true}} },
    { {.record=true, .display=true},    {.A=true, .A_3=true, .B=true,   .C=true,    .C_A=true,  .E=true},   { {.full=true},                 {.AFull=true},  {.full=true},   {.display=true, .record=true}} },
};

#if TEST_IO_VENDOR_ALONE
TEST(FeatureIO, IO_VENDOR_ALONE)
{
    TestNodePath recordPath = {VENDOR_ALONE_RECORD_PATH}, displayPath = {VENDOR_ALONE_DISPLAY_PATH}, nodes = {VENDOR_ALONE_NODES};
    unsigned n = sizeof(testCasesVendorAlone)/sizeof(testCasesVendorAlone[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesVendorAlone, n);
}
#endif

#define DEPTH_NODES         &nodeDB, &nodeDA, &nodeA
#define DEPTH_RECORD_PATH   &nodeDB, &nodeDA, &nodeA
#define DEPTH_DISPLAY_PATH  &nodeDB, &nodeDA, &nodeA

TestCase testCasesDepth[] = {
    //       OutputConfig               NodeConfig                        NodeDB Expect     NodeDA Expect     NodeA Expect
    { {.display=true},                  {.DA=true, .A=true},            { {},               {.full=true},   {.display=true}, } },
    { {.display=true},                  {.DB=true, .DA=true, .A=true},  { {.DFull=true},    {.full=true},   {.display=true}, } },
};

#if TEST_IO_DEPTH
TEST(FeatureIO, IO_DEPTH)
{
    TestNodePath recordPath = {DEPTH_RECORD_PATH}, displayPath = {DEPTH_DISPLAY_PATH}, nodes = {DEPTH_NODES};
    unsigned n = sizeof(testCasesDepth)/sizeof(testCasesDepth[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesDepth, n);
}
#endif

#define DEPTH_NG_NODES         &nodeDA, &nodeDB, &nodeA
#define DEPTH_NG_RECORD_PATH   &nodeDA, &nodeDB, &nodeA
#define DEPTH_NG_DISPLAY_PATH  &nodeDA, &nodeDB, &nodeA

TestCase testCasesDepthNG[] = {
    //       OutputConfig               NodeConfig                        NodeDB Expect     NodeDA Expect     NodeA Expect
    { {.display=true},                  {.DA=true, .A=true},            { {},               {.full=true},   {.display=true}, } },
    { {.display=true},                  {.DB=true, .DA=true, .A=true},  { {.DFull=true},    {.full=true},   {.display=true}, } },
};
/*
TEST(FeatureIO, IO_DEPTH_NG)
{
    TestNodePath recordPath = {DEPTH_NG_RECORD_PATH}, displayPath = {DEPTH_NG_DISPLAY_PATH}, nodes = {DEPTH_NG_NODES};
    unsigned n = sizeof(testCasesDepthNG)/sizeof(testCasesDepthNG[0]);
    runIOTestCase(recordPath, displayPath, nodes, testCasesDepthNG, n);
}*/

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
