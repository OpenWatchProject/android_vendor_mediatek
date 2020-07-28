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
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
//
using namespace NSCamClient;
using namespace NSFDClient;
using namespace NSCam::Utils;
//
/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define SCALE_NUM (11)

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
*******************************************************************************/
bool
FDClient::initBuffers(sp<IImgBufQueue>const &rpBufQueue)
{
     return createDetectedBuffers() && createWorkingBuffers(rpBufQueue) && createDDPWorkBuffers() && createFDWorkBuffers();
}


/******************************************************************************
*
*******************************************************************************/
void
FDClient::uninitBuffers()
{
    destroyDetectedBuffers();
    destroyWorkingBuffers();
    destroyDDPWorkBuffers();
    destroyFDWorkBuffers();
}

/******************************************************************************
*
*******************************************************************************/
bool
FDClient::
createDetectedBuffers()
{

    bool ret = false;

    mpDetectedFaces = new MtkCameraFaceMetadata;
    if ( NULL != mpDetectedFaces )
    {
        {
            mpDetectedFaces->number_of_faces = 0;
            mpDetectedFaces->ImgWidth = 640; // init value
            mpDetectedFaces->ImgHeight = 480; // init value
            ret = true;
        }
    }

    mpDetectedGestures = new MtkCameraFaceMetadata;
    if ( NULL != mpDetectedGestures )
    {
        {
            mpDetectedGestures->number_of_faces = 0;
            ret = true;
        }
    }

    return ret;
}



/******************************************************************************
*
*******************************************************************************/
bool
FDClient::
createWorkingBuffers(sp<IImgBufQueue>const &rpBufQueue)
{
    bool ret = true;
    //
    // [Seed FD buffer]
    // use AP setting: format/width/height
    //
    MUINT32 bufWidth = 0, bufHeight = 0;
    MINT32 pv_bufWidth =0, pv_bufHeight = 0;
    mpParamsMgr->getPreviewSize(&pv_bufWidth, &pv_bufHeight);

    bufWidth = mFDBufferWidth = 640;
    mFDBufferPlanes = 1;
    if(pv_bufWidth != 0)
        bufHeight = bufWidth * pv_bufHeight / pv_bufWidth;
    else
        bufHeight = 480;

    mPadding_w = FD_IMG_PADDING;
    mPadding_h = FD_IMG_PADDING * bufHeight / bufWidth;

    String8 const format = String8(MtkCameraParameters::PIXEL_FORMAT_YUV422I);
    int32_t const imgFormat = eImgFmt_YUY2;
    uint32_t const bpp = NSCam::Utils::Format::queryImageBitsPerPixel(imgFormat);
    size_t const bufsize = (( bpp
                            * NSCam::Utils::Format::queryPlaneWidthInPixels(imgFormat, 0, bufWidth)
                            * bufHeight ) >> 3);
    //
    MY_LOGD("[seed FD buffer] w: %d, h: %d, format: %s bufsize: %zu", bufWidth, bufHeight, format.string(), bufsize);
    //
    for (int i = 0; i < FDClient::mBufCnt; i++)
    {
        sp<FDBuffer> one = new FDBuffer(bufWidth, bufHeight,
                                        bpp,
                                        bufsize,
                                        imgFormat, format, "FDBuffer");

        ret = rpBufQueue->enqueProcessor(
                ImgBufQueNode(one, ImgBufQueNode::eSTATUS_TODO)
        );

        if ( ! ret )
        {
            MY_LOGW("enqueProcessor() fails");
        }
    }
    mFDPaddingImg = new FDBuffer(bufWidth, bufHeight,
                                        bpp,
                                        bufsize,
                                        imgFormat, format, "FDBufferPadding");
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
FDClient::
createDDPWorkBuffers()
{
    mpDDPBuffer = new unsigned char[640*640*2];
    if (mpDDPBuffer == NULL)
    {
        MY_LOGE("Create DDPBuffer failed");
        return false;
    }
    mpExtractYBuffer = new unsigned char[1920*1080];
    if (mpExtractYBuffer == NULL)
    {
        MY_LOGE("Create ExtractYBuffer failed");
        if (mpDDPBuffer)
        {
            delete [] mpDDPBuffer;
            mpDDPBuffer = NULL;
        }
        return false;
    }
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
FDClient::
createFDWorkBuffers()
{
    bool ret = true;
    mFDWorkingBufferSize = 1024*1024*4; //25M: 1024*1024*25
    mpFDWorkingBuffer = new unsigned char[mFDWorkingBufferSize];

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
void
FDClient::
destroyDetectedBuffers()
{
    if ( mpDetectedFaces != NULL )
    {
        delete mpDetectedFaces;
        mpDetectedFaces = NULL;
    }

    if ( mpDetectedGestures != NULL )
    {
        delete mpDetectedGestures;
        mpDetectedGestures = NULL;
    }
}


/******************************************************************************
*
*******************************************************************************/
void
FDClient::
destroyWorkingBuffers()
{
    mFDPaddingImg = NULL;
}


/******************************************************************************
*
*******************************************************************************/
void
FDClient::
destroyDDPWorkBuffers()
{
    if (mpDDPBuffer)
    {
        delete [] mpDDPBuffer;
        mpDDPBuffer = NULL;
    }
    if (mpExtractYBuffer)
    {
        delete [] mpExtractYBuffer;
        mpExtractYBuffer = NULL;
    }
}

void
FDClient::
destroyFDWorkBuffers()
{
    delete [] mpFDWorkingBuffer;
}


/******************************************************************************
*
*******************************************************************************/
bool
FDClient::
handleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue, ImgBufQueNode const &rQueNode)
{
    bool ret = true;

    ret = rpBufQueue->enqueProcessor(
            ImgBufQueNode(rQueNode.getImgBuf(), ImgBufQueNode::eSTATUS_TODO));

    if ( ! ret )
    {
        MY_LOGE("enqueProcessor() fails");
        ret = false;
    }

    return ret;
}



/******************************************************************************
* buffer can be reached either by client enque back buffer
* or by previewclient.
*******************************************************************************/
bool
FDClient::
waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue, ImgBufQueNode &rQueNode)
{
    bool ret = false;
    Vector<ImgBufQueNode> vQueNode;
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    // (1) wait and deque from processor
    rpBufQueue->dequeProcessor(vQueNode);
    if ( vQueNode.empty() )
    {
        ret = false;
        MY_LOGD("Deque from processor is empty. Suppose stopProcessor has been called");
        goto lbExit;

    }
    // (2) check vQueNode:
    //    - TODO or CANCEL
    //    - get and keep the latest one with TODO tag;
    //    - otherwise, return to processors
    for (size_t i = 0; i < vQueNode.size(); i++)
    {
        if (vQueNode[i].isDONE() && vQueNode[i].getImgBuf() != 0)
        {
             if (rQueNode.getImgBuf() != 0 ) // already got
             {
                 handleReturnBuffers(rpBufQueue, rQueNode);
             }
             rQueNode = vQueNode[i];  // update a latest one
             ret = true;
        }
        else
        {
             handleReturnBuffers(rpBufQueue, vQueNode[i]);
        }
    }
lbExit:
    //
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return ret;
}

