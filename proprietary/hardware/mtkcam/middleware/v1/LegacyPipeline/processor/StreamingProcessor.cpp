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

#define LOG_TAG "MtkCam/StreamingProcessor"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
#include <system/camera.h>
#include <system/camera_metadata_tags.h>
//
#include "MyUtils.h"
#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor.h>
//#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor_V0.h>

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1) || (MTK_CAM_STEREO_DENOISE_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1)
// DualCam support
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#define DUALCAM 1
#else
#define DUALCAM 0
#endif


#if MTKCAM_HAVE_AEE_FEATURE == 1
#include <aee.h>
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
        aee_system_exception( \
            LOG_TAG, \
            NULL, \
            DB_OPT_DEFAULT|DB_OPT_FTRACE , \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
    } while(0)

#endif

//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata metadata,
    MUINT32 const tag,
    T & rVal
)
{
    IMetadata::IEntry entry = metadata.entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/

class StreamingProcessorImp
    : public StreamingProcessor
    , public IRequestCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void                onResultReceived(
                                    MUINT32         const requestNo,
                                    StreamId_T      const streamId,
                                    MBOOL           const errorResult,
                                    IMetadata       const result
                                );

    virtual void                onFrameEnd(
                                    MUINT32         const /*requestNo*/
                                ) {};

    virtual String8             getUserName();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDataListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual void                onMetaReceived(
                                    MUINT32         const requestNo,
                                    StreamId_T      const streamId,
                                    MBOOL           const errorResult,
                                    IMetadata       const result
                                );

    virtual void                onCtrlSetting(
                                    MUINT32         const requestNo,
                                    StreamId_T      const metaAppStreamId,
                                    IMetadata&            rAppMetaData,
                                    StreamId_T      const metaHalStreamId,
                                    IMetadata&            rHalMetaData,
                                    MBOOL&                rIsChanged
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void                onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StreamingProcessor Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual status_t            startAutoFocus();

    virtual status_t            cancelAutoFocus();

    virtual status_t            preCapture(int& flashRequired, nsecs_t tTimeout);

    virtual status_t            startSmoothZoom(int value);

    virtual status_t            stopSmoothZoom();

    virtual status_t            sendCommand(
                                    int32_t cmd,
                                    int32_t arg1, int32_t arg2
                                );

    virtual void                dump();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IRequestCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual void                RequestCallback(
                                    uint32_t requestNo,
                                    MINT32   type,
                                    MINTPTR  _ext1,
                                    MINTPTR  _ext2
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    StreamingProcessorImp(
                                        MINT32                            openId,
                                        sp<INotifyCallback>         const &rpCamMsgCbInfo,
                                        wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
                                        sp< IParamsManagerV3 >      const &rpParamsManagerV3,
                                        MINT32                            aStartRequestNumber,
                                        MINT32                            aEndRequestNumber
                                    );

                                    ~StreamingProcessorImp();

            void                    doNotifyCb(
                                        uint32_t requestNo,
                                        int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2,
                                        int32_t _ext3
                                    );

            bool                    needFocusCallback(StreamId_T const streamId);

            bool                    needZoomCallback(
                                        MUINT32 requestNo,
                                        MINT32  &zoomIndex
                                    );

            bool                    isAfCallback(
                                        MUINT8  afState,
                                        MINT&   msg,
                                        MINT&   msgExt,
                                        MINT32  masterid
                                    );

            void                    checkMultiZoneAfWindow(IMetadata const result);

          MINT32                    getOpenId() const       { return mOpenId; }

public:     ////                    Definitions.

    typedef KeyedVector< MUINT32, MUINT32 > Que_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    sp<INotifyCallback>             mpCamMsgCbInfo;
    sp< IParamsManagerV3 >          mpParamsManagerV3;
    wp< RequestSettingBuilder >     mpRequestSettingBuilder;

protected: ////                     Logs.
    MINT32                          mLogLevel;
    MBOOL                           isSupportMultiZonAfWindow;
    MINT32                          mOpenId;

protected: ////                     3A information.
    MUINT8                          mAestate;
    MUINT8                          mAfstate;
    MUINT8                          mFlashstate;
    MUINT8                          mLastFlashstate;
    MUINT8                          mAfstateLastCallback;
    MUINT8                          mAemode;
    mutable Mutex                   m3ALock;

protected: ////                     zoom callback information.
    MUINT32                         mZoomTargetIndex;
    Que_T                           mZoomResultQueue;
    mutable Mutex                   mZoomLock;
    MBOOL                           mZoomTriggerd;
    mutable Condition               mWaitZoomLock;

protected: ////                     af callback information.
    MBOOL                           mListenAfResult;
    MBOOL                           mNeedAfCallback;
    MUINT32                         mAfRequest;
    mutable Mutex                   mAutoFocusLock;

protected: ////                     ae information for preCapture.
    MBOOL                           mListenPreCapture;
    MBOOL                           mListenPreCaptureStart;
    MBOOL                           mListenAfResultPreCapture;
    MBOOL                           mListenFlashResultPreCapture;
    MBOOL                           mListenAeResult;
    mutable Mutex                   mPreCaptureLock;
    Condition                       mPreCaptureCond;
    MBOOL                           mbAeFlashRequired;
    MBOOL                           mbUpdateAf;
    MBOOL                           mbNeedWaitFlashDone;
    MBOOL                           mbBvTrigger;

    MUINT32                         mAeRequest;
    MBOOL                           mSkipCheckAeRequestNumber;
protected:
    MBOOL                           mUninit;
protected:
    MRect                           mActiveArray;
    MINT                            lastAFCBMsg;
    MINT                            lastAFCBExt;
    MINT32                          mneedAFUpdate;
    //
    MBOOL                           mInPrecapturePeriod;
    MBOOL                           mbNeedTriggerPrecapture;
    MBOOL                           mAfTriggeredByAP;
    mutable Mutex                   mPreCap3ALock;
    MINT                            mLastPrecapEndRequestNo;
    MBOOL                           mbNeedResumeFocus;
    mutable Mutex                   mbResumeFocusLock;
    //
    MINT32                          mCancelAfStatus;
    mutable Mutex                   mCancelAfStatusLock;
    //
    MINT32                          mNeedQualityUp;
    //
    MINT32                          mStartRequestNumber;
    MINT32                          mEndRequestNumber;
    //
    mutable Mutex                   mBurstNumLock;
    MINT32                          mBurstNum;
    enum eCancelAfStatus
    {
        eCancelAF_NoRequest,
        eCancelAF_ReceivedRequest,
        eCancelAF_AlreadyPassTo3A
    };
    MINT32                          mMain1Id = -1;
    MINT32                          mMain2Id = -1;
    MINT32                          mCurrentStereoWarning = -1;
    //
    MBOOL                           mPrecapInAeAfLock = MFALSE;
    MBOOL                           mLastErrorResult = MFALSE;
    //
    MUINT8                          mInPrecaptureFirstAfMode = 0xFF;
};

/////////////////////////////////////////////////////////
class LegacyStreamingProcessorImp
    : public StreamingProcessorImp
{
public:
                                LegacyStreamingProcessorImp(
                                    MINT32                            openId,
                                    sp<INotifyCallback>         const &rpCamMsgCbInfo,
                                    wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
                                    sp< IParamsManagerV3 >      const &rpParamsManagerV3,
                                    MINT32                            aStartRequestNumber,
                                    MINT32                            aEndRequestNumber
                                );

    virtual void                onResultReceived(
                                    MUINT32         const requestNo,
                                    StreamId_T      const streamId,
                                    MBOOL           const errorResult,
                                    IMetadata       const result
                                );

    virtual void                onMetaReceived(
                                    MUINT32         const requestNo,
                                    StreamId_T      const streamId,
                                    MBOOL           const errorResult,
                                    IMetadata       const result
                                );

    virtual void                RequestCallback(
                                    uint32_t requestNo,
                                    MINT32   type,
                                    MINTPTR  _ext1,
                                    MINTPTR  _ext2
                                );

    virtual void                onCtrlSetting(
                                    MUINT32         const requestNo,
                                    StreamId_T      const metaAppStreamId,
                                    IMetadata&            rAppMetaData,
                                    StreamId_T      const metaHalStreamId,
                                    IMetadata&            rHalMetaData,
                                    MBOOL&                rIsChanged
                                ) {}

    virtual status_t            preCapture(int& flashRequired, nsecs_t tTimeout);

protected: ////                     debug
    MBOOL                      mLastErrorResult;

};


/******************************************************************************
 *
 ******************************************************************************/
sp< StreamingProcessor >
StreamingProcessor::
createInstance(
    MINT32                            openId,
    sp<INotifyCallback>         const &rpCamMsgCbInfo,
    wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
    sp< IParamsManagerV3 >      const &rpParamsManagerV3,
    MINT32                            aStartRequestNumber,
    MINT32                            aEndRequestNumber
)
{
    sp< StreamingProcessor > ret;
    int fastPrecap = ::property_get_int32("debug.camera.fast.precap", -1);
    if(fastPrecap > 1)
    {
        CAM_LOGD("[%d:StreamingProcessor::createInstance] forced to run fast precapture flow", openId);
        ret = new StreamingProcessorImp(
                    openId,
                    rpCamMsgCbInfo,
                    rpRequestSettingBuilder,
                    rpParamsManagerV3,
                    aStartRequestNumber,
                    aEndRequestNumber
               );
    }
    else if(fastPrecap == 0)
    {
        CAM_LOGD("[%d:StreamingProcessor::createInstance] forced to run legacy precapture flow", openId);
        ret = new LegacyStreamingProcessorImp(
                    openId,
                    rpCamMsgCbInfo,
                    rpRequestSettingBuilder,
                    rpParamsManagerV3,
                    aStartRequestNumber,
                    aEndRequestNumber
               );
    }
    else
    {
#if MTK_CAM_FAST_PRECAPTURE_FLOW_ON
        CAM_LOGD("[%d:StreamingProcessor::createInstance] the project run fast precapture flow", openId);
        ret = new StreamingProcessorImp(
                    openId,
                    rpCamMsgCbInfo,
                    rpRequestSettingBuilder,
                    rpParamsManagerV3,
                    aStartRequestNumber,
                    aEndRequestNumber
               );
#else
        CAM_LOGD("[%d:StreamingProcessor::createInstance] the project run legacy precapture flow", openId);
        ret = new LegacyStreamingProcessorImp(
                    openId,
                    rpCamMsgCbInfo,
                    rpRequestSettingBuilder,
                    rpParamsManagerV3,
                    aStartRequestNumber,
                    aEndRequestNumber
               );
#endif
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
StreamingProcessorImp::
StreamingProcessorImp(
    MINT32                            openId,
    sp<INotifyCallback>         const &rpCamMsgCbInfo,
    wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
    sp< IParamsManagerV3 >      const &rpParamsManagerV3,
    MINT32                            aStartRequestNumber,
    MINT32                            aEndRequestNumber
)
    : mpCamMsgCbInfo(rpCamMsgCbInfo)
    , mpParamsManagerV3(rpParamsManagerV3)
    , mpRequestSettingBuilder(rpRequestSettingBuilder)
    , mOpenId(openId)
    //
    , mAfstate(0)
    , mAfstateLastCallback(0)
    , mFlashstate(0)
    , mLastFlashstate(0)
    , mAemode(0)
    //
    , mZoomTriggerd(MFALSE)
    //
    , mListenAfResultPreCapture(MFALSE)
    , mListenAeResult(MFALSE)
    , mListenFlashResultPreCapture(MFALSE)
    , mbUpdateAf(MFALSE)
    , mbNeedWaitFlashDone(MFALSE)
    , mbBvTrigger(MFALSE)
    , mUninit(false)
    //
    , lastAFCBMsg(0)
    , lastAFCBExt(0)
    , mneedAFUpdate(0)
    , mAestate(0)
    , mListenPreCapture(MFALSE)
    , mListenPreCaptureStart(MFALSE)
    , mbAeFlashRequired(MFALSE)
    , mAeRequest(0)
    , mSkipCheckAeRequestNumber(MFALSE)
    , mListenAfResult(MFALSE)
    , mNeedAfCallback(MFALSE)
    , mAfRequest(0)
    , mZoomTargetIndex(0)
    , mInPrecapturePeriod(MFALSE)
    , mbNeedTriggerPrecapture(MFALSE)
    , mAfTriggeredByAP(MFALSE)
    , mLastPrecapEndRequestNo(0)
    , mbNeedResumeFocus(MFALSE)
    //
    , mCancelAfStatus(eCancelAF_NoRequest)
    //
    , mNeedQualityUp(0)
    //
    , mStartRequestNumber(aStartRequestNumber)
    , mEndRequestNumber(aEndRequestNumber)
    , mBurstNum(1)
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.SProcessor", 0);
    }
    // get multiAF window support
    MINT32 value;
    value = ::property_get_int32("debug.camera.multizoneaf", -1);
    if ( value != 1 )
    {
        isSupportMultiZonAfWindow = MFALSE;
    }
    else
    {
        isSupportMultiZonAfWindow = MTRUE;
    }
    //
    // get sensor active area size
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    IMetadata::IEntry activeA = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
    if( !activeA.isEmpty() )
    {
        mActiveArray = activeA.itemAt(0, Type2Type<MRect>());
    }
#if DUALCAM
    // get main1 & main2 open id
    if(!StereoSettingProvider::getStereoSensorIndex(mMain1Id, mMain2Id))
    {
        MY_LOGE("get open id fail");
    }
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
StreamingProcessorImp::
~StreamingProcessorImp()
{
    MY_LOGI("+");
    mPreCaptureCond.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
onLastStrongRef(const void* /*id*/)
{
    MY_LOGI("lastAFCBMsg(%d), lastAFCBExt(%d)", lastAFCBMsg, lastAFCBExt);

    // if last callback is focus moving, make sure send moving stop at last time
    if (lastAFCBMsg == CAMERA_MSG_FOCUS_MOVE && lastAFCBExt == 1)
    {
        MY_LOGI("Send Focus stoped");
        doNotifyCb(
                0,
                CAMERA_MSG_FOCUS_MOVE,
                0,
                0,
                0);
    }

    mUninit = true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    MBOOL bInPrecapturePeriod = MFALSE;
    MBOOL bNeedTriggerPrecapture = MFALSE;
    {
        Mutex::Autolock _l(mPreCap3ALock);
        bInPrecapturePeriod = mInPrecapturePeriod;
        bNeedTriggerPrecapture = mbNeedTriggerPrecapture;
    }
    //
    MINT32 burstNum = 1;
    {
        Mutex::Autolock _l(mBurstNumLock);
        burstNum = mBurstNum;
    }
    MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d) streamId:(%#" PRIx64 ") errorResult:(%d) burstNum:(%d) requestRange(%d,%d) bInPrecapturePeriod(%d) bNeedTriggerPrecapture(%d)",
        requestNo, streamId,errorResult, burstNum, mStartRequestNumber, mEndRequestNumber,bInPrecapturePeriod,bNeedTriggerPrecapture);
    //
    // update request number
    {
        Mutex::Autolock _l(mAutoFocusLock);
        if( (mAfRequest+burstNum-1) > mEndRequestNumber)
        {
            int dist = mEndRequestNumber - mAfRequest;
            mListenAfResult = ((requestNo >= mAfRequest) || (requestNo <= mStartRequestNumber+dist)) ? MTRUE : mListenAfResult;
        }
        else
        {
            mListenAfResult = ((requestNo >= mAfRequest) && (requestNo <= mAfRequest+burstNum-1)) ? MTRUE : mListenAfResult;
        }
    }
    MINT32 masterid = -1;
    //
    if ( errorResult )
    {
        if (requestNo == mAeRequest)
        {
            MY_LOGW("mAeRequest(%d) had errorResult", mAeRequest);
            mSkipCheckAeRequestNumber = MTRUE;
        }
        return;
    }
    // update 3A
    {
        Mutex::Autolock _l(m3ALock);
        MUINT8 curAfstate = 0xFF;
        tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AF_STATE, curAfstate);
        tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_STATE, mAestate);
        tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_MODE, mAemode);
        tryGetMetadata< MBOOL >(result, MTK_3A_AE_BV_TRIGGER, mbBvTrigger);
        //
    #if DUALCAM
        if (StereoSettingProvider::isDualCamMode()
            && eSTREAMID_META_HAL_DYNAMIC_P1 == streamId)
        {
            tryGetMetadata< MINT32 >(result, MTK_STEREO_SYNC2A_MASTER_SLAVE, masterid);
        }
    #endif
        //
        if(curAfstate != 0xFF)
        {
            if( mbUpdateAf==MFALSE)
            {
                mAfstateLastCallback = curAfstate;
            }
            mbUpdateAf = MTRUE;
            mAfstate = curAfstate;
        }
    }
    //
    // update AE/AF/Flash state for PreCapture
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture) {
            MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d,%d) mAestate: %d, mAfstate %d, mbBvTrigger %d mAemode: %d", requestNo, mAeRequest, mAestate, mAfstate, mbBvTrigger,mAemode);
            MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"mListenAeResult: %d, mListenAfResultPreCapture: %d, mListenFlashResultPreCapture: %d", mListenAeResult, mListenAfResultPreCapture, mListenFlashResultPreCapture);

            // check if starting to receive AE and AF results
            if ((mListenAeResult || mListenAfResultPreCapture || mListenFlashResultPreCapture)
                && (requestNo == mAeRequest || mSkipCheckAeRequestNumber))
            {
                    mListenPreCaptureStart = MTRUE;
                    MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d,%d) mListenPreCaptureStart = MTRUE", requestNo, mAeRequest);
            }

            if (mListenPreCaptureStart) {
                if(mAemode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
                   (mAemode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && mbBvTrigger))
                {
                    mbAeFlashRequired = MTRUE;
                    if(mListenFlashResultPreCapture)
                    {
                        mbNeedWaitFlashDone = MTRUE;
                        MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d,%d) mAemode:%d mbBvTrigger:%d mListenFlashResultPreCapture:1 => mbNeedWaitFlashDone = MTRUE",
                            requestNo, mAeRequest, mAemode, mbBvTrigger);
                    }
                }
                MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d,%d) mAemode:%d, mbAeFlashRequired:%d, mbNeedWaitFlashDone:%d",
                    requestNo, mAeRequest, mAemode, mbAeFlashRequired, mbNeedWaitFlashDone);
                //
                if (mListenAeResult) {
                    if (mAestate == MTK_CONTROL_AE_STATE_CONVERGED || mAestate == MTK_CONTROL_AE_STATE_FLASH_REQUIRED )
                    {
                        mListenAeResult = MFALSE;
                        MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:(%d,%d) mAestate:%d, mListenAeResult = 0", requestNo, mAeRequest, mAestate);
                    }
                }
                //
                if(mListenFlashResultPreCapture)
                {
                    MUINT8 curFlashstate = 0xFF;
                    if ( tryGetMetadata< MUINT8 >(result, MTK_FLASH_STATE, curFlashstate) ) {
                        mLastFlashstate = mFlashstate;
                        mFlashstate = curFlashstate;
                    }
                    //
                    MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:%d, mLastFlashstate:%d, mFlashstate:%d", requestNo, mLastFlashstate, mFlashstate);
                    if( mbNeedWaitFlashDone )
                    {
                        if(mLastFlashstate == MTK_FLASH_STATE_FIRED && mFlashstate == MTK_FLASH_STATE_READY ||
                           mLastFlashstate == MTK_FLASH_STATE_PARTIAL && mFlashstate == MTK_FLASH_STATE_READY )
                        {
                            MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"requestNo:%d, Flash Fired Done, mListenFlashResultPreCapture change to 0  mbNeedWaitFlashDone=MFALSE", requestNo);
                            mListenFlashResultPreCapture = MFALSE;
                            mbNeedWaitFlashDone = MFALSE;
                        }
                    }
                }
                //
                if (!mListenAeResult && !mListenAfResultPreCapture && !mListenFlashResultPreCapture) {
                    Mutex::Autolock _l(mPreCap3ALock);
                    mListenPreCapture = MFALSE;
                    mLastPrecapEndRequestNo = requestNo;
                    MY_LOGI("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                    mPreCaptureCond.signal();
                }
            }
        }
    }

    // check focus callback
    if ( needFocusCallback(streamId) )
    {
        MINT msg = 0, msgExt = 0;
        if(isAfCallback(mAfstate, msg, msgExt, masterid))
        {
            Mutex::Autolock _l(mPreCap3ALock);

            if(mInPrecapturePeriod == true && (mAfTriggeredByAP == false  ||  requestNo == (MUINT32)mLastPrecapEndRequestNo))
            {

                MY_LOGD("mInPrecapturePeriod == %d , mAfTriggeredByAP == %d, mLastPrecapEndRequestNo =%d, requestNo =%d, no need do AF callback"
                         , mInPrecapturePeriod, mAfTriggeredByAP, mLastPrecapEndRequestNo, requestNo);
                if(msg == CAMERA_MSG_FOCUS)
                {
                    Mutex::Autolock _l(mAutoFocusLock);
                    mNeedAfCallback = MFALSE;
                    mListenAfResult = MFALSE;
                }

            }
            else
            {
                // do multi af window callback
                //if(isSupportMultiZonAfWindow)
                {
                    checkMultiZoneAfWindow(result);
                }
                //
                doNotifyCb(
                    requestNo,
                    msg,
                    msgExt,
                    0,
                    0);
                lastAFCBMsg = msg;
                lastAFCBExt = msgExt;
            }
            mAfTriggeredByAP = false;
            mpParamsManagerV3->setAfTriggered(MFALSE);
        }
    }
    // check zoom callback
    MINT32 zoomIndex = 0;
    if ( needZoomCallback( requestNo, zoomIndex ) ) {
        doNotifyCb(
            requestNo,
            CAMERA_MSG_ZOOM,
            zoomIndex, 0, 0
        );

        Mutex::Autolock _l(mZoomLock);
        if(mNeedQualityUp)
        {
            sp<IFeatureFlowControl> pIFeatureFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();

            if(! pIFeatureFlowControl.get())
            {
                MY_LOGE("pIFeatureFlowControl is null");
                return;
            }

            pIFeatureFlowControl->highQualityZoom();
            mNeedQualityUp = 0;
        }
    }

