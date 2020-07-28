/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_BASECONTROLLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_BASECONTROLLER_H_

#include "RequestFrame.h"

//AOSP
#include <utils/RefBase.h>

// MTKCAM
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform
#include <mtkcam/utils/hw/IScenarioControl.h>

// STL
#include <cstdint>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <future>
#include <cstdint>
#include <string>

using NSCam::NSIoPipe::NSSImager::IImageTransform;

#define NOTIFY_CALLBACK_MAX_SIZE 2

namespace NSCam {
namespace plugin {

// [This is a reentrant but NOT a thread-safe class]
//
// ScenarioCtrl is a class for caller who needs to handle enter/exit
// camera scenario much easier.
//
// It will enter indicated scenario during instance construct, and
// exit scenario automatically during instance deconstruct.
class ScenarioCtrl
    : public virtual android::RefBase
{
public:
    ScenarioCtrl(
        const MINT32 openId,
        const MUINT32 sensorMode,
        const MINT32 scenario);
    ScenarioCtrl(const ScenarioCtrl&) = delete; // forbidden

    virtual ~ScenarioCtrl();

protected:
    // scenario control
    MINT32                          m_openId;
    MINT32                          m_sensorMode;
    MINT32                          m_scenario;
    MBOOL                           m_enterResult;
    sp<IScenarioControl>            m_scenarioControl;
};

// [This is a thread-safe class]
//
// BaseController is a base class for caller who needs multi input requests and
// only a output result mechanism. To simplify things, BaseController is designed
// as an ONE-SHOT instance which means that, basically, caller doesn't reset
// the BaseController. Instead of reset it, caller usually create a new
// BaseController for the next set of requests. Because it's cheap to create a
// BaseController.
//
// Derived class should re-implement BaseController::job to do the specific job.
// Caller can invoke BaseController::execute and BaseController::waitExecution
// to fire job asynchronized and wait(get result).
//
// If caller wants to execute job synchronized, invoke BaseController::job
// directly.
//
// Note: All methods in this class are reentrant and thread-safe.
class BaseController
{
public:
    typedef NSCam::SpinLock T_MUTEX; // locker type if request number queue

    enum ExecutionStatus
    {
        ES_NOT_STARTED_YET = 0,
        ES_RUNNING,
        ES_READY
    };


//
// Constructor(s)/Destructor
//
public:
    BaseController(const char* callerName = nullptr);
    BaseController(const char* callerName, const MUINT32 sensorMode);
    BaseController(const BaseController&) = delete; // Copy a BaseController is forbidden
    BaseController(BaseController&&) = default;
    virtual ~BaseController();


//
// Public methods
//
public:
    // Check if the request number is belong to this request.
    //  @param requestNo        Request number.
    //  @return                 Yes or no.
    //  @note                   This method is thread-safe
    virtual bool                isBelong(MUINT32 requestNo) const;

    // Check if the belong request number is empty.
    //  @param requestNo        Request number.
    //  @return                 Yes or no.
    //  @note                   This method is thread-safe
    virtual bool                isEmptyBelong() const;

    // Check if controller has been invalidated or not. If invalidated, this
    // controller cannot enque any frame.
    //  @return                 Yes or no.
    //  @note                   This method is thread-safe.
    virtual bool                isInvalidated() const;

    // Set the request number and tell BaseController the request number is
    // belong to it.
    //  @param requestNo        Request number which is belong to me.
    //  @note                   This method is thread-safe.
    virtual void                setBelong(MUINT32 requestNo);

    // Set the message callback instance from IVendor::set
    //  @param cb               Weak pointer of message callback instance.
    //  @param idx              callback array index.
    //  @note                   This method is thread-safe.
    virtual void                setMessageCallback(wp<INotifyCallback> cb, MUINT idx = 0);

    // Clear the request number.
    //  @note                   This method is thread-safe.
    virtual void                clearBelong(MUINT32 requestNo);

    // Clear all request number.
    //  @note                   This method is thread-safe.
    virtual void                clearBelong();

