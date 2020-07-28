package com.mediatek.phone.ext;

import android.content.Context;
import android.os.AsyncResult;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;


import android.util.Log;

public class DefaultCallFeaturesSettingExt implements ICallFeaturesSettingExt {

    public static final int RESUME = 0;
    public static final int PAUSE = 1;
    public static final int DESTROY = 2;

    /**
     * called when init the preference (onCreate) single card.
     * plugin can customize the activity, like add/remove preference screen.
     * plugin should check the activiyt class name, to distinct the caller, avoid do wrong work.
     * if (TextUtils.equals(getClass().getSimpleName(), "CallFeaturesSetting") {}
     *
     * @param activity the PreferenceActivity instance
     * @return
     */
    @Override
    public void initOtherCallFeaturesSetting(PreferenceActivity activity) {
    }

    /**
     * called when init the preference screen for multiple card.
     * plugin can customize the activity, like add/remove preference screen.
     *
     * @param activity the PreferenceActivity instance
     * @param phone the Phone instance
     */
    @Override
    public void initOtherCallFeaturesSetting(PreferenceActivity activity, Object phone) {
    }

    /**
     * called when init the preference (onCreate).
     * plugin can customize the fragment, like add/remove preference screen
     * plugin should check the fragment class name, to distinct the caller, avoid do wrong work.
     * if (TextUtils.equals(getClass().getSimpleName(), "CallFeaturesSetting") {}
     *
     * @param fragment the PreferenceFragment instance
     */
    @Override
    public void initOtherCallFeaturesSetting(PreferenceFragment fragment) {
    }

    /**
     * Init the call forward option item for C2K.
     * @param activity the activity of the setting preference.
     * @param subId the subId of the setting item.
     */
    @Override
    public void initCdmaCallForwardOptionsActivity(PreferenceActivity activity, int subId) {
    }

    /**
     * Need to fire intent to reset IMS PDN connection.
     * @param context the context of the setting preference.
     * @param msg the message to be sent when SS completed.
     * @return
     */
    @Override
    public void resetImsPdnOverSSComplete(Context context, int msg) {
        Log.d("DefaultCallFeaturesSettingExt", "resetImsPdnOverSSComplete");
    }

    /**
     * For WWWOP, Whether need to show open mobile data dialog or not.
     * @param context the context of the setting preference.
     * @param subId the sudId of the setting item.
     * @return true if need to show it.
     */
    @Override
    public boolean needShowOpenMobileDataDialog(Context context, int subId) {
        return true;
    }

    /**
     * handle preference status when error happens.
     * @param preference the preference which error happens on.
     */
    @Override
    public void onError(Preference preference) {
        Log.d("DefaultCallFeaturesSettingExt", "default onError");
    }
    /**
     * handle error dialog for different errors from framework.
     * @param context context
     * @param ar ar
     * @param preference preference
     */
    @Override
    public boolean handleErrorDialog(Context context, AsyncResult ar, Preference preference) {
        Log.d("DefaultCallFeaturesSettingExt", "default handleErrorDialog");
        return false;
    }

    @Override
    /** Initializes  various parameters required.
     * Used in CallFeatureSettings
     * @param pa PreferenceActivity
     * @param wfcPreference wfc preference
     * @return
     */
    public void initPlugin(PreferenceActivity pa, Preference wfcPreference) {
    }

    @Override
    /** Called on events like onResume/onPause etc from WfcSettings.
     * @param event resume/puase etc.
     * @return
     */
    public void onCallFeatureSettingsEvent(int event) {
    }

    /** get operator specific customized summary for WFC button.
     * Used in CallFeatureSettings
     * @param context context
     * @param defaultSummaryResId default summary res id
     * @return summary string to be displayed
     */
    @Override
    public String getWfcSummary(Context context, int defaultSummaryResId) {
        return context.getResources().getString(defaultSummaryResId);
    }

    /** Get video change preference.
     * @param boolean value
     */
    @Override
    public void videoPreferenceChange(boolean value) {
       Log.d("DefaultCallFeaturesSettingExt", "videoPreferenceChange");
    }

    /**
     * Disable CallForward preference (when unreachable) if smartcallforward is active.
     * @param context context
     * @param phone phone
     * @param preference preference
     * @param reason reason
     */
    @Override
    public void disableCallFwdPref(Context context, Object phone, Preference pref, int reason) {
        Log.d("DefaultCallFeaturesSettingExt", "default disableCallFwdPref");
    }

    /**
     * Customize GSM Additional Settings
     * @param activity PreferenceActivity
     * @param phone Phone
     */
    @Override
    public void customizeAdditionalSettings(PreferenceActivity activity, Object phone) {
        Log.d("DefaultCallFeaturesSettingExt", "default customizeAdditionalSettings");
    }

    /**
     * Escape CLIR settings init
     * @return true/false
     */
    @Override
    public boolean escapeCLIRInit() {
        Log.d("DefaultCallFeaturesSettingExt", "default escapeCLIRInit");
        return false;
    }
}
