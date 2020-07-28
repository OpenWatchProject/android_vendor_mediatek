package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

/**
 * Activity for audio volume record settings.
 *
 */
public class AudioVolumeRecord extends Activity implements OnClickListener {

    public static final String TAG = "Audio/VolumeRecord";
    private static final String CATEGORY_RECORDVOL = "RecordVol";
    private static final String CATEGORY_RECORD = "Record";
    private static final String CATEGORY_GAINMAP = "VolumeGainMap";
    private static final String TYPE_APP = "Application";
    private static final String TYPE_PROFILE = "Profile";
    private static final String UL_GAIN = "ul_gain";
    private static final String STF_GAIN = "stf_gain";
    private static final String DL_GAIN = "dl_gain";
    private static final String DL_MAP = "dl_total_gain_decimal";
    private static final String CATEGORY_VOLUME = "Volume";
    private static final String PARAM_COMMON = "VolumeParam,Common";
    private static final String PARAM_UL_INDEX_MAX = "mic_idx_range_max";
    private static final String PARAM_UL_INDEX_MIN = "mic_idx_range_min";
    private static final String PARAM_UL_VALUE_MAX = "dec_rec_max";
    private static final String PARAM_UL_STEP = "dec_rec_step_per_db";
    private static final int DIALOG_RESULT = 3;
    private Spinner mAppSpinner;
    private Spinner mProfileSpinner;
    private EditText mEditUlGain;
    private View mLayoutText1;
    private TableLayout mTableLayout;
    private Button mBtnSet;
    private ArrayAdapter<String> mAppAdatper;
    private ArrayAdapter<String> mProfileAdatper;
    private String[] mArraySpinnerApp;
    private String[] mArraySpinnerProfile;
    private String[] mArraySpinnerAppValue;
    private String[] mArraySpinnerProfileValue;
    private String mCurrentApp;
    private String mCurrentProfile;
    private String[] mArrayDlTable;
    private String mStrErrorInfo;
    private int mUlMaxValue;
    private int mUlMinValue;
    private int mUlMaxIndex;
    private int mUlValueStep;

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
        mBtnSet = (Button) findViewById(R.id.btn_set);
        mBtnSet.setOnClickListener(this);
        // Init Spinner Band
        mAppSpinner = (Spinner) findViewById(R.id.audio_volume_spinner1);
        String strSpinner1 = AudioTuningJni.getCategory(CATEGORY_RECORDVOL, TYPE_APP);
        Elog.d(TAG, "strSpinner1:" + strSpinner1);
        if (strSpinner1 == null) {
            return;
        }

        String[] value = strSpinner1.split(",");
        int length = value.length / 2;
        if (length <= 0) {
            return;
        }