#if 0
    // precapture AE
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture && mListenPreCaptureStart && mListenAeResult) {
            MUINT8 curAestate = 0xFF;
            if ( !errorResult && !tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_STATE, curAestate)) {
                //MY_LOGW("AE state not update.");
            }
            MY_LOGD1("requestNo:(%d) AE state:(%d)", requestNo, curAestate);
            if (curAestate == MTK_CONTROL_AE_STATE_CONVERGED ||
                curAestate == MTK_CONTROL_AE_STATE_FLASH_REQUIRED)
            {
                mListenAeResult = MFALSE;
            }
            //
            if (!mListenAeResult && !mListenAfResultPreCapture && !mListenFlashResultPreCapture) {
                Mutex::Autolock _l(mPreCap3ALock);
                mListenPreCapture = MFALSE;
                mLastPrecapEndRequestNo = requestNo;
                MY_LOGD("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                mPreCaptureCond.signal();
            }
        }
    }
#endif
    // precapture AF
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture && mListenPreCaptureStart && mListenAfResultPreCapture) {
            MUINT8 curAfstate = 0xFF;
            if ( !errorResult && !tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AF_STATE, curAfstate) ) {
                //MY_LOGW("AF state not update.");
            }
            MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1), "requestNo:(%d) curAfstate:(%d)", requestNo, curAfstate);
            if (curAfstate == MTK_CONTROL_AF_STATE_FOCUSED_LOCKED  ||
                curAfstate == MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED)
            {
                mListenAfResultPreCapture = MFALSE;
            }

            if (!mListenAeResult && !mListenAfResultPreCapture && !mListenFlashResultPreCapture) {
                Mutex::Autolock _l(mPreCap3ALock);
                mListenPreCapture = MFALSE;
                mLastPrecapEndRequestNo = requestNo;
                MY_LOGI("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                mPreCaptureCond.signal();
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
onCtrlSetting(
    MUINT32         const requestNo,
    StreamId_T      const metaAppStreamId,
    IMetadata&            rAppMetaData,
    StreamId_T      const metaHalStreamId,
    IMetadata&            rHalMetaData,
    MBOOL&                rIsChanged
)
{
    MBOOL bInPrecapturePeriod = MFALSE;
    MBOOL bNeedTriggerPrecapture = MFALSE;
    {
        Mutex::Autolock _l(mPreCap3ALock);
        bInPrecapturePeriod = mInPrecapturePeriod;
        bNeedTriggerPrecapture = mbNeedTriggerPrecapture;
    }
    //
    MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"reqNo %d, metaAppStreamId (%#" PRIx64 ") metaHalStreamId (%#" PRIx64 ") bInPrecapturePeriod(%d) bNeedTriggerPrecapture(%d)",
            requestNo,
            metaAppStreamId,
            metaHalStreamId,
            bInPrecapturePeriod,
            bNeedTriggerPrecapture);
    //
    {
        Mutex::Autolock _l(mbResumeFocusLock);
        if(mbNeedResumeFocus)
        {
            MY_LOGI("set MTK_FOCUS_PAUSE = 0");
            IMetadata::setEntry<MUINT8>(&rHalMetaData, MTK_FOCUS_PAUSE, 0);
            mbNeedResumeFocus = MFALSE;
        }
    }
    //
    MINT32 cancelAfStatus = eCancelAF_NoRequest;
    {
        Mutex::Autolock _l(mCancelAfStatusLock);
        cancelAfStatus = mCancelAfStatus;
    }
    //
    MBOOL bNeedFlash = MFALSE;
    {
        Mutex::Autolock _l(m3ALock);
        if( mAemode== MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
            (mAemode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && mbBvTrigger) )
        {
            bNeedFlash = MTRUE;
            MY_LOGD1("mAemode:%d mbBvTrigger:%d => Need Flash", mAemode, mbBvTrigger);
        }
    }
    //
    MUINT8 afTrigger = 0xff;
    {
        if(metaAppStreamId == eSTREAMID_META_APP_CONTROL)
        {
            if(IMetadata::getEntry<MUINT8>(&rAppMetaData, MTK_CONTROL_AF_TRIGGER, afTrigger))
            {
                MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"request:%d MTK_CONTROL_AF_TRIGGER(%d) cancelAfStatus(%d) mCancelAfStatus(%d) bInPrecapturePeriod(%d) mAemode(%d) mbBvTrigger(%d)",
                    requestNo,afTrigger,cancelAfStatus,mCancelAfStatus,bInPrecapturePeriod,mAemode,mbBvTrigger);
                if(afTrigger == MTK_CONTROL_AF_TRIGGER_CANCEL)
                {
                    if(cancelAfStatus==eCancelAF_ReceivedRequest)
                    {
                        Mutex::Autolock _l(mCancelAfStatusLock);
                        if(bInPrecapturePeriod)
                        {
                            mCancelAfStatus = eCancelAF_AlreadyPassTo3A;
                        }
                        else
                        {
                            mCancelAfStatus = eCancelAF_NoRequest;
                        }
                        MY_LOGI("request:%d cancelAfStatus(eCancelAF_ReceivedRequest) bInPrecapturePeriod(%d), need to pass MTK_CONTROL_AF_TRIGGER_CANCEL to 3A, then CancelAfStatus(%d -> %d) ",requestNo,bInPrecapturePeriod,cancelAfStatus,mCancelAfStatus);
                    }
                    else
                    {
                        if(bInPrecapturePeriod)
                        {
                            MY_LOGI("request:%d cancelAfStatus(%d), no need to do cancelAutoFocus(), MTK_CONTROL_AF_TRIGGER change to 0",requestNo, cancelAfStatus);
                            afTrigger = MTK_CONTROL_AF_TRIGGER_IDLE;
                            IMetadata::setEntry<MUINT8>(&rAppMetaData, MTK_CONTROL_AF_TRIGGER, afTrigger);
                            rIsChanged = MTRUE;
                        }
                        else
                        {
                            MY_LOGI("request:%d cancelAfStatus(%d), not in precapture, need to pass MTK_CONTROL_AF_TRIGGER_CANCEL to 3A", cancelAfStatus);
                        }
                    }
                    //
                    {
                        Mutex::Autolock _l(mAutoFocusLock);
                        mNeedAfCallback = MFALSE;
                    }
                }
                else if(afTrigger == MTK_CONTROL_AF_TRIGGER_START)
                {
                    MY_LOGI("request:%d cancelAfStatus(%d), MTK_CONTROL_AF_TRIGGER_START need to do autofocus",requestNo, cancelAfStatus);
                    Mutex::Autolock _l(mAutoFocusLock);
                    mNeedAfCallback = MTRUE;
                }
            }
            else
            {
                MY_LOGW("request:%d NO MTK_CONTROL_AF_TRIGGER");
            }
        }

    }
    //
    MY_LOGD_IF((bInPrecapturePeriod||bNeedTriggerPrecapture||mLogLevel>1),"bInPrecapturePeriod(%d) bNeedTriggerPrecapture(%d) cancelAfStatus(%d)",bInPrecapturePeriod, bNeedTriggerPrecapture, cancelAfStatus);
    if(bInPrecapturePeriod &&
       metaHalStreamId == eSTREAMID_META_HAL_CONTROL &&
       bNeedTriggerPrecapture &&
       (cancelAfStatus == eCancelAF_NoRequest || cancelAfStatus == eCancelAF_AlreadyPassTo3A))
    {
        MY_LOGI("precapture request:%d cancelAfStatus(%d) mCancelAfStatus(%d) mAemode(%d) mbBvTrigger(%d)",
            requestNo,cancelAfStatus,mCancelAfStatus,mAemode,mbBvTrigger);
        //
        mAeRequest                = requestNo;
        mListenPreCapture         = MTRUE;
        mListenPreCaptureStart    = MFALSE;
        mListenAeResult           = MTRUE;
        mListenAfResultPreCapture = MFALSE;
        mSkipCheckAeRequestNumber = MFALSE;
        //
        IMetadata::setEntry<MUINT8>(&rHalMetaData, MTK_HAL_REQUEST_PRECAPTURE_START, 1);
        //
        // get Client's property.
        String8 s8ClientAppModeVal(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        ::property_get("vendor.client.em.appmode", value, "NULL");
        s8ClientAppModeVal = value;
        bool engMode = false;
        if ( s8ClientAppModeVal == MtkCameraParameters::APP_MODE_NAME_MTK_ENG )
        {
            engMode = true;
        }
        //
        String8 pFocusMode = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE));
        String8 pAeLock = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_AUTO_EXPOSURE_LOCK));
        MBOOL isAeAfLock = !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_AUTO)) && !strcmp(pAeLock, String8(CameraParameters::TRUE));
        MY_LOGI("FocusMode = %s, AeLock = %s, isAeAfLock = %d, bNeedFlash = %d, mAfstate = %d, afTrigger = %d, appmode = %s, engMode = %d",
            pFocusMode.string(), pAeLock.string(), isAeAfLock, bNeedFlash, mAfstate, afTrigger, s8ClientAppModeVal.string(),engMode);
        MBOOL isSupportAF = mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) == 0 ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_INFINITY)) ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_FIXED)) ? MFALSE :
                           !strcmp(pFocusMode, String8("manual")) ? MFALSE :
                           isAeAfLock ? MFALSE : MTRUE;
        MBOOL needAf = MFALSE;

        if(isSupportAF && (bNeedFlash || engMode || mAfstate == MTK_CONTROL_AF_STATE_ACTIVE_SCAN || mAfstate == MTK_CONTROL_AF_STATE_PASSIVE_SCAN || afTrigger == MTK_CONTROL_AF_TRIGGER_START))
        {
            IMetadata::setEntry<MUINT8>(&rHalMetaData, MTK_HAL_REQUEST_AF_TRIGGER_START, 1);
            //
            mListenAfResultPreCapture = MTRUE;
            needAf = MTRUE;
            MY_LOGI("set MTK_HAL_REQUEST_AF_TRIGGER_START = 1");
        }
        else
        {
            MY_LOGI("set MTK_FOCUS_PAUSE = 1");
            IMetadata::setEntry<MUINT8>(&rHalMetaData, MTK_FOCUS_PAUSE, 1);
        }

        rIsChanged = MTRUE;

        {
            Mutex::Autolock _l(mPreCap3ALock);
            mbNeedTriggerPrecapture = MFALSE;
        }

        if(mListenFlashResultPreCapture &&
           mAemode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && mbBvTrigger==MFALSE)
        {
            mListenFlashResultPreCapture = MFALSE;
            MY_LOGD("AE Mode = auto flash & bvTrigger(0) => precapture don't listen flash result");
        }

        MY_LOGI("reqNo %d, metaHalStreamId %#" PRIx64 " MTK_HAL_REQUEST_PRECAPTURE_START(1) MTK_HAL_REQUEST_AF_TRIGGER_START(%d)",
                requestNo,
                metaHalStreamId,
                needAf);
    }
