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

#ifndef _MTK_CAMERA_INCLUDE_DENOISE_FEATURE_PIPE_DPE_NODE_H
#define _MTK_CAMERA_INCLUDE_DENOISE_FEATURE_PIPE_DPE_NODE_H

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"

#include <mtkcam/drv/def/dpecommon.h>
#include <mtkcam/drv/iopipe/PostProc/IDpeStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalDpePipe.h>
#include <featurePipe/core/include/WaitQueue.h>

using namespace NSCam::NSIoPipe::NSDpe;
using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const int DPE_WORKING_BUF_SET = 1;

class DPENode: public BMDeNoisePipeNode
{
public:
    DPENode() = delete;
    DPENode(const char *name, Graph_T *graph, MINT32 openId);
    virtual ~DPENode();
    virtual MBOOL onData(DataID id, ImgInfoMapPtr& data);
    virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual const char* onDumpBIDToName(BMDeNoiseBufferID BID);
    MVOID   initBufferPool();
private:
    MVOID cleanUp();
    MBOOL prepareDPEEnqueConfig(ImgInfoMapPtr pN3DImgInfo, NSIoPipe::DVEConfig& rDPEConfig, EnquedBufPool *pEnqueBufPool);
    MBOOL setupDPEBufInfo(NSIoPipe::DPEDMAPort dmaPort, SmartImageBuffer   &pSmImgBuf, NSIoPipe::DPEBufInfo& rBufInfo);
	MBOOL setupDPEBufInfo(NSIoPipe::DPEDMAPort dmaPort, SmartGraphicBuffer &pSmImgBuf, NSIoPipe::DPEBufInfo& rBufInfo);
    MBOOL setupDPEBufInfo(NSIoPipe::DPEDMAPort dmaPort, SmartImageBuffer   &pSmImgBuf, NSIoPipe::DPEBufInfo& rBufInfo, EnquedBufPool *pEnqueBufPool, BMDeNoiseBufferID bufID, MSize szActiveArea);
	MBOOL setupDPEBufInfo(NSIoPipe::DPEDMAPort dmaPort, SmartGraphicBuffer &pSmImgBuf, NSIoPipe::DPEBufInfo& rBufInfo, EnquedBufPool *pEnqueBufPool, BMDeNoiseBufferID bufID, MSize szActiveArea);

    static MVOID onDPEEnqueDone(DVEParams& rParams);
    MVOID handleDPEEnqueDone(DVEParams& rParams, EnquedBufPool* pEnqueBufPool);
    MVOID debugDPEConfig(NSIoPipe::DVEConfig& config);

private:
    // job queue
    WaitQueue<ImgInfoMapPtr>                        mJobQueue;
    MBOOL                                           mbFirst = MTRUE;
    // Last frame DMP
    SmartGraphicBuffer                              mLastDMP_L = nullptr;
    SmartImageBuffer                                mLastDMP_R = nullptr;
    // DPE stream
    IDpeStream*                                     mpDPEStream = nullptr;
    
    NSBMDN::BMBufferPool                            mBufPool;
    
    MSize DPE_DMP_SIZE;
    MSize DPE_CFM_SIZE;
    MSize DPE_RESPO_SIZE;
    // stride size
    MUINT32 DPE_DMP_STRIDE_SIZE;
    MUINT32 DPE_CFM_STRIDE_SIZE;
    MUINT32 DPE_RESPO_STRIDE_SIZE;
    MUINT32 DPE_ORG_IMG_WIDTH;
    MUINT32 DPE_ORG_IMG_HEIGHT;
    MINT32  miOpenId = -1;
};


}; //NSFeaturePipe
}; //NSCamFeature
}; //NSCam

#endif