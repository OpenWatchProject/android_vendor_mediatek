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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <sys/stat.h>
#include <plugin/PipelinePluginType.h>
#include "MTKFaceBeauty.h"
#include "MTKFaceBeautyErrCode.h"
#include "MTKFaceBeautyType.h"
#include "BufferUtils.h"
#include "LogUtils.h"
#include "bmp.h"
#include "crz.h"
#include "global.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "mtk/mtk_feature_type.h"

using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;

#define LOG_TAG "PostAlgo/FBProviderImpl"
#define INPUT_YUV420
#define YUV_input
//#define DEBUG_FB
//#define DEBUG_FB_FACE_RECT
/******************************************************************************
 *
 ******************************************************************************/
class FBProviderImpl : public YuvPlugin::IProvider
{
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;

    public:

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        FUNCTION_IN;
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
        mOpenid = iOpenId;
        FUNCTION_OUT;
    }

    virtual const Property& property()
    {
        FUNCTION_IN;
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK FB";
            prop.mFeatures = MTK_FEATURE_FB;
            prop.mInPlace = MFALSE;
            prop.mFaceData = eFD_Current;
            prop.mPosition = 0;
            inited = true;
        }
        FUNCTION_OUT;
        return prop;
    };

    virtual MERROR negotiate(Selection& sel)
    {
        FUNCTION_IN;
        sel.mIBufferFull.setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_YV12)
            .addAcceptedSize(eImgSize_Full);
        sel.mOBufferFull.setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_YV12)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataDynamic.setRequired(MFALSE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MFALSE);
        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MFALSE);

        FUNCTION_OUT;
        return OK;
    };

    virtual void init()
    {
        FUNCTION_IN;
        pFaceBeautyInterface = MTKFaceBeauty::createInstance(DRV_FACEBEAUTY_OBJ_SW);
        FUNCTION_OUT;
    };

    virtual MERROR process(RequestPtr pRequest,
            RequestCallbackPtr pCallback = nullptr)
    {
        MY_LOGD("process FBProviderImpl >>>>>>>>>>>>>>>>>>>");
        FUNCTION_IN;
        IImageBuffer *in = NULL, *out = NULL;
        IMetadata *pIMetataHAL = NULL;

        if (pRequest->mIBufferFull != nullptr) {
            in = pRequest->mIBufferFull->acquire();
        }

        if (pRequest->mOBufferFull != nullptr) {
            out = pRequest->mOBufferFull->acquire();
            //MY_LOGD("[OUT] Full image VA: 0x%p", pImgBuffer->getBufVA(0));
        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetadata *meta = pRequest->mIMetadataDynamic->acquire();
            if (meta != NULL)
                MY_LOGD("[IN] Dynamic metadata count: %d", meta->count());
            else
                MY_LOGD("[IN] Dynamic metadata empty");
        }

        if (pRequest->mIMetadataHal != nullptr) {
            pIMetataHAL = pRequest->mIMetadataHal->acquire();
            if (pIMetataHAL != NULL)
                MY_LOGD("[IN] HAL metadata count: %d", pIMetataHAL->count());
            else
                MY_LOGD("[IN] HAL metadata empty");
        }

        if (in != NULL && out != NULL)
        {
            IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();

            //pFaceBeautyInterface = MTKFaceBeauty::createInstance(DRV_FACEBEAUTY_OBJ_SW);
            MTKFaceBeautyEnvInfo FaceBeautyEnvInfo;
            MTKFaceBeautyProcInfo FaceBeautyProcInfo;
            MTKFaceBeautyResultInfo FaceBeautyResultInfo;
            MTKFaceBeautyTuningPara FaceBeautyTuningInfo;
            MTKFaceBeautyGetProcInfo FaceBeautyGetProcInfo;
            MSize inSize = in->getImgSize();
            gImageWidth = inSize.w;
            gImageHeight = inSize.h;
            MY_LOGD("process gImageWidth = %d, gImageHeight = %d", gImageWidth, gImageHeight);
            gImageNrDsWidth = gImageWidth / 2;
            gImageNrDsHeight = gImageHeight / 2;
            //gImageDsWidth = 640;
            gImageDsHeight = (inSize.h * gImageDsWidth) / inSize.w;

            FaceBeautyProcInfo.Step1SrcImgWidth = gImageNrDsWidth;
            FaceBeautyProcInfo.Step1SrcImgHeight = gImageNrDsHeight;
            FaceBeautyProcInfo.Step2SrcImgWidth = gImageDsWidth;
            FaceBeautyProcInfo.Step2SrcImgHeight = gImageDsHeight;
            FaceBeautyProcInfo.SrcImgWidth = gImageWidth;
            FaceBeautyProcInfo.SrcImgHeight = gImageHeight;
            //FaceBeautyProcInfo.ExtremeMode = 0;
            FaceBeautyProcInfo.ExtremeMode = 1;
            FaceBeautyProcInfo.PMode = 0;
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_PROC_INFO, &FaceBeautyProcInfo, &FaceBeautyGetProcInfo);

            WorkingBuffer = new MUINT8[FaceBeautyGetProcInfo.WorkingBufferSize];
            WorkingBufferSize =  FaceBeautyGetProcInfo.WorkingBufferSize;
            // Set Environment Info into FB Driver

            FaceBeautyEnvInfo.Step1SrcImgWidth = gImageNrDsWidth;
            FaceBeautyEnvInfo.Step1SrcImgHeight = gImageNrDsHeight;
            FaceBeautyEnvInfo.Step2SrcImgWidth = gImageDsWidth;
            FaceBeautyEnvInfo.Step2SrcImgHeight = gImageDsHeight;
            FaceBeautyEnvInfo.SrcImgWidth = gImageWidth;
            FaceBeautyEnvInfo.SrcImgHeight = gImageHeight;
            FaceBeautyEnvInfo.FDWidth = gFDWidth;
            gFDHeight = (gImageHeight*gFDWidth)/gImageWidth;
            FaceBeautyEnvInfo.FDHeight = gFDHeight;
            FaceBeautyEnvInfo.SrcImgFormat = MTKFACEBEAUTY_IMAGE_YV12;
            FaceBeautyEnvInfo.STEP1_ENABLE = true;
            FaceBeautyEnvInfo.pWorkingBufAddr = (void*)WorkingBuffer;
            FaceBeautyEnvInfo.WorkingBufSize = WorkingBufferSize;

            FaceBeautyEnvInfo.pTuningPara = &FaceBeautyTuningInfo;
            MINT32 value = 0;
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_SMOOTH, value);
            MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_SMOOTH = %d", value);
            FaceBeautyEnvInfo.pTuningPara->SmoothLevel = value;

            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_BRIGHT, value);
            MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_BRIGHT = %d", value);
            FaceBeautyEnvInfo.pTuningPara->BrightLevel = value;

            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE, value);
            MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE = %d", value);
            FaceBeautyEnvInfo.pTuningPara->EnlargeEyeLevel = value;

            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE, value);
            MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE = %d", value);
            FaceBeautyEnvInfo.pTuningPara->SlimFaceLevel = value;

            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_RUDDY, value);
            MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_RUDDY = %d", value);
            FaceBeautyEnvInfo.pTuningPara->RuddyLevel = value;

            FaceBeautyEnvInfo.pTuningPara->WarpFaceNum = 1;//gWarpFaceNum;
            FaceBeautyEnvInfo.pTuningPara->MinFaceRatio = 12;
            FaceBeautyEnvInfo.pTuningPara->AlignTH1 = -10000;
            FaceBeautyEnvInfo.pTuningPara->AlignTH2 = -20;
            FaceBeautyEnvInfo.pTuningPara->ContrastLevel = 12;//gContrastLevel;
            Retcode = pFaceBeautyInterface->FaceBeautyInit(&FaceBeautyEnvInfo, 0);
            MY_LOGD("FaceBeautyInit<<<<<<<<<<<<<<");

            ////////////////////////////////////////////////////////////
            ///////////////////////// Step1 ////////////////////////////
            MY_LOGD("process Step1 >>>>>>>>>>>>>");
                ImageNrDs420WorkingBuffer = BufferUtils::acquireWorkingBuffer(MSize(gImageNrDsWidth, gImageNrDsHeight),
                        eImgFmt_YV12);

            BufferUtils::mdpResizeAndConvert(in, ImageNrDs420WorkingBuffer);
            // Set Proc Info into FB Driver for Generate NR Image
#ifdef DEBUG_FB
            YUVBufferSave(getBuffer(in, 1.5),
                    "yuvInbuffer", "YV12", gImageWidth, gImageHeight, gImageWidth*gImageHeight*3/2);
            YUVBufferSave(gImageNrDs420Buffer, "gImageNrDs420Buffer", "YV12",
                    gImageNrDsWidth, gImageNrDsHeight, gImageNrDsWidth*gImageNrDsHeight*3/2);
#endif
            // Set Proc Info into FB Driver for Generate NR Image
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP1;
            FaceBeautyProcInfo.Step1DstImgAddr = NULL;
            FaceBeautyProcInfo.Step1SrcImgAddr = (MUINT8*)ImageNrDs420WorkingBuffer->getBufVA(0);
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
            Retcode = pFaceBeautyInterface->FaceBeautyMain();
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, &FaceBeautyResultInfo);
#ifdef DEBUG_FB
            YUVBufferSave(FaceBeautyResultInfo.Step1ResultAddr,
                    "ImageSWNrDs420Buffer", "YV12",gImageDsWidth, gImageDsHeight, gImageDsWidth*gImageDsHeight*2);
#endif
            MY_LOGD("[UpSampleFromTo] +");
            mInputWorkingBuffer = BufferUtils::acquireWorkingBuffer(
                MSize(gImageNrDsWidth, gImageNrDsHeight),
                eImgFmt_NV12);
            mOutputWorkingBuffer = BufferUtils::acquireWorkingBuffer(
                MSize(gImageWidth, gImageHeight),
                eImgFmt_NV12);
            mSWNRBlurOutputWorkingBuffer = BufferUtils::acquireWorkingBuffer(
                MSize(gImageWidth, gImageHeight),
                eImgFmt_NV12);
            //use mdp instead of function "UpSampleFromDs420ToFull422"
            memcpy((void*)mInputWorkingBuffer->getBufVA(0), FaceBeautyResultInfo.Step1ResultAddr,
                gImageNrDsWidth * gImageNrDsHeight * 1.5);
            BufferUtils::mdpResizeAndConvert(mInputWorkingBuffer, mSWNRBlurOutputWorkingBuffer);
            MY_LOGD("[UpSampleFromTo] -");
#ifdef DEBUG_FB
            YUVBufferSave(gImageSWNRBlurBuffer, "gImageSWNRBlur422Buffer",
                    "422", gImageWidth, gImageHeight, gImageWidth*gImageHeight*2);
#endif
            MY_LOGD("process Step1 <<<<<<<<<<<<<<<");
            ///////////////////////// Step1 ////////////////////////////

            ///////////////////////// Step2 ////////////////////////////
            // Set Proc Info into FB Driver for Face Alignment
            MY_LOGD("process Step2 >>>>>>>>>>>>>");
            getFaceInfo(in);
            MY_LOGD("process gImageDsWidth = %d, gImageDsHeight = %d",
                    gImageDsWidth, gImageDsHeight);
#ifdef DEBUG_FB
            YUVBufferSave(gImageDsBuffer, "gImageDsBuffer", "422",
                    gImageDsWidth,gImageDsHeight, gImageDsWidth*gImageDsHeight*2);
#endif
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP2;
            FaceBeautyProcInfo.Step2SrcImgAddr = gImageDsBuffer; 
#ifdef DEBUG_FB
            string path = "/data/camera_post_algo_fb_dump/gImageDsBuffer.bmp";
            char * str = const_cast<char*>(path.c_str());
            SaveResultBMP(gImageDsBuffer, str, gImageDsWidth, gImageDsHeight);
