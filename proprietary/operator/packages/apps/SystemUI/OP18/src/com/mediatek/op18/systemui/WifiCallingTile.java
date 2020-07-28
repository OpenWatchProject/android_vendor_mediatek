/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op18.systemui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiManager;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsServiceClass;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.PhoneConstants;
import com.android.systemui.plugins.qs.QSTile.SignalState;
import com.android.systemui.qs.QSHost;
import com.android.systemui.qs.QSTileHost;
import com.android.systemui.qs.tileimpl.QSTileImpl;

import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.op18.systemui.R;


/** Quick settings tile: WifiCalling.
**/
public class WifiCallingTile extends QSTileImpl<SignalState> {
    private static final String TAG = "OP18WifiCallingTile";

    // As per Planner spec, no need to launch any screen on long click. Keeping for future use
    private static final String ACTION_WIRELESS_SETTINGS_LAUNCH
            = "android.settings.WIRELESS_SETTINGS";
    private static final String WFC_TILE = "wificallingsettings";

    private WifiCallingController mController = null;
    private boolean mListening;
    private static WifiCallingTile sWifiCallingTile = null;
    private static Context sContext;
    private ImsManager mImsManager;

    private static boolean sIsClassReceiverRegistered = false;
    private static QSTileHost sQSTileHostInstance = null;

    private static final BroadcastReceiver sClassReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "Class receiver, onReceive action:" + intent.getAction());
            if (CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED
                    .equals(intent.getAction())) {
                Log.d(TAG, "PhoneId: " + intent.getIntExtra(PhoneConstants.PHONE_KEY, -1));
                Log.d(TAG, "SubId: " + intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1));
                String tileList = context.getResources()
                        .getString(R.string.quick_settings_tiles_op18);
                OP18QuickSettingsExt qs = new OP18QuickSettingsExt(context);
                for (String tile : tileList.split(",")) {
                    tile = tile.trim();
                    if (tile.isEmpty()) {
                        continue;
                    }
                    if (WFC_TILE.equals(tile)) {
                        if (qs.doOperatorSupportTile(tile)) {
                            sQSTileHostInstance.addTile(tile);
                            Log.d(TAG, "WFC tile added");
                        } else {
                            sQSTileHostInstance.removeTile(tile);
                            Log.d(TAG, "WFC tile removed");
                        }
                        break;
                    }
                }
            }
        }
    };

    /**
     * Constructor.
     * @param host host
     */
    private WifiCallingTile(QSHost host) {
        super(host);
        mController = WifiCallingController.getInstance(sContext);
        mImsManager = ImsManager.getInstance(sContext, getMainCapabilityPhoneId());
    }

    /**
     * Provides WifiCalling tile instance.
     * @param context A Context object
     * @param host host
     * @return WifiCallingTile
     */
    public static WifiCallingTile getInstance(Context context, QSHost host) {
        if (sWifiCallingTile == null) {
            sContext = context;
            sWifiCallingTile = new WifiCallingTile(host);
        }
        return sWifiCallingTile;
    }

    /**
     * Sets QsTileHost app object.
     * @param context A Context object
     * @param o QsTileHost instance
     * @return
     */
    public static void addQSTileHostInstance(Context context, Object o) {
        sQSTileHostInstance = (QSTileHost) o;
        sContext = context;
        Log.d(TAG, "sQSTileHostInstance: " + sQSTileHostInstance);
        Log.d(TAG, "sIsClassReceiverRegistered: " + sIsClassReceiverRegistered);
        if (sIsClassReceiverRegistered) {
            sContext.unregisterReceiver(sClassReceiver);
        }
        final IntentFilter filter
                = new IntentFilter(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        sContext.registerReceiver(sClassReceiver, filter);
        sIsClassReceiverRegistered = true;
    }

    @Override
    public SignalState newTileState() {
        return new SignalState();
    }

    @Override
    public void handleSetListening(boolean listening) {
        if (mListening == listening) {
            return;
        }
        mListening = listening;
        if (listening) {
            final IntentFilter filter = new IntentFilter();
            filter.addAction(ImsManager.ACTION_IMS_SERVICE_DOWN);
            //filter.addAction(ImsManager.ACTION_IMS_STATE_CHANGED);
            filter.addAction(PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED);
            sContext.registerReceiver(mReceiver, filter);
            try {
                mImsManager.addRegistrationListener(ImsServiceClass.MMTEL, mImsRegListener);
            } catch (ImsException e) {
                Log.e(TAG, "addRegistrationListener: " + e);
            }
        } else {
            sContext.unregisterReceiver(mReceiver);
            try {
                mImsManager.removeRegistrationListener(mImsRegListener);
            } catch (ImsException e) {
                Log.e(TAG, "removeRegistrationListener: " + e);
            }
        }
    }

    @Override
    protected void handleClick() {
        final WifiCallingInfo info = new WifiCallingInfo();
        if (mController.isClickable(sContext)) {
            info.mEnabled = !mController.isWifiCallingOn();
            mController.setWifiCallingSetting(info.mEnabled);
        } else {
            info.mEnabled = mController.isWifiCallingOn();
        }
        info.mRegistered =
            ((TelephonyManager) sContext
            .getSystemService(Context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
        Log.d(TAG, "handleClick desiredState=" + info.mEnabled);
        refreshState(info);
    }

    /* As per planner, not needed now. Keeping for future reference.
    @Override
    protected void handleLongClick() {
        Intent intent = new Intent(ACTION_LAUNCH_WFC_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        mHost.startActivityDismissingKeyguard(intent);
    }*/

    @Override
    protected void handleUpdateState(SignalState state, Object arg) {
        WifiCallingInfo info = (WifiCallingInfo) arg;
        Log.d(TAG, "handleUpdateState info=" + info);
        // TODO: check why info is coming as null
        if (info == null) {
            info = new WifiCallingInfo();
            info.mEnabled = mController.isWifiCallingOn();
            info.mRegistered = ((TelephonyManager) sContext
                .getSystemService(Context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
        }
        updateState(state, info);
    }

    @Override
    public CharSequence getTileLabel() {
        return sContext.getString(R.string.quick_settings_wificalling_label);

    }

    @Override
    protected void handleLongClick() {
        Log.d(TAG, "in handleLongClick");
        handleClick();
    }

    @Override
    public Intent getLongClickIntent() {
        Log.d(TAG, "getLongClickIntent");
        return new Intent("android.settings.WIFI_CALLING_SETTINGS");
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.QS_PANEL;
    }

    private void updateState(SignalState state, WifiCallingInfo info) {
        //state.enabled = info.mEnabled;
        //state.connected = info.mRegistered;
        //state.visible = true;
        Log.d(TAG, "handleUpdateState enabled=" + info.mEnabled + ",registered=" +
                info.mRegistered);
        if (info.mEnabled && info.mRegistered) {
            state.icon = com.mediatek.op18.systemui.ResourceIcon
                    .get(R.drawable.ic_signal_vowifi_enable, sContext);
            state.label = getWifiSsId();
        } else if (info.mEnabled) {
            state.icon = com.mediatek.op18.systemui.ResourceIcon
                    .get(R.drawable.ic_signal_vowifi_enable, sContext);
            state.label = sContext.getString(R.string.quick_settings_wificalling_label);
       } else {
            state.icon = com.mediatek.op18.systemui.ResourceIcon
                    .get(R.drawable.ic_signal_vowifi_disable, sContext);
            state.label = sContext.getString(R.string.quick_settings_wificalling_label);
       }
        if (!mController.isClickable(sContext)) {
            Log.d(TAG, "NonClickable, so disable icon");
            state.icon = com.mediatek.op18.systemui.ResourceIcon
                    .get(R.drawable.ic_signal_vowifi_disable, sContext);
        }
    }

    private String getWifiSsId() {
        WifiManager wm = (WifiManager) sContext.getSystemService(Context.WIFI_SERVICE);
        String ssId = wm.getConnectionInfo().getSSID();
        Log.d(TAG, "ssId: " + ssId);
        StringBuilder sb = new StringBuilder(ssId);
        if (sb.charAt(0) == '"') {
            sb.deleteCharAt(0);
        }
        if (sb.charAt(sb.length() - 1) == '"') {
            sb.deleteCharAt(sb.length() - 1);
        }
        String newSsId = sb.toString();
        Log.d(TAG, "new ssId: " + newSsId);
        return newSsId;
    }

    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }
        Log.d(TAG, "getMainCapabilityPhoneId = " + phoneId);
        return phoneId;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        final WifiCallingInfo mInfo = new WifiCallingInfo();
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive action:" + intent.getAction());
            if (ImsManager.ACTION_IMS_SERVICE_DOWN.equals(intent.getAction())) {
                mInfo.mEnabled = mController.isWifiCallingOn();
                mInfo.mRegistered = false;
                refreshState(mInfo);
            } /*else if (ImsManager.ACTION_IMS_STATE_CHANGED.equals(intent.getAction())) {
                mInfo.mEnabled = mController.isWifiCallingOn();
                mInfo.mRegistered = ((TelephonyManager)context
                        .getSystemService(context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
                refreshState(mInfo);
            } */else if (PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED
                    .equals(intent.getAction())) {
                mController.setCallType(intent);
                //final WifiCallingInfo info = new WifiCallingInfo();
                mInfo.mEnabled = mController.isWifiCallingOn();
                mInfo.mRegistered = ((TelephonyManager) sContext
                        .getSystemService(Context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
                Log.d(TAG, "desiredState=" + mInfo.mEnabled);
                refreshState(mInfo);
            }
        }
    };

    private final ImsConnectionStateListener mImsRegListener = new ImsConnectionStateListener() {
        final WifiCallingInfo mInfo = new WifiCallingInfo();
        @Override
        public void onFeatureCapabilityChanged(int serviceClass,
                int[] enabledFeatures, int[] disabledFeatures) {
            mInfo.mEnabled = mController.isWifiCallingOn();
            mInfo.mRegistered = ((TelephonyManager)sContext
                    .getSystemService(sContext.TELEPHONY_SERVICE)).isWifiCallingAvailable();
            refreshState(mInfo);
        }
    };

    /** Class having info needed by Tile.
    **/
    private static final class WifiCallingInfo {
        boolean mEnabled;
        boolean mRegistered;

        @Override
        public String toString() {
            return new StringBuilder("CallbackInfo[")
                .append("enabled=").append(mEnabled)
                .append(",registered=").append(mRegistered)
                .append(']').toString();
        }
    }
}
