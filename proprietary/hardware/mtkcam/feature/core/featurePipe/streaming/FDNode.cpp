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

#include "FDNode.h"

#define PIPE_CLASS_TAG "FDNode"
#define PIPE_TRACE TRACE_FD_NODE
#include <featurePipe/core/include/PipeLog.h>

static unsigned char FaceNum =0;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

FDNode::FDNode(const char *name)
    : StreamingFeatureNode(name)
    , mFDInited(MFALSE)
    , mRotation(0)
    , mFDWidth(0)
    , mFDHeight(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mDsImgDatas);
    TRACE_FUNC_EXIT();
}

FDNode::~FDNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL FDNode::onData(DataID id, const ImgBufferData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_FD_DSIMG )
    {
        mDsImgDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FDNode::onInit()
{
#define FD_WORKING_BUF_SIZE 5242880 //5M: 1024*1024*5

    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
#if 1
    /* Create FD HAL instance */
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_HW);
    if (mpFDHalObj == NULL)
    {
        MY_LOGE("halFDBase create instance failed");
        return 0;
    }

    mpFDWorkingBuffer = new unsigned char[FD_WORKING_BUF_SIZE];
    mFDVGABuffer = new unsigned char[640*480*2];
    mFDYBuffer = new unsigned char[VFB_DS_IMAGE_MAX_WIDTH*VFB_DS_IMAGE_MAX_HEIGHT];
#endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onUninit()
{
    TRACE_FUNC_ENTER();
    if (mpFDWorkingBuffer != NULL)
    {
        free(mpFDWorkingBuffer);
    }
    if (mFDVGABuffer != NULL)
    {
        delete mFDVGABuffer;
    }
    if (mFDYBuffer != NULL)
    {
        delete mFDYBuffer;
    }
    if (mpFDHalObj != NULL)
    {
        if (mFDInited)
        {
            mpFDHalObj->halFDUninit();
        }
        mpFDHalObj->destroyInstance();
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    ImgBufferData dsImgData;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mDsImgDatas.deque(dsImgData) )
    {
        MY_LOGE("DsImgData deque out of sync");
        return MFALSE;
    }
    if( dsImgData.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = dsImgData.mRequest;
    request->mTimer.startFD();
    TRACE_FUNC("Frame %d in FD", request->mRequestNo);
    processFD(request, dsImgData.mData);
    request->mTimer.stopFD();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::processFD(const RequestPtr &request, const ImgBuffer &dsImg)
{
    TRACE_FUNC_ENTER();
    MyFace face;

    MUINT32 imgw = dsImg->getImageBuffer()->getImgSize().w;
    MUINT32 imgh = dsImg->getImageBuffer()->getImgSize().h;

    if(mFDInited &&
      (mFDWidth != 0) &&
      ((mFDWidth != imgw) ||
      (mFDHeight != imgh)))
    {
        mpFDHalObj->halFDUninit();
    }
    if((mFDWidth == 0) ||
       (mFDWidth != imgw) ||
       (mFDHeight != imgh))
    {
        mpFDHalObj->halFDInit(imgw,
                          imgh,
                          mpFDWorkingBuffer,
                          FD_WORKING_BUF_SIZE,
                          1,
                          2); //2:enhanced for FB
        //mp3AHal->setFDEnable(true);
        mFDInited = MTRUE;
    }
    mFDWidth = imgw;
    mFDHeight = imgh;

    MINT32 numFace = 0;
    if(FaceNum>0)
        mpFDHalObj->halFTBufferCreate(mFDVGABuffer, (MUINT8*) dsImg->getImageBuffer()->getBufVA(0), 1, mFDWidth, mFDHeight);

#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
    MUINT8* dstbufadr = (MUINT8*) malloc(imgw * imgh * 3 / 2);
    for (int i=0;i<imgw*imgh*2/4;i++)
    {
        dstbufadr[2*i]=((MUINT8*)pImageBuffer->getBufVA(0))[4*i];
        dstbufadr[2*i+1]=((MUINT8*)pImageBuffer->getBufVA(0))[4*i+2];
    }
    //saveBufToFile("/sdcard/fdi_YV12.yuv", dstbufadr, imgw*imgh * 3/2);
#else
    mpFDHalObj->halFDYUYV2ExtractY(mFDYBuffer, (MUINT8*) dsImg->getImageBuffer()->getBufVA(0), mFDWidth, mFDHeight);
#endif

    //pImageBuffer->saveToFile("/sdcard/fdi_YUY2.yuv");
    MUINT8 count = 0;
    struct FD_Frame_Parameters Param;
    do {
        //MY_LOGE("Face detection try %d", count + 1);
        Param.pScaleImages = NULL;
        Param.pRGB565Image = (MUINT8 *)mFDVGABuffer;
        #ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
        Param.pPureYImage  = dstbufadr;
        #else
        Param.pPureYImage  = mFDYBuffer;
        #endif
        #ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
        Param.pImageBufferPhyP0 = NULL;
        #else
        Param.pImageBufferPhyP0 = (MUINT8*)dsImg->getImageBuffer()->getBufPA(0);
        #endif
        Param.pImageBufferPhyP1 = NULL;
        Param.pImageBufferPhyP2 = NULL;
        Param.Rotation_Info = mRotation;
        Param.SDEnable = false;
        Param.AEStable = 0;
        mpFDHalObj->halFDDo(Param);

        numFace = mpFDHalObj->halFDGetFaceResult(&face.mMeta);
        FaceNum = numFace;
    } while ((numFace == 0) && (++count < 1));
    MY_LOGE("Face detection tried %d times and found %d faces", count, numFace);

#ifdef _WORKAROUND_FD_NOT_SUPPORT_YUY2
    free(dstbufadr);
#endif
    handleData(ID_FD_TO_VFB_FACE, FaceData(face, request));
    request->doExtCallback(FeaturePipeParam::MSG_FD_DONE);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