    // Enque a frame, arguments are passed by IVendor. And signal one thread
    // which is waiting for frame.
    //  @param openId           The open id of from IVendor.
    //  @param requestNo        The request number from IVendor.
    //  @param bufParam         The argument from IVendor.
    //  @param metaParam        The argument from IVendor.
    //  @param cb               The argument from IVendor.
    //  @return                 If enqueued OK return 0, otherwise check retval.
    //  @retval 0               OK.
    //  @retval -1              This controller has been invalidated already, caller
    //                          has responsibility to handle this frame.
    //  @note                   This method is thread-safe.
    virtual int                 enqueFrame(
                                    const MINT32 openId,
                                    MUINT32 requestNo,
                                    const IVendor::BufferParam& bufParam,
                                    const IVendor::MetaParam& metaParam,
                                    wp<IVendor::IDataCallback> cb
                                    );

    // Enque a RequestFrame directly. This method wil signal a thread which is
    // waiting for a frame. It's usually to enque a nullptr as a dummy frame to
    // raise the thread is blocked by dequeFrame.
    //  @param frame            A RequestFrame, can be nullptr.
    //  @return                 If enqueued OK return 0, otherwise check retval.
    //  @retval 0               OK.
    //  @retval -1              This controller has been invalidated already, caller
    //                          has responsibility to handle this frame.
    //  @note                   This method is thread-safe.
    virtual int                 enqueFrame(std::shared_ptr<RequestFrame> frame);

    // Deque a frame from queue. This method will block caller's thread if there's
    // no frame in queue, and wait until a frame has been dequed or
    // BaseController::invalidate has been invoked.
    //  @return                 A frame. If during waiting, other thread invokes
    //                          BaseController::invalidate(), this method returns
    //                          nullptr.
    //  @note                   The method may block caller's thread.
    //  @note                   This method is thread-safe.
    virtual std::shared_ptr<RequestFrame> dequeFrame();

    // Deque a frame from queue with a timeout time. This method will block
    // caller's thread in the maximum milliseconds "timeoutMs" if there's no
    // frame in queue.
    //  @param [out] frame      A shared_ptr to contain dequed frame. If timeout,
    //                          this argument will be NULL.
    //  @param [in] timeoutMs   Timeout time, in milliseconds.
    //  @return                 A status of condition_variable. If timeout,
    //                          this method returns "std::cv_status::timeout".
    //
    //  @retval std::cv_status::no_timeout      Deque ok!
    //  @retval std::cv_status::timeout         Deque failed due to timeout.
    //
    //  @note                   This method is thread-safe.
    virtual std::cv_status      dequeFrame(
                                    std::shared_ptr<RequestFrame>& frame,
                                    int timeoutMs
                                    );

    // Clear all frames in queue.
    //  @note                   This method is thread-safe.
    virtual void                clearFrames();

    // Invalidate the controller. All the frames in queue will be cleared, and
    // the frame is being queued will be dropped immediately and never allows any
    // frame to be queued. After invoked this method, the controller comes into
    // an invalidated status, but caller can invoke BaseController::validate to
    // validate the controller again. Method BaseController::dequeFrame will
    // return nullptr immediately after invalidated.
    //  @note                   This method will raise the threads which is blocked
    //                          by BaseController::dequeFrame
    //
    //  @note                   This method is thread-safe.
    virtual void                invalidate();

    // Validate the controller from invalidated status to validated.
    //  @note                   This method is thread-safe.
    virtual void                validate();

    // Reset the controller to the default state and to be reused.
    //  @note                   If the execution is still working, the caller's
    //                          thread will be blocked until done.
    //                          This method is thread-safe.
    virtual void                reset();

    // Execute job of controller asynchronously. If the previous job hasn't finished,
    // this method returns false.
    //  @note                   This method is thread-safe.
    virtual bool                execute(intptr_t arg1 = 0, intptr_t arg2 = 0);

    // Wait until execution has finished, block caller's thread. If the execution
    // is not started yet, this method returns false immediately. If the execution
    // has already finished, this method returns true immediately.
    //  @param  result          Pointer of an intptr_t, contains return value
    //                          from BaseController::job. Can be null.
    //  @return                 If the job is not executed yet, this method
    //                          returns false, otherwise returns true.
    //
    //  @note                   This method is thread-safe.
    virtual bool                waitExecution(intptr_t* result = nullptr);

    // Get the execution status.
    //  @return                 The result of execution.
    //  @note                   This method is thread-safe.
    virtual ExecutionStatus     getExecutionStatus() const;


    // The job will be executed while invoking BaseController::execute.
    //  @return                 Result of the job.
    //  @note                   This method always returns 0.
    //                          This method is thread-safe.
    virtual intptr_t            job(intptr_t arg1 = 0, intptr_t arg2 = 0);

    // Handle output yuv buffers & digital zoom
    //  @param msg              Message to be printed in handleYuvOutput.
    //  @param pResult          The source image, which must be the same size
    //                          as P1 node's output.
    //  @param pJpegYuv         The output YUV of JPEG, with outOrientaion, which means,
    //                          if (outOrientaion & 90_degree) is true, this method
    //                          will swap width and height of pJpegYuv.
    //  @param pThumbYuv        The thumbnail YUV of JPEG, w/o outOrientaion, which means,
    //                          the orientation of thumbnail YUV MUST be always 0 degree.
    //  @param pAppIn           The app metadata for reference to query view angle.
    //  @param pHalIn           The hal metadata for reference to query view angle.
    //  @param openId           The opened sensor ID for reference to query view angle.
    //  @param outOrientaion    The output orientation of pJpegYuv.
    //  @param requestNo        The debug information for ClearZoom feature.
    //  @param needCrop1        If this value is false, this method won't apply any
    //                          crop to pJpegYuv.
    //  @param needCrop2        If this value is false, this method won't apply any
    //                          crop to pThumbYuv.
    //  @param rpExifMeta       The metadata to be updated of MDP debug EXIF. The DEBUG_EXIF_RESERVE3
    //                          will be updated only.
    //  @return                 The result of execution.
    //  @note                   The image pThumbYuv is necessary to be always with
    //                          orientation 0 degree.
    virtual bool                handleYuvOutput(
                                    std::string     msg,
                                    IImageBuffer*   pResult,
                                    IImageBuffer*   pJpegYuv,
                                    IImageBuffer*   pThumbYuv,
                                    IMetadata*      pAppIn,
                                    IMetadata*      pHalIn,
                                    int openId,
                                    int outOrientaion,
                                    int requestNo = 0,
                                    bool needClearZoom = true,
                                    bool needCrop1 = true,
                                    bool needCrop2 = true,
                                    IMetadata* pMetadataExif = nullptr
                                    );

    virtual void                doCancel();

    // Get sensor mode during  execution.
    //  @return                 sensor mode.
    virtual uint32_t            getSensorMode() const;

    // Enter capture scenario for P2 performance speedup.
    //  @return                 The result of execution.
    virtual bool                enterCaptureScenario(
                                    const MINT32 openId,
                                    const MUINT32 sensorMode
                                    );

    // Exit capture scenario for power consumption.
    //  @return                 The result of execution.
    virtual bool                exitCaptureScenario();

//
// Message Callback
//
public:
    // Try to send shutter sound callback if message callback instance
    // (BaseController::m_messageCb) is still exists.
    //  @note                   This method is thread-safe.
    virtual void                onShutter();

    // Try to send onNextCaptureReady event if message callback instance
    // (BaseController::m_messageCb) is still exists.
    //  @note                   This method is thread-safe.
    virtual void                onNextCaptureReady();


//
// Static methods (helper)
//
public:
    // Set the metadata set of RequestFrame as output metadata.
    //  @param pReqFrm          Set the metadata as the result metadata.
    //  @return                 Set ok returns true.
    //  @note                   This method is thread-safe.
    static bool                 setMetadtaToResultFrame(
                                    const std::shared_ptr<RequestFrame> pMainFrame,
                                    std::shared_ptr<RequestFrame> pResultFrame
                                    );


//
// Get methods
//
public:
    // Return size of frame this controller need to handle.
    //  @return                 Size of belong frame.
    //  @note                   This method is thread-safe.
    inline int getFrameSize() const
    {
        std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
        return m_frameNumber;
    }