        mArraySpinnerApp = new String[length];
        mArraySpinnerAppValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArraySpinnerAppValue[k] = value[k * 2];
            mArraySpinnerApp[k] = value[k * 2 + 1];
        }
        mAppAdatper = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                mArraySpinnerApp);
        mAppAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mAppSpinner.setAdapter(mAppAdatper);
        mCurrentApp = mArraySpinnerAppValue[0];
        mAppSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentApp != mArraySpinnerAppValue[arg2]) {
                    mCurrentApp = mArraySpinnerAppValue[arg2];
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
        String strSpinner2 = AudioTuningJni.getCategory(CATEGORY_RECORDVOL, TYPE_PROFILE);
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

        mTypeScene.initSceneSpinner(this, CATEGORY_RECORD);
        // Init Value
        mEditUlGain = (EditText) findViewById(R.id.audio_volume_edittext1);
        View layout2 = findViewById(R.id.layout_text2);
        layout2.setVisibility(View.GONE);
        View tableTitle = findViewById(R.id.table_title);
        tableTitle.setVisibility(View.GONE);
        View table = findViewById(R.id.dl_table);
        table.setVisibility(View.GONE);
        initTableValue();
        updateValue();
    }

    private void initTableValue() {
        // Init UL gain table value
        String strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                PARAM_UL_VALUE_MAX);
        String strMaxIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                PARAM_UL_INDEX_MAX);
        String strMinIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                PARAM_UL_INDEX_MIN);
        String strValueStep = AudioTuningJni
                .getParams(CATEGORY_VOLUME, PARAM_COMMON, PARAM_UL_STEP);

        try {
            mUlMaxValue = Integer.parseInt(strMaxValue);
            mUlMaxIndex = Integer.parseInt(strMaxIndex);
            mUlValueStep = Integer.parseInt(strValueStep);
            int ulMinIndex = Integer.parseInt(strMinIndex);
            mUlMinValue = mUlMaxValue - mUlValueStep * (mUlMaxIndex - ulMinIndex);
            Elog.d(TAG, "Init UL gain table: mUlMaxValue " + mUlMaxValue + " mUlMinValue "
                    + mUlMinValue + " mUlMaxIndex " + mUlMaxIndex + " mUlValueStep " +
                    mUlValueStep);
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(this, "initTableValue wrong format", Toast.LENGTH_LONG).show();
        }

    }

    private void updateValue() {
        // Update Ul Gain
        String strValue = AudioTuningJni.getParams(CATEGORY_RECORDVOL, mTypeScene
                .getPara2String(TYPE_APP + "," + mCurrentApp
                + "," + TYPE_PROFILE + "," + mCurrentProfile), UL_GAIN);
        Elog.d(TAG, "UL gain Value:" + strValue);
        try {
            int value = Integer.parseInt(strValue);
            value = mUlMaxValue - mUlValueStep * (mUlMaxIndex - value);
            if ((value > mUlMaxValue) || (value < mUlMinValue)) {
                mLayoutText1.setVisibility(View.GONE);
            } else {
                mEditUlGain.setText(String.valueOf(value));
                mLayoutText1.setVisibility(View.VISIBLE);
            }
        } catch (NumberFormatException exception) {
            exception.printStackTrace();
            mLayoutText1.setVisibility(View.GONE);
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
            setUlGain();
            removeDialog(DIALOG_RESULT);
            showDialog(DIALOG_RESULT);
        }
    }

    private void setUlGain() {
        if(mEditUlGain == null || mEditUlGain.getText() == null) {
            mStrErrorInfo += getString(R.string.audio_volume_error_ul_gain_format);
            return;
        }
        String valueUlGain = mEditUlGain.getText().toString();
        try {
            int valueUl = Integer.parseInt(valueUlGain);
            if ((valueUl > mUlMaxValue) || (valueUl < mUlMinValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_ul_invalid, mUlMinValue,
                        mUlMaxValue);
                return;

            }
            valueUl -= 72;
            String strValue = String.valueOf(valueUl / 4);
            if (!AudioTuningJni.setParams(CATEGORY_RECORDVOL, mTypeScene.getPara2String(
                    TYPE_APP + "," + mCurrentApp + ","
                    + TYPE_PROFILE + "," + mCurrentProfile), UL_GAIN, strValue)
                    || !AudioTuningJni.saveToWork(CATEGORY_RECORDVOL)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_set_ul_gain);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            mStrErrorInfo += getString(R.string.audio_volume_error_ul_gain_format);
        }

    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_RESULT:
            if ((mStrErrorInfo != null) && (!mStrErrorInfo.isEmpty())) {
                return new AlertDialog.Builder(AudioVolumeRecord.this)
                        .setTitle(R.string.set_error_title)
                        .setMessage(mStrErrorInfo)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                    }

                                }).create();
            } else {
                return new AlertDialog.Builder(AudioVolumeRecord.this)
                        .setTitle(R.string.set_success_title)
                        .setMessage(R.string.audio_volume_set_success)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                    }

                                }).create();
            }
        default:
            return null;
        }
    }

}
