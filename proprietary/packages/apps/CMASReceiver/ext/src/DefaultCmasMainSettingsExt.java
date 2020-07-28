
package com.mediatek.cmas.ext;

import android.content.Context;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.telephony.SmsManager;

public class DefaultCmasMainSettingsExt implements ICmasMainSettingsExt {

    private static final String TAG = "[CMAS]DefaultCmasMainSettingsExt";

    public DefaultCmasMainSettingsExt(Context context){
        //super(context);
    }

    public float getAlertVolume(int msgId) {
        Log.d(TAG, "Default getAlertVolume");
        return 1.0f;
    }

    public boolean getAlertVibration(int msgId) {
        Log.d(TAG, "Default getAlertVibration");
        return true;
    }

    public boolean setAlertVolumeVibrate(int msgId, boolean currentValue) {
        Log.d(TAG, "Default setAlertVolumeVibrate");
        return currentValue;
    }

    public boolean needToaddAlertSoundVolumeAndVibration() {
        Log.d(TAG, "Default needToaddAlertSoundVolumeAndVibration");
        return false;
    }

    public void updateVolumeValue(float volume) {
        Log.d(TAG, "Default updateVolumeValue");
    }

    public void updateVibrateValue(boolean value) {
        Log.d(TAG, "Default updateVibrateValue");
    }

    public void activateSpanishAlertOption(PreferenceFragment fragment) {
        Log.d(TAG, "Default activateSpanishAlertOption");
    }

    public boolean isSpanishAlert(Context hostContext,
        String languageCode, int msgId, int subId) {
        Log.d(TAG, "Default checkSpanishAlert");
        return true;
    }

    public boolean needBlockMessageInEcbm() {
        Log.d(TAG, "Default isEmergencyCallbackMode");
        return false;
    }

    public void configOpChannel(Context hostContext, SmsManager manager, String slotKey) {
        Log.d(TAG, "Default configOpChannel");
    }

    public void addNewChannelAlertsSetting(PreferenceFragment fragment,
            String slotKey, int slotId) {
        Log.d(TAG, "Default addNewChannelAlertsSetting");
    }

    public boolean isMessageEnabledByUser(int messageClass, boolean defaultValue, String slotKey) {
        return defaultValue;
    }
}
