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

/**
 * @file CallbackImageBufferManager.h
 * @brief Used to maintain all callback image buffer.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_CALLBACK_IMAGE_BUFFER_MANAGER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_CALLBACK_IMAGE_BUFFER_MANAGER_H_

// Standard C header file

// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
// Local header file


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace v1 {
    class StreamBufferProvider;
    class BufferCallbackHandler;
};
};
using namespace std;
using namespace android;
using namespace NSCam::v1;
using namespace NSCam::v3;
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class CallbackImageBufferManager
 * @brief Used to maintain all callback image buffer.
 */
class CallbackImageBufferManager
    : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    CallbackImageBufferManager() = delete;
    CallbackImageBufferManager(
                                MUINT32 openId,
                                sp<ImageStreamManager> pImageStreamManager);

protected:
// Destructor
    virtual ~CallbackImageBufferManager();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  sendCallback Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief Allocate image callback provider by imageStream.
     * @param [in] imageStream allocate image stream info
     * @return return allocate status
     */
    status_t allocBuffer(
                                sp<IImageStreamInfo> pImgStreamInfo);
    /**
     * @brief Allocate image callback provider by imageStream and set to callback handler.
     * @param [in] imageStream allocate image stream info
     * @param [in] pCallbackHandler callback handler
     * @return return allocate status
     */
    status_t allocBuffer(
                                sp<IImageStreamInfo> pImgStreamInfo,
                                sp<BufferCallbackHandler> pCallbackHandler);
    /**
     * @brief get buffer provider with specific streamId.
     * @param [in] streamId to get specific buffer provider.
     * @return return buffer provider address.
     */
    sp<StreamBufferProvider> getBufferProvider(
                                StreamId_T const streamId);
    /**
     * @brief Set timestamp to all callback image provider.
     * @param [in] requestNo The request number of current capture frame.
     * @param [in] timestamp timestamp value.
     * @return The process result
     */
    status_t setTimestamp(
                                MUINT32 requestNo,
                                MINT64 timestamp);
    /**
     * @brief release all provider in pool.
     * @return The process result
     */
    status_t destory();
    /**
     * @brief return buffer provider pool
     * @return The process result
     */
    DefaultKeyedVector<StreamId_T, sp<StreamBufferProvider> > getBufferProviderPool();
private:
    /**
     * @brief add provider to pool.
     * @return The process result
     */
    status_t addToPool(StreamId_T streamId, sp<StreamBufferProvider> provider);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MUINT32     miOpenId = -1;
    //
    DefaultKeyedVector<StreamId_T, sp<StreamBufferProvider> >   mvBufferProviderPool;
    wp<ImageStreamManager> mpImageStreamManager = nullptr;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif