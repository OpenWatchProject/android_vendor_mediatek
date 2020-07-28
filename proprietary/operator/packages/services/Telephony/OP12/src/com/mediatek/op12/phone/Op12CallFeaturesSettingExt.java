package com.mediatek.op12.phone;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.CountDownTimer;
import android.os.Handler;
import android.preference.SwitchPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.phone.CallFeaturesSetting;
import com.mediatek.phone.ext.DefaultCallFeaturesSettingExt;

public class Op12CallFeaturesSettingExt extends DefaultCallFeaturesSettingExt {
    private static final String TAG = "Op12CallFeaturesSettingExt";
    private static final String ENABLE_VIDEO_CALLING_KEY = "button_enable_video_calling";
    private static final String KEY_WFC_SETTINGS = "button_wifi_calling_settings_key";
    private static final String DISABLE_VIDEO_CALLING_KEY = "disable_video_calling";
    private Context mContext;
    private boolean mLVCEnabled;
    private boolean mVolteProvisioned;
    private SwitchPreference mEnableVideoCalling;

    private DisabledAppearancePreference mDisablePreference;
    private CallFeaturesSetting mCallSettingActivity;
    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive()... " + action);
            if (mCallSettingActivity != null) {
                handleVideoSettingProvisioning();
            }
        }
    };

    public Op12CallFeaturesSettingExt(Context context) {
        mContext = context;
        mDisablePreference = new DisabledAppearancePreference(mContext, null);
        mDisablePreference.setKey(DISABLE_VIDEO_CALLING_KEY);
        mDisablePreference.setTitle(mContext.getString(R.string.video_calling_title));
        mDisablePreference.setOnPreferenceChangeListener(
                new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                handleDisabledPreferenceClick(mCallSettingActivity);
                return false;
            }
        });
        mContext.registerReceiver(mIntentReceiver,
                 new IntentFilter("com.android.intent.action.IMS_CONFIG_CHANGED"));
    }

    @Override
    public void initOtherCallFeaturesSetting(final PreferenceActivity activity, Object phone) {
        Log.d(TAG, "initOtherCallFeaturesSetting");
        mCallSettingActivity = (CallFeaturesSetting) activity;
        PreferenceScreen prefScreen = mCallSettingActivity.getPreferenceScreen();
        mEnableVideoCalling = (SwitchPreference) prefScreen
                                               .findPreference(ENABLE_VIDEO_CALLING_KEY);
        handleVideoSettingProvisioning();
        Preference wfcPreference = prefScreen.findPreference(KEY_WFC_SETTINGS);
        if (wfcPreference != null) {
            prefScreen.removePreference(wfcPreference);
            Log.d(TAG, "WFC Preference is removed in Call Settings");
        } else {
            Log.d(TAG, "WFC Preference is not removed in Call Settingss");
        }
    }

    @Override
    public void videoPreferenceChange(boolean value) {
        Log.d(TAG, "videoPreferenceChange:" + value);
        if (value) {
            android.provider.Settings.Global.putInt(mContext.getContentResolver(),
                "KEY_CALL_OPTIONS", 1);
        } else {
            android.provider.Settings.Global.putInt(mContext.getContentResolver(),
                "KEY_CALL_OPTIONS", 0);
        }
    }

    private void handleVideoSettingProvisioning() {
        PreferenceScreen prefScreen = mCallSettingActivity.getPreferenceScreen();
        DisabledAppearancePreference disablePreference = (DisabledAppearancePreference) prefScreen
                .findPreference(DISABLE_VIDEO_CALLING_KEY);
        try {
            ImsConfig imsConfig = ImsManager.getInstance(mContext, SubscriptionManager
                            .getDefaultVoicePhoneId()).getConfigInterface();
            mLVCEnabled = (1 == imsConfig.getProvisionedValue(
                                   ImsConfig.ConfigConstants.LVC_SETTING_ENABLED));
            if (ImsManager.isVolteProvisionedOnDevice(mContext)) {
                Log.d(TAG, "VoLTE provisioned");
                if (mEnableVideoCalling != null) {
                     mDisablePreference.setOrder(mEnableVideoCalling.getOrder());
                     handleVideoPreference(prefScreen);
                }
            } else {
                 Log.d(TAG, "VoLTE de-provisioned");
                 if (mEnableVideoCalling != null) {
                     prefScreen.removePreference(mEnableVideoCalling);
                     Log.d(TAG, "VideoCalling Preference removed");
                 }
                 if (disablePreference != null) {
                     prefScreen.removePreference(disablePreference);
                     Log.d(TAG, "Video disablePreference removed");
                 }
            }
        } catch (ImsException e) {
            Log.e(TAG, "Video call settings not updated, ImsConfig null");
            e.printStackTrace();
        }
    }

    private void handleVideoPreference(PreferenceScreen prefSet) {
        DisabledAppearancePreference disablePreference = (DisabledAppearancePreference) prefSet
                .findPreference(DISABLE_VIDEO_CALLING_KEY);
        SwitchPreference enableVideoCalling = (SwitchPreference) prefSet
                .findPreference(ENABLE_VIDEO_CALLING_KEY);
        boolean advancedCalling = ImsManager.isEnhanced4gLteModeSettingEnabledByUser(mContext);
        Log.d(TAG, "handleVideoPreference, mLVCEnabled: " + mLVCEnabled +
                " advancedCalling:" + advancedCalling);
        if (!mLVCEnabled || !advancedCalling) {
             if (enableVideoCalling != null) {
                 prefSet.removePreference(mEnableVideoCalling);
                 Log.d(TAG, "LVC Disabled, so remove video calling Setting Option");
             }
             if (disablePreference == null) {
                 prefSet.addPreference(mDisablePreference);
                 Log.d(TAG, "LVC Disabled, so add disabled video calling Setting Option");
             }
        } else if (mEnableVideoCalling != null) {
            if (disablePreference != null) {
                prefSet.removePreference(mDisablePreference);
                Log.d(TAG, "LVC Enabled, so remove disabled video calling Setting Option");
            }
            if (enableVideoCalling == null) {
                prefSet.addPreference(mEnableVideoCalling);
            }
            boolean videoCalling = Settings.Global.getInt(mContext.getContentResolver(),
                    "KEY_CALL_OPTIONS", 1) == 1;
            Log.d(TAG, "handleVideoPreference, Restore Video Preference Checked:" +
                    videoCalling);
            mEnableVideoCalling.setChecked(videoCalling);
        }
    }

   private void restoreVideoPreference() {
       final PreferenceScreen prefScreen = mCallSettingActivity.getPreferenceScreen();
       new CountDownTimer(3000, 1000) {
           public void onTick(long millisUntilFinished) {
              Log.d(TAG, "Timer started");
           }
           public void onFinish() {
               prefScreen.removePreference(mDisablePreference);
               prefScreen.addPreference(mEnableVideoCalling);
               boolean videoCalling = Settings.Global.getInt(mContext.getContentResolver(),
                                      "KEY_CALL_OPTIONS", 1) == 1;
               Log.d(TAG, "restoreVideoPreference: videoCalling preference value restored:"
                          + videoCalling);
               mEnableVideoCalling.setChecked(videoCalling);
           }
       }.start();
   }

    private void handleDisabledPreferenceClick(final Context appContext) {
        boolean advancedCalling = ImsManager.isEnhanced4gLteModeSettingEnabledByUser(mContext);
        if (!mLVCEnabled) {
            Log.d(TAG, "handleDisabledPreferenceClick: LVC disabled");
            Handler mainHandler = new Handler(mContext.getMainLooper());
            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(appContext);
                    builder.setCancelable(false);
                    builder.setTitle(mContext.getString(R.string.note));
                    builder.setMessage(mContext.getString(R.string.lvc_disable));
                    builder.setPositiveButton(android.R.string.ok, null);
                    AlertDialog dialog = builder.create();
                    dialog.show();
                }
              });
        } else if (!advancedCalling) {
            Log.d(TAG, "handleDisabledPreferenceClick: advancedCalling disabled");
            Handler mainHandler = new Handler(mContext.getMainLooper());
            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(appContext);
                    builder.setCancelable(false);
                    builder.setTitle(mContext.getString(R.string.note));
                    builder.setMessage(mContext.getString(R.string.volte_disable));
                    builder.setPositiveButton(mContext.getString(R.string.volte_calling_title),
                            new OnClickListener() {
                                  @Override
                                  public void onClick(DialogInterface dialog, int which) {
                                     ImsManager.setEnhanced4gLteModeSetting(mContext, true);
                                     restoreVideoPreference();
                                  }
                    });
                    builder.setNegativeButton(android.R.string.cancel, null);
                    AlertDialog dialog = builder.create();
                    dialog.show();
                }
            });
        }
    }
}
