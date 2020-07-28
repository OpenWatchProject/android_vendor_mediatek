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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_DEVICE_CAMERADEVICE1BASE_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_DEVICE_CAMERADEVICE1BASE_H_

#include "HidlCameraDevice.h"
//
#include <unordered_map>
#include <utils/Mutex.h>
//
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamClient.h>
#include <mtkcam/v1/IDisplayClient.h>
#include <mtkcam/v1/ICamAdapter.h>
//
#include <mtkcam/device/ICameraDeviceManager.h>
#include <mtkcam/utils/sys/CpuCtrl.h>

#include <vendor/mediatek/hardware/camera/device/1.1/IMtkCameraDeviceCallback.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#if MTKCAM_PLUG_PROCESSING_SUPPORT
#include <mtkcam/hwutils/IPlugProcessing.h>
#endif
#include <mtkcam/hal/IHalSensor.h>
//
using ::android::sp;
using ::android::status_t;
using ::std::shared_ptr;


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class CameraDevice1Base
    : public ICameraDeviceManager::IVirtualDevice
    , public ICameraDevice
    , protected ICam1DeviceCallback
    , public android::hardware::hidl_death_recipient
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    //// locks
    //used in open, releaseRecordingFrameLocked, setPreviewWindow, close, deconstructor
    mutable ::android::Mutex                 mOpsLock;
    mutable android::Mutex                   mBatchLock;
    mutable android::Mutex                   mJoinPreviewThreadLock;    //used for joining the start preview thread

    // setup during constructor
    ICameraDeviceManager*                    mDeviceManager = nullptr;       //  device manager.
    std::string                              mInstanceName;    // instance device: "device@<major>.<minor>/<type>/<id>"
    int32_t                                  mInstanceId;
    android::String8  const                  mDevName;           //  device name.
    sp<CamMsgCbInfo>                         mpCamMsgCbInfo;

    // setup during opening camera
    ::android::sp<ICameraDeviceCallback>     mDeviceCallback = nullptr;
    ::android::hidl::base::V1_0::DebugInfo   mDeviceCallbackDebugInfo;
    ::android::sp<IMtkCameraDeviceCallback>  mMtkDeviceCallback = nullptr;

    sp<android::IParamsManager>              mpParamsMgr;
    sp<android::ICamClient>                  mpCamClient;

    //// [TODO]check the life cycles of the followings
    sp<android::IDisplayClient>              mpDisplayClient;
    sp<android::ICamAdapter>                 mpCamAdapter;
    android::Vector<sp<android::ICamClient>> vmpCamClient;

    ////batch callback related
    uint32_t mBatchSize; // 0 for non-batch mode, set to other value to start batching
    bool mDirectReleaseNoCallback;  //used for buffer callbacks after stopRecording in batch mode
    int32_t mBatchMsgType;   // Maybe only allow DataCallbackMsg::VIDEO_FRAME
    std::vector<HandleTimestampMessage>      mInflightBatch;

    ////
    bool                                     mIsRaw16CBEnabled;
    bool                                     mIsPreviewEnabled;
    int32_t                                  mLastEnableMsg;
    int32_t                                  mOrientation;
    bool                                     mMetadataMode;
    bool                                     mbWindowReady;
    bool                                     mIs4CellSensor;

    ////
    pthread_t                                mStartPreviewTThreadHandle;
    //
    CpuCtrl*                                 mpCpuCtrl = nullptr;
    //
    int32_t                                  mNativeWindowConsumerUsage;
    #if MTKCAM_PLUG_PROCESSING_SUPPORT
    sp<IPlugProcessing>                      mpPlugProcess;
    #endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Info.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    struct CommandInfo{
        int32_t     cmd;
        int32_t     arg1;
        int32_t     arg2;
        CommandInfo(
            int32_t _cmd  = 0,
            int32_t _arg1 = 0,
            int32_t _arg2 = 0
        )
              : cmd(_cmd)
              , arg1(_arg1)
              , arg2(_arg2)
              {}
        };
    typedef android::KeyedVector<int32_t, CommandInfo>   CommandInfoMap_t;
    CommandInfoMap_t                         mTodoCmdMap;
    android::Mutex                           mTodoCmdMapLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    ////Instantiation.
    virtual            ~CameraDevice1Base();
                       CameraDevice1Base(
                           ICameraDeviceManager* deviceManager,
                           std::string           instanceName,
                           int32_t               instanceId,
                           android::String8 const&          rDevName
                       );


