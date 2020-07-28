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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _POSTALGO_CONVERT_MAP_H_
#define _POSTALGO_CONVERT_MAP_H_

#include <mtk/mtk_platform_metadata_tag.h>

#define ADD_ALL_MEMBERS \
     _ADD_TAGMAP_(    "postalgo.facebeauty.smooth"  ,    MTK_POSTALGO_FACE_BEAUTY_SMOOTH  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.enlargeeye"  ,    MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.slimface"  ,    MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.bright"  ,    MTK_POSTALGO_FACE_BEAUTY_BRIGHT  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.ruddy"  ,    MTK_POSTALGO_FACE_BEAUTY_RUDDY  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.physicalid"  ,    MTK_POSTALGO_FACE_BEAUTY_PHYSICAL_ID  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.mirror"  ,    MTK_POSTALGO_FACE_BEAUTY_MIRROR  )\
     _ADD_TAGMAP_(    "postalgo.filter.type"  ,    MTK_POSTALGO_FILTER_TYPE  )\
     _ADD_TAGMAP_(    "postalgo.autorama.cmd"  ,    MTK_POSTALGO_AUTORAMA_CMD  )\
     _ADD_TAGMAP_(    "postalgo.autorama.motion.data"  ,    MTK_POSTALGO_AUTORAMA_MOTION_DATA)\
     _ADD_TAGMAP_(    "postalgo.sensor.orientation", MTK_POSTALGO_SENSOR_ORIENTATION) \
     _ADD_TAGMAP_(    "postalgo.lens.facing", MTK_POSTALGO_LENS_FACING) \
     _ADD_TAGMAP_(    "postalgo.capture.jpegorientation", MTK_POSTALGO_JPEG_ORIENTATION) \
     _ADD_TAGMAP_(    "postalgo.portrait.doflevel"  ,    MTK_POSTALGO_PORTRAIT_DOF_LEVEL  )\
     _ADD_TAGMAP_(    "postalgo.portrait.type"  ,    MTK_POSTALGO_PORTRAIT_TYPE  )\
     _ADD_TAGMAP_(    "postalgo.portrait.faceorientation"  ,    MTK_POSTALGO_PORTRAIT_FACEORIENTATION  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.smallface"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.bigeye"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.brighteye"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.bignose"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.smoothface"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.brightwhite"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.beautyall"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.faceorientation"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_FACEORIENTATION  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.effects.value"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_EFFECTS_VALUE  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.physicalid"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_PHYSICAL_ID  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.facedetectionsize"  ,    MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE  )\
     _ADD_TAGMAP_(    "postalgo.facebeauty.facedetection"  ,    MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION  )\
     _ADD_TAGMAP_(    "postalgo.vfacebeauty.facedetection"  ,    MTK_POSTALGO_VENDOR_FACE_BEAUTY_FACE_DETECTION  )\
     _ADD_TAGMAP_(    "postalgo.picturesize"  ,           MTK_POSTALGO_PICTURE_SIZE  )\

#endif
