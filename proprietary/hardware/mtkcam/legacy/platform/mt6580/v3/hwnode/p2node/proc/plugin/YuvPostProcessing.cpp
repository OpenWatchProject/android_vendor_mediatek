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

// auto-mount yuv processing
REGISTER_POSTPROCESSING(Yuv, YuvPostProcessing);

/******************************************************************************
 *
 ******************************************************************************/
YuvPostProcessing::
YuvPostProcessing(MUINT32 const openId)
        : PostProcessing(openId)
{

}


/******************************************************************************
 *
 ******************************************************************************/
YuvPostProcessing::
~YuvPostProcessing() {

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
YuvPostProcessing::
decide(ConditionParams const& condition, DecisionParams &decision)
{


    // Example code start
    // decide to run with plugin and streaming in full gray mode
    decision.processType = eIN_PLACE_PROCESSING;
    decision.inImgFmt = eIMG_FMT_YV12;
    decision.processMode = PROCESS_FULL_GRAY;
    return MTRUE;
    // Example code end

    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
YuvPostProcessing::
process(ProcessParams const& param) {

    MY_LOGD("Yuv post-processing process, ProcessMode:%d", param.uProcessMode);
    switch (param.uProcessMode) {
        case PROCESS_FULL_GRAY:
        {
            // Example code: only process full gray logic when preview
            if (param.captureFrame) {
                // Do nothing in capture case
                break;
            } else {
                // Process full gray in preview case
                IImageBuffer *pInBuf = param.pInBuffer;
                for (int i = 1 ; i < pInBuf->getPlaneCount(); i++) {
                    void *pInVa = (void *) (pInBuf->getBufVA(i));
                    int nBufSize = pInBuf->getBufSizeInBytes(i);
                    memset((void*)pInVa, 128, nBufSize);
                }
                pInBuf->syncCache(eCACHECTRL_FLUSH);
                break;
            }
        }
        case PROCESS_HALF_GRAY:
        {
            IImageBuffer *pInBuf = param.pInBuffer;
            IImageBuffer *pOutBuf = param.pOutBuffer;
            for (int i = 0 ; i < pInBuf->getPlaneCount(); i++) {
                void *pInVa = (void *) (pInBuf->getBufVA(i));
                void *pOutVa = (void *) (pOutBuf->getBufVA(i));
                int nBufSize = pInBuf->getBufSizeInBytes(i);
                memcpy(pOutVa, pInVa, nBufSize);
                if (i > 0)
                    memset((void*)pOutVa, 128, nBufSize/2);
            }
            pOutBuf->syncCache(eCACHECTRL_FLUSH);
            break;
        }
        default:
            return MFALSE;
    }
    return MTRUE;
}

