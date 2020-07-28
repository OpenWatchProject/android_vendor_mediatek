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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/** 
 * @file 
 *   jpeg_enc_hal.h 
 *
 * @par Project:
 *   Yusu 
 *
 * @par Description::
 *   JPEG Encoder Driver definitions and function prototypes
 *
 * @par Author:
 *   Tzu-Meng Chung (mtk02529)
 *
 * @par $Revision$
 * @par $Modtime:$
 * @par $Log:$
 *
 */
 
#ifndef __JPEG_ENC_HAL_H__
#define __JPEG_ENC_HAL_H__

/*=============================================================================
 *                              Type definition
 *===========================================================================*/
  
 /**
 * @par Enumeration
 *   JPEG_ENC_STATUS_ENUM
 * @par Description
 *   This is the return status of each jpeg encoder function 
 */
typedef enum {
   JPEG_ENC_STATUS_OK = 0,                      ///< The function work successfully
   JPEG_ENC_STATUS_ERROR_CODEC_UNAVAILABLE,     ///< Error due to unavailable hardare codec
   JPEG_ENC_STATUS_ERROR_INVALID_PARAMETER,     ///< Error due to invalid parameter      
   JPEG_ENC_STATUS_ERROR_UNSUPPORTED_FORMAT,    ///< Error due to unsupported format of hardware
   JPEG_ENC_STATUS_ERROR_INVALID_WIDTH,         ///< Error due to unsupported dimension of hardware
   JPEG_ENC_STATUS_ERROR_INVALID_HEIGHT,        ///< Error due to unsupported dimension of hardware
   JPEG_ENC_STATUS_ERROR_INSUFFICIENT_MEMORY    ///< Error due to insufficient memory
   
} JPEG_ENC_STATUS_ENUM;

/**
 * @par Enumeration
 *   JPEG_ENC_RESULT_ENUM
 * @par Description
 *   This is the result after trigger jpeg encoder 
 */
typedef enum {
    JPEG_ENC_RESULT_DONE = 0,               ///< The result of encoder is successful
    JPEG_ENC_RESULT_PAUSE = 1,              ///< The result of encoder is pause
    JPEG_ENC_RESULT_HALT = 2,               ///< The result of encoder is timeout

    JPEG_ENC_RESULT_ALL = 0xFFFFFFFF

} JPEG_ENC_RESULT_ENUM;

/**
 * @par Enumeration
 *   JPEG_SAMPLING_FORMAT
 * @par Description
 *   The JPEG sampling format you want to encode
 */
typedef enum {
    JPEG_SAMPLING_FORMAT_YUY2 = 0,
    JPEG_SAMPLING_FORMAT_YVYU = 1,
    JPEG_SAMPLING_FORMAT_NV12 = 2,
    JPEG_SAMPLING_FORMAT_NV21 = 3,
    JPEG_SAMPLING_FORMAT_YUV444 = 444,
    JPEG_SAMPLING_FORMAT_YUV422 = 422,
    JPEG_SAMPLING_FORMAT_YUV411 = 411,
    JPEG_SAMPLING_FORMAT_YUV420 = 420,
    JPEG_SAMPLING_FORMAT_GRAYLEVEL = 400,

   
    JPEG_SAMPLING_FORMAT_ALL = 0xFFFFFFFF

} JPEG_SAMPLING_FORMAT;

/**
 * @par Enumeration
 *   JPEG_ENCODE_QUALITY_ENUM
 * @par Description
 *   The JPEG quality you want to encode
 */
typedef enum {
    JPEG_ENCODE_QUALITY_Q60 = 0x0,
    JPEG_ENCODE_QUALITY_Q80 = 0x1, 
    JPEG_ENCODE_QUALITY_Q90 = 0x2,   
    JPEG_ENCODE_QUALITY_Q95 = 0x3,

    JPEG_ENCODE_QUALITY_Q39 = 0x4,
    JPEG_ENCODE_QUALITY_Q68 = 0x5,  
    JPEG_ENCODE_QUALITY_Q84 = 0x6,
    JPEG_ENCODE_QUALITY_Q92 = 0x7,
   
    JPEG_ENCODE_QUALITY_Q48 = 0x8,   
    JPEG_ENCODE_QUALITY_Q74 = 0xA,   
    JPEG_ENCODE_QUALITY_Q87 = 0xB,

    JPEG_ENCODE_QUALITY_Q34 = 0xC,
    JPEG_ENCODE_QUALITY_Q64 = 0xE,       
    JPEG_ENCODE_QUALITY_Q82 = 0xF,

    JPEG_ENCODE_QUALITY_Q97 = 0x10,

    JPEG_ENCODE_QUALITY_ALL = 0xFFFFFFFF
} JPEG_ENCODE_QUALITY_ENUM;

