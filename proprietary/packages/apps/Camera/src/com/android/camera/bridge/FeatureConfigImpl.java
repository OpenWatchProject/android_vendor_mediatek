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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.android.camera.bridge;

import android.os.SystemProperties;

import com.android.camera.FeatureSwitcher;

import com.mediatek.camera.platform.IFeatureConfig;

public class FeatureConfigImpl implements IFeatureConfig {

    @Override
    public boolean isVfbEnable() {
        return FeatureSwitcher.isVfbEnable();
    }

    @Override
    public boolean isCfbEnable() {
        return FeatureSwitcher.isCfbEnable();
    }

    @Override
    public boolean mfbSupportType() {
        return FeatureSwitcher.isAisSupported();
    }

    @Override
    public boolean isSlowMotionSupport() {
        return FeatureSwitcher.isSlowMotionSupport();
    }

    @Override
    public boolean isGmoRamOptSupport() {
        return FeatureSwitcher.isGmoRAM();
    }

    @Override
    public boolean isGmoRomOptSupport() {
        return FeatureSwitcher.isGmoROM();
    }

    @Override
    public boolean isLowRamOptSupport() {
        return FeatureSwitcher.isLowRAM();
    }

    @Override
    public boolean isMtkFatOnNandSupport() {
        return FeatureSwitcher.isMtkFatOnNand();
    }

    @Override
    public boolean isTablet() {
        return FeatureSwitcher.isTablet();
    }

    @Override
    public boolean isNativePipSupport() {
        return FeatureSwitcher.isNativePIPEnabled();
    }

    @Override
    public boolean isLomoEffectSupport() {
        return FeatureSwitcher.isLomoEffectEnabled();
    }

    @Override
    public boolean isHdRecordingEnabled() {
        return FeatureSwitcher.isHdRecordingEnabled();
    }

    @Override
    public boolean isDualCameraEnable() {
        return FeatureSwitcher.isDualCameraEnable();
    }
    @Override
    public String whichDeanliChip() {
        return FeatureSwitcher.whichDeanliChip();
    }

    @Override
    public boolean isZSDHDRSupported() {
        return FeatureSwitcher.isZSDHDRSupported();
    }
}
