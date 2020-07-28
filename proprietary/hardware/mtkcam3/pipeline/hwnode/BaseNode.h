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
#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_BASENODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_BASENODE_H_

#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>

#include <mtkcam3/pipeline/hwnode/NodeId.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

/******************************************************************************
 *
 ******************************************************************************/
class BaseNode
    : public virtual IPipelineNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:     ////                 Data Members.
    MINT32                          mOpenId;
    IPipelineNode::NodeId_T         mNodeId;
    android::String8                mNodeName;
    typedef unsigned int            ModuleId;

private:
    MINT32                          mLogLevel;

protected:     ////                 Operations.
    virtual MERROR                  ensureMetaBufferAvailable_(
                                        MUINT32 const frameNo,
                                        StreamId_T const streamId,
                                        IStreamBufferSet& rStreamBufferSet,
                                        android::sp<IMetaStreamBuffer>& rpStreamBuffer,
                                        MBOOL acquire = MTRUE
                                    );

    virtual MERROR                  ensureImageBufferAvailable_(
                                        MUINT32 const frameNo,
                                        StreamId_T const streamId,
                                        IStreamBufferSet& rStreamBufferSet,
                                        android::sp<IImageStreamBuffer>& rpStreamBuffer,
                                        MBOOL acquire = MTRUE
                                    );

    virtual MVOID                   onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    virtual MVOID                   onEarlyCallback(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        IMetadata const& rMetaData,
                                        MBOOL error = MFALSE
                                    );

    virtual MVOID                   onCtrlSetting(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const metaAppStreamId,
                                        IMetadata& rAppMetaData,
                                        StreamId_T const metaHalStreamId,
                                        IMetadata& rHalMetaData,
                                        MBOOL& rIsChanged
                                    );

    virtual MVOID                   onCtrlSync(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        MUINT32 index,
                                        MUINT32 type,
                                        MINT64 duration
                                    );

    virtual MVOID                   onCtrlResize(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const metaAppStreamId,
                                        IMetadata& rAppMetaData,
                                        StreamId_T const metaHalStreamId,
                                        IMetadata& rHalMetaData,
                                        MBOOL& rIsChanged
                                    );

    virtual MVOID                   onCtrlReadout(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const metaAppStreamId,
                                        IMetadata& rAppMetaData,
                                        StreamId_T const metaHalStreamId,
                                        IMetadata& rHalMetaData,
                                        MBOOL& rIsChanged
                                    );

    virtual MBOOL                   needCtrlCb(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        IPipelineNodeCallback::eCtrlType eType
                                    );

    virtual MVOID                   onNextCaptureCallBack(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        MUINT32   requestCnt = 0,
                                        MBOOL     bSkipCheck = MFALSE
                                    );

    virtual MERROR                  setNodeCallBack(
                                        android::wp<INodeCallbackToPipeline> pCallback
                                    );

    virtual MERROR                  kick();

    virtual MERROR                  flush(android::sp<IPipelineFrame> const &pFrame);

    virtual MERROR                  triggerdb(TriggerDB const& arg);

    virtual std::string             getStatus();

public:     ////                    Operations.
                                    BaseNode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual MINT32                  getOpenId() const;

    virtual IPipelineNode::NodeId_T getNodeId() const;

    virtual char const*             getNodeName() const;
    virtual ModuleId                getULogModuleId();
};

};
};

#endif