public:    ////    Operations.
    int32_t            getInstanceId() const     { return mInstanceId; }
    static CameraDevice1Base* createCameraDevice(
                                  ICameraDeviceManager* deviceManager,
                                  std::string           instanceName,
                                  int32_t               instanceId
                              );
    void               releaseRecordingFrameLocked(
                           uint32_t memId, uint32_t bufferIndex, const native_handle_t* handle);

protected:
    bool               waitStartPreviewDone();
    static void*       startPreviewThread(void* arg);
    void               startPreviewImp();
    void               setDeviceCallback(const sp<ICameraDeviceCallback>& callback);
    void               releaseBatchBufPool();

protected:  ////virtual functions
    virtual bool       queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height);
    virtual int32_t    queryDisplayBufCount() const    { return 3; }
    virtual bool       disableWaitSensorThread(__attribute__((unused)) bool diable) { return false; }
    virtual status_t   initDisplayClient(preview_stream_ops* window);
    virtual bool       initCameraAdapter();
    virtual status_t   enableDisplayClient();
    virtual void       disableDisplayClient();
    virtual bool       waitThreadSensorOnDone()        { return true; }

public:     ////    ICameraDeviceManager::IVirtualDevice Interfaces.
    virtual auto       getDeviceInterfaceBase(
                               sp<IBase>& rpDevice
                           ) const -> status_t override;

    virtual auto       getDeviceInfo() const -> Info const& override;

