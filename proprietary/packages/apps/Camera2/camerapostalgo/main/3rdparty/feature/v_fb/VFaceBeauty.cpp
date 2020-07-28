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

#include <stdint.h>
#include <vector>
#include <strings.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/log.h>
#include "VFaceBeauty.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "GLContext.h"
#include "LogUtils.h"

#define LOG_TAG "VFaceBeauty"
//#define DEBUG_FACE_INFO

using namespace std;
using namespace NSCam::NSPipelinePlugin;

VFaceBeauty::VFaceBeauty() {
    MY_LOGD("VFaceBeauty");
}

VFaceBeauty::~VFaceBeauty() {
    MY_LOGD("~VFaceBeauty");
}

void VFaceBeauty::init(bool isCapture) {
    MY_LOGD("VFaceBeauty init");
    mAwFaceBeauty = new AwFaceBeauty();
    mAwFaceBeauty->init(isCapture);
    if (!isCapture) {
        mAwFaceBeauty->setUseOES(true);
        mGLContext = new GLContext();
        mGLContext->initialize(true);
    }
    mAwFaceBeauty->setBeautyLevel(BEAUTY_ALL, 1);
}

void VFaceBeauty::setBeautyLevel(aw_beauty_type type, float value) {
    if (mAwFaceBeauty != nullptr) {
        mAwFaceBeauty->setBeautyLevel(type, value);
        MY_LOGD("setBeautyLevel type = %d, value = %f", type, value);
    }
}

void VFaceBeauty::setFaceInfo(aw_face_info faceInfo[], int faceNum) {
    if (mAwFaceBeauty != nullptr) {
        mAwFaceBeauty->setFaceInfo(faceInfo, faceNum);
    }
}

