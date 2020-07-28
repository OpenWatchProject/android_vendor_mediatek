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
#ifndef _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_VSDOF_P2B_NODE_H_
#define _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_VSDOF_P2B_NODE_H_
#include "BokehPipeNode.h"
// queue
#include <queue>
// NormalStream for P2B
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
// PortID
#include <mtkcam/drv/def/ispio_sw_scenario.h>
// QParams
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
// 3DNR
#include <common/3dnr/3dnr_hal_base.h>
//
#include <mtkcam/feature/stereo/hal/pass2_size_data.h>
#include <vsdof/util/TuningBufferPool.h>
//
#include <chrono>
//
using namespace android;
using namespace NSIoPipe;
using namespace StereoHAL;
//
class DpBlitStream;
//
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class enqueData
{
    public:
		enqueData() = default;
		MUINT32 scenarioId = 0;
		EffectRequestPtr request = nullptr;
		KeyedVector<MUINT32, SmartImageBuffer> mEnquedSmartImgBufMap;
		MBOOL isFrist3NDRFrame = MFALSE;
		MBOOL is3DNROn = MFALSE;
        SmartTuningBuffer mpTuningBuffer = nullptr;
		//
		void* cookies = nullptr;
        std::chrono::time_point<std::chrono::system_clock> start;
        std::chrono::time_point<std::chrono::system_clock> end;
};
class VSDOF_P2BNode : public BokehPipeNode
{
    public:
        /**
         * VSDOF_P2BNode does not support default constructor.
         */
        VSDOF_P2BNode() = delete;
        /**
         * A constructor.
         * @param name A name of VSDOF_P2BNode instance.
         * @param graph A pointer of FeaturePipe camera graph which is contain this node.
         */
        VSDOF_P2BNode(const char *name, Graph_T *graph, MINT32 openId, MINT8 mode = GENERAL);
        /**
         * A destructor.
         */
        virtual ~VSDOF_P2BNode();
        /**
         * Receive EffectRequestPtr from previous node.
         * @param id The id of receiverd data.
         * @param request EffectRequestPtr contains image buffer and some information.
         */
        virtual MBOOL onData(DataID id, EffectRequestPtr &request);
        /**
         * Receive SmartImageBuffer from previous node.
         * @param id The id of receiverd data.
         * @param request SmartImageBuffer contains image buffer.
         */
        //virtual MBOOL onData(DataID id, SmartImageBuffer &data);
        //
    protected:
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onThreadStart();
        virtual MBOOL onThreadStop();
        virtual MBOOL onThreadLoop();
    private:
        struct nr3dConfigData
        {
            MUINT32 frameId;
            MINT32 ISO;
            MINT32 GMV_x;
            MINT32 GMV_y;
            MINT32 CMV_x;
            MINT32 CMV_y;
            MINT32 CONF_x;
            MINT32 CONF_y;
        };
    private:        //
        MBOOL createBufferPool(
                android::sp<ImageBufferPool>& pPool,
                MUINT32 width,
                MUINT32 height,
                NSCam::EImageFormat format,
                MUINT32 bufCount,
                const char* caller,
                MUINT32 bufUsage = ImageBufferPool::USAGE_HW,
                MBOOL continuesBuffer = MTRUE);
        /**
         * clean all data and image buffer.
         */
        MVOID cleanUp();
        /**
         * Perpare to enque data to P2B.
         * @param request EffectRequestPtr contain all input buffer and output buffer which is pass by BokehNode.
         * @param dmgBuf dmgBuf is output from GuidedFilterNode. dmgBuf is quality-enhanced depthmap.
         * @param dmbgBuf dmbgBuf is output from GuidedFilterNode. dmbgBuf is used to decide Bokeh level for each pixel.
         */
        MBOOL enqueP2B(
                    EffectRequestPtr request/*,
                    SmartImageBuffer dmgBuf,
                    SmartImageBuffer dmbgBuf*/);
        //
        MBOOL getP2AOutputSize(
                    MINT32 scenarioId,
                    Pass2SizeInfo& outSize);
        //
        MBOOL setSRZInfo(
                    FrameParams& param,
                    MINT32 moduleTag,
                    MSize inputSize,
                    MSize outputSize);
        //
        MBOOL unpackInputBufferFromRequest(
                    EffectRequestPtr request,
                    FrameParams& frameParam);
        //
        MBOOL unpackOutputBufferFromRequest(
                    EffectRequestPtr request,
                    FrameParams& frameParam,
                    MINT32 rot);
        /**
         * Set PortID and ImageBuffer information form FrameInfoPtr.
         * @param param param store tuning setting include in/out port and image buffer setting and tuning buffer.
         * @param frameInfo frameInfo contain input buffer.
         * @param bufferType bufferType is used to decide PortID.
         */
        MBOOL setP2BInputPort(
                    FrameParams &param,
                    FrameInfoPtr frameInfo,
                    MUINT32 bufferType);
        /**
         * Set PortID and ImageBuffer information form SmartImageBuffer.
         * @param param param store tuning setting include in/out port and image buffer setting and tuning buffer.
         * @param imgBuffer imgBuffer contain input buffer.
         * @param bufferType bufferType is used to decide PortID.
         */
        MBOOL setP2BInputPort(
                    FrameParams &param,
                    SmartImageBuffer imgBuffer,
                    MUINT32 bufferType);
        MBOOL setP2BInputPort(
                    FrameParams &param,
                    sp<IImageBuffer> imgBuffer,
                    MUINT32 bufferType);
        /**
         * Set PortID and ImageBuffer information form FrameInfoPtr.
         * @param param param store tuning setting include in/out port and image buffer setting and tuning buffer.
         * @param frameInfo frameInfo contain output buffer.
         * @param bufferType bufferType is used to decide PortID.
         */
        MBOOL setP2BOutputPort(
                    FrameParams &param,
                    FrameInfoPtr frameInfo,
                    MUINT32 bufferType,
                    MINT32 transform = 0);
        /**
         * Set PortID and ImageBuffer information form FrameInfoPtr.
         * @param param param store tuning setting include in/out port and image buffer setting and tuning buffer.
         * @param imgBuffer imgBuffer contain output buffer.
         * @param bufferType bufferType is used to decide PortID.
         */
        MBOOL setP2BOutputPort(
                    FrameParams &param,
                    SmartImageBuffer imgBuffer,
                    MUINT32 bufferType,
                    MINT32 transform = 0);
        MBOOL setP2BOutputPort(
                    FrameParams &param,
                    sp<IImageBuffer> imgBuffer,
                    MUINT32 bufferType,
                    MINT32 transform = 0);
        //
        static MVOID onP2Callback(QParams& rParams);
        MVOID handleP2Done(QParams& rParams);
        static MVOID onP2FailCallback(QParams& rParams);
        MVOID handleP2Fail(QParams& rParams);
        NSCam::NSIoPipe::PortID mapToPortID(const MUINT32 bufDataType);
        MUINT32 mapToBufferID(NSCam::NSIoPipe::PortID const portId, const MUINT32 scenarioID);
        //
        MBOOL getFrameInfoFromRequest(EffectRequestPtr request, FrameInfoPtr& frame, MINT32 bufType, MBOOL isInputPort);
        //
        MVOID setting3DNR(nr3dConfigData& config, FrameParams& rParams, MUINT32 scenarioID);
        //
        MBOOL removeInPortId(FrameParams &frameParam, NSCam::NSIoPipe::PortID portId);
        MBOOL removeOutPortId(FrameParams &frameParam, NSCam::NSIoPipe::PortID portId);
        sp<IImageBuffer> createEmptyImageBuffer(
                                const IImageBufferAllocator::ImgParam imgParam,
                                const char* name,
                                MINT usage);
        MVOID shiftDepthMapValue(
                    IImageBuffer* depthMap,
                    MUINT8 shiftValue);
        MVOID outputDepthMap(
                    MINT32 id,
                    IImageBuffer* depthMap,
                    IImageBuffer* displayResult);
        MVOID outputDepthMapAndAddDebugInfo(
                    MINT32 id,
                    IImageBuffer* depthMap,
                    IImageBuffer* displayResult);
        //
    private:
        WaitQueue<EffectRequestPtr>                     mRequests;
        //
        sp<TuningBufferPool>                            mpP2BTuningPool = nullptr;
        sp<ImageBufferPool>                             mp3DNRBufPool = nullptr;
        sp<ImageBufferPool>                             mp3DNRBufPool_Rec = nullptr;
        // for capture use -> clean_img
        sp<ImageBufferPool>                             mpWDMABufPool = nullptr;
        // for capture use -> clean_img
        sp<ImageBufferPool>                             mpDepthMapBufPool = nullptr;
        //
        SmartImageBuffer                                mpInputFrame = nullptr;
        SmartImageBuffer                                mpOutputFrame = nullptr;
        //
        SmartImageBuffer                                mpInputFrame_Rec = nullptr;
        SmartImageBuffer                                mpOutputFrame_Rec = nullptr;
        //
        std::queue<enqueData*>                          mvEnqueData;
        //
        MBOOL                                           mbIsFirstRun3dnr = MTRUE;
        //
        NSCam::NSIoPipe::NSPostProc::INormalStream*     mpINormalStream = nullptr;  // for P2B
        //
        NSCam::NSIoPipe::NSPostProc::hal3dnrBase*       mp3dnr = nullptr;
        //
        MINT32                                          miOpenId = -1;
        MINT8                                           miMode = GENERAL;
        //
        MSize                                           miDmgi = -1;
        MSize                                           miDepi = -1;
        //
        //MBOOL isRunning = MFALSE;
        //
        //MINT32 inputCount = 0;
        //MINT32 outputCount = 0;
        mutable Mutex   mRequestLock;
        Condition       mRequestCond;
        MINT32          mShowDepthMap = 0;
        DpBlitStream*   mpDpStream = nullptr;
        mutable Mutex   mShiftDepthMapLock;
        std::shared_ptr<NS3Av3::IHal3A>                 mp3A = nullptr;
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_GUIDED_FILTER_NODE_H_