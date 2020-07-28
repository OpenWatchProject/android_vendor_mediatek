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

/**
* @file lmv_ext.h
*
* LMV extension Header File
*/


#ifndef _LMV_EXT_H_
#define _LMV_EXT_H_


#define LMV_MIN_RRZ                       (432)
#define LMV_MAX_GMV_32                    (32)
#define LMV_MAX_GMV_64                    (64)
#define LMV_MAX_GMV_DEFAULT               LMV_MAX_GMV_32
#define LMV_GMV_VALUE_TO_PIXEL_UNIT       (256) // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.



/**
 *  Data type in MTK_EIS_REGION is all MINT32
 */
typedef enum
{
    LMV_REGION_INDEX_XINT = 0,
    LMV_REGION_INDEX_XFLOAT,
    LMV_REGION_INDEX_YINT,
    LMV_REGION_INDEX_YFLOAT,
    /**
     *  Resolution here is describing that the ROI of EIS, a central cropping
     *  should be applied follows this resolution.
     */
    LMV_REGION_INDEX_WIDTH, // width that valid by LMV
    LMV_REGION_INDEX_HEIGHT, // height that valid by LMV
    LMV_REGION_INDEX_MV2CENTERX,
    LMV_REGION_INDEX_MV2CENTERY,
    LMV_REGION_INDEX_ISFROMRZ,
    LMV_REGION_INDEX_GMVX,
    LMV_REGION_INDEX_GMVY, // index 10
    LMV_REGION_INDEX_CONFX,
    LMV_REGION_INDEX_CONFY,
    LMV_REGION_INDEX_EXPTIME,
    LMV_REGION_INDEX_HWTS, // highword timestamp (bit[32:63])
    LMV_REGION_INDEX_LWTS, // lowword timestamp  (bit[0:31])
    LMV_REGION_INDEX_MAX_GMV, //max search range
    LMV_REGION_INDEX_ISFRONTBIN,
    /* for indicating to size only */
    LMV_REGION_INDEX_SIZE,
} LMV_REGION_INDEX;

struct LMVData
{

    LMVData()
    {
    }

    LMVData(MINT32 _cmv_x_int,
            MINT32 _cmv_x_float,
            MINT32 _cmv_y_int,
            MINT32 _cmv_y_float,
            MINT32 _width,
            MINT32 _height,
            MINT32 _cmv_x_center,
            MINT32 _cmv_y_center,
            MINT32 _isFromRRZ,
            MINT32 _gmv_x,
            MINT32 _gmv_y,
            MINT32 _conf_x,
            MINT32 _conf_y,
            MINT32 _expTime,
            MINT32 _hwTs,
            MINT32 _lwTs,
            MINT32 _maxGMV,
            MINT32 _isFrontBin)
    : cmv_x_int(_cmv_x_int)
    , cmv_x_float(_cmv_x_float)
    , cmv_y_int(_cmv_y_int)
    , cmv_y_float(_cmv_y_float)
    , width(_width)
    , height(_height)
    , cmv_x_center(_cmv_x_center)
    , cmv_y_center(_cmv_y_center)
    , isFromRRZ(_isFromRRZ)
    , gmv_x(_gmv_x)
    , gmv_y(_gmv_y)
    , conf_x(_conf_x)
    , conf_y(_conf_y)
    , expTime(_expTime)
    , hwTs(_hwTs)
    , lwTs(_lwTs)
    , maxGMV(_maxGMV)
    , isFrontBin(_isFrontBin)
    {}

    MINT32 cmv_x_int    = 0;
    MINT32 cmv_x_float  = 0;
    MINT32 cmv_y_int    = 0;
    MINT32 cmv_y_float  = 0;
    MINT32 width        = 0;
    MINT32 height       = 0;
    MINT32 cmv_x_center = 0;
    MINT32 cmv_y_center = 0;
    MINT32 isFromRRZ    = 0;
    MINT32 gmv_x        = 0;
    MINT32 gmv_y        = 0;
    MINT32 conf_x       = 0;
    MINT32 conf_y       = 0;
    MINT32 expTime      = 0;
    MINT32 hwTs         = 0;
    MINT32 lwTs         = 0;
    MINT32 maxGMV       = 0;
    MINT32 isFrontBin   = 0;
};

#endif // _LMV_EXT_H_
