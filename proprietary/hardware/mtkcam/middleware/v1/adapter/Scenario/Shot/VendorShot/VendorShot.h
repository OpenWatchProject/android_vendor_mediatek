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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_VENDORSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_VENDORSHOT_H_

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>

namespace NS3Av3 {
    class CaptureParam_T;
}

namespace android {
namespace NSShot {
namespace VShot {
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
                                            MBOOL   const              /*errorBuffer*/,
                                            android::sp<IImageBuffer>& pBuffer
                                        ) {
                                            if( mpListener )
                                                mpListener->onDataReceived(
                                                    RequestNo, streamId, pBuffer
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


/******************************************************************************
 *
 ******************************************************************************/
class VendorShot
    : public ImpShot
    , public CallbackListener
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~VendorShot();
                                    VendorShot(
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


protected:  //// handle data
    MBOOL           handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
    MBOOL           handleJpegData(IImageBuffer* pJpeg);
    MERROR          checkStreamAndEncodeLocked(MUINT32 const requestNo);
    MERROR          postProcessing();

protected:  //// handle setting
    enum SettingType{
        SETTING_NONE,
        SETTING_HDR
    };

    MVOID           decideSettingType( MINT32& type );
    MERROR          updateCaptureParams(
                        MINT32 shotCount,
                        Vector<NS3Av3::CaptureParam_T>& vHdrCaptureParams,
                        Vector<NS3Av3::CaptureParam_T>* vOrigCaptureParams
                    );

protected:
    MVOID           beginCapture();
    MVOID           endCapture();
    MBOOL           constructCapturePipeline();
    MBOOL           createPipeline();
    MERROR          submitCaptureSetting(
                        IMetadata appSetting,
                        IMetadata halSetting
                    );


protected:
    MBOOL           createStreams( MINT32 aBufferCount );
    MINT            getLegacyPipelineMode(void);

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
                        android::sp<IImageBuffer>& pBuffer
                        );

protected:
    struct ResultSet_T {
        MUINT32                       requestNo;
        IMetadata                     appResultMetadata;
        IMetadata                     halResultMetadata;
    };
    KeyedVector<MUINT32, ResultSet_T>                   mResultMetadataSetMap;
    mutable Mutex                                       mResultMetadataSetLock;

    struct EncJob {
        enum {
            STREAM_APP_META = 0,
            STREAM_HAL_META,
            STREAM_IMAGE_MAIN,
            STREAM_IMAGE_THUMB,
        };
        struct Source {
            IMetadata                   HalMetadata;
            IMetadata                   AppMetadata;
            sp<IImageBuffer>            pSrc_main;
            sp<IImageBuffer>            pSrc_thumbnail;
        };
        //
        mutable Mutex                   mLock;
        MINT32                          mSrcCnt;
        DefaultKeyedVector<MUINT32, Source>
                                        mvSource;
        sp<IImageBufferHeap>            mpDst;
        //
        IMetadata                       HalMetadata;
        IMetadata                       AppMetadata;
        sp<IImageBuffer>                pSrc_main;
        sp<IImageBuffer>                pSrc_thumbnail;
        //
                                        EncJob()
                                            : mLock()
                                            , mSrcCnt(0)
                                            , mvSource()
                                            , mpDst()
                                            , HalMetadata()
                                            , AppMetadata()
                                            , pSrc_main()
                                            , pSrc_thumbnail()
                                        {}
                                        ~EncJob() {}
        //
        MBOOL                           add(
                                            MUINT32 const requestNo,
                                            MUINT32 type,
                                            IMetadata const &rMeta
                                        )
                                        {
                                            Mutex::Autolock _l(mLock);
                                            if ( type!=STREAM_APP_META && type!=STREAM_HAL_META)
                                                return MFALSE;
                                            //
                                            ssize_t index = mvSource.indexOfKey(requestNo);
                                            if ( index<0 )
                                                mvSource.add(requestNo, Source());
                                            //
                                            if ( type==STREAM_APP_META )
                                                mvSource.editValueFor(requestNo).AppMetadata = rMeta;
                                            if ( type==STREAM_HAL_META )
                                                mvSource.editValueFor(requestNo).HalMetadata = rMeta;
                                            //
                                            return MTRUE;
                                        }

        MBOOL                           add(
                                            MUINT32 const requestNo,
                                            MUINT32 type,
                                            sp<IImageBuffer> pImage
                                        )
                                        {
                                            Mutex::Autolock _l(mLock);
                                            if ( type!=STREAM_IMAGE_MAIN && type!=STREAM_IMAGE_THUMB)
                                                return MFALSE;
                                            //
                                            ssize_t index = mvSource.indexOfKey(requestNo);
                                            if ( index<0 )
                                                mvSource.add(requestNo, Source());
                                            //
                                            if ( type==STREAM_IMAGE_MAIN )
                                                mvSource.editValueFor(requestNo).pSrc_main = pImage;
                                            if ( type==STREAM_IMAGE_THUMB )
                                                mvSource.editValueFor(requestNo).pSrc_thumbnail = pImage;
                                            //
                                            return MTRUE;
                                        }
        //
        MVOID                           setSourceCnt(MUINT32 count) { mSrcCnt = count; }
        MVOID                           setTarget(sp<IImageBufferHeap> pDst) { mpDst = pDst; }
        //
        MBOOL                           isReady()
                                        {
                                            if ( !mSrcCnt || (size_t)mSrcCnt != mvSource.size() )
                                                return MFALSE;
                                            //
                                            Mutex::Autolock _l(mLock);
                                            for ( size_t i=0; i < mvSource.size(); i++)
                                            {
                                                if ( mvSource[i].HalMetadata.isEmpty()      ||
                                                     mvSource[i].AppMetadata.isEmpty()      ||
                                                     mvSource[i].pSrc_main.get() == NULL    ||
                                                     mvSource[i].pSrc_thumbnail.get() == NULL
                                                   )
                                                return MFALSE;
                                            }
                                            return MTRUE;
                                        }

        MVOID                           clear()
                                        {
                                            Mutex::Autolock _l(mLock);
                                            //
                                            mvSource.clear();
                                            mpDst = NULL;
                                            HalMetadata.clear();
                                            AppMetadata.clear();
                                            pSrc_main = NULL;
                                            pSrc_thumbnail = NULL;
                                        }
    };

protected:
    MSize                                               mSensorSize;
    MINT32                                              mSensorFps;
    MUINT32                                             mPixelMode;
    MSize                                               mJpegsize;
    MUINT32                                             mu4Scenario;
    MUINT32                                             mu4Bitdepth;
    //
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>    mpPipeline;
    sp<ImageCallback>                                   mpImageCallback;
    sp<BufferCallbackHandler>                           mpCallbackHandler;
    sp<MetadataListener>                                mpMetadataListener;
    //
    sp<IImageStreamInfo>                                mpInfo_FullRaw;
    sp<IImageStreamInfo>                                mpInfo_ResizedRaw;
    sp<IImageStreamInfo>                                mpInfo_Yuv;
    sp<IImageStreamInfo>                                mpInfo_YuvPostview;
    sp<IImageStreamInfo>                                mpInfo_YuvThumbnail;
    sp<IImageStreamInfo>                                mpInfo_Jpeg;
    //
    sp<BufferPoolImp>                                   mpJpegPool;
    //
    MUINT32                                             mCapReqNo;
    //
    Condition                                           mEncDoneCond;
    Mutex                                               mEncJobLock;
    EncJob                                              mEncJob;
    //android::KeyedVector< MUINT32, EncJob >             mvEncJob;

private:    //// debug
    MUINT32                     mDumpFlag;

};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace VShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_VENDORSHOT_H_

