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

#ifndef _MY_HDR_H_
#define _MY_HDR_H_

/*******************************************************************************
*
*******************************************************************************/
//should be 0
#define HDR_DEBUG_SAVE_SOURCE_IMAGE				(0)
#define HDR_DEBUG_SAVE_SMALL_IMAGE				(0)
#define HDR_DEBUG_SAVE_NORMALIZED_SMALL_IMAGE	(0)
#define HDR_DEBUG_SAVE_SE_IMAGE		    		(0)
#define HDR_DEBUG_SAVE_WEIGHTING_MAP			(0)
#define HDR_DEBUG_SAVE_DOWNSCALED_WEIGHTING_MAP	(0)
#define HDR_DEBUG_SAVE_BLURRED_WEIGHTING_MAP	(0)
#define HDR_DEBUG_SAVE_HDR_RESULT				(0)
#define HDR_DEBUG_SAVE_HDR_NR				    (0)

#define HDR_DEBUG_SAVE_POSTVIEW         		(0)
#define HDR_DEBUG_SAVE_RESIZE_HDR_RESULT		(0)
#define HDR_DEBUG_SAVE_HDR_JPEG         		(0)

#define HDR_DEBUG_OFFLINE_SOURCE_IMAGE         	(0)
#define HDR_DEBUG_SKIP_HANDLER               	(0)
#define HDR_DEBUG_SKIP_3A                      	(0)
#define HDR_DEBUG_SKIP_MODIFY_POLICY            (0)
#define HDR_DEBUG_SKIP_EXIF                     (0)
#define HDR_DEBUG_SKIP_REUSE_BUFFER             (1)     // since buffer allocation is very fast, we didn't need reuse buffer

//should be 1
#define HDR_SPEEDUP_JPEG                        (1)


#define HDR_AE_12BIT_FLARE                      (1)     // 1:12bit, 0:8bit

#define HDR_BURST_JPEG_SIZE                     (0)     // 0:Full size 1:Jpeg size

/*******************************************************************************
*
*******************************************************************************/
#include <utils/Errors.h>

#include <IShot.h>

#include <mtkcam/camshot/_callbacks.h> //workaround for ISImager.h

#include <mtkcam/iopipe/SImager/ISImager.h>
#include <fcntl.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include <mtkcam/hal/IHalSensor.h>
using namespace CPTool;
#include "camera_custom_hdr.h"	// For HDR Customer Parameters in Customer Folder.

#include "Hdr.h"
#include <mtkcam/camshot/_params.h>
#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/v1/common.h>

#define PTHREAD_MUTEX_INITIALIZER_LOCK  {{1}}

/*******************************************************************************
*
*******************************************************************************/

#define CHECK_OBJECT(x)         do { if (x == NULL) { CAM_LOGE("Null %s Object\n", #x); return MFALSE;}} while(0)
#define CHECK_RET(fmt, arg...)  do { if( !ret ){ CAM_LOGE(fmt, ##arg); ret = MFALSE; goto lbExit; }} while(0)


/*******************************************************************************
*
*******************************************************************************/
#endif  //  _MY_HDR_H_

