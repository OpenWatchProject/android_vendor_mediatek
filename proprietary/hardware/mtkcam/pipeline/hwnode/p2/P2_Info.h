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

#ifndef _MTKCAM_HWNODE_P2_USAGE_HINT_H_
#define _MTKCAM_HWNODE_P2_USAGE_HINT_H_

#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>

#include "P2_Header.h"
#include "P2_Common.h"

namespace P2
{

enum P2Type { P2_UNKNOWN, P2_PREVIEW, P2_PHOTO, P2_VIDEO, P2_CAPTURE, P2_TIMESHARE_CAPTURE };

class P2UsageHint
{
public:
    P2UsageHint();

public:
    MSize mStreamingSize;
    NSCam::EIS::EisInfo mEisInfo;
    MUINT32 m3DNRMode = 0;
    MBOOL   mUseTSQ = MFALSE;
};


class P2Info : virtual public android::RefBase, virtual public ILoggerProvider
{
public:
    P2Info(MUINT32 sensorID, const MRect &activeArray, MUINT32 logLevel);
    virtual ~P2Info();

public:
    virtual Logger getLogger() const;

public:
    const MUINT32 mSensorID;
    const MRect mActiveArray;
    const Logger mLogger;
};

class P2ConfigParam
{
public:
    P2ConfigParam();
    P2ConfigParam(const P2Type &type, const P2UsageHint &usageHint);

public:
    P2Type mP2Type = P2_UNKNOWN;
    P2UsageHint mUsageHint;
};

class P2InitParam
{
public:
    P2InitParam();
    P2InitParam(const sp<P2Info> &info, const P2ConfigParam &config);

public:
    const sp<P2Info> mInfo;
    const P2ConfigParam mConfig;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_USAGE_HINT_H_
