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
 * Activity for audio volume voice settings.
 *
 */
public class AudioVolumeVoice extends Activity implements OnClickListener {

    public static final String TAG = "Audio/VolumeVoice";
    private static final String CATEGORY_SPEECHVOL = "SpeechVol";
    private static final String CATEGORY_GAINMAP = "VolumeGainMap";
    private static final String TYPE_BAND = "Band";
    private static final String TYPE_PROFILE = "Profile";
    private static final String TYPE_NETWORK = "Network";
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
    private static final String PARAM_STF_VALUE_MAX = "dec_stf_max";
    private static final String PARAM_STF_STEP = "dec_stf_step_per_db";
    private static final String PARAM_STF_LIST = "stf_gain_field";
    private static final String LIST_DIVIDER = ",";
    private static final String PARAM2_NEW = "Band,%1$s,Profile,%2$s,Network,%3$s";
    private static final String PARAM2 = "Band,%1$s,Profile,%2$s";
    private static final int STF_MAX = 240;
    private static final int STF_MIN = 0;
    private static final int DIALOG_RESULT = 3;
    private static final int DIALOG_INIT_FAIL = 4;

    private Spinner mBandSpinner;
    private Spinner mProfileSpinner;
    private Spinner mNetworkSpinner;
    private EditText mEditUlGain;
    private EditText mEditStfGain;
    private TextView mTableTitleText;
    private TableLayout mTableLayout;
    private View mLayoutText1;
    private View mLayoutText2;
    private Button mBtnSet;
    private ArrayAdapter<String> mBandAdatper;
    private ArrayAdapter<String> mProfileAdatper;
    private String[] mArraySpinnerBand;
    private String[] mArraySpinnerProfile;
    private String[] mArraySpinnerBandValue;
    private String[] mArraySpinnerProfileValue;
    private String mCurrentBand;
    private String mCurrentProfile;
    private String mCurrentNetwork;
    private ArrayList<EditText> mArrayDlText;
    private String[] mArrayDlTable;
    private String mStrErrorInfo;
    private int mUlMaxValue;
    private int mUlMinValue;
    private int mUlMaxIndex;
    private int mUlValueStep;
    private int mStfMaxValue;
    private int mStfMinValue;
    private int mStfMaxIndex;
    private int mStfValueStep;
    private int mStfIndexStep;
    private boolean mStfAscending;
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
        // Init Spinner Band
        mBandSpinner = (Spinner) findViewById(R.id.audio_volume_spinner1);
        String strSpinner1 = AudioTuningJni.getCategory(CATEGORY_SPEECHVOL, TYPE_BAND);
        Elog.d(TAG, "strSpinner1:" + strSpinner1);
        if (strSpinner1 == null) {
            return;
        }
        String[] value = strSpinner1.split(",");
        int length = value.length / 2;
        if (length <= 0) {
            return;
        }

