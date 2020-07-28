package com.mediatek.op18.phone;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;


/**
 * Class to support operator customizations for WFC settings.
 */
public class WfcSwitchController {

    private static final String TAG = "OP18WfcSwitchController";
    private static final String AOSP_SETTING_WFC_PREFERENCE = "wifi_calling_key";
    private static final String AOSP_CALL_SETTING_WFC_PREFERENCE
            = "button_wifi_calling_settings_key";
    private static final String OP18_WFC_PREFERENCE_KEY = "op18_wfc_pref_switch";
    private static final String CATEGORY_CALLING_KEY = "calling";
    private static WfcSwitchController sController = null;

    private Context mPluginContext;
    private Context mAppContext;
    private int mPhoneId = 0;
    private SwitchPreference mWfcSwitchSettings = null;
    private SwitchPreference mWfcSwitchCallSettings = null;
    private PreferenceCategory mCallingCategory = null;
    private PreferenceScreen mPreferenceScreenSettings = null;
    private PreferenceScreen mPreferenceScreenCallSettings = null;

    private WfcSwitchController(Context context) {
       mPluginContext = context;
    }

    /** Returns instance of OP18WfcSettings.
         * @param context context
         * @return OP18WfcSettings
         */
    public static WfcSwitchController getInstance(Context context) {

        if (sController == null) {
            sController = new WfcSwitchController(context);
        }
        return sController;
    }

    public void customizedWfcPreference(Context context, PreferenceScreen preferenceScreen,
                    int phoneId) {
        mAppContext = context;
        mPhoneId = phoneId;
        /*
        if (!TextUtils.equals(context.getClass().getSimpleName(), "CallFeaturesSetting")) {
            PreferenceCategory callingCategory = (PreferenceCategory) preferenceScreen.
                            findPreference(CATEGORY_CALLING_KEY);
            if(callingCategory == null){
                Log.d(TAG, "Calling Category null, so return");
            return;
        }
            mPreferenceScreenSettings = preferenceScreen;
            customizedWfcPreference(context, callingCategory, phoneId);
            return;
        }*/
        mPreferenceScreenCallSettings = preferenceScreen;
        Preference wfcPreferenceCallSettings = null;

        mWfcSwitchCallSettings = (SwitchPreference) preferenceScreen
                    .findPreference(OP18_WFC_PREFERENCE_KEY);
        Log.d(TAG, "Plugin mWfcSwitch in CallSettings: " + mWfcSwitchCallSettings);

        wfcPreferenceCallSettings =
                    (android.preference.Preference) preferenceScreen
                    .findPreference(AOSP_CALL_SETTING_WFC_PREFERENCE);
        Log.d(TAG, "AOSP wfcPreference in CallSettings: " + wfcPreferenceCallSettings);


        CharSequence title = null;
        int order = 0;
        if (wfcPreferenceCallSettings != null) {
            mPreferenceScreenCallSettings.removePreference(wfcPreferenceCallSettings);
            title = wfcPreferenceCallSettings.getTitle();
            order = wfcPreferenceCallSettings.getOrder();
            checkAndAddWfcSwitch(title, order);
        }

        int callState = Settings.Global.getInt(context.getContentResolver(),
                    OP18WfcSettings.CALL_STATE, OP18WfcSettings.CALL_STATE_IDLE);
        Log.d(TAG, "call_state: " + callState);
        updateWfcSwitchState(callState);
    }

