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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_STEREO_LEGACYPIPELINE_LEGACYPIPELINEBUILDER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_STEREO_LEGACYPIPELINE_LEGACYPIPELINEBUILDER_H_

#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBufferProvider.h>
#include "ContextBuilder/MetaStreamManager.h"
#include "ContextBuilder/ImageStreamManager.h"
#include "ContextBuilder/NodeConfigDataManager.h"
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v1;

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/******************************************************************************
 *
 ******************************************************************************/

struct StereoPipelineSensorParam
{
    MUINT       mode;
    //MINT32      rawType;//not used now
    MSize       size;
    MUINT       fps;
    MUINT32     pixelMode;
};

struct StereoNodeEdge
{
    NodeId_T src;
    NodeId_T dst;
};

class StereoLegacyPipelineBuilder
        : public virtual RefBase
{
public:     ////                    for setup PipelineContext.
                                    StereoLegacyPipelineBuilder(
                                        MINT32 const openId,
                                        char* const pipeName,
                                        MINT32 const mode
                                    );
     virtual                        ~StereoLegacyPipelineBuilder();
    /**
     * set pipeline's source: sensor or memory
     *
     * @param[in] rParam: sensor parameters if use sensor as source.
     *                    or the the stream info of the source memory
     *
     */
     virtual MERROR                 setSrc(StereoPipelineSensorParam const & rParam) {
                                        mbIsSensor = MTRUE;
                                        mSrc_Sensor = rParam;
                                        return OK;
                                    }

    /**
     * create the ILegacyPipeline
     *
     * PipelineManager will hold the sp of this ILegacyPipeline.
     *
     */
     virtual sp<ILegacyPipeline>    create(
                                         sp<MetaStreamManager> spMetaStreamMng,
                                         sp<ImageStreamManager> spImageStreamMng,
                                         sp<NodeConfigDataManager> spNodeConfigDataMng
                                         );

    /**
     * set Meta Control Stream ID for APP/HAL
     *
     * PipelineBuilder will config these two stream ID to LegacyPipeline when calling create().
     *
     */
     virtual MVOID                  setMetaStreamId(
                                        StreamId const halMetaStreamId,
                                        StreamId const appMetaStreamId
                                    ){
                                        mHalMetaStreamId = halMetaStreamId;
                                        mAppMetaStreamId = appMetaStreamId;
                                    }

    /**
     * set available request types
     *
     * @param[in] pNodeConfigDataManager_default: StereoLegacyPipeline will use this node config when submitSetting is called.
     * @param[in] vNodeConfigDataManager_requests: StereoLegacyPipeline will find a proper one from these node config when
     *                                             submitRequest is called.
     */
     virtual MVOID                  setRequestTypes(
                                        sp<NodeConfigDataManager>           pNodeConfigDataManager_default,
                                        Vector< sp<NodeConfigDataManager> > vNodeConfigDataManager_requests
                                    ){
                                        mpNodeConfigDataManager_default     = pNodeConfigDataManager_default;
                                        mvNodeConfigDataManager_requests    = vNodeConfigDataManager_requests;
                                    }

protected:

    MINT32                          getOpenId() const { return mOpenId; }
    const char*                     getName() const { return mPipeName.string(); }

    MBOOL                           querySrcParam(StereoPipelineSensorParam& rParam) {
                                        if( mbIsSensor )
                                            rParam = mSrc_Sensor;
                                        return mbIsSensor;
                                    }

private:
    MINT32 const                    mOpenId;
    String8 const                   mPipeName;
    MINT32 const                    mMode;

    MBOOL                           mbIsSensor;
    StereoPipelineSensorParam       mSrc_Sensor;

    sp<MetaStreamManager>           mspMetaStreamManager;
    sp<ImageStreamManager>          mspImageStreamManager;
    sp<NodeConfigDataManager>       mspNodeConfigDataManager;

//
    StreamId                        mHalMetaStreamId;
    StreamId                        mAppMetaStreamId;

    sp<NodeConfigDataManager>           mpNodeConfigDataManager_default = nullptr;
    Vector< sp<NodeConfigDataManager> > mvNodeConfigDataManager_requests;
};
}; //namespace NSLegacyPipeline
}; //namespace v1
}; //namespace NSCam
#endif
