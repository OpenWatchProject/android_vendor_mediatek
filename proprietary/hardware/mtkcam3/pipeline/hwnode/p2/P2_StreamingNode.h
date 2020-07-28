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

#include <mtkcam3/pipeline/hwnode/P2StreamingNode.h>

#include "P2_MWFrame.h"
#include "P2_MWFrameRequest.h"
#include "P2_DispatchProcessor.h"
#include "BaseNode.h"

using NSCam::v3::P2Common::UsageHint;
using NSCam::v3::P2StreamingNode;

#define INVALID_OPEN_ID (-1)
#define P2S_MAX_SENSOR_SUPPORT 8

namespace P2
{

class P2StreamingNodeImp : virtual public NSCam::v3::BaseNode , virtual public P2StreamingNode
{
public:
    P2StreamingNodeImp(const P2StreamingNode::ePass2Type pass2Type, const UsageHint &usageHint);
    virtual ~P2StreamingNodeImp();
    virtual NSCam::MERROR init(const IPipelineNode::InitParams &initParam);
    virtual NSCam::MERROR uninit();
    virtual NSCam::MERROR config(const P2StreamingNode::ConfigParams &configParam);
    virtual NSCam::MERROR queue(sp<IPipelineFrame> frame);
    virtual NSCam::MERROR kick();
    virtual NSCam::MERROR flush();
    virtual NSCam::MERROR flush(const sp<IPipelineFrame> &frame);

    virtual MINT32 getOpenId() const;
    virtual NodeId_T getNodeId() const;
    virtual char const* getNodeName() const;

private:
    P2Type toP2Type(P2StreamingNode::ePass2Type pass2type, const UsageHint &hint) const;
    P2UsageHint toP2UsageHint(const UsageHint &hint) const;
    MUINT32 getP2PQIndex(const P2UsageHint &usage, P2Type p2Type) const;

    MUINT32 generateFrameID();
    MBOOL parseInitParam(const ILog &log, const IPipelineNode::InitParams &initParam);
    MBOOL parseConfigParam(const P2StreamingNode::ConfigParams &configParam);
    MVOID updateConfigInfo(const sp<P2InfoObj> &p2Info, const sp<MWInfo> &mwInfo);
    MBOOL prepareFrameRequest(const ILog &log, sp<P2FrameRequest> &param, const sp<MWFrame> &frameHolder);
    MVOID printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap);

private:
    static android::Mutex sMutex;
    static android::Condition sCondition;
    static MBOOL sRunning[P2S_MAX_SENSOR_SUPPORT];

    android::Mutex mMutex;
    enum P2StreamingNodeStatus { STATUS_IDLE, STATUS_READY };
    P2StreamingNodeStatus mStatus = STATUS_IDLE;
    IPipelineNode::NodeId_T mNodeID = NULL;
    NodeName_T mNodeName = android::String8("P2_StreamingNode");

    ILog mLog;
    sp<P2InfoObj> mP2Info;
    sp<MWInfo> mMWInfo;
    sp<P2InIDMap> mInIDMap;
    DispatchProcessor mDispatcher;
    MUINT32 mFrameCount = 0;
    P2UsageHint mP2UsageHint;
    NSCam::Utils::FPSCounter mFPSCounter;

};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_NODE_H_
