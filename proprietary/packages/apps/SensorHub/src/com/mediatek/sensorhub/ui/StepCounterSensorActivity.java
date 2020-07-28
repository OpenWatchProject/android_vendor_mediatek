/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.sensorhub.ui;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.hardware.Sensor;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.support.v4.content.LocalBroadcastManager;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.View.MeasureSpec;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.sensorhub.settings.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.List;
import java.util.Locale;

public class StepCounterSensorActivity extends PreferenceActivity implements OnClickListener,
        CompoundButton.OnCheckedChangeListener {

    private static final String TAG = "SH/StepCountSensorActivity";
    public static final String KEY_TOTAL_STEPS_PREFER = "total_steps";
    public static final String KEY_START_STEPS_PREFER = "start_step";
    private Preference mTotalStepsPref;
    private Preference mStartStepsPref;
    private Button mClearButton;
    private float mPreToatalSteps;

    private static final String PEDOMETER_LOG_SRC_DIR = "/MtkSensorTest";
    private static final String PEDOMETER_LOG_DEST_DIR = "/Pedometer_Log";
    private static final String PEDOMETER_LOG_FILE_PREFIX = "sensor";

    private Context mContext;
    private Resources mResources;
    private Handler mTimerHandler;

    private Switch mActionBarSwitch;
    private EditText mEdtCompetitorStartStepDefined;
    private Spinner mSpnSubCategory;
    private Spinner mSpnPlacement;
    private ListView mLvLoggerFiles;
    private TextView mTvStepNum1;
    private TextView mTvStepNum2;

    private int mIdxCategory = 0;
    private int mIdxSubCategory = 0;
    private int mIdxPlacement = 0;
    private int mIdxPersonSex = 0;
    private int mIdxPersonNum1 = 0;
    private int mIdxPersonNum2 = 0;
    private int mIdxDevice = 0;

    private AtomicBoolean mIsNaming;
    private AtomicBoolean mIsLogging;

    private int mCompetitorStartStep = -1;
    private int mMtkSteps;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.sensor_step_counter_pref);
        initializeAllPreferences();

        mContext = this;
        mResources = getResources();
        initUI();
        initActionButton();
        initAtomicBoolean();
        initTimer();
    }

    @Override
    protected void onResume() {
        super.onResume();
        updatePreferenceStatus();
    }

    @Override
    protected void onPause() {
//        float totalStepCounter = 0;
//        try {
//            totalStepCounter = Float.valueOf(findPreference("0").getSummary().toString());
//        } catch (NullPointerException e) {
//            Log.d(TAG, "onPause totalFloorCount : " + totalStepCounter);
//        }
//        Utils.setSensorValues(Utils.KEY_TOTAL_STEP_COUNTER, totalStepCounter);
        super.onPause();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }

    private void initializeAllPreferences() {
        mTotalStepsPref = findPreference(KEY_TOTAL_STEPS_PREFER);
        mStartStepsPref = findPreference(KEY_START_STEPS_PREFER);
        PedometerLayoutPreference pedometerLayout =
            (PedometerLayoutPreference) findPreference("pedometer_set_view");
        pedometerLayout.findViewById(R.id.step_length_value).setVisibility(View.GONE);
        pedometerLayout.findViewById(R.id.step_length_title).setVisibility(View.GONE);
        pedometerLayout.findViewById(R.id.set_step).setVisibility(View.GONE);
        mClearButton = (Button) pedometerLayout.findViewById(R.id.clear_step);
        mClearButton.setOnClickListener(this);
    }

    private void updatePreferenceStatus() {
        mPreToatalSteps = Utils.getSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER);
        float totalStepCounter = Utils.getSensorValues(Utils.KEY_TOTAL_STEP_COUNTER);
        findPreference("0").setSummary(String.valueOf(totalStepCounter));
        mTotalStepsPref.setSummary(String.valueOf(totalStepCounter - mPreToatalSteps));
        mStartStepsPref.setSummary(String.valueOf(mPreToatalSteps));
        if (mTotalStepsPref.getSummary() != null
                && !TextUtils.isEmpty(mTotalStepsPref.getSummary())) {
            mClearButton.setEnabled(true);
        } else {
            mClearButton.setEnabled(false);
        }

        // Note: Update step num
        updateStepNum();
    }

    @Override
    public void onClick(View v) {
        Button buttonViewButton = (Button) v;
        Log.d(TAG, "onclick: " + buttonViewButton.getText());
        if (v == mClearButton) {
            clear();
        }
    }

    private void clear() {
        if (mTotalStepsPref.getSummary() != null
                && !TextUtils.isEmpty(mTotalStepsPref.getSummary())) {
            mTotalStepsPref.setSummary(R.string.init_number_str);
        }
        try {
            mPreToatalSteps = Float.valueOf(findPreference("0").getSummary().toString());
        } catch (NullPointerException e) {
            Log.d(TAG, "clear mPreToatalSteps : " + mPreToatalSteps);
        }
        mStartStepsPref.setSummary(String.valueOf(mPreToatalSteps));
        Utils.setSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER, mPreToatalSteps);

        // Note: Update step num
        updateStepNum();
    }

    public void onSensorChanged(float[] value) {
        // Log.d(TAG, "onSensorChanged value is: " + value[0]);
        for (int i = 0; i < value.length; i++) {
            Preference preference = findPreference(String.valueOf(i));
            if (preference != null) {
                preference.setSummary(String.valueOf(value[i]));
            }
        }
        if (value[0] < mPreToatalSteps) {
            mPreToatalSteps = 0;
            mStartStepsPref.setSummary(String.valueOf(mPreToatalSteps));
            Utils.setSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER, mPreToatalSteps);
        }
        mTotalStepsPref.setSummary(String.valueOf(value[0] - mPreToatalSteps));
        mClearButton.setEnabled(true);

        // Note: Update step num
        updateStepNum();
    }

    private void initUI() {
        PedometerLayoutPreference pedometerLayout =
            (PedometerLayoutPreference) findPreference("pedometer_set_view");

        ArrayAdapter<CharSequence> adapter = ArrayAdapter
            .createFromResource(
                mContext, R.array.pedometer_category, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Spinner spnCategory = (Spinner) pedometerLayout
            .findViewById(R.id.spn_pedometer_category);
        if (spnCategory != null) {
            spnCategory.setAdapter(adapter);
            spnCategory.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxCategory = position;

                    // Update subCategory
                    String[] subCategory = mResources
                        .getStringArray(R.array.pedometer_sub_category);
                    String[] parseSubCategory = subCategory[mIdxCategory].split(",");
                    if (mSpnSubCategory != null) {
                        ArrayAdapter<String> strArrayAdapter = new ArrayAdapter<>(
                            mContext, android.R.layout.simple_spinner_item, parseSubCategory);
                        strArrayAdapter.setDropDownViewResource(
                            android.R.layout.simple_spinner_dropdown_item);
                        mSpnSubCategory.setAdapter(strArrayAdapter);
                        mIdxSubCategory = 0;
                    }
                    // Update placement
                    String[] placement = mResources
                        .getStringArray(R.array.pedometer_placement);
                    String[] parsePlacement = placement[getPlacementID(mIdxSubCategory)]
                        .split(",");
                    if (mSpnPlacement != null) {
                        ArrayAdapter<String> strArrayAdapter = new ArrayAdapter<>(
                            mContext, android.R.layout.simple_spinner_item, parsePlacement);
                        strArrayAdapter.setDropDownViewResource(
                            android.R.layout.simple_spinner_dropdown_item);
                        mSpnPlacement.setAdapter(strArrayAdapter);
                        mIdxPlacement = 0;
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {
                }
            });
        }

        String[] subCategory = mResources.getStringArray(R.array.pedometer_sub_category);
        String[] parseSubCategory = subCategory[mIdxCategory].split(",");
        ArrayAdapter<String> strArrayAdapter = new ArrayAdapter<>(
            mContext, android.R.layout.simple_spinner_item, parseSubCategory);
        strArrayAdapter.setDropDownViewResource(
            android.R.layout.simple_spinner_dropdown_item);
        mSpnSubCategory = (Spinner) pedometerLayout
            .findViewById(R.id.spn_pedometer_sub_category);
        if (mSpnSubCategory != null) {
            mSpnSubCategory.setAdapter(strArrayAdapter);
            mSpnSubCategory.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    int prevMIdxSubCategory = mIdxSubCategory;
                    mIdxSubCategory = position;

                    // Update placement
                    String[] placement = mResources
                        .getStringArray(R.array.pedometer_placement);
                    String[] prevParsePlacement =
                        placement[getPlacementID(prevMIdxSubCategory)].split(",");
                    String[] parsePlacement =
                        placement[getPlacementID(mIdxSubCategory)].split(",");
                    if (mSpnPlacement != null) {
                        ArrayAdapter<String> strArrayAdapter = new ArrayAdapter<>(
                            mContext, android.R.layout.simple_spinner_item, parsePlacement);
                        strArrayAdapter.setDropDownViewResource(
                            android.R.layout.simple_spinner_dropdown_item);
                        mSpnPlacement.setAdapter(strArrayAdapter);

                        String prevPlacement = prevParsePlacement[mIdxPlacement];
                        mIdxPlacement = 0;
                        for (int i = 0; i < parsePlacement.length; i++) {
                            if (prevPlacement.equals(parsePlacement[i])) {
                                mIdxPlacement = i;
                                break;
                            }
                        }

                        mSpnPlacement.setSelection(mIdxPlacement);
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        String[] placement = mResources.getStringArray(R.array.pedometer_placement);
        String[] parsePlacement = placement[getPlacementID(mIdxSubCategory)].split(",");
        strArrayAdapter = new ArrayAdapter<>(
            mContext, android.R.layout.simple_spinner_item, parsePlacement);
        strArrayAdapter.setDropDownViewResource(
            android.R.layout.simple_spinner_dropdown_item);
        mSpnPlacement = (Spinner) pedometerLayout.findViewById(R.id.spn_pedometer_placement);
        if (mSpnPlacement != null) {
            mSpnPlacement.setAdapter(strArrayAdapter);
            mSpnPlacement.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxPlacement = position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        adapter = ArrayAdapter
            .createFromResource(
                this, R.array.pedometer_person_sex, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Spinner spnPersonSex = (Spinner) pedometerLayout
            .findViewById(R.id.spn_pedometer_person_sex);
        if (spnPersonSex != null) {
            spnPersonSex.setAdapter(adapter);
            spnPersonSex.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxPersonSex = position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        adapter = ArrayAdapter
            .createFromResource(
                this, R.array.pedometer_person_num, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Spinner spnPersonNum1 = (Spinner) pedometerLayout
            .findViewById(R.id.spn_pedometer_person_num1);
        if (spnPersonNum1 != null) {
            spnPersonNum1.setAdapter(adapter);
            spnPersonNum1.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxPersonNum1 = position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        adapter = ArrayAdapter
            .createFromResource(
                this, R.array.pedometer_person_num, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Spinner spnPersonNum2 = (Spinner) pedometerLayout
            .findViewById(R.id.spn_pedometer_person_num2);
        if (spnPersonNum2 != null) {
            spnPersonNum2.setAdapter(adapter);
            spnPersonNum2.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxPersonNum2 = position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        Spinner spnDevice = (Spinner) pedometerLayout.findViewById(R.id.spn_pedometer_device);
        if (spnDevice != null) {
            spnDevice.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
                @Override
                public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                    mIdxDevice = position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });
        }

        mLvLoggerFiles = (ListView) pedometerLayout.findViewById(R.id.lv_pedometer_file);
        ArrayAdapter<String> arrayAdapter = new ArrayAdapter<>(
                this,
                android.R.layout.simple_list_item_1,
                getFileList());

        if (mLvLoggerFiles != null) {
            mLvLoggerFiles.setAdapter(arrayAdapter);
            setListViewHeightBasedOnChildren(mLvLoggerFiles);
            mLvLoggerFiles.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(
                    AdapterView<?> parent, View view, int position, long id) {
                    if (mIsLogging != null && mIsLogging.get()) {
                        Toast.makeText(
                            StepCounterSensorActivity.this, "Please stop logging first.",
                                Toast.LENGTH_SHORT).show();
                    } else if (mIsNaming != null) {
                        mIsNaming.set(true);
                        parseFileStr((String) parent.getItemAtPosition(position));
                    }
                }
            });
        }

        mEdtCompetitorStartStepDefined = (EditText) pedometerLayout
            .findViewById(R.id.edt_pedometer_competitor_start);

        mTvStepNum1 = (TextView) pedometerLayout.findViewById(R.id.tv_pedometer_step_num_1);
        mTvStepNum1.setText(String.format(
            mResources.getString(R.string.pedometer_step_num_1), "?"));
        mTvStepNum2 = (TextView) pedometerLayout.findViewById(R.id.tv_pedometer_step_num_2);
        mTvStepNum2.setText(String.format(
            mResources.getString(R.string.pedometer_step_num_2), "?"));
    }

    private int getPlacementID(int subCategoryIdx) {
        String[] subCategory = mResources.getStringArray(R.array.pedometer_sub_category);

        int ans = 0;
        for (int i = 0; i < mIdxCategory; i++) {
            ans += subCategory[i].split(",").length;
        }
        return ans + subCategoryIdx;
    }

    private void initActionButton() {
        mActionBarSwitch = new Switch(getLayoutInflater().getContext());
        final int padding = getResources()
            .getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        mActionBarSwitch.setPaddingRelative(0, 0, padding, 0);
        getActionBar().setDisplayOptions(
                ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar()
                .setCustomView(
                        mActionBarSwitch,
                        new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                                        | Gravity.END));
        getActionBar().setDisplayHomeAsUpEnabled(false);
        mActionBarSwitch.setChecked(Utils.getSensorStatus(Utils.LOG_STATUS));
        mActionBarSwitch.setOnCheckedChangeListener(this);
    }

    private void initAtomicBoolean() {
        mIsNaming = new AtomicBoolean(false);
        mIsLogging = new AtomicBoolean(false);
    }

    private void initTimer() {
        mTimerHandler = new Handler();
        Runnable timerRunnable = new Runnable() {
            @Override
            public void run() {
                if (mIsNaming != null && !mIsNaming.get()) {
                    ArrayAdapter<String> arrayAdapter = new ArrayAdapter<>(
                            StepCounterSensorActivity.this,
                            android.R.layout.simple_list_item_1,
                            getFileList());

                    if (mLvLoggerFiles != null) {
                        mLvLoggerFiles.setAdapter(arrayAdapter);
                        setListViewHeightBasedOnChildren(mLvLoggerFiles);
                    }
                }

                mTimerHandler.postDelayed(this, 1000);
            }
        };

        mTimerHandler.postDelayed(timerRunnable, 0);
    }

    private void updateStepNum() {
        float totalStepCounter = Utils.getSensorValues(Utils.KEY_TOTAL_STEP_COUNTER);
        mMtkSteps = (int) (totalStepCounter - mPreToatalSteps);
        mTvStepNum1.setText(String.format(mResources.getString(R.string.pedometer_step_num_1),
                String.valueOf(mMtkSteps)));
        mTvStepNum2.setText(String.format(mResources.getString(R.string.pedometer_step_num_2),
                String.valueOf((int)totalStepCounter)));
    }

    private boolean isParsable(String input) {
        boolean parsable = true;
        try {
            Integer.parseInt(input);
        } catch (NumberFormatException e) {
            parsable = false;
        }
        return parsable;
    }

    private void parseFileStr(String str) {
        AlertDialog.Builder altDlgConfirm = new AlertDialog
            .Builder(StepCounterSensorActivity.this);
        altDlgConfirm.setTitle("Log file");
        altDlgConfirm.setMessage("Please input Step #.");
        altDlgConfirm.setCancelable(false);

        final LinearLayout layout = new LinearLayout(StepCounterSensorActivity.this);
        layout.setOrientation(LinearLayout.VERTICAL);

        final EditText edtTrueStep = new EditText(StepCounterSensorActivity.this);
        edtTrueStep.setInputType(InputType.TYPE_CLASS_NUMBER);
        edtTrueStep.setHint("Ground Truth #");
        layout.addView(edtTrueStep);

        LinearLayout layoutCompetitor = new LinearLayout(StepCounterSensorActivity.this);
        layoutCompetitor.setOrientation(LinearLayout.HORIZONTAL);

        final EditText edtCompetitorStartStep = new EditText(StepCounterSensorActivity.this);
        edtCompetitorStartStep.setInputType(InputType.TYPE_CLASS_NUMBER);
        edtCompetitorStartStep.setHint("Ref phone start #");
        if (mEdtCompetitorStartStepDefined != null
            && !mEdtCompetitorStartStepDefined.getText().toString().matches("")
            && isParsable(mEdtCompetitorStartStepDefined.getText().toString())) {
                edtCompetitorStartStep.setText(String.format(
                    Locale.getDefault(), "%d",
                        Integer.parseInt(mEdtCompetitorStartStepDefined.getText().toString())));
        } else if (mCompetitorStartStep >= 0) {
            edtCompetitorStartStep.setText(String.format(
                Locale.getDefault(), "%d", mCompetitorStartStep));
        }
        layoutCompetitor.addView(edtCompetitorStartStep);

        final EditText edtCompetitorStopStep = new EditText(StepCounterSensorActivity.this);
        edtCompetitorStopStep.setInputType(InputType.TYPE_CLASS_NUMBER);
        edtCompetitorStopStep.setHint("Ref phone stop #");
        layoutCompetitor.addView(edtCompetitorStopStep);

        LinearLayout layoutMTK = new LinearLayout(StepCounterSensorActivity.this);
        layoutMTK.setOrientation(LinearLayout.HORIZONTAL);

        final EditText edtCompetitor2Step = new EditText(StepCounterSensorActivity.this);
        edtCompetitor2Step.setInputType(InputType.TYPE_CLASS_NUMBER);
        edtCompetitor2Step.setHint("Reversed #");
        layoutMTK.addView(edtCompetitor2Step);

        final EditText edtMTKStep = new EditText(StepCounterSensorActivity.this);
        edtMTKStep.setInputType(InputType.TYPE_CLASS_NUMBER);
        edtMTKStep.setHint("MTK #");
        edtMTKStep.setText(String.format(Locale.getDefault(), "%d", mMtkSteps));
        layoutMTK.addView(edtMTKStep);

        layout.addView(layoutCompetitor);
        layout.addView(layoutMTK);
        altDlgConfirm.setView(layout);

        String tmp;
        if (str.split(",").length > 1) tmp = str.split(",")[0];
        else tmp = str;
        final String prevStr = tmp;

        altDlgConfirm.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                String[] strings = mResources.getStringArray(R.array.pedometer_category);
                String category = strings[mIdxCategory];
                strings = mResources.getStringArray(R.array.pedometer_sub_category);
                strings = strings[mIdxCategory].split(",");
                String subCategory = strings[mIdxSubCategory];
                strings = mResources.getStringArray(R.array.pedometer_placement);
                strings = strings[getPlacementID(mIdxSubCategory)].split(",");
                String placement = strings[mIdxPlacement];
                strings = mResources.getStringArray(R.array.pedometer_person_sex);
                String personSex = strings[mIdxPersonSex];
                strings = mResources.getStringArray(R.array.pedometer_person_num);
                String personNum1 = strings[mIdxPersonNum1];
                strings = mResources.getStringArray(R.array.pedometer_person_num);
                String personNum2 = strings[mIdxPersonNum2];
                strings = mResources.getStringArray(R.array.pedometer_device);
                String device = strings[mIdxDevice];

                int groundTruth = 0;
                if (!edtTrueStep.getText().toString().equals("")
                        && isParsable(edtTrueStep.getText().toString())) {
                    groundTruth = Integer.parseInt(edtTrueStep.getText().toString());
                }
                int competitor = 0;
                if (!edtCompetitorStartStep.getText().toString().equals("")
                        && !edtCompetitorStopStep.getText().toString().equals("")
                        && isParsable(edtCompetitorStartStep.getText().toString())
                        && isParsable(edtCompetitorStopStep.getText().toString())) {
                    competitor = Integer.parseInt(edtCompetitorStopStep.getText().toString())
                        - Integer.parseInt(edtCompetitorStartStep.getText().toString());
                    mCompetitorStartStep = Integer
                        .parseInt(edtCompetitorStopStep.getText().toString());

                    if (mEdtCompetitorStartStepDefined != null) {
                        mEdtCompetitorStartStepDefined.setText(String.format(
                            Locale.getDefault(), "%d", mCompetitorStartStep));
                    }
                } else {
                    mCompetitorStartStep = -1;
                }
                int competitor2 = 0;
                if (!edtCompetitor2Step.getText().toString().equals("")
                       && isParsable(edtCompetitor2Step.getText().toString())) {
                    competitor2 = Integer.parseInt(edtCompetitor2Step.getText().toString());
                }
                int mtk = 0;
                if (!edtMTKStep.getText().toString().equals("")
                       && isParsable(edtMTKStep.getText().toString())) {
                    mtk = Integer.parseInt(edtMTKStep.getText().toString());
                }

                if (prevStr.startsWith(PEDOMETER_LOG_FILE_PREFIX)) {
                    String newStr = prevStr.split("_")[2].substring(2, 4)
                        + prevStr.split("_")[2].substring(5, 7)
                        + prevStr.split("_")[2].substring(8, 10) + "_"
                        + prevStr.split("_")[2].substring(11, 13)
                        + prevStr.split("_")[2].substring(14, 16)
                        + prevStr.split("_")[2].substring(17, 19) + "_"
                        + category + "_" + subCategory + "_" + placement + "_"
                        + personSex + personNum1 + personNum2 + "_" + device + "_"
                        + groundTruth + "_" + competitor + "_" + competitor2 + "_" + mtk
                        + "_mtk.txt";
                    renameLogFile(prevStr, newStr);
                }

                if (mIsNaming != null && mIsNaming.get()) {
                    mIsNaming.set(false);
                }
                dialog.cancel();
            }
        });
        altDlgConfirm.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                if (mIsNaming != null && mIsNaming.get()) {
                    mIsNaming.set(false);
                }

                Toast.makeText(
                    StepCounterSensorActivity.this, "Cancel", Toast.LENGTH_SHORT).show();
            }
        });
        altDlgConfirm.show();
    }

    private void renameLogFile(String strSrc, String strDest) {
        String externalPathSrc = Environment
            .getExternalStorageDirectory().getAbsolutePath() + PEDOMETER_LOG_SRC_DIR;
        File fileDirectorySrc = new File(externalPathSrc);
        if (!fileDirectorySrc.exists() && !fileDirectorySrc.mkdirs()) {
            Log.e(TAG, "Can not create file directory");
            Toast.makeText(
                    StepCounterSensorActivity.this, "Error: can not create log dir",
                    Toast.LENGTH_LONG).show();
            return;
        }
        String externalPathDest = Environment
            .getExternalStorageDirectory().getAbsolutePath() + PEDOMETER_LOG_DEST_DIR;
        File fileDirectoryDest = new File(externalPathDest);
        if (!fileDirectoryDest.exists() && !fileDirectoryDest.mkdirs()) {
            Log.e(TAG, "Can not create file directory");
            return;
        }

        if (strSrc != null) {
            File src = new File(fileDirectorySrc, strSrc);
            File dest = new File(fileDirectoryDest, strDest);

            if (src.exists()) {
                Log.d(TAG, "Rename file: " + src.renameTo(dest));
                Toast.makeText(
                    StepCounterSensorActivity.this, "Done", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(
                    StepCounterSensorActivity.this, "Error: can not find scp log",
                        Toast.LENGTH_SHORT).show();
            }
        } else {
            Toast.makeText(
                StepCounterSensorActivity.this, "Error: can not find scp log",
                    Toast.LENGTH_SHORT).show();
        }
    }

    private List<String> getFileList() {
        String externalPathSrc = Environment
            .getExternalStorageDirectory().getAbsolutePath() + PEDOMETER_LOG_SRC_DIR;
        File fileDirectorySrc = new File(externalPathSrc);
        if (!fileDirectorySrc.exists() && !fileDirectorySrc.mkdirs()) {
            Log.e(TAG, "Can not create file directory");
        }
        String externalPathDest = Environment
            .getExternalStorageDirectory().getAbsolutePath() + PEDOMETER_LOG_DEST_DIR;
        File fileDirectoryDest = new File(externalPathDest);
        if (!fileDirectoryDest.exists() && !fileDirectoryDest.mkdirs()) {
            Log.e(TAG, "Can not create file directory");
        }

        List<String> fileListSrc = new ArrayList<>();
        if(fileDirectorySrc.exists() && fileDirectoryDest.exists()) {
            List<String> fileListDest = new ArrayList<>();
            for (File fileEntry : fileDirectoryDest.listFiles()) {
                if (fileEntry.isFile()) {
                    fileListDest.add(fileEntry.getName());
                }
            }

            for (File fileEntry : fileDirectorySrc.listFiles()) {
                if (!isLogAlreadyParse(fileEntry.getName(), fileListDest)) {
                    fileListSrc.add(fileEntry.getName());
                }
            }
        }
        return fileListSrc;
    }

    private boolean isLogAlreadyParse(String str, List<String> strList) {
        return !(str.split("_").length == 3
            && str.split("_")[0].startsWith(PEDOMETER_LOG_FILE_PREFIX))
            || strList.contains(str);
    }

    private void setListViewHeightBasedOnChildren(ListView listView) {
        ListAdapter listAdapter = listView.getAdapter();
        if (listAdapter == null) {
            return;
        }

        int totalHeight = 0;
        int desiredWidth = MeasureSpec
            .makeMeasureSpec(listView.getWidth(), MeasureSpec.AT_MOST);
        for (int i = 0; i < listAdapter.getCount(); i++) {
            View listItem = listAdapter.getView(i, null, listView);
            listItem.measure(desiredWidth, MeasureSpec.UNSPECIFIED);
            totalHeight += listItem.getMeasuredHeight();
        }

        ViewGroup.LayoutParams params = listView.getLayoutParams();
        params.height = totalHeight
            + (listView.getDividerHeight() * (listAdapter.getCount() - 1));
        listView.setLayoutParams(params);
        listView.requestLayout();
    }

    @Override
    public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
        // enable or disable log
        if(Utils.getSensorStatus(Utils.LOG_STATUS) != (Boolean) arg1) {
            Utils.setSensorStatus(Utils.LOG_STATUS, (Boolean) arg1);
            if (mIsLogging != null) mIsLogging.set(arg1);
            Intent intent = new Intent(Utils.ACTION_STEP_COUNTER_LOGGING);
            LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
        }

        // Note: enable ACCELEROMETER while enable STEP_COUNTER
        Utils.setSensorStatus(Sensor.STRING_TYPE_ACCELEROMETER, arg1);
    }
}
