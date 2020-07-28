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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_IAPPSTREAMMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_IAPPSTREAMMANAGER_H_
//
#include "HidlCameraDevice.h"
//
#include <memory>
#include <string>
#include <vector>
//
#include <utils/KeyedVector.h>
#include <utils/Printer.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/Timers.h>
#include <utils/Vector.h>
//
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * An interface of App stream manager.
 */
class IAppStreamManager
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Definitions.

    struct  Request
    {
        /*****
         * Assigned by App Stream Manager.
         */

        /**
         * @param frame number.
         */
        uint32_t    frameNo;

        /**
         * @param input image stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
            >       vInputImageBuffers;

        /**
         * @param output image stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
            >       vOutputImageBuffers;

        /**
         * @param input meta stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamBuffer>
            >       vInputMetaBuffers;

    };

    struct  ConfigAppStreams
    {
        /**
         * @param image streams.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamInfo>
            >       vImageStreams;

        /**
         * @param stream min frame duration.
         */
        android::KeyedVector<
            StreamId_T,
            int64_t
            >       vMinFrameDuration;

        /**
         * @param stream stall frame duration.
         */
        android::KeyedVector<
            StreamId_T,
            int64_t
            >       vStallFrameDuration;

        /**
         * @param meta streams.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamInfo>
            >       vMetaStreams;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

    /**
     * Create an instance.
     */
    struct  CreationInfo
    {
        int32_t                                     mInstanceId;
        android::sp<V3_4::ICameraDeviceCallback>    mCameraDeviceCallback;
        android::sp<IMetadataProvider>              mMetadataProvider;
        android::sp<IMetadataConverter>             mMetadataConverter;
        std::shared_ptr<android::Printer>           mErrorPrinter;
        std::shared_ptr<android::Printer>           mWarningPrinter;
        std::shared_ptr<android::Printer>           mDebugPrinter;
    };
    static auto     create(const CreationInfo& creationInfo) -> IAppStreamManager*;

    /**
     * Destroy the instance.
     */
    virtual auto    destroy() -> void                                       = 0;

    /**
     * Dump debugging state.
     */
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void                                           = 0;

    /**
     * Configure streams.
     *
     * beginConfigureStreams start stream configuration
     * endConfigureStreams end of stream configuration
     *
     * @param[in] requestedConfiguration: the stream configuration requested by
     *  the framework.
     *
     * @param[out] halConfiguration: the HAL's response to each requested stream
     *  configuration.
     *
     * @param[out] rStreams: contains all configured streams.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    beginConfigureStreams(
                        const V3_4::StreamConfiguration& requestedConfiguration,
                        V3_4::HalStreamConfiguration& halConfiguration,
                        ConfigAppStreams& rStreams
                        ) -> int                                            = 0;

    virtual auto    endConfigureStreams(
                        V3_4::HalStreamConfiguration& halConfiguration
                        ) -> int                                            = 0;

    /**
     * Retrieves the fast message queue used to pass request metadata.
     *
     * If client decides to use fast message queue to pass request metadata,
     * it must:
     * - Call getCaptureRequestMetadataQueue to retrieve the fast message queue;
     * - In each of the requests sent in processCaptureRequest, set
     *   fmqSettingsSize field of CaptureRequest to be the size to read from the
     *   fast message queue; leave settings field of CaptureRequest empty.
     *
     * @return queue the queue that client writes request metadata to.
     */
    virtual auto    getCaptureRequestMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&
                                                                            = 0;

    /**
     * Retrieves the fast message queue used to read result metadata.

     * Clients to ICameraDeviceSession must:
     * - Call getCaptureRequestMetadataQueue to retrieve the fast message queue;
     * - In implementation of ICameraDeviceCallback, test whether
     *   .fmqResultSize field is zero.
     *     - If .fmqResultSize != 0, read result metadata from the fast message
     *       queue;
     *     - otherwise, read result metadata in CaptureResult.result.
     *
     * @return queue the queue that implementation writes result metadata to.
     */
    virtual auto    getCaptureResultMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&
                                                                            = 0;

    /**
     * Flush requests.
     *
     * @param[in] requests: the requests to flush.
     *
     */
    virtual auto    flushRequest(
                        const hidl_vec<V3_4::CaptureRequest>& requests
                        ) -> void                                           = 0;

    /**
     * Remove the
     *
     * @param[in] cachesToRemove: The cachesToRemove argument contains a list of
     *  buffer caches to be removed
     */
    virtual auto    removeBufferCache(
                        const hidl_vec<BufferCache>& cachesToRemove
                        ) -> void                                           = 0;

    /**
     * Submit a set of requests.
     * This call is valid only after streams are configured successfully.
     *
     * @param[in] requests: a set of given requests in terms of a form of
     *  HIDL-Hal CaptureRequest.
     *
     * @param[out] rRequests: a set of requests, created by IAppStreamManager,
     *  associated with the given CaptureRequest.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    submitRequest(
                        const hidl_vec<V3_4::CaptureRequest>& requests,
                        android::Vector<Request>& rRequests
                        ) -> int                                            = 0;

    /**
     * Wait until all the registered requests have finished returning.
     *
     * @param[in] timeout
     */
    virtual auto    waitUntilDrained(nsecs_t const timeout) -> int          = 0;

    /**
     * Update a given result frame.
     *
     * @param[in] frameNo: the frame number to update.
     *
     * @param[in] userId: user id.
     *
     * @param[in] resultMeta: result partial metadata to update.
     *
     * @param[in] hasLastPartial: contain last partial metadata in result partial metadata vector.
     */
    virtual auto    updateResult(
                        uint32_t const frameNo,
                        intptr_t const userId,
                        android::Vector<android::sp<IMetaStreamBuffer>> resultMeta,
                        bool hasLastPartial
                        ) -> void                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_IAPPSTREAMMANAGER_H_

