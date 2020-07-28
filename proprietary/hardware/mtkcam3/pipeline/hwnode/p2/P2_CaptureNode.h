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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTKCAM_HWNODE_P2_CAPTURENODE_H_
#define _MTKCAM_HWNODE_P2_CAPTURENODE_H_

#include <mtkcam3/pipeline/hwnode/P2CaptureNode.h>

#include "P2_Common.h"
#include "P2_MWFrame.h"
#include "P2_MWFrameRequest.h"
#include "P2_CaptureProcessor.h"
#include "P2_DrawIDPlugin.h"
#include "BaseNode.h"
#include <mtkcam/utils/std/ULog.h>

using NSCam::v3::P2Common::Capture::UsageHint;
using NSCam::v3::P2CaptureNode;

/******************************************************************************
 *
 ******************************************************************************/
namespace P2 {

/******************************************************************************
 *
 ******************************************************************************/
class P2CaptureNodeImp
    : virtual public NSCam::v3::BaseNode, public virtual P2CaptureNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    P2CaptureNodeImp(ePass2Type const type, UsageHint const &usageHint);

    ~P2CaptureNodeImp();

    virtual NSCam::MERROR config(ConfigParams const &rParams);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual NSCam::MERROR init(InitParams const &rParams);

    virtual NSCam::MERROR uninit();

    virtual NSCam::MERROR flush();

    virtual NSCam::MERROR flush(
            sp<IPipelineFrame> const &pFrame
    );

    virtual NSCam::MERROR queue(
            sp<IPipelineFrame> pFrame
    );

	virtual std::string  getStatus();

    virtual MUINT32 getULogModuleId();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    NSCam::MERROR verifyConfigParams(
            ConfigParams const &rParams
    ) const;
private:
    P2Type toP2Type(ePass2Type pass2type) const;

    MBOOL parseInitParam(const ILog &log, const IPipelineNode::InitParams &rParam);
    MBOOL parseConfigParam(const ConfigParams &rParam);
    MVOID updateConfigInfo(const sp<P2InfoObj> &p2Info, const sp<MWInfo> &mwInfo);
    sp<P2FrameRequest> createFrameRequest(const ILog &log, const sp<MWFrame> &frameHolder);
    MVOID printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap);


private:
    android::Mutex  mMutex;
    enum P2CaptureNodeStatus { STATUS_IDLE, STATUS_READY };
    P2CaptureNodeStatus mStatus = STATUS_IDLE;

    ILog mLog;
    sp<P2InfoObj>   mP2Info;
    sp<MWInfo>      mMWInfo;
    sp<P2InIDMap>   mInIDMap;
    sp<P2DrawIDPlugin> mDrawIDPlugin;
    MUINT32         mRequestNo;
    CaptureProcessor mCaptureProcessor;

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace P2

#endif // _MTKCAM_HWNODE_P2_CAPTURENODE_H_
