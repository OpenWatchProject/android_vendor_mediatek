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

#ifndef _MTK_CAMERA_STEREO_FLOW_CONTROL_SHOT_CALLBACK_PROCESSOR_H_
#define _MTK_CAMERA_STEREO_FLOW_CONTROL_SHOT_CALLBACK_PROCESSOR_H_

#include <list>
#include <set>
// Module header file
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
// Local header file
#include "../ICaptureRequestCB.h"
#include "../builder/ICaptureRequestBuilder.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace android::NSShot;
using namespace NSCam::v1;
using namespace android::NSPostProc;

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
class IImageShotCallback;
class IMetaShotCallback;
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class ImageCallbackPool
 * @brief Store and maintain IImageShotCallback.
 */
class ImageCallbackPool
{
public:
    ImageCallbackPool() = default;
    /**
     * @brief add callback for image.
     * @param [in] callback The callback to process specific image stream.
     * @return The process result
     */
    status_t addCallback(sp<IImageShotCallback>& callback);
    /**
     * @brief clear pool data.
     * @return The process result
     */
    status_t clearPool();
    /**
     * @brief get callback for image.
     * @param [in] Query callback by streamId.
     * @param [in, out] callback instance
     * @return If return value is true, it means current callback is valid.
     */
    MBOOL getCallback(MINT32 streamId, sp<IImageShotCallback>& callback);
private:
    //
    DefaultKeyedVector<MINT32, sp<IImageShotCallback> >
                            mvImageShotCallbackPool;
};
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class MetaCallbackPool
 * @brief Store and maintain IMetaShotCallback
 */
class MetaCallbackPool
{
public:
    MetaCallbackPool() = default;
    /**
     * @brief add callback for metadata.
     * @param [in] callback The callback to process specific metadata.
     * @return The process result
     */
    status_t addCallback(sp<IMetaShotCallback>& callback);
    /**
     * @brief clear pool data.
     * @return The process result
     */
    status_t clearPool();
    /**
     * @brief get callback for metadata.
     * @param [in] Query callback by streamId.
     * @param [in, out] callback instance
     * @return If return value is true, it means current callback is valid.
     */
    MBOOL getCallback(MINT32 streamId, sp<IMetaShotCallback>& callback);
private:
    //
    DefaultKeyedVector<MINT32, sp<IMetaShotCallback> >
                            mvMetaShotCallbackPool;
};
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class ExtraDataImageCallback
 * @brief Processing extra data image callback.
 */
class ShotCallbackProcessor:
      public NSCam::v1::ResultProcessor::IListener,
      public NSCam::v1::IImageCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ShotCallbackProcessor() = delete;
    ShotCallbackProcessor(
                const char* pszShotName,
                CaptureRequestInitSetting &setting);
    ~ShotCallbackProcessor();
    /**
     * @brief add callback for image.
     * @param [in] imageCallback The callback to process specific image stream.
     * @return The process result
     */
    virtual status_t          addCallback(sp<IImageShotCallback>& imageCallback);
    /**
     * @brief add callback for metadata.
     * @param [in] metaCallback The callback to process specific metadata.
     * @return The process result
     */
    virtual status_t          addCallback(sp<IMetaShotCallback>& metaCallback);
    /**
     * @brief clear mvImageShotCallbackConfigList and mvMetaShotCallbackConfigList.
     * @return The process result
     */
    virtual status_t          resetProcessor();
    /**
     * @brief destroy all member field.
     * @return The process result
     */
    virtual status_t          destroy();
    /**
     * @brief add PostProcInfo
     * @return The process result
     */
    virtual status_t          addPostProcInfo(StreamId_T const streamId, IMetadata  const result, MINT64 const timestamp, StereoShotParam const shotParams);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ResultProcessor::IListener interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual void              onResultReceived(
                                        MUINT32    const requestNo,
                                        StreamId_T const streamId,
                                        MBOOL      const errorResult,
                                        IMetadata  const result) override;
    void                    onFrameEnd(
                                        MUINT32         const requestNo) override {};
    String8                 getUserName() override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageCallback interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MERROR          onResultReceived(
                                        MUINT32    const requestNo,
                                        StreamId_T const streamId,
                                        MBOOL      const errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer) override;
protected:
    virtual status_t        readyToPostProcessor();
    void                    prepareMetaCheckSet();
    void                    checkMetadataType(MINT32 streamId, MBOOL& isAppMeta);
    void                    sendImageCallback(
                                        sp<IImageShotCallback> &pImgShotCb,
                                        android::sp<IImageBuffer> &pImgBuf);
    void                    pushToPostProcQueue(
                                        sp<IImageShotCallback> &pImgShotCb);
    void                    sendEvent(
                                        StreamId_T const &streamId,
                                        MBOOL      const &errorBuffer,
                                        MBOOL isFinalImage,
                                        sp<IImageShotCallback> &pImgShotCb);
    void                    sendDoneCallback();
    void                    callbackImage(
                                        MUINT32    const &requestNo,
                                        StreamId_T const &streamId,
                                        MBOOL      const &errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer,
                                        MBOOL isFinalImage,
                                        sp<IImageShotCallback> &callback);
protected:
    Mutex                   mMetadataLock;
    Mutex                   mImgResultLock;
    const char*             mShotName;
    //
    ImageCallbackPool       mvImageCallbackPool;
    //
    MetaCallbackPool        mvMetaCallbackPool;
    //
    sp<IShotCallback>       mpShotCallback = nullptr;
    //
    MBOOL                   mbDumpResult   = MFALSE;
    //
    std::string             msFilename     = "";
    //
    MINT8                   miImgCount     = 0;
    MINT8                   miTotalImgCount = 0;
    MINT8                   miMetaCount     = 0;
    MINT8                   miTotalMetaCount = 0;
    MBOOL                   mbDepthMeta = MFALSE;
    //
    wp<ICaptureRequestCB> mpCb = nullptr;
    //
    MINT32                  miCaptureNo = 0;
    // result queue
    std::list< sp<IImageShotCallback> > mImgCallbackQue;
    std::list< android::sp<IImageBuffer> > mImgBufferQue;
    //
    Mutex                   mPostProcessorLock;
    MBOOL                   mbSupportPostProcessor = MFALSE;
    MBOOL                   mbImgPushDone = MFALSE;
    MBOOL                   mbMetaPushDone = MFALSE;
    MBOOL                   mbEnqueDone    = MFALSE;
    sp<ImagePostProcessData>   mPostProcess_DataQueue;
    std::set<MINT32>        appMetaDefinitionSet;
    std::set<MINT32>        halMetaDefinitionSet;
    IMetadata               appMetadata;
};
};
};
};
#endif  //  _MTK_CAMERA_STEREO_FLOW_CONTROL_SHOT_CALLBACK_MANAGER_H_