#if 1
    //to guarantee AF Mode can't be changed during precapture
    if(bInPrecapturePeriod)
    {
        if(metaAppStreamId == eSTREAMID_META_APP_CONTROL && mListenAfResultPreCapture == MTRUE)
        {
            MUINT8 afMode = 0;
            //
            if ( IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&rAppMetaData), MTK_CONTROL_AF_MODE, afMode) )
            {
                if(mInPrecaptureFirstAfMode == 0xFF)
                {
                    mInPrecaptureFirstAfMode = afMode;
                    MY_LOGI("In Precapture, first time MTK_CONTROL_AF_MODE(%d)",afMode);
                }
                else if( mInPrecaptureFirstAfMode != afMode )
                {
                    MY_LOGW("mInPrecaptureFirstAfMode(%d), MTK_CONTROL_AF_MODE(%d) need to change to (%d)",
                        mInPrecaptureFirstAfMode,afMode,mInPrecaptureFirstAfMode);
                    IMetadata::setEntry<MUINT8>(&rAppMetaData, MTK_CONTROL_AF_MODE, mInPrecaptureFirstAfMode);
                    IMetadata::setEntry<MUINT8>(&rHalMetaData, MTK_HAL_REQUEST_REPEAT, 0);
                    rIsChanged = MTRUE;
                }
                else
                {
                    // mInPrecaptureFirstAfMode == afMode && not first request during precapture
                    // do nothing here
                }
            }
            else
            {
                MY_LOGW("MTK_CONTROL_AF_MODE is NULL!");
            }
        }
    }
    else
    {
        mInPrecaptureFirstAfMode = 0xFF;
    }
