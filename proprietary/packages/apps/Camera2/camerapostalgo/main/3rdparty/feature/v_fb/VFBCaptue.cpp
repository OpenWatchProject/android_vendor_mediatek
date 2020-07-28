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
#include <system/graphics-base-v1.0.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "vndk/hardware_buffer.h"
#include "LogUtils.h"
#include "GLContext.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "VFaceBeauty.h"
#include "FaceDetect.h"
#include "VFBImpl.cpp"
#include "FaceDetect.h"

#undef LOG_TAG
#define LOG_TAG "VFBCapture"

//#define DEBUG_FACE_INFO
/******************************************************************************
 *
 ******************************************************************************/

class VFBCapture: public VFBProviderImpl {
    public:
        typedef YuvPlugin::Property Property;
        typedef YuvPlugin::Selection Selection;
        typedef YuvPlugin::Request::Ptr RequestPtr;
        typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;
        typedef YuvPlugin::ConfigParam ConfigParam;

        VFBCapture():VFBProviderImpl(CAPTURE) {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
        ~VFBCapture() {
            FUNCTION_IN;
            FUNCTION_OUT;
        }

        Property& property()
        {
            FUNCTION_IN;
            if (!inited) {
                mProperty.mName = "360 FB CAPTURE";
                mProperty.mFeatures = MTK_FEATURE_VFB_CAPTURE;
                mProperty.mInPlace = MFALSE;
                mProperty.mFaceData = eFD_Current;
                mProperty.mPosition = 0;
                inited = true;

            }
            FUNCTION_OUT;
            return mProperty;
        };

        void init()
        {
            FUNCTION_IN;

            FUNCTION_OUT;
        };

        virtual void doUninit()
        {
            FUNCTION_IN;
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
            MERROR ret = OK;
            if (mHasInit == false) {
                mVFB = new VFaceBeauty();
                mVFB->init(true);
                mHasInit = true;
            }
            IImageBuffer *in = NULL, *out = NULL;

            if (pRequest->mIBufferFull != nullptr) {
                in = pRequest->mIBufferFull->acquire();
            }

            if (pRequest->mOBufferFull != nullptr) {
                out = pRequest->mOBufferFull->acquire();
            }

            if (in != NULL && out != NULL)
            {
                setFaceBeautyLevel(pRequest);
                aw_face_orientation orientation = getFaceOrientation(pRequest);
                MSize inSize = in->getImgSize();
                if (in->getImgFormat() != eImgFmt_RGBA8888) {
                    if (mInputWorkingBuffer == nullptr) {
                        mInputWorkingBuffer = BufferUtils::acquireWorkingBuffer(inSize,
                                eImgFmt_RGBA8888);
                    }
                    BufferUtils::mdpResizeAndConvert(in, mInputWorkingBuffer);
                    mInputRGBABuffer = mInputWorkingBuffer;
                } else {
                    mInputRGBABuffer = in;
                }
                sp<FaceDetect> request = new FaceDetect();
                request->in = mInputRGBABuffer;
                request->is_preview_request = false;
                request->orientation = orientation;
                request->init();
                request->processing();
                request->uninit();
                int face_number = request->detected_face_number;
                MY_LOGD("process faceNumber = %d", face_number);
                int rotationDegree = getJpegRotation(pRequest);
                if (face_number > 0) {
                    aw_face_info face_infos[face_number];
                    for (int i = 0; i < face_number; i++) {
                        face_infos[i] = *((request->face_info_array)[i]);
                    }
                    mVFB->setFaceInfo(face_infos, face_number);
#ifdef DEBUG_FACE_INFO
                    for (int i = 0; i < face_number; i++) {
                        mVFB->dumpFaceDetectInfo(out,
                                (request->face_info_array)[i]->facePoints,
                                (request->face_info_array)[i]->pointCount);
                    }
#endif
                } else {
                    mVFB->setFaceInfo(nullptr, 0);
                }
                mVFB->makeupForCapture(mInputRGBABuffer, out, rotationDegree);

                MY_LOGD("process+ finish >>>>>>");
            }
            FUNCTION_OUT;
            return ret;
        };

        int getJpegRotation(RequestPtr pRequest) {
            MINT32 jpegRotation = 0;
            IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
            if (pImetadata != nullptr && pImetadata->count() > 0) {
                tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_JPEG_ORIENTATION, jpegRotation);
            }
            return jpegRotation;
        }

};

REGISTER_PLUGIN_PROVIDER(Yuv, VFBCapture);
