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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_RESULTPROCESSOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_RESULTPROCESSOR_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
using namespace NSCam::v3;

using namespace android;
//
/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

class ResultProcessor
    : public virtual RefBase
    , public NSCam::v3::IPipelineBufferSetFrameControl::IAppCallback
    , public virtual NSCam::v3::NSPipelineContext::DataCallbackBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    class IListener : public virtual RefBase
    {
    public:

        virtual                     ~IListener() {};

        virtual void                onResultReceived(
                                        MUINT32         const requestNo,
                                        StreamId_T      const streamId,
                                        MBOOL           const errorResult,
                                        IMetadata       const result
                                    )                                   = 0;

        virtual void                onFrameEnd(
                                        MUINT32         const requestNo
                                    )                                   = 0;

        virtual String8             getUserName()                       = 0;

    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static sp< ResultProcessor >    createInstance();

    virtual void                    flush(MBOOL reuse = MFALSE)          = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * Register IListener.
     *
     * @param[in] streamId: streamId.
     *
     * @param[in] listener: IListener.
     *
     */
    virtual status_t                registerListener(
                                        StreamId_T      const streamId,
                                        wp< IListener > const listener
                                    )                                       = 0;

    /**
     * Register IListener.
     *
     * @param[in] startRequestNo: start request number.
     *
     * @param[in] endRequestNo: End request number.
     *
     * @param[in] listener: IListener.
     *
     */
    virtual status_t                registerListener(
                                        MUINT32         const startRequestNo,
                                        MUINT32         const endRequestNo,
                                        MBOOL           const needPartial,
                                        wp< IListener > const listener
                                    )                                       = 0;

    /**
     * Remove IListener.
     *
     * @param[in] streamId: streamId.
     *
     * @param[in] listener: IListener.
     *
     */
    virtual status_t                removeListener(
                                        StreamId_T      const streamId,
                                        wp< IListener > const listener
                                    )                                       = 0;

    /**
     * Remove IListener.
     *
     * @param[in] startRequestNo: Start request number.
     *
     * @param[in] endRequestNo: End request number.
     *
     * @param[in] listener: IListener.
     *
     */
    virtual status_t                removeListener(
                                        MUINT32         const startRequestNo,
                                        MUINT32         const endRequestNo,
                                        MBOOL           const needPartial,
                                        wp< IListener > const listener
                                    )                                       = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDataCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * for early callback.
 *
 * [2016/05/17] : P1 node support AF related infomation.
 */
public:
    class IDataListener : public virtual RefBase
    {
    public:

        virtual                     ~IDataListener() {};

        virtual void                onMetaReceived(
                                        MUINT32         const requestNo,
                                        StreamId_T      const streamId,
                                        MBOOL           const errorResult,
                                        IMetadata       const result
                                    )                                   = 0;

        virtual void                onCtrlSetting(
                                        MUINT32         const requestNo,
                                        StreamId_T      const metaAppStreamId,
                                        IMetadata&            rAppMetaData,
                                        StreamId_T      const metaHalStreamId,
                                        IMetadata&            rHalMetaData,
                                        MBOOL&                rIsChanged
                                    )                                   = 0;
    };

public:
    /**
     * Register IDataListener.
     *
     * @param[in] streamId: streamId.
     *
     * @param[in] listener: IDataListener.
     *
     */
    virtual status_t                registerListener(
                                        wp< IDataListener > const listener
                                    )                                       = 0;

    /**
     * Remove IDataListener.
     *
     * @param[in] streamId: streamId.
     *
     * @param[in] listener: IDataListener.
     *
     */
    virtual status_t                removeListener(
                                        wp< IDataListener > const listener
                                    )                                       = 0;

public:
    /**
     * Currently not support.
     */
    virtual MVOID                   onImageCallback(
                                        MUINT32                         /*requestNo*/,
                                        Pipeline_NodeId_T               /*nodeId*/,
                                        StreamId_T                      /*streamId*/,
                                        android::sp<IImageBufferHeap>&  /*rpImageBuffer*/,
                                        android::sp<IImageStreamInfo>&  /*rpStreamInfo*/,
                                        MBOOL                           /*errorResult*/
                                    ) { /*NOT support*/ }

    virtual MVOID                   onMetaCallback(
                                        MUINT32              requestNo,
                                        Pipeline_NodeId_T    nodeId,
                                        StreamId_T           streamId,
                                        IMetadata const&     rMetaData,
                                        MBOOL                errorResult
                                    )                                       = 0;

    virtual MBOOL                   isCtrlSetting() = 0;
    virtual MVOID                   onCtrlSetting(
                                        MUINT32              requestNo,
                                        Pipeline_NodeId_T    nodeId,
                                        StreamId_T const     metaAppStreamId,
                                        IMetadata&           rAppMetaData,
                                        StreamId_T const     metaHalStreamId,
                                        IMetadata&           rHalMetaData,
                                        MBOOL&               rIsChanged
                                    )                                       = 0;

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_RESULTPROCESSOR_H_

