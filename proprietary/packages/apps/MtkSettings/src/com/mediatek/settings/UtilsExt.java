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
 */

package com.mediatek.settings;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;
import android.os.SystemProperties;
import com.mediatek.settings.ext.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class UtilsExt {

    private static final String TAG = "UtilsExt";
    private static IDataUsageSummaryExt sDataUsageSummaryExt;
    private static ISimManagementExt sSimManagementExt;
    private static ISimRoamingExt sSimRoamingExt;
    private static IWfcSettingsExt sWfcSettingsExt;
    private static IDeviceInfoSettingsExt sDeviceInfoSettingsExt;

    public static IDataUsageSummaryExt getDataUsageSummaryExt(Context context) {
        if (sDataUsageSummaryExt == null) {
            synchronized (IDataUsageSummaryExt.class) {
                if (sDataUsageSummaryExt == null) {
                    sDataUsageSummaryExt = OpSettingsCustomizationUtils.getOpFactory(context)
                            .makeDataUsageSummaryExt();
                    log("[getDataUsageSummaryExt]create ext instance: " + sDataUsageSummaryExt);
                }
            }
        }
        return sDataUsageSummaryExt;
    }

    public static ISimManagementExt getSimManagementExt(Context context) {
        if (sSimManagementExt == null) {
            synchronized (ISimManagementExt.class) {
                if (sSimManagementExt == null) {
                    sSimManagementExt = OpSettingsCustomizationUtils.getOpFactory(context)
                            .makeSimManagementExt();
                    log("[getSimManagementExt] create ext instance: " + sSimManagementExt);
                }
            }
        }
        return sSimManagementExt;
    }

    public static ISimRoamingExt getSimRoamingExt(Context context) {
        if (sSimRoamingExt == null) {
            synchronized (ISimRoamingExt.class) {
                if (sSimRoamingExt == null) {
                    sSimRoamingExt = OpSettingsCustomizationUtils.getOpFactory(context)
                            .makeSimRoamingExt();
                    log("[getSimRoamingExt] create ext instance: " + sSimRoamingExt);
                }
            }
        }
        return sSimRoamingExt;
    }

    public static IWfcSettingsExt getWfcSettingsExt(Context context) {
        if (sWfcSettingsExt == null) {
            synchronized (IWfcSettingsExt.class) {
                if (sWfcSettingsExt == null) {
                    sWfcSettingsExt = OpSettingsCustomizationUtils.getOpFactory(context)
                            .makeWfcSettingsExt();
                    log("[getWfcSettingsExt] create ext instance: " + sWfcSettingsExt);
                }
            }
        }
        return sWfcSettingsExt;
    }

    // M: create settigns plugin object
    public static ISettingsMiscExt getMiscPlugin(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeSettingsMiscExt(context);
    }

  public static IDisplaySettingsExt getDisplaySettingsExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeDisplaySettingsExt(context);
    }

    public static IApnSettingsExt getApnSettingsExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeApnSettingsExt(context);
    }

    // M: Add for MTK Wifi feature.
    public static IWifiExt getWifiExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeWifiExt(context);
    }

    // M: Add for MTK Wifi settings feature.
    public static IWifiSettingsExt getWifiSettingsExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeWifiSettingsExt();
    }

    public static IDevExt getDevExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeDevExt(context);
    }

    public static IDeviceInfoSettingsExt getDeviceInfoSettingsExt(Context context) {
        if (sDeviceInfoSettingsExt == null) {
            synchronized (IDeviceInfoSettingsExt.class) {
                if (sDeviceInfoSettingsExt == null) {
                    sDeviceInfoSettingsExt = OpSettingsCustomizationUtils.getOpFactory(context)
                            .makeDeviceInfoSettingsExt();
                    log("[sDeviceInfoSettingsExt]create ext instance: " + sDeviceInfoSettingsExt);
                }
            }
        }
        return sDeviceInfoSettingsExt;
    }

    public static IDeviceInfoSettingsExt useDeviceInfoSettingsExt() {
        return sDeviceInfoSettingsExt;
    }

    public static IMobileNetworkSettingsExt getMobileNetworkSettingsExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeMobileNetworkSettingsExt(context);
    }

    public static INetworkSettingExt getNetworkSettingExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeNetworkSettingExt(context);
    }

    public static IStatusExt getStatusExt(Context context) {
        return OpSettingsCustomizationUtils.getOpFactory(context).makeStatusExt();
    }
    /**
     * do not show SIM Activity Dialog for auto sanity.
     * 1.FeatureOption.MTK_AUTOSANITY is true
     * 2.FeatureOption.MTK_BUILD_TYPE is ENG
     * @return true disable SIM Dialog
     */
    public static boolean shouldDisableForAutoSanity() {
        boolean autoSanity = SystemProperties.get("ro.mtk.autosanity").equals("1");
        String buildType = SystemProperties.get("ro.build.type", "");
        Log.d(TAG, "autoSanity: " + autoSanity + " buildType: " + buildType);
        if (autoSanity && (!TextUtils.isEmpty(buildType)) && buildType.endsWith("eng")) {
            Log.d(TAG, "ShouldDisableForAutoSanity()...");
            return true;
        }
        return false;
    }
    // / @}

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    public static void resetSettingExt() {
        sWfcSettingsExt = null;
        sSimRoamingExt = null;
        sSimManagementExt = null;
        sDataUsageSummaryExt = null;
    }
}
