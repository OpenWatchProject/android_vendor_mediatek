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
#ifndef _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_GUIDED_FILTER_NODE_H_
#define _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_GUIDED_FILTER_NODE_H_
//
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <gf_hal.h>
//
#include "BokehPipeNode.h"
//
using namespace android;
using namespace StereoHAL;

//
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{
class GuidedFilterNode : public BokehPipeNode
{
//************************************************************************
// Public function
//************************************************************************
    public:
        //****************************************************************
        // Does not support default construct
        //****************************************************************
        GuidedFilterNode() = delete;
        GuidedFilterNode(
                const char *name,
                Graph_T *graph,
                MINT8 mode = GENERAL);
        virtual ~GuidedFilterNode();
//************************************************************************
// BokehPipeNode interface
//************************************************************************
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
//************************************************************************
// Private function
//************************************************************************
    public:
        virtual MBOOL onData(
                DataID id,
                EffectRequestPtr &request);
    private:
        MVOID cleanUp();
        //
        MBOOL createBufferPool(
                android::sp<ImageBufferPool>& pPool,
                MUINT32 width,
                MUINT32 height,
                NSCam::EImageFormat format,
                MUINT32 bufCount,
                const char* caller,
                MUINT32 bufUsage = ImageBufferPool::USAGE_HW);
        //
        MVOID setImageBufferValue(
                SmartImageBuffer& buffer,
                MINT32 width,
                MINT32 height,
                MINT32 value);
        //
        MBOOL executeAlgo(
                EffectRequestPtr request);
        //
        MBOOL requireAlgoDataFromRequest(
                const EffectRequestPtr request,
                GF_HAL_IN_DATA& inData,
                sp<IImageBuffer>& pMYS,
                sp<IImageBuffer>& pDMW);
        //
        MBOOL getBufferFromRequest(
                const EffectRequestPtr request,
                BokehEffectRequestBufferType type,
                sp<IImageBuffer>& frame
                );
//************************************************************************
// Private member
//************************************************************************
    private:
        WaitQueue<EffectRequestPtr>             mRequests;
        sp<ImageBufferPool>                     mpDMGBufPool = nullptr;
        sp<ImageBufferPool>                     mpDMBGBufPool= nullptr;
        GF_HAL*                                 mpGf_Hal     = nullptr;
        MINT8                                   mGFMode      = GF_NODE_MODE_NONE;
};

};
};
};
#endif // _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_GUIDED_FILTER_NODE_H_