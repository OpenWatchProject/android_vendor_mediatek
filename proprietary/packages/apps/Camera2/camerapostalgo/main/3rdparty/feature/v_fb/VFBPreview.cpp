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

#include <stdio.h>
#include <cutils/properties.h>
#include <utils/std/Format.h>
#include <sys/time.h>
#include <system/graphics-base-v1.0.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "vndk/hardware_buffer.h"
#include "LogUtils.h"
#include "GLContext.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "VFBImpl.cpp"
#include "ProcessingThread.h"
#include "FaceDetect.h"
#include "VFaceBeauty.h"

#undef LOG_TAG
#define LOG_TAG "VFBPreview"
#define INTERVAL_FRAME 3
//#define DEBUG_FACE_INFO
//#define DEBUG_DUMP_BUFFER

class VFBPreview: public VFBProviderImpl{
    public:
        typedef YuvPlugin::Property Property;
        typedef YuvPlugin::Selection Selection;
        typedef YuvPlugin::Request::Ptr RequestPtr;
        typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;
        typedef YuvPlugin::ConfigParam ConfigParam;

        sp<ProcessingThread> mProcessingThread =  new ProcessingThread();
        sp<FaceDetect> request = nullptr;
        int m_face_number = 0;
        MRect* faceRect = nullptr;
        long frame_count = 0;

