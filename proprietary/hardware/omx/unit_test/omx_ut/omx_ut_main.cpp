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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   omx_ut_main.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video unit test code
 *
 * Author:
 * -------
 *   Bruce Hsu (mtk04278)
 *
 ****************************************************************************/

#include <stdio.h>

#include "dec_test_case.h"
#include "enc_test_case.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "OmxUT"
#include <utils/Log.h>

#include "gtest/gtest.h"

typedef struct name_map_struct {
    const char *szName;
    int         iIsEncode;
    int         iCodec;
} TNameMap;

static TNameMap NameMap[] = {
        {"OMX.MTK.VIDEO.DECODER.MPEG4", 0, 1},
        {"OMX.MTK.VIDEO.DECODER.AVC", 0, 2},
        {"OMX.MTK.VIDEO.DECODER.VPX", 0, 3},
//#ifdef MTK_SUPPORT_MJPEG
        {"OMX.MTK.VIDEO.DECODER.MJPEG", 0, 4},
//#endif//MTK_SUPPORT_MJPEG
        {"OMX.MTK.VIDEO.ENCODER.MPEG4", 1, 1},
        {"OMX.MTK.VIDEO.ENCODER.AVC", 1, 2}
};

int mapName(const char *inputName, int *piIsEncode, int *piCodec)
{
    for(size_t i=0;i<sizeof(NameMap)/sizeof(TNameMap);i++)
    {
        if(!strcmp(inputName, NameMap[i].szName))
        {
            *piIsEncode = NameMap[i].iIsEncode;
            *piCodec = NameMap[i].iCodec;
            return 1;
        }
    }
    return 0;
}
#if 0 // no gtest
int main(int argc, const char *argv[])
{
    DecTestCases    decTestCases = DecTestCases();
    EncTestCases    encTestCases = EncTestCases();

    int iIsEncode, iCodec;

    if(argc != 2)
    {
        printf("usage:%s component_name\n", argv[0]);
        return 0;
    }

    if(!mapName(argv[1], &iIsEncode, &iCodec))
    {
        printf("unknown name:%s\n", argv[1]);
        return 0;
    }
    if(iIsEncode == 0)
    {
        DecTestCaseConf    tConf;
        tConf.iCodec = iCodec;
        if(decTestCases.lastFrameTest(tConf)) printf("case 1(lastFrame) success\n");
        else printf("case 1(lastFrame) fail\n");
        if(decTestCases.portReconfigTest1(tConf)) printf("case 2(portReconfig1) success\n");
        else printf("case 2(portReconfig1) fail\n");
        //decTestCases.portReconfigTest2(tConf);
        if(decTestCases.frame1NotITest(tConf)) printf("case 4(frame1NotI) success\n");
        else printf("case 4(frame1NotI) fail\n");
        if(decTestCases.partialFrameTest(tConf)) printf("case 5(partialFrame) success\n");
        else printf("case 5(partialFrame) fail\n");
        if(decTestCases.outBufFullTest(tConf)) printf("case 9(outBufFull) success\n");
        else printf("case 9(outBufFull) fail\n");
        if(decTestCases.loopPlaybackTest(tConf)) printf("case 12(loopPlayback) success\n");
        else printf("case 12(loopPlayback) fail\n");
        if(decTestCases.trickPlayTest(tConf)) printf("case 13(trickPlay) success\n");
        else printf("case 13(trickPlay) fail\n");
        if(decTestCases.multiInstancesTest(tConf)) printf("case 14(multiInstances) success\n");
        else printf("case 14(multiInstances) fail\n");
        if(decTestCases.bufferFlagLeakTest(tConf)) printf("case 6(bufferFlagLeak) success\n");
        else printf("case 6(bufferFlagLeak) fail\n");
        if(decTestCases.noEOSFlagTest(tConf)) printf("case 7(noEOSFlag) success\n");
        else printf("case 7(noEOSFlag) fail\n");
        if(decTestCases.portReconfigFailTest(tConf)) printf("case 8(portReconfigFail) success\n");
        else printf("case 8(portReconfigFail) fail\n");
        if(decTestCases.noSequenceHeadTest(tConf)) printf("case 10(noSequenceHead) success\n");
        else printf("case 10(noSequenceHead) fail\n");
        if(iCodec != 3)//VP8 not support yet
        {
            if(decTestCases.corruptDataTest(tConf)) printf("case 11(corruptData) success\n");
            else printf("case 11(corruptData) fail\n");
        }
    }
    else
    {
        EncTestCaseConf     tConf;
        tConf.iCodec = iCodec;
        if(encTestCases.lastFrameTest(tConf)) printf("case 1(lastFrame) success\n");
        else printf("case 1(lastFrame) fail\n");
        if(encTestCases.bufferFlagLeakTest(tConf)) printf("case 6(bufferFlagLeak) success\n");
        else printf("case 6(bufferFlagLeak) fail\n");
        if(encTestCases.noEOSFlagTest(tConf)) printf("case 7(noEOSFlag) success\n");
        else printf("case 7(noEOSFlag) fail\n");
        if(encTestCases.outBufFullTest(tConf)) printf("case 9(outBufFull) success\n");
        else printf("case 9(outBufFull) fail\n");
        if(encTestCases.multiInstancesTest(tConf)) printf("case 14(multiInstances) success\n");
        else printf("case 14(multiInstances) fail\n");
    }
    printf("program return\n");
    return 0;
}
#endif//0

