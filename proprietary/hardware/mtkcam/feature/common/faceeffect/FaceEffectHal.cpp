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
#define LOG_TAG "FaceEffectHal"

#include <cutils/log.h>
#include "include/FaceEffectHal.h"

/******************************************************************************
 *
 ******************************************************************************/
/*#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)*/

#define FUNCTION_LOG_START
#define FUNCTION_LOG_END

#define NOT_BUILD (0)
#define UNUSED(x) (void)x

#define MHAL_ASD_WORKING_BUF_SIZE       (160*120*2*11+200*1024)
#define ASD_DEFAULT_WIDTH (320)
#define ASD_DEFAULT_HEIGHT (240)
#define max(a,b)  ((a) < (b) ? (b) : (a))
#define min(a,b)  ((a) < (b) ? (a) : (b))

using namespace NSCam;
using namespace android;

#define DETECTED_FACE_NUMBER (15)
#define FACE_EFFECT_IMAGE_WIDTH (640)
#define FACE_EFFECT_IMAGE_HEIGHT (480)

// Internal function
MVOID
FaceEffectHal::
ReturnResult(Request_T &rFrame, MtkCameraFaceMetadata info, int SD_Detected, int GD_Detected, int Scene)
{
    android::String8 EFFECTHAL_KEY_COMPLETE("completed");
    sp<EffectResult> Result = new EffectResult();
    // For Test
    //info.number_of_faces = 2;
    // -
    Result->set(BasicParameters::KEY_DETECTED_SD_ENABLE, mSDEnable);
    Result->set(BasicParameters::KEY_DETECTED_FD_ENABLE, mFDEnable);
    Result->set(BasicParameters::KEY_DETECTED_GS_ENABLE, mGDEnable);
    Result->set(BasicParameters::KEY_DETECTED_ASD_ENABLE, mASDEnable);

    if(mFDEnable) {
        Result->set(BasicParameters::KEY_DETECTED_FACE_NUM, info.number_of_faces);
        #if 0
        char str[512];
        str[0] = 0;
        for(int i = 0; i < info.number_of_faces; i++) {
            if(i == 0) {
                sprintf(str, "%d,%d,%d,%d,%d,", info.faces[i].rect[0], info.faces[i].rect[1],
                                                 info.faces[i].rect[2], info.faces[i].rect[3], info.faces[i].score);
            } else {
                sprintf(str, "%s%d,%d,%d,%d,%d,", str, info.faces[i].rect[0], info.faces[i].rect[1],
                                                 info.faces[i].rect[2], info.faces[i].rect[3], info.faces[i].score);
            }
        }
        Result->set(BasicParameters::KEY_DETECTED_FACE_RESULT, str);
        #endif
        Result->setPtr(BasicParameters::KEY_DETECTED_FACE_RESULT, (void *)info.faces);
    }
    if(mSDEnable) {
        Result->set(BasicParameters::KEY_DETECTED_SMILE_RESULT, SD_Detected);
    }
    if(mGDEnable) {
        Result->set(BasicParameters::KEY_DETECTED_GESTURE_RESULT, GD_Detected);
    }
    if(mASDEnable) {
        Result->set(BasicParameters::KEY_DETECTED_SCENE, Scene);
    }
    rFrame->setRequestResult(Result);
    rFrame->mpOnRequestProcessed(rFrame->mpTag, EFFECTHAL_KEY_COMPLETE, rFrame);
}

