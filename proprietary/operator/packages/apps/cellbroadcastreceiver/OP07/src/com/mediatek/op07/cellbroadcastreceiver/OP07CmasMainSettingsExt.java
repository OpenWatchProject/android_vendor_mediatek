
package com.mediatek.op07.cellbroadcastreceiver;

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
import android.telephony.SubscriptionManager;

import mediatek.telephony.MtkSmsCbCmasInfo;
import com.mediatek.internal.telephony.gsm.MtkSmsCbConstants;
import com.mediatek.cmas.ext.DefaultCmasMainSettingsExt;

public class OP07CmasMainSettingsExt extends DefaultCmasMainSettingsExt {

    private static final String TAG = "[CMAS]OP07CmasMainSettingsExt";
    private static final int PRESIDENT_ALERT_ID = 4370;

    public static final String KEY_SPANISH_ALERT = "enable_spanish_alert";
    //channel 4396/4397
    public static final String ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS = "key_enable_public_safty_message";
    //channel 4398/4399
    public static final String ENABLE_WEA_TEST_ALERTS = "key_enable_wea_test_alerts";

    private Context mContext;

    CheckBoxPreference publicSaftyAlert;
    CheckBoxPreference weaTestAlert;

    public OP07CmasMainSettingsExt(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * Add Spanish Alert Option in Main Setting
     * @param prefActivity Current Preference Activity
     * @return void
     */
    @Override
    public void activateSpanishAlertOption(PreferenceFragment fragment) {
        Log.i(TAG, "Plugin activateSpanishAlertOption");
        PreferenceScreen prefSet = fragment.getPreferenceScreen();
        SharedPreferences pref =
            PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        boolean isSpanishAlertEnable = pref.getBoolean(KEY_SPANISH_ALERT, false);

        CheckBoxPreference spanishAlert = new CheckBoxPreference(fragment.getContext());
        spanishAlert.setKey(KEY_SPANISH_ALERT);
        spanishAlert.setChecked(isSpanishAlertEnable);
        spanishAlert.setTitle(mContext.getString(R.string.spanish_alert_title));
        spanishAlert.setSummary(mContext.getString(R.string.spanish_alert_title_summary));

        prefSet.addPreference(spanishAlert);
        Log.i(TAG, "Spanish alert checkbox added isSpanishAlertEnable = " +
            isSpanishAlertEnable);

        spanishAlert.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                Log.i(TAG, "Spanish Alert checkbox value = " + value);
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(KEY_SPANISH_ALERT, value);
                editor.commit();
                return true;
            }
        });
    }

    /**
     * Get Spanish Alert value from Main Setting
     * @param languageCode of Alert
     * @param msgId
     * @return boolean whether alert is Spanish or not
     */
    @Override
    public boolean isSpanishAlert(Context hostContext,
        String languageCode, int msgId, int subId) {
        Log.i(TAG, "Plugin checkSpanishAlert");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostContext);
        boolean spanishAlert = prefs.getBoolean(KEY_SPANISH_ALERT, false);
        Log.i(TAG, "Language code:" + languageCode + "  SpanishAlert option:" + spanishAlert);

        if (msgId == PRESIDENT_ALERT_ID) {
            return true;
        } else if (languageCode != null && languageCode.contains("es")) {
            return spanishAlert;
        } else {
            return true;
        }
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
        final int[] subIds = SubscriptionManager.getSubId(slotId);
        final SmsManager sManager;
        if (subIds != null && subIds.length > 0) {
            int subId = subIds[0];
            Log.d(TAG, "valid subIds subId = " + subId);
            sManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        } else {
            sManager = SmsManager.getDefault();
        }
        SharedPreferences prefs =
            PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        publicSaftyAlert = new CheckBoxPreference(fragment.getContext());
        publicSaftyAlert.setKey(ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS);
        boolean currentValue = prefs.getBoolean(slotKey +
            ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);
        publicSaftyAlert.setChecked(currentValue);
        publicSaftyAlert.setTitle(
            mContext.getString(R.string.enable_public_safety_messages_title));
        publicSaftyAlert.setSummary(
            mContext.getString(R.string.enable_public_safety_messages_summary));

        prefSet.addPreference(publicSaftyAlert);
        Log.i(TAG, "public Safty alert checkbox added isPublicSaftyEnable = " + currentValue);

        publicSaftyAlert.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                Log.i(TAG, "public Safty alert checkbox value = " + value);
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(slotKey + ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, value);
                editor.commit();

                setCellBroadcastRange(sManager, value,
                SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT,
                MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE);
                return true;
            }
        });
        boolean currentWeaTestValue = prefs.getBoolean(slotKey +
            ENABLE_WEA_TEST_ALERTS, false);
        weaTestAlert = new CheckBoxPreference(fragment.getContext());
        weaTestAlert.setKey(ENABLE_WEA_TEST_ALERTS);
        weaTestAlert.setChecked(currentWeaTestValue);
        weaTestAlert.setDefaultValue(currentWeaTestValue);
        weaTestAlert.setTitle(
            mContext.getString(R.string.enable_wea_test_messages_title));
        weaTestAlert.setSummary(
            mContext.getString(R.string.enable_wea_test_messages_summary));

        prefSet.addPreference(weaTestAlert);
        Log.i(TAG, "wea test alert checkbox added isWeaTestEnable = " + currentWeaTestValue);

        weaTestAlert.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                Log.i(TAG, "wea test alert checkbox value = " + value);
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(slotKey + ENABLE_WEA_TEST_ALERTS, value);
                editor.commit();
                // Enable/Disable wea test alert messages (4398-4399).
                setCellBroadcastRange(sManager, value,
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
                .getBoolean(slotKey + ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);
                Log.d(TAG, "in isMessageEnabledByUser,public safty setting " + resOfPub);
                return resOfPub;
            case MtkSmsCbCmasInfo.CMAS_CLASS_WEA_TEST:
                boolean resOfWea = PreferenceManager.getDefaultSharedPreferences(mContext)
                .getBoolean(slotKey + ENABLE_WEA_TEST_ALERTS, true);
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
        Log.i(TAG, "configOpChannel");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostContext);
        // Enable/Disable public safty alert messages (4396-4397).
        boolean isPublicSaftyEnable = prefs.getBoolean(
            slotKey + ENABLE_PUBLIC_SAFTY_MESSAGE_ALERTS, true);
        setCellBroadcastRange(manager, isPublicSaftyEnable,
        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT,
            MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE);

        // Enable/Disable wea test alert messages (4398-4399).
        boolean isWeaTestEnable = prefs.getBoolean(
            slotKey + ENABLE_WEA_TEST_ALERTS, false);
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
