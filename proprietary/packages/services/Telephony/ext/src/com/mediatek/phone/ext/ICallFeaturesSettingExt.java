package com.mediatek.phone.ext;

import android.content.Context;
import android.os.AsyncResult;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;


public interface ICallFeaturesSettingExt {

    /**
     * called when init the preference (onCreate) single card
     * plugin can customize the activity, like add/remove preference screen
     * plugin should check the activiyt class name, to distinct the caller, avoid do wrong work.
     * if (TextUtils.equals(getClass().getSimpleName(), "CallFeaturesSetting") {}
     *
     * @param activity the PreferenceActivity instance
     * @return
     * @internal
     */
    void initOtherCallFeaturesSetting(PreferenceActivity activity);

    /**
     * called when init the preference screen for multiple card.
     * plugin can customize the activity, like add/remove preference screen.
     *
     * @param activity the PreferenceActivity instance
     * @param phone the Phone instance
     */
    void initOtherCallFeaturesSetting(PreferenceActivity activity, Object phone);

    /**
     * called when init the preference (onCreate)
     * plugin can customize the fragment, like add/remove preference screen
     * plugin should check the fragment class name, to distinct the caller, avoid do wrong work.
     * if (TextUtils.equals(getClass().getSimpleName(), "CallFeaturesSetting") {}
     *
     * @param fragment the PreferenceFragment instance
     * @internal
     */
    void initOtherCallFeaturesSetting(PreferenceFragment fragment);

    /**
     * Init the call forward option item for C2K.
     * @param activity the activity of the setting preference.
     * @param subId the subId of the setting item.
     * @internal
     */
    void initCdmaCallForwardOptionsActivity(PreferenceActivity activity, int subId);

    /**
     * Need to fire intent to reset IMS PDN connection.
     * @param context the context of the setting preference.
     * @param msg the message to be sent when SS completed.
     * @return
     * @internal
     */
    void resetImsPdnOverSSComplete(Context context, int msg);

    /**
     * For WWWOP, Whether need to show open mobile data dialog or not.
     * @param context the context of the setting preference.
     * @param subId the sudId of the setting item.
     * @return true if need to show it.
     * @internal
     */
    boolean needShowOpenMobileDataDialog(Context context, int subId);

    /**
     * handle preference status when error happens
     * @param preference the preference which error happens on.
     * @internal
     */
    public void onError(Preference preference);

    /** Initializes various parameters required.
     * Used in  CallFeatureSettings
     * @param pa PreferenceActivity
     * @param wfcPreference wfc preference
     * @return
     * @internal
     */
    void initPlugin(PreferenceActivity pa, Preference wfcPreference);

    /** Called on events like onResume/onPause etc from WfcSettings.
     * @param event resume/puase etc.
     * @return
     * @internal
     */
    void onCallFeatureSettingsEvent(int event);

    /** get operator specific customized summary for WFC button.
     * Used in CallFeatureSettings
     * @param context context
     * @param defaultSummaryResId default summary res id
     * @return res id of summary to be displayed
     * @internal
     */
    String getWfcSummary(Context context, int defaultSummaryResId);

     /**
     * handle error dialog for different errors from framework
     * @param context
     * @param ar
     * @param preference
     */
     boolean handleErrorDialog(Context context, AsyncResult ar, Preference preference);

    /** Get video change preference
     * @param boolean value
     */
    public void videoPreferenceChange(boolean value);

    /**
     * Disable CallForward preference (when unreachable) if smartcallforward is active.
     * @param context
     * @param phone
     * @param preference
     * @param reason
     */
    public void disableCallFwdPref(Context context, Object phone, Preference pref, int reason);

    /**
     * Customize GSM Additional Settings
     * @param activity PreferenceActivity
     * @param phone Phone
     */
    void customizeAdditionalSettings(PreferenceActivity activity, Object phone);

    /**
     * Escape CLIR settings init
     * @return true/false
     */
    boolean escapeCLIRInit();
}
