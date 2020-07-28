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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#define LOG_TAG "MtkCam/HDRNode"
#define DEBUG_LOG_TAG "HDRNode"

#include "BaseNode.h"
#include "hwnode_utilities.h"
#include <mtkcam/v3/hwnode/HDRNode.h>

#include <utils/RWLock.h>
#include <utils/Thread.h>

#include <mtkcam/v3/hal/IHal3A.h>

#include <sys/prctl.h>
#include <sys/resource.h>

#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>

#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/drv/IHalSensor.h>

#include "../camera_custom_AEPlinetable.h"

#include <cutils/properties.h>

#include <vector>

#include <mtkcam/feature/hdr/2.0/HDRProc2.h>
#include <mtkcam/feature/hdr/2.0/utils/Debug.h>
#include <mtkcam/feature/hdr/2.0/utils/ImageBufferUtils.h>

#define P2THREAD_NAME_ENQUE "Cam@HDRNode"
#define P2THREAD_NAME_HDR   "Cam@HDRProc"

// round-robin time-sharing policy
#define P2THREAD_POLICY     SCHED_OTHER

// most threads run at normal priority
#define P2THREAD_PRIORITY   ANDROID_PRIORITY_NORMAL

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NS3Av3;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;

// ---------------------------------------------------------------------------

// defined in ispio_pipe_ports.h
// type(port type) index(port index) inout(0:in/1:out)
static const PortID PORT_IMGI  (EPortType_Memory, EPipePortIndex_IMGI , 0);
static const PortID PORT_WDMAO (EPortType_Memory, EPipePortIndex_WDMAO, 1);
static const PortID PORT_WROTO (EPortType_Memory, EPipePortIndex_WROTO, 1);

static MINT32      mHDRFrameNum;

// ---------------------------------------------------------------------------

class HDRNodeImp
    : public BaseNode
    , public HDRNode
{
    struct FrameInput
    {
        PortID                  mPortId;

        StreamId_T              streamId;

        sp<IImageStreamBuffer>  pStreamBuffer;
        sp<IImageBufferHeap>    pImageBufferHeap;
        sp<IImageBuffer>        pImageBuffer;
    };

    struct FrameOutput
    {
        PortID                  mPortId;

        StreamId_T              streamId;
        MINT32                  mTransform;

        sp<IImageStreamBuffer>  pStreamBuffer;
        sp<IImageBufferHeap>    pImageBufferHeap;
        sp<IImageBuffer>        pImageBuffer;
    };

    struct FrameParams
    {
        sp<IPipelineFrame>      pFrame;
        MINT32                  enqueIndex;
        Vector<FrameInput*>     vpIn;   // full_raw, resized_raw
        Vector<FrameOutput*>    vpOut;  // yuv_jpeg, tb_jpeg, yuv_preview

        //MBOOL                  bResized;

        IMetadata               *pMeta_InApp;
        IMetadata               *pMeta_InHal;
        IMetadata               *pMeta_OutApp;
        IMetadata               *pMeta_OutHal;
        sp<IMetaStreamBuffer>   pMeta_InAppStreamBuffer;
        sp<IMetaStreamBuffer>   pMeta_InHalStreamBuffer;
        sp<IMetaStreamBuffer>   pMeta_OutAppStreamBuffer;
        sp<IMetaStreamBuffer>   pMeta_OutHalStreamBuffer;

        FrameParams()
            : enqueIndex(-1)
            , pMeta_InApp(NULL)
            , pMeta_InHal(NULL)
            , pMeta_OutApp(NULL)
            , pMeta_OutHal(NULL)
        {}
    };

public:
    typedef sp<IPipelineFrame>                     QueNode_T;
    typedef List<QueNode_T>                        Que_T;
    typedef Vector<android::sp<IMetaStreamInfo> >  MetaStreamInfoSetT;
    typedef Vector<android::sp<IImageStreamInfo> > ImageStreamInfoSetT;

protected:
    mutable RWLock                  mConfigRWLock;
    sp<IMetaStreamInfo>             mpInAppMeta_Request;
    sp<IMetaStreamInfo>             mpInHalMeta_P1;
    sp<IMetaStreamInfo>             mpOutAppMeta_Result;
    sp<IMetaStreamInfo>             mpOutHalMeta_Result;
    sp<IImageStreamInfo>            mpInFullRaw;
    sp<IImageStreamInfo>            mpInResizedRaw;
    ImageStreamInfoSetT             mvOutImages;

public:
                                    HDRNodeImp();
    virtual                        ~HDRNodeImp();

    // IPipelineNode Interface
    virtual MERROR                  init(const InitParams& rParams);
    virtual MERROR                  config(const ConfigParams& rParams);

public:
    virtual MERROR                  uninit();

    // tell the compiler we want both the fluch from BaseNode and ours
    using BaseNode::flush;
    virtual MERROR                  flush();

    virtual MERROR                  queue(sp<IPipelineFrame> pFrame);

    static  MBOOL                   HDRProcCompleteCallback(MVOID* user, MBOOL ret);

    static MVOID                    pass2CbFunc(QParams& rParams);

    MVOID                           handleDeque(QParams& rParams);

    // getScalerCropRegion() is a helper function that
    // get the scaler crop region from static metadata information
    // and transform the coordinate system from active array to sensor
    MBOOL                           getScalerCropRegion(
                                        MRect& cropRegion, MSize& sensorSize
                                    ) const;

protected:
    MERROR                          onDequeRequest(
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;


public:
    // StreamControl
    MERROR                          getInfoIOMapSet(
                                        sp<IPipelineFrame> const& pFrame,
                                        IPipelineFrame::InfoIOMapSet& rIOMapSet,
                                        MINT32 frameIndex
                                    ) const;
    MBOOL                           isInImageStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;

    // markImageStream() is a helper function that marks image stream to
    // 1. STREAM_BUFFER_STATUS::WRITE_OK
    // 2. IUsersManager::UserStatus::USED | IUsersManager::UserStatus::RELEASE
    virtual MVOID                   markImageStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IImageStreamBuffer> const pStreamBuffer
                                    ) const;

    // unlockImage() is a helper function that unlocks
    // 1. image stream buffer
    // 2. image buffer
    virtual MVOID                   unlockImage(
                                        sp<IImageStreamBuffer> const pStreamBuffer,
                                        sp<IImageBuffer> const pImageBuffer
                                    ) const;

    // markMetaStream() is a helper function that marks meta stream to
    // 1. STREAM_BUFFER_STATUS::WRITE_OK
    // 2. IUsersManager::UserStatus::USED | IUsersManager::UserStatus::RELEASE
    virtual MVOID                   markMetaStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IMetaStreamBuffer> const pStreamBuffer
                                    ) const ;

    // unlockMetadata() is a helper function that unlocks
    // 1. meta stream buffer
    virtual MVOID                   unlockMetadata(
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        IMetadata* const pMetadata
                                    ) const ;

    virtual MRect                   calCrop(MRect const &rSrc, MRect const &rDst, uint32_t ratio);

protected:
    class ThreadBase : public Thread
    {
        public:
            ThreadBase(const char* name, const sp<HDRNodeImp>& pNodeImp);

        protected:
            char             mThreadName[256];
            sp<HDRNodeImp>   mpNodeImp;

            MERROR threadSetting(const char* threadName);
    };

    class HDRThread : public ThreadBase
    {
    public:
        HDRThread(const char* name, const sp<HDRNodeImp>& pNodeImp);

        // good place to do one-time initializations
        virtual status_t readyToRun();
        virtual bool     threadLoop();

        // ask this object's thread to exit.
        // this function is asynchronous, when the function returns
        // the thread might still be running.
        // of course, this function can be called from a different thread.
        virtual void     requestExit();
    };

    class HDRProcThread : public ThreadBase
    {
    public:
        HDRProcThread(const char* name, const sp<HDRNodeImp>& pNodeImp);

        // good place to do one-time initializations
        virtual status_t readyToRun();
        virtual bool     threadLoop();
    };

public:
MVOID                 waitForRequestDrained() const;
inline MBOOL          isStream(const sp<IStreamInfo>& streamInfo,
                          StreamId_T streamId) const;
inline MBOOL          isFullRawLocked(StreamId_T const streamId) const;

inline MBOOL          isResizedRawLocked(StreamId_T const streamId) const;

protected:  ////      Data Members. (Request Queue)
    mutable Mutex     mRequestQueueLock;
    Condition         mRequestQueueCond;
    Que_T             mRequestQueue;
    MBOOL             mbRequestDrained;
    mutable Condition mbRequestDrainedCond;
    MBOOL             mbRequestExit;
    ////                      sync with HDRProc Thread
    mutable Mutex     mMetatDataInfoLock;
    Condition         mMetatDataInfoCond;
    mutable Mutex     mHDRInputFrameLock;
    Condition         mHDRInputFrameCond;
    mutable Mutex     mHDRProcCompleteLock;
    Condition         mHDRProcCompleteCond;
    mutable Mutex     mHDRProcCompleteLock2;
    Condition         mHDRProcCompleteCond2;
    mutable Mutex     mP2Lock;
    Condition         mP2Cond;

    // since HDR needs a burst of shots for post-processing
    // we use this index to indicate these shots
    MINT32            mNodeEnqueIndex;

    // p2 frame params
    Vector<FrameParams>   mvFrameParams;

    // input working buffers for HDR post-processing
    // please note that the index ordering should be
    // {0, 2, 4, ...} for main YUV and {1, 3, 5, ...} for small Y8 frames
    vector< sp<IImageBuffer> > mvHDRInputFrames;

