package com.mediatek.op18.phone;

import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.telecom.TelecomManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.phone.CallFeaturesSetting;
import com.android.phone.CallWaitingSwitchPreference;
import com.android.phone.GsmUmtsAdditionalCallOptions;
import com.android.phone.settings.PhoneAccountSettingsFragment;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.phone.ext.DefaultCallFeaturesSettingExt;
import com.mediatek.settings.TelephonyUtils;


public class Op18CallFeaturesSettingExt extends DefaultCallFeaturesSettingExt {
    private static final String TAG = "Op18CallFeaturesSettingExt";
    private static final String SMART_CALL_FORWARD_KEY = "smart_forward_key";
    private static final String BUTTON_CLIR_KEY  = "button_clir_key";
    private static final String BUTTON_CW_KEY    = "button_cw_key";
    private OP18WfcSettings mWfcSettings = null;
    private Context mContext;

    private boolean mIsWfcReceiverRegistered = false;
    private Preference mSmartCallForward;

    private BroadcastReceiver mAirplaneModeReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "receive volume change broadcast, stop sound and vibration alert");
            if (mSmartCallForward != null) {
                boolean airplaneModeOn = intent.getBooleanExtra("state", false);
                if (airplaneModeOn) {
                    mSmartCallForward.setEnabled(false);
                } else {
                    mSmartCallForward.setEnabled(true);
                }
            }
        }
    };


    /** Constructor.
     * @param context context
     */
    public Op18CallFeaturesSettingExt(Context context) {
        super();
        mContext = context;
        mWfcSettings = OP18WfcSettings.getInstance(context);
        mContext.registerReceiver(mAirplaneModeReceiver,
                           new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED));
    }

    @Override
    public void initOtherCallFeaturesSetting(PreferenceActivity activity, Object phone) {
        Log.d(TAG, "initOtherCallFeaturesSetting" + activity.getClass().getSimpleName());
        int phoneId = ((Phone) phone).getPhoneId();
        Log.d(TAG, "Customize WFC preference for phoneId :" + phoneId);
        mWfcSettings.customizedWfcPreference(activity, activity.getPreferenceScreen(), null,
                                phoneId);
    }

    @Override
    public boolean needShowOpenMobileDataDialog(Context context, int subId) {
        Log.d(TAG, "needShowOpenMobileDataDialog false");
        return false;
    }

    @Override
    /** Called on events like onResume/onPause etc from CallFeatureSettings.
     * @param event resume/puase etc.
     * @return
     */
    public void onCallFeatureSettingsEvent(int event) {
        Log.d("@M_" + TAG, "CallFeature setting event:" + event);
        //mWfcSettings = OP18WfcSettings.getInstance(mContext);
        switch (event) {
            case DefaultCallFeaturesSettingExt.RESUME:
                if (ImsManager.isWfcEnabledByPlatform(mContext)
                    && !MtkImsManager.isSupportMims()) {
                    mWfcSettings.register();
                    mIsWfcReceiverRegistered = true;
                }
                break;
            case DefaultCallFeaturesSettingExt.PAUSE:
                /* Need to check this to handle Dynamic IMS Switch feature
                * in which IMS can be enable/disabled on the fly */
                if (mIsWfcReceiverRegistered && !MtkImsManager.isSupportMims()) {
                    mWfcSettings.unRegister();
                    mIsWfcReceiverRegistered = false;
                }
                if (!ImsManager.isWfcEnabledByPlatform(mContext)) {
                    mWfcSettings.removeWfcPreference();
                }
                break;
            default:
                break;
        }
    }

    @Override
    public void initOtherCallFeaturesSetting(final PreferenceFragment fragment) {
         Log.d(TAG, "initOtherCallFeaturesSetting: Smart Call Forward");

         final TelecomManager telecomManager = TelecomManager.from(mContext);
         int enabledAccounts = telecomManager.getCallCapablePhoneAccounts().size();
         if (enabledAccounts < 2) {
             return;
         }
         mSmartCallForward = new Preference(fragment.getActivity());
         mSmartCallForward.setKey(SMART_CALL_FORWARD_KEY);
         mSmartCallForward.setTitle(mContext.getString(R.string.smart_call_fwd_settings_title));
         mSmartCallForward.setSummary(mContext.getString(R.string.smart_call_fwd_settings_summary));
         if (TelephonyUtils.isAirplaneModeOn(mContext)) {
             mSmartCallForward.setEnabled(false);
         } else {
             mSmartCallForward.setEnabled(true);
         }

         PreferenceScreen prefSet = ((PhoneAccountSettingsFragment) fragment).getPreferenceScreen();
         prefSet.addPreference(mSmartCallForward);

         mSmartCallForward.setOnPreferenceClickListener(new OnPreferenceClickListener() {

            @Override
            public boolean onPreferenceClick(Preference preference) {
                Handler mainHandler = new Handler(mContext.getMainLooper());
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        AlertDialog.Builder builder = new AlertDialog.Builder(
                                    fragment.getActivity());
                        builder.setTitle(mContext.getString(
                                R.string.smart_call_fwd_enable_alert_dlg_title));
                        builder.setCancelable(false);
                        builder.setMessage(mContext.getString(
                                R.string.smart_call_fwd_alert_dlg_new));
                        builder.setPositiveButton(mContext.getString(
                                R.string.positive_button_title),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Log.d(TAG, "Smart Call Forward started");
                                try {
                                    Intent smartForwadIntent = new Intent();
                                    smartForwadIntent.setFlags(
                                    Intent.FLAG_ACTIVITY_NEW_TASK);
                                    smartForwadIntent.setClassName("com.android.settings",
                                            "com.android.settings.Settings$SmartCallFwdActivity");
                                    mContext.startActivity(smartForwadIntent);
                                } catch (ActivityNotFoundException e) {
                                    Log.e(TAG, e.toString());
                                } catch (Exception e) {
                                    Log.e(TAG, e.toString());
                                }
                                dialog.dismiss();
                            }
                        });
                        builder.setNegativeButton(mContext
                               .getString(R.string.negative_button_title),
                                new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Log.d(TAG, "User select No");
                                dialog.dismiss();
                            }
                        });
                        builder.show();
                    }
                });
                return true;
            }
        });
    }

    public void disableCallFwdPref(Context context, Object obj, Preference pref, int reason) {
        Phone phone = (Phone) obj;
        Log.d(TAG, "disableCallFwdPref");
        Log.d(TAG, "context: " + context);
        Log.d(TAG, "phone: " + phone);
        Log.d(TAG, "pref: " + pref);
        Log.d(TAG, "reason: " + reason);
        String smartKey = "smartcallmode";
        SwitchPreference callWaitPref = null;
        EditTextPreference callFwdPref = null;
        final int SIM1_TO_SIM2 = 1;
        final int SIM2_TO_SIM1 = 2;
        final int SIM1 = 0;
        final int SIM2 = 1;
        boolean needDisable = false;
        int simId = phone.getPhoneId();
        int mode = android.provider.Settings.System.getInt(
                context.getContentResolver(), smartKey, -1);
        if ((reason == -1 || reason == 3) && mode != 0) {
            needDisable = true;
            Log.d(TAG, "mode:" + mode);
            Log.d(TAG, "sim:" + simId);
            if (mode == SIM1_TO_SIM2) {
                Log.d(TAG, "disable preference only for SIM1");
                needDisable = (simId == SIM1) ? true : false;
            } else if (mode == SIM2_TO_SIM1) {
                Log.d(TAG, "disable preference only for SIM2");
                needDisable = (simId == SIM2) ? true : false;
            }

            if (pref != null) {
                if (reason == -1) {
                    callWaitPref = (SwitchPreference) pref;
                }
                if (reason == 3) {
                    callFwdPref = (EditTextPreference) pref;
                }
                Log.d(TAG, "callFwdPref:" + callFwdPref);
                Log.d(TAG, "callWaitPref:" + callWaitPref);
                if (mode > 0 && callFwdPref != null && needDisable)
                    callFwdPref.setEnabled(false);
                if (mode > 0 && callWaitPref != null && needDisable)
                    callWaitPref.setEnabled(false);
            }
        }
    }

    /**
     * Customize GSM Additional Settings
     * @param activity PreferenceActivity
     */
    @Override
    public void customizeAdditionalSettings(PreferenceActivity activity, Object obj) {
        Log.d(TAG, "plugin customizeAdditionalSettings");
        GsmUmtsAdditionalCallOptions additionalPref = (GsmUmtsAdditionalCallOptions) activity;
        Phone phone = (Phone) obj;
        Preference prefCLIR = activity.getPreferenceScreen().findPreference(BUTTON_CLIR_KEY);
        Preference prefCW = activity.getPreferenceScreen().findPreference(BUTTON_CW_KEY);
        prefCLIR.setSummary("");
        prefCLIR.setEnabled(false);
        ((CallWaitingSwitchPreference) prefCW).init(additionalPref, false, phone);
    }

    /**
     * Escape CLIR settings init
     * @return true/false
     */
    @Override
    public boolean escapeCLIRInit() {
        Log.d(TAG, "plugin escapeCLIRInit");
        return true;
    }
}
