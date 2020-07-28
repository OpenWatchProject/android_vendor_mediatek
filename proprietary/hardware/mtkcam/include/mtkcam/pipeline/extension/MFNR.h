/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_MFNR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_MFNR_H_

// AOSP
#include <utils/RefBase.h>
#include <utils/Mutex.h>

// MTKCAM
#include <mtkcam/pipeline/extension/IVendorManager.h>

#if MTKCAM_HAVE_MFB_SUPPORT
#define MAKE_MFNRVendor(name, id, mode) NSCam::plugin::MFNRVendor::createInstance(name, id, mode)
#else
#define MAKE_MFNRVendor(name, id, mode) nullptr
#endif

using namespace android;


namespace NSCam {
namespace plugin {

class MFNRVendor : public virtual IVendor
{
//
// Customized IVENDOR_QUERY_INFO
//
public:
    enum INFO_MFNR_CMD {
        /**
         * Send EM Config to MFNR vendor.
         *
         * arg1 : null
         * arg2 : null
         * arg3 : address of a EMConfig
         */
        INFO_MFNR_ENG_PARAM         = INFO_USER_DEFINE_START,

        /**
         * Send default ConfigParams to MFNR vendor.
         *
         * arg1 : null
         * arg2 : mfbMode
         * arg3 : address of a ConfigParams
         */
        INFO_MFNR_CONFIG_PARAM,
    };
    static Mutex sCtrlerAccessLock;

public:
    struct ConfigParams
    {
        MBOOL isZSDMode;
        MBOOL isZHDRMode;
        MBOOL isAutoHDR;
        MBOOL isFlashOn;
        uint32_t exposureTime;
        uint32_t realIso;
        int32_t mfbMode;
        int32_t customHint;
        MUINT32 sensorMode;
        MBOOL   usingBackgroundService; // is using Background service, MFNR won't be canceled if
                                        // received cancel picture command
        ConfigParams()
            : isZSDMode(MTRUE)
            , isZHDRMode(MFALSE)
            , isAutoHDR(MFALSE)
            , isFlashOn(MFALSE)
            , exposureTime(0)
            , realIso(0)
            , mfbMode(0)
            , customHint(0)
            , sensorMode(SENSOR_SCENARIO_ID_UNNAMED_START)
            , usingBackgroundService(MFALSE)
        {}
    };


    struct FeatureInfo
    {
        MBOOL   doMfb;
        MINT32  frameCapture;
        MINT32  frameBlend;
        FeatureInfo()
            : doMfb(MFALSE)
            , frameCapture(0)
            , frameBlend(0)
        {}
    };


    // EM usage
    struct EMConfig
    {
        int captureNum;
        int blendNum;
        int realIso;
        int shutterTimeUs; //unit: micro-second(us)
        MBOOL isZHDRMode;
        MBOOL isAutoHDR;
        EMConfig()
            : captureNum(0)
            , blendNum(0)
            , realIso(0)
            , shutterTimeUs(0)
            , isZHDRMode(MFALSE)
            , isAutoHDR(MFALSE)
        {}
    };


//
// Static Methods
//
public:
    static
    android::sp<MFNRVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );

    // Query the feature info.
    //  @param sensorId         Current sensor ID
    //  @param mfbMode          MFB mode.
    //                          0: Not specified, as normal capture.
    //                          1: Specified to do MFNR
    //                          2: Specified to do AIS
    //                          3: Specified to enter EM (Engineering Mode)
    //                          4: Specified to do MFSR (not ready yet)
    //  @param rParams          Is zsd mode, zhdr mode.
    //  @return                 FeatureInfo
    static
    FeatureInfo                 queryFeatureInfo(
                                    int sensorId,
                                    int mfbMode,
                                    MFNRVendor::ConfigParams const& rParams
                                );


public:
    virtual MERROR              config(MFNRVendor::ConfigParams const& rParams) = 0;
    virtual MERROR              query(MFNRVendor::FeatureInfo& rParams) = 0;


};// class MFNRVendor
} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_MFNR_H_
