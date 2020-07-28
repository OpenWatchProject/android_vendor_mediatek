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
#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_P2AFM_NODE_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_P2AFM_NODE_H_

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <stereo_tuning_provider.h>
#include <featurePipe/vsdof/util/QParamTemplate.h>

class DpBlitStream;

using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

const int P2AFM_CONST_FE_EXEC_TIMES = 2;
const int P2AFM_WORKING_BUF_SET = 1;
const int P2AFM_TUNING_BUF_SET = 10;

class P2AFMNode : public BMDeNoisePipeNode
{
    public:
        P2AFMNode() = delete;
        P2AFMNode(const char *name, Graph_T *graph, MINT32 openId);
        virtual ~P2AFMNode();

        virtual MBOOL onData(DataID id, ImgInfoMapPtr &imgInfo);
        virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
        MVOID   initBufferPool();
    private:
        MVOID cleanUp();

        MBOOL doP2AFM(EffectRequestPtr request);

        MVOID doDataDump(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx);
        MVOID doInputDataDump(EffectRequestPtr request);
        MVOID doOutputDataDump(EffectRequestPtr request, EnquedBufPool* pEnqueBufferPool);

        // P2 callbacks
        static MVOID onP2Callback(QParams& rParams);
        MVOID handleP2Done(QParams& rParams, EnquedBufPool* pEnqueData);
        static MVOID onP2FailedCallback(QParams& rParams);

        // Buffer pool init
        MVOID queryFEOBufferSize(MSize iBufSize, MUINT iBlockSize, MUINT32 &riFEOWidth, MUINT32 &riFEOHeight);
        MVOID queryFMOBufferSize(MUINT32 iFEOWidth, MUINT32 iFEOHeight, MUINT32 &riFMOWidth, MUINT32 &riFMOHeight);

        // prepare template
        MVOID prepareTemplateParams();
        MVOID debugQParams(const NSIoPipe::QParams& rInputQParam);

        // Brief dump buffers after p2 operation done
        MVOID dumpingP2Buffers(EnquedBufPool* pEnqueBufPool);

        // Prepare the QParam template of each scenario
        MBOOL prepareBurstQParams();
        MBOOL prepareBurstQParams_CAP(MINT32 iModuleTrans);

        // Build the to-enque QParam in runtime
        MBOOL buildQParams_CAP(
                            ImgInfoMapPtr &imgInfo,
                            QParams& rEnqueParam,
                            EnquedBufPool* pEnquePoolData
                            );

        MVOID _buildFEFMFrame(QParamTemplateFiller& rQParamFiller, SmartImageBuffer feoBuf[], EnquedBufPool* pEnquePoolData);

        /**
        * @brief Calaulate the crop region according to the screen ratio
        * @param [in] pFSRAW_frameBuf : input frameBuf
        * @param [out] rCropStartPt : start point of the crop region
        * @param [out] rCropSize : crop region size
        * @return operation result
        */
        MBOOL calCropForScreen(sp<IImageBuffer> pFrameBuf, MPoint &rCropStartPt, MSize& rCropSize );

        // mapping between portID and node data type
        NSCam::NSIoPipe::PortID mapToPortID(const MUINT32 nodeDataType);

        // isp tuning operations
        TuningParam applyISPTuning(
                                SmartTuningBuffer& targetTuningBuf,
                                const ISPTuningConfig& ispConfig,
                                MBOOL isMain1);

        MVOID setupEmptyTuningWithFM(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameID);



    private:
        WaitQueue<ImgInfoMapPtr>                        mImgInfoRequests;

        INormalStream*                                  mpINormalStream = nullptr;
        // 3A hal
        IHal3A*                                         mp3AHal_Main1 = nullptr;
        IHal3A*                                         mp3AHal_Main2 = nullptr;
        // Burst triggering QParams template
        NSIoPipe::QParams mBurstParamTmplate_Cap;
        NSIoPipe::QParams mBurstParamTmplate_NORMAL;
        // SRZ CROP/RESIZE template, frameID to SRZInfo map
        KeyedVector<MUINT, _SRZ_SIZE_INFO_> mSrzSizeTemplateMap;
        // FE Tuning Buffer Map, key=stage, value= tuning buffer
        KeyedVector<MUINT, NSCam::NSIoPipe::FEInfo> mFETuningBufferMap;
        // FM Tuning Buffer Map, key=frame ID, value= tuning buffer
        KeyedVector<MUINT, NSCam::NSIoPipe::FMInfo> mFMTuningBufferMap;

        MINT32                                          miOpenId = -1;

        StereoSizeProvider*                             mSizePrvider = StereoSizeProvider::getInstance();
        NSBMDN::BMBufferPool                            mBufPool;

        MSize                                           P2AFM_FD_IMG_SIZE;
        MSize                                           P2AFM_FEAO_INPUT_SIZE;
        MSize                                           P2AFM_FE1BO_INPUT_SIZE;
        MSize                                           P2AFM_FE2BO_INPUT_SIZE;
        MSize                                           P2AFM_FE1CO_INPUT_SIZE;
        MSize                                           P2AFM_FE2CO_INPUT_SIZE;
        MSize                                           P2AFM_RECT_IN_VR_SIZE_MAIN1;
        MSize                                           P2AFM_RECT_IN_VR_SIZE_MAIN2;
        MSize                                           P2AFM_RECT_IN_PV_SIZE_MAIN1;
        MSize                                           P2AFM_RECT_IN_PV_SIZE_MAIN2;
        MSize                                           P2AFM_RECT_IN_CAP_SIZE;
        MSize                                           P2AFM_RECT_IN_CAP_IMG_SIZE;
        MSize                                           P2AFM_MAIN_IMAGE_SIZE;
        MSize                                           P2AFM_MAIN_IMAGE_CAP_SIZE;
        MSize                                           P2AFM_CCIN_SIZE;

        StereoArea                                      P2AFM_MAIN2_FEAO_AREA;
        StereoArea                                      P2AFM_MAIN2_FEBO_AREA;
        StereoArea                                      P2AFM_MAIN2_FECO_AREA;

        int                                             frameIdx_LSide[2];
        int                                             frameIdx_RSide[2];
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_P2AFM_NODE_H_
