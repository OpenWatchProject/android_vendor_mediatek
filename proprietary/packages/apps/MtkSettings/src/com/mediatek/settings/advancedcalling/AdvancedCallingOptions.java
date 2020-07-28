
package com.mediatek.settings.advancedcalling;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.Phone;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.widget.SwitchBar;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * "Wi-Fi Calling settings" screen.  This preference screen lets you
 * enable/disable Wi-Fi Calling and change Wi-Fi Calling mode.
 */
public class AdvancedCallingOptions extends SettingsPreferenceFragment
        implements SwitchBar.OnSwitchChangeListener {

    private static final String TAG = "OP12AdvancedCallingOptionsFragment";
    private static final String PREF_ADVANCED_SETTING = "preff_advanced_calling";

    private SwitchBar mSwitchBar;
    private Switch mSwitch;
    private RadioGroup mRadioGroup;
    private RadioButton mVoiceVideoButton = null;
    private RadioButton mVoiceButton;

    private Context mContext;
    private boolean mValidListener = false;
    private IntentFilter mIntentFilter;
    private boolean mEnablePlatform;
    private boolean mEnableLVC;
    private ImsManager mImsManager;
    private int mMainId;

    private final ImsConnectionStateListener mImsRegListener = new ImsConnectionStateListener() {
        @Override
        public void onFeatureCapabilityChanged(int serviceClass,
                int[] enabledFeatures, int[] disabledFeatures) {
            Log.d(TAG, "Receive IMS FeatureCapabilityChanged");
            handleImsStateChange(serviceClass, enabledFeatures, disabledFeatures);
        }
    };

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive()... " + action);
            if (action.equals(ImsManager.ACTION_IMS_REGISTRATION_ERROR)) {
                Log.d(TAG, "IMS Registration error, disable Switch");
                setResultCode(Activity.RESULT_CANCELED);
                // UX requirement is to disable WFC in case of "permanent" registration failures.
                mSwitch.setChecked(false);
                showAlert(intent);
            } else if (action.equals("com.android.intent.action.IMS_CONFIG_CHANGED")) {
            	int type = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
            	String value = intent.getStringExtra(ImsConfig.EXTRA_NEW_VALUE);
            	boolean needRecreate = false;
                Log.i(TAG, "IMS_CONFIG_CHANGED type = " + type + " value = " + value);
            	if (ImsConfig.ConfigConstants.LVC_SETTING_ENABLED != type) {
            		Log.i(TAG, "do nothing because LVC_SETTING_ENABLED no changed");
            		return;
            	}
            	if (mEnableLVC && (null != value && !("1".equals(value)))) {
            		/*
            		 * 'mEnableLVC = true' that means :
            		 * before imsConfig.getProvisionedValue(ImsConfig.ConfigConstants.LVC_SETTING_ENABLED) == 1
            		 * after receive broadcast, if value != 1;
            		 * means value changed , need finish activity
            		 */
            		needRecreate = true;
            	}
            	if (!mEnableLVC && (null != value && ("1".equals(value)))) {
            		/*
            		 * 'mEnableLVC = false' that means :
            		 * before imsConfig.getProvisionedValue(ImsConfig.ConfigConstants.LVC_SETTING_ENABLED) == 0
            		 * after receive broadcast, if value == 1;
            		 * means value changed , need finish activity
            		 */
            		needRecreate = true;
            	}
                Log.d(TAG, "config changed, finish Advance Calling activity");
                if (!needRecreate) {
            		Log.i(TAG, "do nothing because LVC_SETTING_ENABLED value no changed"
            				+ " mEnableLVC = " + mEnableLVC + " value = " + value
            				+ " needRecreate = " + needRecreate);
            		return;
                }
                try {
                    ImsConfig imsConfig = ImsManager.getInstance(mContext, SubscriptionManager
                                .getDefaultVoicePhoneId()).getConfigInterface();
                    mEnableLVC = (1 == imsConfig.getProvisionedValue(
                                    ImsConfig.ConfigConstants.LVC_SETTING_ENABLED));
                    Log.d(TAG, "enableLVC:" + mEnableLVC + "  enablePlatform:" +
                                             mEnablePlatform);
                 } catch (ImsException e) {
                     Log.e(TAG, "Advanced settings not updated, ImsConfig null");
                     e.printStackTrace();
                 }
                 getActivity().finish();

            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                Log.d(TAG, "Phone state changed, so update the screen");
                updateScreen();
            }
        }
    };

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.d(TAG, "onActivityCreated");
        final SettingsActivity activity = (SettingsActivity) getActivity();

        mSwitchBar = activity.getSwitchBar();
        mSwitch = mSwitchBar.getSwitch();
        mSwitchBar.show();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getActivity();
        mMainId = TelephonyUtils.getMainCapabilityPhoneId();
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(ImsManager.ACTION_IMS_REGISTRATION_ERROR);
        mIntentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        mIntentFilter.addAction("com.android.intent.action.IMS_CONFIG_CHANGED");
        mEnablePlatform = ImsManager.isVtEnabledByPlatform(mContext);
        mImsManager = ImsManager.getInstance(mContext, mMainId);
        try {
             ImsConfig imsConfig = ImsManager.getInstance(mContext, SubscriptionManager
                              .getDefaultVoicePhoneId()).getConfigInterface();
             mEnableLVC = (1 == imsConfig.getProvisionedValue(
                           ImsConfig.ConfigConstants.LVC_SETTING_ENABLED));
            Log.d(TAG, "enableLVC:" + mEnableLVC
                    + "  enablePlatform:" + mEnablePlatform
                    + " mMainId = " + mMainId);
        } catch (ImsException e) {
            Log.e(TAG, "Advanced settings not updated, ImsConfig null");
            e.printStackTrace();
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                        Bundle savedInstanceState) {
        View radioView;
        if (mEnablePlatform) {
            radioView = inflater.inflate(R.layout.calling_pref_layout, container, false);
            mVoiceVideoButton = (RadioButton) radioView.findViewById(R.id.hd_voice_video);
        } else {
            radioView = inflater.inflate(R.layout.calling_voice_only_pref_layout, container, false);
        }
        mRadioGroup = (RadioGroup) radioView.findViewById(R.id.hd_voice_video_group);
        mVoiceButton = (RadioButton) radioView.findViewById(R.id.hd_voice_only);
        mRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
          public void onCheckedChanged(RadioGroup group, int checkedId) {
            // checkedId is the RadioButton selected
              switch (checkedId) {
                case R.id.hd_voice_video:
                    Log.d(TAG, "Video Button checked");
                    if (!mEnableLVC) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                        builder.setCancelable(false);
                        builder.setTitle(mContext.getString(R.string.note));
                        builder.setMessage(mContext.getString(R.string.lvc_disable));
                        builder.setPositiveButton(android.R.string.ok, null);
                        AlertDialog dialog = builder.create();
                        dialog.show();
                        mVoiceVideoButton.setChecked(false);
                        mVoiceButton.setEnabled(false);
                        mVoiceButton.setChecked(true);
                        mVoiceButton.setEnabled(true);
                        Log.d(TAG, "LVC is disabled, so disable HD Voice and Video option");
                    } else {
                        if (mVoiceVideoButton.isEnabled()) {
                            Log.d(TAG, "Show Video Button ON dialog");
                            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                            LayoutInflater builderInflater = LayoutInflater.from(mContext);
                            View checkboxLayout = builderInflater.inflate(
                                                       R.layout.skip_checkbox, null);
                            final CheckBox dontShowAgain = (CheckBox) checkboxLayout
                                            .findViewById(R.id.skip_box);
                            builder.setView(checkboxLayout);
                            builder.setCancelable(false);
                            builder.setTitle(mContext.getString(R.string.note));
                            builder.setMessage(mContext.getString(
                                            R.string.advance_calling_enable_msg))
                                   .setPositiveButton(android.R.string.ok,
                                             new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                          String checkBoxResult = "NOT checked";
                                          if (dontShowAgain.isChecked()) {
                                              checkBoxResult = "checked";
                                          }
                                            SharedPreferences settings = mContext
                                             .getSharedPreferences(PREF_ADVANCED_SETTING, 0);
                                            SharedPreferences.Editor editor = settings.edit();
                                            editor.putString("skipMessage3", checkBoxResult);
                                            editor.commit();
                                    }
                                });
                            AlertDialog dialog = builder.create();
                            SharedPreferences settings = mContext.getSharedPreferences(
                                                                 PREF_ADVANCED_SETTING, 0);
                            String skipMessage = settings.getString("skipMessage3",
                                                                    "NOT checked");
                            if (!skipMessage.equalsIgnoreCase("checked")) {
                                  dialog.show();
                            }
                            mImsManager.setVtSetting(mContext, true);
                            Log.d(TAG, "Set VT true...");
                            Settings.Global.putInt(mContext.getContentResolver(),
                                        "KEY_CALL_OPTIONS", 1);
                      }
                    }
                break;
            case R.id.hd_voice_only:
                    Log.d(TAG, "Voice only button checked");
                    if (mVoiceButton.isEnabled()) {
                        Log.d(TAG, "Show Voice only Button ON dialog");
                        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                        LayoutInflater builderInflater = LayoutInflater.from(mContext);
                        View checkboxLayout = builderInflater.inflate(R.layout.skip_checkbox,
                                               null);
                        final CheckBox dontShowAgain = (CheckBox) checkboxLayout
                                                              .findViewById(R.id.skip_box);
                        builder.setView(checkboxLayout);
                        builder.setTitle(mContext.getString(R.string.note));
                        builder.setCancelable(false);
                        builder.setMessage(mContext.getString(R.string.enable_hd_voice_only_msg))
                               .setPositiveButton(android.R.string.ok,
                                               new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                      String checkBoxResult = "NOT checked";
                                      if (dontShowAgain.isChecked()) {
                                          checkBoxResult = "checked";
                                      }
                                        SharedPreferences settings = mContext
                                                 .getSharedPreferences(PREF_ADVANCED_SETTING, 0);
                                        SharedPreferences.Editor editor = settings.edit();
                                        editor.putString("skipMessage4", checkBoxResult);
                                        editor.commit();
                                }
                            });
                        AlertDialog dialog = builder.create();
                        SharedPreferences settings = mContext.getSharedPreferences(
                                                              PREF_ADVANCED_SETTING, 0);
                        String skipMessage = settings.getString("skipMessage4", "NOT checked");
                        if (!skipMessage.equalsIgnoreCase("checked")) {
                              dialog.show();
                        }
                        if (mEnablePlatform && mEnableLVC) {
                           Settings.Global.putInt(mContext.getContentResolver(),
                                       "KEY_CALL_OPTIONS", 0);
                            mImsManager.setVtSetting(mContext, false);
                            Log.d(TAG, "Set VT false...");
                        }
                    }
                break;
              }
          }
      });
      return radioView;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "On Resume...");
        if (mImsManager.isVolteEnabledByPlatform(mContext)) {
            mSwitchBar.addOnSwitchChangeListener(this);
            mValidListener = true;
        }

        // NOTE: Buttons will be enabled/disabled in mPhoneStateListener
        boolean advCallingEnabled = mImsManager.isEnhanced4gLteModeSettingEnabledByUser(mContext);
        if (advCallingEnabled) {
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                mRadioGroup.getChildAt(i).setEnabled(false);
            }
            if (mEnablePlatform && mEnableLVC) {
                mVoiceVideoButton.setAlpha(1.0f);
                if (mImsManager.isVtEnabledByUser(mContext)) {
                    mRadioGroup.check(R.id.hd_voice_video);
                } else {
                    mRadioGroup.check(R.id.hd_voice_only);
                }
            } else if (mEnablePlatform && !mEnableLVC) {
                    mVoiceVideoButton.setAlpha(0.2f);
                    mRadioGroup.check(R.id.hd_voice_only);
            } else {
                    mRadioGroup.check(R.id.hd_voice_only);
            }
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                    mRadioGroup.getChildAt(i).setEnabled(true);
            }
        } else {
            if (mEnablePlatform && mEnableLVC) {
                mVoiceVideoButton.setAlpha(1.0f);
            } else if (mEnablePlatform && !mEnableLVC) {
                mVoiceVideoButton.setAlpha(0.2f);
            }
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                mRadioGroup.getChildAt(i).setEnabled(false);
            }
            mRadioGroup.clearCheck();
        }
        mSwitch.setChecked(advCallingEnabled);
        updateScreen();
        mContext.registerReceiver(mIntentReceiver, mIntentFilter);

        Intent intent = getActivity().getIntent();
        if (intent.getBooleanExtra(Phone.EXTRA_KEY_ALERT_SHOW, false)) {
             showAlert(intent);
        }
        registerForImsStateChange();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mValidListener) {
            mValidListener = false;
            mSwitchBar.removeOnSwitchChangeListener(this);
        }
        mContext.unregisterReceiver(mIntentReceiver);
        unRegisterForImsStateChange();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mSwitchBar.hide();
    }

    /**
     * Listens to the state change of the switch.
     */
    @Override
    public void onSwitchChanged(Switch switchView, boolean isChecked) {
        Log.d(TAG, "OnSwitchChanged, disable switchbar");
        mSwitchBar.setEnabled(false);
        if (isChecked) {
            Log.d(TAG, "Switch is checked");
            Settings.Global.putInt(mContext.getContentResolver(),
                    "KEY_ADVANCED_CALLING", 1);
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            LayoutInflater builderInflater = LayoutInflater.from(mContext);
            View checkboxLayout = builderInflater.inflate(R.layout.skip_checkbox, null);
            final CheckBox dontShowAgain = (CheckBox) checkboxLayout.findViewById(R.id.skip_box);
            builder.setView(checkboxLayout);
            builder.setTitle(mContext.getString(R.string.note));
            builder.setCancelable(false);
            builder.setMessage(mContext.getString(R.string.advance_calling_enable_msg))
                   .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                          String checkBoxResult = "NOT checked";
                          if (dontShowAgain.isChecked()) {
                              checkBoxResult = "checked";
                          }
                            SharedPreferences settings = mContext.getSharedPreferences(
                                                          PREF_ADVANCED_SETTING, 0);
                            SharedPreferences.Editor editor = settings.edit();
                            editor.putString("skipMessage1", checkBoxResult);
                            editor.commit();
                    }
                });
            AlertDialog dialog = builder.create();
            SharedPreferences settings = mContext.getSharedPreferences(PREF_ADVANCED_SETTING, 0);
            String skipMessage = settings.getString("skipMessage1", "NOT checked");
            if (!skipMessage.equalsIgnoreCase("checked")) {
                  dialog.show();
            }
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                mRadioGroup.getChildAt(i).setEnabled(false);
            }
            int callingOption = Settings.Global.getInt(mContext.getContentResolver(),
                    "KEY_CALL_OPTIONS", 1);
            if (mEnablePlatform && mEnableLVC && callingOption == 1) {
                mRadioGroup.check(R.id.hd_voice_video);
            } else {
                mRadioGroup.check(R.id.hd_voice_only);
            }
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                mRadioGroup.getChildAt(i).setEnabled(true);
            }
        } else {
            Log.d(TAG, "Switch is Unchecked");
            Settings.Global.putInt(mContext.getContentResolver(),
                    "KEY_ADVANCED_CALLING", 0);
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            LayoutInflater builderInflater = LayoutInflater.from(mContext);
            View checkboxLayout = builderInflater.inflate(R.layout.skip_checkbox, null);
            final CheckBox dontShowAgain = (CheckBox) checkboxLayout.findViewById(R.id.skip_box);
            builder.setView(checkboxLayout);
            builder.setTitle(mContext.getString(R.string.note));
            builder.setCancelable(false);
            builder.setMessage(mContext.getString(R.string.advance_calling_disable_msg))
                   .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                          String checkBoxResult = "NOT checked";
                          if (dontShowAgain.isChecked()) {
                              checkBoxResult = "checked";
                          }
                            SharedPreferences settings = mContext.getSharedPreferences(
                                                          PREF_ADVANCED_SETTING, 0);
                            SharedPreferences.Editor editor = settings.edit();
                            editor.putString("skipMessage2", checkBoxResult);
                            editor.commit();
                    }
                });
            AlertDialog dialog = builder.create();
            SharedPreferences settings = mContext.getSharedPreferences(PREF_ADVANCED_SETTING, 0);
            String skipMessage = settings.getString("skipMessage2", "NOT checked");
            if (!skipMessage.equalsIgnoreCase("checked")) {
                  dialog.show();
            }
            if (mEnableLVC) {
                if (mRadioGroup.getCheckedRadioButtonId() == R.id.hd_voice_video) {
                     Settings.Global.putInt(mContext.getContentResolver(),
                              "KEY_CALL_OPTIONS", 1);
                } else {
                     Settings.Global.putInt(mContext.getContentResolver(),
                              "KEY_CALL_OPTIONS", 0);
                }
            }
            for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
                mRadioGroup.getChildAt(i).setEnabled(false);
            }
            mRadioGroup.clearCheck();
        }
        int callingOption = Settings.Global.getInt(mContext.getContentResolver(),
                "KEY_CALL_OPTIONS", 1);
        boolean isVtEnable = mEnablePlatform && mEnableLVC && isChecked && (callingOption == 1);
        Log.d(TAG, "onSwitchChanged, VoLTE: " + isChecked + " ViLTE: " + isVtEnable +
                " mEnablePlatform: " + mEnablePlatform + " mEnableLVC: " + mEnableLVC +
                " callingOption: " + callingOption);
        ImsManager imsManager = ImsManager.getInstance(
                mContext, TelephonyUtils.getMainCapabilityPhoneId());
        ((MtkImsManager) imsManager).setEnhanced4gLteModeVtSetting(mContext, isChecked, isVtEnable);
    }

    private void showAlert(Intent intent) {

        CharSequence title = intent.getCharSequenceExtra(Phone.EXTRA_KEY_ALERT_TITLE);
        CharSequence message = intent.getCharSequenceExtra(Phone.EXTRA_KEY_ALERT_MESSAGE);

        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        builder.setMessage(message)
                .setTitle(title)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setPositiveButton(android.R.string.ok, null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void updateScreen() {
        final SettingsActivity activity = (SettingsActivity) getActivity();
        if (activity == null) {
            return;
        }
        final SwitchBar switchBar = activity.getSwitchBar();
        boolean isAdvanceCallingEnabled = switchBar.getSwitch().isChecked();
        boolean isCallStateIdle = !TelecomManager.from(activity).isInCall();
        Log.d(TAG, "isAdvanceCallingEnabled: " + isAdvanceCallingEnabled
                + ", isCallStateIdle: " + isCallStateIdle);
        switchBar.setEnabled(isCallStateIdle);
        if (mEnablePlatform) {
            mVoiceVideoButton.setEnabled(isAdvanceCallingEnabled && isCallStateIdle);
        }
        mVoiceButton.setEnabled(isAdvanceCallingEnabled && isCallStateIdle);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.WIFI_CALLING;
    }

    private boolean isInternetConnected() {
        ConnectivityManager connectivityMgr = (ConnectivityManager) mContext
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo wifi = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        NetworkInfo mobile = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        // Check if wifi or mobile network is available or not. If any of them is
        // available or connected then it will return true, otherwise false;
        if (wifi != null) {
            if (wifi.isConnected()) {
                Log.d(TAG, "isInternetConnected: Wifi is connected");
                return true;
            }
        }
        if (mobile != null) {
            if (mobile.isConnected()) {
                Log.d(TAG, "isInternetConnected: Mobile data is connected");
                return true;
            }
        }
        Log.d(TAG, "isInternetConnected: Internet not Connected");
        return false;
    }


    private void registerForImsStateChange() {
        try {
            mImsManager.addRegistrationListener(ImsServiceClass.MMTEL, mImsRegListener);
        } catch (ImsException e) {
            Log.e(TAG, "addRegistrationListener: " + e);
        }
    }

    private void unRegisterForImsStateChange() {
        try {
            mImsManager.removeRegistrationListener(mImsRegListener);
        } catch (ImsException e) {
            Log.e(TAG, "removeRegistrationListener: " + e);
        }
    }

    private void handleImsStateChange(int serviceClass,
                int[] enabledFeatures, int[] disabledFeatures) {
        if (serviceClass == ImsServiceClass.MMTEL) {
            Log.d(TAG, "VoLTE capability changed to :" +
                    enabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE]);
            mSwitchBar.setEnabled(true);
            Log.d(TAG, "handleImsStateChange: Feature VoLTE is enabled, so enable switch");
        }
    }
}

