/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <utils/Vector.h>

using namespace NSCam::v1;

/*******************************************************************************
*
********************************************************************************/

namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////
class ResourceHolder;
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
class CamShotImp;
/*******************************************************************************
*
********************************************************************************/
class SingleShot
    : public CamShotImp
    , public CallbackListener
{
typedef ISelector::BufferItemSet BufferItemSet;
public:     ////    Constructor/Destructor.
                    SingleShot(
                        EShotMode const eShotMode,
                        char const*const szCamShotName,
                        EPipelineMode const ePipelineMode
                    );
    virtual         ~SingleShot();

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);
    virtual MBOOL   startOne(SensorParam const  & rSensorParam);
    virtual MBOOL   startOne(SensorParam const & rSensorParam, StreamBufferProvider* pProvider, IMetadata* rOverwrite = NULL);
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);

public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer);

public:     ////    Info.
    virtual MBOOL   getRawSize(SensorParam const & rSensorParam, MSize& size);

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3, MVOID* arg4 = NULL);
public:     ////    Result Metadata.
    virtual MBOOL   getResultMetadata(IMetadata &rAppResultMetadata, IMetadata &rHalResultMetadata);

protected:  ////
    MUINT32                             getRotation() const;
    MBOOL                               createStreams(StreamBufferProvider* pProvider = NULL);
    MBOOL                               createPipeline(Vector<BufferItemSet>& rBufferSet);
    MINT                                getLegacyPipelineMode(void);
    status_t                            getSelectorData(ISelector* pSelector,
                                                             ShotParam& rShotParam,
                                                             Vector<BufferItemSet>& rBufferSet,
                                                             IMetadata* rOverwrite);

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

private:    //// data members
    ////      parameters
    SensorParam                 mSensorParam;
    ShotParam                   mShotParam;
    JpegParam                   mJpegParam;
    ////
    MSize                       mSensorSize;
    MINT32                      mSensorFps;
    MUINT32                     mPixelMode;
    //
    sp<IImageStreamInfo>        mpInfo_FullRaw;
    sp<IImageStreamInfo>        mpInfo_ResizedRaw;
    sp<IImageStreamInfo>        mpInfo_LcsoRaw;
    sp<IImageStreamInfo>        mpInfo_Yuv;
    sp<IImageStreamInfo>        mpInfo_YuvPostview;
    sp<IImageStreamInfo>        mpInfo_YuvThumbnail;
    sp<IImageStreamInfo>        mpInfo_Jpeg;
    //
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>
                                mpPipeline;
    sp<ResourceHolder>          mpResource;

    //
    Mutex                       mLock;
    Condition                   mCond;

    //
    IMetadata                   mAppResultMetadata;
    IMetadata                   mHalResultMetadata;
    MBOOL                       mAppDone;
    MBOOL                       mHalDone;

    //
    IMetadata                   mSelectorAppMetadata;

    //
    Mutex                       mShutterLock;
    MBOOL                       mbCBShutter;
    //
    Mutex                       mP2DoneLock;
    MBOOL                       mbP2Done;
    MBOOL                       mbRequestSubmitted;
    mutable Mutex               mStopMtx;
    Condition                   mSubmittedCond;

    //
    KeyedVector< MINT32, sp<IImageBuffer> > mvRegBuf;
    //
    MINT32             mDataMsgEnableSet;

    //TODO
    //MUINT32                  muNRType;

    // TODO: need this????
    // vhdr use
    //MBOOL                    mbVHdr;
    //NS3A::EIspProfile_T      mVhdrProfile;

    //ScenarioCtrl mmdvfs
    sp<IScenarioControl>        m_scenarioControl;

    //
    MBOOL                       mbIsLongExposureTime;

private:    //// debug
    MUINT32                     mDumpFlag;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_

