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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_INC_POST_PROC_H_
#define _MTK_CAMERA_INC_POST_PROC_H_
//
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>

#include <vector>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>

#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

#include <mtkcam/utils/std/Log.h>

namespace android {
namespace NSPostProc {

using namespace NSCam;
using namespace NSShot;
/******************************************************************************
 * PostProcessorType :
 *      The purpose is to create Processor,
 *      you MUST also add MTK_PLUGIN_MODE to Metadata for ThirdPartyNode to use.
 *      (e.g.: MTK_PLUGIN_MODE_MFNR_3RD_PARTY mode for THIRDPARTY_MFNR type)
 *      If you want to execute mix feature(e.g.: HDR + FD) in ThirdPartyNode
 *      please define your metadata and pass to ThirdPartyNode
 ******************************************************************************/
enum class PostProcessorType
{
    /* internal feature */
    NONE             = 1 << 0,
    DUMMY            = 1 << 1,    // ONLY for test
    MFNR             = 1 << 2,
    HDR              = 1 << 3,
    BOKEH            = 1 << 4,

    /* third party feature */
    THIRDPARTY_MFNR  = 1 << 8,
    THIRDPARTY_HDR   = 1 << 9,
    THIRDPARTY_BOKEH = 1 << 10,
    THIRDPARTY_DCMF  = 1 << 11,
};

/******************************************************************************
 * type for onPostProcEvent
 ******************************************************************************/
enum class PostProcRequestCB
{
    POSTPROC_DONE,
};

/******************************************************************************
 * middleware need to pass <wp>IPostProcRequestCB to processorManager
 * if you want to know when the PostProcess is done for this requestNO
 ******************************************************************************/
class IPostProcRequestCB
    : virtual public android::RefBase
{
public:
    IPostProcRequestCB() = default;
    virtual ~IPostProcRequestCB(){}
    //
    virtual android::status_t onPostProcEvent(
                        MINT32  requestNo,
                        PostProcRequestCB callbackType,
                        MUINT32 streamId,
                        MBOOL bError,
                        void* params1 = nullptr,
                        void* params2 = nullptr) = 0;
};

/******************************************************************************
 * Interface to callback jpeg buffer
 * Each PostProcessor needs to callback jpeg buffer while done
 ******************************************************************************/
class IPostProcImageCallback
    : public virtual android::RefBase
{
public:
    IPostProcImageCallback() = default;
    virtual ~IPostProcImageCallback(){}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool                    onCB_CompressedImage_packed(
                                        PostProcessorType  processType,
                                        int64_t const      i8Timestamp,
                                        uint32_t const     u4BitstreamSize,
                                        uint8_t const*     puBitstreamBuf,
                                        uint32_t const     u4CallbackIndex = 0,
                                        bool     const     fgIsFinalImage = true,
                                        uint32_t const     msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                        uint32_t const     shotMode = eShotMode_ZsdShot
                                    ) = 0;
};

// Data for PostProcessing enqued by Middleware Feature
struct InputData{
        std::vector<android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>>> mImageQueue;
        std::vector<IMetadata>                                                      mAppMetadataQueue;
        std::vector<IMetadata>                                                      mHalMetadataQueue;
};

class ImagePostProcessData : virtual public android::RefBase
{
    public:
        PostProcessorType                                                           mProcessType = PostProcessorType::NONE;
        MINT32                                                                      mRequestNo  = -1;
        MINT64                                                                      miTimeStamp = -1;
        std::vector<InputData>                                                      mvInputData;
        ShotParam                                                                   mShotParam;
        JpegParam                                                                   mJpegParam;
        android::wp<IPostProcRequestCB>                                             mpCb = nullptr;         // notify Middleware this requestNO is done if needed
        MBOOL                                                                       mIsFinalData = MTRUE;   // default is MTURE for backward compatibility
    public:
        MVOID dump(){
            // TODO
            // refactor: use string stream
            String8 str = String8::format("\n[ippd_info] ======= PostProcessData reqNo(%d) type(%d) timestamp(%" PRId64 ") mpCb(%p)======\n",
                mRequestNo, mProcessType, miTimeStamp, mpCb.promote().get()
            );

            // ShotParam & JpegParam
            str = str + String8::format("[ippd_info] ShotParam: picFmt(%d) picSize(%dx%d) JpegParam: quality(%d) thumbQuality(%d) thumbSize(%dx%d) isFinalData(%d)\n",
                mShotParam.miPictureFormat,
                mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight,
                mJpegParam.mu4JpegQuality, mJpegParam.mu4JpegThumbQuality,
                mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight,
                mIsFinalData
            );
            CAM_LOGD("%s", str.string());

            // inputDatas
            MINT32 dataIdx = 0;
            for(auto &e_InputData : mvInputData){
                String8 str = String8::format("data[%d]:\n", dataIdx);

                // image
                MINT32 sensorIdx = 0;
                for(auto &e_imgQue : e_InputData.mImageQueue){
                    str = str + String8::format("[ippd_info] sensor[%d]: ", sensorIdx);
                    for(size_t i = 0 ; i < e_imgQue.size() ; i++){
                        str = str + String8::format("[streamId(0x%x):streamBuf(%p)]", e_imgQue.keyAt(i), e_imgQue.valueAt(i).get());
                    }
                    sensorIdx ++;
                    str = str + String8::format("\n");
                }

                // app meta
                sensorIdx = 0;
                for(auto &e_AppMeta : e_InputData.mAppMetadataQueue){
                    str = str + String8::format("[ippd_info] sensor[%d]: [app metadata buf(%p)]\n", sensorIdx, &e_AppMeta);
                    sensorIdx++;
                }

                // hal meta
                sensorIdx = 0;
                for(auto &e_HalMetaQue : e_InputData.mHalMetadataQueue){
                    str = str + String8::format("[ippd_info] sensor[%d]: [hal metadata buf(%p)]\n", sensorIdx, &e_HalMetaQue);
                    sensorIdx++;
                }

                dataIdx ++;
                CAM_LOGD("%s",  str.string());
            }
            CAM_LOGD("[ippd_info] ==================================");
        }
};

// PostProcessorManager submit setting to specific PostProcessor to execute PostProcessing
class PostProcRequestSetting : virtual public android::RefBase
{
    public:
        PostProcessorType                                                           mProcessType = PostProcessorType::NONE;
        MINT32                                                                      mRequestNo  = -1;
        MINT64                                                                      miTimeStamp = -1;
        std::vector<InputData>                                                      mvInputData;
        ShotParam                                                                   mShotParam;
        JpegParam                                                                   mJpegParam;
        android::sp<IPostProcImageCallback>                                         mpImageCallback = nullptr;   // callback Image to PostProcessorManager
        MBOOL                                                                       mIsFinalData;
    public:
        MVOID dump(){
            String8 str = String8::format("\n[pprs_info] ======= PostProcessData reqNo(%d) type(%d) timestamp(%" PRId64 ") mpImageCallback(%p)======\n",
                mRequestNo, mProcessType, miTimeStamp, mpImageCallback.get()
            );

            // ShotParam & JpegParam
            str = str + String8::format("[pprs_info] ShotParam: picFmt(%d) picSize(%dx%d) JpegParam: quality(%d) thumbQuality(%d) thumbSize(%dx%d) isFinalData(%d)\n",
                mShotParam.miPictureFormat,
                mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight,
                mJpegParam.mu4JpegQuality, mJpegParam.mu4JpegThumbQuality,
                mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight,
                mIsFinalData
            );
            CAM_LOGD("%s", str.string());

            // inputDatas
            MINT32 dataIdx = 0;
            for(auto &e_InputData : mvInputData){
                String8 str = String8::format("data[%d]:\n", dataIdx);

                // image
                MINT32 sensorIdx = 0;
                for(auto &e_imgQue : e_InputData.mImageQueue){
                    str = str + String8::format("[pprs_info] sensor[%d]: ", sensorIdx);
                    for(size_t i = 0 ; i < e_imgQue.size() ; i++){
                        str = str + String8::format("[streamId(0x%x):streamBuf(%p)]", e_imgQue.keyAt(i), e_imgQue.valueAt(i).get());
                    }
                    sensorIdx ++;
                    str = str + String8::format("\n");
                }

                // app meta
                sensorIdx = 0;
                for(auto &e_AppMeta : e_InputData.mAppMetadataQueue){
                    str = str + String8::format("[pprs_info] sensor[%d]: [app metadata buf(%p)]\n", sensorIdx, &e_AppMeta);
                    sensorIdx++;
                }

                // hal meta
                sensorIdx = 0;
                for(auto &e_HalMetaQue : e_InputData.mHalMetadataQueue){
                    str = str + String8::format("[pprs_info] sensor[%d]: [hal metadata buf(%p)]\n", sensorIdx, &e_HalMetaQue);
                    sensorIdx++;
                }

                dataIdx ++;
                CAM_LOGD("%s",  str.string());
            }
            CAM_LOGD("[pprs_info] ==================================");
        }
};

class IImagePostProcessor:
        virtual public android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static android::sp<IImagePostProcessor> createInstance(PostProcessorType type);
    /**
     * @brief submit setting to post processor and execute PostProc
     * @return The process result
     */
    virtual android::status_t                  doPostProc(android::sp<PostProcRequestSetting> setting) = 0;
    /**
     * @brief flush post processor
     * @return The process result
     */
    virtual android::status_t                  flush() = 0;
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    virtual android::status_t                  waitUntilDrained() = 0;
};

class ImagePostProcessorBase:
      public IImagePostProcessor
{
public:
    ImagePostProcessorBase(PostProcessorType /*type*/) {/*mType = type;*/}
public:
    /**
     * @brief submit setting to post processor and execute PostProc
     * @return The process result
     */
    virtual android::status_t                  doPostProc(android::sp<PostProcRequestSetting> setting) = 0;
    /**
     * @brief flush post processor
     * @return The process result
     */
    virtual android::status_t                  flush();
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    virtual android::status_t                  waitUntilDrained();

protected:
//    PostProcessorType mType;
    android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> mpPipeline = nullptr;
};

class IImagePostProcessManager
{
public:
    static IImagePostProcessManager*         getInstance();
    static void                              destroyPostProcessor();
    virtual                                 ~IImagePostProcessManager() {}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief add processor to post process manager.
     * @return The process result
     */
    virtual android::status_t                  addProcesssor(PostProcessorType type, android::sp<IImagePostProcessor> pProcessor) = 0;
    /**
     * @brief remove processor to post process manager.
     * @return The process result
     */
    virtual android::status_t                  clearProcesssor(PostProcessorType type = PostProcessorType::NONE) = 0;
    /**
     * @brief enque to post processor.
     * @return The process result
     */
    virtual android::status_t                  enque(android::sp<ImagePostProcessData> data) = 0;
    /**
     * @brief flush post processor queue.
     * @return The process result
     */
    virtual android::status_t                  flush() = 0;
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    virtual android::status_t                  waitUntilDrained() = 0;
    /**
     * @brief destroy all member field.
     * @return The process result
     */
    virtual android::status_t                  destroy() = 0;
    /**
     * @brief get queue size
     * @return return size
     */
    virtual MINT32                             size() = 0;

    /**
     * @brief set IShotCallback while turn-off fast s2s
     * @return The process result
     */
    virtual android::status_t                  setShotCallback(sp<IShotCallback> pShotCallback) = 0;

    /**
     * @brief check background service is available or not.
     * @return The check result
     */
     virtual MBOOL                              isAvailable() = 0;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  IPostProcImageCallback Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual bool                               onCB_CompressedImage_packed(
                                                    PostProcessorType processType,
                                                    int64_t const     i8Timestamp,
                                                    uint32_t const    u4BitstreamSize,
                                                    uint8_t const*    puBitstreamBuf,
                                                    uint32_t const    u4CallbackIndex = 0,
                                                    bool              fgIsFinalImage = true,
                                                    uint32_t const    msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                                    uint32_t const    shotMode = eShotMode_ZsdShot
                                               ) = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSPostProc
}; // namespace android
#endif  //_MTK_CAMERA_INC_POST_PROC_H_