//--- avc decoder ---
TEST(OMXAVCDTest, test1)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.lastFrameTest(tConf), 1);
}
TEST(OMXAVCDTest, test2)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.portReconfigTest1(tConf), 1);
}
TEST(OMXAVCDTest, test3)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.frame1NotITest(tConf), 1);
}
TEST(OMXAVCDTest, test4)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.partialFrameTest(tConf), 1);
}
TEST(OMXAVCDTest, test5)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.outBufFullTest(tConf), 1);
}
TEST(OMXAVCDTest, test6)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.loopPlaybackTest(tConf), 1);
}
TEST(OMXAVCDTest, test7)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.trickPlayTest(tConf), 1);
}
TEST(OMXAVCDTest, test8)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.multiInstancesTest(tConf), 1);
}
TEST(OMXAVCDTest, test9)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.bufferFlagLeakTest(tConf), 1);
}
TEST(OMXAVCDTest, test10)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.noEOSFlagTest(tConf), 1);
}
TEST(OMXAVCDTest, test11)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.portReconfigFailTest(tConf), 1);
}
TEST(OMXAVCDTest, test12)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.noSequenceHeadTest(tConf), 1);
}
TEST(OMXAVCDTest, test13)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(decTestCases.corruptDataTest(tConf), 1);
}
//--- mp4 decoder ---
TEST(OMXMP4DTest, test1)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.lastFrameTest(tConf), 1);
}
TEST(OMXMP4DTest, test2)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.portReconfigTest1(tConf), 1);
}
TEST(OMXMP4DTest, test3)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.frame1NotITest(tConf), 1);
}
TEST(OMXMP4DTest, test4)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.partialFrameTest(tConf), 1);
}
TEST(OMXMP4DTest, test5)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.outBufFullTest(tConf), 1);
}
TEST(OMXMP4DTest, test6)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.loopPlaybackTest(tConf), 1);
}
TEST(OMXMP4DTest, test7)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.trickPlayTest(tConf), 1);
}
TEST(OMXMP4DTest, test8)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.multiInstancesTest(tConf), 1);
}
TEST(OMXMP4DTest, test9)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.bufferFlagLeakTest(tConf), 1);
}
TEST(OMXMP4DTest, test10)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.noEOSFlagTest(tConf), 1);
}
TEST(OMXMP4DTest, test11)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.portReconfigFailTest(tConf), 1);
}
TEST(OMXMP4DTest, test12)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.noSequenceHeadTest(tConf), 1);
}
TEST(OMXMP4DTest, test13)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(decTestCases.corruptDataTest(tConf), 1);
}
//--- vp8 decoder ---
TEST(OMXVP8DTest, test1)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.lastFrameTest(tConf), 1);
}
TEST(OMXVP8DTest, test2)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.portReconfigTest1(tConf), 1);
}
TEST(OMXVP8DTest, test3)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.frame1NotITest(tConf), 1);
}
TEST(OMXVP8DTest, test4)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.partialFrameTest(tConf), 1);
}
TEST(OMXVP8DTest, test5)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.outBufFullTest(tConf), 1);
}
TEST(OMXVP8DTest, test6)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.loopPlaybackTest(tConf), 1);
}
TEST(OMXVP8DTest, test7)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.trickPlayTest(tConf), 1);
}
TEST(OMXVP8DTest, test8)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.multiInstancesTest(tConf), 1);
}
TEST(OMXVP8DTest, test9)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.bufferFlagLeakTest(tConf), 1);
}
TEST(OMXVP8DTest, test10)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.noEOSFlagTest(tConf), 1);
}
TEST(OMXVP8DTest, test11)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.portReconfigFailTest(tConf), 1);
}
TEST(OMXVP8DTest, test12)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.noSequenceHeadTest(tConf), 1);
}
TEST(OMXVP8DTest, test13)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;

    tConf.iCodec = 3;
    EXPECT_EQ(decTestCases.corruptDataTest(tConf), 1);
}
//#ifdef MTK_SUPPORT_MJPEG
//--- mjpeg decoder ---
TEST(OMXMJPGDTest, test1)
{
    DecTestCases    decTestCases = DecTestCases();
    DecTestCaseConf tConf;
    tConf.iCodec = 4;
    EXPECT_EQ(decTestCases.lastFrameTest(tConf), 1);
}
//#endif//MTK_SUPPORT_MJPEG
//--- avc encoder ---
TEST(OMXAVCETest, test1)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(encTestCases.lastFrameTest(tConf), 1);
}
TEST(OMXAVCETest, test2)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(encTestCases.bufferFlagLeakTest(tConf), 1);
}
TEST(OMXAVCETest, test3)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(encTestCases.noEOSFlagTest(tConf), 1);
}
TEST(OMXAVCETest, test4)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(encTestCases.outBufFullTest(tConf), 1);
}
TEST(OMXAVCETest, test5)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 2;
    EXPECT_EQ(encTestCases.multiInstancesTest(tConf), 1);
}
//--- mp4 encoder ---
TEST(OMXMP4ETest, test1)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(encTestCases.lastFrameTest(tConf), 1);
}
TEST(OMXMP4ETest, test2)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(encTestCases.bufferFlagLeakTest(tConf), 1);
}
TEST(OMXMP4ETest, test3)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(encTestCases.noEOSFlagTest(tConf), 1);
}
TEST(OMXMP4ETest, test4)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(encTestCases.outBufFullTest(tConf), 1);
}
TEST(OMXMP4ETest, test5)
{
    EncTestCases    encTestCases = EncTestCases();
    EncTestCaseConf tConf;

    tConf.iCodec = 1;
    EXPECT_EQ(encTestCases.multiInstancesTest(tConf), 1);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}

