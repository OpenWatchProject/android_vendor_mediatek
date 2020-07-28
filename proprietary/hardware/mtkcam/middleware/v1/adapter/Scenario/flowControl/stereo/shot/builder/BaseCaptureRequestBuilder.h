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
 * @file BaseCaptureRequestBuilder.h
 * @brief Base capture request builder, implement some useful function.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_BASE_CAPTURE_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_BASE_CAPTURE_REQUEST_BUILDER_H_

// Standard C header file

// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
// Local header file
#include "ICaptureRequestBuilder.h"

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
 * @class BaseCaptureRequestBuilder
 * @brief B+B capture request builder.
 */
class BaseCaptureRequestBuilder
    : public ICaptureRequestBuilder
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    BaseCaptureRequestBuilder();

protected:
// Destructor
    virtual ~BaseCaptureRequestBuilder();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Basic operation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual status_t onInit(
                    CaptureRequestInitSetting &setting);
    virtual status_t onPrepareImageBufferProvider();
    virtual status_t onDestory();
    virtual status_t onUpdateMetadata(
                    IMetadata* appMeta,
                    IMetadata* halMeta,
                    IMetadata* main2AppMeta,
                    IMetadata* main2HalMeta);
    virtual status_t onSetImageCallback();
    virtual status_t onSetMetaCallback();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  implement ICaptureRequestBuilder
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init capture request builder
     * @param [in] setting using initial setting to initialize capture request builder.
     * @return return initial status
     */
    status_t init(
                                CaptureRequestInitSetting &setting) override final;
    /**
     * @brief get capture frame
     * @param [in] setting capture pipeline setting
     * @param [in] frame pieline frame
     * @return return create pipeline frame status, if return status not equal OK, don't use.
     */
    status_t doCapture(
                                CaptureSetting &setting) override final;
    /**
     * @brief destroy capture request builder.
     * @return return destroy status.
     */
    status_t destroy() override final;
protected:
    /**
     * @brief Build node config manager. Node config manager is used to decide IO map.
     * @param [in] content contain ImageTbl, MetaTbl and node connect table.
     * @return return build status.
     */
    status_t buildNodeConfigManager(
                                            ContextBuilderContent content) override;
    /**
     * @brief get selector from IResourceContain by specific open id and stream id.
     * @param [in] openId open id to get ResourceContainer.
     * @param [in] streamId stream id to get buffer provider from ResourceContainer.
     * @return return selector
     */
    sp<ISelector> getSelector(
                                            MUINT32 openId,
                                            StreamId_T streamId) override;
    /**
     * @brief get buffer data from selector.
     * @param [in] selector get metadata and image buffer from selector.
     * @return return buffer data.
     */
    std::shared_ptr<BufferData> 
                  getBufferDataFromSelect(
                                            sp<ISelector> selector,
                                            MINT32 const iAppMetadataStreamId,
                                            MINT32 const iHalMetadataStreamId) override;
    /**
     * @brief prepare metadata
     * @param [in] vBufferDataList buffer data list.
     * @return return buffer data.
     */
    status_t prepareMetadata(
                                            IMetadata* main1AppMeta,
                                            IMetadata* main1HalMeta,
                                            IMetadata* main2AppMeta,
                                            IMetadata* main2HalMeta) override;
    /**
     * @brief prepare image provider for capture frame.
     * @return return allocate status
     */
    status_t prepareImageProvider() override;
    /**
     * @brief get timestamp from metadata
     * @param [in] metadata hal meta data.
     * @return return process result.
     */
    status_t getTimestamp(
                            IMetadata *metadata,
                            MINT64 &timestamp) override;
    /**
     * @brief send time stamp to provider
     * @param [in] timestamp time stamp
     * @return return process result.
     */
    status_t sendTimestampToProvider(
                            MUINT32 reqNo,
                            MINT64 timestamp) override;
    /**
     * @brief set image callback
     * @return return process result.
     */
    status_t setImageCallback() override;
    /**
     * @brief set metadata callback
     * @return return process result.
     */
    status_t setMetadataCallback() override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// overrided RefBase
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID                               onLastStrongRef( const void* /*id*/);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    //
    MINT32                              miCaptureNo = -1;
    //
    MINT64                              miTimeStamp = -1;
    //
    sp<BufferCallbackHandler>           mpCallbackBufferHandler = nullptr;
    //
    wp<IParamsManagerV3>                mpParamMgrV3 = nullptr;
    //
    wp<ILegacyPipeline>                 mpILegacyPipeline = nullptr;
    //
    sp<CallbackImageBufferManager>      mpCallbackImageBufferManager = nullptr;
    //
    sp<ShotCallbackProcessor>           mpShotCallbackProcessor = nullptr;
    //
    Vector<sp<IMetaShotCallback> >      mvMetadataCallbackList;
    //
    Vector<sp<IImageShotCallback> >     mvImageCallbackList;
    //
    sp<NodeConfigDataManager>           mpNodeConfigDataManager = nullptr;
    //
    MVOID*                              mpUserData = nullptr;
    //
    StereoShotParam     mShotParams;
    BufferList mvDstStreams;
    IMetadata mAppMetadata;
    IMetadata mHalMetadata;
    MINT32              miMain1Id = -1;
    MINT32              miMain2Id = -1;
    MINT32              miTargetPipelineId = -1;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // table
    ContextBuilderContent mContent;
    //
protected:
    IMetadata*          mMain1AppMetadata = nullptr;
    IMetadata*          mMain1HalMetadata = nullptr;
    IMetadata*          mMain2AppMetadata = nullptr;
    IMetadata*          mMain2HalMetadata = nullptr;
    //
    MRect               mCropSize_Main1;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif