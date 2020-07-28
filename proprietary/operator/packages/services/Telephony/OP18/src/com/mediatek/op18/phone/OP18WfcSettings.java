package com.mediatek.op18.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

/**
 * Class to support operator customizations for WFC settings.
 */
public class OP18WfcSettings {

    private static final String TAG = "Op18WfcSettings";
    public static final String NOTIFY_CALL_STATE = "OP18:call_state_Change";
    public static final String CALL_STATE = "call_state";
    public static final int CALL_STATE_IDLE = 1;
    public static final int CALL_STATE_CS = 2;
    public static final int CALL_STATE_PS = 3;
    static OP18WfcSettings sWfcSettings = null;

    Context mContext;
    IntentFilter mIntentFilter;
    IntentFilter  mCallNotifyIntentFilter;
    IntentFilter mWfcSettingsReceiverIntentFilter;
    private WfcSwitchController mController;
    private Op18WfcSettingsReceiver mWfcSettingsReceiver = null;

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "action: " + intent.getAction());
            if (TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE.equals(intent.getAction())
                    || (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())
                            && IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(intent
                            .getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE)))) {
                if (ImsManager.isWfcEnabledByPlatform(context)) {
                    Log.d(TAG, "Sim is of RJIL, add WFC setting");
                    mController.addWfcPreference();
                } else {
                    Log.d(TAG, "Sim not RJIL, remove WFC setting");
                    mController.removeWfcPreference();
                }
            }
        }
    };

    private final BroadcastReceiver mCallNotifyBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "action: " + intent.getAction());
            int callState = intent.getIntExtra(CALL_STATE, CALL_STATE_IDLE);
            int phoneId = intent.getIntExtra("phoneId", -1);
            ImsManager imsManager = ImsManager.getInstance(context, phoneId);
            if (!imsManager.isWfcEnabledByPlatform()) {
                Log.d(TAG, "isWfcEnabledByPlatform: false");
                return;
            }
            Log.v(TAG, "br call_satte: " + callState);
            mController.updateWfcSwitchState(callState);
        }
    };

    private final ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            this.onChange(selfChange, Settings.Global
            .getUriFor(Settings.Global.WFC_IMS_ENABLED));
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            int callState = Settings.Global.getInt(mContext.getContentResolver(),
                    OP18WfcSettings.CALL_STATE, OP18WfcSettings.CALL_STATE_IDLE);
            mController.updateWfcSwitchState(callState);
        }
    };

    private OP18WfcSettings(Context context) {
       mContext = context;
       mController = WfcSwitchController.getInstance(context);
       mCallNotifyIntentFilter = new IntentFilter(NOTIFY_CALL_STATE);
       mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
       mIntentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);

       mWfcSettingsReceiver = Op18WfcSettingsReceiver.getInstance();
       mWfcSettingsReceiverIntentFilter = new IntentFilter(
            PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED);
    }

    /** Returns instance of OP18WfcSettings.
         * @param context context
         * @return OP18WfcSettings
         */
    public static OP18WfcSettings getInstance(Context context) {

        if (sWfcSettings == null) {
            sWfcSettings = new OP18WfcSettings(context);
        }
        return sWfcSettings;
    }

    /** Customize WFC pref as per operator requirement
         * @param context context
         * @param preferenceScreen preferenceScreen
         * @return
         */
    public void customizedWfcPreference(Context context, PreferenceScreen preferenceScreen,
                    int phoneId) {
        Log.d(TAG, "Call Setting preferenceScreen:");
        mController.customizedWfcPreference(context, preferenceScreen, phoneId);
    }

    public void customizedWfcPreference(Context context, PreferenceScreen preferenceScreen,
                    PreferenceCategory callingCategory, int phoneId) {
        Log.d(TAG, "Main Setting preferenceScreen:");
        mController.customizedWfcPreference(context, preferenceScreen, callingCategory, phoneId);
    }

    /** Returns instance of OP18WfcSettings.
     * @return
     */
    public void removeWfcPreference() {
        mController.removeWfcPreference();
    }

    /** Registers receiver.
     * @return
     */
    public void register() {
        mContext.registerReceiver(mBroadcastReceiver, mIntentFilter);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(mCallNotifyBroadcastReceiver,
                mCallNotifyIntentFilter);

        mContext.getContentResolver().registerContentObserver(Settings.Global
                .getUriFor(Settings.Global.WFC_IMS_ENABLED), false, mContentObserver);
    }

    /** Unregisters receiver.
     * @return
     */
    public void unRegister() {
        mContext.unregisterReceiver(mBroadcastReceiver);
        LocalBroadcastManager.getInstance(mContext).unregisterReceiver(
                mCallNotifyBroadcastReceiver);
        mContext.getContentResolver().unregisterContentObserver(mContentObserver);
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        return mController.onPreferenceTreeClick(preferenceScreen, preference);
    }
}
