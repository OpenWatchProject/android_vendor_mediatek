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
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceScreen;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;

import com.android.settings.wifi.LongPressAccessPointPreference;
import com.android.settings.wifi.details.WifiNetworkDetailsFragment;
import com.android.mtksettingslib.wifi.AccessPoint;
import com.android.mtksettingslib.wifi.WifiTracker;

import com.mediatek.settings.ext.DefaultWifiSettingsExt;

import java.util.List;

/**
 * OP01 plugin implementation of WifiSettings feature.
 */
public class Op01WifiSettingsExt extends DefaultWifiSettingsExt {

    private static final String TAG = "Op01WifiSettingsExt";
    private static final int MENU_ID_DISCONNECT = Menu.FIRST + 100;
    private static final String KEY_REFRESH_PREFERENCE = "refersh_preference_key";
    private static final String PREF_KEY_ADDITIONAL_SETTINGS = "additional_settings";
    private static final String KEY_CONFIGURE_SETTINGS = "configure_settings";

    private Preference mRefreshPreference;
    private Context mContext;
    private WifiManager mWifiManager;

    private PreferenceCategory mCmccConfigedAP = null;
    private PreferenceCategory mCmccNewAP = null;
    private String CMCC_CONFINGED_AP = "cmcc_confinged_ap";
    private String CMCC_NEW_AP = "cmcc_new_ap";
    //google default category
    private static final String PREF_KEY_CONNECTED_ACCESS_POINTS = "connected_access_point";
    private static final String PREF_KEY_ACCESS_POINTS = "access_points";
    private PreferenceCategory mConnectedAccessPoint = null;
    private PreferenceCategory mAccessPoints = null;
    private static WifiTracker mWifiTracker = null;
    /**
     * Op01WifiSettingsExt.
     * @param context Context
     */
    public Op01WifiSettingsExt(Context context) {
        super();
        mContext = context;
        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        Log.d("@M_" + TAG, "WifiSettingsExt mContext = " + mContext);
    }

    @Override
    public  void updateContextMenu(ContextMenu menu, int menuId, Preference preference) {
        Log.d("@M_" + TAG, "updateContextMenu preference = " + preference);
        boolean isFirstOne = false;
        if (null !=  mCmccConfigedAP) {
            Preference firstPreference = mCmccConfigedAP.getPreference(0);
            String preferenceString = preference.getTitle().toString();
            String firstPreferenceString = firstPreference.getTitle().toString();
            Log.d("@M_" + TAG, "updateContextMenu firstPreference = " + firstPreference
                    + " preferenceString = " + preferenceString
                    + " firstPreferenceString = " + firstPreferenceString);
            isFirstOne = preferenceString.equals(firstPreferenceString);
        }
        Log.d("@M_" + TAG, "updateContextMenu isFirstOne = " + isFirstOne);
        if (preference != null && isFirstOne
            && mWifiManager.getConnectionInfo().getSupplicantState()
                    == SupplicantState.COMPLETED) {
            Log.d("@M_" + TAG, "updateContextMenu string = "
                + mContext.getString(R.string.wifi_menu_disconnect));
            menu.add(Menu.NONE, menuId, 0, mContext.getString(R.string.wifi_menu_disconnect));
        }
    }

    @Override
    public boolean addModifyMenu(boolean isSaved) {
        Log.i(TAG, "addModifyMenu isSaved = " + isSaved);
        return isSaved;
    }

    @Override
    public void emptyCategory(PreferenceScreen screen) {
        Log.d("@M_" + TAG, "emptyCategory()");
        if (null != mCmccConfigedAP) {
            mCmccConfigedAP.removeAll();
            screen.removePreference(mCmccConfigedAP);
        }
        if (null != mCmccNewAP) {
            mCmccNewAP.removeAll();
            screen.removePreference(mCmccNewAP);
        }
    }

    @Override
    public void emptyScreen(PreferenceScreen screen) {
        if (null != mConnectedAccessPoint) {
            mConnectedAccessPoint.removeAll();
            mConnectedAccessPoint.setVisible(false);
        }
        if (null != mAccessPoints) {
            mAccessPoints.removeAll();
            mAccessPoints.setVisible(false);
        }
    }

    @Override
    public boolean disconnect(MenuItem item, WifiConfiguration wifiConfig) {
        Log.d("@M_" + TAG, "disconnect() item from current active AP");
        switch (item.getItemId()) {
        case MENU_ID_DISCONNECT:
            disconnect(wifiConfig);
            return true;
        default:
            break;
        }
        return false;
    }

    public void disconnect(WifiConfiguration wifiConfig) {
        Log.d("@M_" + TAG, "disconnect() from current active AP");
        if (wifiConfig != null) {
            int networkId = wifiConfig.networkId;
            if (networkId != WifiConfiguration.INVALID_NETWORK_ID) {
                mWifiManager.disable(networkId, null);
            }
        }
    }