#endif
#if 0
    //to guarantee AF Mode must be same with ParamsManagerV3's newest setting
    {
        if(metaAppStreamId == eSTREAMID_META_APP_CONTROL)
        {
            IMetadata meta;
            mpParamsManagerV3->getLastUpdateRequest(meta);
            MUINT8 afModeFromParam = 0;
            MUINT8 afModeFromAppMeta = 0;
            //
            if ( IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&meta), MTK_CONTROL_AF_MODE, afModeFromParam) )
            {
                if ( IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&rAppMetaData), MTK_CONTROL_AF_MODE, afModeFromAppMeta) )
                {
                    if(afModeFromParam != afModeFromAppMeta)
                    {
                        MY_LOGD("MTK_CONTROL_AF_MODE (ParamV3:%d AppMeta:%d), need to change MTK_CONTROL_AF_MODE to (%d)",
                            afModeFromParam,afModeFromAppMeta,afModeFromParam);
                        IMetadata::setEntry<MUINT8>(&rAppMetaData, MTK_CONTROL_AF_MODE, afModeFromParam);
                        rIsChanged = MTRUE;
                    }
                }
                else
                {
                    MY_LOGW("eSTREAMID_META_APP_CONTROL:MTK_CONTROL_AF_MODE is null");
                }
            }
            else
            {
                MY_LOGW("getLastUpdateRequest:MTK_CONTROL_AF_MODE is null");
            }
        }
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
#if 0
    MY_LOGD2("reqNo %d, errRes %d",
            requestNo,
            errorResult);

    // update request number
    {
        Mutex::Autolock _l(mAutoFocusLock);
        mListenAfResult = ( requestNo == mAfRequest ) ? MTRUE : mListenAfResult;
    }
    MINT32 masterid = -1;
    //
    if ( errorResult )
    {
        if (requestNo == mAeRequest)
        {
            MY_LOGW("mAeRequest(%d) had errorResult", mAeRequest);
            mSkipCheckAeRequestNumber = MTRUE;
        }
        return;
    }
    // update 3A
    {
        Mutex::Autolock _l(m3ALock);
        MUINT8 curAfstate = 0xFF;
        tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AF_STATE, curAfstate);
        tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_STATE, mAestate);
        #if DUALCAM
        if (StereoSettingProvider::isDualCamMode()
            && eSTREAMID_META_HAL_DYNAMIC_P1 == streamId)
        {
            tryGetMetadata< MINT32 >(result, MTK_STEREO_SYNC2A_MASTER_SLAVE, masterid);
        }
        #endif
        //
        if(curAfstate != 0xFF)
        {
            if( mbUpdateAf==MFALSE)
            {
                mAfstateLastCallback = curAfstate;
            }
            mbUpdateAf = MTRUE;
            mAfstate = curAfstate;
        }
    }
    //
    // update AE/AF/Flash state for PreCapture
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture) {
            MY_LOGD1("requestNo:(%d,%d) mAestate: %d, mAfstate %d", requestNo, mAeRequest, mAestate, mAfstate);
            MY_LOGD1("mListenAeResult: %d, mListenAfResultPreCapture: %d, mListenFlashResultPreCapture: %d", mListenAeResult, mListenAfResultPreCapture, mListenFlashResultPreCapture);

            // check if starting to receive AE and AF results
            if ((mListenAeResult || mListenAfResultPreCapture || mListenFlashResultPreCapture)
                && (requestNo == mAeRequest || mSkipCheckAeRequestNumber))
            {
                    mListenPreCaptureStart = MTRUE;
            }

            if (mListenPreCaptureStart) {
                if (mListenAeResult) {
                    MY_LOGD1("requestNo:(%d,%d) mAestate: %d", requestNo, mAeRequest, mAestate);
                    if (mAestate == MTK_CONTROL_AE_STATE_CONVERGED || mAestate == MTK_CONTROL_AE_STATE_FLASH_REQUIRED)
                    {

                        mbAeFlashRequired = (mAestate == MTK_CONTROL_AE_STATE_FLASH_REQUIRED ? MTRUE : MFALSE);
                        mListenAeResult = MFALSE;
                        //
                        MUINT8 curAeMode = 0xFF;
                        if ( tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_MODE, curAeMode) ) {
                            mAemode = curAeMode;
                            if( mAemode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
                                mbAeFlashRequired == MTRUE)
                            {
                                mbNeedWaitFlashDone = MTRUE;
                                MY_LOGD1("requestNo:(%d,%d) mAemode:%d, mbAeFlashRequired:%d, mbNeedWaitFlashDone:%d", requestNo, mAeRequest, mAemode, mbAeFlashRequired, mbNeedWaitFlashDone);
                            }
                        }
                    }
                }

                /*if (mListenAfResultPreCapture)
                {
                    MY_LOGD1("requestNo:%d mAfstate: %d", requestNo, mAfstate);
                    if (mAfstate == MTK_CONTROL_AF_STATE_FOCUSED_LOCKED  ||
                        mAfstate == MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED)
                    {
                        mListenAfResultPreCapture = MFALSE;
                    }
                }*/
                //
                if(mListenFlashResultPreCapture)
                {
                    MUINT8 curFlashstate = 0xFF;
                    if ( tryGetMetadata< MUINT8 >(result, MTK_FLASH_STATE, curFlashstate) ) {
                        mLastFlashstate = mFlashstate;
                        mFlashstate = curFlashstate;
                    }
                    //
                    MY_LOGD1("requestNo:%d, mbNeedWaitFlashDone:%d, mAestate:%d, mLastFlashstate:%d, mFlashstate:%d", requestNo, mbNeedWaitFlashDone, mAestate, mLastFlashstate, mFlashstate);
                    if( mbNeedWaitFlashDone )
                    {
                        if(mLastFlashstate == MTK_FLASH_STATE_FIRED && mFlashstate == MTK_FLASH_STATE_READY)
                        {
                            MY_LOGD1("requestNo:%d, Flash Fired Done, mListenFlashResultPreCapture change to 0  ", requestNo);
                            mListenFlashResultPreCapture = MFALSE;
                            mbNeedWaitFlashDone = MFALSE;
                        }
                    }
                    else
                    {
                        if(!mListenAeResult)
                        {
                            MY_LOGD1("requestNo:%d, No Need Wait Flash Fired, mListenFlashResultPreCapture change to 0  ", requestNo);
                            mListenFlashResultPreCapture = MFALSE;
                            mbNeedWaitFlashDone = MFALSE;
                        }
                    }
                }
                //
                if (!mListenAeResult && !mListenAfResultPreCapture && !mListenFlashResultPreCapture) {
                    Mutex::Autolock _l(mPreCap3ALock);
                    mListenPreCapture = MFALSE;
                    mLastPrecapEndRequestNo = requestNo;
                    MY_LOGD("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                    mPreCaptureCond.signal();
                }
            }
        }
    }

    // check focus callback
    if ( needFocusCallback(streamId) )
    {
        MINT msg = 0, msgExt = 0;
        if(isAfCallback(mAfstate, msg, msgExt, masterid))
        {
            Mutex::Autolock _l(mPreCap3ALock);

            if(mInPrecapturePeriod == true && (mAfTriggeredByAP == false  ||  requestNo == (MUINT32)mLastPrecapEndRequestNo))
            {

                MY_LOGD("mInPrecapturePeriod == %d , mAfTriggeredByAP == %d, mLastPrecapEndRequestNo =%d, requestNo =%d, no need do AF callback"
                         , mInPrecapturePeriod, mAfTriggeredByAP, mLastPrecapEndRequestNo, requestNo);
                if(msg == CAMERA_MSG_FOCUS)
                {
                    mNeedAfCallback = MFALSE;
                    mListenAfResult = MFALSE;
                }

            }
            else
            {
                // do multi af window callback
                //if(isSupportMultiZonAfWindow)
                {
                    checkMultiZoneAfWindow(result);
                }
                //
                doNotifyCb(
                    requestNo,
                    msg,
                    msgExt,
                    0,
                    0);
                lastAFCBMsg = msg;
                lastAFCBExt = msgExt;
            }
            mAfTriggeredByAP = false;
        }
    }
    // check zoom callback
    MINT32 zoomIndex = 0;
    if ( needZoomCallback( requestNo, zoomIndex ) ) {
        doNotifyCb(
            requestNo,
            CAMERA_MSG_ZOOM,
            zoomIndex, 0, 0
        );

        Mutex::Autolock _l(mZoomLock);
        if(mNeedQualityUp)
        {
            sp<IFeatureFlowControl> pIFeatureFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();

            if(! pIFeatureFlowControl.get())
            {
                MY_LOGE("pIFeatureFlowControl is null");
                return;
            }

            pIFeatureFlowControl->highQualityZoom();
            mNeedQualityUp = 0;
        }
    }