MINT32
FaceEffectHal::
getGestureResult(MtkCameraFaceMetadata *pGestureInfo, MtkCameraFaceMetadata *pFaceInfo, int FDEnable)
{
    MINT32 GD_Result;
    if(mGDEnable) {
        if(pGestureInfo == NULL)
            return -1;
        GD_Result = mpGSHalObj->halGSGetGestureResult(pGestureInfo);
        ALOGD("Original Gesture result : %d", GD_Result);
        if(pFaceInfo == NULL || FDEnable == 0)
            return 0;
        /////////////////////////////////////////////////////////////////////
        // cpchen: filter GS results with FD results: no gesture inside face regions
        /////////////////////////////////////////////////////////////////////

        float fIntersetAreaRatio = 0.25f;
        float fMaxRelativeRatio = 3.0f;
        if(pGestureInfo->number_of_faces != 0)
        {
           int newCount = 0;
           for (int gi = 0; gi < pGestureInfo->number_of_faces; ++gi)
           {
              // gesture rectangle
              int gx0 = pGestureInfo->faces[gi].rect[0];
              int gy0 = pGestureInfo->faces[gi].rect[1];
              int gx1 = pGestureInfo->faces[gi].rect[2];
              int gy1 = pGestureInfo->faces[gi].rect[3];
              int garea = (gx1 - gx0) * (gy1 - gy0);

              bool bIsOverlap = false;
              for (int fi = 0; fi < pFaceInfo->number_of_faces; ++fi)
              {
                 // face rectangle
                 int fx0 = pFaceInfo->faces[fi].rect[0];
                 int fy0 = pFaceInfo->faces[fi].rect[1];
                 int fx1 = pFaceInfo->faces[fi].rect[2];
                 int fy1 = pFaceInfo->faces[fi].rect[3];
                 int farea = (fx1 - fx0) * (fy1 - fy0);

                 // interset rectangle
                 int ix0 = max(gx0, fx0);
                 int iy0 = max(gy0, fy0);
                 int ix1 = min(gx1, fx1);
                 int iy1 = min(gy1, fy1);
                 int iarea = 0;
                 if ((ix1 > ix0) && (iy1 > iy0))
                    iarea = (ix1 - ix0) * (iy1 - iy0);

                 // overlap determination
                 float minArea = min(garea, farea);
                 float overlapRatio = (float)iarea / minArea;
                 float relativeRatio = (float)farea / garea;

                 if (overlapRatio >= fIntersetAreaRatio)
                 {
                    bIsOverlap = true;
                    break;
                 }
              } // end of for each face rectangle

              // skip overlapped gesture rectangles, move non-overlapped gesture rectangles forward
              if (!bIsOverlap)
              {
                 pGestureInfo->faces[newCount].rect[0] = pGestureInfo->faces[gi].rect[0];
                 pGestureInfo->faces[newCount].rect[1] = pGestureInfo->faces[gi].rect[1];
                 pGestureInfo->faces[newCount].rect[2] = pGestureInfo->faces[gi].rect[2];
                 pGestureInfo->faces[newCount].rect[3] = pGestureInfo->faces[gi].rect[3];
                 pGestureInfo->faces[newCount].score = pGestureInfo->faces[gi].score;
                 pGestureInfo->faces[newCount].id = pGestureInfo->faces[gi].id;
                 pGestureInfo->faces[newCount].left_eye[0] = pGestureInfo->faces[gi].left_eye[0];
                 pGestureInfo->faces[newCount].left_eye[1] = pGestureInfo->faces[gi].left_eye[1];
                 pGestureInfo->faces[newCount].right_eye[0] = pGestureInfo->faces[gi].right_eye[0];
                 pGestureInfo->faces[newCount].right_eye[1] = pGestureInfo->faces[gi].right_eye[1];
                 pGestureInfo->faces[newCount].mouth[0] = pGestureInfo->faces[gi].mouth[0];
                 pGestureInfo->faces[newCount].mouth[1] = pGestureInfo->faces[gi].mouth[1];
                 pGestureInfo->posInfo[newCount].rop_dir = pGestureInfo->posInfo[gi].rop_dir;
                 pGestureInfo->posInfo[newCount].rip_dir = pGestureInfo->posInfo[gi].rip_dir;
                 ++newCount;
              }
           }
           // number of gesture rectangles after filtering
           pGestureInfo->number_of_faces = newCount;
           GD_Result = newCount;

           // debug message
           if (GD_Result == 0)
              ALOGD("Scenario GD: Gesture detected but filtered out by face!!!");
        }
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // cpchen: face is a prerequiste of gesture shot, no face no gesture shot
        /////////////////////////////////////////////////////////////////////
        if (pFaceInfo->number_of_faces == 0)
        {
           pGestureInfo->number_of_faces = 0;
           GD_Result = 0;

           // debug message
           ALOGD("Scenario GD: Gesture detected but no face!");
        }
        /////////////////////////////////////////////////////////////////////

        ALOGD("Scenario GD Result: %d",GD_Result );
    }
    return 0;
}

