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
#ifndef _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_IMP_H_
#define _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_IMP_H_
//
#include "SWNode/SWFilterNode.h"
#include "SWNode/VendorFilterNode.h"
#include "HWNode/VSDOF_P2BNode.h"
#include "UtilityNode/StereoMDPNode.h"
//
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
class BokehPipeImp : public CamPipe<BokehPipeNode>, public BokehPipeNode::Handler_T, public IBokehPipe
{
    public:
        BokehPipeImp(MINT32 openSensorIndex, MINT32 runPath = 0);
        virtual ~BokehPipeImp();
        //
        MBOOL   init(const char *name=NULL) override;
        MBOOL   uninit(const char *name=NULL) override;
        MBOOL   enque(EffectRequestPtr &param) override;
        MVOID   flush() override;
        MVOID   sync() override;
        MVOID   setStopMode(MBOOL flushOnStop) override;
        MINT32  getPipeMode() override;

    protected:
        typedef CamPipe<BokehPipeNode> PARENT_PIPE;
        MBOOL onInit() override;
        MVOID onUninit() override;

    protected:
        MBOOL onData(DataID id, EffectRequestPtr &data) override;
        MBOOL onData(DataID id, FrameInfoPtr &data) override;
        MBOOL onData(DataID id, SmartImageBuffer &data) override;

    //
    private:
        void buildGeneralPath();
        void buildHWBokehPath();
        void buildSWBokehPath();
        void buildVendorBokehPath();
    private:
        VSDOF_P2BNode mP2BNode;
        StereoMDPNode mMDPNode;
        //SWFilterNode mSWFilterNode;
        VendorFilterNode mVendorFilterNode;
        //
        MUINT32     miOpenId = 0;
        // mutex for request map
        android::Mutex mReqMapLock;
        // EffectRequest collections
        KeyedVector<MUINT32, EffectRequestPtr> mvRequestPtrMap;
        // create mode (eg: sw,hw,vendor mode)
        MINT32     mMode = 0;
};
};
};
};
#endif // _MTK_CAMERA_FEATURE_PIPE_BOKEH_PIPE_IMP_H_