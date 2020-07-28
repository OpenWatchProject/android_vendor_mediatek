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

#ifndef VFACEBEAUTY_H_
#define VFACEBEAUTY_H_
#include <stdint.h>
#include <vector>
#include <string>
#include <cutils/properties.h>
#include "GLContext.h"
#include "aw_face_detect_api.h"
#include "aw_face_common.h"
#include "aw_facebeauty_api.h"

#include <plugin/PipelinePluginType.h>

using namespace std;

#define FD_QUALITY "debug.camerapostalgo.feature.fb.quality"
static int fd_quality = ::property_get_int32(FD_QUALITY, 70);

class VFaceBeauty {
    public:
        GLContext *mGLContext = nullptr;
        AwFaceBeauty * mAwFaceBeauty = nullptr;
        float quality = 0.8f;
        IImageBuffer* mOutputWorkingBuffer = nullptr;
        IImageBuffer* mInputWorkingBuffer = nullptr;

        bool hasInitFaceDetect = false;
        float identityMatrix[16] =
           {1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};

        VFaceBeauty();
        ~VFaceBeauty();
        void init(bool isCapture);
        char* getRGBABuffer(IImageBuffer *in);
        void makeupForPreview(IImageBuffer *in, IImageBuffer *out);
        void makeupForCapture(IImageBuffer *in, IImageBuffer *out, int rotationDegree);
        void dupFacePointsFrame(float inPoints[], float *outPoints, int length, int width, int height,
                int faceOrientation, bool isFrontCamera);
        void setBeautyLevel(aw_beauty_type type, float value);
        void releaseWorkingBuffer(IImageBuffer *buf);
        void dumpFaceDetectInfo(IImageBuffer *dump_buffer, float* face_point, int face_point_length);
        void dumpFaces(aw_face_info** faces, int face_number);
        void dumpFaceRect(IImageBuffer *dump_buffer, MRect faceRect);
        void setFaceInfo(aw_face_info faceInfo[], int faceNum);
        void uninit();
};
#endif /* VFACEBEAUTY_H_ */