#endif
//
#if DUALCAM
    // for dual cam (vsdof) callback
    {
        if(StereoSettingProvider::isDualCamMode() &&
            mMain1Id == getOpenId())
        {
            if ( errorResult )
            {
                MY_LOGD("reqNo (%d) had errorResult!", requestNo);
            }
            if(streamId == eSTREAMID_META_APP_DYNAMIC_P1)
            {
                // check stereo warning
                MINT32 newResult = -1;
                if(tryGetMetadata<MINT32>(result, MTK_STEREO_FEATURE_WARNING, newResult))
                {
                    MY_LOGD1("warning(%d) currentStereoWarning(%d)", newResult, mCurrentStereoWarning);
                    if(newResult != mCurrentStereoWarning)
                    {
                        MY_LOGD("do cb MTK_STEREO_FEATURE_WARNING: %d", newResult);
                        doNotifyCb(requestNo,
                                    CAMERA_MSG_PREVIEW_METADATA,
                                    MTK_CAMERA_MSG_EXT_NOTIFY_STEREO_WARNING,
                                    newResult, 0);
                        mCurrentStereoWarning = newResult;
                    }
                }
            }
        }
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
RequestCallback(
    uint32_t requestNo,
    MINT32   _type,
    MINTPTR  _ext1,
    MINTPTR  _ext2
)
{
    MINT32 burstNum = 1;
    {
        Mutex::Autolock _l(mBurstNumLock);
        burstNum = mBurstNum;
    }
    //
    MY_LOGI("requestNo:%d burstNum:(%d) _msgType:%d %#" PRIx64 " %#" PRIx64 , requestNo, burstNum, _type, _ext1, _ext2);
    switch(_type)
    {
        case IRequestCallback::MSG_START_AUTOFOCUS:
        {
            Mutex::Autolock _l(mAutoFocusLock);
            MY_LOGI("MSG_START_AUTOFOCUS %d", requestNo);
            mAfRequest = requestNo;
            if(burstNum > 1) //slow motion
            {
                mNeedAfCallback = MTRUE;
            }
        } break;
        case IRequestCallback::MSG_START_ZOOM:
        {
            Mutex::Autolock _l(mZoomLock);
            MY_LOGI("MSG_START_ZOOM %d", requestNo);
            mZoomResultQueue.add(_ext1, requestNo);
        } break;
        case IRequestCallback::MSG_START_PRECAPTURE:
        {
            Mutex::Autolock _l(mPreCaptureLock);
            MY_LOGI("MSG_START_PRECAPTURE af %d", requestNo);

            mAeRequest                = requestNo;
            mListenPreCapture         = MTRUE;
            mListenPreCaptureStart    = MFALSE;
            mListenAeResult           = MTRUE;
            mListenAfResultPreCapture = MFALSE;
            mSkipCheckAeRequestNumber = MFALSE;
        } break;
        case IRequestCallback::MSG_START_PRECAPTURE_WITH_AF:
        {
            Mutex::Autolock _l(mPreCaptureLock);
            MY_LOGI("MSG_START_PRECAPTURE_WITH_AF af %d", requestNo);

            mAeRequest                = requestNo;
            mListenPreCapture         = MTRUE;
            mListenPreCaptureStart    = MFALSE;
            mListenAeResult           = MTRUE;
            mListenAfResultPreCapture = MTRUE;
            mSkipCheckAeRequestNumber = MFALSE;
        } break;
        default:
            MY_LOGE("Unsupported message type %d", _type);
        break;
    };
}

/******************************************************************************
 *
 ******************************************************************************/
String8
StreamingProcessorImp::
getUserName()
{
    return String8::format("StreamingProcessor");
}

/******************************************************************************
 *
 ******************************************************************************/
bool
StreamingProcessorImp::
needFocusCallback(StreamId_T const streamId)
{
    Mutex::Autolock _l(mAutoFocusLock);
    //
    if (streamId != eSTREAMID_META_HAL_DYNAMIC_P1)
    {
        return false;
    }
    if( mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE) == CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO ||      
    mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE) == CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE) 
    {
        return true;
    }
    else
    {
        MY_LOGD2("%d/%d",mNeedAfCallback,mListenAfResult);
        return ( mNeedAfCallback && mListenAfResult );
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
StreamingProcessorImp::
needZoomCallback(
    MUINT32 requestNo,
    MINT32  &zoomIndex
)
{
    Mutex::Autolock _l(mZoomLock);
    for ( size_t i = 0; i < mZoomResultQueue.size(); ++i ) {
        if ( mZoomResultQueue.valueAt(i) == requestNo ) {
            zoomIndex = mZoomResultQueue.keyAt(i);
            return true;
        }
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StreamingProcessorImp::
doNotifyCb(
    uint32_t requestNo,
    int32_t _msgType,
    int32_t _ext1,
    int32_t _ext2,
    int32_t /*_ext3*/
)
{
    int32_t msg1 = _ext1;
    int32_t msg2 = _ext2;
    //
    switch (_msgType)
    {
        case CAMERA_MSG_FOCUS:
        {
            Mutex::Autolock _l(mAutoFocusLock);
            //
            MY_LOGD("CAMERA_MSG_FOCUS requestNo:%d _msgType:%d msg:%d,%d",
                    requestNo,
                    _msgType,
                    msg1,
                    msg2);
            mNeedAfCallback = MFALSE;
            mListenAfResult = MFALSE;
        } break;
        case CAMERA_MSG_FOCUS_MOVE:
        {
            MY_LOGD("CAMERA_MSG_FOCUS_MOVE requestNo:%d _msgType:%d msg:%d,%d",
                    requestNo,
                    _msgType,
                    msg1,
                    msg2);
        } break;
        case CAMERA_MSG_ZOOM:
        {
            Mutex::Autolock _l(mZoomLock);
            //
            mZoomResultQueue.removeItem(msg1);
            msg2 = (mUninit) || (msg1 == (MINT32)mZoomTargetIndex) || (!mZoomTriggerd);
            MY_LOGD("smoothZoom requestNo:%d (%d, %d) target:%d uninit:%d",
                requestNo, msg1, msg2, mZoomTargetIndex, mUninit
            );
            //
            mpParamsManagerV3->getParamsMgr()->set(
                CameraParameters::KEY_ZOOM,
                msg1
            );
            if (msg2 == 1) //last zoom
            {
                mZoomTriggerd = MFALSE;
                mNeedQualityUp = 1;
            }
            mWaitZoomLock.signal();
        } break;
#if DUALCAM
        case CAMERA_MSG_PREVIEW_METADATA:
        {
            _msgType = MTK_CAMERA_MSG_EXT_NOTIFY;
        }
        break;
#endif
        default:
            MY_LOGE("Unsupported message type %d", _msgType);
            return;
        break;
    };
    //
    mpCamMsgCbInfo->doNotifyCallback(
        _msgType,
        msg1,
        msg2
    );
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StreamingProcessorImp::
startAutoFocus()
{
    FUNC_START;
    //
    {
        Mutex::Autolock _l(mPreCap3ALock);
        mAfTriggeredByAP = MTRUE;
    }
    //
    MUINT8 afState = 0;
    MUINT8 afMode  = 0;
    String8 const s = mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE);
    if  ( !s.isEmpty() ) {
        afMode = PARAMSMANAGER_MAP_INST(IParamsManager::eMapFocusMode)->valueFor(s);
    } else {
        MY_LOGW("cannot get focus mode from parameter.");
    }

    MY_LOGD1("afMode %d", afMode );

    {
        Mutex::Autolock _l(m3ALock);
        afState = mAfstate;
    }
    //
    {
        IParamsManager::IMap const* focusMap = IParamsManager::getMapInst(IParamsManager::int2type<IParamsManager::eMapFocusMode>());
        /**
          * If the camera does not support auto-focus, it is a no-op and
          * onAutoFocus(boolean, Camera) callback will be called immediately
          * with a fake value of success set to true.
          *
          * Similarly, if focus mode is set to INFINITY, there's no reason to
          * bother the HAL.
          */
        if ( focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_FIXED) ) == afMode ||
             focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_INFINITY) ) == afMode) {
            if ( mpCamMsgCbInfo->msgTypeEnabled(CAMERA_MSG_FOCUS) ) {
                mpCamMsgCbInfo->doNotifyCallback(CAMERA_MSG_FOCUS, 1, 0);
                //
                MY_LOGD1("afMode %d", afMode);
                //
                FUNC_END;
                return OK;
            }
        }

        /**
         * If we're in CAF mode, and AF has already been locked, just fire back
         * the callback right away; the HAL would not send a notification since
         * no state change would happen on a AF trigger.
         */
        if ( ( afMode == focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE) ) ||
               afMode == focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO) ) ) &&
               afState == ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED ) {
            if ( mpCamMsgCbInfo->msgTypeEnabled(CAMERA_MSG_FOCUS) ) {
                mpCamMsgCbInfo->doNotifyCallback(CAMERA_MSG_FOCUS, 1, 0);
                //
                MY_LOGD1("afMode %d", afMode);
                //
                FUNC_END;
                return OK;
            }
        }
    }
    //
    {
        Mutex::Autolock _l(mCancelAfStatusLock);
        if( mCancelAfStatus == eCancelAF_ReceivedRequest)
        {
            mCancelAfStatus = eCancelAF_NoRequest;
            MY_LOGD("mCancelAfStatus: eCancelAF_ReceivedRequest => eCancelAF_NoRequest");
        }
    }
    //
    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder != 0 ) {
        builder->triggerAutofocus(static_cast< IRequestCallback* >(this));
    } else {
        MY_LOGW("builder cannot be promoted.");
    }

    FUNC_END;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StreamingProcessorImp::
cancelAutoFocus()
{
    FUNC_START;
    //
    {
        Mutex::Autolock _l(mAutoFocusLock);
        //mNeedAfCallback = MFALSE;
        mListenAfResult = MFALSE;
    }
    //
    {
        Mutex::Autolock _l(mPreCap3ALock);
        mAfTriggeredByAP = MFALSE;
    }
    //
    MUINT8 afMode = 0;
    String8 const s = mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE);
    if  ( !s.isEmpty() ) {
        afMode = PARAMSMANAGER_MAP_INST(IParamsManager::eMapFocusMode)->valueFor(s);
    }
    // Canceling does nothing in FIXED or INFINITY modes
    IParamsManager::IMap const* focusMap = IParamsManager::getMapInst(IParamsManager::int2type<IParamsManager::eMapFocusMode>());
    if ( focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_FIXED) ) == afMode ||
         focusMap->valueFor( String8(CameraParameters::FOCUS_MODE_INFINITY) ) == afMode) {
        //
        MY_LOGD1("afMode %d", afMode);
        //
        FUNC_END;
        return OK;
    }
    //
    {
        Mutex::Autolock _l(mCancelAfStatusLock);
        if( mCancelAfStatus == eCancelAF_NoRequest)
        {
            mCancelAfStatus = eCancelAF_ReceivedRequest;
            MY_LOGD("mCancelAfStatus: eCancelAF_NoRequest => eCancelAF_ReceivedRequest");
        }
    }
    //
    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder != 0 ) {
        builder->triggerCancelAutofocus();
    } else {
        MY_LOGW("builder cannot be promoted.");
    }

    FUNC_END;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StreamingProcessorImp::
