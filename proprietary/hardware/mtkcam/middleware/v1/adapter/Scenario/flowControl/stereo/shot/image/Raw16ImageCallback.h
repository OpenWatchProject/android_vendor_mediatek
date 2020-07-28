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
 * @file Raw16ImageCallback.h
 * @brief Raw16ImageCallback interface.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_RAW16_IMETACALLBACK_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_RAW16_IMETACALLBACK_H_

// Standard C header file
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "IImageShotCallback.h"


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
 * @class ExtraDataImageCallback
 * @brief Processing extra data image callback.
 */
class Raw16ImageCallback : public IImageShotCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    Raw16ImageCallback() = delete;
    Raw16ImageCallback(ImageCallbackParams params);

protected:
// Destructor
   virtual ~Raw16ImageCallback();

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
    status_t      sendCallback(
                                sp<IShotCallback> pShotCallback,
                                StreamId_T const streamId,
                                sp<IImageBuffer>& pBuffer,
                                MBOOL bFinalImage) override;
    /**
     * @brief If result processor receives callback fail message, it will call processFailData to notify user.
     * @param [in] reqNo failed request number.
     * @param [in] streamId failed request stream id.
     * @param [in] pBuffer failed request iamge buffer.
     * @return The process result
     */
    status_t      processFailData(
                                MUINT32 const reqNo,
                                StreamId_T const streamId,
                                sp<IImageBuffer>& pBuffer) override;
    /**
     * @brief If result processor receives callback fail message, it will call processFailData to notify user.
     * @param [in] reqNo failed request number.
     * @param [in] streamId failed request stream id.
     * @param [in] pBuffer failed request iamge buffer.
     * @return The process result
     */
    status_t      dumpBuffer(sp<IImageBuffer>& pBuffer) override;
    /**
     * @brief return StreamId_T.
     * @return The process result
     */
    MUINT32 getStreamId() override;
    /**
     * @brief return mbNeedToCountForCapture in ImageCallbackParams.
     * @return If this image needs to count, it will return true.
     */
    MBOOL isNeedImageCount() override;
    /**
     * @brief set dump buffer infomation.
     * @param [in] enable enable dump buffer flag.
     * @param [in] filePath file path to store image.
     * @return If this image needs to count, it will return true.
     */
    MBOOL setDumpInfo(MBOOL enable, string filePath) override;

    CaptureRequestCB getCBType() override { return CaptureRequestCB::RAW16; }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif