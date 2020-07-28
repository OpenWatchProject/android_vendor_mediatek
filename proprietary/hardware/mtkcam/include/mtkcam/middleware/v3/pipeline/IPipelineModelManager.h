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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MIDDLEWARE_V3_PIPELINE_IPIPELINEMODELMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MIDDLEWARE_V3_PIPELINE_IPIPELINEMODELMANAGER_H_
//
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
//
#include <string>
#include <vector>
//
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>
#include <utils/Vector.h>
//

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineModelManager : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    class IAppCallback
        : public virtual android::RefBase
    {
    public:     ////    Operations.
        virtual auto    onFrameUpdated(
                            uint32_t const requestNo,
                            intptr_t const userId,
                            ssize_t const nOutMetaLeft,
                            android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
                            ) -> void                                       = 0;
    };


    struct  AppCreation
    {
        /**
         * @param[in] open Id.
         *  The caller must promise its value.
         */
        int32_t                     openId;

        /**
         * @param[in] user name.
         *  The caller must promise its value.
         */
        android::String8            userName;

        /**
         * @param[in] App callback.
         *  The caller must promise its value.
         */
        android::wp<IAppCallback>   appCallback;
    };


    struct  AppConfiguration
    {
        /**
         * @param[in] Pipeline operation mode.
         *  The caller must promise its value.
         */
        uint32_t                    operationMode;

        /**
         * @param[in] App image streams to configure.
         *  The caller must promise the number of buffers and each content.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamInfo>
                >                   vImageStreams;

        /**
         * @param[in] App meta streams to configure.
         *  The caller must promise the number of buffers and each content.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamInfo>
                >                   vMetaStreams;

        /**
         * @param[in] App image streams min frame duration to configure.
         *  The caller must promise its initial value.
         */
        android::KeyedVector<
            StreamId_T,
            int64_t
                >                   vMinFrameDuration;

        /**
         * @param[in] App image streams stall frame duration to configure.
         *  The caller must promise its initial value.
         */
        android::KeyedVector<
            StreamId_T,
            int64_t
                >                   vStallFrameDuration;

    };


    struct  AppRequest
    {
        /**
         * @param[in] request number.
         *  The caller must promise its content.
         *  The callee can not modify it.
         */
        uint32_t                    requestNo;

        /**
         * @param[in,out] input App image stream buffers, if any.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
                            >       vIImageBuffers;

        /**
         * @param[in,out] output App image stream buffers.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
                            >       vOImageBuffers;

        /**
         * @param[in,out] input App meta stream buffers.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamBuffer>
                            >       vIMetaBuffers;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    static  auto    create(
                        AppCreation const& params
                        ) -> android::sp<IPipelineModelManager>;

    /**
     * Wait until the initialization is done.
     *
     * @return
     *      Nonzero/true indicates success; otherwise failure.
     */
    virtual auto    waitUntilInitDone() -> bool                             = 0;

    /**
     * Configure the pipeline
     *
     * @param[in] rParams: the parameters to configure.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    configure(AppConfiguration const& params) -> int        = 0;

    /**
     * Submit a set of requests.
     *
     * @param[in] requests: a set of App requests to submit.
     *
     * @param[out] numRequestProcessed: number of requests successfully submitted.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    submitRequest(
                        android::Vector<AppRequest>const& requests,
                        uint32_t& numRequestProcessed
                        ) -> int                                            = 0;

    /**
     * turn on flush flag as flush begin and do flush
     *
     */
    virtual auto    beginFlush() -> int                                     = 0;

    /**
     * turn off flush flag as flush end
     *
     */
    virtual auto    endFlush() -> void                                      = 0;

    /**
     * Dump debugging state.
     */
    virtual auto    dumpState(
                        int fd,
                        const std::vector<std::string>& options
                        ) -> void                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MIDDLEWARE_V3_PIPELINE_IPIPELINEMODELMANAGER_H_

