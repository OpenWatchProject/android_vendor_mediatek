/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _HDRHAL_H_
#define _HDRHAL_H_

#include "IHDRHAL.h"

// ---------------------------------------------------------------------------

namespace NSCam {

class HDRHAL final : public IHDRHAL
{
public:
    HDRHAL();
    ~HDRHAL();

    MBOOL init(void *pInitInData);

    MBOOL Do_Normalization();
    MBOOL Do_SE(HDR_PIPE_SE_INPUT_INFO& rHdrPipeSEInputInfo);
    MBOOL Do_FeatureExtraction(
    HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO& rHdrPipeFeatureExtractInputInfo);
    MBOOL Do_Alignment();
    MBOOL Do_Fusion(HDR_PIPE_WEIGHT_TBL_INFO** pprBlurredWeightMapInfo);
    MBOOL WeightingMapInfoGet(HDR_PIPE_WEIGHT_TBL_INFO** pprWeightMapInfo);
    MBOOL WeightingMapInfoSet(HDR_PIPE_BMAP_BUFFER* pBmapInfo);
    MBOOL ResultBufferSet(MUINT8* bufferAddr, MUINT32 bufferSize);
    MBOOL HdrCroppedResultGet(HDR_PIPE_HDR_RESULT_STRUCT& rHdrCroppedResult);
    MBOOL HdrSmallImgBufSet(HDR_PIPE_CONFIG_PARAM& rHdrPipeConfigParam);
    MBOOL HdrWorkingBufSet(MUINT8* u4BufAddr, MUINT32 u4BufSize);
    MUINT32 HdrWorkingBuffSizeGet();
    void QuerySmallImgResolution(MUINT32& ru4Width, MUINT32& ru4Height);
    void QuerySEImgResolution(MUINT32& ru4Width, MUINT32& ru4Height);
    MUINT32 SEImgBuffSizeGet();
    void SaveHdrLog(MUINTPTR u4RunningNumber);

private:
    MTKHdr* m_pHdrDrv;
    MINT32 m_G_i4GMV2[10];
    // Output Frame Num passed by HDR scenario, used in Pipe layer.
    MUINT32 m_GS_u4OutputFrameNum;

    MBOOL uninit();
};

} // namespace NSCam

#endif // _HDRHAL_H_
