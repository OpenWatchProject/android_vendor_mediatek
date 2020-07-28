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
#ifndef _MTK_CAMERA_VSDOF_DCMF_FEATURE_PIPE_BSS_NODE_H_
#define _MTK_CAMERA_VSDOF_DCMF_FEATURE_PIPE_BSS_NODE_H_

#include "DualCamMFPipe_Common.h"
#include "DualCamMFPipeNode.h"

#include <MTKBss.h>

using namespace NS3Av3;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{
namespace NSDCMF{

class BssNode : public DualCamMFPipeNode
{
    /*
    *  Tuning Param for BSS ALG. Should not be configure by customer
    */
    static const int MF_BSS_ON = 1;
    static const int MF_BSS_VER = 2;
    static const int MF_BSS_ROI_PERCENTAGE = 95;

    /*
    *  Tuning Param for EIS. Should not be configure by customer
    */
    static const int MFC_GMV_CONFX_TH = 25;
    static const int MFC_GMV_CONFY_TH = 25;
    static const int MAX_GMV_CNT = 12;

    struct GMV{
        MINT32 x = 0;
        MINT32 y = 0;
    };
    public:
        BssNode() = delete;
        BssNode(const char *name, Graph_T *graph, MINT32 openId);
        virtual ~BssNode();

        virtual MBOOL onData(DataID id, PipeRequestPtr &request);
        virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
        MVOID   initBufferPool();
    private:
        // routines
        MVOID cleanUp();

        MVOID addPendingRequests(PipeRequestPtr request);

        MVOID getReadyData(list<PipeRequestPtr>& rvToDoRequests);

        MBOOL retrieveGmvInfo(IMetadata* pMetadata, int& x, int& y, MSize& size) const;

        GMV   calMotionVector(IMetadata* pMetadata, MBOOL isMain) const;

        MVOID updateBssProcInfo(IImageBuffer* pBuf, BSS_PARAM_STRUCT& p, MINT32 frameNum) const;

        MVOID updateBssIOInfo(IImageBuffer* pBuf, BSS_INPUT_DATA_G& bss_input) const;

        MBOOL appendBSSInput(MINT32 idx, PipeRequestPtr& request, BSS_INPUT_DATA_G& bss_input) const;

        MBOOL doBss(list<PipeRequestPtr>& rvToDoRequests);

        MVOID collectPreBSSExifData(
            list<PipeRequestPtr>& rvToDoRequests,
            BSS_PARAM_STRUCT& bss_param,
            BSS_INPUT_DATA_G& bss_input) const;

        MVOID collectPostBSSExifData(MINT32 reqId, Vector<MINT32>& vNewIndex);

        MVOID handleFinish(list<PipeRequestPtr>& rvRequest, list<PipeRequestPtr>& rvBssOrderedRequests);

        MVOID handleByPass(PipeRequestPtr request);

    private:
        MINT32                                          miOpenId = -1;

        mutable Mutex                                   mLock;
        WaitQueue<PipeRequestPtr>                       mRequests;

        list<PipeRequestPtr>                            mvPendingRequests;

        NSDCMF::DCBufferPool                            mBufPool;
};

// Namespaces
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_DCMF_FEATURE_PIPE_PRE_VENDOR_NODE_H_