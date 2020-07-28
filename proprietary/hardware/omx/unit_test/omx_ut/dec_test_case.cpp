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
 *   dec_test_case.cpp
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

#include "dec_test_case.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "OmxUT"
#include <utils/Log.h>

int DecTestCases::decTestConfig(int iCodec)
{
    ALOGD("codec=%d\n", iCodec);
    memset(&mConf, 0, sizeof(DecTestConf));
    switch(iCodec)
    {
        case 1://MPEG4
            mConf.eCodec       = DEC_CODEC_MP4;
            mConf.szInFile1    = "/mnt/sdcard/Omx/mp4.m4v";
            //mConf.szInFile1    = "/mnt/sdcard2/MPEG4/352x288_30_512.cmp";
            mConf.iWidth1      = 352;
            mConf.iHeight1     = 288;
            mConf.iFrameNum    = 1951;
            break;
        case 2://H264
            mConf.eCodec       = DEC_CODEC_AVC;
            mConf.szInFile1    = "/mnt/sdcard/Omx/h264.264";
            //mConf.szInFile1    = "/mnt/sdcard2/H264/H264_HP_1280x544_StarTrek_clip_01.h264";
            mConf.iWidth1      = 1280;
            mConf.iHeight1     = 544;
            mConf.iFrameNum    = 136;
            break;
        case 3://VP8
            mConf.eCodec       = DEC_CODEC_VP8;
            mConf.szInFile1    = "/mnt/sdcard/Omx/vp8.ivf";
            //mConf.szInFile1    = "/mnt/sdcard2/VP8/640x360_big_buck_bunny_trailer.ivf";
            mConf.iWidth1      = 640;
            mConf.iHeight1     = 360;
            mConf.iFrameNum    = 812;
            break;
//#ifdef MTK_SUPPORT_MJPEG
        case 4://MJPG
            mConf.eCodec       = DEC_CODEC_MJPG;
            mConf.szInFile1    = "/mnt/sdcard/Omx/mjpg.mymjpg";
            mConf.iWidth1      = 640;
            mConf.iHeight1     = 480;
            mConf.iFrameNum    = 30;
            break;
//#endif//MTK_SUPPORT_MJPEG
        default:
            ALOGE("unknown codec:%d\n", iCodec);
            break;
    }
    return 1;
}

//conformance
int DecTestCases::lastFrameTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    int check = decTest.checkLastFrame();
    if(check == 0)
    {
        printf("last frame fail\n");
    }

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    return (check == 0) ? 0 : 1;
}
int DecTestCases::portReconfigTest1(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 1;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    return 1;
}
int DecTestCases::portReconfigTest2(DecTestCaseConf &tCaseConf)
{//not support now (2012/05/16)
    decTestConfig(tCaseConf.iCodec);
#if 0//H264
    mConf.szInFile1 = "/mnt/sdcard2/H264/H264_HP_176x144_30_384.264";
    mConf.szInFile2 = "/mnt/sdcard2/H264/H264_MP_640x480_Home_28_40.264";
    mConf.iPortReconfigType = 2;
    mConf.iWidth1   = 176;
    mConf.iHeight1  = 144;
    mConf.iWidth2   = 640;
    mConf.iHeight2  = 480;
#endif//0
    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
static void *multiInstanceThread(void *pvData)
{
    DecTestCases *pDecTestCases = (DecTestCases*)pvData;

    DecTest decTest=DecTest();
    unsigned int current_tid = gettid();

    printf("[tid:%d] start thread!\n", current_tid);
    if(decTest.init(pDecTestCases->mConf) != 0)
    {
        printf("[tid:%d] Fail to init!\n", current_tid);
        return NULL;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("[tid:%d] Fail to prepare!\n", current_tid);
        return NULL;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("[tid:%d] Fail to decode\n", current_tid);
        return NULL;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("[tid:%d] Fail to finalize\n", current_tid);
        return NULL;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("[tid:%d] Fail to deinit!\n", current_tid);
        return NULL;
    }

    return NULL;
}
int DecTestCases::multiInstancesTest(DecTestCaseConf &tCaseConf)
{
    const int   iNum = 2;
    pthread_t   aTid[iNum];

    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bMultiInstance = true;
    pthread_mutex_init(&mConf.tInitLock, NULL);

    //do multipthread
    for(int i=0;i<2;i++)
    {
        pthread_create(&aTid[i], NULL, multiInstanceThread, (void *)(this));
    }

    //join all threads
    for(int i=0;i<2;i++)
    {
        pthread_join(aTid[i], NULL);
    }

    pthread_mutex_destroy(&mConf.tInitLock);
    return 1;
}
//error handle
int DecTestCases::frame1NotITest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bFrame1NotI = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int DecTestCases::partialFrameTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bPartialFrame = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int DecTestCases::bufferFlagLeakTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bBufFlagLeak = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {//H264 may fail
        printf("Fail to decode\n");
        decTest.finalize();
        decTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int DecTestCases::noEOSFlagTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.bNoEOS    = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {//MPEG4 may fail
        printf("Fail to decode\n");
        decTest.errorhandle();
        printf("wierd 1\n");
        decTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    return 1;
}
int DecTestCases::portReconfigFailTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 1;
    mConf.bPortReconfigFail = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        decTest.errorhandle();
        decTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 0;
}
int DecTestCases::outBufFullTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 1;
    mConf.bOutBufFull = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int DecTestCases::noSequenceHeadTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bNoSequenceHead = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {//H264, MPEG4 may fail
        printf("Fail to decode\n");
        decTest.errorhandle();
        decTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    //VP8 may pass
    return 1;
}
int DecTestCases::corruptDataTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bCorruptData = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        decTest.errorhandle();
        decTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
//special
int DecTestCases::loopPlaybackTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bLoopPlayback = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int DecTestCases::trickPlayTest(DecTestCaseConf &tCaseConf)
{
    decTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bTrickPlay = true;

    DecTest decTest=DecTest();

    if(decTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(decTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(decTest.decode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(decTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(decTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}

