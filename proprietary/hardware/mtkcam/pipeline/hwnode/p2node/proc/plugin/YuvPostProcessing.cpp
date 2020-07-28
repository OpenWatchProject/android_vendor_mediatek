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

#include "YuvPostProcessing.h"
#include "../PluginProcedure.h"

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// test mode: 0=> in-place processing, 1=> in-out processing
//  => in-place: do a software NR and draw a thick line in the top of image
//  => in-out: memcpy the source image and draw a thick line in the bottom of image
#define TEST_MODE (0)

// auto-mount yuv processing
REGISTER_POSTPROCESSING(Yuv, YuvPostProcessing);

/******************************************************************************
 *
 ******************************************************************************/
YuvPostProcessing::
YuvPostProcessing(MUINT32 const openId)
        : PostProcessing(openId)
{
    mpSwnr = MAKE_SwNR(muOpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
YuvPostProcessing::
~YuvPostProcessing() {
    delete mpSwnr;
}


/******************************************************************************
 *
 ******************************************************************************/
PostProcessing::ProcessingProfile&
YuvPostProcessing::
profile() {
#if TEST_MODE == 0
    static PostProcessing::ProcessingProfile profile(
            eIN_PLACE_PROCESSING,
            eIMG_FMT_I420);
#elif TEST_MODE == 1
    static PostProcessing::ProcessingProfile profile(
            eIN_OUT_PROCESSING,
            eIMG_FMT_I420,
            eIMG_FMT_I420);
#else
#error unsupported this test mode
#endif
    return profile;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
YuvPostProcessing::
doOrNot(PreConditionParams const& params) {
    MUINT8 flag = 0;
    if (params.bResized || (tryGetMetadata<MUINT8>(params.pInHalMeta, MTK_P2NODE_UT_PLUGIN_FLAG, flag) && !(flag & MTK_P2NODE_UT_PLUGIN_FLAG_YUV)))
        return MFALSE;

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
YuvPostProcessing::
process(ProcessingParams const& param) {
    MINT32 iso = 100;
    if (param.pInAppMeta && !tryGetMetadata<MINT32>(param.pInAppMeta, MTK_SENSOR_SENSITIVITY, iso))
        MY_LOGW_IF(1, "no MTK_SENSOR_SENSITIVITY from InAppMeta");

#if TEST_MODE == 0
    IImageBuffer *pInBuf = param.pInBuffer;
    ISwNR::SWNRParam swnrParam;
    swnrParam.iso = iso;
    if (!mpSwnr->doSwNR(swnrParam, pInBuf)) {
        MY_LOGE("SWNR failed");
        return MFALSE;
    }

    void *pInVa = (void *) (pInBuf->getBufVA(0));
    int nBufSize = pInBuf->getBufSizeInBytes(0);
    memset((void*)pInVa+nBufSize*2/10, 200, nBufSize/10);

#elif TEST_MODE == 1
    IImageBuffer *pInBuf = param.pInBuffer;
    IImageBuffer *pOutBuf = param.pOutBuffer;
    for (int i = 0 ; i < pInBuf->getPlaneCount(); i++) {
        void *pInVa = (void *) (pInBuf->getBufVA(i));
        void *pOutVa = (void *) (pOutBuf->getBufVA(i));
        int nBufSize = pInBuf->getBufSizeInBytes(i);
        MY_LOGD("in-out processing. plane:%d. va[in]:0x%x, va[out]:0x%x",i ,pInVa, pOutVa);
        memcpy(pOutVa, pInVa, nBufSize);
        memset((void*)pOutVa+nBufSize*8/10, 100, nBufSize/10);
    }
    pOutBuf->syncCache(eCACHECTRL_FLUSH);
#endif

    return MTRUE;
}

