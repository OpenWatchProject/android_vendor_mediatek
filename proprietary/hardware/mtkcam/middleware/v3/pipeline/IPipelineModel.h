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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_IPIPELINEMODEL_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_IPIPELINEMODEL_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

namespace NSPipelineContext {
    class PipelineContext;
}

/******************************************************************************
 *
 ******************************************************************************/
class IPipelineModel
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    struct  AppRequest
    {
        /**
         * @param[in] request number.
         *  The caller must promise its content.
         *  The callee can not modify it.
         */
        MUINT32                     requestNo;

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
public:     ////                    Attributes.
    virtual char const*             getName() const                         = 0;
    virtual MINT32                  getOpenId() const                       = 0;

public:     ////                    Operations.
    /**
     * Submit a request.
     *
     * @param[in] request: App request to submit.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  submitRequest(AppRequest& request)      = 0;

    /**
     * turn on flush flag as flush begin and do flush
     *
     */
    virtual MERROR                  beginFlush()                            = 0;

    /**
     * turn off flush flag as flush end
     *
     */
    virtual MVOID                   endFlush()                              = 0;

    /**
     * Notify the IPipelineModel that it won't get any further requests.
     * IPipelineModel cound base on this api to properly finish previous
     * requests.
     *
     */
    virtual MVOID                   endRequesting()                         = 0;

    /**
     * Wait for requests drained
     *
     */
    virtual MVOID                   waitDrained()                           = 0;

    /**
     * Get context of this pipelinemodel
     *
     */
    virtual android::sp<NSCam::v3::NSPipelineContext::PipelineContext>
                                    getContext()                                = 0;
};


/******************************************************************************
*
******************************************************************************/

};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_IPIPELINEMODEL_H_