/**
 * @par Structure
 *   JPEG_ENC_HAL_IN
 * @par Description
 *   This is a structure which store jpeg information for IDP configuration
 */
typedef struct
{
    unsigned int dstBufferAddr;           ///< the destination address of encoded data
    unsigned int dstBufferSize;             ///< the predictive destination buffer size
    
    unsigned int dstWidth;                  ///< the destination width
    unsigned int dstHeight;                 ///< the destination height

    unsigned int restartInterval;           ///< the restart interval 
    unsigned int srcBufferStride;             ///< set luma buffer stride
    unsigned int srcBufferAddr;             ///< set luma buffer address
    unsigned int srcChromaAddr;             ///< set chroma buffer address
    //unsigned int srcChromaStride;             ///< set luma buffer stride
    
    bool enableEXIF;                        ///< the flag to encode SOI header or not
    bool isPhyAddr;                         ///< the flag to record dstBufferAddr is physical or not 
    bool enableSyncReset;                   ///< the flag to enable sync reset bit


    JPEG_ENCODE_QUALITY_ENUM dstQuality;    ///< the encoded quality
    JPEG_SAMPLING_FORMAT dstFormat;         ///< the encoded format
    
}JPEG_ENC_HAL_IN;

/*=============================================================================
 *                             Function Declaration
 *===========================================================================*/
 
 /**
 * @par Function       
 *   jpegEncLockDecoder
 * @par Description    
 *   Lock hardware controller  
 * @param              
 *   drvID          [OUT]       identify for hardware
 * @par Returns        
 *   JPEG_ENC_STATUS_ENUM
 */
JPEG_ENC_STATUS_ENUM jpegEncLockEncoder(int* drvID);

 /**
 * @par Function       
 *   jpegEncUnlockDecoder
 * @par Description    
 *   Unlock hardware controller  
 * @param              
 *   drvID          [IN]       identify for hardware
 * @par Returns        
 *   JPEG_ENC_STATUS_ENUM
 */
JPEG_ENC_STATUS_ENUM jpegEncUnlockEncoder(int drvID);

 /**
 * @par Function       
 *   jpegEncConfigEncoder
 * @par Description    
 *   Configure hardware register  
 * @param              
 *   drvID          [IN]       identify for hardware
 * @param              
 *   inParams       [IN]       the configuration for jpeg encoder hardware
 * @par Returns        
 *   JPEG_ENC_STATUS_ENUM
 */
JPEG_ENC_STATUS_ENUM jpegEncConfigEncoder(int drvID, JPEG_ENC_HAL_IN inParams);

 /**
 * @par Function       
 *   jpegEncStart
 * @par Description    
 *   start encode hardware  
 * @param              
 *   drvID          [IN]       identify for hardware
 * @par Returns        
 *   JPEG_ENC_STATUS_ENUM
 */
JPEG_ENC_STATUS_ENUM jpegEncStart(int drvID);

 /**
 * @par Function       
 *   jpegEncWaitIRQ
 * @par Description    
 *   Lock hardware controller  
 * @param              
 *   drvID          [IN]       identify for hardware
 * @param              
 *   timeout_msec   [IN]       identify for hardware
 * @param              
 *   encodeSize     [OUT]      the size of encoded data  
 * @param              
 *   status         [OUT]      the result of hardware encoder
 * @par Returns        
 *   JPEG_ENC_STATUS_ENUM
 */
JPEG_ENC_STATUS_ENUM jpegEncWaitIRQ(int drvID, long timeout_msec, unsigned int *encodeSize, JPEG_ENC_RESULT_ENUM *status);

#endif
