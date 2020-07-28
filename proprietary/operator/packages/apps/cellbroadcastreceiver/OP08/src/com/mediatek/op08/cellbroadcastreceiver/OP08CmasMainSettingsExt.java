
package com.mediatek.op08.cellbroadcastreceiver;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.telephony.SmsManager;

import mediatek.telephony.MtkSmsCbCmasInfo;
import com.mediatek.internal.telephony.gsm.MtkSmsCbConstants;
import com.mediatek.cmas.ext.DefaultCmasMainSettingsExt;


public class OP08CmasMainSettingsExt extends DefaultCmasMainSettingsExt {

    private static final int PRESIDENT_ALERT_ID = 4370;
    private static final String TAG = "CellBroadcastReceiver/OP08CmasMainSettingsExt";

    public static final String KEY_ALERT_SOUND_VOLUME = "enable_key_sound_volume";
    public static final String KEY_ENABLE_ALERT_VIBRATE = "enable_key_alert_vibrate";
    public static final String KEY_ENABLE_CELLBROADCAST = "enable_cell_broadcast";
    //channel 4396/4397
    public static final String ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS = "key_enable_public_safty_message";
    //channel 4398/4399
    public static final String ENABLE_WEA_TEST_ALERTS = "key_enable_wea_test_alerts";

    private float mAlertVolume = 1.0f;
    private Context mContext;

    CheckBoxPreference publicSaftyAlert;
    CheckBoxPreference weaTestAlert;

    public OP08CmasMainSettingsExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public boolean needToaddAlertSoundVolumeAndVibration() {
        Log.d(TAG, "OP08 NeedToaddAlertSoundVolumeAndVibration");
        return true;
    }

    @Override
    public float getAlertVolume(int msgId) {
        Log.d("@M_" + TAG, "[getAlertVolume]");

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        float alertVolume = prefs.getFloat(KEY_ALERT_SOUND_VOLUME, 1.0f);

        Log.d("@M_" + TAG, "[getAlertVolume] AlertVolume: " + alertVolume);

        if (msgId == PRESIDENT_ALERT_ID && alertVolume == 0.0f) {
            Log.d("@M_" + TAG, "[getAlertVolume] PRESIDENT_ALERT");
            return 1.0f;
        }
        return alertVolume;
    }

    @Override
    public boolean getAlertVibration(int msgId) {
        Log.d("@M_" + TAG, "[getAlertVibration]");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);

