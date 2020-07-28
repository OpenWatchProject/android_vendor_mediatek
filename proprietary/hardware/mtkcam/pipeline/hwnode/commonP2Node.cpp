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

#include <mtkcam/pipeline/hwnode/P2Node.h>

#ifdef MTKCAM_SUPPORT_COMMON_P2NODE
#include "p2/P2_Node.h"
#endif // MTKCAM_SUPPORT_COMMON_P2NODE

#define KEY_COMMON_P2             "debug.mtkcam.p2.common"
#define VAL_COMMON_P2_AUTO        0
#define VAL_COMMON_P2_STREAMING   1
#define VAL_COMMON_P2_DISABLE     2
#define VAL_COMMON_P2_VSS         3

#define USE_COMMON_P2F MFALSE

#define LOG_TAG "MtkCam/commonP2"
#include <mtkcam/utils/std/Log.h>

namespace NSCam {
namespace v3 {

#ifndef MTKCAM_SUPPORT_COMMON_P2NODE

android::sp<P2Node> P2Node::createInstance(const P2Node::ePass2Type pass2type, P2Common::UsageHint usageHint)
{
    return P2Node::createInstance(pass2type);
}

#else

MBOOL useCommonP2(MBOOL hint, MBOOL isStreaming)
{
    MBOOL useCommon = hint;
    MUINT32 commonP2 = property_get_int32(KEY_COMMON_P2, VAL_COMMON_P2_AUTO);
    switch( commonP2 )
    {
    case VAL_COMMON_P2_STREAMING: useCommon = isStreaming;  break;
    case VAL_COMMON_P2_DISABLE:   useCommon = MFALSE;       break;
    case VAL_COMMON_P2_VSS:       useCommon = MTRUE;        break;
    default: break;
    }

    CAM_LOGI("hint=%d streaming=%d adb=%d result=%d", hint, isStreaming, commonP2, useCommon);

    return useCommon;
}

sp<P2Node> P2Node::createInstance(const P2Node::ePass2Type type, P2Common::UsageHint usage)
{
    if( useCommonP2(usage.mP2NodeType != P2Common::P2_NODE_UNKNOWN, type == P2Node::PASS2_STREAM) )
    {
        return new P2::P2_Node(type, usage);
    }
    else
    {
        return P2Node::createInstance(type);
    }
}

sp<P2FeatureNode> P2FeatureNode::createInstance(const P2FeatureNode::ePass2Type type, const P2FeatureNode::UsageHint usage)
{
    if( useCommonP2(USE_COMMON_P2F, type == P2FeatureNode::PASS2_STREAM) )
    {
        return new P2::P2_Node(type, usage);
    }
    else
    {
        return P2FeatureNode::createHal1Instance(type, usage);
    }
}

#endif // MTKCAM_SUPPORT_COMMON_P2NODE

} // namespace v3
} // namespace NSCam
