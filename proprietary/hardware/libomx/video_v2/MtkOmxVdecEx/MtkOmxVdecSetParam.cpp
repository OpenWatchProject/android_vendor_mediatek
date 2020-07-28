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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxVdecDriver.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Decoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include "MtkOmxVdecEx.h"
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include <ion.h>
#include <ui/Rect.h>

#if (ANDROID_VER >= ANDROID_ICS)
#include <android/native_window.h>
#include <HardwareAPI.h>
//#include <gralloc_priv.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"  //for GRALLOC_USAGE_SECURE
#endif
#include <system/window.h>

OMX_ERRORTYPE MtkOmxVdec::CheckSetParamState()
{
#if (ANDROID_VER >= ANDROID_ICS)
    if ((mState != OMX_StateLoaded) && (mPortReconfigInProgress != OMX_TRUE))
    {
#else
    if (mState != OMX_StateLoaded)
    {
#endif
        return OMX_ErrorIncorrectStateOperation;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::SetInputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pCompParam)
{
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

    UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERCOUNTACTUAL, pCompParam->nBufferCountActual, false);
    UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, pCompParam->nBufferSize, false);
    memcpy(GetInputPortStrut(), pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    if (GetInputPortStrut()->format.video.nFrameWidth * GetInputPortStrut()->format.video.nFrameHeight >= FHD_AREA)
    {
        if (GetInputPortStrut()->nBufferSize < SIZE_512K)
        {
            UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, SIZE_512K, true);
            //GetInputPortStrut()->nBufferSize = SIZE_512K;
        }
    }

    if ((mCodecId == MTK_VDEC_CODEC_ID_VP9)&&(meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE))
    {
        UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_SW, true);
        //GetInputPortStrut()->nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_SW;
    }

    if (GetInputPortStrut()->format.video.nFrameWidth * GetInputPortStrut()->format.video.nFrameHeight > FHD_AREA)
    {
        if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
        {
            if (mIsSecureInst && INHOUSE_TEE == mTeeType)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE_4K, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_AVCSEC_DEFAULT_INPUT_BUFFER_SIZE_4K;
            }
            else if (GetInputPortStrut()->nBufferSize < MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_4K;
            }
        }
        if (mCodecId == MTK_VDEC_CODEC_ID_HEVC)
        {
            if (GetInputPortStrut()->nBufferSize < MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE_4K;
            }
        }
        if (mCodecId == MTK_VDEC_CODEC_ID_HEIF)
        {
            VAL_CHAR_T value[PROPERTY_VALUE_MAX];
            VAL_UINT32_T u4MaxGridCount;
            property_get("vendor.mtk.venc.heif.gridnum", value, "8");
            u4MaxGridCount = (VAL_BOOL_T) atoi(value);

            if (GetInputPortStrut()->nBufferSize < MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE * u4MaxGridCount)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE * u4MaxGridCount, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_HEVC_DEFAULT_INPUT_BUFFER_SIZE * u4MaxGridCount;
            }
        }
        if (mCodecId == MTK_VDEC_CODEC_ID_VP9)
        {
            if (mIsSecureInst && INHOUSE_TEE == mTeeType)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE;
            }
            else if (GetInputPortStrut()->nBufferSize < MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K)
            {
                UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K, true);
                //GetInputPortStrut()->nBufferSize = MTK_VDEC_VP9_DEFAULT_INPUT_BUFFER_SIZE_4K;
            }
        }
    }

    if (OMX_TRUE == mViLTESupportOn)
    {
        SetInputPortViLTE();
    }
    MTK_OMX_LOGD(this, "[INPUT] (%d)(%d)(%d)", GetInputPortStrut()->format.video.nFrameWidth, GetInputPortStrut()->format.video.nFrameHeight, GetInputPortStrut()->nBufferSize);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::SetOutputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pCompParam)
{
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *) pCompParam;


    MTK_OMX_LOGD(this, "--- SetOutputPortDefinition --- (%d %d %d %d %d %d) -> (%d %d %d %d %d %d)",
                     GetOutputPortStrut()->format.video.nFrameWidth, GetOutputPortStrut()->format.video.nFrameHeight,
                     GetOutputPortStrut()->format.video.nStride, GetOutputPortStrut()->format.video.nSliceHeight,
                     GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferSize,
                     pCompParam->format.video.nFrameWidth, pCompParam->format.video.nFrameHeight,
                     pCompParam->format.video.nStride, pCompParam->format.video.nSliceHeight,
                     pCompParam->nBufferCountActual, pCompParam->nBufferSize);


    if (pPortDef->format.video.nFrameWidth == 0 || pPortDef->format.video.nFrameHeight == 0)
    {
        MTK_OMX_LOGE(this, "SetParameter bad parameter width/height is 0");
        return OMX_ErrorBadParameter;
    }

    OMX_VIDEO_PORTDEFINITIONTYPE *pstOMXVideoPortDef = &(GetOutputPortStrut()->format.video);

    if ((pstOMXVideoPortDef->nFrameWidth == pPortDef->format.video.nFrameWidth) &&
        (pstOMXVideoPortDef->nFrameHeight == pPortDef->format.video.nFrameHeight))
    {
        if (pPortDef->nBufferCountActual - GetOutputPortStrut()->nBufferCountActual > mStarvationSize)
        {
            return OMX_ErrorBadParameter;
        }
        else
        {
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, pCompParam->format.video.nFrameWidth, false);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, pCompParam->format.video.nFrameHeight, false);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, pCompParam->format.video.nStride, false);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, pCompParam->format.video.nSliceHeight, false);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, pCompParam->nBufferCountActual, false);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, pCompParam->nBufferCountMin, false);
            memcpy(GetOutputPortStrut(), pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

            if (OMX_TRUE == mDecoderInitCompleteFlag)
            {
                return OMX_ErrorNone; // return ok
            }
            else
            {
                MTK_OMX_LOGD(this, "keep going before decoder init");
            }
        }
    }
    else if ((pstOMXVideoPortDef->nFrameWidth != pPortDef->format.video.nFrameWidth) ||
             (pstOMXVideoPortDef->nFrameHeight != pPortDef->format.video.nFrameHeight))
    {
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, pCompParam->format.video.nFrameWidth, false);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, pCompParam->format.video.nFrameHeight, false);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, pCompParam->format.video.nStride, false);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, pCompParam->format.video.nSliceHeight, false);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, pCompParam->nBufferCountActual, false);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, pCompParam->nBufferCountMin, false);
        memcpy(GetOutputPortStrut(), pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    }
    else   // for error case
    {
        MTK_OMX_LOGE(this, "OMX_IndexParamPortDefinition for output buffer error");
        return OMX_ErrorBadParameter;
    }


    MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));

    if (OMX_FALSE == QueryDriverFormat(&mQInfoOut))
    {
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, mQInfoOut.u4Width, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, mQInfoOut.u4Height, true);
        //pstOMXVideoPortDef->nStride         = mQInfoOut.u4Width;
        //pstOMXVideoPortDef->nSliceHeight    = mQInfoOut.u4Height;
        MTK_OMX_LOGD(this, "Unsupported Video Resolution (%d, %d), MAX(%d, %d)", pstOMXVideoPortDef->nFrameWidth, pstOMXVideoPortDef->nFrameHeight, mQInfoOut.u4Width, mQInfoOut.u4Height);

        return OMX_ErrorBadParameter;
    }
    else
    {
        //for MTK VIDEO 4KH264 SUPPORT  // //query output color format again avoid wrong format
        SetColorFormatbyDrvType(mQInfoOut.ePixelFormat);

        if (mLegacyMode == OMX_TRUE)
        {
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mMaxWidth, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mMaxHeight, true);
            //pstOMXVideoPortDef->nFrameWidth     = mMaxWidth;
            //pstOMXVideoPortDef->nFrameHeight    = mMaxHeight;
        }
        if (OMX_TRUE == needColorConvert())
        {
            // set stride/sliceHeight to crop size (not aligned) if need color convert
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, pstOMXVideoPortDef->nFrameWidth, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, pstOMXVideoPortDef->nFrameHeight, true);
        }
        else
        {
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, VDEC_ROUND_N(pstOMXVideoPortDef->nFrameWidth, mQInfoOut.u4StrideAlign), true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, VDEC_ROUND_N(pstOMXVideoPortDef->nFrameHeight, mQInfoOut.u4SliceHeightAlign), true);
            //pstOMXVideoPortDef->nStride             = VDEC_ROUND_N(pstOMXVideoPortDef->nFrameWidth, mQInfoOut.u4StrideAlign);
            //pstOMXVideoPortDef->nSliceHeight    	= VDEC_ROUND_N(pstOMXVideoPortDef->nFrameHeight, mQInfoOut.u4SliceHeightAlign);
        }
        MTK_OMX_LOGD(this, " Video Resolution width/height(%d, %d), nStride/nSliceHeight(%d, %d), StrideAlign/SliceHeightAlign(%d, %d)",
                     pstOMXVideoPortDef->nFrameWidth, pstOMXVideoPortDef->nFrameHeight,
                     pstOMXVideoPortDef->nStride, pstOMXVideoPortDef->nSliceHeight,
                     mQInfoOut.u4StrideAlign,  mQInfoOut.u4SliceHeightAlign);
    }


#if (ANDROID_VER >= ANDROID_KK)
    mCropLeft = 0;
    mCropTop = 0;
    mCropWidth = GetOutputPortStrut()->format.video.nFrameWidth;
    mCropHeight = GetOutputPortStrut()->format.video.nFrameHeight;
    MTK_OMX_LOGD(this, "mCropWidth : %d , mCropHeight : %d", mCropWidth, mCropHeight);
#endif

    if (mSetOutputColorFormat == OMX_COLOR_Format32BitRGBA8888)
    {
        GetOutputPortStrut()->nBufferSize = (pstOMXVideoPortDef->nStride * pstOMXVideoPortDef->nSliceHeight * 4) + 16;
    }
    else if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE)
    {
        GetOutputPortStrut()->nBufferSize = (pstOMXVideoPortDef->nStride * pstOMXVideoPortDef->nSliceHeight * 3 >> 1) + 16;
    }
    else
    {
        GetOutputPortStrut()->nBufferSize = (pstOMXVideoPortDef->nStride * (pstOMXVideoPortDef->nSliceHeight + 1) * 3) >> 1;
    }

    if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingMJPEG)
    {
        SetupBufferInfoForMJPEG();
    }

    MTK_OMX_LOGD(this, "nStride = %d,  nSliceHeight = %d, PicSize = %d, ori_nBufferCountActual = %d", GetOutputPortStrut()->format.video.nStride, GetOutputPortStrut()->format.video.nSliceHeight,
                 GetOutputPortStrut()->nBufferSize, GetOutputPortStrut()->nBufferCountActual);

    if(OMX_FALSE == mSeqInfoCompleteFlag)
    {
    	switch(mInputPortFormat.eCompressionFormat)
    	{
    		case OMX_VIDEO_CodingAVC:
    		{
    	    	SetupBufferInfoForH264();
    		}
    		break;

    	    case OMX_VIDEO_CodingHEVC:
    	    case OMX_VIDEO_CodingImageHEIC:
    		{
    	    	SetupBufferInfoForHEVC();
    		}
    		break;

    		default:
    			break;
    	}
    }
    SetupBufferInfoForCrossMount();

    return OMX_ErrorNone;
}