MINT32
FaceEffectHal::
updateASD(MUINT8* pRGBImg, MINT32 BufWidth, MINT32 BufHeight, MINT32 FaceNum, mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene, void *ASDInfo)
{
    MUINT32 u4Scene = 0;
    MUINT32 ASDWidth = BufWidth;
    MUINT32 ASDHeight = BufHeight;

    if (mASDWorkingBuffer == NULL || mpASDHalObj == NULL) {
        ALOGE("The ASD module is not init");
        return -1;
    }

    if(!mASDHalInited) {
        mpASDHalObj->mHalAsdInit((void*)ASDInfo, mASDWorkingBuffer, (mSensorType==SENSOR_TYPE_RAW)?0:1, ASDWidth/2, ASDHeight/2);
        mASDHalInited = 1;
    }
    mpASDHalObj->mHalAsdDecider((void*)ASDInfo, FaceNum ,Scene);

    mpASDHalObj->mHalAsdDoSceneDet((void*)pRGBImg, ASDWidth, ASDHeight);

    return 0;
}

MVOID
FaceEffectHal::
RunFaceEffect(Request_T &rFrameIn)
{
    // For Test
    if (mDoFD) {
    //if (0) {
        int RotateInfo = 0;
        MUINT8* pDstY = NULL;
    #if 0
        p3AHal = MAKE_Hal3A(getOpenId(), getNodeName());
    #endif
        #if 0
        if(p3AHal)
        {
            p3AHal->setFDEnable(true);
        }
        #endif

    /*
            char str_dstfile_0[128];
            FILE *cfptr_1;
            int read_size;
            sprintf(str_dstfile_0, "/system/data/640x480_Y_9_1.raw");
            if((cfptr_1=fopen(str_dstfile_0,"rb")) ==NULL)
            {
                printf("File could not be opened \n");
            }
            else
            {
                read_size = fread(FDImgBuffer, sizeof(unsigned char), 640*480, cfptr_1);
                fclose(cfptr_1);
            }
    */

        // + Get request frame and parameter
        int srcWidth;
        int srcHeight;
        int numFace = 0;
        int frameNo = rFrameIn->vInputFrameInfo.keyAt(0);
        sp<EffectFrameInfo> pFrameInfo = rFrameIn->vInputFrameInfo.valueAt(0);
        sp<IImageBuffer> pImage;
        pFrameInfo->getFrameBuffer(pImage);
        srcWidth = pFrameInfo->getFrameParameter()->getInt(BasicParameters::KEY_IMAGE_WIDTH);
        srcHeight = pFrameInfo->getFrameParameter()->getInt(BasicParameters::KEY_IMAGE_HEIGHT);
        mDupImage.AddrY = (MUINT8 *)pImage->getBufVA(0);
        //mDupImage.AddrU = (MUINT8 *)pImage->getBufVA(1);
        //mDupImage.AddrV = (MUINT8 *)pImage->getBufVA(2);
        mDupImage.planes = pImage->getPlaneCount();
        mDupImage.PAddrY = (MUINT8 *)pImage->getBufPA(0);
        mSDEnable = mFDSDUsed && rFrameIn->getRequestParameter()->getInt(BasicParameters::KEY_DETECTED_SD_ENABLE);
        mFDEnable = mFDSDUsed && rFrameIn->getRequestParameter()->getInt(BasicParameters::KEY_DETECTED_FD_ENABLE);
        mGDEnable = mGDUsed && rFrameIn->getRequestParameter()->getInt(BasicParameters::KEY_DETECTED_GS_ENABLE);
        mASDEnable = mASDUsed && rFrameIn->getRequestParameter()->getInt(BasicParameters::KEY_DETECTED_ASD_ENABLE);
        RotateInfo = rFrameIn->getRequestParameter()->getInt(BasicParameters::KEY_ROTATION);
        // -
        if(mFDEnable) {
            if(mImageWidth == 0 || mImageHeight == 0) {
                mpFDHalObj->halFDInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize, 1, mSDEnable);
            } else if(mImageWidth != srcWidth || mImageHeight != srcHeight || mSDEnable != mPrevSD) {
                mpFDHalObj->halFDUninit();
                mpFDHalObj->halFDInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize, 1, mSDEnable);
            }
            mPrevSD = mSDEnable;

            mpFDHalObj->halFTBufferCreate((MUINT8 *)mFTImgBuffer, (MUINT8 *)mDupImage.AddrY, mDupImage.planes, srcWidth, srcHeight);
            if(mDupImage.planes == 1) {
                pDstY = mPureYBuf;
                mpFDHalObj->halFDYUYV2ExtractY(pDstY, (MUINT8 *)mDupImage.AddrY, srcWidth, srcHeight);
            } else {
                pDstY = (MUINT8 *)mDupImage.AddrY;
            }

            // Do FD
            struct FD_Frame_Parameters Param;
            Param.pScaleImages = NULL;
            Param.pRGB565Image = (MUINT8 *)mFTImgBuffer;
            Param.pPureYImage  = pDstY;
            Param.pImageBufferPhyP0 = mDupImage.PAddrY;
            Param.pImageBufferPhyP1 = NULL;
            Param.pImageBufferPhyP2 = NULL;
            Param.Rotation_Info = RotateInfo;
            Param.SDEnable = mSDEnable;
            Param.AEStable = 0;
            mpFDHalObj->halFDDo(Param);

            ALOGD("halFDDo Out.");

            {
                // reset face number
                mFaceInfo.number_of_faces = 0;

                numFace = mpFDHalObj->halFDGetFaceResult(&mFaceInfo);

                ALOGD("NumFace = %d, ", numFace);

                if(mSDEnable) {
                    mSD_Result = mpFDHalObj->halSDGetSmileResult();
                    ALOGD("Smile Result:%d", mSD_Result);
                }
            }
        }

        if(mGDEnable && pDstY != NULL) {
            if(mImageWidth == 0 || mImageHeight == 0) {
                mpGSHalObj->halGSInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize);
            } else if(mImageWidth != srcWidth || mImageHeight != srcHeight) {
                mpGSHalObj->halGSUninit();
                mpGSHalObj->halGSInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize);
            }
            if(pDstY == NULL) {
                // TBD
            }
            if(!mFDEnable || mFaceInfo.number_of_faces != 0) {
                mpGSHalObj->halGSDo(pDstY, RotateInfo);
                getGestureResult(&mGestureInfo, &mFaceInfo, mFDEnable);
            }
        }

        if(mASDEnable && mFTImgBuffer != NULL) {
            MINT32 ASDWidth = ASD_DEFAULT_WIDTH;
            MINT32 ASDHeight;
            if ( srcWidth != 0 ) {
                ASDHeight = ASDWidth * srcHeight / srcWidth;
            } else {
                ASDHeight = ASD_DEFAULT_HEIGHT;
            }
            void *ASDInfo;
            if(mFTImgBuffer == NULL) {
                // TBD
            }
            ASDInfo = pFrameInfo->getFrameParameter()->getPtr(BasicParameters::KEY_DETECTED_ASD_3A_INFO);
            updateASD((MUINT8 *)mFTImgBuffer, ASDWidth,ASDHeight, mFaceInfo.number_of_faces, mSceneCur, ASDInfo);
        }

        mImageWidth = srcWidth;
        mImageHeight = srcHeight;
    }
    ReturnResult(rFrameIn, mFaceInfo, mSD_Result, mGestureInfo.number_of_faces, (int)mSceneCur);
}

MBOOL
FaceEffectHal::
onDequeue(Request_T &rFrameIn)
{
    Mutex::Autolock _l(mFrameQueueLock);
    //
    //  Wait until the queue is not empty or this thread will exit.

    while ( mInputQ.empty() && !mStop )
    {
        status_t status = mInputQueueCond.wait(mFrameQueueLock);
        if  ( OK != status ) {
            ALOGW(
                "wait status:%d:%s, mInputQ.size:%zu, mStop:%d",
                status, ::strerror(-status), mInputQ.size(), mStop
            );
        }
    }

    //
    if  ( mStop ) {
        ALOGD("[StopFD] mInputQ.size:%zu", mInputQ.size());
        return MFALSE;
    }
    mDoFD = true;
    /*
    if(mInputQ.size() > 1) {
        Request_T pFrame = *mInputQ.begin();
        android::String8 EFFECTHAL_KEY_ALLOW_SKIP("allow-skip");
        int bskip = 0;
        bskip = pFrame->getRequestParameter()->getInt(EFFECTHAL_KEY_ALLOW_SKIP.string());
        if(bskip) {
            mDoFD = false;
        }
        pFrame = NULL;
    }
    */
    //
    //  Here the queue is not empty, take the first request from the queue.
    rFrameIn = *mInputQ.begin();
    mInputQ.erase(mInputQ.begin());
    return MTRUE;
}

MBOOL
FaceEffectHal::
onFlush()
{
    FUNCTION_LOG_START;
    Mutex::Autolock _l(mFrameQueueLock);
    mInputQ.clear();
    FUNCTION_LOG_END;
    return true;
}

MVOID*
FaceEffectHal::
onThreadLoop(MVOID* arg)
{
    FaceEffectHal *_FaceHal = reinterpret_cast<FaceEffectHal*>(arg);
    MBOOL ret;
    Request_T request;
    while(1) {
        ret = _FaceHal->onDequeue(request);
        if(!ret) {
            _FaceHal->onFlush();
            break;
        }
        _FaceHal->RunFaceEffect(request);
    }
    return NULL;
}

// FaceEffectHal interface
FaceEffectHal::
~FaceEffectHal()
{
    ALOGD("WillDBG delete FaceEffect");
}

status_t
FaceEffectHal::
initImpl()
{
    FUNCTION_LOG_START;

    mpFDHalObj = NULL;
    mFDWorkingBuffer = NULL;
    mpGSHalObj = NULL;
    mASDWorkingBuffer = NULL;

    mFDSDUsed = 0;
    mGDUsed = 0;
    mASDUsed = 0;

    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceEffectHal::
uninitImpl()
{
    FUNCTION_LOG_START;
    mFDSDUsed = 0;
    mGDUsed = 0;
    mASDUsed = 0;
    FUNCTION_LOG_END;
    return OK;
}

status_t
FaceEffectHal::
setParametersImpl(android::sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    mFDSDUsed = parameter->getInt(BasicParameters::KEY_DETECTED_USE_FD);
    mGDUsed = parameter->getInt(BasicParameters::KEY_DETECTED_USE_GS);
    mASDUsed = parameter->getInt(BasicParameters::KEY_DETECTED_USE_ASD);
    FUNCTION_LOG_END;
    return OK;
}

//non-blocking
status_t
FaceEffectHal::
prepareImpl()
{
    EffectResult result;
    FUNCTION_LOG_START;

    FUNCTION_LOG_END;
    prepareDone(result, OK);
    return OK;
}

bool
FaceEffectHal::
allParameterConfigured()
{
    FUNCTION_LOG_START;
    mImageWidth = 0;
    mImageHeight = 0;

    mFaceInfo.number_of_faces = 0;
    mSD_Result = 0;
    if (mFDSDUsed) {

        mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
        if(mpFDHalObj == NULL) {
            ALOGD("Create FD hal instance failed!!");
            return INVALID_OPERATION;
        }

        mFDWorkingBufferSize = 20971520; //20M: 1024*1024*20
        mFDWorkingBuffer = new unsigned char[mFDWorkingBufferSize];

        mPureYBuf = new unsigned char[FACE_EFFECT_IMAGE_WIDTH*FACE_EFFECT_IMAGE_HEIGHT];
        mFTImgBuffer = new unsigned char[FACE_EFFECT_IMAGE_WIDTH*FACE_EFFECT_IMAGE_HEIGHT*2];
    }

    mGestureInfo.number_of_faces = 0;
    if (mGDUsed) {
        if(mFDWorkingBuffer == NULL) {
            mFDWorkingBufferSize = 20971520; //20M: 1024*1024*20
            mFDWorkingBuffer = new unsigned char[mFDWorkingBufferSize];
        }

        mpGSHalObj = halGSBase::createInstance(HAL_GS_OBJ_SW);
    }
    if (mASDUsed) {
        mSensorType = SENSOR_TYPE_RAW;
        mASDWorkingBuffer = (MUINT8*)malloc(MHAL_ASD_WORKING_BUF_SIZE);
        mASDWorkingBufferSize = MHAL_ASD_WORKING_BUF_SIZE;
        mpASDHalObj = halASDBase::createInstance(HAL_ASD_OBJ_AUTO);
        mASDHalInited = 0;
        mSceneCur = mhal_ASD_DECIDER_UI_AUTO;
    }
    FUNCTION_LOG_END;
    return true;
}

status_t
FaceEffectHal::
releaseImpl()
{
    FUNCTION_LOG_START;
    // face
    if(mpFDHalObj) {
        mpFDHalObj->halFDUninit();
        mpFDHalObj->destroyInstance();
        mpFDHalObj = NULL;
    }
    if(mFDWorkingBuffer) {
        delete mFDWorkingBuffer;
        mFDWorkingBuffer = NULL;
    }
    if(mPureYBuf) {
        delete mPureYBuf;
        mPureYBuf = NULL;
    }
    if(mFTImgBuffer) {
        delete mFTImgBuffer;
        mFTImgBuffer = NULL;
    }

    mFaceInfo.number_of_faces = 0;
    mSD_Result = 0;


    // gesture
    mGestureInfo.number_of_faces = 0;
    if(mpGSHalObj) {
        mpGSHalObj->halGSUninit();
        mpGSHalObj->destroyInstance();
        mpGSHalObj = NULL;
    }
    // ASD
    if (mpASDHalObj != NULL)
    {
        mpASDHalObj->mHalAsdUnInit();
        mpASDHalObj->destroyInstance();
        mpASDHalObj = NULL;
    }
    if (mASDWorkingBuffer != NULL) {
        free(mASDWorkingBuffer);
        mASDWorkingBuffer = NULL;
    }
    mASDWorkingBufferSize = 0;
    mASDHalInited = 0;
    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;

    FUNCTION_LOG_END;
    return OK;
}

status_t
FaceEffectHal::
getNameVersionImpl(EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;
    UNUSED(nameVersion);
    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceEffectHal::
getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    UNUSED(inputParam);
    EffectCaptureRequirement result;
    char str[32];
    sprintf(str, "%dx%d", FACE_EFFECT_IMAGE_WIDTH, FACE_EFFECT_IMAGE_HEIGHT);
    result.set(BasicParameters::KEY_PICTURE_SIZE, str);
    result.set(BasicParameters::KEY_PICTURE_FORMAT, eImgFmt_YUY2);
    //result.set("allow-skip", 1);
    result.dump();
    requirements.push_back(result);
    FUNCTION_LOG_END;
    return OK;
}

MBOOL
FaceEffectHal::
isParameterValid(const char *param)
{
    FUNCTION_LOG_START;
    UNUSED(param);
    FUNCTION_LOG_END;
    return true;
}


status_t
FaceEffectHal::
setParameterImpl(android::String8 &key, android::String8 &object)
{
    FUNCTION_LOG_START;
    UNUSED(key);
    UNUSED(object);
    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceEffectHal::
startImpl(uint64_t *uid)
{
    FUNCTION_LOG_START;
    EffectResult result;
    UNUSED(uid);

    mSDEnable = 0;
    mPrevSD = 0;
    // start thread
    mStop = false;
    pthread_create(&mFaceThread, NULL, onThreadLoop, this);

    result.set("onCompleted", 1);

    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceEffectHal::
abortImpl(EffectResult &result, EffectParameter const *parameter)
{
    FUNCTION_LOG_START;

    UNUSED(result);
    UNUSED(parameter);
    {
        Mutex::Autolock _l(mFrameQueueLock);
        mStop = true;
        mInputQueueCond.signal();
    }
    pthread_join(mFaceThread, NULL);

    mImageWidth = 0;
    mImageHeight = 0;
    mPrevSD = mSDEnable = 0;
    // face
    mpFDHalObj->halFDUninit();
    mFaceInfo.number_of_faces = 0;
    mSD_Result = 0;
    // Gesture
    mpGSHalObj->halGSUninit();
    mGestureInfo.number_of_faces = 0;
    // ASD
    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;

    ALOGD("abort effect dones");
    FUNCTION_LOG_END;
    return OK;
}
//non-blocking
status_t
FaceEffectHal::
updateEffectRequestImpl(const android::sp<EffectRequest> request)
{
    FUNCTION_LOG_START;
    if(request == NULL){
        ALOGE("[updateEffectRequestImpl] request is NULL\n");
        return BAD_VALUE;
    }
    {
        Mutex::Autolock _l(mFrameQueueLock);

        mInputQ.push_back(request);
        mInputQueueCond.broadcast();
    }
    return OK;
    FUNCTION_LOG_END;
}

