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

package com.mediatek.op15.ims;

import android.content.Context;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.ims.internal.ext.ImsManagerExt;

import java.util.Arrays;
import java.util.List;

/**
 * Plugin implementation for Imsmanager.
 */

public class Op15ImsManagerExt extends ImsManagerExt {

    private static final String TAG = "Entitlement-Op15ImsManagerExt";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    //private static final String VLT_SETTING_SYTEM_PROPERTY =
            //"persist.vendor.ril.vt.setting.support";
    //private static final String WFC_SETTING_SYTEM_PROPERTY =
            //"persist.vendor.ril.wfc.setting.support";
    private static final List<String> OP15_MCCMNC_LIST = Arrays.asList("23410");
    private Context mContext;

    /**
     * Constructor of plugin.
     * @param context context
     */
    public Op15ImsManagerExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    /**
      * An API to customize platform enabled status.
      * @param context The context for retrive plug-in.
      * @param feature The IMS feature defined in ImsConfig.FeatureConstants.
      * @param phoneId PhoneId for which feature to be enabled.
      * @return return enabled status.
      */
    public boolean isFeatureEnabledByPlatform(Context context, int feature, int phoneId) {
        //if ("1".equals(SystemProperties.get("persist.vendor.mtk_bypass_ImsBySim"))) {
            //Log.d(TAG, "Due to system property set, return directly");
            //return true;
        //}
        Log.d(TAG, "feature:" + feature + ", phoneId:" + phoneId);
        if ((feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE)
                || (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI)) {
                return isFeatureProvisionedOnDevice(feature, phoneId);
        } else {
            return super.isFeatureEnabledByPlatform(context, feature, phoneId);
        }
    }

    public boolean isFeatureProvisionedOnDevice(int feature, int phoneId) {
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(mContext, phoneId);
        int value = 0;
        boolean result = false;
        if (!isEntitlementEnabled()) {
            Log.d(TAG, "Entitlement sys property not enabled return true");
            return true;
        }
        if (isMccMncReady(phoneId) && !isOp15Card(phoneId)) {
            Log.d(TAG, "This operator is no need to check provision.");
            return true;
        }
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    if(feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE) {
                        value = imsConfig.getProvisionedValue(
                            ImsConfig.ConfigConstants.VLT_SETTING_ENABLED);
                        Log.d(TAG, "VoLTE provisioned value = " + value);
                        if (value == 1) {
                            result = true;
                        }
                    } else {
                        value = imsConfig.getProvisionedValue(
                            ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED);
                        Log.d(TAG, "VoWifi provisioned value = " + value);
                        if (value == 1) {
                            result = true;
                        }
                    }
                }
            } catch (ImsException e) {
                Log.e(TAG, "Volte not updated, ImsConfig null");
                e.printStackTrace();
            } catch (RuntimeException e) {
                Log.e(TAG, "ImsConfig not ready");
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "Volte not updated, ImsManager null");
        }
        /*if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE) {
            result = SystemProperties.get(VLT_SETTING_SYTEM_PROPERTY + phoneId).equals("1");
            Log.d(TAG, "phoneId:" + phoneId + ", volte system property value = " + result);

        } else if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI) {
            result = SystemProperties.get(WFC_SETTING_SYTEM_PROPERTY + phoneId).equals("1");
            Log.d(TAG, "phoneId:" + phoneId + ", wfc system property value = " + result);
        }*/
        Log.d(TAG, "isFeatureProvisionedOnDevice returns " + result);
        return result;
    }

    private static int getSubIdUsingPhoneId(int phoneId) {
        int [] values = SubscriptionManager.getSubId(phoneId);
        if(values == null || values.length <= 0) {
            return SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            Log.d(TAG, "getSubIdUsingPhoneId:" + values[0] + ", phoneId:" + phoneId);
            return values[0];
        }
    }

    private static boolean isOp15Card(int phoneId) {
        String mccMnc = getSimOperatorNumericForPhone(phoneId);
        return OP15_MCCMNC_LIST.contains(mccMnc);
    }

    private static boolean isMccMncReady(int phoneId) {
        String mccMnc = getSimOperatorNumericForPhone(phoneId);
        if (TextUtils.isEmpty(mccMnc)) {
            Log.d(TAG, "MccMnc is empty.");
            return false;
        }
        return true;
    }

    private static String getSimOperatorNumericForPhone(int phoneId) {
        String mccMncPropertyName = "";
        String mccMnc = "";
        int phoneType = 0;
        int subId = getSubIdUsingPhoneId(phoneId);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.d(TAG, "Is Invalid Subscription id.");
            return mccMnc;
        }
        phoneType = TelephonyManager.getDefault().getCurrentPhoneType(subId);
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc." + phoneId;
            }
        } else {
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc." + phoneId;
            }
        }
        mccMnc = SystemProperties.get(mccMncPropertyName, "");
        if (ENG || TELDBG) {
            Log.d(TAG, "getMccMnc, mccMnc value:" + mccMnc);
        }
        return mccMnc;
    }

    private static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        Log.d(TAG, "in Op15fwkplugin, isEntitlementEnabled:" + isEntitlementEnabled);
        return isEntitlementEnabled;
    }
}