    @Override
    public void init(PreferenceScreen screen) {
        mConnectedAccessPoint = (PreferenceCategory)screen
                .findPreference(PREF_KEY_CONNECTED_ACCESS_POINTS);
        mAccessPoints = (PreferenceCategory)screen
                .findPreference(PREF_KEY_ACCESS_POINTS);
        if (mCmccConfigedAP == null) {
            mCmccConfigedAP = new PreferenceCategory((screen.getPreferenceManager().getContext()));
            String cmccConfigedAPString = mContext.getResources().getString(R.string.configed_access_points);
            int cmccConfigedAPOrder = mConnectedAccessPoint.getOrder();
            mCmccConfigedAP.setTitle(cmccConfigedAPString);
            mCmccConfigedAP.setKey(CMCC_CONFINGED_AP);
            mCmccConfigedAP.setOrder(cmccConfigedAPOrder);
            Log.i(TAG, "create mCmccConfigedAP"
                    + " title = " + cmccConfigedAPString
                    + " cmccConfigedAPOrder = " + cmccConfigedAPOrder);
        }
        if (mCmccNewAP == null) {
            mCmccNewAP = new PreferenceCategory((screen.getPreferenceManager().getContext()));
            String cmccNewAPString = mContext.getResources().getString(R.string.new_access_points);
            int cmccNewAPOrder = mAccessPoints.getOrder();
            mCmccNewAP.setTitle(cmccNewAPString);
            mCmccNewAP.setKey(CMCC_NEW_AP);
            mCmccNewAP.setOrder(cmccNewAPOrder);
            Log.i(TAG, "create mCmccNewAP"
                    + " title = " + cmccNewAPString
                    + " cmccNewAPOrder = " + cmccNewAPOrder);
        }
        Log.i(TAG, "init addCategories");
        emptyScreen(screen);
    }

    @Override
    public boolean removeConnectedAccessPointPreference() {
        return true;
    }

    @Override
    public void addPreference(PreferenceScreen screen,
            PreferenceCategory preferenceCategory,
            Preference addPreference,
            boolean isConfiged){
        if (isConfiged) {
            if (null == screen.findPreference(CMCC_CONFINGED_AP)) {
                screen.addPreference(mCmccConfigedAP);
            }
            if (addPreference instanceof LongPressAccessPointPreference
                    && ((LongPressAccessPointPreference)addPreference)
                        .getAccessPoint().isActive()) {
                ((LongPressAccessPointPreference)addPreference).getAccessPoint()
                        .saveWifiState(addPreference.getExtras());
                addPreference.setFragment(WifiNetworkDetailsFragment.class.getName());
                ((LongPressAccessPointPreference)addPreference).refresh();
            }
            mCmccConfigedAP.addPreference(addPreference);
        } else {
            if (null == screen.findPreference(CMCC_NEW_AP)) {
                screen.addPreference(mCmccNewAP);
            }
            mCmccNewAP.addPreference(addPreference);
        }
    }

    @Override
    public void emptyConneCategory(PreferenceScreen screen){
        if (null != mCmccConfigedAP) {
            mCmccConfigedAP.removeAll();
            screen.removePreference(mCmccConfigedAP);
        }
    }
    @Override
    public void addRefreshPreference(PreferenceScreen screen,
            Object wifiTracker,
            boolean isUiRestricted) {
        Log.d(TAG, "addRefreshPreference, isUiRestricted = " + isUiRestricted);
        if (isUiRestricted) {
            return;
        } else {
            mWifiTracker = (WifiTracker)wifiTracker;
            Preference configPreference
                = screen.findPreference(KEY_CONFIGURE_SETTINGS);
            mRefreshPreference = new Preference(
                    configPreference.getPreferenceManager().getContext());
            mRefreshPreference.setKey(KEY_REFRESH_PREFERENCE);
            mRefreshPreference.setTitle(mContext.getResources()
                    .getString(R.string.menu_stats_refresh));
            Preference oldRefreshPreference
                = (Preference)screen.findPreference(KEY_REFRESH_PREFERENCE);
            if (null != oldRefreshPreference) {
                screen.removePreference(oldRefreshPreference);
            }
            screen.addPreference(mRefreshPreference);
            mRefreshPreference.setEnabled(mWifiTracker.isWifiEnabled());
        }
    }

    @Override
    public boolean customRefreshButtonClick(Preference preference) {
        if (null != preference
                && KEY_REFRESH_PREFERENCE.equals(preference.getKey())) {
            Log.d(TAG, "customRefreshButtonClick");
            if (null != mWifiTracker) {
                Log.d(TAG, "customRefreshButtonClick, mWifiTracker.forceScan()");
                mWifiTracker.forceScan();
            }
            return true;
        }
        return false;
    }

    @Override
    public void customRefreshButtonStatus(boolean checkStatus) {
        Log.d(TAG, "customRefreshButtonStatus checkStatus = " + checkStatus);
        if (null != mRefreshPreference) {
            Log.d(TAG, "customRefreshButtonStatus checkStatus1 = " + checkStatus);
            mRefreshPreference.setEnabled(checkStatus);
        }
    }
}
