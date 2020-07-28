
package com.mediatek.cmas.ext;

import android.content.Context;
import android.preference.PreferenceFragment;
import android.telephony.SmsManager;

public interface ICmasMainSettingsExt {

    /**
     * Get Alert Volume value.
     * @internal
     */
    public float getAlertVolume(int msgId);

    /**
     * Get Alert Vibration value.
     * @internal
     */
    public boolean getAlertVibration(int msgId);

    /**
     * Set Alert Volume and Vibration value.
     * @internal
     */
    public boolean setAlertVolumeVibrate(int msgId, boolean currentValue);

    /**
     * Update the volume value.
     * @param volume value between 0 and 1 for alert volume
     */
    public void updateVolumeValue(float volume);

    /**
     * Update the volume value.
     * @param value bool vaue for vibrate option
     */
    public void updateVibrateValue(boolean value);

    /**
     * Add Alert Volume and Vibration in Main Setting.
     * @return true if need to show vibration and volume update option
     * @internal
     */
    public boolean needToaddAlertSoundVolumeAndVibration();

    /**
     * Add Spanish Alert Option in Main Setting.
     * @param prefActivity Current Preference Activity
     * @return void
     */
    public void activateSpanishAlertOption(PreferenceFragment fragment);

    /**
     * Get Spanish Alert value from Main Setting
     * @param languageCode of Alert
     * @param msgId CMAS Channel ID
     * @return boolean whether alert is Spanish or not
     */
    public boolean isSpanishAlert(Context hostContext,
        String languageCode, int msgId, int subId);

    /**
     * Get Emergency Call back Mode value from Main Setting
     * @return boolean whether is Emergency Callback Mode
     */
    public boolean needBlockMessageInEcbm();

    /**
     * config ATT and TMO operator only channel
     * @param hostContext is host Context
     * @param manager is smsManager instance
     * @param slotKey to judge slot index
     */
    public void configOpChannel(Context hostContext, SmsManager manager, String slotKey);

    /**
     * add new channel alert setting for ATT and TMO
     * @param fragment of settings
     * @param slotKey to judge slot index
     * @param slotId is slot index
     */
    public void addNewChannelAlertsSetting(PreferenceFragment fragment, String slotKey, int slotId);

    /**
     * check if message enabled for ATT and TMO
     * @param messageClass is message Class
     * @param defaultValue is alert or not
     * @param slotKey to judge slot index
     * @return boolean whether enabled by user
     */
    public boolean isMessageEnabledByUser(int messageClass, boolean defaultValue, String slotKey);

}
