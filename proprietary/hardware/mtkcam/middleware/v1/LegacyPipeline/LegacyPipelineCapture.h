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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINECAPTURE_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINECAPTURE_H_
//
#include "MyUtils.h"
#include "LegacyPipelineBase.h"

using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class LegacyPipelineCapture
    : public LegacyPipelineBase
{
public:
    struct Configuration
    {
        // in
        sp<IMetaStreamInfo>                 pControlMeta_App;
        sp<IMetaStreamInfo>                 pControlMeta_Hal;
        sp<IImageStreamInfo>                pImage_ImgoRawSrc;
        //
        // out
        sp<IMetaStreamInfo>                 pResultMeta_P1_App;
        sp<IMetaStreamInfo>                 pResultMeta_P1_Hal;
        sp<IMetaStreamInfo>                 pResultMeta_P2_App;
        sp<IMetaStreamInfo>                 pResultMeta_P2_Hal;
        sp<IMetaStreamInfo>                 pResultMeta_Jpeg_App;
        //
        sp<IImageStreamInfo>                pImage_RrzoRaw;
        sp<IImageStreamInfo>                pImage_ImgoRaw;
        sp<IImageStreamInfo>                pImage_LcsoRaw;
        sp<IImageStreamInfo>                pImage_Yuv00;
        sp<IImageStreamInfo>                pImage_Yuv01;
        sp<IImageStreamInfo>                pImage_YuvJpeg;
        sp<IImageStreamInfo>                pImage_YuvThumbnail;
        sp<IImageStreamInfo>                pImage_Jpeg;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        LegacyPipelineCapture(MINT32 openId);

                                        ~LegacyPipelineCapture();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ILegacyPipeline Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual MERROR                      waitUntilDrainedAndFlush();
    virtual MERROR                      waitUntilP1DrainedAndFlush();
    virtual MERROR                      waitUntilP2JpegDrainedAndFlush();

    virtual MERROR                      submitSetting(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            ResultSet* pResultSet
                                        );

    virtual MERROR                      submitRequest(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            Vector<BufferSet> vDstStreams,
                                            ResultSet* pResultSet
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //// for legacy pipeline builder

    MVOID                               setConfiguration(
                                            Configuration const& config
                                        );

protected:
    template <typename _Node_>
    MERROR                              waitUntilNodeDrainedAndFlush(
                                            NodeId_T const nodeId,
                                            android::sp< NodeActor<_Node_> >& pNodeActor
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    Configuration                       mConfiguration;
};


};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINECAPTURE_H_

