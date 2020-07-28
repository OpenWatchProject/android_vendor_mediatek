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

#include "enc_test_case.h"

#define LOG_TAG "OmxUT"
#include <utils/Log.h>

int EncTestCases::encTestConfig(int iCodec)
{
    ALOGD("codec=%d\n", iCodec);
    memset(&mConf, 0, sizeof(EncTestConf));
    switch(iCodec)
    {
        case 1://MPEG4
            mConf.eCodec       = ENC_CODEC_MP4;
            //mConf.szInFile1    = "/mnt/sdcard2/qvga/akiyo_qvga.yuv";
            mConf.szInFile1    = "/mnt/sdcard/Omx/mp4.yuv";
            mConf.iWidth1      = 320;
            mConf.iHeight1     = 240;
            mConf.iFrameNum    = 300;
            break;
        case 2://H264
            mConf.eCodec       = ENC_CODEC_AVC;
            //mConf.szInFile1    = "/mnt/sdcard2/hvga/lab_fast_hvga_212f.yuv";
            mConf.szInFile1    = "/mnt/sdcard/Omx/h264.yuv";
            mConf.iWidth1      = 480;
            mConf.iHeight1     = 320;
            mConf.iFrameNum    = 212+1;//for header
            break;
        case 3://VP8
            mConf.eCodec       = ENC_CODEC_VP8;
            //mConf.szInFile1    = "/mnt/sdcard2/notsupport";
            mConf.szInFile1    = "/mnt/sdcard/notsupport";
            mConf.iWidth1      = 640;
            mConf.iHeight1     = 360;
            mConf.iFrameNum    = 812;
            break;
        default:
            ALOGE("unknown codec:%d\n", iCodec);
            break;
    }
    return 1;
}

//conformance
int EncTestCases::lastFrameTest(EncTestCaseConf &tCaseConf)
{
    encTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;

    EncTest encTest=EncTest();

    if(encTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(encTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(encTest.encode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    int check = encTest.checkLastFrame();
    if(check == 0)
    {
        printf("last frame fail\n");
    }

    if(encTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(encTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    return (check == 0) ? 0 : 1;
}
static void *multiInstanceThread(void *pvData)
{
    EncTestCases *pEncTestCases = (EncTestCases*)pvData;

    EncTest encTest=EncTest();
    unsigned int current_tid = gettid();

    printf("[tid:%d] start thread!\n", current_tid);
    if(encTest.init(pEncTestCases->mConf) != 0)
    {
        printf("[tid:%d] Fail to init!\n", current_tid);
        return NULL;
    }
    ALOGI("init ok!\n");

    if(encTest.prepare() != 0)
    {
        printf("[tid:%d] Fail to prepare!\n", current_tid);
        return NULL;
    }
    ALOGI("prepare ok!\n");

    if(encTest.encode() != 0)
    {
        printf("[tid:%d] Fail to decode\n", current_tid);
        return NULL;
    }
    ALOGI("decode ok!\n");

    if(encTest.finalize() != 0)
    {
        printf("[tid:%d] Fail to finalize\n", current_tid);
        return NULL;
    }
    ALOGI("finalize ok!\n");

    if(encTest.deInit() != 0)
    {
        printf("[tid:%d] Fail to deinit!\n", current_tid);
        return NULL;
    }

    return NULL;
}
int EncTestCases::multiInstancesTest(EncTestCaseConf &tCaseConf)
{
    const int   iNum = 2;
    pthread_t   aTid[iNum];

    encTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
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
int EncTestCases::bufferFlagLeakTest(EncTestCaseConf &tCaseConf)
{
    encTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 0;
    mConf.bBufFlagLeak = true;

    EncTest encTest=EncTest();

    if(encTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(encTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(encTest.encode() != 0)
    {//H264 may fail
        printf("Fail to decode\n");
        encTest.finalize();
        encTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(encTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(encTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
int EncTestCases::noEOSFlagTest(EncTestCaseConf &tCaseConf)
{
    encTestConfig(tCaseConf.iCodec);

    mConf.bNoEOS    = true;

    EncTest encTest=EncTest();

    if(encTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(encTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(encTest.encode() != 0)
    {//MPEG4 may fail
        printf("Fail to decode\n");
        encTest.finalize();
        encTest.deInit();
        return 1;
    }
    ALOGI("decode ok!\n");

    if(encTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(encTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }
    return 1;
}
int EncTestCases::outBufFullTest(EncTestCaseConf &tCaseConf)
{
    encTestConfig(tCaseConf.iCodec);

    mConf.iPortReconfigType = 1;
    mConf.bOutBufFull = true;

    EncTest encTest=EncTest();

    if(encTest.init(mConf) != 0)
    {
        printf("Fail to init!\n");
        return 0;
    }
    ALOGI("init ok!\n");

    if(encTest.prepare() != 0)
    {
        printf("Fail to prepare!\n");
        return 0;
    }
    ALOGI("prepare ok!\n");

    if(encTest.encode() != 0)
    {
        printf("Fail to decode\n");
        return 0;
    }
    ALOGI("decode ok!\n");

    if(encTest.finalize() != 0)
    {
        printf("Fail to finalize\n");
        return 0;
    }
    ALOGI("finalize ok!\n");

    if(encTest.deInit() != 0)
    {
        printf("Fail to deinit!\n");
        return 0;
    }

    return 1;
}
//special