private:
    // threads
    sp<HDRThread>        mpHDRThread;
    sp<HDRProcThread>    mpHDRProcThread;

    // active array
    MRect                mActiveArray;

    // normal stream & 3A HAL
    // CAN ONLY BE MODIFIED via createPipe() and destroyPipe()
    INormalStream* mpPipe;
    IHal3A*        mp3A;

    // log level
    MINT32 mLogLevel;

    // debug dump (0: no dump; 1: dump src/dst buffer; 2: 1 + raw)
    MINT32 mDebugDump;

    // createPipe() is used to create INormalStream and IHal3A
    MBOOL createPipe();

    // destroyPipe() is used to destroy INormalStream and IHal3A
    MVOID destroyPipe();

    // getImageBuffer() is a helper function that
    // 1. gets image buffer heap from streambuffer (with holding a read or write lock)
    // 2. creates imagebuffer from image buffer heap
    // 3. locks imageBuffer
    //
    // return OK if success; otherwise INVALID_OPERATION is returned
    MERROR getImageBuffer(
            MINT32 type, StreamId_T const streamId,
            sp<IImageStreamBuffer>& streamBuffer,
            sp<IImageBufferHeap>& imageBufferHeap,
            sp<IImageBuffer>& imageBuffer);

    // getMetadata() is a helper function that
    // gets meta buffer from streambuffer (with holding a read lock)
    MERROR getMetadata(
            MINT32 type, StreamId_T const streamId,
            sp<IMetaStreamBuffer>& streamBuffer,
            IMetadata*& metadata);
};

// ---------------------------------------------------------------------------

