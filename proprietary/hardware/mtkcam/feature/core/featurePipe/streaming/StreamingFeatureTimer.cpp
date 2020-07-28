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

#include "StreamingFeatureTimer.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "Timer"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_TIMER
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

StreamingFeatureTimer::StreamingFeatureTimer()
    : Timer()
    , mDisplayReady(MFALSE)
    , mDisplayMark()
    , mFrameMark()
{
}

MVOID StreamingFeatureTimer::markDisplayDone()
{
    if( !mDisplayReady )
    {
        mDisplayReady = MTRUE;
        mDisplayMark = Timer::getTimeSpec();
    }
}

MVOID StreamingFeatureTimer::markFrameDone()
{
    mFrameMark = Timer::getTimeSpec();
}

timespec StreamingFeatureTimer::getDisplayMark()
{
    return mDisplayMark;
}

timespec StreamingFeatureTimer::getFrameMark()
{
    return mFrameMark;
}

MVOID StreamingFeatureTimer::print(MUINT32 requestNo, MUINT32 recordNo, double displayFPS, double frameFPS)
{
    MUINT32 total       = getElapsed();
    MUINT32 p2a         = getElapsedP2A();
    MUINT32 p2aEnque    = getElapsedEnqueP2A();
    MUINT32 eis         = getElapsedEIS();
    //MUINT32 fd          = getElapsedFD();
    //MUINT32 vfb         = getElapsedVFB();
    MUINT32 warp        = getElapsedWarp();
    MUINT32 warpEnque   = getElapsedEnqueWarp();
    MUINT32 warpMDP     = getElapsedWarpMDP();
    MUINT32 mdp         = getElapsedMDP();
    //MUINT32 p2b         = getElapsedP2B();
    //MUINT32 p2bEnque    = getElapsedEnqueP2B();
    MUINT32 helper      = getElapsedHelper();
    MUINT32 vendorProcess = getElapsedVendorProcess();
    MUINT32 vendor      = getElapsedVendor();
    MUINT32 vmdp        = getElapsedVMDP();
    MUINT32 rsc         = getElapsedRSC();
    MUINT32 rscEnque    = getElapsedEnqueRSC();
    MUINT32 fov         = getElapsedFOV();
    MUINT32 fovWarp     = getElapsedFOVWarp();
    MUINT32 vendorFOV   = getElapsedVendorFOV();

    MY_LOGD("Frame timer [#%5d/%4d][t%4d][a%4d/%4d][v%4d/%4d/%4d][h%4d][e%4d][r%4d/%4d][f%4d/%4d][w%4d/%4d/%4d][m%4d][fps%05.2f/%05.2f]",
            requestNo, recordNo, total, p2aEnque, p2a, vendorProcess, vendor, vmdp, helper, eis, rscEnque, rsc, fov, fovWarp, warpEnque, warpMDP, warp, mdp, displayFPS, frameFPS);

#if defined(DEBUG_TIMER) && (DEBUT_TIMER == 1)
    {
        MUINT32 t1 = getElapsedT1();
        MUINT32 t2 = getElapsedT2();
        MUINT32 t3 = getElapsedT3();
        MUINT32 t4 = getElapsedT4();
        MY_LOGD("Frame [t1%5d][t2%5d][t3%5d][t4%5d]",
                requestNo, t1, t2, t3, t4);
    }
#endif
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
