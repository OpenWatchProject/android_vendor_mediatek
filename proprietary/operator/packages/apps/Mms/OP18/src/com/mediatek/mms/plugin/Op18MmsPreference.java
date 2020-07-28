package com.mediatek.op18.mms;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioAttributes;
import android.media.RingtoneManager;
import android.net.Uri;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.RingtonePreference;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.op18.mms.R;
import com.mediatek.setting.SimStateMonitor ;


import java.util.List;

/**
 *  Op18MmsPreference.
 *
 */
public class  Op18MmsPreference extends ContextWrapper {

    private static int msimcount;
    private static Op18MmsPreference sRef;
    private static boolean isSms = true;
    private static final int REQUEST_CODE = 17;
    public static final long    EXPIRY_ONE_HOUR          = 1 * 60 * 60;
    public static final long    EXPIRY_SIX_HOURS         = 6 * 60 * 60;
    public static final long    EXPIRY_TWELVE_HOURS      = 12 * 60 * 60;
    public static final long    EXPIRY_ONE_DAY           = 1  * 24 * 60 * 60;
    public static final long    EXPIRY_MAX_DURATION      = 7 * 24 * 60 * 60;
    /**
     * Construction.
     * @param base Context
     */

    private Op18MmsPreference mMmsPreferenceExt = null;
    private Context mContext;
    private Activity mHost;
    private Activity parentRingActivity;
    public  Op18MmsPreference(Context base) {
        super(base);
        mContext = base;
    }

    public static final String TAG = "Mms/Op18MmsPreferenceExt";
    public static final String PREFERENCE_NAME = "preference";

    public static final String PREFERENCE_NAME_SMS_VALIDITY_PERIOD = "sms_validity_period";
    public static final String PREFERENCE_NAME_MMS_VALIDITY_PERIOD = "mms_validity_period";

    public static final String SMS_VALIDITY_PERIOD_PREFERENCE_KEY = "pref_key_sms_validity_period";
    public static final String MMS_VALIDITY_PERIOD_PREFERENCE_KEY = "pref_key_mms_validity_period";
    public static final String SMS_QUICK_TEXT_EDITOR = "pref_key_quick_text_editor";

    public static Op18MmsPreference getInstance(Context base)
    {
         if (sRef == null) {
            sRef = new Op18MmsPreference(base);
         }
         return sRef;
    }

    public void onSimStateChanged() {
        Log.d(TAG, "onSimStateChanged + " + parentRingActivity);
        if (parentRingActivity != null) {
            parentRingActivity.finishActivity(REQUEST_CODE);
        }
    }

    /**
     * configSmsPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     * @param simCount int
     */
    public void configSmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
        Log.d(TAG, "configSmsPreference");
        isSms = true;
        Preference mSmsQuickTextEditorPref = pC.findPreference(SMS_QUICK_TEXT_EDITOR);
        if (mSmsQuickTextEditorPref != null) {
            Log.d(TAG, "configSmsPreference set template string");
            mSmsQuickTextEditorPref.setTitle(getString(R.string.sms_message_template));
            mSmsQuickTextEditorPref.setSummary(getString(R.string.sms_message_template));
        }

