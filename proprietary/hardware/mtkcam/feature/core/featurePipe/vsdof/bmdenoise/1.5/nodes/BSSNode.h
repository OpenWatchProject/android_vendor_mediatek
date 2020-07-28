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
#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BSS_NODE_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BSS_NODE_H_

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"

#include <libbss/MTKBss.h>

using namespace NS3Av3;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class BSSNode : public BMDeNoisePipeNode
{
    /*
    *  Tuning Param for BSS ALG. Should not be configure by customer
    */
    static const int MF_BSS_ON = 1;
    static const int MF_BSS_SCALE_FACTOR = 8;
    static const int MF_BSS_CLIP_TH0 = 96;
    static const int MF_BSS_CLIP_TH1 = 512;
    static const int MF_BSS_ZERO = 12;
    static const int MF_BSS_ADF_TH = 12;
    static const int MF_BSS_SDF_TH = 70;
    static const int MF_BSS_ROI_PERCENTAGE = 95;

    struct GMV{
        MINT32 x = 0;
        MINT32 y = 0;
    };
    public:
        BSSNode() = delete;
        BSSNode(const char *name, Graph_T *graph, MINT32 openId);
        virtual ~BSSNode();

        virtual MBOOL onData(DataID id, ImgInfoMapPtr& data);
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
    private:
        // routines
        MVOID   cleanUp();
        MBOOL   retrieveGmvInfo(IMetadata* pMetadata, int& x, int& y, MSize& size);
        GMV     calMotionVector(IMetadata* pMetadata, MBOOL isMain);
        MVOID   updateBSSProcInfo(IImageBuffer* pBuf, BSS_PARAM_STRUCT& bss_param, BSS_INPUT_DATA_G& bss_data);
        MVOID   collectPreBSSExifData(MINT32 reqId, BSS_PARAM_STRUCT& bss_param, BSS_INPUT_DATA_G& bss_data);
        MVOID   collectPostBSSExifData(MINT32 reqId, Vector<MINT32>& vGMV, MINT32 bestShotIdx);

        // image process
        ImgInfoMapPtr doBSS(ImgInfoMapPtr imgInfo);

    private:
        WaitQueue<ImgInfoMapPtr>                        mImgInfoRequests;

        IHal3A*                                         mp3AHal_Main1 = nullptr;
        IHal3A*                                         mp3AHal_Main2 = nullptr;

        MINT32                                          miOpenId = -1;

        StereoSizeProvider*                             mSizePrvider = StereoSizeProvider::getInstance();

        list<ImgInfoMapPtr>                             mvPendingBSSCandidates;

        MBOOL                                           mbEnableBSS = MFALSE;
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BSS_NODE_H_