package com.mediatek.engineermode.voicesettings;

import android.app.Activity;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.audio.AudioTuningJni;
import com.mediatek.provider.MtkSettingsExt;

/**
 * Class for VoiceWakeupRecognition.
 *
 */
public class VoiceWakeupRecognition extends Activity implements OnClickListener {
    private static final String TAG = "VoiceWakeupRecognition";
    private static final String CATEGORY = "VOW";
    private static final String TYPE = "VOW,VOW_common";
    private static final String KEY_PRE_KR = "vow_KR";
    private static final String KEY_PRE_KRSR = "vow_KRSR";
    private static final int RECOGNITION_CLEAN = 1;
    private static final int RECOGNITION_NOISY = 2;
    private static final int RECOGNITION_TRAINING = 3;
    private static final int RECOGNITION_TESTING = 4;
    private static final int WAKEUP_MODE_KEYWORD = 1;
    private static final int WAKEUP_MODE_KEYWORD_SPEAKER = 2;
    private static final int WAKEUP_MODE_TRIGGER_KEYWORD = 3;
    private Spinner mSpnClean;
    private Spinner mSpnNoisy;
    private Spinner mSpnTraining;
    private Spinner mSpnTesting;
    private int mWakeupMode = 0;
    private Button mBtnSet;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.voice_common_setting);
        int wakeupMode = Settings.System.getInt(getContentResolver(),
                MtkSettingsExt.System.VOICE_WAKEUP_MODE, 0);
        //Override TriggerMode = Keyword
        mWakeupMode = wakeupMode == WAKEUP_MODE_TRIGGER_KEYWORD ?
            WAKEUP_MODE_KEYWORD : wakeupMode;
        if (mWakeupMode != WAKEUP_MODE_KEYWORD
                && mWakeupMode != WAKEUP_MODE_KEYWORD_SPEAKER) {
            Toast.makeText(this, "Invalid voice wake-up mode:"
                    + mWakeupMode, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        initUiComponent();
    }

    private void initUiComponent() {
        if (mWakeupMode == WAKEUP_MODE_KEYWORD) {
            setTitle(R.string.voice_wakeup_switch_1);
        } else if (mWakeupMode == WAKEUP_MODE_KEYWORD_SPEAKER) {
            setTitle(R.string.voice_wakeup_switch_2);
        }
        mSpnClean = (Spinner) findViewById(R.id.voice_settings_clean_spn);
        mSpnNoisy = (Spinner) findViewById(R.id.voice_settings_noisy_spn);
        mSpnTraining = (Spinner) findViewById(R.id.voice_settings_training_spn);
        mSpnTesting = (Spinner) findViewById(R.id.voice_settings_testing_spn);
        ArrayAdapter<String> adapter;
        adapter = createEmptySpinnerAdapter();
        for (int i = 0; i <= 15; i++) {
            adapter.add(String.valueOf(i));
        }
        mSpnClean.setAdapter(adapter);

        adapter = createEmptySpinnerAdapter();
        for (int i = 0; i <= 15; i++) {
            adapter.add(String.valueOf(i));
        }
        mSpnNoisy.setAdapter(adapter);

        adapter = createEmptySpinnerAdapter();
        for (int i = 0; i <= 7; i++) {
            adapter.add(String.valueOf(i));
        }
        mSpnTraining.setAdapter(adapter);

        adapter = createEmptySpinnerAdapter();
        for (int i = 0; i <= 7; i++) {
            adapter.add(String.valueOf(i));
        }
        mSpnTesting.setAdapter(adapter);

        mBtnSet = (Button) findViewById(R.id.voice_settings_set_btn);
        mBtnSet.setOnClickListener(this);

        initUiByData();
    }

    private void initUiByData() {
        mSpnClean.setSelection(getValue(RECOGNITION_CLEAN));
        mSpnNoisy.setSelection(getValue(RECOGNITION_NOISY));
        mSpnTraining.setSelection(getValue(RECOGNITION_TRAINING));
        mSpnTesting.setSelection(getValue(RECOGNITION_TESTING));
    }

    private int getValue(int index) {
        Elog.i(TAG, "index:" + index);
        String key = (mWakeupMode == WAKEUP_MODE_KEYWORD) ? (KEY_PRE_KR + index)
                : (KEY_PRE_KRSR + index);
        Elog.i(TAG, "key:" + key);
        String result = AudioTuningJni.getParams(CATEGORY, TYPE, key);
        Elog.i(TAG, "result" + result);
        return Integer.parseInt(result);
    }

    private ArrayAdapter<String> createEmptySpinnerAdapter() {
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        return adapter;
    }

    private void setRecognitionSetting(int clean, int noisy, int training, int testing) {
        String strPreKey = (mWakeupMode == WAKEUP_MODE_KEYWORD) ? KEY_PRE_KR : KEY_PRE_KRSR;
        Elog.i(TAG, "setRecognitionSetting:" + strPreKey + " strPreKey,"
            + clean + "," + noisy + "," + training + "," + testing);
        AudioTuningJni.setParams(CATEGORY, TYPE,
                                 strPreKey + RECOGNITION_CLEAN, String.valueOf(clean));
        AudioTuningJni.setParams(CATEGORY, TYPE,
                                 strPreKey + RECOGNITION_NOISY, String.valueOf(noisy));
        AudioTuningJni.setParams(CATEGORY, TYPE,
                                 strPreKey + RECOGNITION_TRAINING, String.valueOf(training));
        AudioTuningJni.setParams(CATEGORY, TYPE,
                                 strPreKey + RECOGNITION_TESTING, String.valueOf(testing));
        AudioTuningJni.saveToWork(CATEGORY);
    }

    private boolean checkSetResult() {
        int clean = mSpnClean.getSelectedItemPosition();
        int noisy = mSpnNoisy.getSelectedItemPosition();
        int training = mSpnTraining.getSelectedItemPosition();
        int testing = mSpnTesting.getSelectedItemPosition();
        int clean2 = getValue(RECOGNITION_CLEAN);
        int noisy2 = getValue(RECOGNITION_NOISY);
        int training2 = getValue(RECOGNITION_TRAINING);
        int testing2 = getValue(RECOGNITION_TESTING);
        if (clean != clean2 || noisy != noisy2 ||
                training != training2 || testing != testing2) {
            return false;
        }
        return true;
    }

    @Override
    public void onClick(View view) {
        if (view == mBtnSet) {
            int clean = mSpnClean.getSelectedItemPosition();
            int noisy = mSpnNoisy.getSelectedItemPosition();
            int training = mSpnTraining.getSelectedItemPosition();
            int testing = mSpnTesting.getSelectedItemPosition();
            setRecognitionSetting(clean, noisy, training, testing);
            int msgid = -1;
            if (checkSetResult()) {
                msgid = R.string.voice_set_success_msg;
            } else {
                msgid = R.string.voice_set_fail_msg;
            }
            Toast.makeText(this, msgid, Toast.LENGTH_SHORT).show();
        }
    }
}
