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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SMARTSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SMARTSHOT_H_

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>

#include <future>
#include <vector>
#include <atomic>

#include <mtkcam/pipeline/extension/IVendorManager.h>
//
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#include <mtkcam/middleware/v1/ICommonCapturePipeline.h>
#endif
//

#include <mtkcam/utils/std/Semaphore.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/utils/sys/CpuCtrl.h>

namespace NS3Av3 {
    struct CaptureParam_T;
}

namespace android {
namespace NSShot {
namespace NSSmartShot {
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
/******************************************************************************
 *
 ******************************************************************************/
class CallbackListener
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
                                            CallbackListener* pListener,
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
                                            if( mpListener )
                                                mpListener->onDataReceived(
                                                    RequestNo, streamId, errorBuffer, pBuffer
                                                    );
                                            return OK;
                                        }
protected:
    CallbackListener*                   mpListener;
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

    virtual void                       onFrameEnd(
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
class SmartShotUtil
{
public:
    SmartShotUtil();
   ~SmartShotUtil();
    static SmartShotUtil *getInstance();
    void   SyncDualCam(int openId);

private:
    mutable Mutex   mOpenIdMutex;
    Vector<MUINT32> mvOpenIds;
    Semaphore      *mpSem;
};

/******************************************************************************
 *
 ******************************************************************************/
class SmartShot
    : public ImpShot
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    , public SCallbackListener
#else
    , public CallbackListener
#endif
    , public plugin::INotifyCallback
{

public: // Pipeline type

    typedef
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        NSCam::ICommonCapturePipeline
#else
        NSCam::v1::NSLegacyPipeline::ILegacyPipeline
#endif
        Pipeline_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~SmartShot();
                                    SmartShot(
                                        char const*const pszShotName,
                                        uint32_t const   u4ShotMode,
                                        int32_t const    i4OpenId,
                                        bool const       isZsd
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
//  INotifyCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
virtual android::status_t   onMsgReceived(
                                    MINT32 cmd, /*NOTIFY_MSG*/
                                    MINT32 arg1, MINT32 arg2,
                                    void*  arg3
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_reset();
    virtual bool                    onCmd_capture();
    virtual void                    onCmd_cancel();


protected:  //// handle data
    MBOOL           handlePostViewData(IImageBuffer *pImgBuffer);
    MBOOL           handleRaw16CB(IImageBuffer const *pImgBuffer);
    MBOOL           handleDNGMetaCB(MUINT32 const requestNo);
    virtual MBOOL   handleJpegData(MUINT32 const requestNo, IImageBuffer* pJpeg);
    MERROR          checkStreamAndEncodeLocked(MUINT32 const requestNo);

protected:  //// handle setting
    enum SettingType{
        SETTING_NONE,
        SETTING_FLASH_ACQUIRE,
        SETTING_HDR
    };

    virtual MBOOL   applyRawBufferSettings(
                        Vector<SettingSet>  vSettings,
                        MINT32              shotCount,
                        Vector<MINT32>&     rvRequestNo,
                        MINT32              dummyCount = 0,
                        MINT32              delayedCount = 0
                    );

    MERROR          updateCaptureParams(
                        MINT32 shotCount,
                        Vector<NS3Av3::CaptureParam_T>& vHdrCaptureParams
                    );

protected:
    MERROR          updateSetting(
                        NSCam::plugin::OutputInfo& param,
                        NSCam::plugin::InputSetting& setting
                    );
    MVOID           beginCapture( MINT32 rAllocateCount );
    virtual MVOID   updateLastInfo();
    virtual MVOID   endCapture();
    virtual MBOOL   constructCapturePipeline();
    virtual MBOOL   createPipeline();
    virtual MERROR  submitCaptureSetting(
                        MBOOL     mainFrame,
                        IMetadata appSetting,
                        IMetadata halSetting
                    );

    // Submit a request to pipeline with only a RRZO source image as dummy frame.
    //  @note If no source image (RRZO), the request cannot be dispatched to IVendor
    //        framework due to P2 design.
    virtual MERROR  submitZsdDummyCaptureSetting(
                        IMetadata appSetting,
                        IMetadata halSetting
                    );

    // Pause preview, and return a resume preview controller. The reference
    // count of the returned shared_ptr counts to 0 (destroying), the preview
    // will be resumed.
    //  @return             A shared pointer to hold resource of IFeatureFlowControl
    //                      instance, while it's being destroyed, the method
    //                      IFeatureFlowControl::resumePreview will be invoked
    virtual std::shared_ptr<void> pausePreview();

protected:
    MBOOL           createNewPipeline();
    // check if the shot should be re-constructed.
    //  @return                 Returns TRUE if it should be re-constructed.
    virtual MBOOL   isNeedToReconstruct();
    virtual MBOOL   createStreams();
    MINT            getLegacyPipelineMode(void);
    virtual status_t        getSelectorData(IMetadata& rAppSetting,
                                         IMetadata& rHalSetting,
                                         android::sp<IImageBuffer>& pBuffer);

    // get requested selector data from ZsdSelector
    status_t        getZsdSelectorRequestedData(
                        IMetadata&                  rAppSetting,
                        IMetadata&                  rHalSetting,
                        android::sp<IImageBuffer>&  pBuffer,
                        MINT32                      requestNo
                    );

    // get the continuous buffers and submit settings w/ condition ZsdSelector
    // and using ZSD buffers.
    status_t        submitZslCaptureSetting(
                        const plugin::InputSetting  inputSetting,
                        Vector<MINT32>&             vRequestNo
                    );

    MBOOL           makeVendorCombination(
                        plugin::InputInfo& inputInfo,
                        const MINT64 mode
                    );

    inline
    sp<Pipeline_T>  getPipeline() const
    {
        sp<Pipeline_T> p;
        std::lock_guard<std::mutex> l(mpPipelineLock);
        p = mpPipeline;
        return p;
    }

    inline void     setPipeline(const sp<Pipeline_T>& p)
    {
        std::lock_guard<std::mutex> l(mpPipelineLock);
        mpPipeline = p;
    }

    // Wait until pipeline drain. After invoked this method. Member SmartShot::mpPipeline
    // will be cleared as nullptr.
    void            waitUntilDrained();

protected:  //// getter/setter of max capture frame number
    void            setMaxCaptureFrameNum(MINT32 num);
    MINT32          getMaxCaptureFrameNum();


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
                        MBOOL const errorResult,
                        android::sp<IImageBuffer>& pBuffer
                        );

protected:
    enum CallbackState
    {
        NONE_CB_DONE          = 0x00,
        RAW_CB_DONE           = 0X01,
        JPEG_CB_DONE          = 0X02,
        FULL_APP_META_CB_DONE = 0X04,
        FULL_HAL_META_CB_DONE = 0X08,
        FULL_ALL_META_CB_DONE = (FULL_APP_META_CB_DONE|FULL_HAL_META_CB_DONE),
        DNG_CB_DONE = (RAW_CB_DONE|FULL_APP_META_CB_DONE|FULL_HAL_META_CB_DONE),
    };
    //
    struct ResultSet_T {
        MUINT32                       requestNo;
        IMetadata                     appResultMetadata;
        IMetadata                     halResultMetadata;
        IMetadata                     selectorAppMetadata;
        Vector<ISelector::BufferItemSet>         selectorGetBufs;
        MINT32                        callbackState;
    };
    KeyedVector<MUINT32, ResultSet_T>                   mResultMetadataSetMap;
    mutable Mutex                                       mResultMetadataSetLock;

protected:
    MSize                                               mSensorSize;
    MINT32                                              mSensorFps;
    MUINT32                                             mPixelMode;
    MSize                                               mJpegsize;
    MUINT32                                             mu4Scenario;
    MUINT32                                             mu4Bitdepth;
    MBOOL                                               mbZsdFlow;
    //
    sp<NSCam::plugin::IVendorManager>                   mpManager;
    sp<Pipeline_T>                                      mpPipeline;
    mutable std::mutex                                  mpPipelineLock;
    sp<ImageCallback>                                   mpImageCallback;
    sp<BufferCallbackHandler>                           mpCallbackHandler;
    sp<MetadataListener>                                mpMetadataListener;
    sp<MetadataListener>                                mpMetadataListenerFull;
    //
    sp<IImageStreamInfo>                                mpInfo_FullRaw;
    sp<IImageStreamInfo>                                mpInfo_ResizedRaw;
    sp<IImageStreamInfo>                                mpInfo_LcsoRaw;
    sp<IImageStreamInfo>                                mpInfo_Yuv;
    sp<IImageStreamInfo>                                mpInfo_YuvPostview;
    sp<IImageStreamInfo>                                mpInfo_YuvThumbnail;
    sp<IImageStreamInfo>                                mpInfo_Jpeg;
    //
    sp<BufferPoolImp>                                   mpJpegPool;
    //
    MUINT32                                             mCapReqNo;
    wp<StreamBufferProvider>                            mpConsumer;
    //
    //android::KeyedVector< MUINT32, EncJob >             mvEncJob;
    //
    std::atomic<MINT32>                                 mMaxCaptureFrameNum;
    //
    MSize                                               mLastJpegsize;
    MUINT32                                             mLastRotation;
    int                                                 mLastPicFmt;
    MBOOL                                               mNeedReconstruct;

    // Describes which selector will be used (default is ZsdRequestSelector)
    std::atomic<ISelector::SelectorType>                mSelectorUsage;

    // If using ZsdSelector, this flag describes if it's necessary to ask for
    // the requested frame or just using ZSD buffers.
    std::atomic<MBOOL>                                  mSelectorRequested;

public:
    // Check if using ZSD buffers. This method only works if the selector type
    // is set as ZsdSelector. Otherwise returns MFALSE.
    inline MBOOL                        isUseZsdBuffers() const
    {
        if (mSelectorUsage.load() == ISelector::SelectorType_ZsdSelector)
            return mSelectorRequested.load();
        else
            return MFALSE;
    }

    // Set if using ZSD buffers.
    inline void                         setUseZsdBuffers(
                                            MBOOL bUse
                                        )
    {
        if (mSelectorUsage.load() != ISelector::SelectorType_ZsdSelector)
            mSelectorRequested.store(MFALSE); // always false due to not ZsdSelector
        else
            mSelectorRequested.store(bUse);
    }



protected:
    std::vector< std::future<MERROR> >                  mvFutures;
    sp<ISelector>                                       mspOriSelector;

    // state controller
    enum ShotState {
        SHOTSTATE_INIT = 0,
        SHOTSTATE_CAPTURE,
        SHOTSTATE_CANCEL,
        SHOTSTATE_DONE,
    };
    //
    mutable std::mutex                                  mShotLock;
    mutable std::condition_variable                     mShotCond;
    ShotState                                           mShotState;

private:    //// debug
    MUINT32                                             mDumpFlag;
    int                                                 mDbgLevel;

private:
    std::unique_ptr<
        CpuCtrl,
        std::function< void(CpuCtrl*) >
    >
    mCpuCtrl;

protected:
    // notice, non-ZSD mode, one shot instance one take picture. Hence mFlushP1Done
    // has no need to be reset to false.
    bool                                                mFlushP1Done;
    std::mutex                                          mFlushP1Lock;

    // This method is valid if SmartShot::supportFastP1Done returns ture.
    virtual void                                        fastP1DoneDrain();
    virtual bool                                        supportFastP1Done();

};


/******************************************************************************
 *
 ******************************************************************************/
}; // NSSmartShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SMARTSHOT_H_