    // To get the times that frame has been enqued.
    //  @return                 Enqued times.
    //  @note                   This method is thread-safe.
    inline int getEnquedTimes() const
    {
        std::lock_guard<std::mutex> __L(m_incomingFramesMx);
        return m_enquedTimes;
    }

    // Return size of RequestFrame in queue.
    //  @return                 Size of queued frames
    //  @note                   This method is thread-safe.
    inline size_t getQueuedFramesSize() const
    {
        std::lock_guard<std::mutex> __L(m_incomingFramesMx);
        return m_incomingFrames.size();
    }

    // To get the image buffer StreamID map of the controller.
    //  @return                 Return by a const reference of map container.
    //  @note                   This method is thread-safe.
    inline const RequestFrame::StreamIdMap_Img& getStreamIdMapImg() const
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        return m_streamIdMap_img;
    }

    // To get the metadata StreamID map of the controller.
    //  @return                 Return by a const reference of map container.
    //  @note                   This method is thread-safe.
    inline const RequestFrame::StreamIdMap_Meta& getStreamIdMapMeta() const
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        return m_streamIdMap_meta;
    }


//
// Set methods
//
public:
    inline void setFrameSize(const int f)
    {
        std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
        m_frameNumber = f;
    }
    // Copy image buffers StreamID map.
    //  @param m                StreamID map container.
    //  @note                   This method is thread-safe.
    inline void setStreamIdMapImg(const RequestFrame::StreamIdMap_Img& m)
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        m_streamIdMap_img = m;
    }

    // Move image buffers StreamID map w/o copying.
    //  @param m                THe rvalue reference of StreamID container.
    //  @note                   This method is thread-safe.
    inline void setStreamIdMapImg(RequestFrame::StreamIdMap_Img&& m)
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        m_streamIdMap_img = std::move(m);
    }

    // Copy metadata StreamID map.
    //  @param m                StreamID map container.
    //  @note                   This method is thread-safe.
    inline void setStreamIdMapMeta(const RequestFrame::StreamIdMap_Meta& m)
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        m_streamIdMap_meta = m;
    }

    // Move metadata StreamID map.
    //  @param m                THe rvalue reference of StreamID container.
    //  @note                   This method is thread-safe.
    inline void setStreamIdMapMeta(RequestFrame::StreamIdMap_Meta&& m)
    {
        std::lock_guard<std::mutex> __L(m_streamIdMapMx);
        m_streamIdMap_meta = std::move(m);
    }


//
// Methods for caller wants to traverse request number queue or incoming frame queue.
// Notice that, if caller entered the critical section, only these raw methos can
// be invoked, or it happens a deadlock problem.
//
public:
    // To retrieve request number queue w/o locked.
    //  @return                 Reference of request number queue.
    inline std::deque<MUINT32>& requestNumbers()
    {
        return m_requestNoStack;
    }

    // To retrieve request number queue as a constant w/o locked.
    //  @return                 Reference of request number queue.
    inline const std::deque<MUINT32>& requestNumbers() const
    {
        return m_requestNoStack;
    }

    // To retrieve incoming frame queue w/o locked.
    //  @return                 Reference of incoming frame queue.
    inline std::deque< std::shared_ptr<RequestFrame> >& incomingFrames()
    {
        return m_incomingFrames;
    }

    // To retrieve incoming frame queue as a constain w/o locked.
    //  @return                 Reference of incoming frame queue.
    inline const std::deque< std::shared_ptr<RequestFrame> >& incomingframes() const
    {
        return m_incomingFrames;
    }

    // To lock and enter the critical section for accessing request number queue.
    //  @return                 An std::unique_lock instance w/ locked mutex.
    inline std::unique_lock<T_MUTEX> lockRequestQueueLocker()
    {
        return std::unique_lock<T_MUTEX>(m_requestNoStackMx);
    }

    // To lock and enter the critical section for accessing request number queue.
    //  @return                 An std::unique_lock instance w/ locked mutex.
    inline std::unique_lock<T_MUTEX> lockRequestQueueLocker() const
    {
        return std::unique_lock<T_MUTEX>(m_requestNoStackMx);
    }

    // To lock and enter the critical section for incoming frames queue.
    //  @return                 An std::unique_lock instance w/ locked mutex.
    inline std::unique_lock<std::mutex> lockIncomingFramesLocker()
    {
        return std::unique_lock<std::mutex>(m_incomingFramesMx);
    }

    // To lock and enter the critical section for incoming frames queue.
    //  @return                 An std::unique_lock instance w/ locked mutex.
    inline std::unique_lock<std::mutex> lockIncomingFramesLocker() const
    {
        return std::unique_lock<std::mutex>(m_incomingFramesMx);
    }