void MtkOmxVdec::SetColorFormatbyDrvType(int ePixelFormat)
{
    switch (ePixelFormat)
    {
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            MTK_OMX_LOGE(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
            if (mIsInterlacing)
            {
                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
            }
            else
            {
                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
            }
            MTK_OMX_LOGE(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            MTK_OMX_LOGE(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_YV12");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
            mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            GetOutputPortStrut()->format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO");
            break;
        case VDEC_DRV_PIXEL_FORMAT_YUV_NV12:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
            MTK_OMX_LOGUD("SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_NV12");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP");
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP");
            break;
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO:
            mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
            GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
            MTK_OMX_LOGD(this, "SetColorFormatbyDrvType: VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO");
            break;

        default:
            break;
    }
}

void MtkOmxVdec::SetupBufferInfoForMJPEG()
{
    //for MTK SUPPORT MJPEG [
    //if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingMJPEG)
    {
        GetOutputPortStrut()->nBufferSize = (GetOutputPortStrut()->format.video.nStride * GetOutputPortStrut()->format.video.nSliceHeight) << 2;
        if (mThumbnailMode == OMX_TRUE)
        {
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, 2, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, 2, true);
            //GetOutputPortStrut()->nBufferCountActual = 2;
            //GetOutputPortStrut()->nBufferCountMin = 2;
        }
    }
    //] MJPEG
}

void MtkOmxVdec::SetupBufferInfoForH264()
{
    int32_t MaxDPBSize, PicSize, MaxDPBNumber, ActualOutBuffNums, ActualOutBuffSize;
    OMX_VIDEO_PORTDEFINITIONTYPE *pstOMXVideoPortDef = &(GetOutputPortStrut()->format.video);

    if (mSeqInfoCompleteFlag == OMX_FALSE)
    {
        //MaxDPBSize = 6912000; // level 3.1
        MaxDPBSize = 70778880; // level 5.1
        PicSize = ((GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) * 3) >> 1;
        MaxDPBNumber = MaxDPBSize / PicSize;
    }
    else
    {
        MaxDPBNumber = mDPBSize;
    }

    MaxDPBNumber = (((MaxDPBNumber) < (16)) ? MaxDPBNumber : 16);

    if (mLegacyMode == OMX_TRUE)
    {
        MaxDPBNumber = 16;
		MTK_OMX_LOGD(this, "***mLegacyMode***(%d), MaxDPBNumber(%d)", __LINE__, MaxDPBNumber);
    }
    ActualOutBuffNums = MaxDPBNumber + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD; // for some H.264 baseline with other nal headers

    UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, ActualOutBuffNums, true);
    UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, ActualOutBuffNums - mMinUndequeuedBufs, true);
    //GetOutputPortStrut()->nBufferCountActual = ActualOutBuffNums;
    //GetOutputPortStrut()->nBufferCountMin = ActualOutBuffNums - mMinUndequeuedBufs;

	MTK_OMX_LOGD(this, "(%s)@(%d), GetOutputPortStrut()->nBufferCountActual(%d), GetOutputPortStrut()->nBufferCountMin(%d)", __FUNCTION__, __LINE__, GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferCountMin);

    MTK_OMX_LOGD(this, "mSeqInfoCompleteFlag %d, mLegacyMode = %d, mIsUsingNativeBuffers %d",
                 mSeqInfoCompleteFlag, mLegacyMode, mIsUsingNativeBuffers);

    if ((mThumbnailMode == OMX_TRUE)||((mSeqInfoCompleteFlag == OMX_FALSE) && (mLegacyMode == OMX_FALSE)))
    {
        // v4l2 todo: no port-reconfig. need to refine the initial buffer count
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT-mMinUndequeuedBufs, true);
        //GetOutputPortStrut()->nBufferCountActual = 3 + mMinUndequeuedBufs;//16 + 3 + mMinUndequeuedBufs + MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;  //For initHW get DPB size and free
        //GetOutputPortStrut()->nBufferCountMin = 3;//16 + 3 + MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
        MTK_OMX_LOGD(this, "(%s)@(%d), GetOutputPortStrut()->nBufferCountActual(%d), GetOutputPortStrut()->nBufferCountMin(%d)", __FUNCTION__, __LINE__, GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferCountMin);


        /* Workaround to make a little change on the crop size, so that we won't fail
                               in the formatHasNotablyChanged when format changed was sent.
                          */
        //mCropWidth --; // for output format change
        //mCropWidth -= GetOutputPortStrut()->format.video.nFrameWidth > 32 ? 32 : 1;
        //MTK_OMX_LOGD(this, "Force port re-config for actual output buffer size!!");
    }

    MTK_OMX_LOGD(this, "MaxDPBNumber = %d,  OutputBuffers = %d (%d) ,PicSize=%d", MaxDPBNumber, ActualOutBuffNums, GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferSize);

}

void MtkOmxVdec::SetupBufferInfoForHEVC()
{
    int32_t PicSize, MaxDPBNumber, ActualOutBuffNums, ActualOutBuffSize;
    OMX_VIDEO_PORTDEFINITIONTYPE *pstOMXVideoPortDef = &(GetOutputPortStrut()->format.video);

    PicSize = ((pstOMXVideoPortDef->nFrameWidth * pstOMXVideoPortDef->nFrameHeight) * 3) >> 1;
    MaxDPBNumber = mDPBSize;
    //MaxDPBNumber = (((MaxDPBNumber) < (16)) ? MaxDPBNumber : 16);
    ActualOutBuffNums = MaxDPBNumber + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD; // for some HEVC baseline with other nal headers

    //#endif
    UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, ActualOutBuffNums, true);
    UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, ActualOutBuffNums - mMinUndequeuedBufs, true);
    //GetOutputPortStrut()->nBufferCountActual = ActualOutBuffNums;
    //GetOutputPortStrut()->nBufferCountMin = ActualOutBuffNums - mMinUndequeuedBufs;
	MTK_OMX_LOGD(this, "(%s)@(%d), GetOutputPortStrut()->nBufferCountActual(%d), GetOutputPortStrut()->nBufferCountMin(%d)", __FUNCTION__, __LINE__, GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferCountMin);

    if ( (mThumbnailMode == OMX_TRUE) || (mSeqInfoCompleteFlag == OMX_FALSE))
    {
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT-mMinUndequeuedBufs, true);
        //GetOutputPortStrut()->nBufferCountActual = 2 + mMinUndequeuedBufs;  //For initHW get DPB size and free
        //GetOutputPortStrut()->nBufferCountMin = 2;
    }

    MTK_OMX_LOGD(this, "MaxDPBNumber = %d,  OutputBuffers = %d (%d) ,PicSize=%d", MaxDPBNumber, ActualOutBuffNums, GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferSize);
}

void MtkOmxVdec::SetupBufferInfoForCrossMount()
{
    if (/*(OMX_FALSE == mIsUsingNativeBuffers) &&*/ (OMX_TRUE == mCrossMountSupportOn))
    {
        //if( CROSSMOUNT_MAX_COLORCONVERT_OUTPUTBUFFER_COUNT == mMaxColorConvertOutputBufferCnt )
        {
            //for port reconfig
            mMaxColorConvertOutputBufferCnt = (GetOutputPortStrut()->nBufferCountActual / 2);
        }
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, GetOutputPortStrut()->nBufferCountActual+mMaxColorConvertOutputBufferCnt, true);
        //GetOutputPortStrut()->nBufferCountActual += mMaxColorConvertOutputBufferCnt;
        MTK_OMX_LOGD(this, "original nBufferCountActual after adjust = %d(+%d) ",
                     GetOutputPortStrut()->nBufferCountActual, mMaxColorConvertOutputBufferCnt);
    }
}