        if (msgId == PRESIDENT_ALERT_ID) {
            Log.d("@M_" + TAG, "[getAlertVibration] PRESIDENT_ALERT");
            return true;
        }
        return prefs.getBoolean(KEY_ENABLE_ALERT_VIBRATE, true);
    }

    @Override
    public boolean setAlertVolumeVibrate(int msgId, boolean currentValue) {
        Log.d("@M_" + TAG, "[setAlertVolumeVibrate]");
        if (msgId == PRESIDENT_ALERT_ID) {
            Log.d("@M_" + TAG, "[setAlertVolume] PRESIDENT_ALERT");
            return true;
        }
        Log.d("@M_" + TAG, "[setAlertVolumeVibrate] return currentValue:" + currentValue);
        return currentValue;
    }

    @Override
    public void updateVolumeValue(float alertVolume) {
        Log.d("@M_" + TAG, "[updateVolumeValue] " + alertVolume);
        mAlertVolume = alertVolume;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putFloat(KEY_ALERT_SOUND_VOLUME, mAlertVolume);
        editor.commit();
    }

    @Override
    public void updateVibrateValue(boolean value) {
        Log.d("@M_" + TAG, "[updateVibrateValue] " + value);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(KEY_ENABLE_ALERT_VIBRATE, value);
        editor.commit();
    }

    /**
     * Add New Channel Alerts in main Setting
     * @param prefActivity Current Preference Activity
     * @return void
     */
    @Override
    public void addNewChannelAlertsSetting(PreferenceFragment fragment,
                                    String slotKey, int slotId) {
        Log.i(TAG, "Plugin addNewChannelAlertsSetting");
        PreferenceScreen prefSet = fragment.getPreferenceScreen();
        SmsManager defaultManager = SmsManager.getDefault();
        SharedPreferences sharePrefs =
            PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        boolean isPublicSaftyEnable = sharePrefs.getBoolean(
            ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);

        publicSaftyAlert = new CheckBoxPreference(fragment.getContext());
        publicSaftyAlert.setKey(ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS);
        publicSaftyAlert.setChecked(isPublicSaftyEnable);
        publicSaftyAlert.setTitle(
            mContext.getString(R.string.enable_public_safety_messages_title));
        publicSaftyAlert.setSummary(
            mContext.getString(R.string.enable_public_safety_messages_summary));

        prefSet.addPreference(publicSaftyAlert);
        Log.i(TAG, "public Safty alert checkbox added isPublicSaftyEnable = " + isPublicSaftyEnable);

        publicSaftyAlert.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                Log.i(TAG, "public Safty alert checkbox value = " + value);
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, value);
                editor.commit();

                setCellBroadcastRange(defaultManager, value,
                SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE);
                return true;
            }
        });

        boolean isWeaTestEnable = sharePrefs.getBoolean(
            ENABLE_WEA_TEST_ALERTS, false);
        weaTestAlert = new CheckBoxPreference(fragment.getContext());
        weaTestAlert.setKey(ENABLE_WEA_TEST_ALERTS);
        weaTestAlert.setChecked(isWeaTestEnable);
        weaTestAlert.setTitle(
            mContext.getString(R.string.enable_wea_test_messages_title));
        weaTestAlert.setSummary(
            mContext.getString(R.string.enable_wea_test_messages_summary));

        prefSet.addPreference(weaTestAlert);
        Log.i(TAG, "wea test alert checkbox added isWeaTestEnable = " + isWeaTestEnable);

        weaTestAlert.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                Log.i(TAG, "wea test alert checkbox value = " + value);
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(ENABLE_WEA_TEST_ALERTS, value);
                editor.commit();
                // Enable/Disable wea test alert messages (4398-4399).
                setCellBroadcastRange(defaultManager, value,
                SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST_LANGUAGE);
                return true;
            }
        });
    }


    /**
     * check if message enabled for ATT and TMO
     */
    @Override
    public boolean isMessageEnabledByUser(int messageClass, boolean defaultValue, String slotKey) {
        Log.i(TAG, "isMessageEnabledByUser");
        switch (messageClass) {
            case MtkSmsCbCmasInfo.CMAS_CLASS_PUBLIC_SAFETY:
                boolean resOfPub =	PreferenceManager.getDefaultSharedPreferences(mContext)
                .getBoolean(ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);
                Log.d(TAG, "in isMessageEnabledByUser,public safty setting " + resOfPub);
                return resOfPub;
            case MtkSmsCbCmasInfo.CMAS_CLASS_WEA_TEST:
                boolean resOfWea = PreferenceManager.getDefaultSharedPreferences(mContext)
                .getBoolean(ENABLE_WEA_TEST_ALERTS, true);
                Log.d(TAG, "in isMessageEnabledByUser,wea test setting " + resOfWea);
                return resOfWea;
            default:
                return defaultValue;
        }
    }

    /**
     * config ATT only operator channel
     */
    @Override
    public void configOpChannel(Context hostContext, SmsManager manager, String slotKey) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostContext);
        // Enable/Disable public safty alert messages (4396-4397).
        boolean isPublicSaftyEnable = prefs.getBoolean(
            ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);
        setCellBroadcastRange(manager, isPublicSaftyEnable,
        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE);

        // Enable/Disable wea test alert messages (4398-4399).
        boolean isWeaTestEnable = prefs.getBoolean(
            ENABLE_WEA_TEST_ALERTS, false);
        Log.i(TAG, "configOpChannel isPublicSaftyEnable = " + isPublicSaftyEnable +
            ", isWeaTestEnable = " + isWeaTestEnable);
        setCellBroadcastRange(manager, isWeaTestEnable,
            SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST_LANGUAGE);

    }

    /**
     * Enable/disable cell broadcast with messages id range.
     * @param manager SMS manager
     * @param enable True for enabling cell broadcast with id range, otherwise for disabling.
     * @param type GSM or CDMA
     * @param start Cell broadcast id range start
     * @param end Cell broadcast id range end
     */
    private boolean setCellBroadcastRange(
            SmsManager manager, boolean enable, int type, int start, int end) {
        if (enable) {
            return manager.enableCellBroadcastRange(start, end, type);
        } else {
            return manager.disableCellBroadcastRange(start, end, type);
        }
    }
}
