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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_TIMER_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_TIMER_H_

#include <featurePipe/core/include/Timer.h>
#include "DebugControl.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class StreamingFeatureTimer : public Timer
{
public:
    ADD_TIMER(P2A);
    ADD_TIMER(EnqueP2A);
    ADD_TIMER(MDP);
    ADD_TIMER(VFB);
    ADD_TIMER(P2B);
    ADD_TIMER(EnqueP2B);
    ADD_TIMER(FD);
    ADD_TIMER(EIS);
    ADD_TIMER(Helper);
    ADD_TIMER(VendorProcess);
    ADD_TIMER(Vendor);
    ADD_TIMER(VMDP);
    ADD_TIMER(RSC);
    ADD_TIMER(EnqueRSC);
    ADD_TIMER(Warp);
    ADD_TIMER(EnqueWarp);
    ADD_TIMER(WarpMDP);
    ADD_TIMER(FOV);
    ADD_TIMER(FOVWarp);
    ADD_TIMER(N3DP2);
    ADD_TIMER(N3D);
    ADD_TIMER(VendorFOV);

#if defined(DEBUG_TIMER) && (DEBUG_TIMER == 1)
    ADD_TIMER(T1);
    ADD_TIMER(T2);
    ADD_TIMER(T3);
    ADD_TIMER(T4);
#endif

    StreamingFeatureTimer();
    MVOID markDisplayDone();
    MVOID markFrameDone();
    timespec getDisplayMark();
    timespec getFrameMark();
    MVOID print(MUINT32 requestNo, MUINT32 recordNo, double displayFPS = 0, double frameFPS = 0);

private:
    MBOOL mDisplayReady;
    timespec mDisplayMark;
    timespec mFrameMark;

};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_TIMER_H_