OMX_ERRORTYPE MtkOmxVdec::HandleSetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pCompParam)
{
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;
    OMX_ERRORTYPE err;

    if (!checkOMXParams(pPortDef)) {
        MTK_OMX_LOGE(this, "invalid OMX header OMX_IndexParamPortDefinition");
        return OMX_ErrorBadParameter;
    }

    if (pPortDef->nBufferCountActual > MAX_TOTAL_BUFFER_CNT) {
        MTK_OMX_LOGE(this, "not support buffer count %u", pPortDef->nBufferCountActual);
        return OMX_ErrorBadParameter;
    }

    if (GetInputPortStrut()->nPortIndex == pPortDef->nPortIndex)
    {
        err = SetInputPortDefinition(pCompParam);
        return err;
    }
    else if (GetOutputPortStrut()->nPortIndex == pPortDef->nPortIndex)
    {
        err = SetOutputPortDefinition(pCompParam);
        return err;
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::HandleSetVideoPortFormat(OMX_PARAM_PORTDEFINITIONTYPE *pCompParam)
{
    OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam;

    if (!checkOMXParams(pPortFormat)) {
        MTK_OMX_LOGE(this, "invalid OMX header OMX_IndexParamVideoPortFormat");
        return OMX_ErrorBadParameter;
    }

    if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
    {
        // TODO: should we allow setting the input port param?
    }
    else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        MTK_OMX_LOGD(this, "Set Output eColorFormat before %x, after %x", mOutputPortFormat.eColorFormat, pPortFormat->eColorFormat);
        //keep original format for CTS Vp8EncoderTest and GTS Vp8CocecTest, sw decode YV12 -> I420 -> set to OMX component
        //L-MR1 change the flow to preference the flexible format if hasNativeWindow=true but component can not support flexible format
        //framework will do SW render(clear NW), OMX component will do color convert to I420 for this SW rendering
        if (mOutputPortFormat.eColorFormat != pPortFormat->eColorFormat)
        {
            mOutputPortFormat.eColorFormat = pPortFormat->eColorFormat;
        }
        else
        {
            //CTS DecodeEditEncodeTest, Surface for output config in mediacodec will cause flexibleYUV flow enable.
            //acodec log: does not support color format 7f000789, err 80000000
            if (OMX_TRUE == mbYUV420FlexibleMode)
            {
                MTK_OMX_LOGD(this, "Disable FlexibleYUV Flow for surface format, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface in certification test case");
                mbYUV420FlexibleMode = OMX_FALSE;
            }
        }
        mSetOutputColorFormat = pPortFormat->eColorFormat;
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

void MtkOmxVdec::HandleSetMtkOmxVdecThumbnailMode()
{
    MTK_OMX_LOGD(this, "Set thumbnail mode enable %06x", this);
    mThumbnailMode = OMX_TRUE;
    if ((mCodecId == MTK_VDEC_CODEC_ID_VPX) ||
        (mCodecId == MTK_VDEC_CODEC_ID_VP9) ||
        (mCodecId == MTK_VDEC_CODEC_ID_VC1) ||
        (mCodecId == MTK_VDEC_CODEC_ID_MPEG2) ||
        (mCodecId == MTK_VDEC_CODEC_ID_MPEG4))
    {
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT, true);
        //GetOutputPortStrut()->nBufferCountActual = MTK_VDEC_THUMBNAIL_OUTPUT_BUFFER_COUNT;
        MTK_OMX_LOGD(this, "SetParameter OMX_IndexVendorMtkOmxVdecThumbnailMode GetOutputPortStrut()->nBufferCountActual %d", GetOutputPortStrut()->nBufferCountActual);
    }
}

void MtkOmxVdec::HandleSetMtkOmxVdecUseClearMotion()
{
#if 0
    if (mMJCEnable == OMX_TRUE)
    {
        if (bOmxMJCLogEnable)
        {
            MTK_OMX_LOGD(this, "[MJC] ======== Use ClearMotion ========");
        }

        if (OMX_TRUE == mIsSecureInst)
        {
            MTK_OMX_LOGD(this, "BYPASS MJC for SVP");
            mUseClearMotion = OMX_FALSE;
            mMJCReconfigFlag = OMX_FALSE;
            MJC_MODE mMode;
            mMode = MJC_MODE_BYPASS;
            m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
            return;
        }


        if (mUseClearMotion == OMX_FALSE)
        {
            if ((GetOutputPortStrut()->nBufferCountActual + FRAMEWORK_OVERHEAD + mMinUndequeuedBufs + TOTAL_MJC_BUFFER_CNT) <= MAX_TOTAL_BUFFER_CNT) // Jimmy temp
            {
                MTK_OMX_LOGD(this, "[MJC] (1)\n");

                MJC_MODE mMode;
                if (mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC || mInputPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)
                {
                    mMJCReconfigFlag = OMX_TRUE;    //MJCReconfigFlag for port reconfig can reopen MJC
                    mUseClearMotion = OMX_TRUE;
                    mMode = MJC_MODE_NORMAL;
                }
                else if (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight > 1920 * 1088)
                {
                    GetOutputPortStrut()->nBufferCountActual += (FRAMEWORK_OVERHEAD + mMinUndequeuedBufs);
                    GetOutputPortStrut()->nBufferCountMin += (FRAMEWORK_OVERHEAD + mMinUndequeuedBufs);
                    mMJCReconfigFlag = OMX_FALSE;
                    mUseClearMotion = OMX_FALSE;
                    mMode = MJC_MODE_BYPASS;
                }
                else
                {
                    GetOutputPortStrut()->nBufferCountActual += (FRAMEWORK_OVERHEAD + TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs); // Jimmy temp
                    GetOutputPortStrut()->nBufferCountMin += (FRAMEWORK_OVERHEAD + TOTAL_MJC_BUFFER_CNT + mMinUndequeuedBufs); // Jimmy temp
                    mMJCReconfigFlag = OMX_FALSE;
                    mUseClearMotion = OMX_TRUE;
                    mMode = MJC_MODE_NORMAL;
                }

                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
            }
            else
            {
                MTK_OMX_LOGD(this, "[MJC] (0)\n");
                mUseClearMotion = OMX_FALSE;
                mMJCReconfigFlag = OMX_FALSE;
                MJC_MODE mMode;
                mMode = MJC_MODE_BYPASS;
                m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &mMode);
            }
        }
        else
        {
            MTK_OMX_LOGD(this, "[MJC] (2)\n");
        }

        MTK_OMX_LOGD(this, "[MJC] buf %d\n", GetOutputPortStrut()->nBufferCountActual);
    }
#endif
}

void MtkOmxVdec::HandleMinUndequeuedBufs(VAL_UINT32_T* pCompParam)
{
    mMinUndequeuedBufsDiff = *((VAL_UINT32_T *)pCompParam) - mMinUndequeuedBufs;
    mMinUndequeuedBufs = *((VAL_UINT32_T *)pCompParam);

    if (mMinUndequeuedBufs > MAX_MIN_UNDEQUEUED_BUFS)
    {
        MTK_OMX_LOGD(this, "[MJC][ERROR] mMinUndequeuedBufs : %d\n", mMinUndequeuedBufs);
    }

    if ((mMinUndequeuedBufsFlag == OMX_FALSE) && (GetOutputPortStrut()->nBufferCountActual + mMinUndequeuedBufsDiff <= MAX_TOTAL_BUFFER_CNT))
    {
        //GetOutputPortStrut()->nBufferCountActual += mMinUndequeuedBufsDiff;
        mMinUndequeuedBufsFlag = OMX_TRUE;
    }

    MTK_OMX_LOGD(this, "[MJC] mMinUndequeuedBufs : %d (%i, %d)\n", mMinUndequeuedBufs, mMinUndequeuedBufsDiff, GetOutputPortStrut()->nBufferCountActual);
}

OMX_ERRORTYPE MtkOmxVdec::HandleEnableAndroidNativeBuffers(OMX_PTR pCompParam)
{
    EnableAndroidNativeBuffersParams *pEnableNativeBuffers = (EnableAndroidNativeBuffersParams *) pCompParam;
    if ((NULL == pEnableNativeBuffers) || (pEnableNativeBuffers->nPortIndex != mOutputPortFormat.nPortIndex))
    {
        MTK_OMX_LOGD(this, "@@ OMX_GoogleAndroidIndexEnableAndroidNativeBuffers: invalid port index");
        return OMX_ErrorBadParameter;
    }

    if (NULL != pEnableNativeBuffers)
    {
        MTK_OMX_LOGD(this, "OMX_GoogleAndroidIndexEnableAndroidNativeBuffers enable(%d)", pEnableNativeBuffers->enable);
        mIsUsingNativeBuffers = pEnableNativeBuffers->enable;
    }
    else
    {
        MTK_OMX_LOGE(this, "@@ OMX_GoogleAndroidIndexEnableAndroidNativeBuffers: pEnableNativeBuffers is NULL");
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::UseAndroidNativeBuffer_CheckSecureBuffer(OMX_HANDLETYPE hComp, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err;

    UseAndroidNativeBufferParams *pUseNativeBufferParams = (UseAndroidNativeBufferParams *)pCompParam;

    sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
    buffer_handle_t _handle = nBuf->handle;
    int secure_buffer_handle;

    size_t bufferSize = (nBuf->stride * nBuf->height * 3 >> 1);
    if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
    {
        bufferSize = (nBuf->stride * (nBuf->height + 1) * 3 >> 1);
    }
    //GraphicBufferMapper::getInstance().getSecureBuffer(_handle, &buffer_type, &secure_buffer_handle);
    mSecFrmBufInfo[mSecFrmBufCount].pNativeHandle = (void *)_handle;

#if SECURE_OUTPUT_USE_ION
    int ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);

    if (ret < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR]newOmxMVAwithHndl() failed");
    }

    VBufInfo BufInfo;
    if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
        return OMX_ErrorUndefined;
    }

    //get secure handle from ion
    secure_buffer_handle = BufInfo.u4PA;
    MTK_OMX_LOGD(this, "child Physical address = 0x%x, len = %d\n", BufInfo.u4PA, BufInfo.u4BuffSize);

#else
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &secure_buffer_handle);
#endif
    MTK_OMX_LOGD(this, "@@ _handle(0x%08X), secure_buffer_handle(0x%08X)", _handle, secure_buffer_handle);

    mOutputUseION = OMX_FALSE;
    mSecFrmBufInfo[mSecFrmBufCount].u4BuffId        = secure_buffer_handle;
    mSecFrmBufInfo[mSecFrmBufCount].u4SecHandle = secure_buffer_handle;
    mSecFrmBufInfo[mSecFrmBufCount].u4BuffSize  = bufferSize;

    err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, bufferSize, (OMX_U8 *)secure_buffer_handle);

    if (err != OMX_ErrorNone)
    {
        MTK_OMX_LOGE(this, "[ERROR] UseBuffer failed, LINE:%d", __LINE__);
        return err;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::HandleUseIonBuffer(OMX_HANDLETYPE hComp, UseIonBufferParams *pUseIonBufferParams)
{
    OMX_ERRORTYPE err;

    //MTK_OMX_LOGD ("@@ OMX_IndexVendorMtkOmxVencUseIonBuffer");
    //UseIonBufferParams *pUseIonBufferParams = (UseIonBufferParams *)pCompParam;

    if (pUseIonBufferParams->nPortIndex == mInputPortFormat.nPortIndex)
    {
        //MTK_OMX_LOGD ("@@ OMX_IndexVendorMtkOmxVideoUseIonBuffer for port[%d]", pUseIonBufferParams->nPortIndex);
        mInputUseION = OMX_TRUE;
    }
    if (pUseIonBufferParams->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        //MTK_OMX_LOGD ("@@ OMX_IndexVendorMtkOmxVideoUseIonBuffer for port[%d]", pUseIonBufferParams->nPortIndex);
        mOutputUseION = OMX_TRUE;
    }

    MTK_OMX_LOGD(this, "@@ mIsClientLocally(%d), mIsFromGralloc(%d), VA(0x%08X), FD(%d), size(%d)", mIsClientLocally, mIsFromGralloc, pUseIonBufferParams->virAddr, pUseIonBufferParams->Ionfd, pUseIonBufferParams->size);
#if USE_MVA_MANAGER
    int ret;
    OMX_U8 *buffer;
    size_t bufferSize = pUseIonBufferParams->size;
    if (pUseIonBufferParams->nPortIndex == mInputPortFormat.nPortIndex)
    {
        ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, pUseIonBufferParams->size, NULL, (void **)&buffer);
    }
    else
    {
        ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, pUseIonBufferParams->size, NULL, (void **)&buffer);
    }

    if (ret < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR][ION]Allocate Node failed,line:%d\n", __LINE__);
        return OMX_ErrorUndefined;
    }

