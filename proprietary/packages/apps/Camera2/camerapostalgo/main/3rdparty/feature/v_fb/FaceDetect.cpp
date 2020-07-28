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

#include <utils/Thread.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include "FaceDetect.h"
#include "LogUtils.h"
#include "aw_face_common.h"
#include "aw_facebeauty_api.h"
#include "aw_face_detect_api.h"
#include "BufferUtils.h"

#define LOG_TAG "PostAlgo/VFaceDetect"
//#define DEBUG_FACE_INFO

using namespace NSCam::NSPipelinePlugin;

FaceDetect::FaceDetect() {
    MY_LOGD("FaceDetect!!!!maxFaces = %d", maxFaces);
    for (int i = 0; i < maxFaces; i++) {
        aw_face_info *  current_aw_face_info = new aw_face_info();
        float *face_points = new float[FACE_POINTS];
        current_aw_face_info->gender = -1;
        current_aw_face_info->imgWidth = -1;
        current_aw_face_info->imgHeight = -1;
        current_aw_face_info->pointCount = -1;
        current_aw_face_info->facePoints = face_points;
        face_info_array[i] = current_aw_face_info;
    }

}

FaceDetect::~FaceDetect() {
    MY_LOGD("~~~FaceDetect!!!!maxFaces = %d", maxFaces);
    for (int i = 0; i < maxFaces; i++) {
        aw_face_info * current_aw_face_info = face_info_array[i];
        delete [] current_aw_face_info->facePoints;
        delete current_aw_face_info;
    }

}

void FaceDetect::uninit() {
    MY_LOGD("uninit");
    if (is_preview_request) {
        has_init = false;
        aw_face_detect_api_destroy();
    }
    if (request_buffer != nullptr) {
        delete[] request_buffer;
        request_buffer = nullptr;
    }
}

void FaceDetect::init() {
    FUNCTION_IN;
    if (!has_init) {
        char *str = const_cast<char *>(modeldir.c_str());
        MY_LOGD("modeldir x=%s", str);
        aw_face_detect_api_init(str, LOW_PRECISION);
        has_init= true;
    }

    MSize inSize = in->getImgSize();

#ifdef DEBUG_FACE_INFO
    imageWidth = inSize.w;
    imageHeight = inSize.h;
#else
    imageWidth = FACE_DETECT_WIDTH;
    imageHeight = (FACE_DETECT_WIDTH*inSize.h)/inSize.w;
#endif

    MY_LOGD("init w = %d,h = %d, wf = %d, hf=%d",
        inSize.w, inSize.h,imageWidth, imageHeight);
    int buffer_size;
    MSize size_face_detect(imageWidth, imageHeight);
    // RGBA format for face detect.
    buffer_size = imageWidth*imageHeight*4;
    if (this->request_buffer == nullptr) {
        this->request_buffer = new char[buffer_size];
    }
    if (mInputWorkingBuffer == nullptr) {
        mInputWorkingBuffer = BufferUtils::acquireWorkingBuffer(size_face_detect,
                                                                eImgFmt_RGBA8888);
    }
    BufferUtils::mdpResizeAndConvert(in, mInputWorkingBuffer);
    memcpy((void *)(this->request_buffer), (void *)(mInputWorkingBuffer->getBufVA(0)), buffer_size);

    FUNCTION_OUT;
}

bool FaceDetect::processing() {

    this->mInfos.clear();
    long begin_time = LogUtils::getCurrentMillSeconds();
    if (is_preview_request) {
        // YV12 format for preview face detect.
        aw_face_detect_api_detectBuffer((unsigned char*)request_buffer,
                this->imageWidth,
                this->imageHeight,
                PIX_FMT_RGBA8888,
                this->orientation,
                RESIZE_320,
                10,
                false,
                this->mInfos);
    } else {
        // RGBA format for capture face detect.
        aw_face_detect_api_detectPicData((unsigned char *)(request_buffer),
                this->imageWidth,
                this->imageHeight,
                PIX_FMT_RGBA8888,
                this->orientation,
                RESIZE_320,
                false,
                this->mInfos);
    }
    long end_time = LogUtils::getCurrentMillSeconds();
    MY_LOGD("facedetect for %s consume time = %ld",
            is_preview_request? "preview":"capture", (end_time - begin_time));
    generateFaceInfos();
    return true;
}

