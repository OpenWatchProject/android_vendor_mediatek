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
#define LOG_TAG "vmgr_test"
//
#include <cutils/properties.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam3/feature/FaceDetection/fd_hal_base.h>
#include <faces.h>

#include "AEStable.h"
#include "Rotation.h"

using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        printf("[%s] " fmt, __FUNCTION__, ##arg);
#define MY_LOGD(fmt, arg...)        printf("[%s] " fmt, __FUNCTION__, ##arg);
#define MY_LOGI(fmt, arg...)        printf("[%s] " fmt, __FUNCTION__, ##arg);
#define MY_LOGW(fmt, arg...)        printf("[%s] " fmt, __FUNCTION__, ##arg);
#define MY_LOGE(fmt, arg...)        printf("[%s] " fmt, __FUNCTION__, ##arg);
#endif
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");

IImageBufferAllocator*          mAllocator;
IImageBuffer*                   mpImg;

halFDBase*                      mpFDHalObj;
MtkCameraFaceMetadata*          mpDetectedFaces;
MUINT8*                         mpExtractYBuffer;
MUINT8*                         mpFDWorkingBuffer;
MUINT32                         mFDWorkingBufferSize;

int createAllBuffer(int w,int h)
{
    int size = w*h*2;
    mAllocator = IImageBufferAllocator::getInstance();
    IImageBufferAllocator::ImgParam imgParam(size,0);
    mpImg = mAllocator->alloc("testFDBuffer", imgParam);
    if ( mpImg == 0 )
    {
        MY_LOGE("FDbuffer get NULL Buffer\n");
        return -1;
    }
    if ( !mpImg->lockBuf( "testFDBuffer", (eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK)) )
    {
        MY_LOGE("lock Buffer failed\n");
        mpImg->unlockBuf("FDBuffer");
        mAllocator->free(mpImg);
        mpImg = NULL;
        return -1;
    }

    mpImg->syncCache(eCACHECTRL_INVALID);

    mpExtractYBuffer = new unsigned char[1920*1080];
    mFDWorkingBufferSize = 2621440; //25M: 1024*1024*25
    mpFDWorkingBuffer = new unsigned char[mFDWorkingBufferSize];

    mpDetectedFaces = new MtkCameraFaceMetadata;
    mpDetectedFaces->number_of_faces = 0;
    mpDetectedFaces->ImgWidth = 640; // init value
    mpDetectedFaces->ImgHeight = 480; // init value

    return 0;
}

int destroyAllBuffer()
{
    mpImg->unlockBuf("testFDBuffer");
    mAllocator->free(mpImg);
    mpImg = NULL;
    mAllocator = NULL;

    delete mpExtractYBuffer;
    mpExtractYBuffer = NULL;
    delete mpFDWorkingBuffer;
    mpFDWorkingBuffer = NULL;
    delete mpDetectedFaces;
    mpDetectedFaces = NULL;
    return 0;
}

int initFD(int w, int h)
{
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    mpFDHalObj->halFDInit(w, h, mpFDWorkingBuffer, mFDWorkingBufferSize, 1, 0);
    return 0;
}

int uninitFD()
{
    if (mpFDHalObj != NULL)
    {
      mpFDHalObj->halFDUninit();
      mpFDHalObj->destroyInstance();
      mpFDHalObj = NULL;
    }
    return 0;
}

int doFD(char *prefix, char *postfix, int w, int h)
{
    int i = 0;
    char YUVName[256];
    FILE* pFp;
    int numFace = 0;
    FILE* pAEFile;
    FILE* pRotFile;
    pAEFile = fopen("/sdcard/aedata.txt","r");
    pRotFile = fopen("/sdcard/rotdata.txt","r");
    while(1)
    {
        // read test image
        sprintf(YUVName, "/sdcard/FDTest/%s%04d%s.raw", prefix, i+1, postfix);
        pFp = fopen(YUVName, "rb");
        if (NULL == pFp) {
            return 0;
        }
        fread((unsigned char *)(mpImg->getBufVA(0)), sizeof(MUINT8), w*h*2, pFp);
        fclose(pFp);
        pFp = NULL;
        mpImg->syncCache(eCACHECTRL_FLUSH);
        struct FD_Frame_Parameters Param;
        Param.pScaleImages = NULL;
        // set ot frame
        if( mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW42 && mpFDHalObj->halFDGetVersion() != HAL_FD_VER_HW37 ) {
            MY_LOGE("not support FD version : %d", mpFDHalObj->halFDGetVersion());
            return -1;
        }
        else {
            Param.pRGB565Image = (MUINT8 *)mpImg->getBufVA(0);
        }
        if(mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW40)
        { // HW FD4.0 don't need extract Y buffer
            mpFDHalObj->halFDYUYV2ExtractY(mpExtractYBuffer, (MUINT8*)(mpImg->getBufVA(0)), w, h);
            Param.pPureYImage  = (unsigned char *)mpExtractYBuffer;
        }
        else
        {
            Param.pPureYImage  = (unsigned char *)mpImg->getBufVA(0);
        }
        Param.pImageBufferVirtual  = (unsigned char *)mpImg->getBufVA(0);
        Param.pImageBufferPhyP0 = (unsigned char *)mpImg->getBufPA(0);
        Param.pImageBufferPhyP1 = NULL;
        Param.pImageBufferPhyP2 = NULL;
        // set rotation info
        if (pAEFile != NULL)
        {
            fscanf(pRotFile, "%d", &(Param.Rotation_Info));
        }
        else
        {
            Param.Rotation_Info = mRotation_Info[i];
        }
        Param.SDEnable = 0;
        // set AE stable info
        if (pAEFile != NULL)
        {
            fscanf(pAEFile, "%d", &(Param.AEStable));
        }
        else
        {
            Param.AEStable = mAEStable[i];
        }

        MY_LOGD("frame[%d] : mRotation_Info = %d", i, mRotation_Info[i]);
        mpFDHalObj->halFDDo(Param);
        numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
        MY_LOGD("frame[%d] : numFace = %d", i, numFace);
        i++;
    }
    fclose(pAEFile);
    fclose(pRotFile);
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    int w,h;
    char* prefix = "";
    char* postfix = "";
    if(argc >= 3)
    {
        w = atoi(argv[1]);
        h = atoi(argv[2]);
    }
    if (argc >= 4)
    {
        prefix = argv[3];
    }
    if (argc >= 5)
    {
        postfix = argv[4];
    }
    MY_LOGD("[FD] start of test\n");
    createAllBuffer(w,h);

    initFD(w, h);

    doFD(prefix, postfix, w, h);

    uninitFD();

    destroyAllBuffer();

    MY_LOGD("[FD] end of test\n");
    return 0;
}