#endif
            int face_clock_position = getFaceOrientation(pImetadata);
            MY_LOGD("process Step2 face_clock_position = %d", face_clock_position);
            int face_rect_array_length = 4;
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE, face_rect_array_length);
            gFaceCount = face_rect_array_length/4;
            MY_LOGD("process Step2 gFaceCount = %d", gFaceCount);
            memset(&FaceBeautyProcInfo.fb_pos[0], 0, sizeof(MUINT32) * 2 * 15);
            MY_LOGD("process Step2 memset FaceBeautyProcInfo.fb_pos[0]");
            for(int i = 0; i < gFaceCount; i++)
            {   MRect faceRect;
                tryGetMetadata<MRect>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION, faceRect);
                MPoint leftRightcorner = faceRect.leftTop();
                MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION = %d point x= %d, y = %d",
                        MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION, leftRightcorner.x, leftRightcorner.y);
                MSize  faceSize = faceRect.size();
                MY_LOGD("process MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE width = %d, height = %d",
                        faceSize.w, faceSize.h);
                FaceBeautyProcInfo.FDLeftTopPointX1[i] = leftRightcorner.x;
                FaceBeautyProcInfo.FDLeftTopPointY1[i] = leftRightcorner.y;

                int left_top_x = (leftRightcorner.x*gFDWidth)/(gImageWidth);
                int left_top_y = (leftRightcorner.y*gFDWidth)/(gImageWidth);
                MY_LOGD("process gFDWidth = %d, gFDHeight = %d, gImageWidth = %d, gImageHeight = %d",
                        gFDWidth, gFDHeight,gImageWidth, gImageHeight);
                int face_rect_w = (faceSize.w*gFDWidth)/(gImageWidth);
                int face_rect_h = (faceSize.h*gFDWidth)/(gImageWidth);
                MY_LOGD("process left_top_x: x= %d, left_top_y = %d, face_rect_w = %d, face_rect_h = %d",
                        left_top_x, left_top_y, face_rect_w, face_rect_h);
                FaceBeautyProcInfo.FDLeftTopPointX1[i] = left_top_x;
                FaceBeautyProcInfo.FDLeftTopPointY1[i] = left_top_y;

                FaceBeautyProcInfo.FDBoxSize[i] = face_rect_w;
                FaceBeautyProcInfo.FDPose[i] = face_clock_position; //face orientation
                if(FaceBeautyEnvInfo.pTuningPara -> WarpFaceNum ==1 && i== 0)
                {
                    FaceBeautyProcInfo.fb_pos[i][0] = left_top_x + face_rect_w * 0.5;// face center position x?
                    FaceBeautyProcInfo.fb_pos[i][1] = left_top_y + face_rect_w * 0.5;// face centor position y?

                }
                else if(FaceBeautyEnvInfo.pTuningPara -> WarpFaceNum == 0)
                {
                    FaceBeautyProcInfo.fb_pos[i][0] = leftRightcorner.x + faceSize.w * 0.5;
                    FaceBeautyProcInfo.fb_pos[i][1] = leftRightcorner.y + faceSize.h * 0.5;
                }
            }
            FaceBeautyProcInfo.FaceCount = gFaceCount;
            Retcode = pFaceBeautyInterface -> FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO,
                    &FaceBeautyProcInfo, 0);
            Retcode = pFaceBeautyInterface -> FaceBeautyMain();
            Retcode = pFaceBeautyInterface -> FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0,
                    &FaceBeautyResultInfo);
            ///////////////////////// Step2 ////////////////////////////

            ///////////////////////// Step3 ////////////////////////////
            MY_LOGD("process Step3 >>>>>>>>>>>>>");
            // Set Proc Info into FB Driver for Alpha map
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP3;
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO,
                    &FaceBeautyProcInfo, 0);

            // Process and Get the FB Down Sampled Alpha Map
            // FaceBeautyResultInfo.AlphaMapDsAddr,
            // FaceBeautyResultInfo.AlphaMapColorDsAddr
            // will be set
            Retcode = pFaceBeautyInterface->FaceBeautyMain();
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0,
                    &FaceBeautyResultInfo);
            // downsample the ds alpha map and upsample it to full size by resizer
            // gAplhaMapBuffer will be new unsigned char[gImageWidth*gImage Height] in this function
            MY_LOGD("[UpSampleDsAlphaTextureMap] +");
            //use mdp instead of function "UpSampleDsAlphaTextureMap"
            mInputWorkingBufferDs = BufferUtils::acquireWorkingBuffer(
                MSize(gImageDsWidth, gImageDsHeight),
                eImgFmt_NV12);
            memset((void*)mInputWorkingBufferDs->getBufVA(0), 0,
                gImageDsWidth * gImageDsHeight * 1.5);
            memcpy((void*)mInputWorkingBufferDs->getBufVA(0),
                (void*)FaceBeautyResultInfo.Step3ResultAddr_1,
                gImageDsWidth * gImageDsHeight);
#ifdef DEBUG_FB
            YUVBufferSave(FaceBeautyResultInfo.Step3ResultAddr_1, "Step3ResultAddr_1", "Y",
                          gImageDsWidth, gImageDsHeight,
                          gImageDsWidth * gImageDsHeight);
#endif
            BufferUtils::mdpResizeAndConvert(mInputWorkingBufferDs, mOutputWorkingBuffer);
            gAplhaMapBuffer = new unsigned char[gImageWidth * gImageHeight];
            memcpy((void*)gAplhaMapBuffer, (void*)(mOutputWorkingBuffer->getBufVA(0)),
                gImageHeight * gImageWidth);
            //SaveLuma(gAplhaMapBuffer, "NR_AlphaMap.y", gImageWidth , gImageHeight);
#ifdef DEBUG_FB
            YUVBufferSave(gAplhaMapBuffer, "gAplhaMapBuffer", "Y", gImageWidth, gImageHeight,
                          gImageWidth * gImageHeight);
#endif
            MY_LOGD("[UpSampleDsAlphaTextureMap] -");

            ///////////////////////// Step4 ////////////////////////////
            MY_LOGD("process Step4 >>>>>>>>>>>>>");
            // Set Proc Info into FB Driver for texture alpha blending
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP4;

            FaceBeautyProcInfo.SrcImgAddr = (MUINT8 *)in->getBufVA(0);
            FaceBeautyProcInfo.Step4SrcImgAddr_1 = (MUINT8*)mSWNRBlurOutputWorkingBuffer->getBufVA(0);
            FaceBeautyProcInfo.Step4SrcImgAddr_2 = gAplhaMapBuffer;
#ifdef DEBUG_FB
            YUVBufferSave(gAplhaMapBuffer, "gAplhaMapBuffer", "Y", gImageWidth, gImageHeight, gImageWidth*gImageHeight);