void VFaceBeauty::makeupForPreview(IImageBuffer *in, IImageBuffer *out) {
    FUNCTION_IN;
    MSize inSize = in->getImgSize();
    MSize outSize = out->getImgSize();

    mGLContext->enable();

    GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsIn;
    memset(&paramsIn, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
    paramsIn.eglDisplay = mGLContext->_display;
    paramsIn.isRenderTarget = false;
    paramsIn.graphicBuffer = BufferUtils::getAHWBuffer(in);
    GLUtils::getEGLImageTexture(&paramsIn);

    GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsOut;
    memset(&paramsOut, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
    paramsOut.eglDisplay = mGLContext->_display;
    paramsOut.isRenderTarget = true;
    if (out->getImgFormat() != eImgFmt_RGBA8888) {
        if (mOutputWorkingBuffer == nullptr) {
            mOutputWorkingBuffer = BufferUtils::acquireWorkingBuffer(outSize,
                    eImgFmt_RGBA8888);
        }
        paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(mOutputWorkingBuffer);
    } else {
        paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(out);
    }
    GLUtils::getEGLImageTexture(&paramsOut);
    quality = fd_quality/100.0;
    MY_LOGD("makeup init GL env end, and begin makeup, current quality = %f", quality);
    long begin_time = LogUtils::getCurrentMillSeconds();
    mAwFaceBeauty->drawFaceBeautyFrame(
            paramsIn.textureID,
            identityMatrix,
            inSize.w,
            inSize.h,
            outSize.w,
            outSize.h,
            paramsOut.fbo,
            quality);
    long end_time = LogUtils::getCurrentMillSeconds();
    MY_LOGD("makeup done consume time = %ld", (end_time - begin_time));
    glFlush();
    mGLContext->createSyncKHR();
    int acquireFenceFd = mGLContext->createFenceFd();
    MY_LOGD("acquireFenceFd = %d", acquireFenceFd);
    out->acquireFenceFd = acquireFenceFd;
    if (out->getImgFormat() != eImgFmt_RGBA8888) {
        BufferUtils::mdpResizeAndConvert(mOutputWorkingBuffer, out);
    }
    GLUtils::releaseEGLImageTexture(&paramsIn);
    GLUtils::releaseEGLImageTexture(&paramsOut);
    FUNCTION_OUT;
}

void VFaceBeauty::makeupForCapture(IImageBuffer *in, IImageBuffer *out, int rotationDegree) {
    FUNCTION_IN;
    MSize inSize = in->getImgSize();
    unsigned char * buffer;
    if (in->getImgFormat() != eImgFmt_RGBA8888) {
        if (mInputWorkingBuffer == nullptr) {
            mInputWorkingBuffer = BufferUtils::acquireWorkingBuffer(inSize,
                    eImgFmt_RGBA8888);
        }
        BufferUtils::mdpResizeAndConvert(in, mInputWorkingBuffer);
        buffer = (unsigned char * )mInputWorkingBuffer->getBufVA(0);
    } else {
        buffer = (unsigned char * )in->getBufVA(0);
    }

    aw_image_info_st* info = new aw_image_info_st();
    info->data = buffer;
    info->w = inSize.w;
    info->h = inSize.h;
    info->size = inSize.w*inSize.h*4;
    MY_LOGD("in size = %d", info->size);
    MY_LOGD("capture makeup begin");
    long begin_time = LogUtils::getCurrentMillSeconds();
    aw_image_info_st* result = mAwFaceBeauty->processArgb(info);
    long end_time = LogUtils::getCurrentMillSeconds();
    MY_LOGD("capture makeup done consume time = %ld", (end_time - begin_time));
    if (result == nullptr) {
        MY_LOGE("result is nullptr!!!!!!");
        return;
    }
    if (mOutputWorkingBuffer == nullptr) {
        mOutputWorkingBuffer = BufferUtils::acquireWorkingBuffer(inSize,
                eImgFmt_RGBA8888);
    }
    memcpy((void *)mOutputWorkingBuffer->getBufVA(0), result->data, result->size);
    MY_LOGD("rotationDegree = %d", rotationDegree);
    BufferUtils::mdpResizeAndConvert(mOutputWorkingBuffer, out, rotationDegree);

    delete info;
    delete [] result->data;
    delete result;
    FUNCTION_OUT;
}

void VFaceBeauty::dumpFaceRect(IImageBuffer *dump_buffer, MRect faceRect) {
    //MSize size = dump_buffer->getImgSize();
    if (dump_buffer->getImgFormat() == eImgFmt_RGBA8888) {
        char * pBufferVa = (char*)dump_buffer->getBufVA(0);
        MY_LOGD("dumpFaceRect  333 PlaneCount  = %d", (int)dump_buffer->getPlaneCount());
        MUINT32 stride = dump_buffer->getBufStridesInBytes(0);
        //int length = dump_buffer->getBufSizeInBytes(0);
        MY_LOGD("dumpFaceRect Face Rect: (xmin, ymin, xmax, ymax) => (%d, %d, %d, %d)",
                faceRect.p.x,
                faceRect.p.y,
                faceRect.s.w,
                faceRect.s.h);
        // draw rectangles to output buffer
        memset(
                pBufferVa + stride * faceRect.p.y + 4 * faceRect.p.x,
                255, 4*(faceRect.s.w + 1));

        memset(
                pBufferVa + stride * (faceRect.p.y + faceRect.s.h) + 4 * faceRect.p.x,
                255, 4*(faceRect.s.w + 1));

        for (size_t j = faceRect.p.y + 1; j < faceRect.p.y + faceRect.s.h ; j++) {
            *(pBufferVa + stride * j + 4 * faceRect.p.x) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 1) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 2) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 3) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 4 * faceRect.s.w) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 4 * faceRect.s.w + 1) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 4 * faceRect.s.w + 2) = 255;
            *(pBufferVa + stride * j + 4 * faceRect.p.x + 4 * faceRect.s.w + 3) = 255;
        }

    }

}