#else   //USE_MVA_MANAGER
    if (-1 == mIonDevFd)
    {
        mIonDevFd = mt_ion_open("MtkOmxVdec3");
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR] cannot open ION device.");
            err = OMX_ErrorUndefined;
            return err;
        }
    }
    if (ion_import(mIonDevFd, pUseIonBufferParams->Ionfd, (pUseIonBufferParams->nPortIndex == mOutputPortFormat.nPortIndex ? &mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle : &mIonInputBufferInfo[mIonInputBufferCount].pIonBufhandle)))
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_import failed");
    }
    int share_fd;
    if (ion_share(mIonDevFd, (pUseIonBufferParams->nPortIndex == mOutputPortFormat.nPortIndex ? mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle : mIonInputBufferInfo[mIonInputBufferCount].pIonBufhandle), &share_fd))
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_share failed");
    }
    // map virtual address
    size_t bufferSize = pUseIonBufferParams->size;
    OMX_U8 *buffer = (OMX_U8 *) ion_mmap(mIonDevFd, NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, share_fd, 0);
    if ((buffer == NULL) || (buffer == (void *) - 1))
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_mmap failed");
        err = OMX_ErrorUndefined;
        return err;
    }

    //MTK_OMX_LOGD ("pBuffer(0x%08x), ion_buf_handle (0x%08X)", buffer, mIonBufferInfo[mIonBufferCount].pIonBufhandle);
    MTK_OMX_LOGD(this, "u4OriVA (0x%08X), u4VA(0x%08X)", pUseIonBufferParams->virAddr, buffer);

    // configure buffer
    ConfigIonBuffer(mIonDevFd, (pUseIonBufferParams->nPortIndex == mOutputPortFormat.nPortIndex ? mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle : mIonInputBufferInfo[mIonInputBufferCount].pIonBufhandle));
    if (pUseIonBufferParams->nPortIndex == mInputPortFormat.nPortIndex)
    {
        mIonInputBufferInfo[mIonInputBufferCount].u4OriVA = (VAL_UINT32_T)pUseIonBufferParams->virAddr;
        mIonInputBufferInfo[mIonInputBufferCount].ori_fd = pUseIonBufferParams->Ionfd;
        mIonInputBufferInfo[mIonInputBufferCount].fd = share_fd;
        mIonInputBufferInfo[mIonInputBufferCount].u4VA = (VAL_UINT32_T)buffer;
        mIonInputBufferInfo[mIonInputBufferCount].u4PA = GetIonPhysicalAddress(mIonDevFd, mIonInputBufferInfo[mIonInputBufferCount].pIonBufhandle);
        mIonInputBufferInfo[mIonInputBufferCount].u4BuffSize = bufferSize;
    }
    else
    {
        mIonOutputBufferInfo[mIonOutputBufferCount].u4OriVA = (VAL_UINT32_T)pUseIonBufferParams->virAddr;
        mIonOutputBufferInfo[mIonOutputBufferCount].ori_fd = pUseIonBufferParams->Ionfd;
        mIonOutputBufferInfo[mIonOutputBufferCount].fd = share_fd;
        mIonOutputBufferInfo[mIonOutputBufferCount].u4VA = (VAL_UINT32_T)buffer;
        mIonOutputBufferInfo[mIonOutputBufferCount].u4PA = GetIonPhysicalAddress(mIonDevFd, mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle);
        mIonOutputBufferInfo[mIonOutputBufferCount].u4BuffSize = bufferSize;
    }
