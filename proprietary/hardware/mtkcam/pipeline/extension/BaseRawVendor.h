/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASERAWVENDOR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASERAWVENDOR_H_

#include "BaseVendor.h"
#include "utils/RequestFrame.h"

// STL
#include <queue>
#include <mutex>
#include <condition_variable>


using namespace android;


namespace NSCam {
namespace plugin {

// BaseRawVendor is a base class for who wants to implement RAW in RAW out
// plugin.
class BaseRawVendor : public BaseVendor // use the implementations from BaseRawVendor
{
//
// Re-implementation of IVendor
//
public:
    using BaseVendor::getName;
    using BaseVendor::getOpenId;
    using BaseVendor::getVendorMode;
    using BaseVendor::acquireWorkingBuffer;
    using BaseVendor::releaseWorkingBuffer;


    //
    // Construct setting.
    //
    virtual status_t            get(
                                    MINT32           openId,
                                    const InputInfo& in,
                                    FrameInfoSet&    out
                                ) override;

    virtual status_t            set(
                                    MINT32              openId,
                                    const InputSetting& in
                                ) override;

    //
    // For user
    //
    virtual status_t            queue(
                                    MINT32  const              openId,
                                    MUINT32 const              requestNo,
                                    wp<IVendor::IDataCallback> cb,
                                    BufferParam                bufParam,
                                    MetaParam                  metaParam
                                ) override;

    virtual status_t            beginFlush( MINT32 openId ) override;
    virtual status_t            endFlush( MINT32 openId ) override;
    virtual status_t            sendCommand(
                                    MINT32 cmd,
                                    MINT32 openId,
                                    MetaItem& meta,
                                    MINT32& arg1, MINT32& arg2,
                                    void*  arg3
                                ) override;

    // debug
    virtual status_t            dump( MINT32 openId ) override;

//
// Methods
//
protected:
    //
    // caller need to override this
    //
    virtual void                onProcessFullRaw(
                                    IImageBuffer*   pRaw,
                                    IMetadata*      pAppMeta,
                                    IMetadata*      pHalMeta
                                    );

    virtual void                onProcessRrzoRaw(
                                    IImageBuffer*   pRaw,
                                    IMetadata*      pAppMeta,
                                    IMetadata*      pHalMeta
                                    );

    // Wait until all controllers has finished jobs.
    //  @param bCancel          Tells all controllers to cancel job.
    virtual void                drain(bool bCancel = true);


//
// BaseVendor implementation
//
protected:
    using BaseVendor::setThreadShotsSize;
    using BaseVendor::asyncRun;
    using BaseVendor::syncAllThread;


//
// Attributes
//
protected:
    using BaseVendor::mMode;
    using BaseVendor::mOpenId;
    using BaseVendor::mName;

    // stream ID map
    RequestFrame::StreamIdMap_Img  m_streamIdMap_img;
    RequestFrame::StreamIdMap_Meta m_streamIdMap_meta;

    // RequestFrame queue, locker, condition variable
    std::queue<std::shared_ptr<RequestFrame>>
                                m_requestFrameQueue;
    std::mutex                  m_requestFrameQueueMx;
    std::condition_variable     m_requestFrameQueueCv;


//
// Constructor & Destructor
//
public:
    BaseRawVendor(
            char const*  pcszName,
            MINT32 const i4OpenId,
            MINT64 const vendorMode
        );

    // copy and move object is forbidden
    BaseRawVendor(const BaseRawVendor&) = delete;
    BaseRawVendor(BaseRawVendor&&) = delete;

    virtual ~BaseRawVendor();
};

} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_BASERAWVENDOR_H_
