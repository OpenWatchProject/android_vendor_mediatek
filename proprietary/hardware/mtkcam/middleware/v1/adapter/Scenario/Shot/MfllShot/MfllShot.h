/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_MFLLSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_MFLLSHOT_H_

#include <mtkcam/def/common.h>

#include <mtkcam/middleware/v1/IShot.h>
#include <ImpShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>

#include <mtkcam/middleware/v1/camshot/_params.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>

#include <mtkcam/utils/metadata/IMetadata.h>

#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

#include <semaphore.h>

// STL
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/utils/std/Semaphore.h>
#include <memory>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <future>

using namespace NSCam;
using NSCam::v1::NSLegacyPipeline::IFeatureFlowControl;

// ---------------------------------------------------------------------------

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
    class ILegacyPipeline;
    class IConsumerPool;
}
}
}

// ---------------------------------------------------------------------------

namespace android {
namespace NSShot {

template<typename T>
class LifeSpanObserver final
{
public:
    LifeSpanObserver(const wp<T>& object);
    virtual ~LifeSpanObserver();

private:
    wp<T> mObject;
};

// ---------------------------------------------------------------------------

class MfllShot
    : public ImpShot,
      public NSCam::v1::ResultProcessor::IListener,
      public NSCam::v1::IImageCallback
{
public:
    typedef enum ShotState {
        SHOTSTATE_INIT = 0,
        SHOTSTATE_CAPTURE,
        SHOTSTATE_CANCEL,
        SHOTSTATE_DONE,
    } ShotState;

public:
    MfllShot(const char *pszShotName,
            uint32_t const u4ShotMode,
            int32_t const i4OpenId);

    virtual ~MfllShot();

    // interface of ImpShot
    void onDestroy() override {};

    // interface of ResultProcessor::IListener
    virtual void onResultReceived(
            MUINT32    const requestNo,
            StreamId_T const streamId,
            MBOOL      const errorResult,
            IMetadata  const result) override;
    void onFrameEnd(MUINT32 const requestNo __attribute__((unused))) override {};
    String8 getUserName() override;

    // interface of IImageCallback
    virtual MERROR onResultReceived(
            MUINT32    const requestNo,
            StreamId_T const streamId,
            MBOOL      const errorBuffer,
            sp<IImageBuffer>& pBuffer) override;

    // interface of MfllShot
    bool normalCapture();
    bool zsdCapture();

    // statistics for LifeSpanObserver
    static Mutex sInstanceLock;
    static MINT32 sInstanceCount;

protected:
    struct SensorSetting
    {
        // sensor size
        MSize size;
        // frame rate per second
        MINT32 fps;
        // pixelMode
        MUINT32 pixelMode;
    };

    // sequence number for debug purpose
    static MINT32 sSequence;
    MINT32 mSequence;

    // log level for debug purpose
    MINT32 mLogLevel;

    // shot parameters
    NSCamShot::SensorParam mSensorParam;

    SensorSetting mSensorSetting;

    sp<IImageStreamInfo>   mpImageInfo_fullRaw;
    sp<IImageStreamInfo>   mpImageInfo_resizedRaw;
    sp<IImageStreamInfo>   mpImageInfo_lcsoRaw;
    sp<IImageStreamInfo>   mpImageInfo_yuvJpeg;
    sp<IImageStreamInfo>   mpImageInfo_yuvPostview;
    sp<IImageStreamInfo>   mpImageInfo_yuvThumbnail;
    sp<IImageStreamInfo>   mpImageInfo_jpeg;

    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> mpPipeline;

    sp<NSCam::v1::BufferCallbackHandler> mpCallbackHandler;

    // bitset of data message types
    MINT32 mDataMsgSet;

    // used by ZSD mode
    sp<NSCam::v1::CallbackBufferPool>   mpP1Pool;
    sp<NSCam::v1::CallbackBufferPool>   mpLCSOPool;
    std::vector< sp<IImageBuffer> >     mpSelectedBuffer;
    std::mutex                          mMutexSelectedBuffer;
    std::vector< sp<IImageBuffer> >     mpSelectedLCSOBuffer;
    sp<NSCam::v1::StreamBufferProvider> mpBufferProvider;
    sp<NSCam::v1::StreamBufferProvider> mpLCSOBufferProvider;

    // the number of capture frame
    size_t mFrameCapture;

    // frame done (RAW) counting
    size_t mFrameDoneCnt;

    // for check if using ZSD+AIS
    bool mIsZsdAisMode;

    // state controller
    StateManager<ShotState> mStateMgr;
    Semaphore               mSemCaptureDone;

public:
    // user control functions
    bool sendCommand(
            uint32_t const cmd,
            MUINTPTR const arg1,
            uint32_t const arg2,
            uint32_t const arg3 = 0) override;

protected:
    bool onCmd_reset();
    bool onCmd_capture();
    void onCmd_cancel();

    MUINT32 getRotation() const;

    status_t createStreams(
            const sp<NSCam::v1::StreamBufferProvider>& provider = NULL);
    status_t createPipeline(
            const sp<NSCam::v1::StreamBufferProvider>& provider = NULL);

    status_t getSelectResult(
            const sp<NSCam::v1::ISelector>& selector,
            IMetadata& appSetting,
            IMetadata& halSetting);

    // used to return all selected buffers and then restore selector
    status_t returnSelectedBuffers();

    // used to return the image buffer and restore to selector
    status_t returnSelectedBuffer(sp<IImageBuffer> &buffer);

    status_t releaseResource();

//-----------------------------------------------------------------------------
// JPEG callback
//-----------------------------------------------------------------------------
protected:
    // atomic variable for saving invoke times
    std::atomic_int m_atomicJpegDone;

public:
    // To invoke JPEG callback, this function can be invoked several times but
    // only the first time that fires the JPEG callback.
    // @param buffer            The buffer contains result, this value can be
    //                          NULL which means sends the fake JPEG callback
    void invokeJpegCallback(sp<IImageBuffer> buffer);

//-----------------------------------------------------------------------------
// FeatureFlowControl resume key
//-----------------------------------------------------------------------------
protected:
    // ZSD+MFNR uses heavy traffic of pass 2 therefore we want to reduce pass 2 loading
    // by pause ZSD preview for a while.
    //
    // Resume timing will be the one of the following condition matched:
    // 1. N million seconds later.
    // 2. P2 done callback is invoking.
    struct ___previewResumeOps{
        // {{{
        // interfaces
        std::function<int()> resumePreviewOnce; // the method that to resume preview once
        std::function<int(sp<IFeatureFlowControl>&)> setFlowCtrlOnce; // the method should be invoked only once
        // attributes
        std::mutex mutex;
        sp<IFeatureFlowControl> flow;
        // constructor
        ___previewResumeOps()
        {
            resumePreviewOnce = [this]() -> int {
                sp<IFeatureFlowControl> f = NULL;
                mutex.lock();
                {
                    f = flow;
                    flow = NULL;
                }
                mutex.unlock();

                if (f.get()) {
                    f->resumePreview();
                    return 0;
                }
                else
                    return -1;
            };

            setFlowCtrlOnce = [this](sp<IFeatureFlowControl>&f) -> int {
                mutex.lock();
                flow = f;
                mutex.unlock();
                return 0;
            };
        }
        // }}}
    } mPreviewResumeOps;
//-----------------------------------------------------------------------------
// Phase 1 locker
//-----------------------------------------------------------------------------
protected:
    // atomic counting capture done trigger times
    std::atomic_int m_atomicCaptureDoneTimes;
    void waitCaptureDone();
    void announceCaptureDone();
//-----------------------------------------------------------------------------
// for DNG usage
//-----------------------------------------------------------------------------
protected:
    typedef enum
    {
        SAM_STATUS_OK = 0,
        SAM_STATUS_NO_NEED,
        SAM_STATUS_NOT_INITED,
    } SelectorAppMetaStatus;
    // saves main metadata from selector
    struct {
        IMetadata meta;
        bool bInited; // avoid duplicated init
        Mutex mx; // thread-safe locker
    } mSelectorMetadataPack;

    // this function only works for if APP asked for RAW buffer
    void saveSelectorAppMetadataLocked(const IMetadata &m);

    // this function only works for if APP asked for RAW buffer
    SelectorAppMetaStatus getSelectorAppMetadataLocked(IMetadata *pOutMeta);

//-----------------------------------------------------------------------------
// LCS usage
//-----------------------------------------------------------------------------
public:
    MBOOL mbOpenLCS;

    enum LCSSTATUS {
        LCSSTATUS_DISABLED,
        LCSSTATUS_ENABLED,
    };

    // used to check LCS status
    LCSSTATUS getLcsStatus();
};

} // namespace NSShot
} // namespace android

#endif // _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_MFLLSHOT_H_
