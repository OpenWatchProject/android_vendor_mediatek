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
package com.mediatek.op18.settingsProvider;

import android.content.Context;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import com.android.ims.ImsConfig;

import com.mediatek.providers.settings.ext.DefaultDatabaseHelperExt;

/**
 * Plugin to set customer specific default values for various settings.
 */
public class Op18DatabaseHelperExt extends DefaultDatabaseHelperExt {

    private static final String TAG = "Op18DatabaseHelperExt";

    /**
     * @param context Context
     * constructor
     */
    public Op18DatabaseHelperExt(Context context) {
         super(context);
    }

    /**
     * @param context Context
     * @param name String
     * @param defaultValue String
     * @return the value
     * Used in settings provider for WFC feature
     */
    @Override
    public String getResInteger(Context context, String name, String defaultValue) {
        String res = defaultValue;
        if (Settings.Global.WFC_IMS_MODE.equals(name)) {
            res = Integer.toString(ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
        } else if (Settings.Global.WFC_IMS_ENABLED.equals(name)) {
            res = Integer.toString(ImsConfig.FeatureValueConstants.ON);
        } else if (Settings.System.WHEN_TO_MAKE_WIFI_CALLS.equals(name)) {
            int resInt = 2;
            //TelephonyManager.WifiCallingChoices.NEVER_USE = 2
            res = Integer.toString(resInt);
        }
        Log.d(TAG, "get name = " + name + " int value = " + res);
        return res;
    }
}
