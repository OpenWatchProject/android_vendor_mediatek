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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
 * @file BaseCaptureRequestBuilder.h
 * @brief Base capture request builder, implement some useful function.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_BASE_SWBOKEH_POSTPROCESS_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_BASE_SWBOKEH_POSTPROCESS_REQUEST_BUILDER_H_

// Standard C header file

// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
// Local header file
#include "IPostProcessRequestBuilder.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace std;
using namespace android;
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class SwBokehPostProcessRequestBuilder
 * @brief sw bokeh Post processor request builder
 */
class SwBokehPostProcessRequestBuilder
    : public IPostProcessRequestBuilder
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    SwBokehPostProcessRequestBuilder() = default;

protected:
// Destructor
    virtual ~SwBokehPostProcessRequestBuilder();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  implement IPostProcessRequestBuilder
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init PostProc request builder
     * @param [in] setting using initial setting to initialize PostProc request builder.
     * @return return initial status
     */
    status_t init(PostProcRequestInitSetting &setting) override final;
    /**
     * @brief get PostProc frame
     * @param [in] setting PostProc pipeline setting
     * @param [in] frame pieline frame
     * @return return create pipeline frame status, if return status not equal OK, don't use.
     */
    status_t doPostProc() override final;
    /**
     * @brief destroy PostProc request builder.
     * @return return destroy status.
     */
    status_t destroy() override final;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// overrided RefBase
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID                               onLastStrongRef( const void* /*id*/);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    //
    MINT32                              miRequestNo = -1;
    //
    MINT64                              miTimeStamp = -1;
    //
    sp<BufferCallbackHandler>           mpCallbackBufferHandler = nullptr;
    //
    wp<ILegacyPipeline>                 mpILegacyPipeline = nullptr;
    //
    sp<CallbackImageBufferManager>      mpCallbackImageBufferManager = nullptr;
    //
    sp<IImageCallback>                  mpImageCallback = nullptr;
    //
    sp<NodeConfigDataManager>           mpNodeConfigDataManager = nullptr;
    //
    ShotParam           mShotParam;
    JpegParam           mJpegParam;
    BufferList          mvDstStreams;

    IMetadata           mAppMetadata;
    IMetadata           mHalMetadata;
    MINT32              miTargetPipelineId = -1;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif
