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

#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "FDClient.h"
//

using namespace NSCamClient;
using namespace NSFDClient;
using namespace NSCam::Utils;
//

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
doFD(ImgBufQueNode const& rQueNode, bool &rIsDetected_FD, bool &rIsDetected_SD, bool rDoSD, bool &rIsDetected_GD, bool rDoGD)
{
    bool ret = true;
    MINT32 SD_Result = 0, GD_Result = 0;
    MINT32 numFace = 0;

    MUINT32 srcWidth=0,  srcHeight=0;
    MINT32 pv_srcWidth =0, pv_srcHeight = 0;
    unsigned char *VirAddr, *PhyAddr;
    mpParamsMgr->getPreviewSize(&pv_srcWidth, &pv_srcHeight);
    srcWidth = mFDBufferWidth;
    if(pv_srcWidth != 0)
        srcHeight = srcWidth * pv_srcHeight / pv_srcWidth;
    else
        srcHeight = 480;

    if (mPadding_w != 0)
    {
        unsigned char *startAddr = (unsigned char *)mFDPaddingImg->getVirAddr() + mPadding_w*2 + mPadding_h*srcWidth*2;
        memcpy(startAddr, rQueNode.getImgBuf()->getVirAddr(), srcWidth*(srcHeight - (mPadding_h*2))*2);
        VirAddr = (unsigned char *)(mFDPaddingImg->getVirAddr());
        PhyAddr = (unsigned char *)(mFDPaddingImg->getPhyAddr());
        mFDPaddingImg->syncCache();
        //MY_LOGD("WillFD2 startAddr : %p, virAddr : %p", startAddr, VirAddr);
    }
    else
    {
        VirAddr = (unsigned char *)(rQueNode.getImgBuf()->getVirAddr());
        PhyAddr = (unsigned char *)(rQueNode.getImgBuf()->getPhyAddr());
    }

    // HW FD4.2 don't need RGB buffer for tracking
    if( mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW42 && mpFDHalObj->halFDGetVersion() != HAL_FD_VER_HW37 ) {
        mpFDHalObj->halFTBufferCreateAsync( mpDDPBuffer, (MUINT8*)VirAddr,
                                            mFDBufferPlanes, srcWidth, srcHeight);
    }

    struct FD_Frame_Parameters Param;
    Param.pScaleImages = NULL;
    if( mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW42 && mpFDHalObj->halFDGetVersion() != HAL_FD_VER_HW37 ) {
        Param.pRGB565Image = (MUINT8 *)mpDDPBuffer;
    }
    else {
        Param.pRGB565Image = (MUINT8 *)VirAddr;
    }
    Param.pPureYImage  = (unsigned char *)VirAddr;
    Param.pImageBufferVirtual  = (unsigned char *)VirAddr;
    Param.pImageBufferPhyP0 = (unsigned char *)PhyAddr;
    Param.pImageBufferPhyP1 = NULL;
    Param.pImageBufferPhyP2 = NULL;
    Param.Rotation_Info = mRotation_Info;
    Param.SDEnable = rDoSD;
    Param.AEStable = mAEStable;
    Param.padding_w = mPadding_w;
    Param.padding_h = mPadding_h;
    if(mFDBufferPlanes == 1)
    {
        if(rDoSD == 1 || rDoGD == 1 || mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW40)
        {
            mpFDHalObj->halFDYUYV2ExtractY(mpExtractYBuffer, (MUINT8*)(VirAddr), srcWidth, srcHeight);
        }
        Param.pPureYImage  = (unsigned char *)mpExtractYBuffer;
    }
    mpFDHalObj->halFDDo(Param);

    if( NULL != mpDetectedFaces )
        numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
    else
        MY_LOGW("Get FD Result Fail!");

    mpDetectedFaces->timestamp = rQueNode.getImgBuf()->getTimestamp();

    if(rDoGD == 1 && mpGDHalObj != NULL)
    {
        if(mFDBufferPlanes == 1)
            mpGDHalObj->halGSDo(mpExtractYBuffer, mRotation_Info);
        else
            mpGDHalObj->halGSDo((MUINT8*)(rQueNode.getImgBuf()->getVirAddr()), mRotation_Info);
    }

    if ( mFakeFaceEn )
    {
        mpDetectedFaces->number_of_faces = 1;
        numFace = 1;
        // set a width and height = 80 pixel fake face
        mpDetectedFaces->faces[0].rect[0] = 0;
        mpDetectedFaces->faces[0].rect[1] = 0;
        mpDetectedFaces->faces[0].rect[2] = ((80*2000) / srcWidth);
        mpDetectedFaces->faces[0].rect[3] = ((80*2000) / srcHeight);
        mpDetectedFaces->faces[0].score = 100;
    }

//********************************************************************************************//
    MY_LOGD("Scenario FD Num: %d",numFace );
    if(numFace>0)
        rIsDetected_FD = 1;
    else
        rIsDetected_FD = 0;

    SD_Result = mpFDHalObj->halSDGetSmileResult();

    if(SD_Result>0)
        rIsDetected_SD = 1;
    else
        rIsDetected_SD = 0;

    if(rDoGD == 1 && mpGDHalObj != NULL)
    {
        if( NULL != mpDetectedGestures )
            GD_Result = mpGDHalObj->halGSGetGestureResult(mpDetectedGestures);
        MY_LOGD("GD_Result : %d", GD_Result);
        /////////////////////////////////////////////////////////////////////
        // cpchen: filter GS results with FD results: no gesture inside face regions
        /////////////////////////////////////////////////////////////////////
        bool bEnableGSFilterWithFD = true;
        //bool bEnableGSFilterWithFD = false;
        float fIntersetAreaRatio = 0.35f;
        float fMaxRelativeRatio = 3.0f;
        if(mGestureRatio != 0) {
            fIntersetAreaRatio = ((float)(mGestureRatio))/100;
            MY_LOGD("debug GS filter ratio : %.3f", fIntersetAreaRatio);
        }

        if( bEnableGSFilterWithFD )
        {
           int newCount = 0;
           for (int gi = 0; gi < mpDetectedGestures->number_of_faces; ++gi)
           {
              // gesture rectangle
              int gx0 = mpDetectedGestures->faces[gi].rect[0];
              int gy0 = mpDetectedGestures->faces[gi].rect[1];
              int gx1 = mpDetectedGestures->faces[gi].rect[2];
              int gy1 = mpDetectedGestures->faces[gi].rect[3];
              int garea = (gx1 - gx0) * (gy1 - gy0);

              bool bIsOverlap = false;
              for (int fi = 0; fi < mpDetectedFaces->number_of_faces; ++fi)
              {
                 // face rectangle
                 int fx0 = mpDetectedFaces->faces[fi].rect[0];
                 int fy0 = mpDetectedFaces->faces[fi].rect[1];
                 int fx1 = mpDetectedFaces->faces[fi].rect[2];
                 int fy1 = mpDetectedFaces->faces[fi].rect[3];
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
    //             if ( overlapRatio >= fIntersetAreaRatio &&
    //                   relativeRatio <= fMaxRelativeRatio && relativeRatio >= (1.0 / fMaxRelativeRatio) )
                 if (overlapRatio >= fIntersetAreaRatio)
                 {
                    MY_LOGD("Gesture overlap with Face: (%d,%d,%d,%d) + (%d,%d,%d,%d) = (%d,%d,%d,%d)\n", gx0, gy0, gx1, gy1, fx0, fy0, fx1, fy1, ix0, iy0, ix1, iy1);
                    MY_LOGD("Gesture overlap ratio = %.3f, area = %d, minArea = %.0f\n", overlapRatio, iarea, minArea);
                    bIsOverlap = true;
                    break;
                 }
              } // end of for each face rectangle

              // skip overlapped gesture rectangles, move non-overlapped gesture rectangles forward
              if (!bIsOverlap)
              {
                 mpDetectedGestures->faces[newCount].rect[0] = mpDetectedGestures->faces[gi].rect[0];
                 mpDetectedGestures->faces[newCount].rect[1] = mpDetectedGestures->faces[gi].rect[1];
                 mpDetectedGestures->faces[newCount].rect[2] = mpDetectedGestures->faces[gi].rect[2];
                 mpDetectedGestures->faces[newCount].rect[3] = mpDetectedGestures->faces[gi].rect[3];
                 mpDetectedGestures->faces[newCount].score = mpDetectedGestures->faces[gi].score;
                 mpDetectedGestures->faces[newCount].id = mpDetectedGestures->faces[gi].id;
                 mpDetectedGestures->faces[newCount].left_eye[0] = mpDetectedGestures->faces[gi].left_eye[0];
                 mpDetectedGestures->faces[newCount].left_eye[1] = mpDetectedGestures->faces[gi].left_eye[1];
                 mpDetectedGestures->faces[newCount].right_eye[0] = mpDetectedGestures->faces[gi].right_eye[0];
                 mpDetectedGestures->faces[newCount].right_eye[1] = mpDetectedGestures->faces[gi].right_eye[1];
                 mpDetectedGestures->faces[newCount].mouth[0] = mpDetectedGestures->faces[gi].mouth[0];
                 mpDetectedGestures->faces[newCount].mouth[1] = mpDetectedGestures->faces[gi].mouth[1];
                 mpDetectedGestures->posInfo[newCount].rop_dir = mpDetectedGestures->posInfo[gi].rop_dir;
                 mpDetectedGestures->posInfo[newCount].rip_dir = mpDetectedGestures->posInfo[gi].rip_dir;
                 ++newCount;
              }
           }
           // number of gesture rectangles after filtering
           mpDetectedGestures->number_of_faces = newCount;
           GD_Result = newCount;

           // debug message
           if (GD_Result == 0)
              MY_LOGD("Scenario GD: Gesture detected but filtered out by face!!!");
        }
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // cpchen: face is a prerequiste of gesture shot, no face no gesture shot
        /////////////////////////////////////////////////////////////////////
        bool bEnableGSPrerequisteWithFD = true;
        if (bEnableGSPrerequisteWithFD && mpDetectedFaces->number_of_faces == 0)
        {
           mpDetectedGestures->number_of_faces = 0;
           GD_Result = 0;

           // debug message
           MY_LOGD("Scenario GD: Gesture detected but no face!");
        }
        /////////////////////////////////////////////////////////////////////

        MY_LOGD("Scenario GD Result: %d",GD_Result );
        if(GD_Result>0)
            rIsDetected_GD = 1;
       else
            rIsDetected_GD = 0;
    }

    return ret;
}