#endif
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO,
                    &FaceBeautyProcInfo, 0);
            // SaveResult(gImageSWNRBlurBuffer, "blur.bmp");
            // Process and Get the Texture Smoothed Image
            // FaceBeautyResultInfo.BlendTextureImgAddr
            // will be set
            Retcode = pFaceBeautyInterface->FaceBeautyMain();
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, &FaceBeautyResultInfo);
            // SaveResult(FaceBeautyResultInfo.Step4ResultAddr, "NR.bmp");//
            // SaveResult(FaceBeautyResultInfo.Step4ResultAddr, "/storage/emulated/0/FB/NR", gImageWidth*gImageHeight);
            // downsample the ds alpha color map and upsample it to full size by resizer
            // gAplhaMapColorBuffer will be new unsigned char[gImageWidth*gImageHeight] in this function
            MY_LOGD("[UpSampleDsAlphaColorMap] +");
            //use mdp instead of function "UpSampleDsAlphaColorMap"
            memset((void*)mInputWorkingBufferDs->getBufVA(0), 0,
                gImageDsWidth * gImageDsHeight * 1.5);
            memcpy((void*)mInputWorkingBufferDs->getBufVA(0),
                FaceBeautyResultInfo.Step3ResultAddr_2,
                gImageDsWidth * gImageDsHeight);
#ifdef DEBUG_FB
            YUVBufferSave(FaceBeautyResultInfo.Step3ResultAddr_2, "Step3ResultAddr_2", "Y",
                          gImageDsWidth, gImageDsHeight,
                          gImageDsWidth * gImageDsHeight);
#endif
            BufferUtils::mdpResizeAndConvert(mInputWorkingBufferDs, mOutputWorkingBuffer);
            gAplhaMapColorBuffer = new unsigned char[gImageWidth * gImageHeight];
            memcpy((void *) gAplhaMapColorBuffer, (void *) (mOutputWorkingBuffer->getBufVA(0)),
                   gImageHeight * gImageWidth);
            MY_LOGD("[UpSampleDsAlphaColorMap] -");
#ifdef DEBUG_FB
            YUVBufferSave(gAplhaMapColorBuffer, "gAplhaMapColorBuffer", "Y", gImageWidth,
                          gImageHeight,
                          gImageWidth * gImageHeight);
#endif
            // SaveLuma(gAplhaMapColorBuffer, "CL_AlphaMap.y", gImageWidth , gImageHeight);

            ///////////////////////// Step5 ////////////////////////////
            MY_LOGD("process Step5 >>>>>>>>>>>>>");
            // Set Proc Info into FB Driver for color alpha blending
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP5;
            FaceBeautyProcInfo.Step5SrcImgAddr = gAplhaMapColorBuffer;
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);


            // Process and Get the Color Adjustment Image
            // FaceBeautyResultInfo.AdjustColorImgAddr
            // will be set
            Retcode = pFaceBeautyInterface->FaceBeautyMain();
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, &FaceBeautyResultInfo);
            //SaveResult(FaceBeautyResultInfo.Step5ResultAddr, "NRCL.bmp");

            ///////////////////////// Step6 ////////////////////////////
            MY_LOGD("process Step6 >>>>>>>>>>>>>");

            // Set Proc Info into FB Driver for warping
            //Use MDP to upsample YV12 buffer.
            gImageBlurBuffer = new unsigned char[gImageWidth*gImageHeight*2];
            FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP6;
            FaceBeautyProcInfo.Step6TempAddr = gImageBlurBuffer;
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);


            // Process and Get the Final Warped Image
            // FaceBeautyResultInfo.WarpedImgAddr
            // will be set and this is the final result
            Retcode = pFaceBeautyInterface->FaceBeautyMain();
            Retcode = pFaceBeautyInterface->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, &FaceBeautyResultInfo);
#ifdef DEBUG_FB
            //SaveResult(FaceBeautyResultInfo.Step6ResultAddr, "/storage/emulated/0/FB/NRCLWP.bmp");
