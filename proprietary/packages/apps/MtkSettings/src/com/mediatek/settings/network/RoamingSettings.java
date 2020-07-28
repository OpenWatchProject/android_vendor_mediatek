package com.mediatek.settings.network;

import android.app.ActionBar;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;

import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Settings to show upate Roaming settings.
 */
public class RoamingSettings extends PreferenceActivity {

     //String keys for preference lookup
    public static final String TAG = "RoamingSettings";
    private static Context mContext;
    public static final String SUB_ID_EXTRA =
            "com.android.phone.settings.SubscriptionInfoHelper.SubscriptionId";
    public static final String DOMESTIC_VOICE_TEXT_ROAMING_SETTINGS =
            "domestic_voice_text_roaming_settings";
    public static final String DOMESTIC_DATA_ROAMING_SETTINGS = "domestic_data_roaming_settings";
    public static final String INTERNATIONAL_VOICE_TEXT_ROAMING_SETTINGS =
            "international_voice_text_roaming_settings";
    public static final String INTERNATIONAL_DATA_ROAMING_SETTINGS =
            "international_data_roaming_settings";
    public static final String TELEPHONY_SUBINFO_RECORD_UPDATED =
            "android.intent.action.ACTION_SUBINFO_RECORD_UPDATED";
    private static int sSubId;
    private IntentFilter mIntentFilter;
    private static MtkTelephonyManagerEx mTelephonyManagerEx;
    private static int mPhoneId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Intent intent = getIntent();
        sSubId = intent.getIntExtra(SUB_ID_EXTRA, SubscriptionManager.getDefaultSubscriptionId());
        mPhoneId = SubscriptionManager.getSlotIndex(sSubId);
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsFragment())
                .commit();
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in
            // onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        mContext = getApplicationContext();
        initIntentFilter();
        registerReceiver(mReceiver, mIntentFilter);
    }

    /**
     * SettingsFragment class to show settings for Roaming.
     */
    public static class SettingsFragment extends PreferenceFragment {
        private SwitchPreference mDomesticButtonVoiceRoam;
        private SwitchPreference mDomesticButtonDataRoam;
        private SwitchPreference mInternationalButtonVoiceRoam;
        private SwitchPreference mInternationalButtonDataRoam;
        private Integer mRoamingSettings[] = new Integer[6];


        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.mtk_roaming_preference);
            PreferenceScreen prefSet = getPreferenceScreen();
            mDomesticButtonVoiceRoam = (SwitchPreference) prefSet.findPreference(
                    DOMESTIC_VOICE_TEXT_ROAMING_SETTINGS);
            mDomesticButtonVoiceRoam.setOnPreferenceChangeListener(mRoamingSettingsListener);
            mDomesticButtonDataRoam = (SwitchPreference) prefSet.findPreference(
                    DOMESTIC_DATA_ROAMING_SETTINGS);
            mDomesticButtonDataRoam.setOnPreferenceChangeListener(mRoamingSettingsListener);
            mInternationalButtonVoiceRoam =
                    (SwitchPreference) prefSet.findPreference(
                    INTERNATIONAL_VOICE_TEXT_ROAMING_SETTINGS);
            mInternationalButtonVoiceRoam.setOnPreferenceChangeListener(mRoamingSettingsListener);
            mInternationalButtonDataRoam =
                    (SwitchPreference) prefSet.findPreference(
                    INTERNATIONAL_DATA_ROAMING_SETTINGS);
            mInternationalButtonDataRoam.setOnPreferenceChangeListener(mRoamingSettingsListener);
            this.initRoamingSettings(sSubId);
        }

        OnPreferenceChangeListener mRoamingSettingsListener =
            new Preference.OnPreferenceChangeListener() {
                @Override
                public boolean onPreferenceChange(Preference preference, Object newValue) {

                    if (((SwitchPreference) preference).isChecked() == (Boolean) newValue) {
                    // State got changed
                        Log.i(TAG, preference.getKey() + " : " + String.valueOf(newValue) +
                                " not changed");
                        return false;
                    // return false if you don't want to save the preference change.
                    }
                    String prefKey = preference.getKey();
                    int val = ((Boolean) newValue) ? 1 : 0;
                    boolean isInternationalDataRoamUpdateAgain = false;
                    boolean isDomesticDataRoamUpdateAgain = false;
                    final int phoneId = SubscriptionManager.getPhoneId(sSubId);
                    mRoamingSettings[0] = phoneId;
                    switch (prefKey) {
                        case DOMESTIC_VOICE_TEXT_ROAMING_SETTINGS:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue +
                                    " value: " + val);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + sSubId,
                                    val);
                            int prefVal = ((Boolean) mDomesticButtonDataRoam.isChecked()) ?
                                    1 : 0;
                            if (val == 1) {
                                mDomesticButtonDataRoam.setEnabled(true);

                                Log.d(TAG, "pref: " + "DOMESTIC_DATA_ROAMING_SETTINGS " + newValue +
                                        " value: " + prefVal);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + sSubId,
                                        prefVal);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + sSubId,
                                        prefVal);
                            } else {
                                if (prefVal == 1) {
                                    isDomesticDataRoamUpdateAgain = true;
                                }
                                mDomesticButtonDataRoam.setEnabled(false);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + sSubId, 0);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + sSubId,
                                        0);
                            }

                            break;
                        case DOMESTIC_DATA_ROAMING_SETTINGS:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + sSubId, val);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + sSubId, val);
                            RoamingSettingsReceiver.notifyRoamingSettingsChanged(prefKey);
                            break;
                        case INTERNATIONAL_VOICE_TEXT_ROAMING_SETTINGS:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + sSubId,
                                    val);
                            int interPrefVal =
                                    ((Boolean) mInternationalButtonDataRoam.isChecked()) ? 1 : 0;
                            if (val == 1) {
                                mInternationalButtonDataRoam.setEnabled(true);
                                Log.d(TAG, "pref: " + "INTERNATIONAL_DATA_ROAMING " + newValue +
                                        " value: " + interPrefVal);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + sSubId,
                                        interPrefVal);
                            } else {
                                if (interPrefVal == 1) {
                                    isInternationalDataRoamUpdateAgain = true;
                                }
                                mInternationalButtonDataRoam.setEnabled(false);
                                android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + sSubId,
                                        0);
                            }
                            break;
                        case INTERNATIONAL_DATA_ROAMING_SETTINGS:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                        mContext.getContentResolver(),
                                        MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + sSubId,
                                        val);
                            RoamingSettingsReceiver.notifyRoamingSettingsChanged(prefKey);
                            break;
                        default:
                            return false;
                    }
                    // international_voice_text_roaming (0,1)
                    mRoamingSettings[1] = android.provider.Settings.Global.getInt(
                            mContext.getContentResolver(),
                            MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + sSubId, 1);
                    //international_data_roaming (0,1)
                    mRoamingSettings[2] = android.provider.Settings.Global.getInt(
                            mContext.getContentResolver(),
                            MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + sSubId, 1);
                    //domestic_voice_text_roaming (0,1)
                    mRoamingSettings[3] = android.provider.Settings.Global.getInt(
                            mContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + sSubId, 1);
                    //domestic_data_roaming (0,1)
                    mRoamingSettings[4] = android.provider.Settings.Global.getInt(
                            mContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + sSubId, 0);
                    //domestic_LTE_data_roaming (1)
                    mRoamingSettings[5] = android.provider.Settings.Global.getInt(
                            mContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + sSubId, 0);
                    final int responseArray[] = new int[6];
                    for (int i = 0; i < mRoamingSettings.length; i++) {
                        responseArray[i] = Integer.valueOf(mRoamingSettings[i]);
                        Log.d(TAG, "Send to MD: RoamingSettings[" + i + "]: " + responseArray[i]);
                     }
                    new AsyncTask<Void, Void, Boolean>() {
                        @Override
                        protected Boolean doInBackground(Void... voids) {
                            Log.d(TAG, "doInBackground  phoneId = " + phoneId
                                    + " responseArray = " + responseArray.toString());
                            boolean isSucess = mTelephonyManagerEx
                                    .setRoamingEnable(phoneId, responseArray);
                            return isSucess;
                        }

                        @Override
                        protected void onPostExecute(Boolean result) {
                            Log.d(TAG, "onPostExecute handleSetRoamingSettings onPostExecute = " + result);
                        }
                    }.execute();
                    if (isInternationalDataRoamUpdateAgain) {
                        android.provider.Settings.Global.putInt(
                                mContext.getContentResolver(),
                                MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + sSubId, 1);
                    }
                    if (isDomesticDataRoamUpdateAgain) {
                        android.provider.Settings.Global.putInt(
                                mContext.getContentResolver(),
                                MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + sSubId, 1);
                        android.provider.Settings.Global.putInt(
                                mContext.getContentResolver(),
                                MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + sSubId, 1);
                    }
                    return true;
                }
        };

        private void initRoamingSettings(int subId) {
            Log.d(TAG, "update for subId " + subId + "and update roaming value");
            /**
             * response[0] : phone id
             * response[1] : international_voice_text_roaming (0,1)
             * response[2] : international_data_roaming (0,1)
             * response[3] : domestic_voice_text_roaming (0,1)
             * response[4] : domestic_data_roaming (0,1)
             * response[5] : domestic_LTE_data_roaming (1) no need to care*/
            try {
                // international_voice_text_roaming (0,1)
                int internationalVoiceRoamingSettings = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + subId, 1);
                //international_data_roaming (0,1)
                int internationalDataRoamingSettings = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + subId, 1);
                //domestic_voice_text_roaming (0,1)
                int domesticVoiceRoamingSettings = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + subId, 1);
                //domestic_data_roaming (0,1)
                int domesticDataRoamingSettings = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + subId, 0);
                //domestic_LTE_data_roaming (1)
                mRoamingSettings[5] = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + subId, 0);
                boolean isInternationalVoiceChecked = (internationalVoiceRoamingSettings == 1);
                boolean isInternationalDataChecked = (internationalDataRoamingSettings == 1);
                boolean isDomesticVoiceChecked = (domesticVoiceRoamingSettings == 1);
                boolean isDomesticDataChecked = (domesticDataRoamingSettings == 1);
                mDomesticButtonVoiceRoam.setChecked(isDomesticVoiceChecked);
                mDomesticButtonDataRoam.setChecked(isDomesticDataChecked);
                mInternationalButtonVoiceRoam.setChecked(isInternationalVoiceChecked);
                mInternationalButtonDataRoam.setChecked(isInternationalDataChecked);
                if (!isDomesticVoiceChecked) {
                    mDomesticButtonDataRoam.setEnabled(false);
                }
                if (!isInternationalVoiceChecked) {
                    mInternationalButtonDataRoam.setEnabled(false);
                }
            } catch (Exception e) {
                Log.e(TAG, "Caught exception");
            }

        }
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.roaming_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.roaming_alert_settings:
                Intent showGuardIntent = new Intent(this, GuardSettings.class);
                showGuardIntent.putExtra(SUB_ID_EXTRA, sSubId);
                startActivity(showGuardIntent);
                return true;
            case android.R.id.home:
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        Log.d(TAG, "isValidFragment, fragment=" + fragmentName);
        return super.isValidFragment(fragmentName)
                || SettingsFragment.class.getName().equals(fragmentName);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "action: " + action);
            /// When receive aiplane mode, we would like to finish the activity, for
            //  we can't get the modem capability, and will show the user selected network
            //  mode as summary, this will make user misunderstand.
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simStatus = intent.getStringExtra(
                        IccCardConstants.INTENT_KEY_ICC_STATE);
                Log.d(TAG, "[CDMA]simStatus: " + simStatus);
                if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simStatus)) {
                    finish();
                }
            } else if (action.equals(TELEPHONY_SUBINFO_RECORD_UPDATED)) {
                int detectedType = intent.getIntExtra(
                    MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS,
                    MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE);
                if (detectedType != MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE) {
                    finish();
                }
            }
            /// @}
        }
    };

    private void initIntentFilter() {
        mIntentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(TELEPHONY_SUBINFO_RECORD_UPDATED);
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        /// @}
    }
}
