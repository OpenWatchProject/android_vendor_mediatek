/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_EXIF_
#define _CAMERA_CUSTOM_EXIF_
//
#include "camera_custom_types.h"

//
namespace NSCamCustom
{
/*******************************************************************************
* Custom EXIF: Imgsensor-related.
*******************************************************************************/
typedef struct SensorExifInfo_S
{
    MUINT32 uFLengthNum;
    MUINT32 uFLengthDenom;

} SensorExifInfo_T;

SensorExifInfo_T const&
getParamSensorExif()
{
    static SensorExifInfo_T inst = {
        .uFLengthNum     = 35, // Numerator of Focal Length. Default is 35.
        .uFLengthDenom   = 10, // Denominator of Focal Length, it should not be 0.  Default is 10.
    };
    return inst;
}


/*******************************************************************************
* Custom EXIF
******************************************************************************/
//#define EN_CUSTOM_EXIF_INFO
#define SET_EXIF_TAG_STRING(tag,str) \
    if (strlen((const char*)str) <= 32) { \
        strcpy((char *)pexifApp1Info->tag, (const char*)str); }
        
typedef struct customExifInfo_s {
    unsigned char strMake[32];
    unsigned char strModel[32];
    unsigned char strSoftware[32];
} customExifInfo_t;


#ifdef EN_CUSTOM_EXIF_INFO
#define CUSTOM_EXIF_STRING_MAKE  "custom make"
#define CUSTOM_EXIF_STRING_MODEL "custom model"
#define CUSTOM_EXIF_STRING_SOFTWARE "custom software"
MINT32 custom_SetExif(void **ppCustomExifTag)
{
static customExifInfo_t exifTag = {CUSTOM_EXIF_STRING_MAKE,CUSTOM_EXIF_STRING_MODEL,CUSTOM_EXIF_STRING_SOFTWARE};
    if (0 != ppCustomExifTag) {
        *ppCustomExifTag = (void*)&exifTag;
    }
    return 0;
}
#else
MINT32 custom_SetExif(void **/*ppCustomExifTag*/)
{
    return -1;
}
#endif

/*******************************************************************************
* Custom EXIF: Exposure Program
******************************************************************************/
typedef struct customExif_s
{
    MBOOL   bEnCustom;
    MUINT32 u4ExpProgram;
    
} customExif_t;

customExif_t const&
getCustomExif()
{
    static customExif_t inst = {
        .bEnCustom       =   false,  // default value: false.
        .u4ExpProgram    =   0,      // default value: 0.    '0' means not defined, '1' manual control, '2' program normal
    };
    return inst;
}


};  //NSCamCustom
#endif  //  _CAMERA_CUSTOM_EXIF_