preCapture(int& flashRequired, nsecs_t tTimeout)
{
    CAM_TRACE_CALL();

    FUNC_START;

    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder == 0 )
    {
        MY_LOGW("builder cannot be promoted.");
        return OK;
    }

    {
        MERROR res;
        String8 pAeLock = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_AUTO_EXPOSURE_LOCK));
        String8 pFocusMode = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE));
        //(Focus mode = Auto && AeLock = true) => AE/AF lock ON
        MBOOL isAeAfLock = !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_AUTO)) && !strcmp(pAeLock, String8(CameraParameters::TRUE));
        mPrecapInAeAfLock = isAeAfLock;
        MY_LOGD("FocusMode = %s, AeLock = %s", pFocusMode.string(), pAeLock.string());
        if (isAeAfLock)
        {
            mListenFlashResultPreCapture = MTRUE;
        }

        MBOOL isSupportAF = mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) == 0 ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_INFINITY)) ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_FIXED)) ? MFALSE :
                           !strcmp(pFocusMode, String8("manual")) ? MFALSE :
                           isAeAfLock ? MFALSE : MTRUE;

        String8 sFlashMode = mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FLASH_MODE);
        //
        #if 0
        MBOOL needAf = isSupportAF & !(!strcmp(pFlashMode, CameraParameters::FLASH_MODE_ON) && mAestate == MTK_CONTROL_AE_STATE_CONVERGED);
        #else
        MBOOL needAf = isSupportAF;
        #endif
        MY_LOGI("FlashMode = %s aeState = %d isSupportAF = %d needAf = %d isAeAfLock = %d", sFlashMode.string(), mAestate, isSupportAF, needAf, isAeAfLock);
        //
        if(!strcmp(sFlashMode, String8(CameraParameters::FLASH_MODE_OFF)))
        {
            mListenFlashResultPreCapture = MFALSE;
        }
        //
        //To cancel triggerPrecaptureMetering for new fast precapture flow
        //builder->triggerPrecaptureMetering(static_cast< IRequestCallback* >(this), needAf);
        Mutex::Autolock _l(mPreCaptureLock);
        MY_LOGI("preCapture tTimeout(%" PRId64 ")",tTimeout);
        //
        {
            Mutex::Autolock _l(mPreCap3ALock);
            mInPrecapturePeriod = MTRUE;
            mbNeedTriggerPrecapture = MTRUE;
            mbAeFlashRequired = MFALSE;
        }
        //
        if ( tTimeout != 0LL)
        {
            res = mPreCaptureCond.waitRelative(mPreCaptureLock, tTimeout);
            if (OK != res)
            {
                dump();
                if(!mListenPreCaptureStart)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - precapture not started");
                }
                else if(mLastErrorResult)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - received errorResult");
                }
                else if(mListenAeResult)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - AE state not converged");
                }
                else if(mListenFlashResultPreCapture)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - Flash state not converged");
                }
                else if(mListenAfResultPreCapture)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - AF state not converged");
                }
                else
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout");
                }
            }
        }
        else
        {
            mPreCaptureCond.wait(mPreCaptureLock);
        }

        {
            Mutex::Autolock _l(mPreCap3ALock);
            mInPrecapturePeriod = MFALSE;
            {
                Mutex::Autolock _l(mCancelAfStatusLock);
                if(mCancelAfStatus==eCancelAF_AlreadyPassTo3A)
                {
                    mCancelAfStatus = eCancelAF_NoRequest;
                    MY_LOGD("precapture done, mCancelAfStatus(eCancelAF_AlreadyPassTo3A -> eCancelAF_NoRequest");
                }
            }
        }

        MY_LOGI("AE is already in good state, mbAeFlashRequired(%d), getAfTriggered(%d)", mbAeFlashRequired, mpParamsManagerV3->getAfTriggered());
        flashRequired = (mbAeFlashRequired == MTRUE);
        mpParamsManagerV3->setCancelAF(!mpParamsManagerV3->getAfTriggered());
    }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StreamingProcessorImp::
startSmoothZoom(int value)
{
    FUNC_START;
    //
    MY_LOGD_IF( 1, "startSmoothZoom %d", value );
    //
    int currentIndex = 0;
    int inc = 0;
    {
        Mutex::Autolock _l(mZoomLock);
        mZoomTargetIndex = value;
        currentIndex = mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_ZOOM);
        if( value < 0 || value > mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_ZOOM) )
        {
            MY_LOGE("return fail: smooth zoom(%d)", value);
            return BAD_VALUE;
        }

        if( value == currentIndex )
        {
            MY_LOGE("smooth zoom(%d) equals to current", value);
            return OK;
        }

        inc = ( (MINT32)mZoomTargetIndex > currentIndex ) ? 1 : -1;
    }

    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder != 0 ) {
        for ( currentIndex += inc ; currentIndex != value + inc; currentIndex += inc ) {
            MY_LOGD1("triggerTriggerZoom %d/%d", currentIndex, value );
            builder->triggerTriggerZoom( currentIndex, static_cast< IRequestCallback* >(this) );
            mZoomTriggerd = MTRUE;
        }
    } else {
        MY_LOGW("builder cannot be promoted.");
    }
    //
    FUNC_END;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StreamingProcessorImp::
stopSmoothZoom()
{
    FUNC_START;
    //
    if (mZoomTriggerd)
    {
        mZoomTriggerd = MFALSE;
        MY_LOGD("wait zoom callback stop+");
        Mutex::Autolock _l(mZoomLock);
        mWaitZoomLock.wait(mZoomLock);
        MY_LOGD("wait zoom callback stop -");
    }
    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder != 0 ) {
        builder->triggerCancelZoom();
    } else {
        MY_LOGW("builder cannot be promoted.");
    }
    //
    {
        Mutex::Autolock _l(mZoomLock);
        if(mZoomResultQueue.size()>0)
        {
            mZoomTargetIndex = mZoomResultQueue.valueAt(mZoomResultQueue.size() - 1);
        }
    }
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
StreamingProcessorImp::
isAfCallback(
    MUINT8  afState,
    MINT&   msg,
    MINT&   msgExt,
    MINT32  masterid)
{
    bool bAfCallback = false;
    msgExt = 0;
    //
    if(!mbUpdateAf)
    {
        MY_LOGD1("No AF update");
        return bAfCallback;
    }
    //
    switch(afState)
    {
        case MTK_CONTROL_AF_STATE_INACTIVE:
        case MTK_CONTROL_AF_STATE_ACTIVE_SCAN:
        {
            //No need to callback
            break;
        }
        case MTK_CONTROL_AF_STATE_FOCUSED_LOCKED:
        case MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
        {
            if( afState == MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
            {
                msgExt = 1;
            }
#if DUALCAM
            if (StereoSettingProvider::isDualCamMode())
            {
                if (masterid == mOpenId)
                {
                    bAfCallback = true;
                }
                else
                {
                    bAfCallback = false;
                }
            }
            else
            {
                bAfCallback = true;
            }
#else
             bAfCallback = true;
#endif
            msg = CAMERA_MSG_FOCUS;
            break;
        }
        case MTK_CONTROL_AF_STATE_PASSIVE_SCAN:
        case MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED:
        case MTK_CONTROL_AF_STATE_PASSIVE_UNFOCUSED:
        {
            MBOOL doCb = MTRUE;
            #if DUALCAM
            if (StereoSettingProvider::isDualCamMode())
            {
                if( afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN )
                {
                    if (masterid == mOpenId)
                    {
                        mneedAFUpdate = true;
                    }
                    else
                    {
                        bAfCallback = false;
                        break;
                    }
                }
                else
                {
                    if (mneedAFUpdate)
                    {
                        mneedAFUpdate = false;
                    }
                    else
                    {
                        bAfCallback = false;
                        break;
                    }
                }
            }
            #endif
            if( afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN )
            {
                msgExt = 1;
            }
            // if current status is passive focused/unfocused,
            // and last status is not passive scan,
            // no need to do callback
            else if(mAfstateLastCallback != MTK_CONTROL_AF_STATE_PASSIVE_SCAN)
            {
                doCb = MFALSE;
            }
            //
            if ( doCb && mpCamMsgCbInfo->msgTypeEnabled( CAMERA_MSG_FOCUS_MOVE ) )
            {
                bAfCallback = true;
                msg = CAMERA_MSG_FOCUS_MOVE;
            }
            else
            {
                MY_LOGD2("CAMERA_MSG_FOCUS_MOVE has been disable or doCb(%d)", doCb);
            }
            break;
        }

        default:
        {
            MY_LOGW("Un-process af state %d",afState);
            break;
        }
    }
    //
    if(mAfstateLastCallback != afState)
    {
        MY_LOGI("AFstate(%d -> %d), msg(%d), msgExt(%d), AfCb(%d)",
                mAfstateLastCallback,
                afState,
                msg,
                msgExt,
                bAfCallback);
        //
        mAfstateLastCallback = afState;
    }
    else
    {
        bAfCallback = false;
    }
    //
    return bAfCallback;
}

status_t
StreamingProcessorImp::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    MY_LOGD("cmd:%d arg1:%d arg2:%d",cmd,arg1,arg2);
    //
    if( cmd == eSP_extCmd_resume_focus )
    {
        MY_LOGD("eSP_extCmd_resume_focus");
        Mutex::Autolock _l(mbResumeFocusLock);
        mbNeedResumeFocus = MTRUE;
        return OK;
    }
    //
    if( cmd == eSP_extCmd_burst_num )
    {
        MY_LOGD("eSP_extCmd_burst_num (%d)", arg1);
        Mutex::Autolock _l(mBurstNumLock);
        mBurstNum = arg1;
        return OK;
    }
    return INVALID_OPERATION;
}

