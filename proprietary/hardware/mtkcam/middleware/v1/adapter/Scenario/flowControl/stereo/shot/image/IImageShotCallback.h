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
 * @file IImageShotCallback.h
 * @brief IImageShotCallback interface.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_IMAGECALLBACK_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_IMAGECALLBACK_H_

// Standard C header file
#include <string>
// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
// Local header file
#include "../ICaptureRequestCB.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace android {
namespace NSShot{
class IShotCallback;
};
};
//
using namespace std;
using namespace android;
using namespace android::NSShot;
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class IImageShotCallback
 * @brief Image callback interface.
 */
class IImageShotCallback
    : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Inner struct
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
/**
 * Type of Camera Stream Id.
 */
typedef MINTPTR                     StreamId_T;
    struct ImageCallbackParams
    {
        StreamId_T streamId = 0;
        string streamName = "";
        string fileExtension = "";
        MINT32 callbackIndex = 0;
        MBOOL mbNeedToCountForCapture = MTRUE;
        MINT64 timeStamp = 0;
        MUINT32 msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
        MBOOL mbNeedCaptureDoneCb = MTRUE;
        // let Callback processor to fill under field.
        MBOOL enableDumpBuffer = MFALSE;
        string filePath = "";
        MBOOL bErrorBuffer = MFALSE;
        MUINT32 slot = 0;
    };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    IImageShotCallback() = delete;
    IImageShotCallback(ImageCallbackParams params)
    {
        mParams = params;
    }

protected:
// Destructor
    ~IImageShotCallback()
    {
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  sendCallback Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief If result process reverses callback success message, it will send callback to user.
     * @param [in] pShotCallback shot callback to send related callback to adapter.
     * @param [in] streamId current received stream id
     * @param [in] pBuffer current image buffer that needs to callback
     * @param [in] dumpBuffer if dumpBuffer is true, it will dump image buffer with stream name and related file extension.
     * @return The process result
     */
    virtual status_t sendCallback(
                                sp<IShotCallback> pShotCallback,
                                StreamId_T const streamId,
                                sp<IImageBuffer>& pBuffer,
                                MBOOL bFinalImage) = 0;
    /**
     * @brief If result processor receives callback fail message, it will call processFailData to notify user.
     * @param [in] reqNo failed request number.
     * @param [in] streamId failed request stream id.
     * @param [in] pBuffer failed request iamge buffer.
     * @return The process result
     */
    virtual status_t processFailData(
                                MUINT32 const reqNo,
                                StreamId_T const streamId,
                                sp<IImageBuffer>& pBuffer) = 0;
    /**
     * @brief If result processor receives callback fail message, it will call processFailData to notify user.
     * @param [in] reqNo failed request number.
     * @param [in] streamId failed request stream id.
     * @param [in] pBuffer failed request iamge buffer.
     * @return The process result
     */
    virtual status_t dumpBuffer(sp<IImageBuffer>& pBuffer) = 0;
    /**
     * @brief return StreamId_T.
     * @return The process result
     */
    virtual MUINT32 getStreamId() = 0;
    /**
     * @brief return mbNeedToCountForCapture in ImageCallbackParams.
     * @return If this image needs to count, it will return true.
     */
    virtual MBOOL isNeedImageCount() = 0;
    /**
     * @brief set dump buffer infomation.
     * @param [in] enable enable dump buffer flag.
     * @param [in] filePath file path to store image.
     * @return If this image needs to count, it will return true.
     */
    virtual MBOOL setDumpInfo(MBOOL enable, string filePath) = 0;
    virtual CaptureRequestCB getCBType() = 0;
    inline void  setImgBufferErrorType(MBOOL bError) {mbError = bError;}
    inline MBOOL  getImgBufferErrorType() { return mbError; }
    inline MBOOL isNeedCaptureDoneCb() { return mParams.mbNeedCaptureDoneCb; }
    // for background service
    virtual sp<IImageBuffer> getBuffer() { return mpBuffer; }
    virtual MUINT32 getSlot() { return mParams.slot; }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ImageCallbackParams mParams;
    MBOOL mbError = MFALSE;
    // for background service
    sp<IImageBuffer> mpBuffer = nullptr;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif