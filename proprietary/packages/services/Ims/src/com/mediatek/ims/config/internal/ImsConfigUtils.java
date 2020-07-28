/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.ims.config.internal;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.ims.common.SubscriptionManagerHelper;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ril.ImsCommandsInterface;

/**
 * Class containing util apis for ImsConfig.
 */
public class ImsConfigUtils {
    private static final String TAG = "ImsConfigUtils";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final String ACTION_WIFI_ONLY_MODE_CHANGED
            = "android.intent.action.ACTION_WIFI_ONLY_MODE";
    private static final String EXTRA_WIFI_ONLY_MODE_CHANGED = "state";

    public static final String PROPERTY_IMS_SUPPORT = "persist.vendor.ims_support";
    public static final String PROPERTY_VOLTE_ENALBE = "persist.vendor.mtk.volte.enable";
    public static final String PROPERTY_WFC_ENALBE = "persist.vendor.mtk.wfc.enable";
    public static final String PROPERTY_VILTE_ENALBE = "persist.vendor.mtk.vilte.enable";
    public static final String PROPERTY_VIWIFI_ENALBE = "persist.vendor.mtk.viwifi.enable";
    public static final String PROPERTY_IMS_VIDEO_ENALBE = "persist.vendor.mtk.ims.video.enable";

    public static class MdConfigType {
        public static final int TYPE_UNKNOWN = -1;
        public static final int TYPE_IMSCFG = 0;
        public static final int TYPE_IMSWOCFG = 1;
        public static final int TYPE_IMSIWLANCFG = 2;
    }

    /**
     * Sends wifi-only mode intent.
     * @param WFC preferred mode.
     * @return void.
     */
    public static void sendWifiOnlyModeIntent(Context context, int phoneId, boolean mode) {
        if (ImsCommonUtil.supportMims() == false &&
            ImsCommonUtil.getMainCapabilityPhoneId() != phoneId) {
            return;
        }
        Intent intent = new Intent(ACTION_WIFI_ONLY_MODE_CHANGED);
        intent.setPackage("com.android.phone");
        intent.putExtra(EXTRA_WIFI_ONLY_MODE_CHANGED, mode);
        if (ImsCommonUtil.supportMims() == true) {
            intent.putExtra(PhoneConstants.PHONE_KEY, phoneId);
        }
        if (TELDBG) Log.d(TAG, "sendWifiOnlyModeIntent() intent, mode:" + mode +
                              ", phoneId:" + phoneId);
        context.sendBroadcast(intent);
    }

    public static int getFeaturePropValue(String propName, int phoneId) {
        int defaultPropValue = 0;
        int featureValue, propResult;
        featureValue = SystemProperties.getInt(propName, defaultPropValue);
        if (!checkIsPhoneIdValid(phoneId)) {
            if (DEBUG) Log.d(TAG, "Multi IMS getFeaturePropValue():" + propName +
                    ", phoneId invalid return default value");
            return defaultPropValue;
        }
        if (ImsCommonUtil.supportMims()) {
            propResult = ((featureValue & (1 << phoneId)) > 0)?1:0;
        } else {
            // Backward compatibility, only use phone Id 0 to get.
            propResult = ((featureValue & (1 << 0)) > 0)?1:0;
        }
        if (DEBUG) Log.d(TAG, "Multi IMS getFeaturePropValue() " +
                "featureValue:" + featureValue +
                ", propName:" + propName +
                ", propResult:" + propResult);
        return propResult;
    }

    public static void setFeaturePropValue(String propName, String enabled, int phoneId) {
        int defaultPropValue = 0;
        int featureValue, enabledValue, sumFeatureValue;
        featureValue = SystemProperties.getInt(propName, defaultPropValue);
        if (!checkIsPhoneIdValid(phoneId)) {
            if (DEBUG) Log.d(TAG, "Multi IMS setFeaturePropValue():" + propName +
                    ", phoneId invalid don't set and return");
            return;
        }
        enabledValue = Integer.parseInt(enabled);
        if (ImsCommonUtil.supportMims()) {
            sumFeatureValue = setBitForPhone(featureValue, enabledValue, phoneId);
        } else {
            // Backward compatibility, only use phone Id 0 to save.
            sumFeatureValue = setBitForPhone(featureValue, enabledValue, 0);
        }
        SystemProperties.set(propName, Integer.toString(sumFeatureValue));
        if (DEBUG) Log.d(TAG, "Multi IMS setFeaturePropValue() " +
                "featureValue:" + featureValue +
                ", propName:" + propName +
                ", sumFeatureValue:" + sumFeatureValue +
                ", enabledValue:" + enabledValue);
    }

    private static int setBitForPhone(int featureValue, int enabled, int phoneId) {
        int result;
        if (enabled == 1) {
            result = featureValue | (1 << phoneId);
        } else {
            result = featureValue & ~(1 << phoneId);
        }
        return result;
    }

