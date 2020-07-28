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

#ifndef FACEDETECT_H_
#define FACEDETECT_H_
#include <stdint.h>
#include <vector>
#include <string>
#include <utils/Thread.h>
#include "aw_face_common.h"
#include "aw_facebeauty_api.h"
#include "aw_face_detect_api.h"
#include <plugin/PipelinePluginType.h>
#include "ProcessingThread.h"

#define MAX_FACE 5
#define FACE_POINTS 190
#define FACE_DETECT_WIDTH 320

class FaceDetect: public Request {
    public:
        int maxFaces = MAX_FACE;
        int points_number = FACE_POINTS;
        char * request_buffer = nullptr;
        aw_face_info * face_info_array[MAX_FACE];
        vector<AwFaceInfo> mInfos;
        int detected_face_number = 0;
        bool is_preview_request = true;
        bool has_init = false;
        aw_face_orientation orientation = FACE_LEFT;
        IImageBuffer* mInputWorkingBuffer = nullptr;

        FaceDetect();
        ~FaceDetect();
        virtual void init();
        virtual void uninit();
        virtual bool processing();
        virtual void generateFaceInfos();
        virtual void getFaceRect(int *face_info_rect, float * outPoints, int index, int length);
        virtual void dupFacePointsFrame(float inPoints[], float * outPoints, int length, int width, int height,
                int faceOrientation, bool isFrontCamera);
        virtual void dumpPoint(float* point, int length);
    protected:
        string modeldir = "/sdcard/modeldir";

};

#endif /* FACEDETECT_H_ */
