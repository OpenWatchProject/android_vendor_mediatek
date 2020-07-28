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

#ifndef _MTKCAM_HWNODE_P2_NODE_H_
#define _MTKCAM_HWNODE_P2_NODE_H_

#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>

#include "P2_MWFrame.h"
#include "P2_MWFrameRequest.h"
#include "P2_DispatchProcessor.h"

using NSCam::v3::P2FeatureNode;
using NSCam::v3::P2Node;

namespace P2
{

class P2_Node : virtual public P2FeatureNode , virtual public P2Node
{
public:
    P2_Node(const P2FeatureNode::ePass2Type pass2Type, const P2FeatureNode::UsageHint &usageHint);
    P2_Node(const P2Node::ePass2Type pass2Type, const P2Common::UsageHint &usageHint);
    virtual ~P2_Node();
    virtual NSCam::MERROR init(const IPipelineNode::InitParams &initParam);
    virtual NSCam::MERROR uninit();
    virtual NSCam::MERROR config(const P2FeatureNode::ConfigParams &configParam);
    virtual NSCam::MERROR config(const P2Node::ConfigParams &configParam);
    virtual NSCam::MERROR queue(sp<IPipelineFrame> frame);
    virtual NSCam::MERROR kick();
    virtual NSCam::MERROR flush();
    virtual NSCam::MERROR flush(const sp<IPipelineFrame> &frame);
    virtual NSCam::MERROR setNodeCallBack(android::wp<INodeCallbackToPipeline> pCallback) { return MTRUE; }

    virtual MINT32 getOpenId() const;
    virtual NodeId_T getNodeId() const;
    virtual char const* getNodeName() const;

private:
    P2Type toP2Type(P2FeatureNode::ePass2Type pass2type, const P2FeatureNode::UsageHint &hint) const;
    P2UsageHint toP2UsageHint(const P2FeatureNode::UsageHint &hint) const;
    P2Type toP2Type(P2Node::ePass2Type pass2type, const P2Common::UsageHint &hint) const;
    P2UsageHint toP2UsageHint(const P2Common::UsageHint &hint) const;

    MUINT32 generateFrameID();
    MBOOL parseInitParam(const IPipelineNode::InitParams &initParam, const Logger &logger);
    MBOOL parseConfigParam(const P2FeatureNode::ConfigParams &configParam);
    MBOOL parseConfigParam(const P2Node::ConfigParams &configParam);
    MBOOL updateConfig();
    MBOOL prepareFrameRequest(sp<P2FrameRequest> &param, const sp<MWFrame> &frameHolder, const Logger &logger);
    MBOOL setupRedirect(MBOOL wantRedirect);
    MBOOL needRedirect(const sp<IPipelineFrame> &frame);
    MBOOL isCapture(const sp<IPipelineFrame> &frame);

private:
    enum P2_RedirectMode { REDIRECT_ALWAYS, REDIRECT_NEVER, REDIRECT_SELECT };

private:
    android::Mutex mMutex;
    enum P2_NodeStatus { STATUS_IDLE, STATUS_READY };
    P2_NodeStatus mStatus;
    IPipelineNode::NodeId_T mNodeID;
    NodeName_T mNodeName;
    sp<P2Info> mP2Info;
    P2ConfigParam mConfigParam;
    sp<MWInfo> mMWInfo;
    DispatchProcessor mDispatcher;
    MUINT32 mFrameCount;
    P2UsageHint mP2UsageHint;
    MUINT32 mLogLevel;

    P2_RedirectMode mRedirectMode;
    sp<P2Node> mRedirectP2;
    sp<P2FeatureNode> mRedirectP2F;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_NODE_H_