template <typename T>
inline MBOOL tryGetMetadata(IMetadata* pMetadata, MUINT32 const tag, T& rVal)
{
    if (pMetadata == NULL)
    {
        HDR_LOGW("pMetadata is NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
    {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }

    return MFALSE;
}

template <typename T>
inline MVOID updateEntry(IMetadata* pMetadata, MUINT32 const tag, T const& val)
{
    if (pMetadata == NULL)
    {
        HDR_LOGW("pMetadata is NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

static inline void dumpStreamIfExist(const char* str, const sp<IStreamInfo>& streamInfo)
{
    HDR_LOGD_IF(streamInfo.get(), "%s: streamId(%#" PRIx64 ") %s",
            str, streamInfo->getStreamId(), streamInfo->getStreamName());
}

static void dumpCaptureParam(
        const NS3Av3::CaptureParam_T& captureParam, const char* msg = NULL)
{
    if (msg) HDR_LOGD("%s", msg);

    HDR_LOGD("=================");
    HDR_LOGD("u4ExposureMode(%d)", captureParam.u4ExposureMode);
    HDR_LOGD("u4Eposuretime(%d)", captureParam.u4Eposuretime);
    HDR_LOGD("u4AfeGain(%d)", captureParam.u4AfeGain);
    HDR_LOGD("u4IspGain(%d)", captureParam.u4IspGain);
    HDR_LOGD("u4RealISO(%d)", captureParam.u4RealISO);
    HDR_LOGD("u4FlareGain(%d)", captureParam.u4FlareGain);
    HDR_LOGD("u4FlareOffset(%d)", captureParam.u4FlareOffset);
    HDR_LOGD("i4LightValue_x10(%d)", captureParam.i4LightValue_x10);
}

static MVOID dumpCapPLineTable(const MINT32 aeTableIndex, const strAETable& aeTable)
{
    HDR_LOGD("tableCurrentIndex(%d) eID(%d) u4TotalIndex(%d)",
            aeTableIndex, aeTable.eID, aeTable.u4TotalIndex);
}

const char* getFileFormatName(const MINT format)
{
    switch (format)
    {
        case eImgFmt_YUY2:
            return "yuy2";
        case eImgFmt_YV12:
            return "yv12";
        case eImgFmt_I420:
            return "i420";
        default:
            HDR_LOGE("cannot find format(0x%x)", format);
            return "N/A";
    }
}

// ---------------------------------------------------------------------------

android::sp<HDRNode> HDRNode::createInstance()
{
    return new HDRNodeImp();
}

// ---------------------------------------------------------------------------

MERROR HDRNode::getCaptureParamSet(MINT32 openId, Vector<CaptureParam_T> &vCaptureParams)
{
    IHal3A *mpHal3A = IHal3A::createInstance(
            IHal3A::E_Camera_3, openId, LOG_TAG);
    if (mpHal3A == NULL)
    {
        HDR_LOGE("create 3A HAL failed");
        return NO_INIT;
    }

    // get exposure setting from 3A
    ExpSettingParam_T rExpSetting;
    mpHal3A->send3ACtrl(E3ACtrl_GetExposureInfo, (MINTPTR)(&rExpSetting), 0);

    HDRProc2& hdrProc(HDRProc2::getInstance());

    // set exposure setting to HDR proc
    hdrProc.setParam(
            HDRProcParam_Set_AOEMode, rExpSetting.u4AOEMode, 0);
    hdrProc.setParam(
            HDRProcParam_Set_MaxSensorAnalogGain, rExpSetting.u4MaxSensorAnalogGain, 0);
    hdrProc.setParam(
            HDRProcParam_Set_MaxAEExpTimeInUS, rExpSetting.u4MaxAEExpTimeInUS, 0);
    hdrProc.setParam(
            HDRProcParam_Set_MinAEExpTimeInUS, rExpSetting.u4MinAEExpTimeInUS, 0);
    hdrProc.setParam(
            HDRProcParam_Set_ShutterLineTime, rExpSetting.u4ShutterLineTime, 0);
    hdrProc.setParam(
            HDRProcParam_Set_MaxAESensorGain, rExpSetting.u4MaxAESensorGain, 0);
    hdrProc.setParam(
            HDRProcParam_Set_MinAESensorGain, rExpSetting.u4MinAESensorGain, 0);
    hdrProc.setParam(
            HDRProcParam_Set_ExpTimeInUS0EV, rExpSetting.u4ExpTimeInUS0EV, 0);
    hdrProc.setParam(
            HDRProcParam_Set_SensorGain0EV, rExpSetting.u4SensorGain0EV, 0);
    hdrProc.setParam(
            HDRProcParam_Set_FlareOffset0EV, rExpSetting.u1FlareOffset0EV, 0);
    hdrProc.setParam(
            HDRProcParam_Set_GainBase0EV, rExpSetting.i4GainBase0EV, 0);
    hdrProc.setParam(
            HDRProcParam_Set_LE_LowAvg, rExpSetting.i4LE_LowAvg, 0);
    hdrProc.setParam(
            HDRProcParam_Set_SEDeltaEVx100, rExpSetting.i4SEDeltaEVx100, 0);
    hdrProc.setParam(
            HDRProcParam_Set_DetectFace, rExpSetting.bDetectFace, 0);
    hdrProc.setParam(
            HDRProcParam_Set_Histogram, (MUINTPTR)rExpSetting.u4Histogram, 0);
    hdrProc.setParam(
            HDRProcParam_Set_FlareHistogram, (MUINTPTR)rExpSetting.u4FlareHistogram, 0);
    {
        // get AEPlineTable
        MINT32 aeTableCurrentIndex;
        strAETable aePlineTable;
        mpHal3A->send3ACtrl(E3ACtrl_GetAECapPLineTable,
                (MINTPTR)(&aeTableCurrentIndex), (MINTPTR)(&aePlineTable));

        hdrProc.setParam(HDRProcParam_Set_PLineAETable,
                (MUINTPTR)(&aePlineTable), aeTableCurrentIndex);

        dumpCapPLineTable(aeTableCurrentIndex, aePlineTable);
    }

    // get HDR capture information from HDR proc
    Vector<MUINT32> vu4Eposuretime;
    Vector<MUINT32> vu4SensorGain;
    Vector<MUINT32> vu4FlareOffset;

    hdrProc.getHDRCapInfo(
            mHDRFrameNum, vu4Eposuretime, vu4SensorGain, vu4FlareOffset);

    HDR_LOGD("HDR input frames(%d)", mHDRFrameNum);

    // query the current 3A information
    HDR_LOGD("E3ACtrl_GetExposureParam");
    NS3Av3::CaptureParam_T tmpCap3AParam;
    mpHal3A->send3ACtrl(E3ACtrl_GetExposureParam , (MINTPTR)&tmpCap3AParam, 0);

    dumpCaptureParam(tmpCap3AParam, "Original ExposureParam");

    // update 3A information with information from HDR proc
    for (MINT32 i = 0; i < mHDRFrameNum; i++)
    {
        tmpCap3AParam.u4Eposuretime = vu4Eposuretime[i];
        tmpCap3AParam.u4AfeGain     = vu4SensorGain[i];
        tmpCap3AParam.u4IspGain     = 1024;
        if (rExpSetting.u4SensorGain0EV)
        {
            // HDR_iso = (HDR_gain / 0EV_gain) * 0EV_iso
            tmpCap3AParam.u4RealISO =
                (vu4SensorGain[i] * rExpSetting.u4ISOValue) /
                rExpSetting.u4SensorGain0EV;
        }
        tmpCap3AParam.u4FlareOffset = vu4FlareOffset[i];

        String8 str;
        str.appendFormat("Modified ExposureParam[%d]", i);
        dumpCaptureParam(tmpCap3AParam, str.string());

        vCaptureParams.push_back(tmpCap3AParam);
    }

    mpHal3A->destroyInstance(LOG_TAG);

    return OK;
}

// ---------------------------------------------------------------------------

HDRNodeImp::HDRNodeImp()
    : mRequestQueue()
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    , mNodeEnqueIndex(0)
    , mpPipe(NULL)
    , mp3A(NULL)
{
    char value[PROPERTY_VALUE_MAX];

    // log level
    property_get("vendor.debug.camera.log", value, "0");
    mLogLevel = atoi(value);
    if (mLogLevel == 0)
    {
        property_get("vendor.debug.camera.log.HDRNode", value, "0");
        mLogLevel = atoi(value);
    }

    // debug dump
    property_get("vendor.mediatek.hdr.debug", value, "0");
    mDebugDump = atoi(value);

    HDR_LOGI("log level is set to %d", mLogLevel);
    HDR_LOGI_IF(mDebugDump, "buffer dump enabled");

    // make debug dump path
    if (mDebugDump && !makePath(HDR_DUMP_PATH, 0660))
    {
        HDR_LOGW("make debug dump path %s failed", HDR_DUMP_PATH);
    }
}

HDRNodeImp::~HDRNodeImp()
{
}

MERROR HDRNodeImp::init(const InitParams& rParams)
{
    CAM_TRACE_CALL();

    RWLock::AutoWLock _l(mConfigRWLock);

    mvFrameParams.clear();
    mvHDRInputFrames.clear();

    mOpenId   = rParams.openId;
    mNodeId   = rParams.nodeId;
    mNodeName = rParams.nodeName;

    // initialize HDR proc
    HDR_LOGD("init HDR proc...");
    HDRProc2& hdrProc(HDRProc2::getInstance());

    if (hdrProc.init(mOpenId) != MTRUE)
    {
        HDR_LOGE("init HDR proc failed");
        return INVALID_OPERATION;
    }

    hdrProc.setCompleteCallback(HDRProcCompleteCallback, this);

    // create pipe before HDR processing
    if (createPipe() == MFALSE)
    {
        HDR_LOGE("create pipe failed");
        return INVALID_OPERATION;
    }

    // create worker threads
    mpHDRThread = new HDRThread(P2THREAD_NAME_ENQUE, this);
    if (OK != mpHDRThread->run(P2THREAD_NAME_ENQUE))
    {
        HDR_LOGE("create hdr thread failed");
        return INVALID_OPERATION;
    }

    mpHDRProcThread = new HDRProcThread(P2THREAD_NAME_HDR, this);
    if (OK != mpHDRProcThread->run(P2THREAD_NAME_HDR))
    {
        HDR_LOGE("create HDR proc thread failed");
        return INVALID_OPERATION;
    }

    // query active array size
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray))
    {
        HDR_LOGD("active array(%d, %d, %dx%d)",
                mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
    }
    else
    {
        HDR_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        return UNKNOWN_ERROR;
    }

    HDR_LOGD("nodeId(%08" PRIxPTR ") nodeName(%s)", getNodeId(), getNodeName());

    return OK;
}

MERROR HDRNodeImp::uninit()
{
    CAM_TRACE_CALL();

    if (OK != flush()) HDR_LOGE("flush failed");

    // TODO: check if pending requests are handled properly before exiting threads
    // destroy worker threads
    mpHDRThread->requestExit();
    mpHDRThread->join();
    mpHDRThread = NULL;

    mpHDRProcThread->requestExit();
    mpHDRProcThread->join();
    mpHDRProcThread = NULL;

    // release output frames
    for (MINT32 i = 0; i < mHDRFrameNum; i ++)
    {
        for (size_t j = 0; j < mvFrameParams[i].vpIn.size(); j++)
        {
            HDR_LOGV("delete vpIn i(%d)j(%zu)", i, j);
            delete mvFrameParams[i].vpIn[j];
        }

        for (size_t j = 0; j < mvFrameParams[i].vpOut.size(); j++)
        {
            HDR_LOGV("delete vpOut i(%d)j(%zu)", i, j);
            delete mvFrameParams[i].vpOut[j];
        }
    }

    // destroy pipe after finish HDR processing
    destroyPipe();

    mvFrameParams.clear();
    mvHDRInputFrames.clear();

    return OK;
}

MERROR HDRNodeImp::config(const ConfigParams& rParams)
{
    CAM_TRACE_CALL();

    // check ConfigParams
    MERROR const err = verifyConfigParams(rParams);
    if (err != OK)
    {
        HDR_LOGE("verifyConfigParams failed: err = %d", err);
        return err;
    }

    flush();

    // configure streams into node
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta_Request = rParams.pInAppMeta;
        mpInHalMeta_P1      = rParams.pInHalMeta;
        mpOutAppMeta_Result = rParams.pOutAppMeta;
        mpOutHalMeta_Result = rParams.pOutHalMeta; // not necessary
        // image
        mpInFullRaw         = rParams.pvInFullRaw[0];
        mpInResizedRaw      = rParams.pInResizedRaw;
        mvOutImages         = rParams.vOutImage;
    }

    return OK;
}

MERROR HDRNodeImp::flush()
{
    CAM_TRACE_CALL();

    // flush requests before exit
    {
        Mutex::Autolock _l(mRequestQueueLock);

        Que_T::iterator it = mRequestQueue.begin();
        while (it != mRequestQueue.end())
        {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
            CAM_TRACE_INT("request(hdr)", mRequestQueue.size());
        }
    }

    // wait for HDR thread
    waitForRequestDrained();

    return OK;
}

MERROR HDRNodeImp::queue(android::sp<IPipelineFrame> pFrame)
{
    CAM_TRACE_NAME("queue(HDR)");

    if (!pFrame.get())
    {
        HDR_LOGE("null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);

    // make sure the request with a smaller frame number has a higher priority
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin();)
    {
        --it;
        if (0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()))
        {
            ++it;   //insert(): insert before the current node
            break;
        }
    }

    // insert before the current frame
    mRequestQueue.insert(it, pFrame);

    CAM_TRACE_INT("request(hdr)", mRequestQueue.size());

    mRequestQueueCond.broadcast();

    return OK;
}

MVOID HDRNodeImp::onProcessFrame(android::sp<IPipelineFrame> const& pFrame)
{
    CAM_TRACE_NAME("onProcessFrame(HDR)");

    const MUINT32 FRAME_NO = pFrame->getFrameNo();
    // FRAME_INDEX is uesd to access the YUV and Y8 frames
    const MINT32 FRAME_INDEX = mNodeEnqueIndex << 1;
    FrameParams frameParams;
    MERROR err;
    // used for dump buffer
    char szResultFileName[100];
    MSize size;

    HDR_LOGD("mNodeEnqueIndex(%d) frameNo(%u) nodeId(%08" PRIxPTR ")",
            mNodeEnqueIndex, FRAME_NO, getNodeId());

    // get IOMapSet
    IPipelineFrame::InfoIOMapSet infoIOMapSet;
    if (OK != getInfoIOMapSet(pFrame, infoIOMapSet, mNodeEnqueIndex))
    {
        HDR_LOGE("queryInfoIOMap failed");
        BaseNode::flush(pFrame);
        return;
    }

    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();

    // NOTE: ensure buffers are available
    frameParams.pFrame = pFrame;

    // image
    {
        IPipelineFrame::ImageInfoIOMapSet const& mapSet =
            infoIOMapSet.mImageInfoIOMapSet;

        // traverse image info set
        for (size_t i = 0; i < mapSet.size(); i++)
        {
            HDR_LOGD("ImageInfoIOMapSet(%zu) frameNo(%u) In(%zu) Out(%zu)",
                    i, FRAME_NO, mapSet[i].vIn.size(), mapSet[i].vOut.size());

            // get input image buffer
            {
                FrameInput* in = new FrameInput;

                in->streamId = mapSet[i].vIn[0]->getStreamId();
                const char* streamName = mapSet[i].vIn[0]->getStreamName();
                HDR_LOGD("    IN  streamId(%#" PRIx64 ") name(%s)", in->streamId, streamName);

                // use IMGI port
                in->mPortId = PORT_IMGI;

                sp<IImageStreamBuffer>& inStreamBuffer(in->pStreamBuffer);
                sp<IImageBufferHeap>& inImageBufferHeap(in->pImageBufferHeap);
                sp<IImageBuffer>& inImageBuffer(in->pImageBuffer);

                inStreamBuffer    = NULL;
                inImageBufferHeap = NULL;
                inImageBuffer     = NULL;

                err = ensureImageBufferAvailable_(
                        FRAME_NO, in->streamId, rStreamBufferSet, inStreamBuffer);
                if (err != OK)
                {
                    HDR_LOGE("src buffer err = %d (%s)", err, strerror(-err));
                    if (inStreamBuffer == NULL)
                    {
                        HDR_LOGE("pStreamBuffer is NULL");
                    }
                    delete in;
                    return;
                }

                // get imagebuffer from image buffer heap, which belongs to a streambuffer
                err = getImageBuffer(IN, in->streamId,
                        inStreamBuffer, inImageBufferHeap, inImageBuffer);
                if (err != OK)
                {
                    HDR_LOGE("get imagebuffer failed");
                    delete in;
                    return;
                }

                // dump buffer if necessary
                if (mDebugDump > 1)
                {
                    size = inImageBuffer->getImgSize();

                    // single-channel raw format
                    sprintf(szResultFileName,
                            HDR_DUMP_PATH "00_SrcRawImgBuffer[%d]_%dx%d_%zu.raw",
                            mNodeEnqueIndex,
                            size.w, size.h,
                            inImageBuffer->getBufStridesInBytes(0));

                    HDR_LOGD("dump %s format(0x%x)",
                            szResultFileName, inImageBuffer->getImgFormat());

                    inImageBuffer->saveToFile(szResultFileName);
                }

                frameParams.vpIn.push_back(in);
            }

            // get output image buffers
            for (size_t j = 0; j < mapSet[i].vOut.size(); j++)
            {
                FrameOutput* out = new FrameOutput;

                out->streamId = mapSet[i].vOut[j]->getStreamId();
                const char* streamName = mapSet[i].vOut[j]->getStreamName();
                HDR_LOGD("    OUT streamId(%#" PRIx64 ") name(%s)", out->streamId, streamName);

                // use WROTO port
                out->mPortId = PORT_WROTO;
                // TODO: should base on metadata
                out->mTransform = 0;

                // get output image buffer
                sp<IImageStreamBuffer>& outStreamBuffer(out->pStreamBuffer);
                sp<IImageBufferHeap>& outImageBufferHeap(out->pImageBufferHeap);
                sp<IImageBuffer>& outImageBuffer(out->pImageBuffer);

                outStreamBuffer    = NULL;
                outImageBufferHeap = NULL;
                outImageBuffer     = NULL;

                err = ensureImageBufferAvailable_(
                        FRAME_NO, out->streamId, rStreamBufferSet, outStreamBuffer);
                if (err != OK)
                {
                    HDR_LOGE("dst buffer err = %d (%s)", err, strerror(-err));
                    if (outStreamBuffer == NULL)
                    {
                        HDR_LOGE("pStreamBuffer is NULL");
                    }
                    delete out;
                    return;
                }

                // get imagebuffer from image buffer heap, which belongs to a streambuffer
                err = getImageBuffer(OUT, out->streamId,
                        outStreamBuffer, outImageBufferHeap, outImageBuffer);
                if (err != OK)
                {
                    HDR_LOGE("get imagebuffer failed");
                    delete out;
                    return;
                }

                frameParams.vpOut.push_back(out);
            }
        } // traverse image info set
    } // image

    // metadata
    {
        IPipelineFrame::MetaInfoIOMapSet const& mapSet =
            infoIOMapSet.mMetaInfoIOMapSet;

        // traverse metadata info set
        for (size_t i = 0; i < mapSet.size(); i++)
        {
            HDR_LOGD("MetaInfoIOMapSet(%zu) frameNo(%u) In(%zu) Out(%zu)",
                    i, FRAME_NO, mapSet[i].vIn.size(), mapSet[i].vOut.size());

            // get input meta buffer
            for (size_t j = 0; j < mapSet[i].vIn.size(); j++)
            {
                StreamId_T const streamId = mapSet[i].vIn[j]->getStreamId();
                const char* streamName = mapSet[i].vIn[j]->getStreamName();
                HDR_LOGD("    IN  streamId(%#" PRIx64 ") name(%s)", streamId, streamName);

                // get in app metadata
                if (mpInAppMeta_Request->getStreamId() == streamId)
                {
                    sp<IMetaStreamBuffer>& inStreamBuffer(frameParams.pMeta_InAppStreamBuffer);

                    inStreamBuffer = NULL;

                    err = ensureMetaBufferAvailable_(
                            FRAME_NO, streamId, rStreamBufferSet, inStreamBuffer);
                    if (err != OK)
                    {
                        HDR_LOGE("src metadata err = %d (%s)", err, strerror(-err));
                        if (inStreamBuffer == NULL)
                        {
                            HDR_LOGE("pStreamBuffer is NULL");
                        }
                        // TODO: some error handling
                        return;
                    }

                    // get metadata from meta stream buffer
                    err = getMetadata(IN, streamId,
                            inStreamBuffer, frameParams.pMeta_InApp);
                    if (err != OK)
                    {
                        HDR_LOGE("get metadata failed");
                        return;
                    }
                }

                // get in hal metadata
                if (mpInHalMeta_P1->getStreamId() == streamId)
                {
                    sp<IMetaStreamBuffer>& inStreamBuffer(frameParams.pMeta_InHalStreamBuffer);

                    inStreamBuffer = NULL;

                    err = ensureMetaBufferAvailable_(
                            FRAME_NO, streamId, rStreamBufferSet, inStreamBuffer);
                    if (err != OK)
                    {
                        HDR_LOGE("src metadata err = %d (%s)", err, strerror(-err));
                        if (inStreamBuffer == NULL)
                        {
                            HDR_LOGE("pStreamBuffer is NULL");
                        }
                        // TODO: some error handling
                        return;
                    }

                    // get metadata from meta stream buffer
                    err = getMetadata(IN, streamId,
                            inStreamBuffer, frameParams.pMeta_InHal);
                    if (err != OK)
                    {
                        HDR_LOGE("get metadata failed");
                        return;
                    }
                }
            }

            // get output meta buffer
            for (size_t j = 0; j < mapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = mapSet[i].vOut[j]->getStreamId();
                const char* streamName = mapSet[i].vOut[j]->getStreamName();
                HDR_LOGD("    OUT streamId(%#" PRIx64 ") name(%s)", streamId, streamName);

                // get out app metadata
                if (mpOutAppMeta_Result->getStreamId() == streamId)
                {
                    sp<IMetaStreamBuffer>& outStreamBuffer(frameParams.pMeta_OutAppStreamBuffer);

                    outStreamBuffer = NULL;

                    err = ensureMetaBufferAvailable_(
                            FRAME_NO, streamId, rStreamBufferSet, outStreamBuffer);
                    if (err != OK)
                    {
                        HDR_LOGE("dst metadata err = %d (%s)", err, strerror(-err));
                        if (outStreamBuffer == NULL)
                        {
                            HDR_LOGE("pStreamBuffer is NULL");
                        }
                        // TODO: some error handling
                        return;
                    }

                    // get metadata from meta stream buffer
                    err = getMetadata(OUT, streamId,
                            outStreamBuffer, frameParams.pMeta_OutApp);
                    if (err != OK)
                    {
                        HDR_LOGE("get metadata failed");
                        return;
                    }
                }

                // get out hal metadata
                if (mpOutHalMeta_Result->getStreamId() == streamId)
                {
                    sp<IMetaStreamBuffer>& outStreamBuffer(frameParams.pMeta_OutHalStreamBuffer);

                    outStreamBuffer = NULL;

                    err = ensureMetaBufferAvailable_(
                            FRAME_NO, streamId, rStreamBufferSet, outStreamBuffer);
                    if (err != OK)
                    {
                        HDR_LOGE("dst metadata err = %d (%s)", err, strerror(-err));
                        if (outStreamBuffer == NULL)
                        {
                            HDR_LOGE("pStreamBuffer is NULL");
                        }
                        // TODO: some error handling
                        return;
                    }

                    // get metadata from meta stream buffer
                    err = getMetadata(OUT, streamId,
                            outStreamBuffer, frameParams.pMeta_OutHal);
                    if (err != OK)
                    {
                        HDR_LOGE("get metadata failed");
                        return;
                    }
                }
            } // get output meta buffer
        } // traverse metadata info set
    } // metadata

    if ((frameParams.pMeta_InApp == NULL) || (frameParams.pMeta_InHal == NULL))
    {
        HDR_LOGE_IF(frameParams.pMeta_InApp == NULL, "In app metadata is NULL");
        HDR_LOGE_IF(frameParams.pMeta_InHal == NULL, "In hal metadata is NULL");
        return;
    }

    frameParams.enqueIndex = mNodeEnqueIndex;

    // from now on, frameParams is ready for processing
    mvFrameParams.push_back(frameParams);

    if (mNodeEnqueIndex == 0)
    {
        // we've already collect all meta from the first enqueued frame,
        // signal HDR proc thread to prepare
        HDR_LOGD("notify HDRProcThread to work");
        mMetatDataInfoCond.signal();
    }
    //
    #if 0
    crop_infos cropInfos;
    if( OK != (ret = getCropInfos(pMeta_InApp, pMeta_InHal, params.bResized, cropInfos)) ) {
        HDR_LOGE("getCropInfos failed");
        return ret;
    }
    #endif

    if (mNodeEnqueIndex == 0)
    {
        // we already allocate all hdr src frames
        // when main-frame enqued
        CAM_TRACE_NAME("wait_HDRInput_frames");
        HDR_LOGD("wait HDR input frames...");
        mHDRInputFrameCond.wait(mHDRInputFrameLock);
        HDR_LOGD("wait HDR input frames done");
    }

    //
    QParams enqueParams;
    // input

    MINT fullRawIndex = -1;
    for (size_t i = 0; i < mvFrameParams[mNodeEnqueIndex].vpIn.size(); i++)
    {
        if (eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00 == mvFrameParams[mNodeEnqueIndex].vpIn[i]->streamId)
        {
            HDR_LOGD("found full raw at %zu", i);
            fullRawIndex = i;
            break;
        }
    }

    if (fullRawIndex == -1)
    {
        HDR_LOGE("cannnot find full raw");
        // TODO: some error handling
        return;
    }

    // NOTE: for full sized raw, ISP pass1 uses IMGO as output port
    // and the AWB gain is set via PGN in ISP pass2
    MUINT8 enablePGN = 1;
    {
        Input src;
        src.mPortID       = mvFrameParams[mNodeEnqueIndex].vpIn[fullRawIndex]->mPortId;
        src.mPortID.group = 0;
        src.mBuffer       = mvFrameParams[mNodeEnqueIndex].vpIn[fullRawIndex]->pImageBuffer.get();
        // update src size
        // pSrc->setExtParam(cropInfos.dstsize_resizer);
        //
        enqueParams.mvIn.push_back(src);
        HDR_LOGD("Input mPortID %08" PRIxPTR, reinterpret_cast<intptr_t>(&src.mPortID));
        HDR_LOGD("Input mBuffer %p", (void*)src.mBuffer);
    }

    //
    // output
    // main output i420 source.
    Output dst;
    dst.mPortID       = PORT_WROTO;
    dst.mPortID.group = 0;
    dst.mBuffer       = mvHDRInputFrames[FRAME_INDEX].get();
    enqueParams.mvOut.push_back(dst);

    HDR_LOGD("dst mPortID %08" PRIxPTR, reinterpret_cast<intptr_t>(&dst.mPortID));
    HDR_LOGD("dst mBuffer %08" PRIxPTR, reinterpret_cast<intptr_t>(&dst.mBuffer));
    HDR_LOGD("dst mBuffer VA %08" PRIxPTR, dst.mBuffer != NULL ?
            dst.mBuffer->getBufVA(0) : NULL);

    // main output y800 source.
    Output dst2;
    dst2.mPortID       = PORT_WDMAO;
    dst2.mPortID.group = 0;
    dst2.mBuffer       = mvHDRInputFrames[(FRAME_INDEX + 1)].get();
    enqueParams.mvOut.push_back(dst2);
    HDR_LOGD("dst2 mPortID %08" PRIxPTR, reinterpret_cast<intptr_t>(&dst2.mPortID));
    HDR_LOGD("dst2 mBuffer %08" PRIxPTR, reinterpret_cast<intptr_t>(&dst2.mBuffer));
    HDR_LOGD("dst2 mBuffer VA %08" PRIxPTR, dst2.mBuffer != NULL ?
            dst2.mBuffer->getBufVA(0) : NULL);

    if( enqueParams.mvOut.size() == 0 )
    {
        HDR_LOGW("no dst buffer");
        return;
    }

    {
        void* pTuning = NULL;
        unsigned int tuningsize;
        if( !mpPipe->deTuningQue(tuningsize, pTuning) ) {
            HDR_LOGW("cannot get tunning buffer");
            return ;
        }
        //
        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;
        //
        inMetaSet.appMeta = *(mvFrameParams[0].pMeta_InApp);
        inMetaSet.halMeta = *(mvFrameParams[0].pMeta_InHal);
        //
        MBOOL const bGetResult = (mvFrameParams[0].pMeta_OutApp || mvFrameParams[0].pMeta_OutHal);

        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, enablePGN);

        if( mvFrameParams[0].pMeta_OutHal ) {
            HDR_LOGD("outMetaSet.halMeta assign by *(mvFrameParams[0].pMeta_InHal) ");
            outMetaSet.halMeta = *(mvFrameParams[0].pMeta_InHal);
        }
        //
        mp3A->setIsp(0, inMetaSet, pTuning, bGetResult ? &outMetaSet : NULL);
        //
        if( mvFrameParams[0].pMeta_OutApp ) {
            HDR_LOGD("*(mvFrameParams[0].pMeta_OutApp) assign by outMetaSet.appMeta ");
            *(mvFrameParams[0].pMeta_OutApp) = outMetaSet.appMeta;
            #if 0
            // TODO: Need update Crop region?
            //
            MRect cropRegion = cropInfos.crop_a;
            if( cropInfos.isEisEabled ) {
                cropRegion.p.x += cropInfos.eis_mv_a.p.x;
                cropRegion.p.y += cropInfos.eis_mv_a.p.y;
            }
            //
            updateCropRegion(cropRegion, pMeta_OutApp);
            #endif
        }
        //
        if( mvFrameParams[0].pMeta_OutHal ) {
            HDR_LOGD("*mvFrameParams[0].pMeta_OutHal assign by outMetaSet.halMeta");
            *mvFrameParams[0].pMeta_OutHal = outMetaSet.halMeta;
        }
        //
        enqueParams.mvTuningData.push_back(pTuning);
    }

    // for output group crop
    {
        const MRect srcRect(MPoint(0, 0), enqueParams.mvIn[0].mBuffer->getImgSize());

        Vector<Output>::const_iterator it = enqueParams.mvOut.begin();
        Vector<Output>::const_iterator end = enqueParams.mvOut.end();
        while (it != end)
        {
            const MRect dstRect(MPoint(0, 0), it->mBuffer->getImgSize());

            HDR_LOGD("src rect(%d, %d, %dx%d) -> dst rect(%d, %d, %dx%d)",
                    srcRect.p.x, srcRect.p.y, srcRect.s.w, srcRect.s.h,
                    dstRect.p.x, dstRect.p.y, dstRect.s.w, dstRect.s.h);

            MRect cropDstRect = calCrop(srcRect, dstRect, 100);

            HDR_LOGD("dst crop rect(%d, %d, %dx%d)",
                    cropDstRect.p.x, cropDstRect.p.y,
                    cropDstRect.s.w, cropDstRect.s.h);

            MCrpRsInfo cropInfo;

            if (it->mPortID == PORT_WDMAO)
            {
                cropInfo.mGroupID = 2;
            }
            else if (it->mPortID == PORT_WROTO)
            {
                cropInfo.mGroupID = 3;
            }
            else
            {
                HDR_LOGE("not supported output port(%08" PRIxPTR ")",
                        reinterpret_cast<intptr_t>(&it->mPortID));
                return;
            }

            cropInfo.mCropRect.p_integral.x = cropDstRect.p.x;
            cropInfo.mCropRect.p_integral.y = cropDstRect.p.y;
            cropInfo.mCropRect.p_fractional.x = 0;
            cropInfo.mCropRect.p_fractional.y = 0;
            cropInfo.mCropRect.s.w = cropDstRect.s.w;
            cropInfo.mCropRect.s.h = cropDstRect.s.h;

            enqueParams.mvCropRsInfo.push_back(cropInfo);

            it++;
        }
    }

    // callback
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie     = this;
    //
    enqueParams.mvPrivaData.push_back(NULL);

    #if 1
    // get current p1 buffer crop status
    MRect crop_p1_sensor;
    MSize dstsize_resizer;
    MRect crop_dma;
    MSize sensor_size;
    if(
        !( tryGetMetadata<MRect>(mvFrameParams[0].pMeta_InHal, MTK_P1NODE_SCALAR_CROP_REGION, crop_p1_sensor) &&
           tryGetMetadata<MSize>(mvFrameParams[0].pMeta_InHal, MTK_P1NODE_RESIZER_SIZE      , dstsize_resizer) &&
           tryGetMetadata<MRect>(mvFrameParams[0].pMeta_InHal, MTK_P1NODE_DMA_CROP_REGION   , crop_dma)
         )
      ) {
        HDR_LOGW("cannot find tag, use it as full-size");
        if( ! tryGetMetadata<MSize>(mvFrameParams[0].pMeta_InHal, MTK_HAL_REQUEST_SENSOR_SIZE, sensor_size) ) {
            HDR_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
            return ;
        }
        //
        crop_p1_sensor  = MRect( MPoint(0,0), sensor_size );
        dstsize_resizer = sensor_size;
        crop_dma        = MRect( MPoint(0,0), sensor_size );
    }
    HDR_LOGD("cropInfos.crop_p1_sensor (%dx%d)",crop_p1_sensor.s.w,crop_p1_sensor.s.h);
    HDR_LOGD("cropInfos.crop_p1_sensor (%dx%d)",dstsize_resizer.w,dstsize_resizer.h);
    HDR_LOGD("cropInfos.crop_p1_sensor (%dx%d)",crop_dma.s.w,crop_dma.s.h);
    // for crop
    enqueParams.mvP1SrcCrop.push_back(crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(crop_dma);
    #endif

    //  queue a request into the pipe
    {
        CAM_TRACE_NAME("enque_P2");
        if (!mpPipe->enque(enqueParams))
        {
            HDR_LOGE("enque pass2 failed");
            mNodeEnqueIndex += 1;
            return;
        }
    }

    // wait for P2 to finish raw-to-yuv process
    {
        CAM_TRACE_NAME("wait_P2_done");
        HDR_LOGD("waiting for P2...");
        mP2Cond.wait(mP2Lock);
        HDR_LOGD("wait P2 done");
    }

    // add input/output frames into HDR proc
    {
        HDRProc2& hdrProc(HDRProc2::getInstance());

        // add input frames (YUV and Y8)
        for (MUINT32 i = 0; i < 2; i++)
        {
            const MINT32 INDEX = FRAME_INDEX + i;
            hdrProc.addInputFrame(INDEX, mvHDRInputFrames[INDEX]);
            HDR_LOGD("add input frame(%d) buffer(%p)",
                    INDEX, mvHDRInputFrames[INDEX].get());

            // dump input working buffer
            if (mDebugDump)
            {
                size = mvHDRInputFrames[INDEX]->getImgSize();
                ::sprintf(szResultFileName,
                        HDR_DUMP_PATH "00_Src%sImgBuffer[%d]_%dx%d.%s",
                        ((i & 0x1) == 0) ? "Main" : "Small",
                        mNodeEnqueIndex, size.w, size.h,
                        ((i & 0x1) == 0) ? "i420" : "y");
                mvHDRInputFrames[INDEX]->saveToFile(szResultFileName);
            }
        }

        // output only exist in the main frame (1st queued frame)
        if (mNodeEnqueIndex == 0)
        {
            const FrameParams& frameParams(mvFrameParams[mNodeEnqueIndex]);
            for (size_t i = 0; i < frameParams.vpOut.size(); i++)
            {
                HDROutputType type;

                // only support yuv frames of jpeg and thumbnail
                switch (frameParams.vpOut[i]->streamId)
                {
                   case eSTREAMID_IMAGE_PIPE_YUV_JPEG_00:
                    type = HDR_OUTPUT_JPEG_YUV;
                    break;
                    case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00:
                    type = HDR_OUTPUT_JPEG_THUMBNAIL_YUV;
                    break;
                    default:
                    HDR_LOGW("unsupported streamId(%#" PRIx64 ")",
                            frameParams.vpOut[i]->streamId);
                    continue;
                }

                HDR_LOGD("add output frame(%zu) buffer(%p)",
                        i, frameParams.vpOut[i]->pImageBuffer.get());

                hdrProc.addOutputFrame(type, frameParams.vpOut[i]->pImageBuffer);
            }
        }
    }

    // we've gotten all input and output frames at this round
    // wait unlock buffers after finishing HDR process
    if ((mNodeEnqueIndex + 1) == mHDRFrameNum)
    {
        {
            CAM_TRACE_NAME("waitHDRProc");
            HDR_LOGD("waiting for HDR post-processing...");
            mHDRProcCompleteCond2.wait(mHDRProcCompleteLock2);
            HDR_LOGD("finish HDR post-processing");
        }

        if (mDebugDump)
        {
            // output only exist in the main frame (1st queued frame)
            const MINT OUTPUT_INDEX = 0;
            const Vector<FrameOutput*>& OUTPUT_FRAMES = mvFrameParams[OUTPUT_INDEX].vpOut;
            for (size_t i = 0; i < OUTPUT_FRAMES.size(); i++)
            {
                const sp<IImageBuffer>& IMAGE_BUFFER = OUTPUT_FRAMES[i]->pImageBuffer;
                MINT FORMAT = IMAGE_BUFFER->getImgFormat();
                size = IMAGE_BUFFER->getImgSize();
                HDR_LOGD("dump output frame(%zu) format(0x%x)", i, FORMAT);

                ::sprintf(szResultFileName,
                        HDR_DUMP_PATH "00_DstImgBuffer[%zu]_%dx%d.%s",
                        i, size.w, size.h, getFileFormatName(FORMAT));

                IMAGE_BUFFER->saveToFile(szResultFileName);
            }
        }

        // unlock and image/meta buffer, mark status and then apply buffer to be released
        for (MINT32 i = 0; i < mHDRFrameNum; i++)
        {
            const FrameParams& frameParams(mvFrameParams[i]);

            for (size_t j = 0; j < frameParams.vpIn.size(); j++)
            {
                HDR_LOGD("release input image buffer i(%d)j(%zu)", i, j);

                FrameInput* frameInput(frameParams.vpIn[j]);

                unlockImage(frameInput->pStreamBuffer, frameInput->pImageBuffer);
                markImageStream(frameParams.pFrame, frameInput->pStreamBuffer);
            }

            for (size_t j = 0; j < frameParams.vpOut.size(); j++)
            {
                HDR_LOGD("release output image buffer i(%d)j(%zu)", i, j);

                FrameOutput* frameOutput(frameParams.vpOut[j]);

                unlockImage(frameOutput->pStreamBuffer, frameOutput->pImageBuffer);
                markImageStream(frameParams.pFrame, frameOutput->pStreamBuffer);
            }

            if (frameParams.pMeta_InApp != NULL)
            {
                HDR_LOGD("release pMeta_InAppStreamBuffer");
                unlockMetadata(frameParams.pMeta_InAppStreamBuffer, frameParams.pMeta_InApp);
                markMetaStream(frameParams.pFrame, frameParams.pMeta_InAppStreamBuffer);
            }

            if (frameParams.pMeta_InHal != NULL)
            {
                HDR_LOGD("release pMeta_InHalStreamBuffer");
                unlockMetadata(frameParams.pMeta_InHalStreamBuffer, frameParams.pMeta_InHal);
                markMetaStream(frameParams.pFrame, frameParams.pMeta_InHalStreamBuffer);
            }

            if (frameParams.pMeta_OutApp != NULL)
            {
                HDR_LOGD("release pMeta_OutAppStreamBuffer");
                unlockMetadata(frameParams.pMeta_OutAppStreamBuffer, frameParams.pMeta_OutApp);
                markMetaStream(frameParams.pFrame, frameParams.pMeta_OutAppStreamBuffer);
            }

            if (frameParams.pMeta_OutHal != NULL)
            {
                HDR_LOGD("release pMeta_OutHalStreamBuffer");
                unlockMetadata(frameParams.pMeta_OutHalStreamBuffer, frameParams.pMeta_OutHal);
                markMetaStream(frameParams.pFrame, frameParams.pMeta_OutHalStreamBuffer);
            }

            // apply buffers to be released
            {
                CAM_TRACE_NAME("applyRelease");
                HDR_LOGD("[applyRelease] frameNo(%u) nodeId(%08" PRIxPTR ")", FRAME_NO, getNodeId());

                // after this call, all of RELEASE-marked buffers are released by this user
                frameParams.pFrame->getStreamBufferSet().applyRelease(getNodeId());
            }
        }

        // dispatch the main frame (i.e. index is 0) to the next node
        onDispatchFrame(mvFrameParams[0].pFrame);
    }

    // let's go to the next round
    mNodeEnqueIndex += 1;
}

MERROR HDRNodeImp::verifyConfigParams(ConfigParams const & rParams) const
{
    if (!rParams.pInAppMeta.get())
    {
        HDR_LOGE("out in app metadata");
        return BAD_VALUE;
    }
    if (!rParams.pInHalMeta.get())
    {
        HDR_LOGE("no in hal metadata");
        return BAD_VALUE;
    }

    // TODO: need to check out metadata?
    //if  ( ! rParams.pOutAppMeta.get() ) {
    //    return BAD_VALUE;
    //}
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}

    if (rParams.pvInFullRaw.size() == 0)
    {
        HDR_LOGE("no in image fullraw");
        return BAD_VALUE;
    }

    // resized raw can be null; just log for notification
    if (!rParams.pInResizedRaw.get())
    {
        HDR_LOGD("no in resized raw");
    }

    if (rParams.vOutImage.size() == 0)
    {
        HDR_LOGE("no out yuv image");
        return BAD_VALUE;
    }

    // dump all streams
    size_t count;

    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);

    count = rParams.pvInFullRaw.size();
    for (size_t i = 0; i < count; i++)
    {
        dumpStreamIfExist("[image] in full", rParams.pvInFullRaw[i]);
    }

    dumpStreamIfExist("[image] in resized", rParams.pInResizedRaw);

    count = rParams.vOutImage.size();
    for (size_t i = 0; i < count; i++)
    {
        dumpStreamIfExist("[image] out yuv", rParams.vOutImage[i]);
    }

    return OK;
}

MERROR HDRNodeImp::getInfoIOMapSet(
        sp<IPipelineFrame> const& pFrame,
        IPipelineFrame::InfoIOMapSet& rIOMapSet,
        MINT32 /*frameIndex*/) const
{
    // get PipelineFrame's InfoIOMapSet
    if (OK != pFrame->queryInfoIOMapSet(getNodeId(), rIOMapSet))
    {
        HDR_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    const MUINT32 FRAME_NO = pFrame->getFrameNo();

    // check image part
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if (imageIOMapSet.size() == 0)
    {
        HDR_LOGW("no imageIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < imageIOMapSet.size(); i++)
    {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[i];

        // return if in-degree is not equal to 1
        if (imageIOMap.vIn.size() != 1)
        {
            HDR_LOGE("[image] #%zu: wrong size vIn %zu", i, imageIOMap.vIn.size());
            return BAD_VALUE;
        }

        HDR_LOGD_IF(mLogLevel >= 1, "ImageInfoIOMapSet(%zu) frameNo(%u) In(%zu) Out(%zu)",
                i, FRAME_NO, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }

    // check metadata part
    IPipelineFrame::MetaInfoIOMapSet& metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if (metaIOMapSet.size() == 0)
    {
        HDR_LOGW("no metaIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < metaIOMapSet.size(); i++)
    {
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = metaIOMapSet[i];

        // return if app's meta request is empty or cannot be found in the current MetaInfoIOMap
        if (!mpInAppMeta_Request.get() ||
                metaIOMap.vIn.indexOfKey(mpInAppMeta_Request->getStreamId()) < 0)
        {
            HDR_LOGE("[meta] #%zu: app's meta request is empty or cannot be found", i);
            return BAD_VALUE;
        }

        // return if P1's meta request is empty or cannot be found in the current MetaInfoIOMap
        if (!mpInHalMeta_P1.get() ||
                metaIOMap.vIn.indexOfKey(mpInHalMeta_P1->getStreamId()) < 0)
        {
            HDR_LOGE("[meta] #%zu: P1's meta request is empty or cannot be found", i);
            return BAD_VALUE;
        }

        HDR_LOGD_IF(mLogLevel >= 1, "MetaInfoIOMapSet(%zu) frameNo(%u) In(%zu) Out(%zu)",
                i, FRAME_NO, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }

    return OK;
}

MBOOL HDRNodeImp::getScalerCropRegion(MRect& cropRegion, MSize& sensorSize) const
{
    MRect origCropRegion;

    // query crop region (pixel coordinate is relative to active array)
    IMetadata* dynamic_app_meta = mvFrameParams[0].pMeta_InApp;
    if (!tryGetMetadata<MRect>(dynamic_app_meta, MTK_SCALER_CROP_REGION, origCropRegion))
    {
        // set crop region to full size
        origCropRegion.p = MPoint(0, 0);
        origCropRegion.s = mActiveArray.s;
        HDR_LOGW("no MTK_SCALER_CROP_REGION, set crop region to full size %dx%d",
                origCropRegion.s.w, origCropRegion.s.h);
    }

    // setup transform (active arrary -> sensor)
    simpleTransform tranActive2Sensor =
        simpleTransform(MPoint(0, 0), mActiveArray.s, sensorSize);

    // apply transform
    cropRegion = transform(tranActive2Sensor, origCropRegion);

    HDR_LOGD("cropRegion(%d, %d, %dx%d)",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    return MTRUE;
}

inline MBOOL HDRNodeImp::isStream(const sp<IStreamInfo>& streamInfo,
        StreamId_T streamId) const
{
    return streamInfo.get() && (streamInfo->getStreamId() == streamId);
}

inline MBOOL HDRNodeImp::isFullRawLocked(StreamId_T const streamId) const
{
    return isStream(mpInFullRaw, streamId);
}

inline MBOOL HDRNodeImp::isResizedRawLocked(StreamId_T const streamId) const
{
    return isStream(mpInResizedRaw, streamId);
}

MBOOL HDRNodeImp::isInImageStream(StreamId_T const streamId) const
{
    RWLock::AutoRLock _l(mConfigRWLock);

    if (isFullRawLocked(streamId) || isResizedRawLocked(streamId))
        return MTRUE;

    HDR_LOGD_IF(0, "streamId(%#" PRIx64 ") is not in-stream", streamId);
    return MFALSE;
}

MBOOL HDRNodeImp::isInMetaStream(StreamId_T const streamId) const
{
    RWLock::AutoRLock _l(mConfigRWLock);

    return isStream(mpInAppMeta_Request, streamId) || isStream(mpInHalMeta_P1, streamId);
}

MERROR HDRNodeImp::onDequeRequest(android::sp<IPipelineFrame>& rpFrame)
{
    CAM_TRACE_NAME("onDequeueRequest(HDR)");

    Mutex::Autolock _l(mRequestQueueLock);
    //  Wait until the queue is not empty or this thread will exit
    while (mRequestQueue.empty() && !mbRequestExit)
    {
        // enable drained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();

        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if (OK != status)
        {
            HDR_LOGW("wait status(%d):%s, mRequestQueue.size(%zu)",
                    status, ::strerror(-status), mRequestQueue.size());
        }
    }

    // warn if request queue is not empty
    if (mbRequestExit)
    {
        HDR_LOGE_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size(%zu)",
                mRequestQueue.size());
        return DEAD_OBJECT;
    }

    // request queue is not empty, take the first request from the queue
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());

    CAM_TRACE_INT("request(hdr)", mRequestQueue.size());

    return OK;
}

MVOID HDRNodeImp::waitForRequestDrained() const
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mRequestQueueLock);
    if (!mbRequestDrained)
    {
        HDR_LOGD("wait for request drained...");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
}

MBOOL HDRNodeImp::HDRProcCompleteCallback(MVOID* user, MBOOL ret)
{
    HDRNodeImp* self = reinterpret_cast<HDRNodeImp*>(user);
    if (NULL == self ){
        HDR_LOGE("HDRProcCompleteCallback with NULL user");
        return MFALSE;
    }
    HDR_LOGD("HDRProcCompleteCallback ret:%d",ret);

    HDR_LOGD("signal mHDRProcCompleteLock");
    self->mHDRProcCompleteCond.signal();
    HDR_LOGD("signal mHDRProcCompleteLock2");
    self->mHDRProcCompleteCond2.signal();

    return MTRUE;
}

MVOID HDRNodeImp::pass2CbFunc(QParams& rParams)
{
    HDRNodeImp* self = reinterpret_cast<HDRNodeImp*>(rParams.mpCookie);
    self->handleDeque(rParams);
}

MVOID HDRNodeImp::handleDeque(QParams& rParams)
{
    CAM_TRACE_NAME("p2 deque");

    if (rParams.mvTuningData.size() > 0)
    {
        void* pTuning = rParams.mvTuningData[0];
        if (pTuning)
            mpPipe->enTuningQue(pTuning);
    }

    HDR_LOGD("signal P2");
    mP2Cond.signal();
}

MBOOL HDRNodeImp::createPipe()
{
    MBOOL ret = MTRUE;
    ENormalStreamTag streamTag = ENormalStreamTag_Cap;

    mpPipe = INormalStream::createInstance(
            LOG_TAG, streamTag, mOpenId, true);
    if (mpPipe == NULL)
    {
        HDR_LOGE("create normal stream failed");
        ret = MFALSE;
        goto lbExit;
    }

    if (!mpPipe->init())
    {
        HDR_LOGE("initialize normal stream failed");
        ret = MFALSE;
        goto lbExit;
    }

    mp3A = IHal3A::createInstance(
            IHal3A::E_Camera_3, mOpenId, LOG_TAG);
    if (mp3A == NULL)
    {
        HDR_LOGE("create 3A HAL failed");
        ret = MFALSE;
        goto lbExit;
    }

    HDR_LOGD("pipe created: pipe(%p) 3AHal(%p)", mpPipe, mp3A);

lbExit:
    // fall back if cannot create pipe
    if ((mpPipe == NULL) || (mp3A == NULL))
    {
        destroyPipe();
    }

    return ret;
}

MVOID HDRNodeImp::destroyPipe()
{
    if (mpPipe)
    {
        // uninit pipe
        if (!mpPipe->uninit())
        {
            HDR_LOGE("pipe uninit failed");
        }

        mpPipe->destroyInstance(LOG_TAG);
        mpPipe = NULL;
    }

    if (mp3A)
    {
        mp3A->destroyInstance(LOG_TAG);
        mp3A = NULL;
    }

    HDR_LOGD("pipe destroyed");
}

MERROR HDRNodeImp::getImageBuffer(
    MINT32 type,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& streamBuffer,
    sp<IImageBufferHeap>& imageBufferHeap,
    sp<IImageBuffer>& imageBuffer)
{
    // query the group usage from UsersManager
    MUINT const groupUsage =
        streamBuffer->queryGroupUsage(getNodeId());

    // get image buffer heap from streambuffer (with read or write lock locked)
    switch (type & (IN | OUT))
    {
        case IN:
            imageBufferHeap = streamBuffer->tryReadLock(getNodeName());
            break;
        case OUT:
            imageBufferHeap = streamBuffer->tryWriteLock(getNodeName());
            break;
        default:
            HDR_LOGE("    invalid buffer type(%d)", type);
    }

    if (imageBufferHeap == NULL)
    {
        HDR_LOGE("    node(%" PRIdPTR ") type(%d) stream buffer(%s): cannot get imageBufferHeap",
                getNodeId(), type, streamBuffer->getName());
        return INVALID_OPERATION;
    }

    // create imagebuffer from image buffer heap
    imageBuffer = imageBufferHeap->createImageBuffer();
    if (imageBuffer == NULL)
    {
        HDR_LOGE("    node(%" PRIdPTR ") type(%d) stream buffer(%s): cannot create imageBuffer",
                getNodeId(), type, streamBuffer->getName());
        return INVALID_OPERATION;
    }

    // a buffer is allowed to access only between the interval of
    // lockBuf() and unlockBuf()
    imageBuffer->lockBuf(getNodeName(), groupUsage);
    HDR_LOGD_IF(mLogLevel >= 1, "    streamId(%#" PRIx64 ") buffer(%p) usage(0x%x) type(0x%x)",
            streamId, imageBuffer.get(), groupUsage, type);

    return OK;
}

MERROR HDRNodeImp::getMetadata(
    MINT32 type,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& streamBuffer,
    IMetadata*& metadata)
{
    // get metadata from meta stream buffer
    switch (type & (IN | OUT))
    {
        case IN:
            metadata = streamBuffer->tryReadLock(getNodeName());
            break;
        case OUT:
            metadata = streamBuffer->tryWriteLock(getNodeName());
    }

    if (metadata == NULL)
    {
        HDR_LOGE("      node(%" PRIdPTR ") stream buffer(%s): cannot get app metadata",
                getNodeId(), streamBuffer->getName());
        return INVALID_OPERATION;
    }

    HDR_LOGD_IF(mLogLevel >= 1, "      streamId(%#" PRIx64 ") metadata(%p) type(0x%x)",
            streamId, metadata, type);

    return OK;
}

MVOID HDRNodeImp::markImageStream(
        sp<IPipelineFrame> const& pFrame,
        sp<IImageStreamBuffer> const pStreamBuffer) const
{
    if ((pFrame == NULL) || (pStreamBuffer == NULL))
    {
        HDR_LOGE("pFrame(%08" PRIxPTR ") or pStreamBuffer(%08" PRIxPTR ") should not be NULL",
                reinterpret_cast<intptr_t>(pFrame.get()),
                reinterpret_cast<intptr_t>(pStreamBuffer.get()));
        return;
    }

    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();

    // buffer Producer must set this status
    if (isInImageStream(streamId))
    {
        pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
    }

    // mark this buffer as USED by this user
    // mark this buffer as RELEASE by this user
    streamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE);
}

MVOID HDRNodeImp::markMetaStream(
        android::sp<IPipelineFrame> const& pFrame,
        sp<IMetaStreamBuffer> const pStreamBuffer) const
{
    if ((pFrame == NULL) || (pStreamBuffer == NULL))
    {
        HDR_LOGE("pFrame(%08" PRIxPTR ") or pStreamBuffer(%08" PRIxPTR ") should not be NULL",
                reinterpret_cast<intptr_t>(pFrame.get()),
                reinterpret_cast<intptr_t>(pStreamBuffer.get()));
        return;
    }

    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();


    // buffer Producer must set this status
    if (!isInMetaStream(streamId))
    {
        pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
    }

    // mark this buffer as USED by this user
    // mark this buffer as RELEASE by this user
    rStreamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE);
}

MVOID HDRNodeImp::unlockImage(
        sp<IImageStreamBuffer> const pStreamBuffer,
        sp<IImageBuffer> const pImageBuffer) const
{
    if (pStreamBuffer == NULL || pImageBuffer == NULL)
    {
        HDR_LOGE("pStreamBuffer(%p) or pImageBuffer(%p) should not be NULL",
               pStreamBuffer.get(), pImageBuffer.get());
        return;
    }

    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());
}

MVOID HDRNodeImp::unlockMetadata(
        sp<IMetaStreamBuffer> const pStreamBuffer,
        IMetadata* const pMetadata) const
{
    if (pStreamBuffer == NULL || pMetadata == NULL)
    {
        HDR_LOGE("pStreamBuffer(%p) or pMetadata(%p) should not be NULL",
                pStreamBuffer.get(), pMetadata);
        return;
    }

    pStreamBuffer->unlock(getNodeName(), pMetadata);
}

MRect HDRNodeImp::calCrop(MRect const &rSrc, MRect const &rDst, uint32_t ratio)
{
    #define ROUND_TO_2X(x) ((x) & (~0x1))

    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w * 100 / ratio);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h * 100 / ratio);

    rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
    rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;

    #undef ROUND_TO_2X
    return rCrop;
}

// ---------------------------------------------------------------------------

HDRNodeImp::ThreadBase::ThreadBase(const char* name, const sp<HDRNodeImp>& pNodeImp)
    : mpNodeImp(pNodeImp)
{
    snprintf(mThreadName, sizeof(mThreadName), "%s", name);
}

MERROR HDRNodeImp::ThreadBase::threadSetting(const char* threadName)
{
    //  set thread policy & priority
    //  NOTE:
    //  Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //  may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //  And thus, we must set the expected policy & priority after a thread creation.

    // retrieve the parameters of the calling process
    struct sched_param schedParam;
    sched_getparam(0, &schedParam);

    if (setpriority(PRIO_PROCESS, 0, P2THREAD_PRIORITY))
    {
        HDR_LOGW("set priority failed(%s)", strerror(errno));
    }

    schedParam.sched_priority = 0;
    sched_setscheduler(0, P2THREAD_POLICY, &schedParam);

    HDR_LOGD("thread(%s) tid(%d) policy(%d) priority(%d)",
            threadName, gettid(), P2THREAD_POLICY, P2THREAD_PRIORITY);

    return OK;
}

// ---------------------------------------------------------------------------

HDRNodeImp::HDRThread::HDRThread(const char* name, const sp<HDRNodeImp>& pNodeImp)
    : ThreadBase(name, pNodeImp)
{
}

status_t HDRNodeImp::HDRThread::readyToRun()
{
    return threadSetting(mThreadName);
}

void HDRNodeImp::HDRThread::requestExit()
{
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}

bool HDRNodeImp::HDRThread::threadLoop()
{
    CAM_TRACE_NAME("threadLoop(HDR)");

    sp<IPipelineFrame> pFrame;
    if (!exitPending() &&
        (OK == mpNodeImp->onDequeRequest(pFrame)) &&
        (pFrame != 0))
    {
        HDR_LOGD("onProcessFrame %d", mpNodeImp->mNodeEnqueIndex);
        mpNodeImp->onProcessFrame(pFrame);

        return true;
    }

    HDR_LOGD("exit hdr thread");
    return false;
}

// ---------------------------------------------------------------------------

HDRNodeImp::HDRProcThread::HDRProcThread(const char* name, const sp<HDRNodeImp>& pNodeImp)
    : ThreadBase(name, pNodeImp)
{
}

status_t HDRNodeImp::HDRProcThread::readyToRun()
{
    return threadSetting(mThreadName);
}

bool HDRNodeImp::HDRProcThread::threadLoop()
{
    CAM_TRACE_NAME("threadLoop(HDRProc)");

    MSize size_sensor;

    // wait for P1 to enqueue the first frame with the destination metadata
    {
        CAM_TRACE_NAME("wait_P1_enqueue");
        HDR_LOGD("waiting for P1 enqueue...");
        mpNodeImp->mMetatDataInfoCond.wait(mpNodeImp->mMetatDataInfoLock);

        // we've already collect all meta from the first enqueued frame
        HDR_LOGD("wait P1 done");
    }

    // get app/hal metadata
    IMetadata* dynamic_app_meta = mpNodeImp->mvFrameParams[0].pMeta_InApp;
    IMetadata* dynamic_hal_meta = mpNodeImp->mvFrameParams[0].pMeta_InHal;

    // sensor size
    tryGetMetadata<MSize>(dynamic_hal_meta, MTK_HAL_REQUEST_SENSOR_SIZE, size_sensor);
    HDR_LOGD("MTK_HAL_REQUEST_SENSOR_SIZE(%dx%d)", size_sensor.w, size_sensor.h);

    HDRProc2& hdrProc(HDRProc2::getInstance());

    // scaler crop region
    MRect cropRegion;
    mpNodeImp->getScalerCropRegion(cropRegion, size_sensor);

    // FIXME: set the corresponding postview size
    MSize postviewSize(800, 600);
    hdrProc.setShotParam(size_sensor, postviewSize, cropRegion);

    hdrProc.setParam(HDRProcParam_Set_sensor_size, size_sensor.w, size_sensor.w);

    // currently HAL 3 does not support YUV sensor, so we force RAW as the sensor type
    hdrProc.setParam(HDRProcParam_Set_sensor_type, SENSOR_TYPE_RAW, 0);

    hdrProc.prepare();

    MUINT32 uSrcMainFormat = 0;
    MUINT32 uSrcMainWidth = 0;
    MUINT32 uSrcMainHeight = 0;
    MUINT32 uSrcSmallFormat = 0;
    MUINT32 uSrcSmallWidth = 0;
    MUINT32 uSrcSmallHeight = 0;
    MUINT32 empty = 0;

    hdrProc.getParam(HDRProcParam_Get_src_main_format, uSrcMainFormat, empty);
    hdrProc.getParam(HDRProcParam_Get_src_main_size, uSrcMainWidth, uSrcMainHeight);
    hdrProc.getParam(HDRProcParam_Get_src_small_format, uSrcSmallFormat, empty);
    hdrProc.getParam(HDRProcParam_Get_src_small_size, uSrcSmallWidth, uSrcSmallHeight);

    // each HDR input frame requires one main YUV and one small Y8 frame
    const MINT32 FRAME_NUM = mHDRFrameNum << 1;

    for (MINT32 i = 0; i < FRAME_NUM; i++)
    {
        EImageFormat inputImageFormat;
        MUINT32 inputImageWidth;
        MUINT32 inputImageHeight;

        // set buffer's format and dimension
        // please note that the index ordering should be
        // {0, 2, 4, ...} for main YUV and {1, 3, 5, ...} for small Y8 frames
        if ((i & 0x1) == 0)
        {
            // main YUV
            inputImageFormat = (EImageFormat)uSrcMainFormat;
            inputImageWidth  = uSrcMainWidth;
            inputImageHeight = uSrcMainHeight;
        }
        else
        {
            // small Y8
            inputImageFormat = (EImageFormat)uSrcSmallFormat;
            inputImageWidth  = uSrcSmallWidth;
            inputImageHeight = uSrcSmallHeight;
        }

        HDR_LOGD("input working buffer(%d) format(0x%x) size(%dX%d)",
                i, inputImageFormat, inputImageWidth, inputImageHeight);

        // allocate working buffer
        sp<IImageBuffer> imageBuffer;
        ImageBufferUtils::getInstance().allocBuffer(
                imageBuffer,
                inputImageWidth, inputImageHeight, inputImageFormat);
        if (imageBuffer == NULL)
        {
            HDR_LOGE("image buffer is NULL");
            return false;
        }

        mpNodeImp->mvHDRInputFrames.push_back(imageBuffer);

        HDR_LOGD("alloc input working buffer(%d:%p) size(%dx%d) format(0x%x)",
                i, imageBuffer.get(),
                inputImageWidth, inputImageHeight, inputImageFormat);
    }

    // HDR proc start wait src yuv and dst yuv frame input by hdrnode
    {
        CAM_TRACE_NAME("start_HDRProc");
        HDR_LOGD("start HDR proc");
        hdrProc.start();
        mpNodeImp->mHDRInputFrameCond.signal();
    }

    {
        CAM_TRACE_NAME("wait_HDRProc_complete");
        HDR_LOGD("waiting for HDR proc to complete...");
        mpNodeImp->mHDRProcCompleteCond.wait(mpNodeImp->mHDRProcCompleteLock);
        HDR_LOGD("wait HDR process done");
    }

    for (MINT32 i = 0; i < FRAME_NUM; i++)
    {
        sp<IImageBuffer> imageBuffer(mpNodeImp->mvHDRInputFrames[i]);
        ImageBufferUtils::getInstance().deallocBuffer(imageBuffer);
        imageBuffer.clear();
        HDR_LOGD("dealloc input working buffer(%d)", i);
    }

    hdrProc.release();

    hdrProc.uninit();

    HDR_LOGD("HDR process done");

    return  false;
}