#endif
            MSize outSize = out->getImgSize();
            if (out->getImgFormat() != eImgFmt_YV12 || !(gImageWidth == outSize.w && gImageHeight == outSize.h)) {
                MSize inSize = in->getImgSize();
                mYV12WorkingBuffer = BufferUtils::acquireWorkingBuffer(inSize,
                    eImgFmt_YV12);
                memcpy((void*)mYV12WorkingBuffer->getBufVA(0), FaceBeautyResultInfo.Step6ResultAddr, gImageWidth * gImageHeight);
                memcpy((void*)mYV12WorkingBuffer->getBufVA(1),
                    FaceBeautyResultInfo.Step6ResultAddr + gImageWidth * gImageHeight, (gImageWidth * gImageHeight) / 4);
                memcpy((void*)mYV12WorkingBuffer->getBufVA(2),
                FaceBeautyResultInfo.Step6ResultAddr + (gImageWidth * gImageHeight * 5) / 4,
                    (gImageWidth * gImageHeight) / 4);
#ifdef DEBUG_FB_FACE_RECT
                debugFaceInfo(pImetadata, mYV12WorkingBuffer);
#endif
                int rotationDegree = getJpegRotation(pRequest);
                bool mirror = getMirror(pRequest, getFacing(pRequest));
                MY_LOGD("process jpeg rotationDegree = %d & mirror = %d", rotationDegree,mirror);
                BufferUtils::mdpResizeAndConvert(mYV12WorkingBuffer, out, rotationDegree, mirror);

            } else {
                memcpy((void *)out->getBufVA(0), FaceBeautyResultInfo.Step6ResultAddr, gImageWidth*gImageHeight);
                memcpy((void *)out->getBufVA(1), FaceBeautyResultInfo.Step6ResultAddr + gImageWidth*gImageHeight,
                        (gImageWidth*gImageHeight)/4);
                memcpy((void *)out->getBufVA(2), FaceBeautyResultInfo.Step6ResultAddr+(gImageWidth*gImageHeight*5)/4,
                        (gImageWidth*gImageHeight)/4);
            }

            MY_LOGD("process Step6 <<<<<<<<<<<<<<<<<<<");

            delete[]gAplhaMapBuffer;
            delete[]gAplhaMapColorBuffer;

            Retcode = pFaceBeautyInterface->FaceBeautyExit();

            delete[] gImageDsBuffer;
            delete[] gImageBlurBuffer;
            delete[] WorkingBuffer;
            BufferUtils::deallocBuffer(ImageNrDs420WorkingBuffer);
            BufferUtils::deallocBuffer(mInputWorkingBuffer);
            BufferUtils::deallocBuffer(mOutputWorkingBuffer);
            BufferUtils::deallocBuffer(mInputWorkingBufferDs);
            BufferUtils::deallocBuffer(mYV12WorkingBuffer);
            BufferUtils::deallocBuffer(mSWNRBlurOutputWorkingBuffer);
        }

        if (pCallback != nullptr) {
            MY_LOGD("callback request");
            pCallback->onCompleted(pRequest, 0);
        }
        FUNCTION_OUT;
        return 0;
    };

    int getFaceOrientation(IMetadata const* pImetadata) {
        int physical_id = 0;
        tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_PHYSICAL_ID, physical_id);
        MY_LOGD("physical_id = %d", physical_id);
        int face_clock;
        if (physical_id == 0) {
            face_clock = 9;
        } else {
            face_clock = 3;
        }
        return face_clock;
    }

    int getFacing(RequestPtr pRequest) {
        MINT32 facing = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_LENS_FACING, facing);
        }
        return facing;
    }

    bool getMirror(RequestPtr pRequest,int facing) {
        if (facing != 0) {
            return false;
        }
        MINT32 mirror = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_MIRROR, mirror);
        }
        return mirror == 0;
    }

    int getJpegRotation(RequestPtr pRequest) {
        MINT32 jpegRotation = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_JPEG_ORIENTATION, jpegRotation);
        }
        return jpegRotation;
    }

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        FUNCTION_IN;

        FUNCTION_OUT;
    };

    virtual void uninit()
    {
        FUNCTION_IN;
        pFaceBeautyInterface->destroyInstance();
        FUNCTION_OUT;
    };

    void getFaceInfo(IImageBuffer* in) {
        MY_LOGD("gImageDsWidth = %d, gImageDsHeight = %d, format = %d", gImageDsWidth, gImageDsHeight, eImgFmt_RGBA8888);
        mImageDsRGBAWorkingBuffer = BufferUtils::acquireWorkingBuffer(MSize(gImageDsWidth, gImageDsHeight),
            eImgFmt_RGBA8888);
        BufferUtils::mdpResizeAndConvert(in, mImageDsRGBAWorkingBuffer);
        unsigned char* rgbabuffer = (unsigned char*)mImageDsRGBAWorkingBuffer->getBufVA(0);
        unsigned char* rgbbuffer = new unsigned char[gImageDsWidth * gImageDsHeight * 3];
        //translate RGBA -> RGB
        RGBAtoRGB(rgbabuffer, rgbbuffer, gImageDsWidth * gImageDsHeight);
        getDsImageBuffer(rgbbuffer, gImageDsWidth, gImageDsHeight);

        delete[]rgbbuffer;
        BufferUtils::deallocBuffer(mImageDsRGBAWorkingBuffer);
    };

    void getDsImageBuffer(unsigned char* RGBBuffer, int imageWidth, int imageHeight)
    {

        unsigned char* B_data_ds = new unsigned char[imageWidth*imageHeight];
        unsigned char* R_data_ds = new unsigned char[imageWidth*imageHeight];
        unsigned char* G_data_ds = new unsigned char[imageWidth*imageHeight];
        gImageDsBuffer = new unsigned char[imageWidth*imageHeight*2];

        ReadRGBBuffer(RGBBuffer, R_data_ds, G_data_ds, B_data_ds, imageWidth, imageHeight); //read the RGB value
#ifdef YUV_input
        RGB2YUV(R_data_ds, G_data_ds, B_data_ds, imageWidth, imageHeight); //R->Y, G->Cb, B->Cr
        YUV444to422(G_data_ds, B_data_ds, imageWidth, imageHeight, 0);
#endif

        memcpy(gImageDsBuffer, R_data_ds, imageWidth*imageHeight);
        memcpy(gImageDsBuffer+imageWidth*imageHeight, G_data_ds, imageWidth*imageHeight/2);
        memcpy(gImageDsBuffer+imageWidth*imageHeight*3/2, B_data_ds, imageWidth*imageHeight/2);


        delete[]B_data_ds;
        delete[]R_data_ds;
        delete[]G_data_ds;

    }

    void RGBAtoRGB(unsigned char *rgba, unsigned char *rgb, int numPixels)
    {
        int col;

        for (col = 0; col < numPixels; col++, rgba += 4, rgb += 3) {
            rgb[0] = rgba[0];
            rgb[1] = rgba[1];
            rgb[2] = rgba[2];
        }
    }

    void YUV444to422(unsigned char* U_data2, unsigned char* V_data2, int width, int height, int drop)
    {
        int i, j;
        unsigned char* cb_data_422 = (unsigned char*)malloc(width/2*height*sizeof(unsigned char));
        unsigned char* cr_data_422 = (unsigned char*)malloc(width/2*height*sizeof(unsigned char));
        int place, place1;
        for(i=0;i<height;i++)
        {
            for(j=0;j<width/2;j++)
            {
                place  = i*width/2 + j;
                place1 = i*width + j*2;

                if(drop)
                {
                    cb_data_422[place] = U_data2[place1];
                    cr_data_422[place] = V_data2[place1];
                }
                else
                {
                    if(j==0)
                    {
                        cb_data_422[place] = (3*U_data2[place1] + U_data2[place1+1] + 2)/4;
                        cr_data_422[place] = (3*V_data2[place1] + V_data2[place1+1] + 2)/4;
                    }
                    else
                    {
                        cb_data_422[place] = (U_data2[place1-1] + 2*U_data2[place1] + U_data2[place1+1] + 2)/4;
                        cr_data_422[place] = (V_data2[place1-1] + 2*V_data2[place1] + V_data2[place1+1] + 2)/4;
                    }
                }
            }
        }

        for(i=0;i<height;i++)
        {
            for(j=0;j<width/2;j++)
            {
                place  = i*width/2 + j;
                U_data2[place] = cb_data_422[place];
                V_data2[place] = cr_data_422[place];
            }
        }

        free(cb_data_422);
        free(cr_data_422);
    }

    void RGB2YUV(unsigned char* Y_data2, unsigned char* U_data2, unsigned char* V_data2, int width, int height)
    {
        int i,j;
        int place, Y, Cb, Cr;

        int R2Y = 153;
        int G2Y = 301;
        int B2Y = 58;
        int R2Cb = -86;
        int G2Cb = -170;
        int B2Cb = 256;
        int R2Cr = 256;
        int G2Cr = -214;
        int B2Cr = -42;

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                place = i*width + j;
                Y  = (R2Y * Y_data2[place] + G2Y*U_data2[place] + B2Y*V_data2[place] + 256)>>9;
                Cb = ((R2Cb * Y_data2[place] + G2Cb*U_data2[place] + B2Cb*V_data2[place] + 256)>>9) + 128;
                Cr = ((R2Cr * Y_data2[place] + G2Cr*U_data2[place] + B2Cr*V_data2[place] + 256)>>9) + 128;

                if(Y>255) Y=255;
                else if(Y<0) Y=0;
                //else Y=Y;
                if(Cb>255) Cb=255;
                else if(Cb<0) Cb=0;
                //else Cb=Cb;
                if(Cr>255) Cr=255;
                else if(Cr<0) Cr=0;
                //else Cr=Cr;
                Y_data2[place] = Y;
                U_data2[place] = Cb;
                V_data2[place] = Cr;
            }
        }
    }

    void ReadRGBBuffer(unsigned char* RGBBuffer,unsigned char* R_data_ds,unsigned char* G_data_ds,unsigned char* B_data_ds,int input_img_width_ds, int input_img_height_ds)
    {
        int i,j;
        MY_LOGD("ReadRGBBuffer input_img_width_ds:%d, input_img_height_ds = %d", input_img_width_ds, input_img_height_ds);
        for(i=0;i<input_img_height_ds;i++)
        {
            for(j=0;j<input_img_width_ds*3;j++)
            {
                int place = i*input_img_width_ds*3 + j;
                if(j%3==0)
                    R_data_ds[i*input_img_width_ds+j/3] = RGBBuffer[place];
                else if(j%3==1)
                    G_data_ds[i*input_img_width_ds+j/3] = RGBBuffer[place];
                else
                    B_data_ds[i*input_img_width_ds+j/3] = RGBBuffer[place];
            }
        }
    }

    unsigned char * getBuffer(IImageBuffer * in, float multiple) {
        IImageBufferHeap* pHeap = in->getImageBufferHeap();
        MY_LOGD("getBuffer Format:%d", pHeap->getImgFormat());
        MY_LOGD("getBuffer getImgSize:width = %d, height = %d", pHeap->getImgSize().w, pHeap->getImgSize().h);
        MY_LOGD("getBuffer 2PlaneCount  = %d", (int)pHeap->getPlaneCount());
        int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
        int size = (int)(pHeap->getImgSize().w * pHeap->getImgSize().h * multiple);
        MY_LOGD("getBuffer size  = %d", size);
        unsigned char* imageBuffer = new unsigned char[size];
        pHeap->lockBuf("FB getBuffer", usage);
        int offset = 0;
        for (unsigned int i = 0; i < pHeap->getPlaneCount(); i++)
        {   char * buffer = (char *)pHeap->getBufVA(i);
            int lenght = pHeap->getBufSizeInBytes(i);
            MY_LOGD("getBufSizeInBytes:%d", (int)pHeap->getBufSizeInBytes(i));
            if (buffer)
            {
                //::memset((MUINT8*)pHeap->getBufVA(i), 0xAA, pHeap->getBufSizeInBytes(i));
                memcpy(imageBuffer+offset, buffer, lenght);
                offset = offset + lenght;
            }
        }
        pHeap->unlockBuf("FB getBuffer");
        return imageBuffer;
    };

    FBProviderImpl()
        :mOpenid(-1)
    {
        FUNCTION_IN;

        mEnable = 1;

        FUNCTION_OUT;
    };

    virtual ~FBProviderImpl()
    {
        FUNCTION_IN;

        FUNCTION_OUT;
    };

    template <class T>
        inline bool
        tryGetMetadata( IMetadata const *pMetadata, mtk_platform_metadata_tag_t tag, T& rVal)
        {
            if(pMetadata == nullptr) return MFALSE;

            IMetadata::IEntry entry = pMetadata->entryFor(tag);
            if(!entry.isEmpty())
            {
                rVal = entry.itemAt(0,Type2Type<T>());
                return true;
            }
            else
            {
                MY_LOGW(" no metadata %d ", tag);
            }
            return false;
        };

    void YUVBufferSave(unsigned char* image, const char* filename1, const char* format, int w, int h, int size)
    {

        char path[128] = "/data/camera_post_algo_fb_dump";
        int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        if ((result == -1) && (errno != EEXIST)) {
            MY_LOGE("mkdir fail, error %d, return", errno);
            return;
        }

        char fileName[128];
        sprintf(fileName, "%s/%s_w%d_h%d_%s.raw", path, filename1, w, h, format);
        MY_LOGD("fileName=%s", fileName);
        FILE *fp = fopen(fileName, "wb");
        if (NULL == fp) {
            MY_LOGE("fail to open file %s", fileName);
        } else {
            fwrite(image,1,size,fp);
            fclose(fp);
        }

    }


    void SaveResultBMP(unsigned char* image, char* filename1, int width, int height)
    {
        int i,j;
        int place, place1;
        int pad_width = (width*3 + 3)/4*4;        //padding handling
        unsigned char* BMP_data_o = new unsigned char[pad_width*height];
        unsigned char* buffer = new unsigned char[width*height*3];
        unsigned char* R_data = buffer;
        unsigned char* G_data = R_data+width*height;
        unsigned char* B_data = G_data+width*height;
        memcpy(R_data, image, width*height);
        //#ifdef INPUT_YUV420 //YV12
        //    memcpy(B_data, image+width*height,   width*height/4);
        //    memcpy(G_data, image+width*height*5/4, width*height/4);
        //#else
        memcpy(G_data, image+width*height,   width*height/2);
        memcpy(B_data, image+width*height*3/2, width*height/2);
        //#endif

        //#ifdef INPUT_YUV420
        // YUV420to444(G_data, B_data, width, height,0);
        //#else
        YUV422to444(G_data, B_data, width, height,0);
        //#endif
        YUV2RGB(R_data, G_data, B_data, width, height);

        for(i=0;i<height;i++)
        {
            for(j=0;j<width*3;j++)
            {
                place  = i*pad_width+j;
                place1 = (height-i-1)*width+j/3;
                if(j%3==0)
                    BMP_data_o[place] = B_data[place1];
                else if(j%3==1)
                    BMP_data_o[place] = G_data[place1];
                else
                    BMP_data_o[place] = R_data[place1];
            }
        }

        FILE* stream_write2;
        char string1[200];
        strcpy(string1,filename1);
        if( (stream_write2 = fopen( string1, "wb" )) == NULL )
            printf( "The file _wrinkle_removal.bmp was not opened\n" );
        writeBMP(stream_write2, R_data, G_data, B_data, width, height);
        fclose(stream_write2);
        delete[]buffer;
        delete[]BMP_data_o;

    }

    void writeBMP (FILE *fptr, unsigned char *tgDataR, unsigned char *tgDataG, unsigned char *tgDataB, int w, int h)
    {
        char  header[54];
        // int buf len;
        int   bfSize;
        int   i, j;
        int   R, G, B;

        /* Set unused fields of header to 0 */
        memset(header, 0, sizeof(header));

        // buf_len = ((w-1)/4 + 1) * 4;
        // bfSize = 54 + 3*buf_len*h;
        bfSize = 54 + (3*w+(w & 0x03))*h;
        // printf("buf_len=%d, bfSize=%d, width=%d, height=%d\n", buf_len, bfSize, w, h);
        header[0] = 0x42; // B
        header[1] = 0x4D; // M
        PUT_4B(header, 2, bfSize); /* bfSize */
        PUT_4B(header, 10, 54); /* bfSize */

        /* Fill the info header (Microsoft calls this a BITMAPINFOHEADER) */
        PUT_2B(header, 14, 40); /* biSize */
        // PUT_4B(header, 18, buf_len);  biWidth
        PUT_4B(header, 18, w); /* biWidth */
        PUT_4B(header, 22, h); /* biHeight */
        PUT_2B(header, 26, 1); /* biPlanes - must be 1 */
        PUT_2B(header, 28, 24); /* biBitCount */
        /** we leave biCompression = 0, for none */
        /** we leave biSizeImage = 0; this is correct for uncompressed data */
        PUT_2B(header, 46, 0); /* biClrUsed */
        /** we leave biClrImportant = 0 */

        fwrite(header, 1, 54, fptr);
        for (i=0; i<h; i++)
        {
            for (j=0; j<w; j++)
            {
                R = (int) tgDataR[(h-i-1)*w+j];
                G = (int) tgDataG[(h-i-1)*w+j];
                B = (int) tgDataB[(h-i-1)*w+j];

                fputc(B, fptr);
                fputc(G, fptr);
                fputc(R, fptr);
                // printf("i=%d, j=%d, data_count= %d\n", i, j, i*w+j);
                // printf("%c%c%c\n", G, R, B);
                // getchar();
            }
            for (j=0; j<(w & 0x03); j++) fputc(0, fptr);
        }
    }

    void SaveResult(unsigned char* image, char filename1[])
    {
        int i,j;
        int place, place1;
        int pad_width = (gImageWidth*3 + 3)/4*4;        //padding handling
        unsigned char* BMP_data_o = new unsigned char[pad_width*gImageHeight];
        unsigned char* buffer = new unsigned char[gImageWidth*gImageHeight*3];
        unsigned char* R_data = buffer;
        unsigned char* G_data = R_data+gImageWidth*gImageHeight;
        unsigned char* B_data = G_data+gImageWidth*gImageHeight;
        memcpy(R_data, image, gImageWidth*gImageHeight);
#ifdef INPUT_YUV420 //YV12
        memcpy(B_data, image+gImageWidth*gImageHeight,   gImageWidth*gImageHeight/4);
        memcpy(G_data, image+gImageWidth*gImageHeight*5/4, gImageWidth*gImageHeight/4);
#else
        memcpy(G_data, image+gImageWidth*gImageHeight,   gImageWidth*gImageHeight/2);
        memcpy(B_data, image+gImageWidth*gImageHeight*3/2, gImageWidth*gImageHeight/2);
#endif

#ifdef INPUT_YUV420
        YUV420to444(G_data, B_data, gImageWidth, gImageHeight,0);
#else
        YUV422to444(G_data, B_data, gImageWidth, gImageHeight,0);
#endif
        YUV2RGB(R_data, G_data, B_data, gImageWidth, gImageHeight);


        for(i=0;i<gImageHeight;i++)
        {
            for(j=0;j<gImageWidth*3;j++)
            {
                place  = i*pad_width+j;
                place1 = (gImageHeight-i-1)*gImageWidth+j/3;
                if(j%3==0)
                    BMP_data_o[place] = B_data[place1];
                else if(j%3==1)
                    BMP_data_o[place] = G_data[place1];
                else
                    BMP_data_o[place] = R_data[place1];
            }
        }

        FILE* stream_write2;
        //strcpy(string1,filename1);
        if( (stream_write2 = fopen( filename1, "wb" )) == NULL ) {
            printf( "The file _wrinkle_removal.bmp was not opened\n" );
        } else {
            //fwrite(BMP_header,1,54,stream_write2);
            fwrite(BMP_data_o,pad_width,gImageHeight,stream_write2);
            fclose(stream_write2);
        }
        delete[]buffer;
        delete[]BMP_data_o;

    }
    void YUV2RGB(unsigned char* Y_data2, unsigned char* U_data2, unsigned char* V_data2, int width, int height)
    {
        int i,j;
        int place, Y, Cb, Cr;
        int    Y2R = 256;
        int    Y2G = 256;
        int    Y2B = 256;
        int    Cb2R = 0;
        int    Cb2G = -88;
        int    Cb2B = 455;
        int    Cr2R = 360;
        int    Cr2G = -184;
        int    Cr2B = 0;

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                place = i*width + j;
                //            R = Y + (1.4075 * (V - 128));
                //            G = Y - (0.3455 * (U - 128) - (0.7169 * (V - 128));
                //            B = Y + (1.7790 * (U - 128);
                if(i==65 && j==211) {
                    //i = i;
                }
                Y  = (int)(Y2R*Y_data2[place] + Cb2R*(U_data2[place]-128) + Cr2R*(V_data2[place]-128) + 128)/256;
                Cb = (int)(Y2G*Y_data2[place] + Cb2G*(U_data2[place]-128) + Cr2G*(V_data2[place]-128) + 128)/256;
                Cr = (int)(Y2B*Y_data2[place] + Cb2B*(U_data2[place]-128) + Cr2B*(V_data2[place]-128) + 128)/256;

                if(Y>255) Y=255;
                else if(Y<0) Y=0;
                else {
                    //Y=Y;
                }
                if(Cb>255) Cb=255;
                else if(Cb<0) Cb=0;
                else {
                    // Cb=Cb;
                }
                if(Cr>255) Cr=255;
                else if(Cr<0) Cr=0;
                else {
                    //Cr=Cr;
                }
                Y_data2[place] = Y;
                U_data2[place] = Cb;
                V_data2[place] = Cr;
            }
        }
    };

    void YUV422to444(unsigned char* U_data, unsigned char* V_data, int width, int height, int copy)
    {
        int i,j;
        unsigned char* cb_data_444 = (unsigned char*)malloc(width*height*sizeof(unsigned char));
        unsigned char* cr_data_444 = (unsigned char*)malloc(width*height*sizeof(unsigned char));
        int place, place1;

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                place = i*width + j;
                place1 = i*width/2 + j/2;
                if(j%2==0)
                {
                    cb_data_444[place] = U_data[place1];
                    cr_data_444[place] = V_data[place1];
                }
                else if(j+1==width || copy)
                {
                    cb_data_444[place] = U_data[place1];
                    cr_data_444[place] = V_data[place1];
                }
                else
                {
                    cb_data_444[place] = (U_data[place1]+U_data[place1+1]+1)/2;
                    cr_data_444[place] = (V_data[place1]+V_data[place1+1]+1)/2;
                }
            }
        }

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                place  = i*width + j;
                U_data[place] = cb_data_444[place];
                V_data[place] = cr_data_444[place];
            }
        }
        free(cb_data_444);
        free(cr_data_444);

    }

    void YUV420to444(unsigned char* U_data, unsigned char* V_data, int width, int height, int copy)
    {
        int i,j;
        unsigned char* cb_data_444 = (unsigned char*)malloc(width*height*sizeof(unsigned char));
        unsigned char* cr_data_444 = (unsigned char*)malloc(width*height*sizeof(unsigned char));
        int place, place1;
        if (cb_data_444 != nullptr && cr_data_444 != nullptr) {
            //
            for(i=0;i<height;i+=2)
            {
                for(j=0;j<width;j++)
                {
                    if(j==width-1&&i==height-1)
                    {
                        //i = i;
                    }
                    place = i*width + j;
                    place1 = (i/2)*width/2 + j/2;
                    if(j%2==0)
                    {
                        cb_data_444[place] = U_data[place1];
                        cr_data_444[place] = V_data[place1];
                    }
                    else if(j+1==width || copy)
                    {
                        cb_data_444[place] = U_data[place1];
                        cr_data_444[place] = V_data[place1];
                    }
                    else
                    {
                        cb_data_444[place] = (U_data[place1]+U_data[place1+1]+1)/2;
                        cr_data_444[place] = (V_data[place1]+V_data[place1+1]+1)/2;
                    }
                }
            }
            //
            for(i=0;i<(height-1);i+=2)
            {
                for(j=0;j<width;j++)
                {
                    place = i*width + j;
                    place1 = (i+1)*width + j;

                    cb_data_444[place1] = (cb_data_444[place]+cb_data_444[place+width*2]+1)/2;
                    cr_data_444[place1] = (cr_data_444[place]+cr_data_444[place+width*2]+1)/2;
                }
            }
            //
            for(i=0;i<height;i++)
            {
                for(j=0;j<width;j++)
                {
                    place  = i*width + j;
                    U_data[place] = cb_data_444[place];
                    V_data[place] = cr_data_444[place];
                }
            }

        }
        if (cb_data_444 != nullptr) {
            free(cb_data_444);
        }

        if (cr_data_444 != nullptr) {
            free(cr_data_444);
        }
    }

    void debugFaceInfo(IMetadata* pImetadata, IImageBuffer *out) {
        MINT32 gFaceCount;
        MINT32 face_rect_length = 4;
        tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE, face_rect_length);
        gFaceCount = face_rect_length/4;
        MY_LOGD("process face = %d", gFaceCount);
        for(int i=0;i<gFaceCount;i++) {
            MRect faceRect;
            tryGetMetadata<MRect>(pImetadata, MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION, faceRect);
            char* pBufferVa = (char *) (out->getBufVA(0));
            MUINT32 stride = out->getBufStridesInBytes(0);
            MY_LOGD("Detected Face Rect[%d]: (xmin, ymin, xmax, ymax) => (%d, %d, %d, %d)",
                    i,
                    faceRect.p.x,
                    faceRect.p.y,
                    faceRect.s.w,
                    faceRect.s.h);
            // draw rectangles to output buffer
            memset(pBufferVa + stride * faceRect.p.y + faceRect.p.x,
                    255, faceRect.s.w + 1);

            memset(pBufferVa + stride * (faceRect.p.y + faceRect.s.h) + faceRect.p.x,
                    255, faceRect.s.w + 1);

            for (size_t j = faceRect.p.y + 1; j < (faceRect.p.y + faceRect.s.h) ; j++) {
                *(pBufferVa + stride * j + faceRect.p.x) = 255;
                *(pBufferVa + stride * j + faceRect.p.x + faceRect.s.w) = 255;
            }
        }
    }
    private:
    int    mOpenid;
    int    mEnable;
    int gImageWidth;
    int gImageHeight;
    int gImageNrDsWidth;
    int gImageNrDsHeight;
    const int gImageDsWidth = 640;
    int gImageDsHeight;
    MTKFaceBeauty* pFaceBeautyInterface;
    MUINT8* WorkingBuffer;
    MUINT32 WorkingBufferSize;
    const int gFDWidth = 320;
    int gFDHeight = 240;
    IImageBuffer* ImageNrDs420WorkingBuffer = nullptr;
    IImageBuffer* mImageDsRGBAWorkingBuffer = nullptr;
    unsigned char* gImageDsBuffer;

    int gFaceCount = 1; //Number of face in current image
    //char string1[200];
    //unsigned char BMP_header[54];
    unsigned char* gAplhaMapBuffer;
    unsigned char* gAplhaMapColorBuffer;
    unsigned char* gImageBlurBuffer;
    IImageBuffer* mYV12WorkingBuffer = nullptr;
    IImageBuffer* mOutputWorkingBuffer = nullptr;
    IImageBuffer* mSWNRBlurOutputWorkingBuffer = nullptr;
    IImageBuffer* mInputWorkingBuffer = nullptr;
    IImageBuffer* mInputWorkingBufferDs = nullptr;

    MRESULT Retcode;
};

REGISTER_PLUGIN_PROVIDER(Yuv, FBProviderImpl);