#endif  //USE_MVA_MANAGER
    err = UseBuffer(hComp, pUseIonBufferParams->bufferHeader, pUseIonBufferParams->nPortIndex, pUseIonBufferParams->pAppPrivate, bufferSize, buffer);

    if (err != OMX_ErrorNone)
    {
        MTK_OMX_LOGE(this, "[ERROR] UseBuffer failed");
        err = OMX_ErrorUndefined;
        return err;
    }

    return OMX_ErrorNone;
}



OMX_ERRORTYPE MtkOmxVdec::UseAndroidNativeBuffer_CheckNormalBuffer(OMX_HANDLETYPE hComp, OMX_PTR pCompParam)
{
    UseAndroidNativeBufferParams *pUseNativeBufferParams = (UseAndroidNativeBufferParams *)pCompParam;

    OMX_ERRORTYPE err;

    sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
    buffer_handle_t _handle = nBuf->handle;
    native_handle_t *pGrallocHandle = (native_handle_t *) _handle;
    int ion_buf_fd = -1;

    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &ion_buf_fd);

    if ((-1 < ion_buf_fd) && (mCodecId != MTK_VDEC_CODEC_ID_MJPEG))
    {
        // fd from gralloc and no need to free it in FreeBuffer function
        mIsFromGralloc = OMX_TRUE;
        // use ION
        mOutputUseION = OMX_TRUE;
    }
    else
    {
        // use M4U
        mOutputUseION = OMX_FALSE;
#if USE_MVA_MANAGER
        if (mOutputMVAMgr != NULL)
        {
            delete mOutputMVAMgr;
            mOutputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec2");
        }
#endif
    }

    if (OMX_TRUE == mOutputUseION)
    {
        MTK_OMX_LOGD(this, "buffer_handle_t(0x%08x), ionFd(0x%08X)", _handle, ion_buf_fd);

        if (OMX_TRUE != mIsUsingNativeBuffers)
        {
            MTK_OMX_LOGE(this, "[ERROR] OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
            err = OMX_ErrorBadParameter;
            return err;
        }
#if !USE_MVA_MANAGER
        if (-1 == mIonDevFd)
        {
            mIonDevFd = mt_ion_open("MtkOmxVdec2");

            if (mIonDevFd < 0)
            {
                MTK_OMX_LOGE(this, "[ERROR] cannot open ION device.");
                err = OMX_ErrorUndefined;
                return err;
            }
        }

        if (ion_import(mIonDevFd, ion_buf_fd, &mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle))
        {
            MTK_OMX_LOGE(this, "[ERROR] ion_import failed");
        }
        int share_fd;
        if (ion_share(mIonDevFd, mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle, &share_fd))
        {
            MTK_OMX_LOGE(this, "[ERROR] ion_share failed");
        }

        mIonOutputBufferInfo[mIonOutputBufferCount].pNativeHandle = (void *)_handle;
#endif  //!USE_MVA_MANAGER
        // map virtual address
        size_t bufferSize = (nBuf->stride * nBuf->height * 3 >> 1);
        //fore MTK VIDEO 4KH264 SUPPORT [
        if (MTK_VDEC_CODEC_ID_AVC == mCodecId)
        {
            // query output color format and stride and sliceheigt
            VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
            MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
            if (OMX_TRUE == QueryDriverFormat(&qinfoOut))
            {
                meDecodeType = qinfoOut.eDecodeType;
                MTK_OMX_LOGD(this, " AVC meDecodeType = %d", meDecodeType);
            }
            else
            {
                MTK_OMX_LOGE(this, "[ERROR] AVC QueryDriverFormat failed");
            }
        }
        //] 4KH264
        if (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE)
        {
            bufferSize = (nBuf->stride * (nBuf->height + 1) * 3 >> 1);
        }

#if USE_MVA_MANAGER
        int size, ret;
        OMX_U8 *buffer;
        gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);

        if (bufferSize > size)
        {
            MTK_OMX_LOGE(this, "[ERROR] ion_mmap size(%d) < buffer Size(%d)", size, bufferSize);
        }
        ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);

        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR]newOmxMVAwithHndl() failed");
        }

        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            err = OMX_ErrorUndefined;
            return err;
        }

        buffer = (OMX_U8 *)BufInfo.u4VA;