        addValidityPeriodPreference(hostActivity, pC, simCount);
    }

    /**
        * configMmsPreference.
        * @param hostActivity Activity
        * @param pC PreferenceCategory
        * @param simCount int
        */
    public void configMmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
          Log.d(TAG, "configMmsPreference");
          isSms = false;
          addValidityPeriodPreference(hostActivity, pC, simCount);
      }

    /**
     * configSmsPreferenceEditorWhenRestore.
     * @param hostActivity Activity
     * @param editor SharedPreferences.Editor
     */

    public void configSmsPreferenceEditorWhenRestore(Activity hostActivity,
                                                        SharedPreferences.Editor editor) {
        Log.d(TAG, "configSmsPreferenceEditorWhenRestore");
        List<SubscriptionInfo> simList = SubscriptionManager.from(hostActivity)
                                                .getActiveSubscriptionInfoList();
        if (simList != null) {
            int simCount = simList.size();
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                String key = Integer.toString(subId) + "_" + SMS_VALIDITY_PERIOD_PREFERENCE_KEY;
                editor.remove(key);
            }
        }
    }

    /**
     * configMmsPreferenceEditorWhenRestore.
     * @param hostActivity Activity
     * @param editor SharedPreferences
     */
    public void configMmsPreferenceEditorWhenRestore(Activity hostActivity,
                                                        SharedPreferences.Editor editor) {
        Log.d(TAG, "configMmsPreferenceEditorWhenRestore");
        List<SubscriptionInfo> simList = SubscriptionManager.from(hostActivity)
                                                .getActiveSubscriptionInfoList();
        if (simList != null) {
            int simCount = simList.size();
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                String key = Integer.toString(subId) + "_" + MMS_VALIDITY_PERIOD_PREFERENCE_KEY;
                editor.remove(key);
            }
        }
    }


    // add sms validity period feature
    private void addValidityPeriodPreference(final Activity hostActivity, PreferenceCategory pC,
            int simCount) {
        Log.d(TAG, "addValidityPeriodPreference: simCount = " + simCount + " isSms = " + isSms);
        final List<SubscriptionInfo> simList = SubscriptionManager.from(hostActivity)
                                    .getActiveSubscriptionInfoList();
        Preference p = new Preference(hostActivity);
        if (isSms) {
            p.setTitle(getString(R.string.sms_validity_period));
            p.setSummary(getString(R.string.sms_validity_period));
        } else {
            p.setTitle(getString(R.string.mms_validity_period));
            p.setSummary(getString(R.string.mms_validity_period));
        }
        int smsValidityOrder = 4;
        p.setOrder(smsValidityOrder);
        int count = pC.getPreferenceCount();
        //reset order
        for (int index = 0; index < count; index ++) {
            Preference preference = pC.getPreference(index);
            if (preference != null) {
                int pOrder = preference.getOrder();
                if (pOrder >= smsValidityOrder) {
                    preference.setOrder(pOrder + 1);
                }
            }
        }
        if (simCount > 1) {
            p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    Log.d(TAG, "addSmsValidityPeriodPreference:" +
                                "onPreferenceClick, preference = " + preference);
                    Intent intent = new Intent();
                    intent.setClassName(hostActivity, "com.mediatek.setting.SubSelectActivity");
                    if (isSms) {
                        intent.putExtra("PREFERENCE_KEY", SMS_VALIDITY_PERIOD_PREFERENCE_KEY);
                        intent.putExtra(PREFERENCE_NAME, PREFERENCE_NAME_SMS_VALIDITY_PERIOD);
                    } else {
                        intent.putExtra("PREFERENCE_KEY", MMS_VALIDITY_PERIOD_PREFERENCE_KEY);
                        intent.putExtra(PREFERENCE_NAME, PREFERENCE_NAME_MMS_VALIDITY_PERIOD);
                    }
                    hostActivity.startActivity(intent);
                    return true;
                }
            });
        } else {
            if (simCount == 0) {
                p.setEnabled(false);
            } else {
                p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        Log.d(TAG, "addSmsValidityPeriodPreference: " +
                                    "onPreferenceClick, preference = " + preference);
                        int subId = simList.get(0).getSubscriptionId();
                        showValidityPeriodDialog(hostActivity, subId, null);
                        return true;
                    }
                });
            }
        }
        pC.addPreference(p);
    }

    /**
     * configSelectCardPreferenceTitle.
     * @param hostActivity Activity
     */
    public void configSelectCardPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null) {
            if (preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
                hostActivity.setTitle(getString(R.string.sms_validity_period));
            } else if (preferenceName.equals(PREFERENCE_NAME_MMS_VALIDITY_PERIOD)) {
                hostActivity.setTitle(getString(R.string.mms_validity_period));
            }
        }
    }

    /**
     * configSelectCardPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     * @param intent Intent

    public void configSelectCardPreference(Activity hostActivity, PreferenceCategory pC,
                                        Intent intent) {
        if (intent == null) {
            throw new RuntimeException("configSelectCardPreference: intent cannot be null");
        }
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null && preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            hostActivity.setTitle(getString(R.string.sms_validity_period));
        }else if (preferenceName != null &&
                            preferenceName.equals(MMS_VALIDITY_PERIOD_PREFERENCE_KEY)) {
            hostActivity.setTitle(
                        getString(R.string.mms_validity_period));
        }
    }
*/
    /**
     * handleSelectCardPreferenceTreeClick.
     * @param hostActivity Activity
     * @param subId int
     * @return boolean
     */
    public boolean handleSelectCardPreferenceTreeClick(Activity hostActivity, final int subId) {

        Log.d(TAG, "handleSelectCardPreferenceTreeClick isSms = " + isSms + "subId :" + subId);
        Intent intent = hostActivity.getIntent();
        String intentname = intent.getStringExtra(PREFERENCE_NAME);
        if (intentname != null) {
            if (intentname.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD) ||
                        intentname.equals(PREFERENCE_NAME_MMS_VALIDITY_PERIOD)) {
                showValidityPeriodDialog(hostActivity, subId, intentname);
                return true;
            }
        }
        return false;
    }

    private String getValidityKeyBySubId(Context context, int subId, String intentKey) {
        if (isSms) {
            return Integer.toString(subId) + "_" + SMS_VALIDITY_PERIOD_PREFERENCE_KEY;
        } else {
            return Integer.toString(subId) + "_" + MMS_VALIDITY_PERIOD_PREFERENCE_KEY;
        }
    }

    private void showValidityPeriodDialog(Context context, int subId, String key) {
        final CharSequence[] entries = getResources()
                                   .getTextArray(R.array.sms_validity_peroid_entries);
        final String validityKey = getValidityKeyBySubId(context, subId, key);
        Log.d(TAG, "showValidityPeriodDialog isSms = " + isSms + "validityKey =  " + validityKey);
        final SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(context);
        int title;
        if (isSms) {
            title = R.string.sms_validity_period;
        } else {
            title = R.string.mms_validity_period;
        }

        int index = pref.getInt(validityKey, 4);
        AlertDialog.Builder builder = new AlertDialog.Builder(context)
            .setTitle(getResources().getText(title))
            .setNegativeButton(android.R.string.cancel, null)
            .setSingleChoiceItems(entries, index, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                Log.d(TAG, "showValidityPeriodDialog->onClick: validityKey =" +
                                validityKey + " item =  " + item);
                SharedPreferences.Editor editor = pref.edit();
                editor.putInt(validityKey, item);
                editor.commit();
                dialog.dismiss();
            }
        });
        builder.show();
        builder.create();
    }


    /**
     * configMmsPreferenceState.
     * @param hostActivity Activity
     * @param preference String
     * @param subId int
     * @param cp CheckBoxPreference
     * @return boolean
     */
    public boolean configMmsPreferenceState(Activity hostActivity, String preference,
                                                        int subId, CheckBoxPreference cp) {
        Log.d(TAG, "configMmsPreferenceState[hostActivity]" + hostActivity);
        Log.d(TAG, "configMmsPreferenceState[preference]" + preference);
        Log.d(TAG, "configMmsPreferenceState[simId]" + subId);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostActivity);
        return true;
    }

    /**
     * configMultiSimPreferenceTitle.
     * @param hostActivity Activity
     */
    public void configMultiSimPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
    }

    /**
     * setMmsPreferenceState.
     * @param hostActivity Activity
     * @param preference String
     * @param subId int
     * @param checked boolean
     * @return boolean
     */
    public boolean setMmsPreferenceState(Activity hostActivity, String preference,
                                                    int subId, boolean checked) {
        Log.d(TAG, "setMmsPreferenceState[hostActivity]" + hostActivity);
        Log.d(TAG, "setMmsPreferenceState[preference]" + preference);
        Log.d(TAG, "setMmsPreferenceState[simId]" + subId);
        Log.d(TAG, "setMmsPreferenceState[checked]" + checked);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostActivity);
        SharedPreferences.Editor editor = prefs.edit();
        return false;
    }
}

