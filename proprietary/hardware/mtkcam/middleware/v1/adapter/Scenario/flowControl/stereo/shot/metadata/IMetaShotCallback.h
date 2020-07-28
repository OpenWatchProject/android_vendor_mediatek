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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
 * @file IMetaShotCallback.h
 * @brief MetaCallback interface.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_IMETA_SHOTCALLBACK_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_IMETA_SHOTCALLBACK_H_

// Standard C header file

// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/utils/metadata/IMetadata.h>

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
 * @class IMetaShotCallback
 * @brief MetaCallback interface.
 */
class IMetaShotCallback
    : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
/**
 * Type of Camera Stream Id.
 */
typedef MINTPTR                     StreamId_T;
// Constructor
    IMetaShotCallback() = delete;
    IMetaShotCallback(StreamId_T streamId, IMetadata metadata, MUINT32 type, MUINT32 slot = 0)
    {
        mStreamId = streamId;
        mMetadata = metadata;
        mType = type;
        mSlot = slot;
    }

protected:
// Destructor
    ~IMetaShotCallback(){}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  sendCallback Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief If result process reverses callback success message, it will send callback to user.
     * @param [in] pShotCallback shot callback to send related callback to adapter.
     * @param [in] streamId current received stream id
     * @param [in] result current result that needs to callback
     * @return The process result
     */
    virtual status_t sendCallback(
                                sp<IShotCallback> pShotCallback,
                                StreamId_T const streamId,
                                IMetadata  const result) = 0;
    /**
     * @brief If result processor receives callback fail message, it will call processFailData to notify user.
     * @param [in] reqNo failed request number.
     * @param [in] streamId failed request stream id.
     * @param [in] result failed request metadata.
     * @return The process result
     */
    virtual status_t processFailData(
                                MUINT32 const reqNo,
                                StreamId_T const streamId,
                                IMetadata  const result) = 0;
    /**
     * @brief return StreamId_T.
     * @return The process result
     */
    virtual MUINT32 getStreamId() = 0;
    virtual CaptureRequestCB getCBType() = 0;
    virtual MUINT32 getType() { return mType; };
    virtual MUINT32 getSlot() { return mSlot; }
    virtual IMetadata getMetadata() { return mMetadata; }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    StreamId_T  mStreamId   = 0;
    IMetadata   mMetadata;
    MUINT32     mType = 0;
    MUINT32     mSlot = 0;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif