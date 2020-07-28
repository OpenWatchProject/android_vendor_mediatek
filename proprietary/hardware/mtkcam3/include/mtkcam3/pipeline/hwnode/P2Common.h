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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2COMMON_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2COMMON_H_

#include <mtkcam/utils/std/common.h>
#include <map>
#include <vector>

#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>

#include <mtkcam3/3rdparty/plugin/TPICustomInfo.h>

namespace NSCam {
namespace v3 {
namespace P2Common {

    enum P2NodeType
    {
        P2_NODE_UNKNOWN,
        P2_NODE_COMMON,
    };

    enum AppMode
    {
        APP_MODE_UNKNOWN,
        APP_MODE_PHOTO,
        APP_MODE_VIDEO,
        APP_MODE_HIGH_SPEED_VIDEO,
        APP_MODE_BATCH_SMVR,
    };

    enum eCustomOption
    {
        CUSTOM_OPTION_NONE                = 0,
        CUSTOM_OPTION_SENSOR_4CELL        = 1 << 0,
        CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT  = 1 << 1,
        CUSTOM_OPTION_DRE_SUPPORT         = 1 << 2,
        CUSTOM_OPTION_HFG_SUPPORT         = 1 << 3,
        CUSTOM_OPTION_PQ_SUPPORT          = 1 << 4,
    };

    struct UsageHint
    {
    public:
        class OutConfig
        {
        public:
            MUINT32 mMaxOutNum = 2;// max out buffer num in 1 pipeline frame for 1 sensor
            MBOOL mHasPhysical = MFALSE;
            MBOOL mHasLarge = MFALSE;
            MSize mFDSize;
            MSize mVideoSize;
        };
        class InConfig
        {
        public:
            enum Type
            {
                RAW_IN,
                P1YUV_IN,
            };
            Type    mType = RAW_IN;
            MUINT32 mReqFps = 30;
        };
        P2NodeType mP2NodeType = P2_NODE_UNKNOWN;
        AppMode mAppMode = APP_MODE_UNKNOWN;
        MSize mStreamingSize;
        std::vector<MSize> mOutSizeVector;
        MUINT32 mDsdnHint = 0;
        MUINT64 mPackedEisInfo = 0;
        MUINT32 m3DNRMode = 0;
        MUINT32 mFSCMode = 0;
        MUINT32 mDualMode = 0;
        MUINT32 mSMVRSpeed = 1;
        MBOOL   mUseTSQ = MFALSE;
        MBOOL   mEnlargeRsso = MFALSE;
        MUINT64 mTP = 0;
        // mTPMarginRatio: Fixed TPI margin ratio.
        // 1.2 means output * 1.2 = final rrzo, final rrz / 1.2 = available output area
        MFLOAT  mTPMarginRatio = 1.0f;
        IMetadata mAppSessionMeta;
        OutConfig mOutCfg;
        InConfig  mInCfg;
        NSPipelinePlugin::TPICustomConfig mTPICustomConfig;
        // for multi-cam
        std::map<MUINT32, MSize> mResizedRawMap;
        MUINT32 mDualFeatureMode = 0;
        MUINT32 mSensorModule = 0;

        NSCam::SecType mSecType = NSCam::SecType::mem_normal;
        MBOOL mHasVideo = MFALSE;
    };

namespace Capture
{
    struct UsageHint
    {
        MUINT64 mSupportedScenarioFeatures = ~0;
        MBOOL   mIsSupportedBGPreRelease   = MFALSE;
        MUINT32 mDualFeatureMode = 0;
        MINT32  mPluginUniqueKey = 0;
        MBOOL   mIsHidlIsp = MFALSE;
    };
}


}; // namespace P2Common
}; //namespace v3
}; //namespace NSCam

#endif //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2COMMON_H_