//
//  Operators
//
public:
    // Move assignment operator.
    //  @note                   If the self's execution is still working, the
    //                          caller's thread will be blocked until done.
    //                          This method is thread-safe.
    virtual BaseController& operator = (BaseController&& other);

    // Copy a BaseController is forbidden.
    virtual BaseController& operator = (const BaseController& other) = delete;

    // Check if the controller is the same one using pointer value
    //  @note                   This method is thread-safe.
    virtual bool            operator == (const BaseController& other);

    // Check if the controller is not the same one using pointer value
    //  @note                   This method is thread-safe.
    virtual bool            operator != (const BaseController& other);

private:
    // dump image buffer for debug.
    //  @param pImg             ImageBuffer for dump data.
    //  @return                 Dump ok returns true.
    //  @note                   This method is thread-safe.
    virtual bool dumpImage(
                    IImageBuffer* pImg,
                    IMetadata*    pHalIn,
                    const char*   prefix,
                    const char*   stageName,
                    const char*   extName);

//
// Attributes
//
protected:
    // Represents enque times.
    int m_enquedTimes;

    // Caller name
    const char* m_callerName;

    // Sensor mode
    uint32_t m_sensorMode;

    // Message callback
    wp<INotifyCallback> m_messageCb[NOTIFY_CALLBACK_MAX_SIZE];
    mutable T_MUTEX     m_messageCbMx;

    // Represents if the controller has been invalidated
    volatile bool                       m_bInvalidated;
    mutable T_MUTEX                     m_invalidateMx;

    // Stream ID for input/output image
    RequestFrame::StreamIdMap_Img       m_streamIdMap_img;
    RequestFrame::StreamIdMap_Meta      m_streamIdMap_meta;
    mutable std::mutex                  m_streamIdMapMx;

    // Request number container
    int                                 m_frameNumber;
    std::deque<MUINT32>                 m_requestNoStack;
    mutable T_MUTEX                     m_requestNoStackMx;

    // RequestFrame container
    std::deque< std::shared_ptr<RequestFrame> >     m_incomingFrames;
    mutable std::mutex                              m_incomingFramesMx;
    std::condition_variable                         m_incomingFramesCond;

    // Executable thread and op lock
    std::shared_future<intptr_t>    m_futureExe;
    mutable std::mutex              m_futureExeMx;

    // operator assignment & function locker
    mutable std::mutex              m_funcEnqueFrameMx;
    mutable std::mutex              m_opAssignmentMx;

    // scenario control
    mutable std::mutex              m_opScenarioCtrlMx;
    sp<ScenarioCtrl>                m_spScenarioCtrl;
    static wp<ScenarioCtrl>         m_wpScenarioCtrl;

private:
    // dump flag to enable image dump for debug
    int32_t m_dumpFlag;

}; // class BaseController
}; // namespace plugin
}; // namespace NSCam
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_BASECONTROLLER_H_
