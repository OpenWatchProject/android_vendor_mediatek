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

#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_N3D_NODE_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_N3D_NODE_H_

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"

#include <n3d_hal.h>
#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const int N3D_WORKING_BUF_SET = 1;
const int MAX_WARPING_PLANE_COUNT = 3;
const int DENOISE_CONST_FE_EXEC_TIMES = 2;

class BMN3DNode: public BMDeNoisePipeNode
{
public:
    BMN3DNode() = delete;
    BMN3DNode(const char* name, Graph_T *graph, MINT32 openId);
    virtual ~BMN3DNode();
     /**
     * Receive EffectRequestPtr from previous node.
     * @param id The id of receiverd data.
     * @param request EffectRequestPtr contains image buffer and some information.
     */
    virtual MBOOL onData(DataID id, ImgInfoMapPtr& data);
    virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    MVOID   initBufferPool();
    virtual const char* onDumpBIDToName(BMDeNoiseBufferID BID);
private:

    MVOID cleanUp();
    MBOOL performN3DALGO_VRPV(ImgInfoMapPtr& rP2ImgInfo);
    MBOOL performN3DALGO_CAP(ImgInfoMapPtr& rP2ImgInfo);
    // input N3DParams
    MBOOL prepareN3DInputParam(ImgInfoMapPtr& rP2ImgInfo, N3D_HAL_PARAM& rN3dParam);
    MBOOL prepareN3DInputParam_CAP(ImgInfoMapPtr& rP2ImgInfo, N3D_HAL_PARAM_CAPTURE& rN3dParam);
    // output N3DParams
    MBOOL prepareN3DOutputParam(N3D_HAL_OUTPUT& rN3dParam, ImgInfoMapPtr pToDPEImgInfo, ImgInfoMapPtr pToOCCImgInfo);
    MBOOL prepareN3DOutputParam_CAP(ImgInfoMapPtr pInfoMap, N3D_HAL_OUTPUT_CAPTURE& rN3dParam, ImgInfoMapPtr pToDPEImgInfo);
    MBOOL prepareN3DOutputYUVMask(ImgInfoMapPtr pInfoMap, N3D_HAL_OUTPUT_CAPTURE& rN3dParam, ImgInfoMapPtr pToDPEImgInfo);

    MBOOL prepareFEFMData(ImgInfoMapPtr& rP2ImgInfo, HWFEFM_DATA& rFefmData);
    MBOOL prepareEISData(ImgInfoMapPtr& rP2ImgInfo, EIS_DATA& rEISData);

    MBOOL copyImageBuffer(sp<IImageBuffer> srcImgBuf, sp<IImageBuffer> dstImgBuf);

    MVOID debugN3DParams(N3D_HAL_PARAM& param, N3D_HAL_OUTPUT& output);
    MVOID debugN3DParams_Cap(N3D_HAL_PARAM_CAPTURE& param, N3D_HAL_OUTPUT_CAPTURE& output);

    MBOOL writeN3DResultToMeta(const N3D_HAL_OUTPUT&  n3dOutput, sp<EffectRequest> pEffectReq);

private:
    WaitQueue<ImgInfoMapPtr>                        mJobQueue;

    N3D_HAL*                                        mpN3DHAL_CAP  = nullptr;
    N3D_HAL*                                        mpN3DHAL_VRPV = nullptr;

    NSBMDN::BMBufferPool                            mBufPool;

    MSize                                           N3D_OUTPUT_IMG_SIZE;
    MSize                                           N3D_OUTPUT_MASK_SIZE;

    /*For Using WPE Node only
    MSize                                           N3D_OUTPUT_WARPING_GRID_MAIN1_SIZE;
    MSize                                           N3D_OUTPUT_WARPING_GRID_MAIN2_SIZE;
    MSize                                           N3D_OUTPUT_MASK_MAIN1_SIZE;
    MSize                                           N3D_OUTPUT_IMG_CC_MAIN2SIZE;
    MSize                                           N3D_OUTPUT_MASK_MASK1_SIZE;
    MSize                                           N3D_OUTPUT_PRE_MASK_MAIN2_SIZE;*/

    MINT32                                          miOpenId = -1;
    MINT32                                          mWarpingPlanNum_M;
    MINT32                                          mWarpingPlanNum_S;
};


}; //NSFeaturePipe
}; //NSCamFeature
}; //NSCam

#endif


