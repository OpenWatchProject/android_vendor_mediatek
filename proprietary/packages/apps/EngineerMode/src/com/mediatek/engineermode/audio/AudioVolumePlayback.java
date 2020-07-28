package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Activity for volume playback settings.
 *
 */
public class AudioVolumePlayback extends Activity implements OnClickListener {

    public static final String TAG = "Audio/VolumePlayback";
    private static final String CATEGORY_PLAYBACK = "Playback";
    private static final String CATEGORY_PLAYBACK_ANA = "PlaybackVolAna";
    private static final String CATEGORY_PLAYBACK_DIGI = "PlaybackVolDigi";
    private static final String TYPE_VOLUME_TYPE = "Volume type";
    private static final String TYPE_VOLUME_TYPE_RING = "Ring";
    private static final String TYPE_VOLUME_TYPE_ALARM = "Alarm";
    private static final String TYPE_VOLUME_TYPE_RING_ALARM = "Ring_Alarm";
    private static final String TYPE_VOLUME_TYPE_OTHERS = "Others";
    private static final String TYPE_PROFILE = "Profile";

    private static final String HEADSET_PGA = "headset_pga";
    private static final String SPEAKER_PGA = "speaker_pga";
    private static final String DIGI_GAIN = "digital_gain";
    private static final String CATEGORY_VOLUME = "Volume";
    private static final String PARAM_COMMON = "VolumeParam,Common";
    private static final String PARAM_DG_INDEX_MAX = "play_digi_range_max";
    private static final String PARAM_DG_INDEX_MIN = "play_digi_range_min";
    private static final String PARAM_DG_VALUE_MAX = "dec_play_digi_max";
    private static final String PARAM_DG_STEP = "dec_play_digi_step_per_db";
    private static final String PARAM_HS_LIST = "hs_ana_gain";
    private static final String PARAM_HS_VALUE_MAX = "dec_play_hs_max";
    private static final String PARAM_HS_STEP = "dec_play_hs_step_per_db";
    private static final String PARAM_SPK_LIST = "spk_ana_gain";
    private static final String PARAM_SPK_VALUE_MAX = "dec_play_spk_max";
    private static final String PARAM_SPK_STEP = "dec_play_spk_step_per_db";
    private static final String LIST_DIVIDER = ",";

    private static final int SPEAKER_MAX = 180;
    private static final int SPEAKER_NIN = 128;
    private static final int HEADSET_MAX = 160;
    private static final int HEADSET_MIN = 88;
    private static final int DIALOG_RESULT = 3;
    private static final int DIALOG_INIT_FAIL = 4;
    private static final int DGGAIN_MIN = 0;
    private static final int DGGAIN_MAX = 255;
    private static final int SPECIAL_MAX = 256;
    private static final int INVALID_RESULT = -1;
    private Spinner mVolumeTypeSpinner;
    private Spinner mProfileSpinner;
    private EditText mEditHeaset;
    private EditText mEditSpeaker;
    private TextView mTableTitleText;
    private TableLayout mTableLayout;
    private Button mBtnSet;
    private View mLayoutText1;
    private View mLayoutText2;
    private ArrayAdapter<String> mVolumeTypeAdatper;
    private ArrayAdapter<String> mProfileAdatper;
    private String[] mArraySpinnerVolumeType;
    private String[] mArraySpinnerProfile;
    private String[] mArraySpinnerVolumeTypeValue;
    private String[] mArraySpinnerProfileValue;
    private String mCurrentVolumeType;
    private String mCurrentProfile;
    private ArrayList<EditText> mArrayDlText;
    private String mStrErrorInfo;
    private int mDgMaxValue;
    private int mDgMinValue;
    private int mDgMaxIndex;
    private int mDgValueStep;
    private int mHsMaxValue;
    private int mHsMinValue;
    private int mHsMaxIndex;
    private int mHsValueStep;
    private int mHsIndexStep;
    private boolean mHsAscending;
    private int mSpkMaxValue;
    private int mSpkMinValue;
    private int mSpkMaxIndex;
    private int mSpkValueStep;
    private int mSpkIndexStep;
    private boolean mSpkAscending;

