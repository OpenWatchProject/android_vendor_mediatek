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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_NODE_H_

#include "StreamingFeatureNode.h"
#include "N3DShotModeSelect.h"
#include <n3d_hal.h>

using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
class N3DNode : public StreamingFeatureNode
{
public:
    N3DNode(const char *name);
    virtual ~N3DNode();

public:
    virtual MBOOL onData(DataID id, const N3DData &data);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
private:
    MBOOL performN3DALGO_PV(N3DData &data);
    MBOOL prepareN3DInputParam(N3DData &data, N3D_HAL_PARAM& rN3dParam);
    MBOOL prepareN3DOutputParam(N3D_HAL_OUTPUT& rN3dParam);
    MBOOL prepareFEFMData(N3DData &data, HWFEFM_DATA& rFefmData);
    MVOID selectProperShotMode(bool isExecuted, const RequestPtr &pRequest, int* pSceneInfo);
    MBOOL handleDataToNext(RequestPtr& pRequest);
    MVOID debugN3DParams(N3D_HAL_PARAM& param, N3D_HAL_OUTPUT& output);

private:
    WaitQueue<N3DData> mN3DDatas;
    N3D_HAL* mpN3DHAL_PV = nullptr;
    // shot mode selector
    SelectShotMode mSelectShotMode;
    // buffer pools
    android::sp<ImageBufferPool> mN3DSceneinfoBufPool;
    android::sp<ImageBufferPool> mN3DImgBufPool;
    android::sp<ImageBufferPool> mN3DMaskBufPool;
    // size
    MSize SCENEINFO_SIZE;
    MSize N3D_OUTPUT_IMG_SIZE;
    MSize N3D_OUTPUT_MASK_SIZE;
    // shot mode information
    MINT32 mShotModeInfo = 0;
};


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif
