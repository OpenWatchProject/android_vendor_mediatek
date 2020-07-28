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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "mHalImg"

#include "MediaHal.h"
#include "MediaTypes.h"                 // For MHAL_ERROR_ENUM
#include "mhal_jpeg.h"                  // For MHAL JPEG
//#include <mdp_service.h>
#include <cutils/properties.h>


MINT32
mHalJpeg(
    MUINT32 a_u4CtrlCode,
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize,
    MVOID *a_pOutBuffer,
    MUINT32 a_u4OutBufSize,
    MUINT32 *pBytesReturned
)
{
    // unused params
    (void)a_u4InBufSize;
    (void)a_u4OutBufSize;
    (void)pBytesReturned;

    MINT32 err = JPEG_ERROR_NONE;
    MHAL_JPEG_DEC_SRC_IN *inParam;
    MHAL_JPEG_DEC_START_IN *decStartInParam;
    MHAL_JPEG_DEC_START_IN stopParam = {0};
    MHAL_JPEG_ENC_START_IN *encStartInParam;

    switch (a_u4CtrlCode)
    {
        case MHAL_IOCTL_JPEG_DEC_START:
        case MHAL_IOCTL_WEBP_DEC_START:
        case MHAL_IOCTL_MJPEG_DEC_START:
        {
            decStartInParam = (MHAL_JPEG_DEC_START_IN *)a_pInBuffer;
            decStartInParam->fCancelDecoderFlag = 0;
            err = mi_mHalJpgDecStart(decStartInParam);
            break;
        }

        case MHAL_IOCTL_JPEG_DEC_GET_INFO:
        case MHAL_IOCTL_WEBP_DEC_GET_INFO:
        case MHAL_IOCTL_MJPEG_DEC_GET_INFO:
        {
            err = mi_mHalJpgDecGetInfo((MHAL_JPEG_DEC_INFO_OUT*)a_pOutBuffer);
            break;
        }

        case MHAL_IOCTL_JPEG_DEC_PARSER:
        case MHAL_IOCTL_MJPEG_DEC_PARSER:
        {
            inParam = (MHAL_JPEG_DEC_SRC_IN *)a_pInBuffer;
            err = mi_mHalJpgDecParser(inParam, inParam->srcBuffer, 
                                      inParam->srcLength, inParam->srcFD, MHAL_IMAGE_TYPE_JPEG);
            break;
        }

        case MHAL_IOCTL_JPEG_DEC_CANCEL:
        case MHAL_IOCTL_WEBP_DEC_CANCEL:
        case MHAL_IOCTL_MJPEG_DEC_CANCEL:
        {
            stopParam.jpgDecHandle = (void*) a_pInBuffer;
            stopParam.fCancelDecoderFlag = 1;
            err = mi_mHalJpgDecStart(&stopParam);
            break;
        }

        case MHAL_IOCTL_WEBP_DEC_PARSER:
        {
            inParam = (MHAL_JPEG_DEC_SRC_IN *)a_pInBuffer;
            err = mHalJpgDecParser(inParam->srcBuffer, inParam->srcLength, inParam->srcFD, MHAL_IMAGE_TYPE_WEBP);
            break;
        }

        case MHAL_IOCTL_JPEG_ENC_START:
        {
            err = mHalJpgEncStart((MHAL_JPEG_ENC_START_IN *)a_pInBuffer);
            break;
        }

        default:
            err = JPEG_ERROR_INVALID_CTRL_CODE;
            break;
    }

    return err;
}


signed int mHalMdpIpc_BitBlt( mHalBltParam_t* bltParam )
{
    int ret = MHAL_NO_ERROR;

    if( bltParam == NULL )
    {
        //MDP_ERROR("bltParam is NULL\n");
        return -1;
    }
#if 0
    if( !mHalScaler_BitBlt(bltParam))
      ret = MHAL_UNKNOWN_ERROR ;
#endif
    return ret;
}
