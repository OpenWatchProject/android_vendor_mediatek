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

#pragma once

/******************************************************************************
 *
 ******************************************************************************/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Debug Exif Key ID
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DEBUG_EXIF_KEYID_AAA                0xF0F1F200
#define DEBUG_EXIF_KEYID_ISP                0xF4F5F6F7
#define DEBUG_EXIF_KEYID_CAM                0xF8F9FAFB
#define DEBUG_EXIF_KEYID_SHAD_TABLE         0xFCFDFEFF
#define DEBUG_EXIF_KEYID_EIS                0xF1F3F5F7


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Debug Exif Module ID - DEBUG_EXIF_KEYID_CAM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum
{
    DEBUG_EXIF_MID_CAM_CMN              =   0x5001,
    DEBUG_EXIF_MID_CAM_MF,
    DEBUG_EXIF_MID_CAM_N3D,
    DEBUG_EXIF_MID_CAM_SENSOR,
    DEBUG_EXIF_MID_CAM_RESERVE1,
    DEBUG_EXIF_MID_CAM_RESERVE2,
    DEBUG_EXIF_MID_CAM_RESERVE3,
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Debug Exif Module ID - DEBUG_EXIF_KEYID_EIS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum
{
    DEBUG_EXIF_MID_EIS                  =   0x4001,
};