void
StreamingProcessorImp::
checkMultiZoneAfWindow(IMetadata const result)
{
    /*
    * MTK_FOCUS_AREA_POSITION
    * MINT32
    * X1, Y1, X2, Y2 ...
    *
    * MTK_FOCUS_AREA_SIZE
    * MSize
    * (w, h)
    *
    * MTK_FOCUS_AREA_RESULT
    * MUINT8
    * res1, res2, ...
    */

    /**************************************************************
    * Get informaiton from metadata
    ***************************************************************/
    IMetadata::IEntry entryPosition = result.entryFor(MTK_FOCUS_AREA_POSITION);
    IMetadata::IEntry entrySize     = result.entryFor(MTK_FOCUS_AREA_SIZE);
    IMetadata::IEntry entryResult   = result.entryFor(MTK_FOCUS_AREA_RESULT);
    // how many af window
    MUINT windowCount = 0;
    // no update in multi-zone in meta, meta will be empty
    if( entryPosition.isEmpty() || entrySize.isEmpty() || entryResult.isEmpty() )
    {
        MY_LOGD2("No multi-zone AF update");
        return;
    }
    // there are X and Y in each af window position, so divide by 2 to get amount of windows
    windowCount = entryPosition.count()/2;
    // check if entryResult have same window number
    if ( entryResult.count() != windowCount )
    {
        MY_LOGW("Multi-AF Position#(%d)!=result#(%d)", windowCount, entryResult.count());
        return;
    }
    //
    MINT32 activePisitoinX[windowCount];
    MINT32 activePisitoinY[windowCount];
    MSize  focusAreaSize;
    MUINT8 focusResults[windowCount];
    // get position
    for (MUINT i = 0; i < entryPosition.count(); i++)
    {
        MINT32 val = entryPosition.itemAt(i, Type2Type<MINT32>());
        if (i % 2 ==0)
        {
            activePisitoinX[i/2] = val;
            MY_LOGD2("Position X%d: %d", i/2, val);
        }
        else
        {
            activePisitoinY[i/2] = val;
            MY_LOGD2("Position Y%d: %d", i/2, val);
        }
    }
    // get area size
    tryGetMetadata< MSize >(result, MTK_FOCUS_AREA_SIZE, focusAreaSize);
    MY_LOGD2("Focus area (%dx%d)",focusAreaSize.w,focusAreaSize.h);

    // get focus results
    for (MUINT i = 0; i < entryResult.count(); i++)
    {
        focusResults[i] = entryResult.itemAt(i, Type2Type<MUINT8>());
        MY_LOGD2("Focus result %d (%d)",i, focusResults[i]);
    }

    /**************************************************************
    * Transform informaiton from active window to app window
    *
    *       ---------------------------------
    *       |      Active window            |
    *       |  --------------------------   |
    *       |  |                        |   |
    *       |  |   Crop for display     |   |
    *       |  |                        |   |
    *       |  |                        |   |
    *       |  --------------------------   |
    *       ---------------------------------
    * and APP need -1000~+1000 for x and y
    ***************************************************************/
    MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
    MSize sensorSize;
    mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
    MINT32 appPisitoinX[windowCount];
    MINT32 appPisitoinY[windowCount];

    // convert window location
    for (MUINT i = 0; i < windowCount; i++)
    {
        appPisitoinX[i] = (((activePisitoinX[i]-reqPreviewCropRegion.p.x)*2000)/reqPreviewCropRegion.s.w)-1000;
        appPisitoinY[i] = (((activePisitoinY[i]-reqPreviewCropRegion.p.y)*2000)/reqPreviewCropRegion.s.h)-1000;
        MY_LOGD2("APP_XY_%d(%d,%d)",i,appPisitoinX[i],appPisitoinY[i]);
    }
    // convert window size
    MSize  appfocusAreaSize;
    appfocusAreaSize.w = (focusAreaSize.w * 2000) / reqPreviewCropRegion.s.w;
    appfocusAreaSize.h = (focusAreaSize.h * 2000) / reqPreviewCropRegion.s.h;
    MY_LOGD2("APP window size (%d,%d)",appfocusAreaSize.w, appfocusAreaSize.h);
    /**************************************************************
    * Callback information to APP
    ***************************************************************/
    //camera_memory* mem = mspCamMsgCbInfo->mRequestMemory(-1, sizeof(int32_t), 1, mspCamMsgCbInfo->mCbCookie);
    MINT32 arraySize = 3 + (3 * windowCount);
    MINT32 MultiAFInfo[arraySize];

    MultiAFInfo[0] = (MINT32)windowCount;
    MultiAFInfo[1] = (MINT32)appfocusAreaSize.w;
    MultiAFInfo[2] = (MINT32)appfocusAreaSize.h;

    for (int i = 3 ; i < arraySize ; )
    {
        MultiAFInfo[i]   =  appPisitoinX[(i/3)-1];
        MultiAFInfo[i+1] =  appPisitoinY[(i/3)-1];
        MultiAFInfo[i+2] =  focusResults[(i/3)-1];
        i = i+3;
    }

    mpCamMsgCbInfo->doDataCallback(MTK_CAMERA_MSG_EXT_DATA_AF,MultiAFInfo,arraySize);
}

void
StreamingProcessorImp::
dump()
{
    MY_LOGI("cameraId:%d logLevel:%d multiZoom:%d uninit:%d"
        , mOpenId, mLogLevel, isSupportMultiZonAfWindow, mUninit);
    //
    String8 str = String8::format("Zoom info targetIdx:%d trigger:%d"
        , mZoomTargetIndex, mZoomTriggerd);
    if ( !mZoomResultQueue.isEmpty() ) {
        str += String8::format(" req(%zd): ", mZoomResultQueue.size());
        for (size_t i = 0; i < mZoomResultQueue.size(); ++i)
            str += String8::format(" %d, ", mZoomResultQueue.valueAt(i));
    }
    MY_LOGI("%s", str.string());
    //
    MY_LOGI("[%zd]Focus info cbAFstate:%d AFres:%d needAF:%d needAFUpdate : %d"
        , mAfRequest, mAfstateLastCallback, mListenAfResult, mNeedAfCallback, mneedAFUpdate);
    MY_LOGI("[%zd]precapture info AE:%d AF:%d withAE:%d withAF:%d check:%d start:%d"
        , mAeRequest, mAestate, mAfstate
        , mListenAeResult, mListenAfResultPreCapture
        , mSkipCheckAeRequestNumber
        , mListenPreCaptureStart);
    MY_LOGI("[%zd]Flash info FlashState:%d LastFlashState:%d NeedWaitFlashDone:%d ListenFlashResultPreCapture:%d"
        , mAeRequest
        , mFlashstate
        , mLastFlashstate
        , mbNeedWaitFlashDone
        , mListenFlashResultPreCapture);
    //
    MY_LOGI("multi zone af info (%d, %d, %d, %d) msg:%d ext:%d"
        , mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h
        , lastAFCBMsg, lastAFCBExt);
    // mpParamsManagerV3
    // mpRequestSettingBuilder->dump();
}

////////////////////////////////////////////////////////////////////////////
// LegacyStreamingProcessorImp
////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 *
 ******************************************************************************/

LegacyStreamingProcessorImp::
LegacyStreamingProcessorImp(
    MINT32                            openId,
    sp<INotifyCallback>         const &rpCamMsgCbInfo,
    wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
    sp< IParamsManagerV3 >      const &rpParamsManagerV3,
    MINT32                            aStartRequestNumber,
    MINT32                            aEndRequestNumber
)
    : StreamingProcessorImp(openId, rpCamMsgCbInfo, rpRequestSettingBuilder, rpParamsManagerV3, aStartRequestNumber, aEndRequestNumber)
    , mLastErrorResult(MFALSE)
{

}

/******************************************************************************
 *
 ******************************************************************************/