    private AudioManager mAudioMgr = null;
    private static final String CUST_XML_PARAM = "GET_CUST_XML_ENABLE";
    private static final String CUST_XML_SET_SUPPORT_PARAM = "SET_CUST_XML_ENABLE=1";
    private static final String CUST_XML_SET_UNSUPPORT_PARAM = "SET_CUST_XML_ENABLE=0";
    private static final String RESULT_SUPPORT = "GET_CUST_XML_ENABLE=1";
    private static final String RESULT_UNSUPPORT = "GET_CUST_XML_ENABLE=0";

    private AudioVolumeTypeScene mTypeScene;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_volume_item);
        mAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        mTypeScene = new AudioVolumeTypeScene( new AudioVolumeTypeScene.Listener(){
            @Override
            public void onSceneChanged(){
                updateValue();
            }
        });

        initComponents();
    }

    private void initComponents() {

        mLayoutText1 = findViewById(R.id.layout_text1);
        mLayoutText2 = findViewById(R.id.layout_text2);
        mArrayDlText = new ArrayList<EditText>();
        mBtnSet = (Button) findViewById(R.id.btn_set);
        mBtnSet.setOnClickListener(this);

        // Init Spinner Volume type
        mVolumeTypeSpinner = (Spinner) findViewById(R.id.audio_volume_spinner1);
        String strSpinner1 = AudioTuningJni.getCategory(CATEGORY_PLAYBACK, TYPE_VOLUME_TYPE);
        Elog.d(TAG, "strSpinner1:" + strSpinner1);
        if (strSpinner1 == null) {
            return;
        }

        String[] value = strSpinner1.split(",");
        int length = value.length / 2;
        if (length <= 0) {
            return;
        }

        mArraySpinnerVolumeType = new String[length];
        mArraySpinnerVolumeTypeValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArraySpinnerVolumeTypeValue[k] = value[k * 2];
            mArraySpinnerVolumeType[k] = value[k * 2 + 1];
        }
        mVolumeTypeAdatper = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                mArraySpinnerVolumeType);
        mVolumeTypeAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVolumeTypeSpinner.setAdapter(mVolumeTypeAdatper);
        mCurrentVolumeType = mArraySpinnerVolumeTypeValue[0];
        mVolumeTypeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentVolumeType != mArraySpinnerVolumeTypeValue[arg2]) {
                    mCurrentVolumeType = mArraySpinnerVolumeTypeValue[arg2];
                    updateValue();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // TODO Auto-generated method stub
            }
        });
        // Init Spinner Profile
        mProfileSpinner = (Spinner) findViewById(R.id.audio_volume_spinner2);
        String strSpinner2 = AudioTuningJni.getCategory(CATEGORY_PLAYBACK, TYPE_PROFILE);
        Elog.d(TAG, "strSpinner2:" + strSpinner2);
        if (strSpinner2 == null) {
            return;
        }

        value = strSpinner2.split(",");
        length = value.length / 2;
        if (length <= 0) {
            return;
        }

        mArraySpinnerProfile = new String[length];
        mArraySpinnerProfileValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArraySpinnerProfileValue[k] = value[k * 2];
            mArraySpinnerProfile[k] = value[k * 2 + 1];
        }
        mProfileAdatper = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                mArraySpinnerProfile);
        mProfileAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mProfileSpinner.setAdapter(mProfileAdatper);
        mCurrentProfile = mArraySpinnerProfileValue[0];
        mProfileSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentProfile != mArraySpinnerProfileValue[arg2]) {
                    mCurrentProfile = mArraySpinnerProfileValue[arg2];
                    updateValue();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // TODO Auto-generated method stub

            }
        });

        mTypeScene.initSceneSpinner(this, CATEGORY_PLAYBACK);
        // Init pga
        TextView heasetText = (TextView) findViewById(R.id.audio_volume_text1);
        heasetText.setText(R.string.audio_volume_heaset_pga);
        mEditHeaset = (EditText) findViewById(R.id.audio_volume_edittext1);
        TextView speakerText = (TextView) findViewById(R.id.audio_volume_text2);
        speakerText.setText(R.string.audio_volume_speaker_pga);
        mEditSpeaker = (EditText) findViewById(R.id.audio_volume_edittext2);
        mTableLayout = (TableLayout) findViewById(R.id.dl_table);

        // Init Digital Gain
        mTableTitleText = (TextView) findViewById(R.id.table_title);
        mTableTitleText.setText(R.string.audio_volume_digital_gain);
        initTableValue();
        updateValue();
    }

    private void initTableValue() {
        // Init Digital gain table value
        try {
            String strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_DG_VALUE_MAX);
            String strMaxIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_DG_INDEX_MAX);
            String strMinIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_DG_INDEX_MIN);
            String strValueStep = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_DG_STEP);
            mDgMaxValue = Integer.parseInt(strMaxValue);
            mDgMaxIndex = Integer.parseInt(strMaxIndex);
            mDgValueStep = Integer.parseInt(strValueStep);
            int dgMinIndex = Integer.parseInt(strMinIndex);
            mDgMinValue = mDgMaxValue - mDgValueStep * (mDgMaxIndex - dgMinIndex);
            Elog.d(TAG, "Init UL gain table: mUlMaxValue " + mDgMaxValue + " mUlMinValue "
                    + mDgMinValue + " mUlMaxIndex " + mDgMaxIndex + " mUlValueStep "
                    + mDgValueStep);

            // Init Headset table value

            strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_HS_VALUE_MAX);
            strValueStep = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON, PARAM_HS_STEP);
            String strIndexList = AudioTuningJni.getChecklist(CATEGORY_PLAYBACK_ANA, HEADSET_PGA,
                    PARAM_HS_LIST);
            String[] arrayIndex = strIndexList.trim().split(LIST_DIVIDER);
            if (arrayIndex.length >= 3) {
                strMaxIndex = arrayIndex[0];
            } else {
                removeDialog(DIALOG_INIT_FAIL);
                showDialog(DIALOG_INIT_FAIL);
                return;
            }
            mHsMaxValue = Integer.parseInt(strMaxValue);
            mHsMaxIndex = Integer.parseInt(strMaxIndex);
            mHsValueStep = Integer.parseInt(strValueStep);
            int secondIndex = Integer.parseInt(arrayIndex[2]);
            mHsAscending = (mHsMaxIndex < secondIndex) ? true : false;
            ;
            mHsIndexStep = java.lang.Math.abs(secondIndex - mHsMaxIndex);
            int stfMinIndex = Integer.parseInt(arrayIndex[arrayIndex.length - 2]);
            mHsMinValue = mHsMaxValue - mHsValueStep
                    * java.lang.Math.abs(stfMinIndex - mHsMaxIndex);
            Elog.d(TAG, "Init Headset table: mHsMaxValue " + mHsMaxValue + " mHsMinValue "
                    + mHsMinValue + " mHsMaxIndex " + mHsMaxIndex + " mHsValueStep " + mHsValueStep
                    + " mHsIndexStep " + mHsIndexStep);

            // Init Speaker gain table value

            strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_SPK_VALUE_MAX);
            strValueStep = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON, PARAM_SPK_STEP);
            strIndexList = AudioTuningJni.getChecklist(CATEGORY_PLAYBACK_ANA, SPEAKER_PGA,
                    PARAM_SPK_LIST);
            arrayIndex = strIndexList.trim().split(LIST_DIVIDER);
            if (arrayIndex.length >= 3) {
                strMaxIndex = arrayIndex[0];
            } else {
                removeDialog(DIALOG_INIT_FAIL);
                showDialog(DIALOG_INIT_FAIL);
                return;
            }
            mSpkMaxValue = Integer.parseInt(strMaxValue);
            mSpkMaxIndex = Integer.parseInt(strMaxIndex);
            mSpkValueStep = Integer.parseInt(strValueStep);
            secondIndex = Integer.parseInt(arrayIndex[2]);
            mSpkAscending = (mSpkMaxIndex < secondIndex) ? true : false;
            ;
            mSpkIndexStep = java.lang.Math.abs(mSpkMaxIndex - secondIndex);
            int spkMinIndex = Integer.parseInt(arrayIndex[arrayIndex.length - 2]);
            mSpkMinValue = mSpkMaxValue - mSpkValueStep
                    * java.lang.Math.abs(mSpkMaxIndex - spkMinIndex);
            Elog.d(TAG, "Init Speaker table: mSpkMaxValue " + mSpkMaxValue + " mSpkMinValue "
                    + mSpkMinValue + " mStfMaxIndex " + mSpkMaxIndex + " mSpkValueStep "
                    + mSpkValueStep + " mSpkIndexStep " + mSpkIndexStep);
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(this, "initTableValue wrong format", Toast.LENGTH_LONG).show();
        }
    }

    private void updateValue() {
        // Update headset

        String valueHeadset;
        String valueSpeaker;
        if ((TYPE_VOLUME_TYPE_RING.equals(mCurrentVolumeType))
                || (TYPE_VOLUME_TYPE_ALARM.equals(mCurrentVolumeType))) {
            valueHeadset = AudioTuningJni.getParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                    .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_RING_ALARM
                                    + "," + TYPE_PROFILE + "," + mCurrentProfile),
                            HEADSET_PGA);
            valueSpeaker = AudioTuningJni.getParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                    .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_RING_ALARM
                                    + "," + TYPE_PROFILE + "," + mCurrentProfile),
                    SPEAKER_PGA);

        } else {
            valueHeadset = AudioTuningJni.getParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                    .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_OTHERS
                                    + "," + TYPE_PROFILE + "," + mCurrentProfile),
                    HEADSET_PGA);
            valueSpeaker = AudioTuningJni.getParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                    .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_OTHERS
                                    + "," + TYPE_PROFILE + "," + mCurrentProfile),
                    SPEAKER_PGA);

        }

        Elog.d(TAG, "valueHeadset:" + valueHeadset + " valueSpeaker: " + valueSpeaker);
        // Update pga
        try {
            int value = Integer.parseInt(valueHeadset);
            if (value == INVALID_RESULT) {
                mLayoutText1.setVisibility(View.GONE);
            } else {
                value = mHsMaxValue - mHsValueStep * java.lang.Math.abs(value - mHsMaxIndex);
                if ((value > mHsMaxValue) || (value < mHsMinValue)) {
                    mLayoutText1.setVisibility(View.GONE);
                } else {
                    mEditHeaset.setText(String.valueOf(value));
                    mLayoutText1.setVisibility(View.VISIBLE);
                }
            }

        } catch (NumberFormatException exception) {
            exception.printStackTrace();
            mLayoutText1.setVisibility(View.GONE);
        }

        try {
            int value = Integer.parseInt(valueSpeaker);
            if (value == INVALID_RESULT) {
                mLayoutText2.setVisibility(View.GONE);
            } else {
                value = mSpkMaxValue - mSpkValueStep * java.lang.Math.abs(mSpkMaxIndex - value);
                if ((value > mSpkMaxValue) || (value < mSpkMinValue)) {
                    mLayoutText2.setVisibility(View.GONE);
                } else {
                    mEditSpeaker.setText(String.valueOf(value));
                    mLayoutText2.setVisibility(View.VISIBLE);
                }
            }

        } catch (NumberFormatException exception) {
            exception.printStackTrace();
            mLayoutText2.setVisibility(View.GONE);
        }

        // Update Digi Gain

        String strDigiGain = AudioTuningJni.getParams(CATEGORY_PLAYBACK_DIGI, mTypeScene
                .getPara2String(TYPE_VOLUME_TYPE + "," + mCurrentVolumeType + ","
                        + TYPE_PROFILE + "," + mCurrentProfile), DIGI_GAIN);
        mArrayDlText.clear();
        mTableLayout.removeAllViews();
        if (strDigiGain == null) {
            mTableTitleText.setVisibility(View.GONE);
            return;
        } else {
            mTableTitleText.setVisibility(View.VISIBLE);
        }
        String[] arrayDigi = strDigiGain.split(",");
        int size = arrayDigi.length;
        for (int k = 0; k < size; k++) {
            TableRow row = new TableRow(this);
            TextView textView = new TextView(this);
            EditText editText = new EditText(this);
            textView.setText("Index " + k);
            try {
                int index = Integer.parseInt(arrayDigi[k]);
                int value = mDgMaxValue - mDgValueStep * (mDgMaxIndex - index);
                if ((index == mDgMaxIndex) && (value == SPECIAL_MAX)) {
                    value = SPECIAL_MAX - 1;
                }
                editText.setText(String.valueOf(value));
            } catch (NumberFormatException exception) {
                exception.printStackTrace();
                continue;
            }
            editText.setInputType(InputType.TYPE_CLASS_NUMBER);
            textView.setPadding(0, 3, 3, 3);
            editText.setPadding(20, 3, 3, 3);
            mArrayDlText.add(editText);
            row.addView(textView);
            row.addView(editText);
            mTableLayout.addView(row);
        }
    }

    @SuppressWarnings("deprecation")
    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        if (v.equals(mBtnSet)) {
            if (!FeatureSupport.isEngLoad()) {
                String check = mAudioMgr.getParameters(CUST_XML_PARAM);
                if ((check != null) && (RESULT_SUPPORT.equals(check))) {
                    Elog.d(TAG, "get CUST_XML_PARAM = 1");
                } else {
                    Elog.d(TAG, "set CUST_XML_PARAM = 1");
                    mAudioMgr.setParameters(CUST_XML_SET_SUPPORT_PARAM);
                    AudioTuningJni.CustXmlEnableChanged(1);
                }
            }
            mStrErrorInfo = new String();
            setDigiGain();
            setSpeakerPga();
            setHeadsetPga();
            removeDialog(DIALOG_RESULT);
            showDialog(DIALOG_RESULT);
        }
    }

    private void setDigiGain() {
        String strValueDigiGain = new String();
        for (EditText editText : mArrayDlText) {
            if ((editText == null) || (editText.getText() == null)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_digi_empty);
                return;
            }
            String strValue = editText.getText().toString().trim();
            try {
                int value = Integer.parseInt(strValue);
                if ((value > mDgMaxValue) || (value < mDgMinValue)) {
                    mStrErrorInfo += getString(R.string.audio_volume_error_digital_gain_invalid,
                            mDgMinValue, mDgMaxValue);
                    return;
                }

                if ((mDgMaxValue == SPECIAL_MAX) && (value == mDgMaxValue)) {
                    value = value + 1;
                }
                int index = mDgMaxIndex - (mDgMaxValue - value) / mDgValueStep;
                if (strValueDigiGain.length() == 0) {
                    strValueDigiGain = strValueDigiGain + index;
                } else {
                    strValueDigiGain = strValueDigiGain + "," + index;
                }

            } catch (NumberFormatException exception) {
                exception.printStackTrace();
                mStrErrorInfo += getString(R.string.audio_volume_error_ditigain_format);
                return;
            }
        }

        if (!AudioTuningJni.setParams(CATEGORY_PLAYBACK_DIGI, mTypeScene
                .getPara2String(TYPE_VOLUME_TYPE + "," + mCurrentVolumeType + ","
                                + TYPE_PROFILE + "," + mCurrentProfile), DIGI_GAIN,
                strValueDigiGain)
                || !AudioTuningJni.saveToWork(CATEGORY_PLAYBACK_DIGI)) {
            mStrErrorInfo += getString(R.string.audio_volume_error_set_ditigain);
        }

    }

    private void setSpeakerPga() {

        if (mLayoutText2.getVisibility() != View.VISIBLE) {
            return;
        }

        if(mEditHeaset == null || mEditHeaset.getText() == null) {
            mStrErrorInfo += getString(R.string.audio_volume_error_headset_invalid,
                    mHsMinValue, mHsMaxValue);
        }

        String valueHeadsetPga = mEditSpeaker.getText().toString();
        try {
            int value = Integer.parseInt(valueHeadsetPga);
            if ((value > mSpkMaxValue) || (value < mSpkMinValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_speaker_invalid,
                        mSpkMinValue, mSpkMaxValue);
                return;
            }

            value = (mSpkMaxValue - value) / mSpkValueStep; // Get index to
                                                            // minus
            value = value / mSpkIndexStep * mSpkIndexStep; // Make to be
                                                           // multiple value of
                                                           // index step
            value = mSpkAscending ? (mSpkMaxIndex + value) : (mSpkMaxIndex - value);

            String strValue = String.valueOf(value);

            boolean setResult = false;
            if ((TYPE_VOLUME_TYPE_RING.equals(mCurrentVolumeType))
                    || (TYPE_VOLUME_TYPE_ALARM.equals(mCurrentVolumeType))) {
                setResult = AudioTuningJni.setParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                        .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_RING_ALARM
                                        + "," + TYPE_PROFILE + "," + mCurrentProfile),
                        SPEAKER_PGA, strValue);

            } else {

                setResult = AudioTuningJni.setParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                        .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_OTHERS
                                        + "," + TYPE_PROFILE + "," + mCurrentProfile),
                        SPEAKER_PGA, strValue);

            }

            if (!setResult || !AudioTuningJni.saveToWork(CATEGORY_PLAYBACK_ANA)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_set_speaker_pga);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            mStrErrorInfo += getString(R.string.audio_volume_error_speaker_format);
            return;
        }
    }

    private void setHeadsetPga() {
        if (mLayoutText1.getVisibility() != View.VISIBLE) {
            return;
        }

        if(mEditHeaset == null || mEditHeaset.getText() == null) {
            mStrErrorInfo += getString(R.string.audio_volume_error_headset_invalid,
                    mHsMinValue, mHsMaxValue);
        }

        String valueHeadsetPga = mEditHeaset.getText().toString();

        try {
            int value = Integer.parseInt(valueHeadsetPga);
            if ((value > mHsMaxValue) || (value < mHsMinValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_headset_invalid,
                        mHsMinValue, mHsMaxValue);
                return;
            }
            value = (mHsMaxValue - value) / mHsValueStep; // Get index to minus
            value = value / mHsIndexStep * mHsIndexStep; // Make to be multiple
                                                         // value of index step
            value = mHsAscending ? (mHsMaxIndex + value) : (mHsMaxIndex - value);
            String strValue = String.valueOf(value);
            boolean setResult = false;
            if ((TYPE_VOLUME_TYPE_RING.equals(mCurrentVolumeType))
                    || (TYPE_VOLUME_TYPE_ALARM.equals(mCurrentVolumeType))) {
                setResult = AudioTuningJni.setParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                        .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_RING_ALARM
                                        + "," + TYPE_PROFILE + "," + mCurrentProfile),
                        HEADSET_PGA, strValue);

            } else {
                setResult = AudioTuningJni.setParams(CATEGORY_PLAYBACK_ANA, mTypeScene
                        .getPara2String(TYPE_VOLUME_TYPE + "," + TYPE_VOLUME_TYPE_OTHERS
                                        + "," + TYPE_PROFILE + "," + mCurrentProfile),
                        HEADSET_PGA, strValue);
            }

            if (!setResult || !AudioTuningJni.saveToWork(CATEGORY_PLAYBACK_ANA)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_set_headset_pga);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            mStrErrorInfo += getString(R.string.audio_volume_error_headset_format);
            return;
        }
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_RESULT:
            if ((mStrErrorInfo != null) && (!mStrErrorInfo.isEmpty())) {
                return new AlertDialog.Builder(AudioVolumePlayback.this)
                        .setTitle(R.string.set_error_title)
                        .setMessage(mStrErrorInfo)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                    }

                                }).create();
            } else {
                return new AlertDialog.Builder(AudioVolumePlayback.this)
                        .setTitle(R.string.set_success_title)
                        .setMessage(R.string.audio_volume_set_success)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                    }

                                }).create();
            }
        case DIALOG_INIT_FAIL:
            return new AlertDialog.Builder(AudioVolumePlayback.this)
                    .setTitle(R.string.init_error_title).setMessage(R.string.init_error_msg)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                        }

                    }).create();
        default:
            return null;
        }
    }

}
