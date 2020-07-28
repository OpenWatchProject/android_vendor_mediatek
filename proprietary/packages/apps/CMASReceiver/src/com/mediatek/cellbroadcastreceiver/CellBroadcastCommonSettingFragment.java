package com.mediatek.cellbroadcastreceiver;

import java.io.IOException;
import java.util.List;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.SeekBar;

import com.mediatek.cmas.ext.ICmasMainSettingsExt;
import com.mediatek.cellbroadcastreceiver.CheckBoxAndSettingsPreference.OnSettingChangedListener;

public class CellBroadcastCommonSettingFragment extends PreferenceFragment
                                            implements OnSettingChangedListener {

    private static final String TAG = "[CMAS]CellBroadcastCommonSettingFragment";

    private int mSlotId = 0;
    public static int sReadySlotId = -1;
    private boolean mRemoveCommonSettings = false;

    CellBroadcastLanguageSettings mLanguageSetting;

    private CheckBoxAndSettingsPreference mEnableCBCheckBox;
    private CheckBoxAndSettingsPreference mImminentCheckBox;
    private CheckBoxAndSettingsPreference mAmberCheckBox;
    private CheckBoxAndSettingsPreference mSpeechCheckBox;
    private CheckBoxAndSettingsPreference mEnableAllCheckBox;
    private Preference mLanguagePref;
    public static final String KEY_ENABLE_ALERT_VIBRATE = "enable_key_alert_vibrate";
    public static final String KEY_ALERT_SOUND_VOLUME = "enable_key_sound_volume";
    public static final String KEY_LANGUAGE_SETTING = "key_language_settings";

    private MediaPlayer mMediaPlayer;
    private float mAlertVolume = 1.0f;
    private Context mContext;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getContext();
        mSlotId = getArguments().getInt("slotid");
        Log.d(TAG, "mSlotId = " + mSlotId);
        mRemoveCommonSettings = getArguments().getBoolean("removeCommonSetting");
        addPreferencesFromResource(R.xml.preferences_main);
        mLanguageSetting = new CellBroadcastLanguageSettings(mContext, mSlotId);
        if (mRemoveCommonSettings) {
            PreferenceScreen prefScr = getPreferenceScreen();
            prefScr.removePreference(findPreference("category_common_settings"));
        }
        SharedPreferences prefs =
            PreferenceManager.getDefaultSharedPreferences(mContext);
        String slotKey = CMASUtils.getSlotKey(mSlotId);
        mEnableCBCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST);
        mEnableCBCheckBox.setOnSettingChangedListener(this);
        mEnableCBCheckBox.setSlotId(mSlotId);
        mEnableCBCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST, true));

        mImminentCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_IMMINENT_ALERTS);
        mImminentCheckBox.setSlotId(mSlotId);
        mImminentCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_IMMINENT_ALERTS, true));

        mAmberCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS);
        mAmberCheckBox.setSlotId(mSlotId);
        mAmberCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS, true));
        mSpeechCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_SPEECH);
        mEnableAllCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_ALL_ALERT);
        mEnableAllCheckBox.setOnSettingChangedListener(this);
        mEnableAllCheckBox.setSlotId(mSlotId);
        mEnableAllCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_ALL_ALERT, true));

        mLanguagePref = findPreference(KEY_LANGUAGE_SETTING);
        mLanguagePref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                mLanguageSetting.showLanguageSelectDialog(mSlotId);
                return true;
            }
         });

        refreshEnableCheckBox();
        IntentFilter intentFilter =
                new IntentFilter(CellBroadcastConfigService.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mSimStateChangeReceiver, intentFilter);

        Log.d(TAG, "calling Main setting Plugin");
        ICmasMainSettingsExt optAddingAlertVolumeVibration = (ICmasMainSettingsExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
        if (optAddingAlertVolumeVibration != null) {
            if (optAddingAlertVolumeVibration.needToaddAlertSoundVolumeAndVibration()) {
                addAlertSoundVolumeAndVibration();
            }

            optAddingAlertVolumeVibration.addNewChannelAlertsSetting(
                CellBroadcastCommonSettingFragment.this, slotKey, mSlotId);

            optAddingAlertVolumeVibration.activateSpanishAlertOption(
                    CellBroadcastCommonSettingFragment.this);
            Log.d(TAG, "Called Main setting Plugin");
        }
        mLanguageSetting.initLanguage();
    }

    @Override
    public void onResume() {
        super.onResume();
        mLanguageSetting.updateLanguagesFromModemConfig();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mContext.unregisterReceiver(mSimStateChangeReceiver);
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");
        stopMediaPlayer();

        AlertTonePreviewDialog alertPreviewDialog = (AlertTonePreviewDialog)
                findPreference("cmas_preview_alert_tone");
        if (alertPreviewDialog != null) {
            alertPreviewDialog.onDialogClosed(false);
        }
    }

    void refreshEnableCheckBox() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                boolean canReceiveMessage = false;
                SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
                List<SubscriptionInfo> subscriptionInfoList =
                        subscriptionManager.getActiveSubscriptionInfoList();
                if (subscriptionInfoList == null ||
                        (subscriptionInfoList != null && subscriptionInfoList.isEmpty())) {
                    Log.d(TAG, "there is no sim card");
                    if (CmasConfigManager.getGeminiSupport()) {
                        getActivity().finish();
                        return;
                    }
                    canReceiveMessage = false;
                } else {
                    boolean subchanged = true;
                    for (SubscriptionInfo subInfo : subscriptionInfoList) {
                        // check whether has a SIM that can receive CMAS
                        if (CmasConfigManager.getGeminiSupport() &&
                                 subInfo.getSimSlotIndex() == mSlotId) {
                            subchanged = false;
                        }
                        if (TelephonyManager.getDefault().getSimState(subInfo.getSimSlotIndex()) ==
                                TelephonyManager.SIM_STATE_READY) {
                            canReceiveMessage = true;
                        }
                    }
                    if (CmasConfigManager.getGeminiSupport() && subchanged) {
                        getActivity().finish();
                    }
                }
                Log.d(TAG, "refreshEnableCheckBox canReceiveMessage " + canReceiveMessage);
                if (canReceiveMessage) {
                    mEnableCBCheckBox.setEnabled(true);
                    setCheckBoxPreferenceEnable(mEnableCBCheckBox.isChecked());
                    mLanguagePref.setEnabled(true);
                } else {
                    mEnableCBCheckBox.setEnabled(false);
                    setCheckBoxPreferenceEnable(false);
                    mLanguagePref.setEnabled(false);
                }
            }
        });

    }

    public void setCheckBoxPreferenceEnable(boolean enabled) {
        mImminentCheckBox.setEnabled(enabled && (!mEnableAllCheckBox.isChecked()));
        mAmberCheckBox.setEnabled(enabled && (!mEnableAllCheckBox.isChecked()));
        if (mSpeechCheckBox != null) {
            mSpeechCheckBox.setEnabled(enabled);
        }
        mEnableAllCheckBox.setEnabled(enabled);
    }

    @Override
    public void onEnableCBChanged() {
        Log.d(TAG, "onEnableCBChanged ");
        if (mEnableCBCheckBox.isChecked()) {
            Log.d(TAG, "onEnableCBChanged true ");
            setCheckBoxPreferenceEnable(true);
        } else {
            Log.d(TAG, "onEnableCBChanged false");
            setCheckBoxPreferenceEnable(false);
        }
    }

    @Override
    public void onEnableAllChanged() {
        Log.d(TAG, "onEnableAllChanged ");
        if (mEnableAllCheckBox.isChecked()) {
            Log.d(TAG, "onEnableAllChanged true ");
            setAlertPreferenceEnable(true);
        } else {
            Log.d(TAG, "onEnableAllChanged false");
            setAlertPreferenceChecked(true);
            setAlertPreferenceEnable(false);
        }
    }

    public void setAlertPreferenceEnable(boolean enabled) {
        mImminentCheckBox.setEnabled(enabled);
        mAmberCheckBox.setEnabled(enabled);
    }

    private void setAlertPreferenceChecked(boolean checked) {
        mImminentCheckBox.setChecked(checked);
        mAmberCheckBox.setChecked(checked);
    }

    private BroadcastReceiver mSimStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            refreshEnableCheckBox();
        }
    };

    /**
     * Stop mediaplayer when press Home key.
     */
     private void stopMediaPlayer() {
        if (mMediaPlayer != null) {
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    private void addAlertSoundVolumeAndVibration() {
        Log.d(TAG, "[addAlertSoundVolume]");
        CheckBoxPreference vibrate = new CheckBoxPreference(mContext);
        PreferenceScreen prefSet = getPreferenceScreen();
        vibrate.setKey(KEY_ENABLE_ALERT_VIBRATE);
        vibrate.setChecked(true);
        vibrate.setDefaultValue(true);
        vibrate.setTitle(getString(R.string.enable_alert_vibrate_title));
        vibrate.setSummary(getString(R.string.enable_alert_vibrate_summary));
        prefSet.addPreference(vibrate);

        vibrate.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {

                boolean value = (Boolean) newValue;
                SharedPreferences prefs =
                        PreferenceManager.getDefaultSharedPreferences(mContext);
                SharedPreferences.Editor editor = prefs.edit();

                editor.putBoolean(KEY_ENABLE_ALERT_VIBRATE, value);
                editor.commit();
                ICmasMainSettingsExt optAddingAlertVolumeVibration = (ICmasMainSettingsExt)
                        CellBroadcastPluginManager.getCellBroadcastPluginObject(
                        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
                if (optAddingAlertVolumeVibration != null) {
                    optAddingAlertVolumeVibration.updateVibrateValue(value);
                }
                return true;
            }
        });

        Preference volume = new Preference(mContext);

        volume.setKey(KEY_ALERT_SOUND_VOLUME);
        volume.setDefaultValue(1.0f);
        volume.setTitle(getString(R.string.alert_sound_volume));
        volume.setSummary(getString(R.string.alert_volume_summary));

        prefSet.addPreference(volume);
        Log.d(TAG, "addAlertSoundVolume   Preference is added");

        OnPreferenceClickListener prefClickListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Log.i(TAG, "OnPreferenceClickListener onclicked ");
                final AlertDialog.Builder dialog =
                        new AlertDialog.Builder(mContext);
                LayoutInflater flater = getLayoutInflater();
                View v = flater.inflate(R.layout.alert_dialog_view, null);

                SeekBar sb = (SeekBar) v.findViewById(R.id.seekbar);
                // set bar's progress
                SharedPreferences prefs =
                        PreferenceManager.getDefaultSharedPreferences(
                        mContext);
                float pro = 1.0f;
                pro = prefs.getFloat(KEY_ALERT_SOUND_VOLUME, 1.0f);
                int progress = (int) (pro * 100);
                if (progress < 0) {
                    progress = 0;
                } else if (progress > 100) {
                    progress = 100;
                }
                Log.d(TAG, "open volume setting,progress:" + progress + ",pro:" + pro);
                sb.setProgress(progress);
                sb.setOnSeekBarChangeListener(getSeekBarListener());
                dialog.setTitle(getString(R.string.alert_sound_volume))
                .setView(v)
                .setPositiveButton(getString(R.string.button_dismiss), new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        // TODO Auto-generated method stub
                        SharedPreferences prefs =
                                PreferenceManager.getDefaultSharedPreferences(
                                mContext);
                        SharedPreferences.Editor editor = prefs.edit();

                        editor.putFloat(KEY_ALERT_SOUND_VOLUME, mAlertVolume);
                        editor.commit();
                        ICmasMainSettingsExt optAddingAlertVolumeVibration = (ICmasMainSettingsExt)
                                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
                        if (optAddingAlertVolumeVibration != null) {
                            optAddingAlertVolumeVibration.updateVolumeValue(mAlertVolume);
                        }
                        Log.d(TAG, "Volume saved:" + mAlertVolume);
                        if (mMediaPlayer != null) {
                            mMediaPlayer.release();
                            mMediaPlayer = null;
                        }
                    }
                })
                .setNegativeButton(getString(R.string.button_cancel), new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int arg1) {
                        // TODO Auto-generated method stub
                        dialog.dismiss();

                        if (mMediaPlayer != null) {
                            mMediaPlayer.release();
                            mMediaPlayer = null;
                        }
                    }
                })
                .setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            dialog.cancel();

                            if (mMediaPlayer != null) {
                                mMediaPlayer.release();
                                mMediaPlayer = null;
                            }
                        }
                        return false;
                    }
                })
                .show();
                return true;
            }
        };
        volume.setOnPreferenceClickListener(prefClickListener);
    }

    private SeekBar.OnSeekBarChangeListener getSeekBarListener() {
        return new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    mAlertVolume = progress / 100.0f;
                    Log.d(TAG, "volume:" + mAlertVolume);
                }
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                Log.d(TAG, "onStartTrackingTouch");

                if (mMediaPlayer != null && mMediaPlayer.isPlaying()) {
                    mMediaPlayer.stop();
                } else {
                    mMediaPlayer = new MediaPlayer();
                    mMediaPlayer.setAudioStreamType(AudioManager.STREAM_ALARM);
                    try {
                        AssetFileDescriptor afd =
                                mContext.getResources().openRawResourceFd(
                                        R.raw.attention_signal);
                        if (afd != null) {
                            mMediaPlayer.setDataSource(afd.getFileDescriptor(), afd
                                    .getStartOffset(), afd.getLength());
                            afd.close();
                        }
                    } catch (IOException e) {
                        Log.e(TAG, "IOException onStartTrackingTouch: " + e);
                    } catch (IllegalStateException e) {
                        Log.e(TAG, "IllegalStateException onStartTrackingTouch: " + e);
                    }
                }
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // make some sample sound
                Log.d(TAG, "onStopTrackingTouch");
                try {
                    mMediaPlayer.setVolume(mAlertVolume, mAlertVolume);
                    mMediaPlayer.prepare();
                    mMediaPlayer.seekTo(0);
                    mMediaPlayer.start();
                } catch (IOException e) {
                    Log.e(TAG, "exception onStopTrackingTouch: " + e);
                } catch (IllegalStateException e) {
                    Log.e(TAG, "IllegalStateException onStartTrackingTouch: " + e);
                }
            }
        };
    }
}