void
LegacyStreamingProcessorImp::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const /*streamId*/,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    // precapture AF
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture && mListenPreCaptureStart && mListenAfResultPreCapture) {
            MUINT8 curAfstate = 0xFF;
            if ( !errorResult && !tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AF_STATE, curAfstate) ) {
                //MY_LOGW("AF state not update.");
            }
            MY_LOGD_IF( mLogLevel >= 1, "requestNo:(%d) curAfstate:(%d)", requestNo, curAfstate);
            if (curAfstate == MTK_CONTROL_AF_STATE_FOCUSED_LOCKED  ||
                curAfstate == MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED)
            {
                mListenAfResultPreCapture = MFALSE;
            }

            if (!mListenAeResult && !mListenAfResultPreCapture) {
                Mutex::Autolock _l(mPreCap3ALock);
                mListenPreCapture = MFALSE;
                mLastPrecapEndRequestNo = requestNo;
                MY_LOGD("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                mPreCaptureCond.signal();
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
LegacyStreamingProcessorImp::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    MY_LOGD2("reqNo %d, errRes %d, streamId(%#" PRIx64 ")",
            requestNo,
            errorResult,
            streamId);
    //
    mLastErrorResult = errorResult;
    // update request number
    {
        Mutex::Autolock _l(mAutoFocusLock);
        mListenAfResult = ( requestNo == mAfRequest ) ? MTRUE : mListenAfResult;
    }
    MINT32 masterid = -1;
    //
    if ( errorResult )
    {
        if (requestNo == mAeRequest)
        {
            MY_LOGW("mAeRequest(%d) had errorResult", mAeRequest);
            mSkipCheckAeRequestNumber = MTRUE;
        }
        return;
    }

    // update 3A
    {
        Mutex::Autolock _l(m3ALock);
        MUINT8 curAfstate = 0xFF;
        if(eSTREAMID_META_APP_DYNAMIC_P1 == streamId)
        {
            tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AF_STATE, curAfstate);
            tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_STATE, mAestate);
            tryGetMetadata< MUINT8 >(result, MTK_CONTROL_AE_MODE, mAemode);
        }
        else if(eSTREAMID_META_HAL_DYNAMIC_P1 == streamId)
        {
            tryGetMetadata< MBOOL >(result, MTK_3A_AE_BV_TRIGGER, mbBvTrigger);
        }
        //
        #if DUALCAM
        if (StereoSettingProvider::isDualCamMode()
            && eSTREAMID_META_HAL_DYNAMIC_P1 == streamId)
        {
            tryGetMetadata< MINT32 >(result, MTK_STEREO_SYNC2A_MASTER_SLAVE, masterid);
        }
        #endif
        //
        if(curAfstate != 0xFF)
        {
            if( mbUpdateAf==MFALSE)
            {
                mAfstateLastCallback = curAfstate;
            }
            mbUpdateAf = MTRUE;
            mAfstate = curAfstate;
        }
    }
    //
    // update AE/AF/Flash state for PreCapture
    {
        Mutex::Autolock _l(mPreCaptureLock);
        if (mListenPreCapture) {
            MY_LOGD1("requestNo:(%d,%d) mAestate: %d, mAfstate %d", requestNo, mAeRequest, mAestate, mAfstate);
            MY_LOGD1("mListenAeResult: %d, mListenAfResultPreCapture: %d, mListenFlashResultPreCapture: %d",
                mListenAeResult, mListenAfResultPreCapture, mListenFlashResultPreCapture);

            // check if starting to receive AE and AF results
            if ((mListenAeResult || mListenAfResultPreCapture || mListenFlashResultPreCapture)
                && (requestNo == mAeRequest || mSkipCheckAeRequestNumber))
            {
                    mListenPreCaptureStart = MTRUE;
                    MY_LOGD2("requestNo:(%d,%d) (ListenAE:%d, ListenAF:%d, ListenFlash:%d, SkipCheck:%d) => mListenPreCaptureStart = MTRUE",
                        requestNo, mAeRequest,mListenAeResult, mListenAfResultPreCapture, mListenFlashResultPreCapture, mSkipCheckAeRequestNumber);
            }

            if (mListenPreCaptureStart) {
                if(mAemode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
                   (mAemode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && mbBvTrigger))
                {
                    mbAeFlashRequired = MTRUE;
                    if(mListenFlashResultPreCapture)
                    {
                        mbNeedWaitFlashDone = MTRUE;
                        MY_LOGD2("requestNo:(%d,%d) mAemode:%d mbBvTrigger:%d mListenFlashResultPreCapture:1 => mbNeedWaitFlashDone = MTRUE",
                            requestNo, mAeRequest, mAemode, mbBvTrigger);
                    }
                }
                MY_LOGD1("requestNo:(%d,%d) mAemode:%d, mbAeFlashRequired:%d, mbNeedWaitFlashDone:%d",
                    requestNo, mAeRequest, mAemode, mbAeFlashRequired, mbNeedWaitFlashDone);
                //
                if (mListenAeResult) {
                    if (mAestate == MTK_CONTROL_AE_STATE_CONVERGED || mAestate == MTK_CONTROL_AE_STATE_FLASH_REQUIRED )
                    {
                        mListenAeResult = MFALSE;
                        MY_LOGD2("requestNo:(%d,%d) mAestate:%d, mListenAeResult = 0", requestNo, mAeRequest, mAestate);
                    }
                }
                //
                if(mListenFlashResultPreCapture)
                {
                    MUINT8 curFlashstate = 0xFF;
                    if ( tryGetMetadata< MUINT8 >(result, MTK_FLASH_STATE, curFlashstate) ) {
                        mLastFlashstate = mFlashstate;
                        mFlashstate = curFlashstate;
                    }
                    MY_LOGD1("requestNo:(%d,%d) mAemode:%d, mbBvTrigger:%d, mLastFlashstate:%d, mFlashstate:%d",
                        requestNo, mAeRequest, mAemode, mbBvTrigger, mLastFlashstate, mFlashstate);
                    if( mbNeedWaitFlashDone )
                    {
                        if(mLastFlashstate == MTK_FLASH_STATE_FIRED && mFlashstate == MTK_FLASH_STATE_READY ||
                           mLastFlashstate == MTK_FLASH_STATE_PARTIAL && mFlashstate == MTK_FLASH_STATE_READY )
                        {
                            MY_LOGD2("requestNo:%d, Flash Fired Done, mListenFlashResultPreCapture change to 0  ", requestNo);
                            mListenFlashResultPreCapture = MFALSE;
                            mbNeedWaitFlashDone = MFALSE;
                        }
                    }
                }
                //
                if (!mListenAeResult && !mListenAfResultPreCapture && !mListenFlashResultPreCapture) {
                    Mutex::Autolock _l(mPreCap3ALock);
                    mListenPreCapture = MFALSE;
                    mLastPrecapEndRequestNo = requestNo;
                    MY_LOGD("mPreCaptureCond release, mLastPrecapEndRequestNo=%d",mLastPrecapEndRequestNo);
                    mPreCaptureCond.signal();
                }
            }
        }
    }

    // check focus callback
    if ( needFocusCallback(streamId) )
    {
        MINT msg = 0, msgExt = 0;
        if(isAfCallback(mAfstate, msg, msgExt, masterid))
        {
            Mutex::Autolock _l(mPreCap3ALock);

            if(mInPrecapturePeriod == true && (mAfTriggeredByAP == false  ||  requestNo == (MUINT32)mLastPrecapEndRequestNo))
            {

                MY_LOGD("mInPrecapturePeriod == %d , mAfTriggeredByAP == %d, mLastPrecapEndRequestNo =%d, requestNo =%d, no need do AF callback"
                         , mInPrecapturePeriod, mAfTriggeredByAP, mLastPrecapEndRequestNo, requestNo);
                if(msg == CAMERA_MSG_FOCUS)
                {
                    mNeedAfCallback = MFALSE;
                    mListenAfResult = MFALSE;
                }

            }
            else
            {
                // do multi af window callback
                //if(isSupportMultiZonAfWindow)
                {
                    checkMultiZoneAfWindow(result);
                }
                //
                doNotifyCb(
                    requestNo,
                    msg,
                    msgExt,
                    0,
                    0);
                lastAFCBMsg = msg;
                lastAFCBExt = msgExt;
            }
            mAfTriggeredByAP = false;
        }
    }
    // check zoom callback
    MINT32 zoomIndex = 0;
    if ( needZoomCallback( requestNo, zoomIndex ) ) {
        doNotifyCb(
            requestNo,
            CAMERA_MSG_ZOOM,
            zoomIndex, 0, 0
        );

        Mutex::Autolock _l(mZoomLock);
        if(mNeedQualityUp)
        {
            sp<IFeatureFlowControl> pIFeatureFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();

            if(! pIFeatureFlowControl.get())
            {
                MY_LOGE("pIFeatureFlowControl is null");
                return;
            }

            pIFeatureFlowControl->highQualityZoom();
            mNeedQualityUp = 0;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
LegacyStreamingProcessorImp::
RequestCallback(
    uint32_t requestNo,
    MINT32   _type,
    MINTPTR  _ext1,
    MINTPTR  _ext2
)
{
    MY_LOGD("requestNo:%d _msgType:%d %#" PRIx64 " %#" PRIx64 , requestNo, _type, _ext1, _ext2);
    switch(_type)
    {
        case IRequestCallback::MSG_START_AUTOFOCUS:
        {
            Mutex::Autolock _l(mAutoFocusLock);
            mAfRequest = requestNo;
            mNeedAfCallback = MTRUE;
        } break;
        case IRequestCallback::MSG_START_ZOOM:
        {
            Mutex::Autolock _l(mZoomLock);
            mZoomResultQueue.add(_ext1, requestNo);
        } break;
        case IRequestCallback::MSG_START_PRECAPTURE:
        {
            Mutex::Autolock _l(mPreCaptureLock);
            MY_LOGD("without af %d", requestNo);

            mAeRequest                = requestNo;
            mListenPreCapture         = MTRUE;
            mListenPreCaptureStart    = MFALSE;
            mListenAeResult           = MTRUE;
            mListenAfResultPreCapture = MFALSE;
            mSkipCheckAeRequestNumber = MFALSE;
        } break;
        case IRequestCallback::MSG_START_PRECAPTURE_WITH_AF:
        {
            Mutex::Autolock _l(mPreCaptureLock);
            MY_LOGD("with af %d", requestNo);

            mAeRequest                = requestNo;
            mListenPreCapture         = MTRUE;
            mListenPreCaptureStart    = MFALSE;
            mListenAeResult           = MTRUE;
            mListenAfResultPreCapture = MTRUE;
            mSkipCheckAeRequestNumber = MFALSE;
        } break;
        default:
            MY_LOGE("Unsupported message type %d", _type);
        break;
    };
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
LegacyStreamingProcessorImp::
preCapture(int& flashRequired, nsecs_t tTimeout)
{
    CAM_TRACE_CALL();

    FUNC_START;

    sp< RequestSettingBuilder > builder = mpRequestSettingBuilder.promote();
    if ( builder == 0 )
    {
        MY_LOGW("builder cannot be promoted.");
        return OK;
    }

    {
        MERROR res;
        String8 pAeLock = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_AUTO_EXPOSURE_LOCK));
        String8 pFocusMode = String8(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE));
        //(Focus mode = Auto && AeLock = true) => AE/AF lock ON
        MBOOL isAeAfLock = !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_AUTO)) && !strcmp(pAeLock, String8(CameraParameters::TRUE));
        mPrecapInAeAfLock = isAeAfLock;
        MY_LOGD("FocusMode = %s, AeLock = %s", pFocusMode.string(), pAeLock.string());
        if (isAeAfLock)
        {
            mListenFlashResultPreCapture = MTRUE;
        }

        MBOOL isSupportAF = mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS) == 0 ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_INFINITY)) ? MFALSE :
                           !strcmp(pFocusMode, String8(CameraParameters::FOCUS_MODE_FIXED)) ? MFALSE :
                           !strcmp(pFocusMode, String8("manual")) ? MFALSE :
                           isAeAfLock ? MFALSE : MTRUE;

        String8 pFlash = mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FLASH_MODE);
        const char* pFlashMode = pFlash.string();
    #if 0
        MBOOL needAf = isSupportAF & !(!strcmp(pFlashMode, CameraParameters::FLASH_MODE_ON) && mAestate == MTK_CONTROL_AE_STATE_CONVERGED);
    #else
        MBOOL needAf = isSupportAF;
    #endif
        MY_LOGD("FlashMode = %s aeState = %d isSupportAF = %d needAf = %d isAeAfLock = %d", pFlashMode, mAestate, isSupportAF, needAf, isAeAfLock);
        //
        if(!strcmp(pFlashMode, String8(CameraParameters::FLASH_MODE_OFF)))
        {
            mListenFlashResultPreCapture = MFALSE;
        }
        //
        builder->triggerPrecaptureMetering(static_cast< IRequestCallback* >(this), needAf);
        Mutex::Autolock _l(mPreCaptureLock);
        MY_LOGI("preCapture tTimeout(%lld)",tTimeout);

        {
            Mutex::Autolock _l(mPreCap3ALock);
            mInPrecapturePeriod = MTRUE;
        }

        if ( tTimeout != 0LL)
        {
            res = mPreCaptureCond.waitRelative(mPreCaptureLock, tTimeout);
            if (OK != res)
            {
                dump();
                if(!mListenPreCaptureStart)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - precapture not started");
                }
                else if(mLastErrorResult)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - received errorResult");
                }
                else if(mListenAeResult)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - AE state not converged");
                }
                else if(mListenFlashResultPreCapture)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - Flash state not converged");
                }
                else if(mListenAfResultPreCapture)
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout - AF state not converged");
                }
                else
                {
                    AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam: precapture timeout");
                }
            }
        }
        else
        {
            mPreCaptureCond.wait(mPreCaptureLock);
        }

        {
            Mutex::Autolock _l(mPreCap3ALock);
            mInPrecapturePeriod = MFALSE;
        }

        MY_LOGD1("AE is already in good state, mbAeFlashRequired %d", mbAeFlashRequired);
        flashRequired = (mbAeFlashRequired == MTRUE);
        mpParamsManagerV3->setCancelAF(!mpParamsManagerV3->getAfTriggered());
    }

    FUNC_END;
    return OK;
}