    public static void triggerSendCfg(Context context,
                                      ImsCommandsInterface imsRilAdapter, int phoneId) {
        int[] params = new int[6];
        int isAllowTurnOff = (getBooleanCarrierConfig(context,
                CarrierConfigManager.KEY_CARRIER_ALLOW_TURNOFF_IMS_BOOL, phoneId)) ? 1 : 0;

        // Get latest feature value from sys props
        params[0] = getFeaturePropValue(PROPERTY_VOLTE_ENALBE, phoneId);  // volte
        params[1] = getFeaturePropValue(PROPERTY_VILTE_ENALBE, phoneId);  // vilte
        params[2] = getFeaturePropValue(PROPERTY_WFC_ENALBE, phoneId);    // vowifi
        params[3] = getFeaturePropValue(PROPERTY_VIWIFI_ENALBE, phoneId); // viwifi
        params[4] = SystemProperties.getInt(PROPERTY_IMS_SUPPORT, 0);   // sms

        // logic to decide eims with (volte|vilte|vowifi|viwifi|sms|(isAllowTurnOff^1))
        params[5] = (params[0] | params[1] | params[2] | params[3] | params[4] | (isAllowTurnOff^1));

        Log.i(TAG, "After 93, send EIMS feature value volte:" + params[0] + ", vilte:" + params[1] +
                ", vowifi:" + params[2] + ", viwifi:" + params[3] + ", sms:" + params[4] +
                ", isAllowTurnOff:" + isAllowTurnOff + ", eims:" + params[5] + ", phoneId:" + phoneId);
        // Allow to send with AT+EIMSCFG
        imsRilAdapter.setImsCfg(params, phoneId, null);
    }

    public static void triggerSendCfgForVolte(Context context,
                                      ImsCommandsInterface imsRilAdapter, int phoneId, int volteOn) {
        int[] params = new int[6];
        int isAllowTurnOff = (getBooleanCarrierConfig(context,
                CarrierConfigManager.KEY_CARRIER_ALLOW_TURNOFF_IMS_BOOL, phoneId)) ? 1 : 0;

        // Get latest feature value from sys props
        params[0] = volteOn;  // volte
        params[1] = getFeaturePropValue(PROPERTY_VILTE_ENALBE, phoneId);  // vilte
        params[2] = getFeaturePropValue(PROPERTY_WFC_ENALBE, phoneId);    // vowifi
        params[3] = getFeaturePropValue(PROPERTY_VIWIFI_ENALBE, phoneId); // viwifi
        params[4] = SystemProperties.getInt(PROPERTY_IMS_SUPPORT, 0);   // sms

        // logic to decide eims with (volte|vilte|vowifi|viwifi|sms|(isAllowTurnOff^1))
        params[5] = (params[0] | params[1] | params[2] | params[3] | params[4] | (isAllowTurnOff^1));

        Log.i(TAG, "After 93, send EIMS feature value volte:" + params[0] + ", vilte:" + params[1] +
                ", vowifi:" + params[2] + ", viwifi:" + params[3] + ", sms:" + params[4] +
                ", isAllowTurnOff:" + isAllowTurnOff + ", eims:" + params[5] + ", phoneId:" + phoneId);
        // Allow to send with AT+EIMSCFG
        imsRilAdapter.setImsCfg(params, phoneId, null);
    }

    private static boolean checkIsPhoneIdValid(int phoneId) {
        if (ImsCommonUtil.supportMims()) {
            if(phoneId > 3 || phoneId < 0) {
                if (DEBUG) Log.d(TAG, "Multi IMS support but phone id invalid," +
                        " phoneId:" + phoneId);
                return false;
            }
        } else {
            if(phoneId > 3 || phoneId < 0) {
                if (DEBUG) Log.d(TAG, "Mutli IMS not support and phone id invalid," +
                        " phoneId:" + phoneId);
                return false;
            }
        }
        return true;
    }

    /**
     * Get the boolean config for sub id from carrier config manager.
     *
     * @param context the context to get carrier service
     * @param key config key defined in CarrierConfigManager
     * @param phoneId phone id
     * @return boolean value of corresponding key.
     */
    public static boolean getBooleanCarrierConfig(Context context, String key, int phoneId) {
        CarrierConfigManager configManager = (CarrierConfigManager) context.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(phoneId);
        if (TELDBG) Log.d(TAG, "getBooleanCarrierConfig: phoneId=" + phoneId + " subId=" + subId);

        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }
        if (b != null) {
            return b.getBoolean(key);
        } else {
            // Return static default defined in CarrierConfigManager.
            return CarrierConfigManager.getDefaultConfig().getBoolean(key);
        }
    }

    /**
     * Get the int config for sub id from carrier config manager.
     *
     * @param context the context to get carrier service
     * @param key config key defined in CarrierConfigManager
     * @param phoneId phone id
     * @return int value of corresponding key.
     */
    public static int getIntCarrierConfig(Context context, String key, int phoneId) {
        CarrierConfigManager configManager = (CarrierConfigManager) context.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(phoneId);
        if (DEBUG) Log.d(TAG, "getIntCarrierConfig: phoneId=" + phoneId + " subId=" + subId);

        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }
        if (b != null) {
            return b.getInt(key);
        } else {
            // Return static default defined in CarrierConfigManager.
            return CarrierConfigManager.getDefaultConfig().getInt(key);
        }
    }

    public static boolean isWfcEnabledByUser(Context context, int phoneId) {
        boolean wfcSetting = false;

        if (ImsCommonUtil.supportMims()) {
            wfcSetting = ImsManager.getInstance(context, phoneId).isWfcEnabledByUser();
        } else {
            wfcSetting = ImsManager.isWfcEnabledByUser(context);
        }
        return wfcSetting;
    }

    public static int getWfcMode(Context context, int phoneId) {
        int wfcMode = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;

        if (ImsCommonUtil.supportMims()) {
            wfcMode = ImsManager.getInstance(context, phoneId).getWfcMode();
        } else {
            wfcMode = ImsManager.getWfcMode(context);
        }
        return wfcMode;
    }

    public static String arrayToString(Object[] array) {
        int maxLength = array.length - 1;

        if (array == null) {
            return "null";
        }
        if (maxLength == -1) {
            return "null";
        }

        StringBuilder b = new StringBuilder();
        b.append('"');
        for (int i = 0; ; i++) {
            b.append(String.valueOf(array[i]));
            if (i == maxLength) {
                return b.append('"').toString();
            }
            b.append("\",\"");
        }
    }
}