void VFaceBeauty::dumpFaceDetectInfo(IImageBuffer *dump_buffer,
        float* face_point, int face_point_length) {
    if (dump_buffer == nullptr || face_point == nullptr) {
        MY_LOGE("dumpFaceDetectInfo there has nullptr!");
        return;
    }
    int format = dump_buffer->getImgFormat();
    MY_LOGD("dumpFaceDetectInfo format = %d", format);

    MY_LOGD("dumpFaceDetectInfo face_point_length = %d", face_point_length);
    //char * bufferA = (char*)dump_buffer->getBufVA(0);
    MSize size = dump_buffer->getImgSize();
    if (dump_buffer->getImgFormat() == eImgFmt_RGBA8888) {
        char * bufferRGBA = (char*)dump_buffer->getBufVA(0);
        MY_LOGD("dumpFaceDetectInfo PlaneCount  = %d", (int)dump_buffer->getPlaneCount());
        MUINT32 stride = dump_buffer->getBufStridesInBytes(0);
        int length = dump_buffer->getBufSizeInBytes(0);
        MY_LOGD("dumpFaceDetectInfo buffer w=%d, h=%d,s=%d, l=%d", size.w, size.h, stride, length);
        for (int j = 0; j < face_point_length; j += 2) {
            int x = (int)face_point[j];
            int y = (int)face_point[j + 1];
            //MY_LOGD("dumpFaceDetectInfo : (x = %d y = %d)", x, y);
            memset(bufferRGBA + (stride) * (y -1)   + 4 * (x - 1),
                    255, 4);
            memset(bufferRGBA + (stride) * (y -1)   + 4 * x,
                    255, 4);
            memset(bufferRGBA + (stride) * (y -1)   + 4 * (x +1),
                    255, 4);
            memset(bufferRGBA + stride * y   + 4 * x,
                    255, 4);
            memset(bufferRGBA + stride * y   + 4 * (x - 1),
                    255, 4);
            memset(bufferRGBA + stride * y   + 4 * (x + 1),
                    255, 4);
            memset(bufferRGBA + stride * (y + 1)   + 4 * x,
                    255, 4);
            memset(bufferRGBA + stride * (y + 1)   + 4 * (x - 1),
                    255, 4);
            memset(bufferRGBA + stride * (y + 1)   + 4 * (x + 1),
                    255, 4);
        }
    } else {
        char * bufferY = (char*)dump_buffer->getBufVA(0);
        MUINT32 stride = dump_buffer->getBufStridesInBytes(0);
        int length = dump_buffer->getBufSizeInBytes(0);
        MY_LOGD(" buffer w = %d, h = %d,s = %d, length = %d", size.w, size.h, stride, length);
        for (int j = 0; j < face_point_length; j += 2) {
            int x = (int)face_point[j];
            int y = (int)face_point[j + 1];
            MY_LOGD("dumpFaceDetectInfo : (x = %d y = %d)", x, y);
            memset(bufferY + stride * y  + x,
                    255, 1);
        }
    }


}


void VFaceBeauty::dumpFaces(aw_face_info** faces, int face_number) {
    if (faces != nullptr) {
        MY_LOGD("dumpFaces face_number = %d, faces = %p", face_number, faces);
        for (int i = 0; i < face_number; i++) {
            int gender = faces[i]->gender;
            int imgWidth = faces[i]->imgWidth;
            int imgHeight = faces[i]->imgHeight;
            int pointCount = faces[i]->pointCount;
            float *facePoints = faces[i]->facePoints;
            MY_LOGD("dumpFaces g = %d, w = %d, h = %d, c = %d , fp = %p", gender,
                    imgWidth, imgHeight, pointCount, facePoints);
            for (int j = 0; j < pointCount; j++) {
                MY_LOGD("dumpFaces >> facePoints[%d] = %f", j, facePoints[j]);
            }
        }
    } else {
        MY_LOGD("dumpFaceInfo faceInfos is mullptr !!!");
    }
}

void VFaceBeauty::uninit() {
    if (mAwFaceBeauty != nullptr) {
        mAwFaceBeauty->uninit();
        mAwFaceBeauty = nullptr;
    }
    if (mGLContext != nullptr) {
        mGLContext->unInitialize();
        mGLContext = nullptr;
    }
}


