package com.mediatek.cellbroadcastreceiver;

import java.io.IOException;
import java.util.List;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.AssetFileDescriptor;
import android.graphics.drawable.BitmapDrawable;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.SeekBar;

import com.mediatek.cmas.ext.ICmasMainSettingsExt;

public class CellBroadcastGeminiSettingFragment extends PreferenceFragment {

    private static final String TAG = "[CMAS]CellBroadcastDualSimSettingFragment";

    public static int sSlotId;
    public static int sReadySlotId = -1;

    public static final String KEY_ENABLE_ALERT_VIBRATE = "enable_key_alert_vibrate";
    public static final String KEY_ALERT_SOUND_VOLUME = "enable_key_sound_volume";
    public static final String KEY_ACCOUNT_SETTING = "category_account_settings";
    public static final String KEY_SUB1_SETTING = "subone_account_settings";
    public static final String KEY_SUB2_SETTING = "subtwo_account_settings";

    private MediaPlayer mMediaPlayer;
    private float mAlertVolume = 1.0f;
    private Context mContext;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getContext();
        addPreferencesFromResource(R.xml.preferences_dualsim);
        List<SubscriptionInfo> subInfoList
        = SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
        PreferenceCategory accountPreference =
            (PreferenceCategory) findPreference(KEY_ACCOUNT_SETTING);
        Preference account1ItemPreference = new Preference(mContext);
        account1ItemPreference.setKey(KEY_SUB1_SETTING);
        SubscriptionInfo firstRecord = subInfoList.get(0);
        account1ItemPreference.setIcon(new BitmapDrawable(mContext.getResources(), firstRecord
            .createIconBitmap(mContext)));
        account1ItemPreference.setSummary(firstRecord.getDisplayName().toString());
        accountPreference.addPreference(account1ItemPreference);
        account1ItemPreference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent firstIt = new Intent(mContext, CellBroadcastGeminiSingleSimSettings.class);
                firstIt.putExtra("slotid", 0);
                mContext.startActivity(firstIt);
                return true;
            }
         });

        Preference account2ItemPreference = new Preference(mContext);
        account2ItemPreference.setKey(KEY_SUB2_SETTING);
        SubscriptionInfo secondRecord = subInfoList.get(1);
        account2ItemPreference.setIcon(new BitmapDrawable(mContext.getResources(), secondRecord
            .createIconBitmap(mContext)));
        account2ItemPreference.setSummary(secondRecord.getDisplayName().toString());
        accountPreference.addPreference(account2ItemPreference);
        account2ItemPreference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent secondIt = new Intent(mContext, CellBroadcastGeminiSingleSimSettings.class);
                secondIt.putExtra("slotid", 1);
                mContext.startActivity(secondIt);
                return true;
            }
         });

        IntentFilter intentFilter =
                new IntentFilter(CellBroadcastConfigService.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mDualSimChangedReceiver, intentFilter);

        Log.d(TAG, "calling Main setting Plugin");
        ICmasMainSettingsExt optAddingAlertVolumeVibration = (ICmasMainSettingsExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
        if (optAddingAlertVolumeVibration != null) {
            if (optAddingAlertVolumeVibration.needToaddAlertSoundVolumeAndVibration()) {
                addAlertSoundVolumeAndVibration();
            }

            optAddingAlertVolumeVibration.activateSpanishAlertOption(
                CellBroadcastGeminiSettingFragment.this);
            Log.d(TAG, "Called Main setting Plugin");
        }

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mContext.unregisterReceiver(mDualSimChangedReceiver);
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");
        stopMediaPlayer();

        AlertTonePreviewDialog alertPreviewDialog = (AlertTonePreviewDialog)
                findPreference("cmas_preview_alert_tone");
        alertPreviewDialog.onDialogClosed(false);
    }

    private BroadcastReceiver mDualSimChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            List<SubscriptionInfo> subscriptionInfoList
                = SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
            if (subscriptionInfoList == null || subscriptionInfoList.size() != 2) {
                getActivity().finish();
            }
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
