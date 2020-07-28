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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

//#define LOG_NDEBUG 0
#define LOG_TAG "WVHdcpExt"
#include <utils/Log.h>

#include "WVHdcpExt.h"

bool WvHdcpExt_IsSRMUpdateSupported() {
    // If you implement SRM update for HDCP feature, please return true
    return false;
}

uint32_t WvHdcpExt_GetCurrentSRMVersion(uint16_t* version) {
    // If you implement SRM update for HDCP feature, please return OEMCrypto_SUCCESS.
    // Based on the considerations of security, we will not return SRM version returned by this
    // function, we need you set SRM version to MTK DRM HDCP T-driver by you HDCP TA(we will
    // provide how to do this in guide document), and widevine TA will get the SRM version from
    // the T-driver, then use it to check or return to app.
    //   So please set SRM version to T-driver when this function has been called.
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

uint32_t WvHdcpExt_LoadSRM(const uint8_t* buffer, size_t buffer_length) {
    // If you implement SRM update for HDCP feature, please return below value case by case, default
    // we return OEMCrypto_ERROR_NOT_IMPLEMENTED:
    // OEMCrypto_SUCCESS - if the file was valid and was installed.
    // OEMCrypto_ERROR_INVALID_CONTEXT - if the SRM version is too low, or the file is corrupted.
    // OEMCrypto_ERROR_SIGNATURE_FAILURE - If the signature is invalid.
    // OEMCrypto_ERROR_BUFFER_TOO_LARGE - if the buffer is too large for the device.
    // OEMCrypto_ERROR_NOT_IMPLEMENTED - default
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

uint32_t WvHdcpExt_RemoveSRM() {
    // This function should not be implemented on production devices, and will only be used to
    // verify unit tests on a test device.
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}