#else   //USE_MVA_MANAGER
        OMX_U8 *buffer = (OMX_U8 *) ion_mmap(mIonDevFd, NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, share_fd, 0);
        if ((buffer == NULL) || (buffer == (void *) - 1))
        {
            MTK_OMX_LOGE(this, "[ERROR] ion_mmap failed");
            err = OMX_ErrorUndefined;
            return err;
        }

        MTK_OMX_LOGD(this, "share_fd(0x%08X), pBuffer(0x%08x), ion_buf_handle (0x%08X), mIsClientLocally(%d), bufferSize(%d)",
                     share_fd, buffer, mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle, mIsClientLocally, bufferSize);

        // configure buffer
        ConfigIonBuffer(mIonDevFd, mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle);
        mIonOutputBufferInfo[mIonOutputBufferCount].ori_fd = ion_buf_fd;
        mIonOutputBufferInfo[mIonOutputBufferCount].fd = share_fd;
        mIonOutputBufferInfo[mIonOutputBufferCount].u4OriVA = (VAL_UINT32_T)buffer;
        mIonOutputBufferInfo[mIonOutputBufferCount].u4VA = (VAL_UINT32_T)buffer;
        mIonOutputBufferInfo[mIonOutputBufferCount].u4PA = GetIonPhysicalAddress(mIonDevFd, mIonOutputBufferInfo[mIonOutputBufferCount].pIonBufhandle);
        mIonOutputBufferInfo[mIonOutputBufferCount].u4BuffSize = bufferSize;
