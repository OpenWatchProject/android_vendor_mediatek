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


// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "DepthMapPipeTimer.h"
#include "DepthMapEffectRequest.h"

// Log header file
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DepthMapPipeTimer"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

MVOID
DepthMapPipeTimer::
showPerFrameSummary(
    MUINT32 requestNo,
    const EffectRequestAttrs& attr
)
{
    MUINT32 p2a = getElapsedP2A();
    MUINT32 p2aDrv = getElapsedP2ADrv();
    MUINT32 p2aSetIsp = getElapsedP2ASetIsp();
    MUINT32 p2ab = getElapsedP2ABayer();
    MUINT32 p2abSetIsp = getElapsedP2ABayerSetIsp();
    MUINT32 p2abDrv = getElapsedP2ABayerDrv();
    MUINT32 p2abMYS_Resize = getElapsedP2ABayerMYSResize();
    MUINT32 p2abN3DWait = getElapsedP2ABayerN3DWaiting();
    MUINT32 perFrame = p2a + p2ab;
    MUINT32 overall = getElapsedOverall();


    if(attr.opState == eSTATE_STANDALONE)
        MY_LOGD("[DepthMapPipe][per-frame] reqID=%d, standalone req, P2ANode:%d[drv:%d/setIsp:%d] P2ABayer:%d[drv:%d/mys:%d/waitn3d:%d/setIsp:%d] total:%d[%d]", requestNo, p2a, p2aDrv, p2aSetIsp, p2ab, p2abDrv, p2abMYS_Resize, p2abN3DWait, p2abSetIsp, overall, perFrame);
    else if(attr.opState == eSTATE_NORMAL)
        MY_LOGD("[DepthMapPipe][per-frame] reqID=%d, normal req, fefm:%d, P2ANode:%d[drv:%d/setIsp:%d] P2ABayer:%d[drv:%d/mys:%d/waitn3d:%d/setIsp:%d] total:%d[%d]", requestNo, attr.needFEFM, p2a, p2aDrv, p2aSetIsp, p2ab, p2abDrv, p2abMYS_Resize, p2abN3DWait, p2abSetIsp, overall, perFrame);
    else if(attr.opState == eSTATE_CAPTURE)
    {
        MUINT32 n3d = getElapsedN3D();
        MUINT32 n3dMain1Padding = getElapsedN3DMain1Padding();
        MUINT32 n3dMain2PMask= getElapsedN3DMaskWarping();
        MUINT32 n3dLearning = getElapsedN3DLearning();
        MUINT32 dpe = getElapsedDPE();
        MUINT32 dpeDrv = getElapsedDPEDrv();
        MUINT32 wpe = getElapsedWPE();
        MUINT32 wpeDrv = getElapsedWPEDrv();
        MUINT32 dldepth = getElapsedDLDepth();
        MUINT32 dldepthAlgo = getElapsedDLDepthAlgo();

        MUINT32 perFrame = p2a + p2ab + n3d + dpe + wpe + dldepth;
        MY_LOGD("[DepthMapPipe][per-frame] reqID=%d capture request, P2A:%d[drv:%d] N3D:%d[mask2:%d/learn:%d], WPE:%d[drv:%d] DLDepth:%d[algo:%d] total=%d[%d]",
        requestNo, p2a, p2aDrv, n3d, n3dMain2PMask, n3dLearning, wpe, wpeDrv, dldepth, dldepthAlgo, overall, perFrame);
    }
}

MVOID
DepthMapPipeTimer::
showTotalSummary(
    MUINT32 requestNo,
    const EffectRequestAttrs& attr
)
{
    MUINT32 p2a = getElapsedP2A();
    MUINT32 p2aSetIsp = getElapsedP2ASetIsp();
    MUINT32 p2aDrv = getElapsedP2ADrv();
    MUINT32 p2ab = getElapsedP2ABayer();
    MUINT32 p2abSetIsp = getElapsedP2ABayerSetIsp();
    MUINT32 p2abDrv = getElapsedP2ABayerDrv();
    MUINT32 p2abMYS_Resize = getElapsedP2ABayerMYSResize();
    MUINT32 p2abN3DWait = getElapsedP2ABayerN3DWaiting();
    MUINT32 n3d = getElapsedN3D();
    MUINT32 n3dMain1Padding = getElapsedN3DMain1Padding();
    MUINT32 n3dMain2PMask= getElapsedN3DMaskWarping();
    MUINT32 n3dLearning = getElapsedN3DLearning();
    MUINT32 dpe = getElapsedDPE();
    MUINT32 dpeDrv = getElapsedDPEDrv();
    MUINT32 gf = getElapsedGF();
    MUINT32 gfAlgo = getElapsedGFALGO();
    MUINT32 wpe = getElapsedWPE();
    MUINT32 wpeDrv = getElapsedWPEDrv();
    MUINT32 dldepth = getElapsedDLDepth();
    MUINT32 dldepthAlgo = getElapsedDLDepthAlgo();

    MUINT32 total = p2a + p2ab + n3d + dpe + gf + wpe;

    if(attr.opState == eSTATE_STANDALONE)
        MY_LOGD("[DepthMapPipe][overall] reqID=%d, standalone req, P2A:%d[drv:%d/setIsp:%d] P2ABayer:%d[drv:%d/setIsp:%d] N3D:%d[pad:%d/mask2:%d], total:%d",
        requestNo, p2a, p2aDrv, p2aSetIsp, p2ab, p2abDrv, p2abSetIsp, total);
    else if(attr.opState == eSTATE_NORMAL)
        MY_LOGD("[DepthMapPipe][overall] reqID=%d normal request, fefm:%d P2A:%d[drv:%d/setIsp:%d] P2ABayer:%d[drv:%d/mys:%d/waitn3d:%d/setIsp:%d] N3D:%d[pad:%d/mask2:%d] N3DLearning:%d, WPE:%d[drv:%d] DPE:%d[drv:%d] GF:%d[algo:%d] total:%d",
        requestNo, attr.needFEFM, p2a, p2aDrv, p2aSetIsp, p2ab, p2abDrv, p2abMYS_Resize, p2abN3DWait, p2abSetIsp, n3d, n3dMain1Padding, n3dMain2PMask, n3dLearning,
        wpe, wpeDrv, dpe, dpeDrv, gf, gfAlgo, total);
    else if(attr.opState == eSTATE_CAPTURE)
    {
        MUINT32 total = p2a + p2ab + n3d + dpe + dldepth + wpe;
        MY_LOGD("[DepthMapPipe][per-frame][overall] reqID=%d capture request, P2A:%d[drv:%d] N3D:%d[mask2:%d/learn:%d], WPE:%d[drv:%d] DPE:%d[drv:%d] DLDepth:%d[algo:%d] total:%d",
        requestNo, p2a, p2aDrv, n3d, n3dMain2PMask, n3dLearning,
        wpe, wpeDrv, dpe, dpeDrv, dldepth, dldepthAlgo, total);
    }

}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam