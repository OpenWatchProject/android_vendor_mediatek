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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>

#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#include <mtkcam/middleware/v1/ICommonCapturePipeline.h>
#endif

namespace android {
namespace NSShot {
namespace ZShot {
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
/******************************************************************************
 *
 ******************************************************************************/
class CallbackListener : public virtual RefBase
{
public:
    virtual         ~CallbackListener() {}
public:
    virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    )                                       = 0;
    virtual MVOID   onDataReceived(
                        MUINT32 const requestNo,
                        StreamId_T const streamId,
                        MBOOL const errorResult,
                        android::sp<IImageBuffer>& pBuffer
                    )                                       = 0;
};

/*******************************************************************************
*
********************************************************************************/
class ImageCallback
    : public IImageCallback
{
public:
                                        ImageCallback(
                                            sp<CallbackListener> pListener,
                                            MUINT32 const data
                                            )
                                            : mpListener(pListener)
                                            , mData(data)
                                        {}
public:   ////    interface of IImageCallback
    /**
     *
     * Received result buffer.
     *
     * @param[in] RequestNo : request number.
     *
     * @param[in] pBuffer : IImageBuffer.
     *
     */
    virtual MERROR                      onResultReceived(
                                            MUINT32 const              RequestNo,
                                            StreamId_T const           streamId,
                                            MBOOL   const              errorBuffer,
                                            android::sp<IImageBuffer>& pBuffer
                                        ) {
                                            sp<CallbackListener> pListener = mpListener.promote();
                                            if( pListener != NULL )
                                                pListener->onDataReceived(
                                                    RequestNo, streamId, errorBuffer, pBuffer
                                                    );
                                            return OK;
                                        }
protected:
    wp<CallbackListener>                mpListener;
    MUINT32 const                       mData;
};

/*******************************************************************************
*
********************************************************************************/
class MetadataListener
    : public ResultProcessor::IListener
{
public:
                                        MetadataListener(
                                            CallbackListener* pListener
                                            )
                                            : mpListener(pListener)
                                        {}
public:   ////    interface of IListener
    virtual void                        onResultReceived(
                                            MUINT32         const requestNo,
                                            StreamId_T      const streamId,
                                            MBOOL           const errorResult,
                                            IMetadata       const result
                                        ) {
                                            if( mpListener )
                                                mpListener->onMetaReceived(
                                                    requestNo, streamId, errorResult, result
                                                    );
                                        }

    virtual void                        onFrameEnd(
                                            MUINT32         const /*requestNo*/
                                        ) {};

    virtual String8                     getUserName() { return String8(LOG_TAG); }

protected:
    CallbackListener*                   mpListener;
};
#endif // (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)

/******************************************************************************
 *
 ******************************************************************************/
class ZsdShot
    : public ImpShot
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    , public SCallbackListener
#else
    , public CallbackListener
#endif
{
typedef NSCam::v1::ISelector::BufferItemSet BufferItemSet;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~ZsdShot();
                                    ZsdShot(
                                        char const*const pszShotName,
                                        uint32_t const u4ShotMode,
                                        int32_t const i4OpenId
                                    );

public:     ////                    Operations.

    //  This function is invoked when this object is firstly created.
    //  All resources can be allocated here.
    virtual bool                    onCreate();

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void                    onDestroy();

    virtual bool                    sendCommand(
                                        uint32_t const  cmd,
                                        MUINTPTR const  arg1,
                                        uint32_t const  arg2,
                                        uint32_t const  arg3 = 0
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_reset();
    virtual bool                    onCmd_capture();
    virtual void                    onCmd_cancel();


protected:  ////                    callbacks
    static MBOOL fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    MBOOL           handlePostViewData(IImageBuffer *pImgBuffer);
    MBOOL           handleRaw16CB(IImageBuffer const *pImgBuffer);
    MBOOL           handleDNGMetaCB(MUINT32 const requestNo);
    MBOOL           handleJpegData(IImageBuffer* pJpeg);

protected:
    MBOOL           CheckIsFlashRequired();
    MBOOL           InitFlashFlow();
    MBOOL           UninitFlashFlow();

protected:
    MBOOL           constructNewPipelineFlow(Vector<BufferItemSet>& rBufSet);
    MBOOL           reUsePipelineFlow(Vector<BufferItemSet>& rBufSet);
    MVOID           previousCapFlowCtrl();
    MVOID           prepareSetting();
    MUINT32         getRotation() const;
    MBOOL           createStreams(NSCam::v1::StreamBufferProvider* rpProviders);
    MBOOL           createPipeline(Vector<BufferItemSet>& rBufferSet);
    MINT            getLegacyPipelineMode(void);
    status_t        getSelectorData(IMetadata& rAppSetting,
                                         IMetadata& rHalSetting,
                                         Vector<BufferItemSet>& rBufferSet);
    MBOOL           CheckIsNeedCreatePipeline();
    void            incProcessingCnt();
    void            decProcessingCnt();
    void            waitPreviousProcessing();

public:     //// interface of CallbackListener
    virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    );

    virtual MVOID   onDataReceived(
                        MUINT32 const requestNo,
                        StreamId_T const streamId,
                        MBOOL const errorBuffer,
                        android::sp<IImageBuffer>& pBuffer
                        );

protected:
    enum CallbackState
    {
        NONE_CB_DONE          = 0x00,
        JPEG_CB_DONE          = 0X01,
        FULL_APP_META_CB_DONE = 0X02,
        FULL_HAL_META_CB_DONE = 0X04,
    };
    //
    struct ResultSet_T {
        MUINT32                       requestNo;
        IMetadata                     appResultMetadata;
        IMetadata                     halResultMetadata;
        IMetadata                     selectorAppMetadata;
        Vector<BufferItemSet>         selectorGetBufs;
        bool                          isAlreadyReturnedSelectorBuf;
        MUINT32                       mMetaCnt;
        MINT32                        callbackState;
    };
    KeyedVector<MUINT32, ResultSet_T>                   mResultMetadataSetMap;
    mutable Mutex                                       mResultMetadataSetLock;
    mutable Mutex                                       mPipelineLock;

protected:
    MSize                                               mSensorSize;
    MSize                                               mJpegsize;
    MSize                                               mThumbnailsize;
    MUINT32                                             mSensorMode;
    MUINT32                                             mu4Bitdepth;
    //
    MSize                                               mLastJpegsize;
    MSize                                               mLastThumbnailsize;
    MUINT32                                             mLastRotation;
    MINT                                                mLastPipelineMode;
    MUINT32                                             mLastSensorMode;
    //
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    sp<NSCam::ICommonCapturePipeline>                   mpPipeline;
#else
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>    mpPipeline;
    sp<ImageCallback>                                   mpCallback;
    sp<BufferCallbackHandler>                           mpCallbackHandler;
    sp<MetadataListener>                                mpListenerPartial;
    sp<MetadataListener>                                mpListenerFull;
#endif
    //
    sp<IImageStreamInfo>                                mpInfo_Yuv;
    sp<IImageStreamInfo>                                mpInfo_YuvPostview;
    sp<IImageStreamInfo>                                mpInfo_YuvThumbnail;
    sp<IImageStreamInfo>                                mpInfo_Jpeg;
    //
    enum ShotState
    {
        NotWorking,
        P2OnWorking,
        JpegOnWorking,
    };
    ShotState                                           mShotState;
    mutable Mutex                                       mShotStateLock;
    mutable Condition                                   mCondShotState;
    //
    MINT32                                              mInProcessingCnt;
    mutable Mutex                                       mProcessingCntLock;
    mutable Condition                                   mCondProcessingCnt;
    //
    MINT32                                              mCapReqNo;
    wp<StreamBufferProvider>                            mpConsumer;

    // keep original selector for switch back
    sp<ISelector>                                       mspOriSelector;

private:    //// debug
    MUINT32                     mDumpFlag;

};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace ZShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_