    public void customizedWfcPreference(Context context, PreferenceScreen preferenceScreen,
                        PreferenceCategory callingCategory, int phoneId) {
        mAppContext = context;
        mPhoneId = phoneId;
        mPreferenceScreenSettings = preferenceScreen;
        mCallingCategory = callingCategory;
        Preference wfcPreferenceSettings = null;
        Log.d(TAG, "customizedWfcPreference with category");
        if (callingCategory != null) {
            //Adding preference in Network Settings
            mWfcSwitchSettings = (SwitchPreference) callingCategory
                            .findPreference(OP18_WFC_PREFERENCE_KEY);
            Log.d(TAG, "Plugin wfc switch: " + mWfcSwitchSettings);

            wfcPreferenceSettings =
                        (android.preference.Preference) callingCategory
                        .findPreference(AOSP_SETTING_WFC_PREFERENCE);
            Log.d(TAG, "AOSP wfcPreference: " + wfcPreferenceSettings);
        } else {
            //Adding preference in Call Settings
            mWfcSwitchSettings = (SwitchPreference) preferenceScreen
                    .findPreference(OP18_WFC_PREFERENCE_KEY);
            Log.d(TAG, "Plugin mWfcSwitch in CallSettings: " + mWfcSwitchSettings);

            wfcPreferenceSettings =
                    (android.preference.Preference) preferenceScreen
                    .findPreference(AOSP_CALL_SETTING_WFC_PREFERENCE);
            Log.d(TAG, "AOSP wfcPreference in CallSettings: " + wfcPreferenceSettings);
        }


        CharSequence title = null;
        int order = 0;
        if (wfcPreferenceSettings != null) {
            if (callingCategory != null) {
                callingCategory.removePreference(wfcPreferenceSettings);
            } else {
                mPreferenceScreenSettings.removePreference(wfcPreferenceSettings);
            }
            title = wfcPreferenceSettings.getTitle();
            order = wfcPreferenceSettings.getOrder();
            checkAndAddWfcSwitchForSettings(title, order);
        }

        int callState = Settings.Global.getInt(context.getContentResolver(),
                    OP18WfcSettings.CALL_STATE, OP18WfcSettings.CALL_STATE_IDLE);
        Log.d(TAG, "call_state: " + callState);
        updateWfcSwitchState(callState);
    }