        mArraySpinnerBand = new String[length];
        mArraySpinnerBandValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArraySpinnerBandValue[k] = value[k * 2];
            mArraySpinnerBand[k] = value[k * 2 + 1];
        }

        mBandAdatper = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                mArraySpinnerBand);
        mBandAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBandSpinner.setAdapter(mBandAdatper);
        mCurrentBand = mArraySpinnerBandValue[0];
        mBandSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentBand != mArraySpinnerBandValue[arg2]) {
                    mCurrentBand = mArraySpinnerBandValue[arg2];
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
        String strSpinner2 = AudioTuningJni.getCategory(CATEGORY_SPEECHVOL, TYPE_PROFILE);
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

        initNetworkSpinner();
        mTypeScene.initSceneSpinner(this, CATEGORY_SPEECHVOL);
        // Init Value
        mEditUlGain = (EditText) findViewById(R.id.audio_volume_edittext1);
        mEditStfGain = (EditText) findViewById(R.id.audio_volume_edittext2);
        mTableTitleText = (TextView) findViewById(R.id.table_title);
        mTableLayout = (TableLayout) findViewById(R.id.dl_table);
        initTableValue();
        updateValue();
    }

    private boolean initNetworkSpinner() {

        mNetworkSpinner = (Spinner) findViewById(R.id.audio_volume_spinner3);
        String strSpinner3 = AudioTuningJni.getCategory(CATEGORY_SPEECHVOL, TYPE_NETWORK);
        Elog.d(TAG, "strSpinner3:" + strSpinner3);
        if (strSpinner3 == null) {
            return false;
        }
        String[] value = strSpinner3.split(",");

        int length = value.length / 2;

        if (length <= 0) {
            return false;
        }

        final String[] arraySpinner = new String[length];
        final String[] mArrayValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArrayValue[k] = value[k * 2];
            arraySpinner[k] = value[k * 2 + 1];
        }

        ArrayAdapter<String> adatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, arraySpinner);
        adatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mNetworkSpinner.setAdapter(adatper);
        mCurrentNetwork = mArrayValue[0];
        mNetworkSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentNetwork != mArrayValue[arg2]) {
                    mCurrentNetwork = mArrayValue[arg2];
                    updateValue();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // TODO Auto-generated method stub

            }
        });

        mNetworkSpinner.setVisibility(View.VISIBLE);
        return true;
    }

    private void initTableValue() {
        // Init UL gain table value
        try {
            String strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_UL_VALUE_MAX);
            String strMaxIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_UL_INDEX_MAX);
            String strMinIndex = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_UL_INDEX_MIN);
            String strValueStep = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_UL_STEP);

            mUlMaxValue = Integer.parseInt(strMaxValue);
            mUlMaxIndex = Integer.parseInt(strMaxIndex);
            mUlValueStep = Integer.parseInt(strValueStep);
            int ulMinIndex = Integer.parseInt(strMinIndex);
            mUlMinValue = mUlMaxValue - mUlValueStep * (mUlMaxIndex - ulMinIndex);
            Elog.d(TAG, "Init UL gain table: mUlMaxValue " + mUlMaxValue + " mUlMinValue "
                    + mUlMinValue + " mUlMaxIndex " + mUlMaxIndex + " mUlValueStep "
                    + mUlValueStep);

            // Init Stf gain table value
            strMaxValue = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON,
                    PARAM_STF_VALUE_MAX);
            strValueStep = AudioTuningJni.getParams(CATEGORY_VOLUME, PARAM_COMMON, PARAM_STF_STEP);
            String strIndexList = AudioTuningJni.getChecklist(CATEGORY_SPEECHVOL, STF_GAIN,
                    PARAM_STF_LIST);
            String[] arrayIndex = strIndexList.trim().split(LIST_DIVIDER);
            if (arrayIndex.length >= 3) {
                strMaxIndex = arrayIndex[0];
            } else {
                removeDialog(DIALOG_INIT_FAIL);
                showDialog(DIALOG_INIT_FAIL);
            }
            mStfMaxValue = Integer.parseInt(strMaxValue);
            mStfMaxIndex = Integer.parseInt(strMaxIndex);
            mStfValueStep = Integer.parseInt(strValueStep);
            int secodnIndex = Integer.parseInt(arrayIndex[2]);
            mStfAscending = (mStfMaxIndex < secodnIndex) ? true : false;
            mStfIndexStep = java.lang.Math.abs(mStfMaxIndex - secodnIndex);
            int stfMinIndex = Integer.parseInt(arrayIndex[arrayIndex.length - 2]);
            mStfMinValue = mStfMaxValue - mStfValueStep
                    * java.lang.Math.abs(mStfMaxIndex - stfMinIndex);
            Elog.d(TAG, "Init Stf gain table: mStfMaxValue " + mStfMaxValue + " mStfMinValue "
                    + mStfMinValue + " mStfMaxIndex " + mStfMaxIndex + " mStfValueStep "
                    + mStfValueStep + " mStfIndexStep " + mStfIndexStep);
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(this, "initTableValue Wrong format", Toast.LENGTH_LONG).show();
        }
    }

    private String getParam2() {
        if(mCurrentNetwork == null) {
            return String.format(PARAM2, mCurrentBand, mCurrentProfile);
        } else {
            return String.format(PARAM2_NEW, mCurrentBand, mCurrentProfile, mCurrentNetwork);
        }
    }

    private void updateValue() {
        // Update Ul Gain
        String strValue = AudioTuningJni.getParams(CATEGORY_SPEECHVOL,
                mTypeScene.getPara2String(getParam2()), UL_GAIN);
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

        // Update STF Gain
        strValue = AudioTuningJni.getParams(CATEGORY_SPEECHVOL,
                mTypeScene.getPara2String(getParam2()), STF_GAIN);
        Elog.d(TAG, "STF gain Value:" + strValue);

        try {
            int value = Integer.parseInt(strValue);
            value = mStfMaxValue - mStfValueStep * java.lang.Math.abs(mStfMaxIndex - value);
            if ((value > mStfMaxValue) || (value < mStfMinValue)) {
                mLayoutText2.setVisibility(View.GONE);
            } else {
                mEditStfGain.setText(String.valueOf(value));
                mLayoutText2.setVisibility(View.VISIBLE);
            }
        } catch (NumberFormatException exception) {
            exception.printStackTrace();
            mLayoutText2.setVisibility(View.GONE);
        }

        // Update Dl Gain
        String strDlValueTable = AudioTuningJni.getParams(CATEGORY_GAINMAP, TYPE_PROFILE + ","
                + mCurrentProfile, DL_MAP);
        mArrayDlTable = strDlValueTable.split(",");
        String strDlGain = AudioTuningJni.getParams(CATEGORY_SPEECHVOL,
                mTypeScene.getPara2String(getParam2()), DL_GAIN);
        mArrayDlText.clear();
        mTableLayout.removeAllViews();
        if (strDlGain == null) {
            mTableTitleText.setVisibility(View.GONE);
            return;
        } else {
            mTableTitleText.setVisibility(View.VISIBLE);
        }
        String[] arrayDl = strDlGain.split(",");
        int size = arrayDl.length;
        for (int k = 0; k < size; k++) {
            TableRow row = new TableRow(this);
            TextView textView = new TextView(this);
            EditText editText = new EditText(this);
            textView.setText("Index " + k);
            try {
                int index = Integer.parseInt(arrayDl[k]);
                editText.setText(mArrayDlTable[index]);
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
            setDlGain();
            setUlGain();
            setStfGain();
            if (!AudioTuningJni.saveToWork(CATEGORY_SPEECHVOL)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_save_xml);
            }
            removeDialog(DIALOG_RESULT);
            showDialog(DIALOG_RESULT);
        }
    }

    private void setDlGain() {
        String strValueDlGain = new String();
        for (EditText editText : mArrayDlText) {
            if ((editText == null) || (editText.getText() == null)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_dl_empty);
                return;
            }
            String strValue = editText.getText().toString().trim();
            int size = mArrayDlTable.length;
            int k = 0;
            for (; k < size; k++) {
                if (mArrayDlTable[k].equals(strValue)) {
                    if (strValueDlGain.length() == 0) {
                        strValueDlGain = strValueDlGain + k;
                    } else {
                        strValueDlGain = strValueDlGain + "," + k;
                    }
                    break;
                }
            }

            if (k == size) {
                mStrErrorInfo += strValue + getString(R.string.audio_volume_error_dl_invalid);
                return;
            }
        }
        if (!AudioTuningJni.setParams(CATEGORY_SPEECHVOL,
                mTypeScene.getPara2String(getParam2()), DL_GAIN, strValueDlGain)) {
            mStrErrorInfo += getString(R.string.audio_volume_error_set_dl_gain);
        }
    }

    private void setUlGain() {
        if (mEditUlGain.getText() == null) {
            mStrErrorInfo += getString(R.string.audio_volume_error_ul_gain_format);
            return;
        }
        try {
            String valueUlGain = mEditUlGain.getText().toString();

            int valueUl = Integer.parseInt(valueUlGain);
            if ((valueUl > mUlMaxValue) || (valueUl < mUlMinValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_ul_invalid, mUlMinValue,
                        mUlMaxValue);
                return;
            }
            valueUl = mUlMaxIndex - (mUlMaxValue - valueUl) / mUlValueStep;
            String strValue = String.valueOf(valueUl);
            if (!AudioTuningJni.setParams(CATEGORY_SPEECHVOL,
                    mTypeScene.getPara2String(getParam2()), UL_GAIN, strValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_set_ul_gain);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            mStrErrorInfo += getString(R.string.audio_volume_error_ul_gain_format);
        }
    }

    private void setStfGain() {
        if (mEditStfGain.getText() == null) {
            mStrErrorInfo += getString(R.string.audio_volume_error_stf_gain_format);
            return;
        }
        try {
            String valueStfGain = mEditStfGain.getText().toString();
            int value = Integer.parseInt(valueStfGain);
            if ((value > mStfMaxValue) || (value < mStfMinValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_stf_invalid, mStfMinValue,
                        mStfMaxValue);
                return;
            }

            value = (mStfMaxValue - value) / mStfValueStep; // Get index to
                                                            // minus
            value = value / mStfIndexStep * mStfIndexStep; // Make to be
                                                           // multiple value of
                                                           // index step
            value = mStfAscending ? (mStfMaxIndex + value) : (mStfMaxIndex - value);

            String strValue = String.valueOf(value);
            if (!AudioTuningJni.setParams(CATEGORY_SPEECHVOL,
                    mTypeScene.getPara2String(getParam2()), STF_GAIN, strValue)) {
                mStrErrorInfo += getString(R.string.audio_volume_error_set_stf_gain);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            mStrErrorInfo += getString(R.string.audio_volume_error_stf_gain_format);
        }
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_RESULT:
            if ((mStrErrorInfo != null) && (!mStrErrorInfo.isEmpty())) {
                return new AlertDialog.Builder(AudioVolumeVoice.this)
                        .setTitle(R.string.set_error_title)
                        .setMessage(mStrErrorInfo)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                    }

                                }).create();
            } else {
                return new AlertDialog.Builder(AudioVolumeVoice.this)
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
            return new AlertDialog.Builder(AudioVolumeVoice.this)
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