public:     ////    ICam1DeviceCallback Interface

    virtual void       onCam1Device_NotifyCb(
                               int32_t const   msgType,
                               intptr_t const  ext1
                           );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ::android::hardware::hidl_death_recipient
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void       serviceDied(uint64_t cookie, const android::wp<android::hidl::base::V1_0::IBase>& who) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraDevice Interface implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /* The following interfaces are implemented in CameraDevice:
     *    getResourceCost,
     *    getDeviceInfo,
     *    setTorchMode,
     *    releaseRecordingFrame,
     *    releaseRecordingFrameHandle,
     *    releaseRecordingFrameHandleBatch,
     *
     */
    Return<void>       getResourceCost(getResourceCost_cb _hidl_cb) override;
    Return<void>       getCameraInfo(getCameraInfo_cb _hidl_cb) override;
    Return<Status>     setTorchMode(TorchMode mode) override;
    Return<Status>     dumpState(const hidl_handle& fd) override;
    Return<Status>     open(const sp<ICameraDeviceCallback>& callback) override;
    Return<Status>     setPreviewWindow(const sp<ICameraDevicePreviewCallback>& window) override;
    Return<void>       enableMsgType(uint32_t msgType) override;//
    Return<void>       disableMsgType(uint32_t msgType) override;//
    Return<bool>       msgTypeEnabled(uint32_t msgType) override;//
    Return<Status>     startPreview() override;
    Return<void>       stopPreview() override;
    Return<bool>       previewEnabled() override;
    Return<Status>     storeMetaDataInBuffers(bool enable) override;
    Return<Status>     startRecording() override;
    Return<void>       stopRecording() override;
    Return<bool>       recordingEnabled() override;
    Return<void>       releaseRecordingFrame(uint32_t memId, uint32_t bufferIndex) override;
    Return<void>       releaseRecordingFrameHandle(
                           uint32_t memId, uint32_t bufferIndex, const hidl_handle& frame) override;
    Return<void>       releaseRecordingFrameHandleBatch(
                           const hidl_vec<VideoFrameMessage>&) override;
    Return<Status>     autoFocus() override;
    Return<Status>     cancelAutoFocus() override;
    Return<Status>     takePicture() override;
    Return<Status>     cancelPicture() override;
    Return<Status>     setParameters(const hidl_string& params) override;
    Return<void>       getParameters(getParameters_cb _hidl_cb) override;
    Return<Status>     sendCommand(CommandType cmd, int32_t arg1, int32_t arg2) override;
    Return<void>       close() override;//

    /* The following interfaces are specific for libacdk uses.
     *
     */
    void               open(const sp<ICameraDeviceCallback>& callback, camera_notify_callback notify_cb, camera_data_callback data_cb, camera_data_timestamp_callback data_cb_timestamp, camera_request_memory get_memory);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  [Template method] Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
     * [Template method] Called by open().
     * Initialize the device resources owned by this object and open the camera
     */
    virtual status_t   onOpenLocked();

    /**
     * [Template method] Called by close().
     * Uninitialize and release the device resources owned by this object
     */
    virtual void       onCloseLocked();

    /**
     * [Template method] Called by startPreview().
     */
    virtual bool       onStartPreview() {return true;}

    /**
     * [Template method] Called by stopPreview().
     */
    virtual void       onStopPreview() {}

    /**
     * [Template method] Called by startRecording().
     */
    virtual bool       onStartRecording() {return true;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Wrapper Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static HandleImporter                    sHandleImporter;

    struct CameraMemory : public camera_memory_t {
        MemoryId mId;
        sp<CameraDevice1Base> mDevice = NULL;
    };

    class CameraHeapMemory : public android::RefBase {
    public:
        CameraHeapMemory(int fd, size_t buf_size, uint_t num_buffers = 1);
        explicit CameraHeapMemory(
            sp<IAllocator> ashmemAllocator, size_t buf_size, uint_t num_buffers = 1);
        void commonInitialization();
        virtual ~CameraHeapMemory();

        size_t mBufSize;
        uint_t mNumBufs;

        // Shared memory related members
        hidl_memory      mHidlHeap;
        native_handle_t* mHidlHandle; // contains one shared memory FD
        void*            mHidlHeapMemData;
        sp<IMemory>      mHidlHeapMemory; // munmap happens in ~IMemory()

        CameraMemory handle;
    };

    sp<IAllocator>                           mMemAllocator;
    std::unordered_map<MemoryId, sp<CameraHeapMemory>> mMemoryMap;

public:
    struct CameraPreviewWindow : public preview_stream_ops {
        // Called when we expect buffer will be re-allocated
        void cleanUpCirculatingBuffers();

        android::Mutex mLock;
        sp<ICameraDevicePreviewCallback> mPreviewCallback = nullptr;
        std::unordered_map<uint64_t, buffer_handle_t> mCirculatingBuffers;
        std::unordered_map<buffer_handle_t*, uint64_t> mBufferIdMap;
    } mHalPreviewWindow;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  [Wrapper method] Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    void       initHalPreviewWindow();
    void       releaseRecordingFrame_oldStyle(const void *opaque);
    void       onCallbackTimestamp(
                   nsecs_t timestamp, int32_t msg_type,
                   MemoryId memId , unsigned index, native_handle_t* handle);
    static void sSetDevConnect(std::string mInstanceName, bool isConnect);
    static bool sGetDevConnect(std::string mInstanceName);

    // Preview window callback forwarding methods
    static int sDequeueBuffer(struct preview_stream_ops* w,
                              buffer_handle_t** buffer, int *stride);
    static int sLockBuffer(struct preview_stream_ops* w, buffer_handle_t* buffer);
    static int sEnqueueBuffer(struct preview_stream_ops* w, buffer_handle_t* buffer);
    static int sCancelBuffer(struct preview_stream_ops* w, buffer_handle_t* buffer);
    static int sSetBufferCount(struct preview_stream_ops* w, int count);
    static int sSetBuffersGeometry(struct preview_stream_ops* w,
                                   int width, int height, int format);
    static int sSetCrop(struct preview_stream_ops *w, int left, int top, int right, int bottom);
    static int sSetTimestamp(struct preview_stream_ops *w, int64_t timestamp);
    static int sSetUsage(struct preview_stream_ops* w, int usage);
    static int sSetSwapInterval(struct preview_stream_ops *w, int interval);
    static int sGetMinUndequeuedBufferCount(const struct preview_stream_ops *w, int *count);

    // Device callback forwarding methods
    static void sNotifyCb(int32_t msg_type, int32_t ext1, int32_t ext2, void *user);
    static void sDataCb(int32_t msg_type, const camera_memory_t *data, unsigned int index,
                        camera_frame_metadata_t *metadata, void *user);
    static void sDataCbTimestamp(nsecs_t timestamp, int32_t msg_type,
                                    const camera_memory_t *data, unsigned index, void *user);
    static void sMtkDataCb(int32_t msg_type, camera_metadata_t *result,
                    camera_metadata_t *charateristic, void *user);
    // shared memory methods
    static camera_memory_t* sGetMemory(int fd, size_t buf_size, uint_t num_bufs, void *user);
    static void sPutMemory(camera_memory_t *data);
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_DEVICE_CAMERADEVICE1BASE_H_
