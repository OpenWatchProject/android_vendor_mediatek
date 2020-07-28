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
#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_DENOISE_NODE_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_DENOISE_NODE_H_

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"
#include <bwdn_hal.h>
#include <mtkcam/aaa/ILscTable.h>
#include <stereo_tuning_provider.h>

using namespace StereoHAL;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class DeNoiseNode : public BMDeNoisePipeNode
{
    public:
        DeNoiseNode() = delete;
        DeNoiseNode(const char *name, Graph_T *graph, MINT32 openId);
        virtual ~DeNoiseNode();

        virtual MBOOL onData(DataID id, PipeRequestPtr &request);
        virtual MBOOL onData(DataID id, ImgInfoMapPtr& data);
        virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);
        MBOOL   releaseALG();
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
        MVOID   initBufferPool();
    private:
        // routines
        MVOID  cleanUp();
        MVOID  collectExifData(map<MINT32, MINT32>& exifData, BWDN_HAL_PARAMS &rInData, MINT32* pSceneInfo);

        MBOOL  getIsRotateAndBayerOrder(BWDN_HAL_PARAMS &rInData);
        MBOOL  getDynamicShadingRA(BWDN_HAL_PARAMS &rInData);
        MBOOL  getPerformanceQualityOption(BWDN_HAL_PARAMS &rInData);
        MBOOL  getSensorGainAndShadingRA(IMetadata* pMeta, BWDN_HAL_PARAMS &rInData, MBOOL isBayer);
        MBOOL  getISODependentParams(IMetadata* pMeta, BWDN_HAL_PARAMS &rInData);
        MBOOL  getAffineMatrix(float* warpingMatrix, BWDN_HAL_PARAMS &rInData);
        MBOOL  getShadingGain(IMetadata* pMeta, NS3Av3::TuningParam& tuningParam, NSIspTuning::ILscTable& lscTbl, MBOOL isBayer);
        MINT32 getDVEFactorFromDSMode(MUINT32 dsMode);

        // image processes
        sp<ImageBufferHeap> createImageBufferHeapInRGB48(sp<IImageBuffer> pImgBuf);
        ImgInfoMapPtr doBMDeNoise(ImgInfoMapPtr imgInfo_PreProcess, ImgInfoMapPtr imgInfo_Depth, PipeRequestPtr request);

    private:
        WaitQueue<ImgInfoMapPtr>                        mImgInfoRequests_PreProcess;
        WaitQueue<ImgInfoMapPtr>                        mImgInfoRequests_Depth;
        WaitQueue<PipeRequestPtr>                       mRequests;

        NS3Av3::IHal3A*                                 mp3AHal_Main1 = nullptr;
        NS3Av3::IHal3A*                                 mp3AHal_Main2 = nullptr;

        MINT32                                          miOpenId = -1;
        MINT32                                          mBayerOrder_main1 = -1;

        StereoSizeProvider*                             mSizePrvider = StereoSizeProvider::getInstance();
        MINT32                                          miDumpShadingGain = 0;

        BMDeNoiseQualityPerformanceParam                mDebugPerformanceQualityOption;

        MBOOL                                           mbReUseBuffer = MFALSE;

        NSBMDN::BMBufferPool                            mBufPool;

        StereoHAL::BWDN_HAL*                            mpBWDNHAL = nullptr;

        mutable Mutex                                   mLock;

        NSCam::NSIoPipe::DVEConfig                      mDVEConig;
        MINT32                                          mDebugDsH = -1;
        MINT32                                          mDebugDsV = -1;
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_DENOISE_NODE_H_