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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_TPI_USAGE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_TPI_USAGE_H_

#include <mtkcam/def/common.h>
#include <map>
#include <vector>
#include "tpi/TPIMgr.h"
#include "DebugControl.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class TPIUsage
{
public:
    TPIUsage();

    MVOID config(const TPIMgr *mgr);

    MBOOL useEntry(TPIOEntry entry) const;
    TPIO getTPIO(TPIOEntry entry, MUINT32 index) const;
    MUINT32 getNodeCount(TPIOEntry entry) const;

    MBOOL use(TPIOEntry entry, TPIOUse use) const;
    EImageFormat getCustomFormat(TPIOEntry entry, EImageFormat orginal) const;
    MSize getCustomSize(TPIOEntry entry, const MSize &orginal) const;
    MUINT32 getNumInBuffer(TPIOEntry entry) const;
    MUINT32 getNumOutBuffer(TPIOEntry entry) const;

private:
    static MVOID configNodeMap(const TPI_Session &session, TPIOMap &map);

private:
    MVOID printEntry(TPIOEntry entry, const TPIOGroup &group);
    MVOID configEntry(const TPI_Session &session, TPIOMap &map, TPIOEntry entry, TPI_NODE_ID inID, TPI_NODE_ID outID);
    MVOID setUseOption(TPIOEntry entry, TPIOGroup &group);
    MVOID setBufferNum(TPIOEntry entry, TPIOGroup &group);

    static MBOOL hasNodeOption(const TPIO &io, unsigned nodeOption);
    static MBOOL hasCustomOption(const TPIO &io, unsigned customOption);

private:
    TPI_Session mSession;

    TPIOMap mNodeMap;
    TPIOGroupMap mGroupMap;
};

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_TPI_USAGE_H_