void FaceDetect::generateFaceInfos() {
    int number_of_face = (this->mInfos).size();
    MY_LOGD("generateFaceInfos number_of_face =%d", number_of_face);
    detected_face_number = maxFaces < number_of_face ? maxFaces:number_of_face;

    for (int i = 0; i < detected_face_number; i++) {
        AwFaceInfo faceInfo = mInfos[i];
        vector<float> points5 = faceInfo.points5;
        vector<float> points_fine = faceInfo.points_fine;
        vector<int> attrs = faceInfo.attrs;
        float * facepoint = points_fine.data();
        int number_of_facepoints = (int)points_fine.size();
        aw_face_info* one_of_face_info = face_info_array[i];

        one_of_face_info->gender = 1;
        one_of_face_info->imgWidth = this->imageWidth;
        one_of_face_info->imgHeight = this->imageHeight;
        one_of_face_info->pointCount = number_of_facepoints;
        dupFacePointsFrame(
                facepoint,
                one_of_face_info->facePoints,
                number_of_facepoints,
                one_of_face_info->imgWidth,
                one_of_face_info->imgHeight,
                this->orientation,
                false);
        MY_LOGD("generateFaceInfos number_of_face =%d", number_of_face);
        MY_LOGD("generateFaceInfos gender-> gender=%d",one_of_face_info->gender);
        MY_LOGD("generateFaceInfos width =%d",one_of_face_info->imgWidth);
        MY_LOGD("generateFaceInfos height =%d",one_of_face_info->imgHeight );
        MY_LOGD("generateFaceInfos pointCount =%d",one_of_face_info->pointCount );
    }

}

void FaceDetect::dumpPoint(float* point, int length) {
    for (int i = 0; i < length; i++) {
        MY_LOGD("dumpPoint point[%d]=%f",i, point[i]);
    }
}

void FaceDetect::getFaceRect(int *face_info_rect, float * outPoints, int index, int length) {
    float minX = 0.0F;
    float minY = 0.0F;
    float maxX = 0.0F;
    float maxY = 0.0F;

    for(int i = 0; i < length/2; ++i) {
        if(minX == 0.0F || minX > outPoints[i * 2]) {
            minX = outPoints[i * 2];
        }

        if(minY == 0.0F || minY > outPoints[i * 2 + 1]) {
            minY = outPoints[i * 2 + 1];
        }

        if(maxX == 0.0F || maxX < outPoints[i * 2]) {
            maxX = outPoints[i * 2];
        }

        if(maxY == 0.0F || maxY < outPoints[i * 2 + 1]) {
            maxY = outPoints[i * 2 + 1];
        }
    }

    int top = (int)minY;
    int bottom = (int)maxY;
    int left = (int)minX;
    int right = (int)maxX;
    face_info_rect[index*4 + 0] = left;
    face_info_rect[index*4 + 1] = top;
    face_info_rect[index*4 + 2] = right;
    face_info_rect[index*4 + 3] = bottom;
    MY_LOGD("getFaceRect x=%d, y = %d, width = %d, height = %d", left, top,
            (right - left), (bottom - top));
}

void FaceDetect::dupFacePointsFrame(float inPoints[],
        float * outPoints, int length, int width, int height,
        int faceOrientation, bool isFrontCamera) {
    MY_LOGD("dupFacePointsFrame length = %d, w = %d, h = %d, orientation = %d",
         length, width, height, faceOrientation);
    if (nullptr == inPoints) {
        return;
    }

    if (isFrontCamera) {
        switch (faceOrientation) {
            case FACE_UP:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = inPoints[i];
                    outPoints[i + 1] = inPoints[i + 1];
                }
                break;

            case FACE_RIGHT:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = width - inPoints[i + 1];
                    outPoints[i + 1] = inPoints[i];
                }
                break;

            case FACE_DOWN:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = width - inPoints[i];
                    outPoints[i + 1] = height - inPoints[i + 1];
                }
                break;

            case FACE_LEFT:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = inPoints[i + 1];
                    outPoints[i + 1] = height - inPoints[i];
                }
                break;
        }
    } else {
        switch (faceOrientation) {
            case FACE_UP:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = inPoints[i];
                    outPoints[i + 1] = inPoints[i + 1];
                }
                break;

            case FACE_RIGHT:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = width - inPoints[i + 1];
                    outPoints[i + 1] = inPoints[i];
                }
                break;

            case FACE_DOWN:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = width - inPoints[i];
                    outPoints[i + 1] = height - inPoints[i + 1];
                }
                break;

            case FACE_LEFT:
                for (int i = 0; i < length; i += 2) {
                    outPoints[i] = inPoints[i + 1];
                    outPoints[i + 1] = height - inPoints[i];
                    //MY_LOGE("dupFacePointsFrame points = %f, %f", inPoints[i], inPoints[i + 1]);
                }
                break;
        }
    }
}


