/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.settings.op01;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.custom.CustomProperties;
import com.mediatek.settings.op01.R;
import com.mediatek.settings.ext.DefaultDeviceInfoSettingsExt;

/**
 * Device info plugin.
 */
public class Op01DeviceInfoSettingsExt extends DefaultDeviceInfoSettingsExt {
    private static final String TAG = "Op01DeviceInfoSettingsExt";
    private static final String KEY_BUILD_NUMBER = "build_number";
    private static final String KEY_DEVICE_MODEL = "device_model";

    private Context mContext;

    /**
     * Init context.
     * @param context The Context
     */
    public Op01DeviceInfoSettingsExt(Context context) {
        mContext = context;
    }

    @Override
    public String customeBuildNumber(CharSequence defaultBuildNumber) {
        String newBuildNumber = CustomProperties.getString("Setting", "SWVerno", defaultBuildNumber.toString());
        Log.i(TAG, "customeBuildNumber defaultBuildNumber = " + defaultBuildNumber
                + " newBuildNumber = " + newBuildNumber);
        return newBuildNumber;
    }

    public String customeModelInfo(String defaultModeInfo) {
        String value = CustomProperties.getString("Setting", "Model", defaultModeInfo);
        String modeString = SystemProperties.get("ro.product.model", "unknown");
        Log.i(TAG, "value = " + value
                +"modeString = " + modeString);
        if (null != value && value.equals(modeString)) {
            return defaultModeInfo;
        }
        Log.d("@M_" + TAG, "customeModelInfo"
                + " defaultModeInfo = " + defaultModeInfo
                + " value = " + value);
        return value;
    }

}