#endif  //USE_MVA_MANAGER

        err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, bufferSize, (OMX_U8 *)buffer);

        if (err != OMX_ErrorNone)
        {
            MTK_OMX_LOGE(this, "[ERROR] UseBuffer failed");
            err = OMX_ErrorUndefined;
            return err;
        }
    }
    else     // M4U
    {
        sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
        buffer_handle_t _handle = nBuf->handle;
        //MTK_OMX_LOGD ("@@ buffer_handle_t(0x%08x)", _handle);

        //IMG_native_handle_t *pGrallocHandle = (IMG_native_handle_t*) _handle;
        native_handle_t *pGrallocHandle = (native_handle_t *) _handle;
        //MTK_OMX_LOGD ("0x%08x iFormat(0x%08X)(%d)(%d)", _handle, pGrallocHandle->iFormat, pGrallocHandle->iWidth, pGrallocHandle->iHeight);
        //MTK_OMX_LOGD ("0x%08x width(%d), height(%d), stride(%d)", _handle, nBuf->width, nBuf->height, nBuf->stride);
        if (OMX_TRUE != mIsUsingNativeBuffers)
        {
            MTK_OMX_LOGE(this, "@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
            err = OMX_ErrorBadParameter;
            return err;
        }

        //LOGD ("@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer");
        OMX_U8 *buffer;
        GraphicBufferMapper &gbm = GraphicBufferMapper::getInstance();
        int iRet;
        iRet = gbm.lock(_handle, GRALLOC_USAGE_SW_READ_OFTEN, Rect(nBuf->width, nBuf->height), (void **)&buffer);
        if (iRet != 0)
        {
            MTK_OMX_LOGE(this, "gbm->lock(...) failed: %d", iRet);
            return OMX_ErrorNone;
        }

        //LOGD ("@@ buffer(0x%08x)", buffer);
        err = UseBuffer(hComp, pUseNativeBufferParams->bufferHeader, pUseNativeBufferParams->nPortIndex, pUseNativeBufferParams->pAppPrivate, nBuf->stride * nBuf->height * 3 >> 1, buffer);

        iRet = gbm.unlock(_handle);
        if (iRet != 0)
        {
            MTK_OMX_LOGE(this, "gbm->unlock() failed: %d", iRet);
            return OMX_ErrorNone;
        }
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::HandleUseAndroidNativeBuffer(OMX_HANDLETYPE hComp, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err;

    // Bruce Hsu 20120329 cancel workaround
    // Morris Yang 20111128 temp workaround for ANativeWindowBuffer
    UseAndroidNativeBufferParams *pUseNativeBufferParams = (UseAndroidNativeBufferParams *)pCompParam;
    //UseAndroidNativeBufferParams3* pUseNativeBufferParams = (UseAndroidNativeBufferParams3*)pCompParam;

    if (pUseNativeBufferParams->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        MTK_OMX_LOGD(this, "@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: invalid port index");
        return OMX_ErrorBadParameter;
    }

    if (OMX_TRUE != mIsUsingNativeBuffers)
    {
        MTK_OMX_LOGE(this, "@@ OMX_GoogleAndroidIndexUseAndroidNativeBuffer: we are not using native buffers");
        return OMX_ErrorBadParameter;
    }

    sp<android_native_buffer_t> nBuf = pUseNativeBufferParams->nativeBuffer;
    buffer_handle_t _handle = nBuf->handle;
    native_handle_t *pGrallocHandle = (native_handle_t *) _handle;
    int ion_buf_fd = -1;

    // TODO: check secure case
    int secure_buffer_handle;

    if (OMX_TRUE == mIsSecureInst)
    {
        err = UseAndroidNativeBuffer_CheckSecureBuffer(hComp, pCompParam);
        return err;
    }
    else
    {
        err = UseAndroidNativeBuffer_CheckNormalBuffer(hComp, pCompParam);
        return err;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::CheckICSLaterSetParameters(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err;

    //MTK_OMX_LOGD(this, "CheckICSLaterSetParameters");

#if (ANDROID_VER >= ANDROID_ICS)
    switch (nParamIndex)
    {
        case OMX_GoogleAndroidIndexEnableAndroidNativeBuffers:
        {
            MTK_OMX_LOGD(this, "HandleEnableAndroidNativeBuffers");
            err = HandleEnableAndroidNativeBuffers(pCompParam);
            return err;
        }
        break;

        case OMX_GoogleAndroidIndexUseAndroidNativeBuffer:
        {
            MTK_OMX_LOGD(this, "HandleUseAndroidNativeBuffer");
            err = HandleUseAndroidNativeBuffer(hComp, pCompParam);
            return err;
        }
        break;

        default:
        {
            //MTK_OMX_LOGD(this, "not ICS later SetParam");
            return OMX_ErrorNotImplemented;
        }
    }
#endif

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::CheckKKLaterSetParameters(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
#if (ANDROID_VER >= ANDROID_KK)
    switch (nParamIndex)
    {
        case OMX_GoogleAndroidIndexStoreMetaDataInBuffers:
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                // return unsupport intentionally
                if (mAdaptivePlayback)
                {

                }
                else
                {
                    return  OMX_ErrorUnsupportedIndex;
                }

                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                MTK_OMX_LOGD(this, "@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);
            }
            else
            {
                return OMX_ErrorBadPortIndex;
            }
        }
        break;

#if 1// Temporary fix - disable the support of adaptive playback to workaround certification test case
        case OMX_GoogleAndroidIndexPrepareForAdaptivePlayback:
        {
            PrepareForAdaptivePlaybackParams* pAdaptivePlaybackParams = (PrepareForAdaptivePlaybackParams*)pCompParam;
            if (pAdaptivePlaybackParams->nPortIndex == mOutputPortFormat.nPortIndex) {
                mEnableAdaptivePlayback = pAdaptivePlaybackParams->bEnable;
                if (mEnableAdaptivePlayback)
                {
                    //Don't trust pAdaptivePlaybackParams's max width and height.
                    //When codec don't support auto enlarge, mMaxWidth and mMaxHeight is set in ComponentInit()
                    if (supportAutoEnlarge())
                    {
                        if((pAdaptivePlaybackParams->nMaxFrameWidth > mMaxWidth) || (pAdaptivePlaybackParams->nMaxFrameHeight > mMaxHeight))
                        {
                            MTK_OMX_LOGD(this,"@@ keep original size, pAdaptivePlaybackParamsmMaxWidth(%d), mMaxHeight(%d), component: mMaxWidth(%d), mMaxHeight(%d)",
                                    pAdaptivePlaybackParams->nMaxFrameWidth, pAdaptivePlaybackParams->nMaxFrameHeight, mMaxWidth, mMaxHeight);
                        }
                        else
                        {
                            mMaxWidth = VDEC_ROUND_16(pAdaptivePlaybackParams->nMaxFrameWidth);
                            mMaxHeight = VDEC_ROUND_16(pAdaptivePlaybackParams->nMaxFrameHeight);
                        }
                    }
                    mLegacyMode = needLegacyMode();
                }
                MTK_OMX_LOGU ("@@ mEnableAdaptivePlayback(%d), mMaxWidth(%d), mMaxHeight(%d), Legacy(%d)",
                        mEnableAdaptivePlayback, mMaxWidth, mMaxHeight, mLegacyMode);
            }
            else {
                return OMX_ErrorBadPortIndex;
            }
            break;
        }

#endif
        default:
            return OMX_ErrorNotImplemented;
    }
#endif

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVdec::CheckMLaterSetParameters(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
#if (ANDROID_VER >= ANDROID_M)
    switch (nParamIndex)
    {
        case OMX_GoogleAndroidIndexstoreANWBufferInMetadata:
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                char value2[PROPERTY_VALUE_MAX];

                property_get("vendor.mtk.omxvdec.USANWInMetadata", value2, "0");
                OMX_BOOL  mDisableANWInMetadata = (OMX_BOOL) atoi(value2);
                if (1 == mDisableANWInMetadata)
                {
                    MTK_OMX_LOGD(this, "@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata return un support by setting property");
                    return OMX_ErrorUnsupportedIndex;
                }
                // return unsupport intentionally
                if (mAdaptivePlayback)
                {

                }
                else
                {
                    return OMX_ErrorUnsupportedIndex;
                }

                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                MTK_OMX_LOGD(this, "@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata");
                MTK_OMX_LOGD(this, "@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);
            }
            else
            {
                return OMX_ErrorBadPortIndex;
            }
        }
        break;

//ANDROID_N
        case OMX_GoogleAndroidIndexEnableAndroidNativeHandle:
        {
            EnableAndroidNativeBuffersParams *pEnableNativeBuffers = (EnableAndroidNativeBuffersParams *) pCompParam;
            //input secure only
            if (pEnableNativeBuffers != NULL && pEnableNativeBuffers->nPortIndex != mInputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGD(this, "@@ OMX_GoogleAndroidIndexEnableAndroidNativeHandle: invalid port index");
                return OMX_ErrorBadParameter;
            }

            if (NULL != pEnableNativeBuffers)
            {
                MTK_OMX_LOGUD("OMX_GoogleAndroidIndexEnableAndroidNativeHandle enable(%d)", pEnableNativeBuffers->enable);
                //maybe

                mIsSecUsingNativeHandle = pEnableNativeBuffers->enable;

                if(!mIsSecUsingNativeHandle)
                {
                    MTK_OMX_LOGUD("OMX_GoogleAndroidIndexEnableAndroidNativeHandle not support, enable(%d)", pEnableNativeBuffers->enable);
                    return OMX_ErrorUnsupportedSetting;
                }
            }
            else
            {
                MTK_OMX_LOGE(this, "@@ OMX_GoogleAndroidIndexEnableAndroidNativeHandle: pEnableNativeBuffers is NULL");
                return OMX_ErrorBadParameter;
            }

            break;
        }
//endof ANDROID_N

        default:
            return OMX_ErrorNotImplemented;
    }
#endif

    return OMX_ErrorNone;
}

