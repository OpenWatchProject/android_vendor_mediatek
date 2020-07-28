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
import android.content.res.Resources;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;

import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.mtksettingslib.wifi.AccessPoint;

import com.mediatek.settings.ext.DefaultWifiExt;

import java.util.ArrayList;
import java.util.List;

/**
 * OP01 plugin implementation of WifiExt feature.
 */
public class Op01WifiExt extends DefaultWifiExt {
    private static final String TAG = "Op01WifiExt";
    static final String CMCC_SSID = "CMCC";
    static final String CMCC_AUTO_SSID = "CMCC-AUTO";
    static final int SECURITY_EAP = 3;

    /* These values from from "wifi_eap_method" resource array */
    static final int WIFI_EAP_METHOD_PEAP = 0;
    static final int WIFI_EAP_METHOD_SIM = 4;
    static final int WIFI_EAP_METHOD_NUM = 2;
    static final int WIFI_EAP_SKIP_NUM = 2;

    private Context mContext;
    private WifiManager mWifiManager;

    /**
     * Op01WifiExt.
     * @param context Context
     */
    public Op01WifiExt(Context context) {
        super(context);
        mContext = context;
        mContext.setTheme(R.style.SettingsPluginBase);
        mWifiManager = ((WifiManager) mContext.getSystemService(Context.WIFI_SERVICE));
        Log.d("@M_" + TAG, "Op01WifiExt");
    }

    @Override
    public void setSecurityText(TextView view) {
        view.setText(mContext.getString(R.string.wifi_security_cmcc));
        Log.d("@M_" + TAG, "set wifi_security_cmcc");
    }

    @Override
    public void setProxyText(TextView view) {
        view.setText(mContext.getString(R.string.proxy_exclusionlist_label_cmcc));
    }

     @Override
     public void setEapMethodArray(ArrayAdapter adapter, String ssid, int security) {
        Log.d("@M_" + TAG, "setEapMethodArray()");
        if (ssid != null
            && ((CMCC_SSID.equals(ssid) && SECURITY_EAP == security)
                || (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security))) {
            String[] eapString =
                mContext.getResources().getStringArray(R.array.wifi_eap_method_values);
            adapter.clear();
            for (int i = 0; i < WIFI_EAP_METHOD_NUM; i++) {
                adapter.insert(eapString[i], i);
            }
        }
    }

    @Override
    public void hideWifiConfigInfo(Builder builder , Context context) {
        if (builder != null) {
            AccessPoint accessPoint = builder.getAccessPoint();
            View view = (View) builder.getViews();
            Log.d("@M_" + TAG, "hideWifiConfigInfo():" + accessPoint);
            if (accessPoint == null || view == null) {
                return;
            }
            String ssid = accessPoint.getSsidStr();
            int security = accessPoint.getSecurity();
            Log.d("@M_" + TAG, "hideWifiConfigInfo():ssid" + ssid + "," + security);
           if (ssid == null
              || !((CMCC_SSID.equals(ssid) && SECURITY_EAP == security)
                    || (CMCC_AUTO_SSID.equals(ssid)
                        && SECURITY_EAP == security))) {
               return;
            }

            int networkId = WifiConfiguration.INVALID_NETWORK_ID;
            WifiConfiguration wifiConfig = accessPoint.getConfig();
            if (wifiConfig != null) {
                networkId = wifiConfig.networkId;
            }
            boolean edit = builder.getEdit();
            List<View> lists = new ArrayList<View>();
            Resources res = null;
            res = context.getResources();
            String packageName = context.getPackageName();
            lists.add(view.findViewById(
                res.getIdentifier("info", "id", packageName)));
            lists.add(view.findViewById(
                res.getIdentifier("wifi_advanced_toggle", "id", packageName)));
            lists.add(view.findViewById(
                res.getIdentifier("wifi_advanced_fields", "id", packageName)));

            if ((networkId == WifiConfiguration.INVALID_NETWORK_ID)
                || (networkId != WifiConfiguration.INVALID_NETWORK_ID && edit)) {
                for (int i = 0; i < lists.size(); i++) {
                    ((View) lists.get(i)).setVisibility(View.GONE);
                }
                LinearLayout eap = (LinearLayout) view.findViewById(
                    res.getIdentifier("eap", "id", packageName));
                int count = eap.getChildCount();
                for (int j = WIFI_EAP_SKIP_NUM; j < count; j++) {
                    ((View) eap.getChildAt(j)).setVisibility(View.GONE);
                }
                LinearLayout identity = (LinearLayout) view.findViewById(
                    res.getIdentifier("l_identity", "id", packageName));
                Spinner eapMethodSpinner = (Spinner) view.findViewById(
                    res.getIdentifier("method", "id", packageName));
                if (eapMethodSpinner != null) {
                    int eapMethod = eapMethodSpinner.getSelectedItemPosition();
                    if (eapMethod == WIFI_EAP_METHOD_PEAP) {
                         identity.setVisibility(View.VISIBLE);
                    }
                }
            }
        }
    }

     @Override
     public int getEapMethodbySpinnerPos(int spinnerPos, String ssid, int security) {
        Log.d("@M_" + TAG, "getEapMethodbySpinnerPos() spinnerPos = " + spinnerPos);
        Log.d("@M_" + TAG, "getEapMethodbySpinnerPos() ssid = " + ssid);
        Log.d("@M_" + TAG, "getEapMethodbySpinnerPos() security = " + security);
        if (ssid != null
            && ((CMCC_SSID.equals(ssid) && SECURITY_EAP == security)
                || (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security))) {
            if (spinnerPos == 1) {
                spinnerPos = WIFI_EAP_METHOD_SIM;
            } else {
                spinnerPos = WIFI_EAP_METHOD_PEAP;
            }
        }
        Log.d("@M_" + TAG, "getEapMethodbySpinnerPos() EapMethod = " + spinnerPos);
        if (spinnerPos < 0) {
            spinnerPos = 0;
        }
        return spinnerPos;
    }

    @Override
    public int getPosByEapMethod(int spinnerPos, String ssid, int security) {
        Log.d("@M_" + TAG, "getPosByEapMethod() EapMethod = " + spinnerPos);
        Log.d("@M_" + TAG, "getPosByEapMethod() ssid = " + ssid);
        Log.d("@M_" + TAG, "getPosByEapMethod() security = " + security);
        if (ssid != null
            && ((CMCC_SSID.equals(ssid) && SECURITY_EAP == security)
                || (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security))) {
            if (spinnerPos == WIFI_EAP_METHOD_SIM) {
                spinnerPos = 1;
            } else {
                spinnerPos = 0;
            }
        }

        Log.d("@M_" + TAG, "getPosByEapMethod() spinnerPos = " + spinnerPos);
        if (spinnerPos < 0) {
            spinnerPos = 0;
        }
        return spinnerPos;
    }

    @Override
    public void addPreferenceController(Object controllers,
            Object wifiPreferenceController) {
        Log.i(TAG, "addPreferenceController");
        List<AbstractPreferenceController> con =  (List<AbstractPreferenceController>)controllers;
        AbstractPreferenceController preferenceController
            = (OP01WifiPreferenceController)wifiPreferenceController;
        con.add(preferenceController);
    }

    @Override
    public Object createWifiPreferenceController(Context context, Object lifecycle) {
        Log.i(TAG, "createWifiPreferenceController START");
        Lifecycle cycle = (Lifecycle) lifecycle;
        OP01WifiPreferenceController wifiPreferenceController
              = new OP01WifiPreferenceController(context, mContext, cycle);
        return wifiPreferenceController;
    }

}
