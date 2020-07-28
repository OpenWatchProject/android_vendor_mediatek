/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_VENDOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_VENDOR_H_

#include <utils/RefBase.h>

#include <mtkcam/pipeline/extension/IVendorManager.h>

using namespace android;

#if MTKCAM_HAVE_HDR_SUPPORT
#define MAKE_HDRVendor(name, id, mode) NSCam::plugin::HDRVendor::createInstance(name, id, mode)
#else
#define MAKE_HDRVendor(name, id, mode) nullptr
#endif

#if MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT
#define MAKE_BMDNVendor(name, id, mode) NSCam::plugin::BMDNVendor::createInstance(name, id, mode)
#else
#define MAKE_BMDNVendor(name, id, mode) nullptr
#endif

#if MTKCAM_HAVE_MTKSTEREO_SUPPORT
#define MAKE_DCMFVendor(name, id, mode) NSCam::plugin::DCMFVendor::createInstance(name, id, mode)
#else
#define MAKE_DCMFVendor(name, id, mode) nullptr
#endif

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace plugin {

class NRvendor
    : public virtual IVendor
{
public:
    static sp<NRvendor>        createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

class CopyVendor
    : public virtual IVendor
{
public:
    static sp<CopyVendor>       createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

class HDRVendor : public virtual IVendor
{
public:
    static
    android::sp<HDRVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

class mixYuvVendor : public virtual IVendor
{
public:
    static
    android::sp<mixYuvVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

class BMDNVendor : public virtual IVendor
{
public:
    static
    android::sp<BMDNVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

class FusionVendor : public virtual IVendor
{
public:
    static
    android::sp<FusionVendor>   createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

#if (MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT == 1)
class VsdofVendor : public virtual IVendor
{
public:
    static
    android::sp<VsdofVendor>   createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};
#endif

class DCMFVendor : public virtual IVendor
{
public:
    static
    android::sp<DCMFVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );
};

} // namespace plugin
} // namespace NSCam

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_VENDOR_H_
