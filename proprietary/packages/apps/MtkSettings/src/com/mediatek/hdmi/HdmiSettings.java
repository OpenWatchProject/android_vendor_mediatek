package com.mediatek.hdmi;

import android.app.Activity;
import android.content.Context;
import android.database.ContentObserver;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserHandle;
import android.os.UEventObserver;
import android.os.Message;
import android.provider.Settings;
import androidx.preference.SwitchPreference;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import android.util.Log;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;
import com.mediatek.provider.MtkSettingsExt;
import android.os.SystemProperties;

import java.util.ArrayList;
import java.util.List;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import android.widget.Toast;

public class HdmiSettings extends SettingsPreferenceFragment implements
        Preference.OnPreferenceChangeListener {
    private static final String TAG = "HDMISettings";
    private static final String KEY_TOGGLE_HDMI = "hdmi_toggler";
    private static final String KEY_VIDEO_RESOLUTION = "video_resolution";
    private static final String KEY_VIDEO_SCALE = "video_scale";
    private static final String KEY_AUDIO_OUTPUT = "audio_output";
    private static final String ACTION_EDID_UPDATE = "mediatek.action.HDMI_EDID_UPDATED";
    private SwitchPreference mToggleHdmiPref;
    private ListPreference mVideoResolutionPref;
    private ListPreference mVideoScalePref;
    private ListPreference mAudioOutputPref;
    private Object mHdmiManager;
    private Activity mActivity;
    private HdmiObserver mHdmiObserver;
    private boolean isPlugIn = false;
    private static final int MSG_HDMI_PLUG_IN = 10;
    private static final int MSG_HDMI_PLUG_OUT = 11;
    private static final int MSG_HDMI_SWITCH_DELAY = 12;
    private static final int HDMI_SWITCH_DELAY_TIME = 500;

    private static final String HDMI_VIDEO_RESOLUTION = "persist.vendor.sys.hdmi_hidl.resolution";
    private static final String HDMI_ENABLE = "persist.vendor.sys.hdmi_hidl.enable";
    private static final String HDMI_VIDEO_AUTO = "persist.vendor.sys.hdmi_hidl.auto";

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_HDMI_PLUG_IN:
                    if(mVideoResolutionPref != null){
                        mVideoResolutionPref.setEnabled(true);
                        updatePref();
                     }
                    break;
                case MSG_HDMI_PLUG_OUT:
                     if(mVideoResolutionPref != null){
                        mVideoResolutionPref.setEnabled(false);
                        updatePref();
                     }
                    break;
                case MSG_HDMI_SWITCH_DELAY:
                    if (mToggleHdmiPref != null){
                        mToggleHdmiPref.setEnabled(true);
                    }
                    updatePrefStatus();
                    break;
                default:
                    super.handleMessage(msg);
                }
            }
        };

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.DISPLAY;
    }

    private ContentObserver mHdmiSettingsObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Log.d("@M_" + TAG, "mHdmiSettingsObserver onChanged: " + selfChange);
            updatePref();
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("@M_" + TAG, "HdmiSettings.onCreate()");
        if (HdimReflectionHelper.HDMI_TB_SUPPORT){
            addPreferencesFromResource(R.xml.hdmi_box_settings);
        } else {
            addPreferencesFromResource(R.xml.hdmi_settings);
        }
        mActivity = getActivity();
        mToggleHdmiPref = (SwitchPreference) findPreference(KEY_TOGGLE_HDMI);
        mToggleHdmiPref.setOnPreferenceChangeListener(this);
        mVideoResolutionPref = (ListPreference) findPreference(KEY_VIDEO_RESOLUTION);
        mVideoResolutionPref.setOnPreferenceChangeListener(this);
        mVideoScalePref = (ListPreference) findPreference(KEY_VIDEO_SCALE);
        mVideoScalePref.setOnPreferenceChangeListener(this);
        CharSequence[] entries = mVideoScalePref.getEntries();
        CharSequence[] values = mVideoScalePref.getEntryValues();
        List<CharSequence> scaleEntries = new ArrayList<CharSequence>();
        for (int i = 0; i < values.length; i++) {
            if (Integer.parseInt(values[i].toString()) != 0) {
                scaleEntries.add(mActivity.getResources().getString(R.string.hdmi_scale_scale_down,
                        values[i]));
            } else {
                scaleEntries.add(mActivity.getResources().getString(R.string.hdmi_scale_no_scale));
            }
        }
        mVideoScalePref.setEntries(scaleEntries.toArray(new CharSequence[scaleEntries.size()]));
        mAudioOutputPref = (ListPreference) findPreference(KEY_AUDIO_OUTPUT);
        mAudioOutputPref.setOnPreferenceChangeListener(this);

        Class contextClass = Context.class;
        mHdmiManager = HdimReflectionHelper.getHdmiService();

        if (HdimReflectionHelper.HDMI_TB_SUPPORT){
            if (null == mHdmiObserver) {
                mHdmiObserver = new HdmiObserver(mActivity.getApplicationContext());
            }
            mHdmiObserver.startObserve();
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        if (mHdmiManager == null) {
            finish();
            return;
        }
        // Always set title because HDMISettings activity title is
        // "Settings" to
        // workaround for recent APP list show "HDMI Settings" when MHL
        // support
        String title = getString(R.string.hdmi_settings);
        String hdmi = getString(R.string.hdmi_replace_hdmi);
        int typeMhl = HdimReflectionHelper.getHdmiDisplayTypeConstant("DISPLAY_TYPE_MHL");
        int typeSlimport = HdimReflectionHelper.getHdmiDisplayTypeConstant("DISPLAY_TYPE_SLIMPORT");
        int displayType = HdimReflectionHelper.getHdmiDisplayType(mHdmiManager);
        if (displayType == typeMhl) {
            String mhl = getString(R.string.hdmi_replace_mhl);
            mActivity.setTitle(title.replaceAll(hdmi, mhl));
            mToggleHdmiPref.setTitle(mToggleHdmiPref.getTitle().toString().replaceAll(hdmi, mhl));
        } else if (displayType == typeSlimport) {
            String slimport = getString(R.string.slimport_replace_hdmi);
            mActivity.setTitle(title.replaceAll(hdmi, slimport));
            mToggleHdmiPref.setTitle(mToggleHdmiPref.getTitle().toString().replaceAll(hdmi,
                    slimport));
        } else {
            mActivity.setTitle(title);
        }
        if (!HdimReflectionHelper.hasCapability(mHdmiManager)) {
            Log.d("@M_" + TAG, "remove mVideoScalePref");
            getPreferenceScreen().removePreference(mVideoScalePref);
        }

        if (HdimReflectionHelper.getAudioParameter(mHdmiManager) <= 2) {
            Log.d("@M_" + TAG, "remove mAudioOutputPref");
            getPreferenceScreen().removePreference(mAudioOutputPref);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updatePref();
        mActivity.getContentResolver().registerContentObserver(
                Settings.System.getUriFor(MtkSettingsExt.System.HDMI_ENABLE_STATUS), false,
                mHdmiSettingsObserver);
        mActivity.getContentResolver().registerContentObserver(
                Settings.System.getUriFor(MtkSettingsExt.System.HDMI_CABLE_PLUGGED), false,
                mHdmiSettingsObserver);
    }

    @Override
    public void onPause() {
        mActivity.getContentResolver().unregisterContentObserver(mHdmiSettingsObserver);
        super.onPause();
    };

    @Override
    public void onDestroy() {
        if (null != mHdmiObserver) {
            mHdmiObserver.stopObserve();
        }
        super.onDestroy();
    };

    private void updatePref() {
        Log.i("@M_" + TAG, "updatePref");
        updatePrefStatus();
        updateSelectedResolution();
        updateSelectedScale();
        updateSelectedAudioOutput();
    }

    private void updatePrefStatus() {
        Log.i("@M_" + TAG, "updatePrefStatus");
        boolean shouldEnable = false;
        shouldEnable = HdimReflectionHelper.isSignalOutputting(mHdmiManager);
        Log.i("@M_" + TAG, "updatePrefStatus, shouldEnable = " + shouldEnable);
        if (HdimReflectionHelper.HDMI_TB_SUPPORT){
            mVideoResolutionPref.setEnabled(isPlugIn);
        } else {
            mVideoResolutionPref.setEnabled(shouldEnable);
        }
        mVideoScalePref.setEnabled(shouldEnable);
        boolean hdmiEnabled = Settings.System.getInt(mActivity.getContentResolver(),
                MtkSettingsExt.System.HDMI_ENABLE_STATUS, 1) == 1;
        if (HdimReflectionHelper.HDMI_TB_SUPPORT){
            hdmiEnabled = (SystemProperties.getInt(HDMI_ENABLE, 0) == 1);
        }
        mToggleHdmiPref.setChecked(hdmiEnabled);
    }

    private void updateSelectedResolution() {
        Log.i("@M_" + TAG, "updateSelectedResolution");
        int hdmiAuto = HdimReflectionHelper.getHdmiDisplayTypeConstant("AUTO");
        int videoResolution = Settings.System.getInt(mActivity.getContentResolver(),
                MtkSettingsExt.System.HDMI_VIDEO_RESOLUTION, hdmiAuto);
        int autoVideoResolution = 0;
        if (HdimReflectionHelper.HDMI_TB_SUPPORT){
            videoResolution = SystemProperties.getInt(HDMI_VIDEO_RESOLUTION,13);
            autoVideoResolution = SystemProperties.getInt(HDMI_VIDEO_AUTO,13);
        }

        if (videoResolution > hdmiAuto || autoVideoResolution == 1) {
            videoResolution = hdmiAuto;
        }
        int[] supportedResolutions = { hdmiAuto };
        supportedResolutions = HdimReflectionHelper.getSupportedResolutions(mHdmiManager);
        CharSequence[] resolutionEntries = mActivity.getResources().getStringArray(
                HdimReflectionHelper.HDMI_TB_SUPPORT ? R.array.hdmi_box_video_resolution_entries :
                R.array.hdmi_video_resolution_entries);
        List<CharSequence> realResolutionEntries = new ArrayList<CharSequence>();
        List<CharSequence> realResolutionValues = new ArrayList<CharSequence>();
        realResolutionEntries.add(mActivity.getResources().getString(R.string.hdmi_auto));
        realResolutionValues.add(Integer.toString(hdmiAuto));
        for (int resolution : supportedResolutions) {
            try {
                realResolutionEntries.add(resolutionEntries[resolution]);
                realResolutionValues.add(Integer.toString(resolution));
            } catch (ArrayIndexOutOfBoundsException e) {
                Log.d("@M_" + TAG, e.getMessage());
            }
        }
        mVideoResolutionPref.setEntries((CharSequence[]) realResolutionEntries
                .toArray(new CharSequence[realResolutionEntries.size()]));
        mVideoResolutionPref.setEntryValues((CharSequence[]) realResolutionValues
                .toArray(new CharSequence[realResolutionValues.size()]));
        mVideoResolutionPref.setValue(Integer.toString(videoResolution));
    }

    private void updateSelectedScale() {
        Log.i("@M_" + TAG, "updateSelectedScale");
        int videoScale = Settings.System.getInt(mActivity.getContentResolver(),
                MtkSettingsExt.System.HDMI_VIDEO_SCALE, 0);
        mVideoScalePref.setValue(Integer.toString(videoScale));
    }

    private void updateSelectedAudioOutput() {
        Log.i("@M_" + TAG, "updateSelectedAudioOutput");
            mAudioOutputPref.setEnabled(HdimReflectionHelper.isSignalOutputting(mHdmiManager));
        int audioOutputMode = Settings.System.getIntForUser(mActivity.getContentResolver(),
                MtkSettingsExt.System.HDMI_AUDIO_OUTPUT_MODE, 0, UserHandle.USER_CURRENT);
        mAudioOutputPref.setValue(Integer.toString(audioOutputMode));
        Log.i("@M_" + TAG, "updateSelectedAudioOutput audioOutputMode: " + audioOutputMode);
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String key = preference.getKey();
        Log.d("@M_" + TAG, key + " preference changed");
        if (KEY_TOGGLE_HDMI.equals(key)) {
            boolean checked = ((Boolean) newValue).booleanValue();
            boolean wifi_display_on = Settings.Global.getInt(getContentResolver(),
                Settings.Global.WIFI_DISPLAY_ON, 0) != 0;
            if (wifi_display_on && checked){
                Toast.makeText(mActivity.getApplicationContext(), R.string.hdmi_wfd_on_hint, Toast.LENGTH_SHORT).show();
            } else {
                HdimReflectionHelper.enableHdmi(mHdmiManager, checked);
            }
            mToggleHdmiPref.setEnabled(false);
            mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_HDMI_SWITCH_DELAY),
                HDMI_SWITCH_DELAY_TIME);
        } else if (KEY_VIDEO_RESOLUTION.equals(key)) {
            if (HdimReflectionHelper.HDMI_TB_SUPPORT){
                if (Integer.parseInt((String) newValue) ==
                    HdimReflectionHelper.getHdmiDisplayTypeConstant("AUTO")) {
                    HdimReflectionHelper.setAutoMode(mHdmiManager, true);
                } else {
                    HdimReflectionHelper.setAutoMode(mHdmiManager, false);
                }
            }
            HdimReflectionHelper.setVideoResolution(mHdmiManager, Integer.parseInt((String) newValue));
        } else if (KEY_VIDEO_SCALE.equals(key)) {
            int scaleValue = Integer.parseInt((String) newValue);
            if (scaleValue >= 0 && scaleValue <= 10) {
                HdimReflectionHelper.setVideoScale(mHdmiManager, scaleValue);
            } else {
                Log.d("@M_" + TAG, "scaleValue error: " + scaleValue);
            }
        } else if (KEY_AUDIO_OUTPUT.equals(key)) {
            int which = Integer.parseInt((String) newValue);
            int maxChannel = HdimReflectionHelper.getHdmiDisplayTypeConstant("AUDIO_OUTPUT_STEREO");
            if (which == 1) {
                maxChannel = HdimReflectionHelper.getAudioParameter(mHdmiManager);
            }
            AudioSystem.setParameters("HDMI_channel=" + maxChannel);
            Settings.System.putIntForUser(mActivity.getContentResolver(),
                    MtkSettingsExt.System.HDMI_AUDIO_OUTPUT_MODE, which, UserHandle.USER_CURRENT);
            Log.d("@M_" + TAG, "AudioSystem.setParameters HDMI_channel = " + maxChannel
                    + ",which: " + which);
        }
        return true;
    }

    private class HdmiObserver extends UEventObserver {
        private static final String TAG = "HdmiReceiver.HdmiObserver";
        private static final String HDMI_UEVENT_MATCH = "DEVPATH=/devices/virtual/switch/hdmi";
        private static final String HDMI_STATE_PATH = "/sys/class/switch/hdmi/state";
        private final Context mContext;
        public HdmiObserver(Context context) {
            mContext = context;
            init();
        }

        public void startObserve() {
            startObserving(HDMI_UEVENT_MATCH);
        }

        public void stopObserve() {
            stopObserving();
        }

        @Override
        public void onUEvent(UEventObserver.UEvent event) {
            int state = 0;
            try {
                state = Integer.parseInt(event.get("SWITCH_STATE"));
            } catch (NumberFormatException e) {
                Log.w(TAG,
                        "HdmiObserver: Could not parse switch state from event "
                                + event);
            }
            update(state);
        }

        private synchronized void init() {
            int newState;
            try {
                newState = Integer
                        .parseInt(getContentFromFile(HDMI_STATE_PATH));
            } catch (NumberFormatException e) {
                Log.w(TAG, "HDMI state fail");
                return;
            }
            update(newState);
        }

        private String getContentFromFile(String filePath) {
            char[] buffer = new char[1024];
            FileReader reader = null;
            String content = null;
            try {
                reader = new FileReader(filePath);
                int len = reader.read(buffer, 0, buffer.length);
                content = String.valueOf(buffer, 0, len).trim();
                Log.d(TAG, filePath + " content is " + content);
            } catch (FileNotFoundException e) {
                Log.w(TAG, "can't find file " + filePath);
            } catch (IOException e) {
                Log.w(TAG, "IO exception when read file " + filePath);
            } catch (IndexOutOfBoundsException e) {
                Log.w(TAG, "index exception: " + e.getMessage());
            } finally {
                if (null != reader) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        Log.w(TAG, "close reader fail: " + e.getMessage());
                    }
                }
            }
            return content;
        }

        private synchronized void update(int newState) {
            if(mVideoResolutionPref != null){
                if (newState == 0){
                   isPlugIn = false;
                   mHandler.sendEmptyMessage(MSG_HDMI_PLUG_OUT);
                } else {
                   isPlugIn = true;
                   mHandler.sendEmptyMessage(MSG_HDMI_PLUG_IN);
                }
            }
        }
    }
}
