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

#ifndef _IHDRHAL_H_
#define _IHDRHAL_H_

#include <utils/def/BuiltinTypes.h>
#include <HDRDefs2.h>

#include <memory>

#include <MTKHdr.h>

namespace NSCam {

typedef struct
{
    // For HDR Drv Init.
    MUINT32 u4OutputFrameNum;   // image_num;
    MUINT32 u4FinalGainDiff0;   // ev_gain1;
    MUINT32 u4FinalGainDiff1;   // ev_gain3;
    MUINT32 u4ImgW;             // image_width;
    MUINT32 u4ImgH;             // image_height;
    MUINT32 u4TargetTone;       // target_tone;

    MUINT8* pSourceImgBufAddr[HDR_MAX_INPUT_FRAME]; // image_addr[3]; // input image address

    // For Sensor Infor
    MUINT32 u4SensorType;

    // For HW isp gamma
    MUINT32* pIsp_gamma;
    MUINT16 pIsp_gamma_size;

} HDR_PIPE_INIT_INFO, *P_HDR_PIPE_INIT_INFO;

typedef struct
{
    MUINT32 eHdrRound;  //kidd modify to enum
    MUINT32 u4SourceImgWidth;
    MUINT32 u4SourceImgHeight;
    MUINT8* pSourceImgBufAddr[HDR_MAX_INPUT_FRAME]; // source image address
    MUINT8* pSmallImgBufAddr[HDR_MAX_INPUT_FRAME];  // small image address
    MUINT32 manual_PreProcType;
} HDR_PIPE_CONFIG_PARAM, *P_HDR_PIPE_CONFIG_PARAM;

typedef struct
{
    MUINT16 u2SEImgWidth;       // se_image_width;
    MUINT16 u2SEImgHeight;      // se_image_height;
    MUINT8* pSEImgBufAddr[HDR_MAX_INPUT_FRAME]; // se_image_addr;
} HDR_PIPE_SE_INPUT_INFO, *P_HDR_PIPE_SE_INPUT_INFO;

typedef struct
{
    MUINT16 u2SmallImgW;            // input image width
    MUINT16 u2SmallImgH;            // input image height
    MUINT8* pSmallImgBufAddr[HDR_MAX_INPUT_FRAME];  // ImgAddr;
    MUINT8* pWorkingBuffer;

} HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO, *P_HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO;

typedef HDR_BMAP_BUFFER HDR_PIPE_BMAP_BUFFER;

typedef WT_TBL_INFO HDR_PIPE_WEIGHT_TBL_INFO;

typedef HDR_RESULT_STRUCT HDR_PIPE_HDR_RESULT_STRUCT;

// ---------------------------------------------------------------------------

class IHDRHAL
{
public:
    static std::unique_ptr<IHDRHAL> createInstance();

    virtual ~IHDRHAL() = default;

    virtual MBOOL init(void *pInitInData) = 0;

    virtual MBOOL Do_Normalization() = 0;
    virtual MBOOL Do_SE(HDR_PIPE_SE_INPUT_INFO& rHdrPipeSEInputInfo) = 0;
    virtual MBOOL Do_FeatureExtraction(
            HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO& rHdrPipeFeatureExtractInputInfo) = 0;
    virtual MBOOL Do_Alignment() = 0;
    virtual MBOOL Do_Fusion(HDR_PIPE_WEIGHT_TBL_INFO** pprBlurredWeightMapInfo) = 0;
    virtual MBOOL WeightingMapInfoGet(HDR_PIPE_WEIGHT_TBL_INFO** pprWeightMapInfo) = 0;
    virtual MBOOL WeightingMapInfoSet(HDR_PIPE_BMAP_BUFFER* pBmapInfo) = 0;
    virtual MBOOL ResultBufferSet(MUINT8* bufferAddr, MUINT32 bufferSize) = 0;
    virtual MBOOL HdrCroppedResultGet(HDR_PIPE_HDR_RESULT_STRUCT& rHdrCroppedResult) = 0;
    virtual MBOOL HdrSmallImgBufSet(HDR_PIPE_CONFIG_PARAM& rHdrPipeConfigParam) = 0;
    virtual MBOOL HdrWorkingBufSet(MUINT8* u4BufAddr, MUINT32 u4BufSize) = 0;
    virtual MUINT32 HdrWorkingBuffSizeGet() = 0;
    virtual void QuerySmallImgResolution(MUINT32& ru4Width, MUINT32& ru4Height) = 0;
    virtual void QuerySEImgResolution(MUINT32& ru4Width, MUINT32& ru4Height) = 0;
    virtual MUINT32 SEImgBuffSizeGet() = 0;
    virtual void SaveHdrLog(MUINTPTR u4RunningNumber) = 0;
};

} // namespace NSCam

#endif // _IHDRHAL_H_