        VFBPreview():VFBProviderImpl(PREVIEW) {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
        ~VFBPreview() {
            FUNCTION_IN;
            FUNCTION_OUT;
        }

        virtual Property& property()
        {
            FUNCTION_IN;
            if (!inited) {
                mProperty.mName = "360 FB PREVIEW";
                mProperty.mFeatures = MTK_FEATURE_VFB_PREVIEW;
                mProperty.mInPlace = MFALSE;
                mProperty.mFaceData = eFD_Current;
                mProperty.mPosition = 0;
                inited = true;
            }
            FUNCTION_OUT;
            return mProperty;
        };

        virtual void doInit() {
            FUNCTION_IN;
            frame_count = 0;
            FUNCTION_OUT;
        };

        virtual void doUninit()
        {
            FUNCTION_IN;
            MY_LOGW("doUninit");
            if (request != nullptr) {
                request->uninit();
                request = nullptr;
            }
            if (mVFB != nullptr) {
                mVFB->uninit();
                mVFB = nullptr;
                mHasInit = false;
            }
            FUNCTION_OUT;
        };

        virtual MERROR doProcess(RequestPtr pRequest)
        {
            FUNCTION_IN;
            FPS;
            MERROR ret = OK;
            if (mHasInit == false) {
                mVFB = new VFaceBeauty();
                mVFB->init(false);
                if (mProcessingThread != nullptr) {
                    mProcessingThread->run("ProcessingThread", PRIORITY_DEFAULT);
                }
                mHasInit = true;
            }
            MY_LOGD("process current pRequest mRequestNo = %d, frameNo = %d", pRequest->mRequestNo,
            pRequest->mFrameNo);
            IImageBuffer *in = NULL, *out = NULL;

            if (pRequest->mIBufferFull != nullptr) {
                in = pRequest->mIBufferFull->acquire();
            }

            if (pRequest->mOBufferFull != nullptr) {
                out = pRequest->mOBufferFull->acquire();
                MY_LOGD("process current ImageBuffer  = %p", out);
            }

            if (in != NULL && out != NULL)
            {
                MY_LOGD("process current mTimestamp = %ld", (long)(in->getTimestamp()));
                setFaceBeautyLevel(pRequest);
                aw_face_orientation orientation = getFaceOrientation(pRequest);
                MY_LOGD("Get current face orientation  = %d", orientation);

                if (facedetectCallback == nullptr) {
                    facedetectCallback = new FaceDetectCallback(this);
                }
                MY_LOGD("m_face_number = %d, count = %ld", m_face_number, frame_count);
                if (request == nullptr) {
                    request = new FaceDetect();
                }
                if (frame_count%INTERVAL_FRAME == 0) {
                    ///////////////// callback fact rect/////////////////////////
                    setFaceRect(pRequest, faceRect);
                    //////////////////// send new request ////////////////////////
                    request->in = in;
                    request->is_preview_request = true;
                    request->orientation = orientation;
                    mProcessingThread->sendRequest(request, facedetectCallback);
                }
                frame_count++;
                MY_LOGD("MakeupForPreview >>>>>>>>>>>>>>>>>>");
#ifdef DEBUG_DUMP_BUFFER
                string inbuffer = std::to_string(pRequest->mRequestNo) +"inbuffer";
                BufferUtils::dumpBuffer(in, const_cast<char*>(inbuffer.c_str()));
#endif
                mVFB->makeupForPreview(in, out);
#ifdef DEBUG_DUMP_BUFFER
                string outbuffer = std::to_string(pRequest->mRequestNo) + "fb_outbuffer";
                BufferUtils::dumpBuffer(out, const_cast<char*>(outbuffer.c_str()));
#endif
                MY_LOGD("MakeupForPreview <<<<<<<<<<<<<<<<<<");
                if (m_face_number > 0) {
                    ///////////////Add debug info to display///////////////////////////////////
#ifdef DEBUG_FACE_INFO
                    for (int k = 0; k < m_face_number; k++) {
                        mVFB->dumpFaceDetectInfo(out, (request->face_info_array)[k]->facePoints,
                                (request->face_info_array)[k]->pointCount);
                    }
                    mVFB->dumpFaceRect(out, *faceRect);
#endif
                    /////////////////////////////////////////////////////////

                }
            }
            FUNCTION_OUT;
            return ret;
        };

        class FaceDetectCallback: public RequestCallback {
            private:
                VFBPreview *mImpl;
                VFaceBeauty *mFacebeauty;
            public:
                FaceDetectCallback(VFBPreview *impl) {
                    mImpl = impl;
                    mFacebeauty = impl->mVFB;
                }
                virtual ~FaceDetectCallback() {
                }
                virtual void onCompleted(sp<Request> current_request) {
                    MY_LOGW("onCompleted mImpl = %p", mImpl);
                    if (mImpl != nullptr) {
                        MY_LOGW("onCompleted mImpl->mVFB  = %p", mImpl->mVFB );
                    }
                    if (mImpl != nullptr && mImpl->mVFB != nullptr) {
                        FaceDetect * face_info = static_cast<FaceDetect*>(current_request.get());
                        int face_number = face_info->detected_face_number;
                        MY_LOGW("onCompleted face_number = %d", face_number);
                        mImpl->m_face_number = face_number;
                        if (face_number > 0) {
                            /////////////////////////Generate face rect for faceRect display///////
                            int face_info_rect[face_info->maxFaces*4];
                            aw_face_info ** face_info_array = face_info->face_info_array;
                            for (int i = 0; i < face_number; i++) {
                                face_info->getFaceRect(
                                        face_info_rect,
                                        face_info_array[i]->facePoints,
                                        i,
                                        face_info_array[i]->pointCount);
                            }
                            int left = face_info_rect[0];
                            int top = face_info_rect[1];
                            int right = face_info_rect[2];
                            int bottom = face_info_rect[3];
                            mImpl->faceRect = new MRect(MPoint(left, top), MPoint(right, bottom));
                            MY_LOGW("face rect left = %d, top = %d, right = %d, bottom = %d",
                                    left,
                                    top,
                                    right,
                                    bottom);
                            ///////////////////////////////////////////////////////////////////

                            //////////// create face info struct for fb. //////////////////////
                            MY_LOGW("onCompleted >>>>>>>>>>>>>>>>>>");
                            aw_face_info face_infos[face_number];
                            for (int i = 0; i < face_number; i++) {
                                face_infos[i] = *((face_info->face_info_array)[i]);
                            }
                            mFacebeauty->setFaceInfo(face_infos, face_number);
                            MY_LOGW("onCompleted <<<<<<<<<<<<<<<<<<");
                            /////////////////////////////////////////////////////////////
                        } else {
                            mFacebeauty->setFaceInfo(nullptr, 0);
                        }
                    }
                }
        };
        sp<FaceDetectCallback> facedetectCallback ;

};

REGISTER_PLUGIN_PROVIDER(Yuv, VFBPreview);