    private void checkAndAddWfcSwitchForSettings(CharSequence title, int order) {
        Log.d(TAG, "checkAndAddWfcSwitchForSettings ");
        if (mWfcSwitchSettings == null) {
                mWfcSwitchSettings = new SwitchPreference(mAppContext);
                mWfcSwitchSettings
                        .setOnPreferenceChangeListener(new WfcSwitchListenerForSettings());
                mWfcSwitchSettings.setKey(OP18_WFC_PREFERENCE_KEY);
                mWfcSwitchSettings.setTitle(title);
                mWfcSwitchSettings.setOrder(order);
            if (mCallingCategory != null) {
                Log.d(TAG, "addPreference for non null category");
                mCallingCategory.addPreference(mWfcSwitchSettings);
            } else {
                Log.d(TAG, "addPreference for null call settings preference");
                mPreferenceScreenSettings.addPreference(mWfcSwitchSettings);
            }
            ImsManager.getInstance(mAppContext, mPhoneId).setWfcMode(
               ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
        }
    }

    private void checkAndAddWfcSwitch(CharSequence title, int order) {
        Log.d(TAG, "checkAndAddWfcSwitch: " + mWfcSwitchSettings);
        if (mWfcSwitchSettings != null) {
            mWfcSwitchSettings = null;
        }
        if (mWfcSwitchCallSettings == null) {
            if (mPreferenceScreenCallSettings != null) {
                mWfcSwitchCallSettings = new SwitchPreference(mAppContext);
                mWfcSwitchCallSettings
                        .setOnPreferenceChangeListener(new WfcSwitchListenerForCallSettings());
                mWfcSwitchCallSettings.setKey(OP18_WFC_PREFERENCE_KEY);
                mWfcSwitchCallSettings.setTitle(title);
                mWfcSwitchCallSettings.setOrder(order);
                mPreferenceScreenCallSettings.addPreference(mWfcSwitchCallSettings);
            }
            ImsManager.getInstance(mAppContext, mPhoneId).setWfcMode(
               ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
        }
    }

    public void updateWfcSwitchState(int callState) {
        // Disable switch if PS call ongoing
        if (mWfcSwitchCallSettings != null) {
            mWfcSwitchCallSettings.setEnabled(callState == OP18WfcSettings.CALL_STATE_PS ?
                    false : true);
            mWfcSwitchCallSettings.setChecked(ImsManager.getInstance(mAppContext,
                    mPhoneId).isWfcEnabledByUser());
            mWfcSwitchCallSettings.setSummary(ImsManager.getInstance(mAppContext,
                    mPhoneId).isWfcEnabledByUser() == true ?
                    mPluginContext.getResources().getString(R.string.enabled)
                    : mPluginContext.getResources().getString(R.string.disabled));
        } else if (mWfcSwitchSettings != null) {
            mWfcSwitchSettings.setEnabled(callState == OP18WfcSettings.CALL_STATE_PS ?
                    false : true);
            mWfcSwitchSettings.setChecked(ImsManager.getInstance(mPluginContext,
                    mPhoneId).isWfcEnabledByUser());
            mWfcSwitchSettings.setSummary(ImsManager.getInstance(mAppContext,
                    mPhoneId).isWfcEnabledByUser() == true ?
                    mPluginContext.getResources().getString(R.string.enabled)
                    : mPluginContext.getResources().getString(R.string.disabled));
        }
    }

    private boolean isInSwitchProcess() {
        int imsState = MtkPhoneConstants.IMS_STATE_DISABLED;
        try {
         imsState = MtkImsManagerEx.getInstance().getImsState(mPhoneId);
        } catch (ImsException e) {
           return false;
        }
        Log.d(TAG, "isInSwitchProcess , imsState = " + imsState);
        return imsState == MtkPhoneConstants.IMS_STATE_DISABLING
                || imsState == MtkPhoneConstants.IMS_STATE_ENABLING;
    }

    /** Need for the case when rjil specific wfc pref is to be added again after being removed.
     * For rest of wfc setting customization use customizedWfcPreference()
     * @return
     */
    public void addWfcPreference() {
        Log.d(TAG, "addWfcPreference,mPreferenceScreenCallSettings"
                + mPreferenceScreenCallSettings);
        Log.d(TAG, "addWfcPreference,mCallingCategory:" + mCallingCategory);
        if (mPreferenceScreenCallSettings != null) {
            android.preference.Preference aospWfcSettingsCallPreference =
                    (android.preference.Preference) mPreferenceScreenCallSettings
                    .findPreference(AOSP_SETTING_WFC_PREFERENCE);
            android.preference.SwitchPreference customizedWfcPreference =
                    (android.preference.SwitchPreference) mPreferenceScreenCallSettings
                    .findPreference(OP18_WFC_PREFERENCE_KEY);
            if (aospWfcSettingsCallPreference == null && customizedWfcPreference == null) {
                mPreferenceScreenCallSettings.addPreference(mWfcSwitchCallSettings);
                ImsManager.getInstance(mAppContext, mPhoneId).setWfcMode(
                    ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                mWfcSwitchCallSettings.setEnabled(Settings.Global
                        .getInt(mPluginContext.getContentResolver(),
                    OP18WfcSettings.CALL_STATE, OP18WfcSettings.CALL_STATE_IDLE) ==
                    OP18WfcSettings.CALL_STATE_PS ? false : true);
                mWfcSwitchCallSettings.setChecked(ImsManager.getInstance(mAppContext,
                    mPhoneId).isWfcEnabledByUser());
            }
        } else if (mCallingCategory != null) {
            Preference aospWfcSettingsPreference =
                    (Preference) mCallingCategory
                    .findPreference(AOSP_SETTING_WFC_PREFERENCE);
            SwitchPreference customizedWfcPreference =
                    (SwitchPreference) mCallingCategory
                    .findPreference(OP18_WFC_PREFERENCE_KEY);
            if (aospWfcSettingsPreference == null && customizedWfcPreference == null) {
                mCallingCategory.addPreference(mWfcSwitchSettings);
                ImsManager.getInstance(mAppContext, mPhoneId).setWfcMode(
                    ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                mWfcSwitchSettings.setEnabled(Settings.Global
                        .getInt(mPluginContext.getContentResolver(),
                    OP18WfcSettings.CALL_STATE, OP18WfcSettings.CALL_STATE_IDLE) ==
                    OP18WfcSettings.CALL_STATE_PS ? false : true);
                mWfcSwitchSettings.setChecked(ImsManager.getInstance(mAppContext,
                    mPhoneId).isWfcEnabledByUser());
            }
        }
    }

    /** Returns instance of OP18WfcSettings.
     * @return
     */
    public void removeWfcPreference() {
        if (mPreferenceScreenCallSettings != null && mWfcSwitchCallSettings != null) {
            mPreferenceScreenCallSettings.removePreference(mWfcSwitchCallSettings);
        }
        if (mCallingCategory != null && mWfcSwitchSettings != null) {
            mCallingCategory.removePreference(mWfcSwitchSettings);
        }
    }

    private class WfcSwitchListenerForSettings implements
            Preference.OnPreferenceChangeListener {

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            boolean isChecked = ((Boolean) newValue).booleanValue();
            if (isChecked) {
                mWfcSwitchSettings.setChecked(false);
                displayConfirmationDialog();
                return false;
            }
            mWfcSwitchSettings.setSummary(mPluginContext.getResources()
                                .getString(R.string.disabled));
            ImsManager.getInstance(mPluginContext, mPhoneId).setWfcSetting(false);
            return true;
        }
    }

    private class WfcSwitchListenerForCallSettings implements
            android.preference.Preference.OnPreferenceChangeListener {

        @Override
        public boolean onPreferenceChange(android.preference.Preference preference,
                Object newValue) {
            boolean isChecked = ((Boolean) newValue).booleanValue();
            if (isChecked) {
                mWfcSwitchCallSettings.setChecked(false);
                displayConfirmationDialog();
                return false;
            }
            mWfcSwitchCallSettings.setSummary(mPluginContext.getResources()
                                .getString(R.string.disabled));
            ImsManager.getInstance(mPluginContext, mPhoneId).setWfcSetting(false);
            return true;
        }
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Log.d(TAG, "onPreferenceTreeClick");
        if (preference == mWfcSwitchSettings) {
            Log.d(TAG, "Plugin WFC preference clicked");
            return true;
        } else {
            return false;
        }
    }

    private void displayConfirmationDialog() {
        new AlertDialog.Builder(mAppContext)
            .setCancelable(true)
            .setTitle(mPluginContext.getText(R.string.confirmation_dialog_title))
            .setMessage(mPluginContext.getText(R.string.confirmation_dialog_message))
            .setPositiveButton(mPluginContext.getText(R.string.enable), new OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    Log.d(TAG, "Enabling WFC switch");
                    if (isInSwitchProcess()) {
                       Toast.makeText(mPluginContext, R.string.Switch_not_in_use_string,
                                Toast.LENGTH_SHORT).show();
                       return;
                    }
                    if (mWfcSwitchCallSettings != null) {
                        mWfcSwitchCallSettings.setChecked(true);
                        mWfcSwitchCallSettings.setSummary(mPluginContext.getResources()
                                .getString(R.string.enabled));
                        Log.d(TAG, "set Call Setting WFC switch true");
                    } else if (mWfcSwitchSettings != null) {
                        mWfcSwitchSettings.setChecked(true);
                        mWfcSwitchSettings.setSummary(mPluginContext.getResources()
                                .getString(R.string.enabled));
                        Log.d(TAG, "set Setting WFC switch true");
                    }
                    ImsManager.getInstance(mPluginContext, mPhoneId).setWfcSetting(true);
                }
            })
            .setNegativeButton(mPluginContext.getText(R.string.cancel), new OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    dialog.dismiss();
                }
            })
            .show();
    }
}
