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

//
#include <stdio.h>
#include <cutils/properties.h>
#include <utils/std/Format.h>
#include <system/graphics-base-v1.0.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "LogUtils.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "aw_face_common.h"
#include "aw_facebeauty_api.h"
#include "TaskThread.h"
#include "FaceDetect.h"
#include "VFaceBeauty.h"

#undef LOG_TAG
#define LOG_TAG "VFBProvider"

using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;

/******************************************************************************
 *
 ******************************************************************************/

class VFBProviderImpl : public YuvPlugin::IProvider
{
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;


    private:
    enum TaskType {
        INIT,
        PROCESS,
        UNINIT,
        EXIT,
        UNKNOWN,
    };

    class FBTask : public Task {
        private:
            TaskType type = UNKNOWN;
            VFBProviderImpl* fb = nullptr;
            RequestPtr request;
            RequestCallbackPtr callback;

        public:
            FBTask() {}
            FBTask(TaskType t, VFBProviderImpl* f) : type(t), fb(f) {}
            FBTask(TaskType t, VFBProviderImpl* f, RequestPtr r) : type(t), fb(f), request(r) {}
            void run() {
                switch (type) {
                    case INIT:
                        fb->doInit();
                        break;
                    case UNINIT:
                        fb->doUninit();
                        break;
                    case PROCESS:
                        fb->doProcess(request);
                        break;
                    case EXIT:
                        fb->mTaskThread->exit();
                        break;
                    case UNKNOWN:
                        break;
                }
            }
    };

    protected:
    int    mOpenid;
    bool mHasInit = false;
    const mtk_platform_metadata_tag_t VFB_TAG[7] =
    {   MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE,
        MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL,

    };
    enum FBType {
        PREVIEW,
        CAPTURE,
    };

    FBType mFBType;
    Property mProperty;
    bool inited = false;
    TaskThread<FBTask>* mTaskThread = nullptr;
    VFaceBeauty *mVFB = nullptr;
    IImageBuffer* mInputWorkingBuffer = nullptr;
    IImageBuffer* mInputRGBABuffer = nullptr;

    public:

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        FUNCTION_IN;
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
        mOpenid = iOpenId;
        FUNCTION_OUT;
    }

    virtual Property& property()
    {
        FUNCTION_IN;
        FUNCTION_OUT;
        return mProperty;
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

    virtual MERROR process(RequestPtr pRequest,
            RequestCallbackPtr pCallback = nullptr)
    {
        FUNCTION_IN;
        mTaskThread->postTaskAndWaitDone(FBTask(PROCESS, this, pRequest));
        if (pCallback != nullptr) {
            MY_LOGD("callback request");
            pCallback->onCompleted(pRequest, OK);
        }
        FUNCTION_OUT;
        return OK;
    };

    aw_beauty_type getBeautyType(mtk_platform_metadata_tag_t metadataType) {
        switch(metadataType) {
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE:
                return SMALL_FACE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE:
                return BIG_EYE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE:
                return BRIGHT_EYE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE:
                return BIG_NOSE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE:
                return SMOOTH_FACE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE:
                return BRIGHT_WHITE;
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL:
                return BEAUTY_ALL;
            default:
                return BEAUTY_ALL;
        }
    }

    string getTagName(mtk_platform_metadata_tag_t metadataType) {
        switch(metadataType) {
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE:
                return "SMALL_FACE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE:
                return "BIG_EYE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE:
                return "BRIGHT_EYE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE:
                return "BIG_NOSE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE:
                return "SMOOTH_FACE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE:
                return "BRIGHT_WHITE";
            case MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL:
                return "BEAUTY_ALL";
            default:
                return "BEAUTY_ALL";
        }
    }


    aw_face_orientation getFaceOrientation(RequestPtr pRequest) {
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        int face_orientation = 1;
        if (pImetadata->count() > 0) {
            ///////////////////Get camera id ///////////////////////////
            tryGetMetadata<MINT32>(pImetadata,
                    MTK_POSTALGO_VENDOR_FACE_BEAUTY_FACEORIENTATION, face_orientation);
            MY_LOGD("current face_orientation = %d", face_orientation);
        }
        switch(face_orientation) {
            case 0:
                return FACE_UP;
            case 1:
                return FACE_LEFT;
            case 2:
                return FACE_DOWN;
            case 3:
                return FACE_RIGHT;
            default:
                return FACE_LEFT;
        }
    }

    void setFaceRect(RequestPtr pRequest, MRect* faceRect) {
        if (faceRect != nullptr) {
            IMetadata* pOmetadata = pRequest->mOMetadataApp->acquire();
            if (pOmetadata != nullptr && pOmetadata->count() > 0) {
                updateEntry<MRect>(pOmetadata,
                        MTK_POSTALGO_VENDOR_FACE_BEAUTY_FACE_DETECTION, *faceRect);
            }
        }
    }

    void setFaceBeautyLevel(RequestPtr pRequest) {
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata->count() > 0) {
            ////////////////////Get fb value///////////////////////////
            int size = sizeof(VFB_TAG)/sizeof(VFB_TAG[0]);
            MY_LOGD("getMetaData size = %d", size);
            for (int i = 0; i < size; i++) {
                mtk_platform_metadata_tag_t type = VFB_TAG[i];
                aw_beauty_type beauty_type ;
                int value = 0;
                tryGetMetadata<MINT32>(pImetadata, type, value);
                beauty_type = getBeautyType(type);
                string type_name = getTagName(type);
                char * str = const_cast<char*>(type_name.c_str());
                float level = value/100.0;
                mVFB->setBeautyLevel(beauty_type, level);
                MY_LOGD("setBeautyLevel VFB_TAG = %s, beauty_type = %d, level = %f",
                        str,
                        beauty_type,
                        level);
            }
        }

    }

    template <typename T>
        inline bool updateEntry(
                IMetadata* metadata, const MUINT32 tag, const T& val)
        {
            if (metadata == NULL)
            {
                MY_LOGD("updateEntry pMetadata is NULL");
                return false;
            }

            IMetadata::IEntry entry(tag);
            entry.push_back(val, Type2Type<T>());
            metadata->update(tag, entry);
            return true;
        }

    template <class T>
        inline bool
        tryGetMetadata( IMetadata const *pMetadata, mtk_platform_metadata_tag_t tag, T& rVal )
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
                MY_LOGW("no metadata %d ", tag);
            }
            return false;
        }

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        FUNCTION_IN;

        FUNCTION_OUT;
    };

    virtual void init()
    {
        FUNCTION_IN;
        mTaskThread->postTaskAndWaitDone(FBTask(INIT, this));
        FUNCTION_OUT;
    };

    virtual void uninit()
    {
        FUNCTION_IN;
        mTaskThread->postTaskAndWaitDone(FBTask(UNINIT, this));
        FUNCTION_OUT;
    };

    VFBProviderImpl(FBType type)
        :mOpenid(-1)
    {
        FUNCTION_IN;
        mFBType = type;
        mTaskThread = new TaskThread<FBTask>();
        FUNCTION_OUT;
    };

    virtual ~VFBProviderImpl()
    {
        FUNCTION_IN;
        mTaskThread->postTaskAndWaitDone(FBTask(EXIT, this));
        mTaskThread->join();
        delete mTaskThread;
        FUNCTION_OUT;
    };

    virtual void doUninit(){};
    virtual MERROR doProcess(RequestPtr pRequest) {return OK;};
    virtual void doInit() {};

};
