/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_FUSIONCTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_FUSIONCTRLER_H_

#include "BaseFusionCtrler.h"
#include <DpBlitStream.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT == 1)
#include <dlfcn.h>
#include "cFusionAPI.h"

typedef struct
{
    void* ptr;
    InitResult(*fusionInit)(const calib_data_t calibData, char* strBinaryPath);
    ProcessResult(*fusionProcess)(uint8_t wideImage[], uint8_t teleImage[], fusionInputParams_t
                                  inParams, uint8_t fusedImage[], fusionOutputParams_t& outParams);
    DestroyResult(*fusionDestroy)();
    const IO_triplet_t* (*getSupportedIoSizes)();
    SetIoSizeResult(*setIoSize)(IO_triplet_t io_triplet);
    zoomRange_t (*getSupportedZoomFactorRange)(IO_triplet_t io_triplet);
} my_fusion_t;
#endif


using namespace android;

namespace NSCam
{
namespace plugin
{

struct sMDP_Config
{
    DpBlitStream* pDpStream = nullptr;
    IImageBuffer* pSrcBuffer = nullptr;
    IImageBuffer* pDstBuffer = nullptr;
    MINT32 rotAngle = 0;

    // These 2 size are used for special request that the
    // allocated buffer size is bigger than actual content size
    MSize customizedSrcSize = MSize(0, 0);
    MSize customizedDstSize = MSize(0, 0);
    DpRect dst_roi;
    DpRect src_roi;
};

class FusionCtrler : public BaseFusionCtrler
{
        // this class is not allow to instantiate.
    public:
        FusionCtrler(MINT32 mode);
        ~FusionCtrler();
    public:
        using BaseFusionCtrler::getCaptureNum;
        using BaseFusionCtrler::getDelayFrameNum;
        using BaseFusionCtrler::setCaptureNum;
        using BaseFusionCtrler::setDelayFrameNum;
        //
        // override base operation
        //
    public:
        bool            init() override;
        bool            doFusion(ReqFrameSP main1Frame,
                                 ReqFrameSP main2Frame) override;

    private:
        void            doCopy(ReqFrameSP main1Frame,
                               ReqFrameSP main2Frame, MBOOL srcWide);

        // MDP related utility
        DpBlitStream*   mpDpStream = nullptr;
        DpColorFormat   getDpColorFormat(MINT32 format);
        MBOOL           excuteMDP(sMDP_Config config);
        MBOOL           createEmptyImageBuffer(sp<IImageBuffer>& imageBuf,
                            MUINT32 w, MUINT32 h, MUINT32 format, MBOOL isContinuous);
        MINT32          getZoomRatio(ReqFrameSP main1Frame);


#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT == 1)
        void   doFusion01_setIoTriplet(IO_triplet_t& io_triplet,
                                       int wideWidth, int wideHeight, int widePpln,
                                       int teleWidth, int teleHeight, int telePpln,
                                       int outWidth, int outHeight, int outPpln);
        void   doFusion01_updateParamsGet3A(info_3a_t* inout, ReqFrameSP frame);
        void   doFusion01_updateParams(fusionInputParams_t& inParams,
                                       ReqFrameSP wide, ReqFrameSP tele);
        MBOOL  doFusion01_loadLib(my_fusion_t& my_fusion);
        MINT32 doFusion01(ReqFrameSP main1Frame, ReqFrameSP main2Frame);
#endif


#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT == 1)
        void doFusion02(ReqFrameSP main1Frame,
                        ReqFrameSP main2Frame);
#endif


#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT == 1)
        void doFusion03(ReqFrameSP main1Frame,
                        ReqFrameSP main2Frame);
#endif

};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_FUSIONCTRLER_H_
