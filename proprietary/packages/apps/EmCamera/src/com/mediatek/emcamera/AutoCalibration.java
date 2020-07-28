/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.emcamera;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Application;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.emcamera.R;
import com.mediatek.emcamera.config.ConfigBase;
import com.mediatek.emcamera.config.FpsConfig;
import com.mediatek.emcamera.config.MfllConfig;
import com.mediatek.emcamera.config.StrobeConfig;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class AutoCalibration extends Activity implements OnItemClickListener, View.OnClickListener,
ActivityCompat.OnRequestPermissionsResultCallback {

    public static final String PREFERENCE_KEY = "camera_inter_settings";
    private static final String TAG = "AutoCalibration";
    private static final int DIALOG_ISO_SPEED = 5;
    private static final int DIALOG_VERIFICATION_AWB = 6;
    private static final int DIALOG_VERIFICATION_SHADING = 7;
    private static final int DIALOG_CAM_HAL_SWITCH = 8;

    private static final String[] ISO_STRS_ARRAY = { "0", "100", "150", "200", "300", "400",
            "600", "800", "1200", "1600", "2000", "2400", "2800", "3200", "4000", "4800", "5600",
            "6400", "7200", "8000", "8800", "9600", "11200", "12800" };
    public static final int ID_EV_CALIBRATION_NONE = 0;
    public static final int ID_EV_CALIBRATION_ACTION = 1;

    public static final int ID_GIS_CALIBRATION_NONE = 2;
    public static final int ID_GIS_CALIBRATION_ACTION = 3;

    // camera parameter key for eng mode
    private static final String KEY_ENG_MFLL_SUPPORTED = "eng-mfll-s";
    private static final String KEY_ENG_SENSOR_MODE_SLIM_VIDEO1_SUPPORTED = "sv1-s";
    private static final String KEY_ENG_SENSOR_MODE_SLIM_VIDEO2_SUPPORTED = "sv2-s";
    private static final String KEY_ENG_SENSOR_MODE_SUPPORTED = "sen-mode-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_2M_SUPPORTED = "vdr-r2m-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_4K2K_SUPPORTED = "vdr-r4k2k-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_CROP_CENTER_2M_SUPPORTED = "vdr-cc2m-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED = "vrd-mfr-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MIN = "vrd-mfr-min";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MAX = "vrd-mfr-max";

    private static final String KEY_ENG_MTK_AWB_SUPPORTED = "mtk-awb-s";
    private static final String KEY_ENG_SENSOR_AWB_SUPPORTED = "sr-awb-s";
    private static final String KEY_ENG_MTK_AWB_ENABLE = "mtk-awb-e";
    private static final String KEY_ENG_SENSOR_AWB_ENABLE = "sr-awb-e";

    private static final String KEY_ENG_MTK_SHADING_SUPPORTED = "mtk-shad-s";
    private static final String KEY_ENG_MTK_1TO3_SHADING_SUPPORTED = "mtk-123-shad-s";
    private static final String KEY_ENG_SENSOR_SHADNING_SUPPORTED = "sr-shad-s";
    private static final String KEY_ENG_MTK_SHADING_ENABLE = "mtk-shad-e";
    private static final String KEY_ENG_MTK_1TO3_SHADING_ENABLE = "mtk-123-shad-e";
    private static final String KEY_ENG_SENSOR_SHADNING_ENABLE = "sr-shad-e";

    private static final String KEY_ENG_VIDEO_HDR_SUPPORTED = "vhdr-s";

    private static final String KEY_ENG_MULTI_NR_SUPPORTED = "mnr-s";
    private static final String FALSE = "false";
    private static final String KEY_IMAGE_REFOCUS_SUPPORTED = "stereo-image-refocus-values";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_SUPPORTED = "vrd-s";

    private static final String KEY_ENG_GIS_CALIBRATION_SUPPORTED = "eng-gis-cali-s";

    private static final int CAPTURE_SIZE_NUM = 11;
    private static final int CAPTURE_SIZE_ZSD_INDEX = 2;
    private static final int CAPTURE_SIZE_SLIM_VIDEO1 = 4;
    private static final int CAPTURE_SIZE_SLIM_VIDEO2 = 5;
    private static final int CAPTURE_SIZE_CUSTOM_BASE = 6;
    private static final int CAPTURE_SIZE_CUSTOM_END = 10;

    private static final int SECNE_MODE_AUTO = 0;
    private static final int SECNE_MODE_FIREWORKS = 11;

    private static final int ISO_NONE_ID = 0;
    private static final int ISO_32_ID = 1;
    private static final int ISO_64_ID = 2;
    private static final int ISO_ALL_ID = 3;
    private static final int ISO_AUTO_ID = 4;
    private static final int ISO32X_LENGTH = 18;
    private static final int ISO64X_LENGTH = 22;

    private static final String[] SYSPROP_LIST = new String[] {
            "vendor.debug.lsc_mgr.type 2", "vendor.debug.awb_mgr.lock 1",
            "vendor.debug.ae.enable 9", "vendor.debug.shot.forcenr 0",
            "vendor.debug.hdr 1", "vendor.debug.lsc_mgr.manual_tsf 0",
            "vendor.debug.lsc_mgr.ct 0", "vendor.debug.lsc_mgr.enable 0" };
    //TODO switch camera hal
    private static final String ROPERTY_CAMERA_HAL_KEY =
            "persist.vendor.mtkcam.aosp_hal_version";
    private static final String ROPERTY_CAMERA_HAL_VAL_1 = "1";
    private static final String ROPERTY_CAMERA_HAL_VAL_3 = "3";
    // components
    private RadioButton mNormalMode;
    private LinearLayout mNormalCaptureLayout;
    private Spinner mNormalCaptureSize;
    private RadioButton mCaptureRatioStandard;
    private RadioButton mCaptureRatioFull;
    private Spinner mNormalCaptureType;
    private EditText mNormalCaptureNum;
    private RadioButton mVideoCliplMode;
    private LinearLayout mVideoClipLayout;
    private Spinner mVideoClipResolution;
    private RadioButton mAfAuto;
    private RadioButton mAfFullScan;
    private RadioButton mAfBracket;
    private RadioButton mAfThrough;
    private RadioButton mAfContinuous;
    private RadioButton mAfTempCali;
    private LinearLayout mAfBracketLayout;
    private EditText mAfBracketRange;
    private Spinner mAfBracketInterval;
    private LinearLayout mAfThroughLayout;
    private EditText mAfThroughInterval;
    private Spinner mAfThroughDirec;
    private ListView mIsoListView;
    private ListView mCaptureListView;

    private Spinner mAfThroughRepeat;
    private LinearLayout mAfFullScanSetLayout;
    private EditText mAfFullScanFrameInterval;
    private int mMaxFullScanFrameInterval;
    private int mMinFullScanFrameInterval;
    private EditText mAfFullScanDacStep;
    private EditText mAfFullScanFocusWinWidth;
    private EditText mAfFullScanFocusWinHeight;
    private CheckBox mAfAdvacedFullScan;
    private LinearLayout mAdvacedFullScanSetLayout;
    private EditText mAfFullScanRepeat;
    // Use for 2nd MP(JB2)
    // private Spinner mPreFlashSpinner;
    // private Spinner mMainFlashSpinner;
    private Spinner mFlickerSpinner;
    // Through focus: Manual configure
    private LinearLayout mThroughFocusStart;
    private LinearLayout mThroughFocusEnd;
    private EditText mThroughFocsuStartPos;
    private EditText mThroughFocsuEndPos;
    private LinearLayout mStrobeModeLayout;
    private Spinner mSensorSpinner;
    private int mCameraId;
    private int mStereoCameraId;
    private boolean mIsInitialized = false;
    private int mAvailCameraNumber = 0;
    private ListView mCommonOptionLv;
    private ListView mCommonCalibrationLv;
    private Spinner mSdblkSpinner;
    private Spinner mShadingSpinner;
    private Spinner mShutterDelaySpinner;
    // Video Raw Dump - Multi Frame Rate
    private LinearLayout mVideoFrameRateLayout;
    private Spinner mVideoFrameRateSpinner;
    private EditText mVideoFrameMinRate;
    private EditText mVideoFrameMaxRate;
    private int mMinFrameRate;
    private int mMaxFrameRate;
    private boolean mVideoFrameRateSupport = false;
    private int mVideoFrameRateMin = 0;
    private int mVideoFrameRateMax = 0;

    private boolean[] mMulISOFlags;
    private boolean mAfModeStatus = true; // auto is true, others is false
    private int mAfSpecialIso = 0;
    private int mCaptureMode = 0;
    private int mAfMode = 0;
    // ISO interplation
    private EditText mIsoRatioEdit;
    private Spinner mIsoInterSpinner;

    private CheckBox[] mAwbCkBoxs = new CheckBox[2];
    private CheckBox[] mShadingCkBoxs = new CheckBox[3];
    private TextView mAwb;
    private TextView mShading;
    // Vidoe HDR
    private Spinner mVideoHdrSpinner;
    private Spinner mVHdrModeSpinner;
    private EditText mVHdrRatio;
    // Multi Pass NR
    private Spinner mMultiNrSpinner;
    // Capture Size - Start
    private ArrayAdapter<String> mCaptureSizeAdapter = null;
    // default true: preview(0), capture(1), video(3);
    private boolean[] mCaptureSizeState = { true, true, false, true, false, false, false, false,
                                            false, false, false };
    private String[] mCaptureSize;
    // Capture Size - End
    private EditText mShutterSpeed;
    private EditText mSensorGain;

    // Scene mode
    private Spinner mSceneModeSpinner;
    // Flash calibration
    private TextView mFlashCalibration;
    private Spinner mFlashCalibSpinner;

    private TextView mOutputScreen = null;
    private TextView mSysPropName = null;
    private TextView mSysPropValue = null;
    // Face detection
    private Spinner mFaceDetectSpinner;
    // AE range
    private Spinner spinnerAeMode;
    private EditText etMinusEv;
    private EditText etPlusEv;
    private EditText etEvInterval;
    private AeSettings aeSettings = new AeSettings();
    // manual iso
    private Spinner spinnerISOManualSwitch;
    private EditText etISOManual;
    private boolean isManualISO = false;
    // video dump buffer
    private Spinner spinnerRawDumpBuffer;
    // ispProfile & sensorMode
    private Spinner spinnerNormalIspProfile;
    private Spinner spinnerVideoIspProfile;
    private Spinner spineerVideoSensorMode;

    private List<ConfigBase> mConfigList;
    private StrobeConfig mStrobeConfig;
    private MfllConfig mMfllConfig;
    private PermissionManager mPermissionManager;

    private CompoundButton.OnCheckedChangeListener mRadioListener =
                                            new CompoundButton.OnCheckedChangeListener() {

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            Elog.v(TAG, "Button is " + buttonView.getId() + "isChecked " + isChecked);
            if (isChecked) {
                View[] afLayoutArr = { mAfBracketLayout, mAfThroughLayout, mAfFullScanSetLayout };
                RadioButton[] afRadioArr = { mAfAuto, mAfBracket, mAfFullScan,
                                             mAfThrough, mAfContinuous, mAfTempCali };
                // when select full scan, capture size need support
                // "capture size(ZSD)"
                boolean changeAf = false;
                for (RadioButton rb : afRadioArr) {
                    if (rb == buttonView) {
                        changeAf = true;
                        break;
                    }
                }
                if (changeAf) {
                    boolean isCaptureModeZsd = mCaptureSizeState[CAPTURE_SIZE_ZSD_INDEX];
                    if(buttonView == mAfFullScan || buttonView == mAfTempCali) {
                            mCaptureSizeState[CAPTURE_SIZE_ZSD_INDEX] = true;
                    } else {
                        mCaptureSizeState[CAPTURE_SIZE_ZSD_INDEX] = false;
                    }
                    if(isCaptureModeZsd != mCaptureSizeState[CAPTURE_SIZE_ZSD_INDEX]) {
                        updateCaptureSizeAdapter();
                    }
                }

                if (buttonView == mNormalMode) {
                    mCaptureMode = 0;
                    mNormalCaptureLayout.setVisibility(View.VISIBLE);
                    mVideoClipLayout.setVisibility(View.GONE);
                    mVideoCliplMode.setChecked(false);
                    // set af status
                    setAfModeAccessble(true);
                } else if (buttonView == mVideoCliplMode) {
                    mCaptureMode = 2;
                    mNormalCaptureLayout.setVisibility(View.GONE);
                    mVideoClipLayout.setVisibility(View.VISIBLE);
                    mNormalMode.setChecked(false);
                    // set af status
                    mAfAuto.setChecked(true);
                    setAfModeAccessble(false);
                    // isp profile & sensor mode
                } else if (buttonView == mAfAuto) {
                    mAfMode = Camera.AF_MODE_AUTO;
                    setVisibleLayout(afLayoutArr, null);
                    setCheckedRadioGroup(afRadioArr, mAfAuto);
                } else if (buttonView == mAfBracket) {
                    mAfMode = Camera.AF_MODE_BRACKET;
                    setVisibleLayout(afLayoutArr, new View[] { mAfBracketLayout });
                    setCheckedRadioGroup(afRadioArr, mAfBracket);
                } else if (buttonView == mAfFullScan) {
                    mAfMode = Camera.AF_MODE_FULL_SCAN;
                    setVisibleLayout(afLayoutArr, new View[] { mAfFullScanSetLayout });
                    setCheckedRadioGroup(afRadioArr, mAfFullScan);
                } else if (buttonView == mAfThrough) {
                    mAfMode = Camera.AF_MODE_THROUGH_FOCUS;
                    setVisibleLayout(afLayoutArr, new View[] { mAfThroughLayout });
                    setCheckedRadioGroup(afRadioArr, mAfThrough);
                } else if (buttonView == mAfContinuous) {
                    mAfMode = Camera.AF_MODE_CONTINUOUS;
                    setVisibleLayout(afLayoutArr, null);
                    setCheckedRadioGroup(afRadioArr, mAfContinuous);
                } else if (buttonView == mAfTempCali) {
                    mAfMode = Camera.AF_MODE_TEMP_CALI;
                    setVisibleLayout(afLayoutArr, null);
                    setCheckedRadioGroup(afRadioArr, mAfTempCali);
                }

                if (mAfMode == Camera.AF_MODE_AUTO) {
                    if (!mAfModeStatus) {
                        statusChangesByAf(true);
                        mAfModeStatus = true;
                    }
                } else {
                    if (mAfModeStatus) {
                        statusChangesByAf(false);
                        mAfModeStatus = false;
                    }
                }
            }
        }
    };

    private MfllConfig.Listener mMfllConfigListener = new MfllConfig.Listener() {

        @Override
        public void updateUi(int state) {
            if(MfllConfig.ON == state || MfllConfig.AUTO == state) {
                if (mNormalCaptureType.getSelectedItemPosition() != 1) {
                    mNormalCaptureType.setSelection(1); // Pure Raw only
                    Toast.makeText(AutoCalibration.this, "Swtich to Pure Raw",
                                    Toast.LENGTH_LONG).show();
                }
                mNormalCaptureType.setEnabled(false);
                mStrobeConfig.setStrobeState(StrobeConfig.OFF); // off
            } else {
                mNormalCaptureType.setEnabled(true);
                mStrobeConfig.setStrobeState(StrobeConfig.AUTO); // auto
            }
        }
    };

    private void initConfigList() {
        mStrobeConfig = new StrobeConfig(this);
        mMfllConfig = new MfllConfig(this);
        mMfllConfig.registerListener(mMfllConfigListener );

        mConfigList = new ArrayList<ConfigBase>();
        mConfigList.add(mStrobeConfig);
        mConfigList.add(mMfllConfig);
        mConfigList.add(new FpsConfig(this));
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(Util.isAutoTest()) {
            Elog.i(TAG, "isAutoTest return.");
            finish();
            return;
        }
        if(isNeedSwitchHal()) {
            showDialog(DIALOG_CAM_HAL_SWITCH);
            return;
        }
        Elog.i(TAG, "onCreate.");
        mIsInitialized = false;
        setContentView(R.layout.new_auto_calibration);

        mAvailCameraNumber = android.hardware.Camera.getNumberOfCameras();
        Elog.i(TAG, "mAvailCameraNumber =" + mAvailCameraNumber);
        if (mAvailCameraNumber <= 0) {
            Toast.makeText(this, "No available Camera", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        mMulISOFlags = new boolean[28];
        resetISOValue();

        initConfigList();
        inintComponents();
        mCameraId = 0;
        mPermissionManager = new PermissionManager(this);

        if (mPermissionManager.requestCameraLaunchPermissions()) {
            Elog.i(TAG, "mPermissionManager.requestCameraLaunchPermissions");
            safeUseCameraDevice(mCameraId, new CameraSafeUser() {
                @Override
                public void useCamera(android.hardware.Camera camera) {
                    initByCameraDeviceCapability(camera.getParameters());
                }
            });
            setStatusTodefault();
            mIsInitialized = true;
        }

        mOutputScreen = (TextView) findViewById(R.id.outputText);
        mSysPropName = (EditText) findViewById(R.id.syspropName);
        mSysPropValue = (EditText) findViewById(R.id.syspropValue);
        Spinner spinner2 = (Spinner) findViewById(R.id.sysPropSpinnner);
        ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(this,
                                        android.R.layout.simple_spinner_item, SYSPROP_LIST);
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner2.setAdapter(adapter2);
        spinner2.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View arg1,
                                        int position, long arg3) {
                Object obj = adapterView.getSelectedItem();
                if (obj == null) {
                    Elog.e(TAG, "SysProp selected item obj is null");
                    return;
                }
                mSysPropName.setText(obj.toString().split(" ")[0]);
                mSysPropValue.setText(obj.toString().split(" ")[1]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        Button button = (Button) findViewById(R.id.setBtn);
        button.setOnClickListener(this);

        button = (Button) findViewById(R.id.getBtn);
        button.setOnClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Elog.i(TAG, "onItemClick view: " + parent.getId() + " position: " + position);
        if (parent.getId() == mIsoListView.getId()) {
            if (position == 0) { // set ISO speed
                showDialog(DIALOG_ISO_SPEED);
            }
        } else if (parent.getId() == R.id.common_option_list) {
            if (position == 0) { // EV Calibration
                if (!putValuesToPreference(PREFERENCE_KEY)) {
                    return;
                }
                Elog.d(TAG, "Start EV Calibration");
                Intent previewIntent = new Intent(this, Camera.class);
                previewIntent.putExtra(getString(R.string.camera_key_ev_calibration),
                                                ID_EV_CALIBRATION_ACTION);
                startActivity(previewIntent);
            }
        } else if (parent.getId() == R.id.common_calibration_list) {
            if (position == 0) { // GIS Calibration
                setCaptureRatio(false);
                if (!putValuesToPreference(PREFERENCE_KEY)) {
                    return;
                }

                Elog.d(TAG, "Start GIS Calibration");
                Intent previewIntent = new Intent(this, Camera.class);
                previewIntent.putExtra(getString(R.string.camera_key_gis_calibration),
                                                    ID_GIS_CALIBRATION_ACTION);
                startActivity(previewIntent);
            }
        } else if (parent.getId() == R.id.listview_capture) {
            if (position == 0) { // start preview
                if (!putValuesToPreference(PREFERENCE_KEY)) {
                    return;
                }
                Intent captureIntent = new Intent();
                captureIntent.setClass(this, Camera.class);
                this.startActivity(captureIntent);
                Elog.i(TAG, "Start captureIntent!");
            }
        } else if (parent.getId() == R.id.flash_calibration_list) {
            if (position == 0) { // start flash calibration
                Intent intent = new Intent();
                intent.setClassName("com.mediatek.camera",
                        "com.mediatek.camera.CameraActivity");
                intent.putExtra("flash_calibration", true);
                if(isActivityAvailable(intent)) {
                    this.startActivity(intent);
                }
                Elog.i(TAG, "Start flash calibration!");
            }
        } else {
            Elog.w(TAG, "UNKNOWN Parent ID:" + parent.getId());
        }
    }

    private boolean isActivityAvailable(Intent intent) {
        return null != this.getPackageManager().resolveActivity(intent, 0);
    }

    private void setCaptureRatio(boolean isStandard) {
        if (isStandard) {
            mCaptureRatioStandard.setChecked(true);
        } else {
            mCaptureRatioFull.setChecked(true);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Elog.i(TAG, "onCreateDialog id: " + id);
        Dialog dialog = null;
        Builder builder = null;
        if (DIALOG_ISO_SPEED == id) {
            builder = new AlertDialog.Builder(AutoCalibration.this);
            builder.setTitle(R.string.auto_clibr_iso_setting);
            builder.setCancelable(false);
            if (mAfAuto.isChecked() && mNormalMode.isChecked()) {
                builder.setMultiChoiceItems(R.array.iso_multi_dialog_entries, mMulISOFlags,
                        new DialogInterface.OnMultiChoiceClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                                mMulISOFlags[which] = isChecked;
                                ListView lv = ((AlertDialog) dialog).getListView();
                                switch (which) {
                                case ISO_NONE_ID:
                                    if (isChecked) {
                                        for (int i = ISO_NONE_ID + 1; i < mMulISOFlags.length; i++) {
                                            setIsoCheck(lv, i, false);
                                        }
                                        setIsoCheck(lv, ISO_AUTO_ID, true);
                                    }
                                    break;
                                case ISO_32_ID:
                                    if (isChecked) {
                                        setIsoCheck(lv, ISO_NONE_ID, false);
                                        setIsoCheck(lv, ISO_ALL_ID, false);
                                        setIsoCheck(lv, ISO_64_ID, false);
                                        for (int i = ISO_AUTO_ID; i < ISO32X_LENGTH; i++) {
                                            setIsoCheck(lv, i, true);
                                        }
                                        for (int i = ISO32X_LENGTH; i < mMulISOFlags.length; i++) {
                                            setIsoCheck(lv, i, false);
                                        }
                                    }
                                    break;
                                case ISO_64_ID:
                                    if (isChecked) {
                                        setIsoCheck(lv, ISO_NONE_ID, false);
                                        setIsoCheck(lv, ISO_ALL_ID, false);
                                        setIsoCheck(lv, ISO_32_ID, false);
                                        for (int i = ISO_AUTO_ID; i < ISO64X_LENGTH; i++) {
                                            setIsoCheck(lv, i, true);
                                        }
                                        for (int i = ISO64X_LENGTH; i < mMulISOFlags.length; i++) {
                                            setIsoCheck(lv, i, false);
                                        }
                                    }
                                    break;
                                case ISO_ALL_ID:
                                    if (isChecked) {
                                        setIsoCheck(lv, ISO_NONE_ID, false);
                                        setIsoCheck(lv, ISO_32_ID, false);
                                        setIsoCheck(lv, ISO_64_ID, false);
                                        for (int i = ISO_AUTO_ID; i < mMulISOFlags.length; i++) {
                                            setIsoCheck(lv, i, true);
                                        }
                                    }
                                    break;
                                default:
                                    if (isChecked) {
                                        setIsoCheck(lv, ISO_NONE_ID, false);
                                    } else {
                                        setIsoCheck(lv, ISO_ALL_ID, false);
                                    }
                                    setIsoCheck(lv, ISO_32_ID, false);
                                    setIsoCheck(lv, ISO_64_ID, false);
                                    break;
                                }
                            }
                        });
            } else {
                builder.setSingleChoiceItems(R.array.iso_single_dialog_entries, mAfSpecialIso,
                        new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mAfSpecialIso = whichButton;
                    }
                });
            }
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    // Open next time, need check camera mode and AF mode again.
                    removeDialog(DIALOG_ISO_SPEED);
                }
            });
        } else if (id == DIALOG_CAM_HAL_SWITCH) {
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.camera_hal_switch_title);
            builder.setCancelable(false);
            builder.setMessage(R.string.camera_hal_switch_msg);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    Util.setProperty(ROPERTY_CAMERA_HAL_KEY,ROPERTY_CAMERA_HAL_VAL_1);
                    AutoCalibration.this.finish();
                }
            });
        }
        if (builder != null) {
            dialog = builder.create();
        }
        return dialog;
    }

    private void setIsoCheck(ListView lv, int position, boolean isChecked) {
        mMulISOFlags[position] = isChecked;
        lv.setItemChecked(position, isChecked);
    }

    private void setAfLayout(int[] visis, RadioButton[] buttons) {
        mAfBracketLayout.setVisibility(visis[0]);
        mAfThroughLayout.setVisibility(visis[1]);
        for (RadioButton radioButton : buttons) {
            radioButton.setChecked(false);
        }
    }

    private void setVisibleLayout(View[] allLayouts, View[] visibleLayouts) {
        for (View view : allLayouts) {
            if (visibleLayouts != null) {
                boolean inVisible = false;
                for (View visiLayout : visibleLayouts) {
                    if (visiLayout == view) {
                        inVisible = true;
                        break;
                    }
                }
                if (inVisible) {
                    view.setVisibility(View.VISIBLE);
                } else {
                    view.setVisibility(View.GONE);
                }
            } else {
                view.setVisibility(View.GONE);
            }
        }
    }

    private void setCheckedRadioGroup(RadioButton[] allRadios, RadioButton checkedRadio) {
        for (RadioButton radio : allRadios) {
            if (radio == checkedRadio) {
                radio.setChecked(true);
            } else {
                radio.setChecked(false);
            }
        }
    }

    private void setAfModeAccessble(boolean access) {
        RadioButton[] buttons = { mAfBracket, mAfFullScan, mAfThrough, mAfContinuous, mAfTempCali };
        for (RadioButton radioButton : buttons) {
            radioButton.setEnabled(access);
        }
    }

    private String collectCustomParams() {
        StringBuilder builder = new StringBuilder();
        int[] editIds = { R.id.camera_custom_param_0_edit, R.id.camera_custom_param_1_edit,
                R.id.camera_custom_param_2_edit };
        for (int i = 0; i < editIds.length; i++) {
            String value = ((EditText) findViewById(editIds[i])).getText().toString();
            if (value != null) {
                builder.append(value).append(";");
            } else {
                builder.append(";");
            }
        }
        return builder.toString();
    }

    private void updateCaptureSizeAdapter() {
        mCaptureSizeAdapter.clear();
        for (int i = 0; i < CAPTURE_SIZE_NUM; i++) {
            if (mCaptureSizeState[i]) {
                mCaptureSizeAdapter.add(mCaptureSize[i]);
            }
        }
    }

    private void inintComponents() {
        // Capture mode
        mNormalMode = (RadioButton) findViewById(R.id.raido_capture_normal);
        mNormalMode.setOnCheckedChangeListener(mRadioListener);
        mVideoCliplMode = (RadioButton) findViewById(R.id.raido_capture_video);
        mVideoCliplMode.setOnCheckedChangeListener(mRadioListener);
        mNormalCaptureLayout = (LinearLayout) findViewById(R.id.normal_capture_set);
        mNormalCaptureSize = (Spinner) findViewById(R.id.normal_capture_size);
        mCaptureSize = getResources().getStringArray(R.array.normal_captrue_size);
        mCaptureSizeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        for (int i = 0; i < CAPTURE_SIZE_NUM; i++) {
            if (mCaptureSizeState[i]) {
                mCaptureSizeAdapter.add(mCaptureSize[i]);
            }
        }
        mCaptureSizeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mNormalCaptureSize.setAdapter(mCaptureSizeAdapter);

        mCaptureRatioStandard = (RadioButton) findViewById(R.id.raido_capture_ratio_standard);
        mCaptureRatioFull = (RadioButton) findViewById(R.id.raido_capture_ratio_full);

        mNormalCaptureType = (Spinner) findViewById(R.id.normal_capture_type);
        mNormalCaptureNum = (EditText) findViewById(R.id.normal_capture_number);
        mShutterSpeed = (EditText) findViewById(R.id.normal_capture_shutter_speed);
        mSensorGain = (EditText) findViewById(R.id.normal_capture_sensor_gain);
        mVideoClipLayout = (LinearLayout) findViewById(R.id.video_capture_set);
        mVideoClipResolution = (Spinner) findViewById(R.id.video_capture_resolution);
        // Video Raw Dump - Multi Frame Rate
        String[] videoResolution = AutoCalibration.this.getResources().getStringArray(
                                                                    R.array.auto_calib_video_clip);
        List<String> videoResolutionList = new ArrayList<String>();
        for (int i = 0; i < videoResolution.length; i++) {
            videoResolutionList.add(videoResolution[i]);
        }
        ArrayAdapter<String> adp = new ArrayAdapter<String>(AutoCalibration.this,
                                        android.R.layout.simple_spinner_item, videoResolutionList);
        adp.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVideoClipResolution.setAdapter(adp);
        // Manual Frame Rate
        mVideoFrameRateLayout = (LinearLayout) findViewById(R.id.video_frame_set);
        mVideoFrameMinRate = (EditText) findViewById(R.id.video_frame_min_rate);
        mVideoFrameMaxRate = (EditText) findViewById(R.id.video_frame_max_rate);
        mVideoFrameRateSpinner = (Spinner) findViewById(R.id.auto_clibr_video_frame_set);
        mVideoFrameRateSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 == 1) {
                    mVideoFrameMinRate.setEnabled(true);
                    mVideoFrameMaxRate.setEnabled(true);
                } else {
                    mVideoFrameMinRate.setEnabled(false);
                    mVideoFrameMaxRate.setEnabled(false);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "Frame rate - select nothing.");
            }

        });
        // Af mode
        mAfAuto = (RadioButton) findViewById(R.id.raido_af_auto);
        mAfAuto.setOnCheckedChangeListener(mRadioListener);
        mAfFullScan = (RadioButton) findViewById(R.id.raido_af_full);
        mAfFullScan.setOnCheckedChangeListener(mRadioListener);
        mAfFullScanSetLayout = (LinearLayout) findViewById(R.id.af_full_scan_set_layout);
        mAfFullScanFrameInterval = (EditText) findViewById(R.id.af_full_scan_frame_interval_edit);
        mAfFullScanDacStep = (EditText) findViewById(R.id.et_af_full_dac_step);
        mAfFullScanFocusWinWidth = (EditText) findViewById(R.id.et_af_full_focus_win_width);
        mAfFullScanFocusWinHeight = (EditText) findViewById(R.id.et_af_full_focus_win_height);
        mAfAdvacedFullScan =  (CheckBox) findViewById(R.id.check_adv_af_full);
        mAdvacedFullScanSetLayout = (LinearLayout) findViewById(R.id.af_adv_full_scan_set_layout);
        mAdvacedFullScanSetLayout.setVisibility(View.GONE);
        mAfAdvacedFullScan.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
                if(arg1) {
                    mAdvacedFullScanSetLayout.setVisibility(View.VISIBLE);
                } else {
                    mAdvacedFullScanSetLayout.setVisibility(View.GONE);
                }

            }});
        mAfFullScanRepeat = (EditText) findViewById(R.id.full_scan_repeat_edit);

        mAfBracket = (RadioButton) findViewById(R.id.raido_af_bracket);
        mAfBracket.setOnCheckedChangeListener(mRadioListener);
        mAfThrough = (RadioButton) findViewById(R.id.raido_af_through);
        mAfThrough.setOnCheckedChangeListener(mRadioListener);
        mAfBracketLayout = (LinearLayout) findViewById(R.id.af_bracket_set);
        mAfBracketInterval = (Spinner) findViewById(R.id.af_bracket_interval);
        mAfBracketRange = (EditText) findViewById(R.id.af_bracket_range);
        mAfThroughLayout = (LinearLayout) findViewById(R.id.af_through_set);
        mAfThroughDirec = (Spinner) findViewById(R.id.af_through_dirct);
        // If Manual configure(2) has been selected, show start/stop pos
        // editors, else let them gone.
        mAfThroughDirec.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 == 2) {
                    mThroughFocusStart.setVisibility(View.VISIBLE);
                    mThroughFocusEnd.setVisibility(View.VISIBLE);
                } else {
                    mThroughFocusStart.setVisibility(View.GONE);
                    mThroughFocusEnd.setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "select nothing.");
            }

        });
        mAfThroughInterval = (EditText) findViewById(R.id.af_through_interval);
        // Through focus: Manual configure
        mThroughFocusStart = (LinearLayout) findViewById(R.id.through_focus_start_set);
        mThroughFocusEnd = (LinearLayout) findViewById(R.id.through_focus_end_set);
        mThroughFocsuStartPos = (EditText) findViewById(R.id.af_through_manual_start);
        mThroughFocsuEndPos = (EditText) findViewById(R.id.af_through_manual_end);
        mAfThroughRepeat = (Spinner) findViewById(R.id.af_through_repeat_sp);
        mAfContinuous = (RadioButton) findViewById(R.id.raido_af_continuous);
        mAfContinuous.setOnCheckedChangeListener(mRadioListener);
        mAfTempCali = (RadioButton) findViewById(R.id.raido_af_temp_cali);
        mAfTempCali.setOnCheckedChangeListener(mRadioListener);
        // ISO list view
        mIsoListView = (ListView) findViewById(R.id.listview_iso);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                                                android.R.layout.simple_list_item_1,
                new String[] { getString(R.string.auto_clibr_iso_speed) });
        mIsoListView.setAdapter(adapter);
        setLvHeight(mIsoListView);
        mIsoListView.setOnItemClickListener(this);
        // init spinners
        mFlickerSpinner = (Spinner) findViewById(R.id.flicker);
        // Capture lis view
        adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                Arrays.asList(getResources().getStringArray(R.array.auto_calib_capture)));
        mCaptureListView = (ListView) findViewById(R.id.listview_capture);
        mCaptureListView.setAdapter(adapter);
        setLvHeight(mCaptureListView);
        mCaptureListView.setOnItemClickListener(this);

        mSensorSpinner = (Spinner) findViewById(R.id.auto_clibr_camera_sensor_spnr);

        mCommonOptionLv = (ListView) findViewById(R.id.common_option_list);
        adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                getResources().getStringArray(R.array.camera_common_option_entries));
        mCommonOptionLv.setAdapter(adapter);
        mCommonOptionLv.setOnItemClickListener(this);

        mCommonCalibrationLv = (ListView) findViewById(R.id.common_calibration_list);
        adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                getResources().getStringArray(R.array.common_calibration_entries));
        mCommonCalibrationLv.setAdapter(adapter);
        mCommonCalibrationLv.setOnItemClickListener(this);

        // Verification
        mAwb = (TextView) findViewById(R.id.auto_clibr_veri_awb);
        mShading = (TextView) findViewById(R.id.auto_clibr_veri_shading);
        mAwbCkBoxs[0] = (CheckBox) findViewById(R.id.auto_clibr_veri_awb_mtk);
        mAwbCkBoxs[1] = (CheckBox) findViewById(R.id.auto_clibr_veri_awb_sensor);

        mShadingCkBoxs[0] = (CheckBox) findViewById(R.id.auto_clibr_veri_shading_mtk);
        mShadingCkBoxs[1] = (CheckBox) findViewById(R.id.auto_clibr_veri_shading_mtk_1to3);
        mShadingCkBoxs[2] = (CheckBox) findViewById(R.id.auto_clibr_veri_shading_mtk_sensor);
        // Video Hdr
        mVideoHdrSpinner = (Spinner) findViewById(R.id.video_hdr_spinner);
        mVideoHdrSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 == 1) {
                    if (mNormalCaptureSize.getSelectedItemPosition() != 2) {
                        mNormalCaptureSize.setSelection(2); // Video Size only
                        Toast.makeText(AutoCalibration.this, "Swtich to Video Size",
                                        Toast.LENGTH_LONG).show();
                    }
                    findViewById(R.id.vhdr_set_id).setVisibility(View.VISIBLE);
                } else {
                    findViewById(R.id.vhdr_set_id).setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "mVideoHdrSpinner - select nothing.");
            }

        });
        mVHdrModeSpinner = (Spinner) findViewById(R.id.vhdr_set_mode_spinner);
        mVHdrRatio = (EditText) findViewById(R.id.vhdr_set_ratio_edit);
        // Multi Pass NR
        mMultiNrSpinner = (Spinner) findViewById(R.id.multi_pass_nr_spinner);
        mSdblkSpinner = (Spinner) findViewById(R.id.output_sdblk_file_spn);
        mShadingSpinner = (Spinner) findViewById(R.id.shading_table_spn);
        findViewById(R.id.output_sdblk_file_ll).setVisibility(View.GONE);
        String[] shadingTableItem = getResources().getStringArray(
                                                    R.array.auto_clib_shading_table_items);
        ArrayList<String> shadingTableItemArray = new ArrayList<String>();
        for (String str : shadingTableItem) {
            shadingTableItemArray.add(str);
        }
        shadingTableItemArray.add("Disable");
        adp = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                                        shadingTableItemArray);
        adp.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mShadingSpinner.setAdapter(adp);
        mShutterDelaySpinner = (Spinner) findViewById(R.id.shutter_delay_spn);
        mIsoRatioEdit = (EditText) findViewById(R.id.auto_clibr_iso_inter_edit);
        mIsoInterSpinner = (Spinner) findViewById(R.id.auto_clibr_iso_inter_spinner);
        mIsoInterSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 == 1) {
                    findViewById(R.id.auto_clibr_iso_inter_set).setVisibility(View.VISIBLE);
                } else {
                    findViewById(R.id.auto_clibr_iso_inter_set).setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "mIsoInterSpinner - select nothing.");
            }

        });
        // Flash Calibration
        mFlashCalibration = (TextView) findViewById(R.id.auto_clibration_flash);
        mFlashCalibSpinner = (Spinner) findViewById(R.id.auto_clibr_flash_spinner);
        if (!isCameraHal1()) {

            ListView flashCaliLv = (ListView) findViewById(R.id.flash_calibration_list);
            flashCaliLv.setVisibility(View.VISIBLE);
            flashCaliLv.setOnItemClickListener(this);

        }
        mSceneModeSpinner = (Spinner) findViewById(R.id.auto_clibr_scene_mode_spinner);
        mSceneModeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            // need sync with normal camera
            // ISO, Flash
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 != SECNE_MODE_AUTO) {
                    resetISOValue(); // change ISO to Auto.
                }
                if (arg2 == SECNE_MODE_FIREWORKS) {
                    mStrobeConfig.setStrobeState(StrobeConfig.OFF); // fireworks need set flash off.
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "mSceneModeSpinner - select nothing.");
            }

        });
        mFaceDetectSpinner = (Spinner) findViewById(R.id.auto_clibr_face_detect_spinner);

        initComponentAeMode();
        // manual iso
        spinnerISOManualSwitch = (Spinner) findViewById(R.id.spinner_normal_manual_iso);
        spinnerISOManualSwitch.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                if (pos == 1) {
                    findViewById(R.id.ll_manual_iso_input).setVisibility(View.VISIBLE);
                    findViewById(R.id.listview_iso).setVisibility(View.GONE);
                    isManualISO = true;
                } else {
                    findViewById(R.id.ll_manual_iso_input).setVisibility(View.GONE);
                    findViewById(R.id.listview_iso).setVisibility(View.VISIBLE);
                    isManualISO = false;
                }
                Elog.i(TAG, "spinnerManualISO - select "
                        + getResources().getStringArray(R.array.auto_calib_on_off_value)[pos]);
            }

            public void onNothingSelected(AdapterView<?> parent) {
                Elog.d(TAG, "spinnerAeMode - select nothing.");
            }
        });
        etISOManual = (EditText) findViewById(R.id.et_iso_manual);
        spinnerNormalIspProfile = (Spinner) findViewById(R.id.normal_capture_isp_profile);
        // video ispProfile & sensorMode
        spinnerVideoIspProfile = (Spinner) findViewById(R.id.spinner_video_isp_profile);
        spineerVideoSensorMode = (Spinner) findViewById(R.id.spinner_video_sensor_mode);
        // raw dump buffer
        spinnerRawDumpBuffer = (Spinner) findViewById(R.id.spinner_video_dump_buffer);
        spinnerNormalIspProfile.setSelection(1); // Capture
        spinnerVideoIspProfile.setSelection(1); // video
        spineerVideoSensorMode.setSelection(1); // video
        spinnerRawDumpBuffer.setSelection(0); // IMGO
        for(ConfigBase config : mConfigList) {
            config.initComponents();
        }
    }

    private void putInPreferenceIsp(SharedPreferences.Editor editor, int key, int resId, int pos) {
        String item = getResources().getStringArray(resId)[pos];
        if (item.equals("Preview")) {
            putInPreference(editor, key, 0);
        } else if (item.equals("Capture")) {
            putInPreference(editor, key, 1);
        } else if (item.equals("Video")) {
            putInPreference(editor, key, 2);
        } else {
            putInPreference(editor, key, 0);
        }
    }

    private void putInPreferenceSensor(SharedPreferences.Editor editor, int key, int resId
            , int pos) {
        String item = getResources().getStringArray(resId)[pos];
        if (item.equals("Preview")) {
            putInPreference(editor, key, 1);
        } else if (item.equals("Capture")) {
            putInPreference(editor, key, 2);
        } else if (item.equals("Jpeg")) {
            putInPreference(editor, key, 3);
        } else if (item.equals("Video")) {
            putInPreference(editor, key, 4);
        } else if (item.equals("Slim_video1")) {
            putInPreference(editor, key, 5);
        } else if (item.equals("Slim_video2")) {
            putInPreference(editor, key, 6);
        } else {
            putInPreference(editor, key, 0);
        }
    }

    private void initComponentAeMode() {
        spinnerAeMode = (Spinner) findViewById(R.id.spiner_auto_clibr_ae_mode);
        spinnerAeMode.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                if (pos == 1) {
                    findViewById(R.id.ll_auto_clibr_ae_mode_bracket_input).setVisibility(
                                                                                View.VISIBLE);
                    aeSettings.setAeMode(AeMode_e.Bracket);
                } else {
                    findViewById(R.id.ll_auto_clibr_ae_mode_bracket_input).setVisibility(View.GONE);
                    aeSettings.setAeMode(AeMode_e.Auto);
                }
                Elog.i(TAG, "spinnerAeMode - select " + aeSettings.getAeMode().getDesc());
            }

            public void onNothingSelected(AdapterView<?> parent) {
                Elog.d(TAG, "spinnerAeMode - select nothing.");
            }
        });
        etMinusEv = (EditText) findViewById(R.id.et_auto_clibr_ae_minus);
        etPlusEv = (EditText) findViewById(R.id.et_auto_clibr_ae_plus);
        etEvInterval = (EditText) findViewById(R.id.et_auto_clibr_ae_interval);
    }

    private boolean setPrefAeMode(SharedPreferences.Editor editor) {
        int minusEvValue = Util.getIntegerFromEdit(etMinusEv, 0, true, 200, true);
        int plusEvValue = Util.getIntegerFromEdit(etPlusEv, 0, true, 200, true);
        int evIntervalValue = Util.getIntegerFromEdit(etEvInterval, 0, true, 200, true);
        if (minusEvValue == -1 || (minusEvValue % 10 !=0)
                || plusEvValue == -1 || (plusEvValue % 10 !=0)
                || evIntervalValue == -1 || (evIntervalValue % 10 !=0)) {
            Toast.makeText(this, R.string.auto_clibr_ae_range_tip, Toast.LENGTH_LONG).show();
            return false;
        }
        aeSettings.setMinus(minusEvValue);
        aeSettings.setPlus(plusEvValue);
        aeSettings.setInterval(evIntervalValue);
        editor.putString(getString(R.string.auto_clibr_ae_mode), aeSettings.getAeMode().getDesc());
        if (aeSettings.getAeMode().equals(AeMode_e.Bracket)) {
            editor.putInt(getString(R.string.auto_clibr_ae_minus), aeSettings.getMinus());
            editor.putInt(getString(R.string.auto_clibr_ae_plus), aeSettings.getPlus());
            editor.putInt(getString(R.string.auto_clibr_ae_interval), aeSettings.getInterval());
        }
        Elog.i(TAG, "@setPrefAeMode,toString: " + aeSettings.toString());
        return true;
    }

    private void resetISOValue() {
        for (int i = 0; i < mMulISOFlags.length; i++) {
            mMulISOFlags[i] = false;
        }
        mMulISOFlags[ISO_NONE_ID] = true;
        mMulISOFlags[ISO_AUTO_ID] = true;
        mAfSpecialIso = 0;
    }

    private void initSupportedUi(Parameters parameters) {
        Elog.i(TAG, "hal version = " + CameraJni.getCamHalVersion());
        if(!isCameraHal1()) {
            removeHal1Feature();
        } else {

            List<String> focusModes = parameters.getSupportedFocusModes();
            if (focusModes == null || !Camera.isSupportAf(parameters)) {
                findViewById(R.id.camera_af_full_scan_ll).setVisibility(View.GONE);
                findViewById(R.id.camera_af_temp_cali).setVisibility(View.GONE);
                findViewById(R.id.camera_af_bracket_ll).setVisibility(View.GONE);
                findViewById(R.id.camera_af_through_focus_ll).setVisibility(View.GONE);
                findViewById(R.id.camera_af_continuous).setVisibility(View.GONE);
            } else {
                if (!focusModes.contains(Parameters.FOCUS_MODE_AUTO)) {
                    findViewById(R.id.camera_af_auto_ll).setVisibility(View.GONE);
                } else {
                    findViewById(R.id.camera_af_auto_ll).setVisibility(View.VISIBLE);
                }
                if (!focusModes.contains(Camera.KEY_ENG_FOCUS_MODE_FULLSCAN)) {
                    findViewById(R.id.camera_af_full_scan_ll).setVisibility(View.GONE);
                    findViewById(R.id.camera_af_temp_cali).setVisibility(View.GONE);
                } else {
                    mMaxFullScanFrameInterval = parameters.getInt(
                            Camera.KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX);
                    mMinFullScanFrameInterval = parameters.getInt(
                            Camera.KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN);
                    findViewById(R.id.camera_af_full_scan_ll).setVisibility(View.VISIBLE);
                    findViewById(R.id.camera_af_temp_cali).setVisibility(View.VISIBLE);
                }
                if (!focusModes.contains(Camera.KEY_ENG_FOCUS_MODE_MANUAL)) {
                    findViewById(R.id.camera_af_bracket_ll).setVisibility(View.GONE);
                    findViewById(R.id.camera_af_through_focus_ll).setVisibility(View.GONE);
                } else {
                    findViewById(R.id.camera_af_bracket_ll).setVisibility(View.VISIBLE);
                    findViewById(R.id.camera_af_through_focus_ll).setVisibility(View.VISIBLE);
                }
                if (!focusModes.contains(Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                    findViewById(R.id.camera_af_continuous).setVisibility(View.GONE);
                } else {
                    findViewById(R.id.camera_af_continuous).setVisibility(View.VISIBLE);
                }
            }
            int silmV1Support = getParameterValue(KEY_ENG_SENSOR_MODE_SLIM_VIDEO1_SUPPORTED, parameters);
            if ((silmV1Support == 1 && mCameraId == 0) || (silmV1Support == 2 && mCameraId == 1)
                    || silmV1Support == 3) {
                mCaptureSizeState[CAPTURE_SIZE_SLIM_VIDEO1] = true;
            } else {
                mCaptureSizeState[CAPTURE_SIZE_SLIM_VIDEO1] = false;
            }
            int silmV2Support = getParameterValue(KEY_ENG_SENSOR_MODE_SLIM_VIDEO2_SUPPORTED, parameters);
            if ((silmV2Support == 1 && mCameraId == 0) || (silmV2Support == 2 && mCameraId == 1)
                    || silmV2Support == 3) {
                mCaptureSizeState[CAPTURE_SIZE_SLIM_VIDEO2] = true;
            } else {
                mCaptureSizeState[CAPTURE_SIZE_SLIM_VIDEO2] = false;
            }
            int customSupport = getParameterValue(KEY_ENG_SENSOR_MODE_SUPPORTED, parameters);
            for (int i = 0; i < 5; i++) {
                mCaptureSizeState[CAPTURE_SIZE_CUSTOM_BASE + i] = (customSupport >> i & 1) == 1 ?
                        true : false;
            }
            updateCaptureSizeAdapter();

            // Verification
            if (!Util.getFeatureSupported(KEY_ENG_MTK_AWB_SUPPORTED, parameters)
                    && !Util.getFeatureSupported(KEY_ENG_SENSOR_AWB_SUPPORTED, parameters)) {
                mAwb.setVisibility(View.GONE);
                findViewById(R.id.auto_clibr_veri_awb_id).setVisibility(View.GONE);
            } else {
                mAwb.setVisibility(View.VISIBLE);
                findViewById(R.id.auto_clibr_veri_awb_id).setVisibility(View.VISIBLE);
                if (Util.getFeatureSupported(KEY_ENG_MTK_AWB_SUPPORTED, parameters)) {
                    findViewById(R.id.auto_clibr_veri_awb_mtk).setVisibility(View.VISIBLE);
                } else {
                    findViewById(R.id.auto_clibr_veri_awb_mtk).setVisibility(View.GONE);
                }
                if (Util.getFeatureSupported(KEY_ENG_SENSOR_AWB_SUPPORTED, parameters)) {
                    findViewById(R.id.auto_clibr_veri_awb_sensor).setVisibility(View.VISIBLE);
                } else {
                    findViewById(R.id.auto_clibr_veri_awb_sensor).setVisibility(View.GONE);
                }

            }

            initVideoRawDumpUi(parameters);
            initShadingUi(parameters);
            initNrUi(parameters);
            initSceneModeUi();

            List<String> vHdrList = getVHdrSupportedList(parameters);
            if (vHdrList == null) {
                findViewById(R.id.video_hdr_id).setVisibility(View.GONE);
            } else {
                findViewById(R.id.video_hdr_id).setVisibility(View.VISIBLE);
                // update vHDR list by return string array
                ArrayAdapter<String> vHdrAdapter = new ArrayAdapter<String>(this,
                        android.R.layout.simple_spinner_item, vHdrList);
                vHdrAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                mVHdrModeSpinner.setAdapter(vHdrAdapter);
            }
            findViewById(R.id.vhdr_set_id).setVisibility(View.GONE);

            if (parameters.getMaxNumDetectedFaces() <= 0) {
                mFaceDetectSpinner.setEnabled(false);
            }

            if (FALSE == parameters.get(KEY_ENG_GIS_CALIBRATION_SUPPORTED)) {
                mCommonCalibrationLv.setVisibility(View.GONE);
            } else {
                mCommonCalibrationLv.setVisibility(View.VISIBLE);
            }
        }
        // init sensor spinner
        if (!mIsInitialized) {
            List<String> sensorList = new ArrayList<String>();
            boolean isAddMain2 = false;
            boolean isAddSub2 = false;
            for (int i = 0; i < mAvailCameraNumber; i++) {
                CameraInfo info = new CameraInfo();
                android.hardware.Camera.getCameraInfo(i, info);
                if (info.facing == CameraInfo.CAMERA_FACING_BACK
                        && Camera.isRawSensor(CameraInfo.CAMERA_FACING_BACK, parameters)) {
                    if (i == 0) {
                       //avoid show several main sensor
                       sensorList.add(getString(R.string.auto_clibr_camera_sensor_main));
                       if (isImageRefocusSupported(parameters)) {
                           isAddMain2 = true;
                       }
                    } else {
                        isAddMain2 = true;
                    }
                } else if (info.facing == CameraInfo.CAMERA_FACING_FRONT
                        && Camera.isRawSensor(CameraInfo.CAMERA_FACING_FRONT, parameters)) {
                    if (i <= 1) {
                        sensorList.add(getString(R.string.auto_clibr_camera_sensor_sub));
                    } else {
                        isAddSub2 = true;
                    }
                } else {
                    Elog.w(TAG, "add raw sensor; Invalid facing info from CameraInfo.facing: "
                                + info.facing);
                }
            }
            if (isAddMain2) {
                sensorList.add(getString(R.string.auto_clibr_camera_sensor_main2));
            }
            if (isAddSub2) {
                sensorList.add(getString(R.string.auto_clibr_camera_sensor_sub2));
            }
            if (sensorList.size() == 0) {
                Toast.makeText(this, "no available raw sensor", Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
            ArrayAdapter<String> sensorAdapter = new ArrayAdapter<String>(this,
                                            android.R.layout.simple_spinner_item, sensorList);
            sensorAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mSensorSpinner.setAdapter(sensorAdapter);
            mSensorSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent,View view, int position, long id) {
                    Elog.i(TAG, "mSensorSpinner.onItemSelected; position" + position
                                + " mIsInitialized:" + mIsInitialized);
                    if (!mIsInitialized) {
                        return;
                    }
                    String tag = parent.getSelectedItem().toString();
                    int oriPos = mCameraId;
                    if (tag.equals(getString(R.string.auto_clibr_camera_sensor_main))) {
                        mCameraId = 0;
                        mStereoCameraId = 0;
                    } else if (tag.equals(getString(R.string.auto_clibr_camera_sensor_main2))) {
                        // For Stereo project, the device number is 2,
                        // so the mCameraId can not set as 2 since the device id should less than 2
                        // (can set 0,1); This may have problem since the framework use the camera
                        // id 0, and will use device 0 info, but mtkcam device will open 2 according
                        // to debug property("2"). If the Main1 and Main2 device info is different,
                        // it maybe have problem. And for single BackCamera2, the device number is
                        // 3, we should set the device id as 2 to make sure the framework use the
                        // device 2 info.
                        if (mAvailCameraNumber <= 2) {
                            mCameraId = 0;
                            mStereoCameraId = 2;
                        } else {
                            mCameraId = 2;
                            mStereoCameraId = 0;
                        }
                    } else if (tag.equals(getString(R.string.auto_clibr_camera_sensor_sub))) {
                        if (mAvailCameraNumber == 1) {
                            mCameraId = 0;
                            mStereoCameraId = 1;
                        } else {
                            mCameraId = 1;
                            mStereoCameraId = 1;
                        }
                    } else if (tag.equals(getString(R.string.auto_clibr_camera_sensor_sub2))) {
                        mCameraId = 3;
                        mStereoCameraId = 1;
                    }
                    Elog.i(TAG, "Camera ID:" + mCameraId + " Tag:" + tag);
                    safeUseCameraDevice(mCameraId, new CameraSafeUser() {
                        @Override
                        public void useCamera(android.hardware.Camera camera) {
                            initByCameraDeviceCapability(camera.getParameters());
                        }
                    });
                    setStatusTodefault();
                    if (oriPos != position) {
                        Toast.makeText(AutoCalibration.this, "Reset to default state!",
                                        Toast.LENGTH_SHORT).show();
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> arg0) {
                    Elog.d(TAG, "mSensorSpinner onNothingSelected()");
                }
            });
        }

        for(ConfigBase config : mConfigList) {
            config.initSupportedUi(parameters);
        }
    }

    private void removeHal1Feature() {
//        findViewById(R.id.normal_capture_mode_ll).setVisibility(View.GONE);
//        findViewById(R.id.normal_capture_set).setVisibility(View.GONE);
        findViewById(R.id.isp_profile_layout_id).setVisibility(View.GONE);
        findViewById(R.id.capture_ratio_layout_id).setVisibility(View.GONE);
        findViewById(R.id.capture_type_layout_id).setVisibility(View.GONE);
        findViewById(R.id.capture_number_layout_id).setVisibility(View.GONE);
        findViewById(R.id.iso_manual_switch_layout_id).setVisibility(View.GONE);
        findViewById(R.id.shutter_speed_layout_id).setVisibility(View.GONE);
        findViewById(R.id.sensor_gain_layout_id).setVisibility(View.GONE);

        findViewById(R.id.video_clip_raw_id).setVisibility(View.GONE);
        findViewById(R.id.video_capture_set).setVisibility(View.GONE);
        findViewById(R.id.listview_iso).setVisibility(View.GONE);
        findViewById(R.id.flicker_layout_id).setVisibility(View.GONE);
        findViewById(R.id.auto_calib_strobe_mode_ll).setVisibility(View.GONE);
        findViewById(R.id.shutter_delay_ll).setVisibility(View.GONE);
        findViewById(R.id.shading_table_ll).setVisibility(View.GONE);
        findViewById(R.id.output_sdblk_file_ll).setVisibility(View.GONE);
        findViewById(R.id.video_hdr_id).setVisibility(View.GONE);
        findViewById(R.id.vhdr_set_id).setVisibility(View.GONE);
        findViewById(R.id.multi_pass_nr_id).setVisibility(View.GONE);
        findViewById(R.id.mult_frame_capture).setVisibility(View.GONE);
        findViewById(R.id.mult_frame_capture_set).setVisibility(View.GONE);
        findViewById(R.id.layout_iso_interpolation).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_iso_inter_set).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_flash_ll).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_scene_mode_ll).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_face_detection_ll).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_veri_awb).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_veri_awb_id).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_veri_shading).setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_veri_shading_id).setVisibility(View.GONE);
        findViewById(R.id.ll_auto_clibr_ae_mode).setVisibility(View.GONE);
        findViewById(R.id.ll_auto_clibr_af_mode).setVisibility(View.GONE);
        findViewById(R.id.ll_auto_clibr_custom_parameter).setVisibility(View.GONE);
        findViewById(R.id.common_option_list).setVisibility(View.GONE);
        findViewById(R.id.ll_auto_clibr_capture).setVisibility(View.GONE);
        findViewById(R.id.ll_auto_clibr_property).setVisibility(View.GONE);
    }

    private void initSceneModeUi() {
        if (CameraJni.getEmcameraVersion() != CameraJni.MTK_EMCAMERA_VERSION1) {
            findViewById(R.id.auto_clibr_scene_mode_ll).setVisibility(View.GONE);
        }
    }
    private void initNrUi(Parameters parameters) {
        if (CameraJni.getEmcameraVersion() == CameraJni.MTK_EMCAMERA_VERSION1
                && Util.getFeatureSupported(KEY_ENG_MULTI_NR_SUPPORTED, parameters)) {
            findViewById(R.id.multi_pass_nr_id).setVisibility(View.VISIBLE);
        } else {
            findViewById(R.id.multi_pass_nr_id).setVisibility(View.GONE);
        }
    }

    private void initShadingUi(Parameters parameters) {
        if(CameraJni.getEmcameraVersion() != CameraJni.MTK_EMCAMERA_VERSION1) {
            findViewById(R.id.shading_table_ll).setVisibility(View.GONE);
        }
        if (CameraJni.getEmcameraVersion() != CameraJni.MTK_EMCAMERA_VERSION1
         || (!Util.getFeatureSupported(KEY_ENG_MTK_SHADING_SUPPORTED, parameters)
            && !Util.getFeatureSupported(KEY_ENG_MTK_1TO3_SHADING_SUPPORTED, parameters)
            && !Util.getFeatureSupported(KEY_ENG_SENSOR_SHADNING_SUPPORTED, parameters))) {
            mShading.setVisibility(View.GONE);
        findViewById(R.id.auto_clibr_veri_shading_id).setVisibility(View.GONE);
    } else {
        mShading.setVisibility(View.VISIBLE);
        findViewById(R.id.auto_clibr_veri_shading_id).setVisibility(View.VISIBLE);
        findViewById(R.id.auto_clibr_veri_shading_mtk).setVisibility(View.GONE);
        if (Util.getFeatureSupported(KEY_ENG_MTK_1TO3_SHADING_SUPPORTED, parameters)) {
            findViewById(R.id.auto_clibr_veri_shading_mtk_1to3).setVisibility(View.VISIBLE);
        } else {
            findViewById(R.id.auto_clibr_veri_shading_mtk_1to3).setVisibility(View.GONE);
        }
        if (Util.getFeatureSupported(KEY_ENG_SENSOR_SHADNING_SUPPORTED, parameters)) {
            findViewById(R.id.auto_clibr_veri_shading_mtk_sensor).setVisibility(View.VISIBLE);
        } else {
            findViewById(R.id.auto_clibr_veri_shading_mtk_sensor).setVisibility(View.GONE);
        }
    }}
    private void initVideoRawDumpUi(Parameters parameters) {
        if (isSupportedVideoClipRaw(parameters)
                && CameraJni.getEmcameraVersion() == CameraJni.MTK_EMCAMERA_VERSION1) {
            findViewById(R.id.video_clip_raw_id).setVisibility(View.VISIBLE);
            // Video Raw Dump - Multi Frame Rate
            String[] videoResolution = AutoCalibration.this.getResources().getStringArray(
                    R.array.auto_calib_video_clip);
            List<String> videoResolutionList = new ArrayList<String>();
            for (int i = 0; i < videoResolution.length; i++) {
                videoResolutionList.add(videoResolution[i]);
            }
            if (Util.getFeatureSupported(KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_2M_SUPPORTED, parameters)) {
                videoResolutionList.add(AutoCalibration.this.getString(
                        R.string.auto_clibr_video_clip_2M));
            }
            if (Util.getFeatureSupported(KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_4K2K_SUPPORTED, parameters)) {
                videoResolutionList.add(AutoCalibration.this.getString(
                        R.string.auto_clibr_video_clip_2K4K));
            }
            if (Util.getFeatureSupported(KEY_ENG_VIDEO_RAW_DUMP_CROP_CENTER_2M_SUPPORTED, parameters)) {
                videoResolutionList.add(AutoCalibration.this.getString(
                        R.string.auto_clibr_video_clip_center_2M));
            }
            ArrayAdapter<String> adp = new ArrayAdapter<String>(AutoCalibration.this,
                    android.R.layout.simple_spinner_item,
                    videoResolutionList);
            adp.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mVideoClipResolution.setAdapter(adp);
            if (!Util.getFeatureSupported(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED, parameters)) {
                mVideoFrameRateLayout.setVisibility(View.GONE);
                mVideoFrameRateSupport = false;
            } else {
                mVideoFrameRateLayout.setVisibility(View.VISIBLE);
                mVideoFrameRateSupport = true;
                mVideoFrameRateMin = getParameterValue(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MIN,
                        parameters);
                mVideoFrameRateMax = getParameterValue(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MAX,
                        parameters);
            }
        } else {
            findViewById(R.id.video_clip_raw_id).setVisibility(View.GONE);
        }
    }
    private void initByCameraDeviceCapability(Parameters parameters) {
        initSupportedUi(parameters);

    }

    private interface CameraSafeUser {
        void useCamera(android.hardware.Camera camera);
    }

    int safeUseCameraDevice(int cameraId, CameraSafeUser user) {
        android.hardware.Camera cameraDevice = null;
        if (mStereoCameraId > 1) {
            Elog.d(TAG, "enabled main2/sub2");
            openStereoSensor(true);
        }
        try {
            cameraDevice = android.hardware.Camera.open(cameraId);
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (cameraDevice == null) {
            Elog.e(TAG, "Open Camera fail; ");
            Toast.makeText(this, "fail to open camera", Toast.LENGTH_SHORT).show();
            return -1;
        }
        user.useCamera(cameraDevice);
        cameraDevice.release();
        if (mStereoCameraId > 1) {
            Elog.d(TAG, "disabled main2/sub2");
            openStereoSensor(false);
        }
        return 0;
    }

    private void openStereoSensor(boolean enabled) {
        if (enabled) {
            String enabledStr = String.valueOf(mStereoCameraId);
            Elog.d(TAG, "vendor.debug.camera.open, enable :" + mStereoCameraId);
            Util.setProperty("vendor.debug.camera.open", enabledStr);
        } else {
            Util.setProperty("vendor.debug.camera.open", "-1");
        }
    }

    private void setStatusTodefault() {
        if (!isCameraHal1()) {
            mNormalCaptureSize.setSelection(1);
            return;
        }
        Elog.v(TAG, "setStatusTodefault()");
        mCaptureMode = 0;
        mNormalMode.setChecked(true);
        resetISOValue();
        mAfMode = Camera.AF_MODE_AUTO;
        mAfAuto.setChecked(true);
        mAfModeStatus = true;
        mAfBracketRange.setText("0");
        mAfThroughInterval.setText("1");
        mNormalCaptureNum.setText("1");
        mShutterSpeed.setText("0");
        mSensorGain.setText("0");
        mVideoFrameMinRate.setText("15");
        mVideoFrameMaxRate.setText("30");
        mIsoInterSpinner.setSelection(0);
        mIsoRatioEdit.setText("10"); // set 10% as default.
        mThroughFocsuStartPos.setText("0");
        mThroughFocsuEndPos.setText("1023");
        mThroughFocusStart.setVisibility(View.GONE);
        mThroughFocusEnd.setVisibility(View.GONE);

        List<String> throughRepeatList = new ArrayList<String>();
        for (int i = 0; i < 8; i++) {
            throughRepeatList.add(String.valueOf(i + 1));
        }
        ArrayAdapter<String> afThroughRepeatAdapter = new ArrayAdapter<String>(this,
                                        android.R.layout.simple_spinner_item, throughRepeatList);
        afThroughRepeatAdapter.setDropDownViewResource(
                                        android.R.layout.simple_spinner_dropdown_item);
        mAfThroughRepeat.setAdapter(afThroughRepeatAdapter);

        mAfFullScanFrameInterval.setText("1");
        mAfFullScanDacStep.setText("1");
        mAfFullScanFocusWinWidth.setText("18");
        mAfFullScanFocusWinHeight.setText("24");
        mAfAdvacedFullScan.setChecked(false);
        mAfFullScanRepeat.setText("10");
        // set default Capture size = capture size
        mNormalCaptureSize.setSelection(1);
        spinnerNormalIspProfile.setSelection(1); // Capture
        spinnerVideoIspProfile.setSelection(1); // video
        spineerVideoSensorMode.setSelection(1); // video
        spinnerRawDumpBuffer.setSelection(0); // IMGO
        // set default Capture ratio = 4:3
        mCaptureRatioStandard.setChecked(true);
        // set default Capture type = pure raw
        mNormalCaptureType.setSelection(1);
        mVideoFrameRateSpinner.setSelection(0);
        mVideoHdrSpinner.setSelection(0); // off
        mMultiNrSpinner.setSelection(0);
        mSdblkSpinner.setSelection(0);
        mShadingSpinner.setSelection(4);
        mShutterDelaySpinner.setSelection(0);
        mFlickerSpinner.setSelection(0);
        mAwbCkBoxs[0].setChecked(true);
        mAwbCkBoxs[1].setChecked(false);

        for (int i = 0; i < 3; i++) {
            mShadingCkBoxs[i].setChecked(true); // default ON as AE request
        }
        mSceneModeSpinner.setSelection(0);
        mFlashCalibSpinner.setSelection(0);
        mFaceDetectSpinner.setSelection(0);
        spinnerAeMode.setSelection(0);
        aeSettings.reset();
        etMinusEv.setText(String.valueOf(aeSettings.getMinus()));
        etPlusEv.setText(String.valueOf(aeSettings.getPlus()));
        etEvInterval.setText(String.valueOf(aeSettings.getInterval()));
        // manual iso
        etISOManual.setText("0");

        spinnerISOManualSwitch.setSelection(0);
        for(ConfigBase config : mConfigList) {
            config.setStatusToDefault();
        }
    }

    private int getShutterDelayFromUi() {
        String strVal = mShutterDelaySpinner.getSelectedItem().toString();
        int endPos = strVal.indexOf("s");
        if (endPos > 0) {
            strVal = strVal.substring(0, endPos);
        }
        strVal = strVal.trim();
        int val = 0;
        try {
            val = Integer.valueOf(strVal);
        } catch (NumberFormatException e) {
            Elog.e(TAG, "NumberFormatException:" + e.getMessage() + " strVal:" + strVal);
        }
        return val;
    }

    private int getShadingTableFromUi() {
        int selectedIdx = mShadingSpinner.getSelectedItemPosition();
        return selectedIdx;
    }

    private int getOutputSdblkFromUi() {
        return 1;
    }

    private boolean putValuesToPreference(String key) {
        final SharedPreferences preferences = getSharedPreferences(key,
                android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        if(!isCameraHal1()) {
            int pos = mNormalCaptureSize.getSelectedItemPosition();
            editor.putInt(getString(R.string.auto_clibr_key_capture_size), pos);
            editor.putInt(getString(R.string.camera_key_selected_sensor_id), mCameraId);
            editor.putInt(getString(R.string.auto_clibr_camera_sensor_main2), mStereoCameraId);
            editor.commit();
            return true;
        }

        editor.putInt(getString(R.string.auto_clibr_key_capture_mode), mCaptureMode);
        int value;
        int ratio; // 0 means 4:3, 1 means 16:9
        if (mCaptureMode == 0) {
            int pos = spinnerNormalIspProfile.getSelectedItemPosition();
            putInPreferenceIsp(editor, R.string.auto_clibr_isp_profile,
                                R.array.auto_calib_capture_isp_profile, pos);
            pos = mNormalCaptureSize.getSelectedItemPosition();
            String size = (String) mNormalCaptureSize.getItemAtPosition(pos);
            for (int i = 0; i < CAPTURE_SIZE_NUM; i++) {
                if (size.equals(mCaptureSize[i])) {
                    pos = i;
                }
            }

            editor.putInt(getString(R.string.auto_clibr_key_capture_size), pos);
            if (mCaptureRatioStandard.isChecked()) {
                ratio = 0;
            } else if (mCaptureRatioFull.isChecked()) {
                ratio = 1;
            } else {
                ratio = 0;
                Elog.w(TAG, "[putValuesToPreference] capture ratio error!");
            }
            Elog.i(TAG, "[putValuesToPreference] capture ratio value: " + ratio);
            editor.putInt(getString(R.string.auto_clibr_capture_ratio), ratio);
            editor.putInt(getString(R.string.auto_clibr_key_capture_type),
                    mNormalCaptureType.getSelectedItemPosition());
            value = Util.getIntegerFromEdit(mNormalCaptureNum, 1, true, 100, true);
            if (value == -1) {
                Toast.makeText(this, R.string.auto_clibr_capture_number_range_tip,
                                Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_capture_number), value);
            value = Util.getIntegerFromEdit(mShutterSpeed, 0, true, 1000000, true);
            if (value == -1) {
                Toast.makeText(this, R.string.capture_shutter_range_tip, Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_capture_shutter_speed), value);
            value = Util.getIntegerFromEdit(mSensorGain, 0, true, 1000000, true);
            if (value == -1) {
                Toast.makeText(this, R.string.capture_sensor_range_tip, Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_capture_sensor_gain), value);
        } else {
            String SelectedItem = spinnerRawDumpBuffer.getSelectedItem().toString();
            if (SelectedItem.equals("IMGO")) {
                putInPreference(editor, R.string.auto_clibr_video_dump_buffer, 0);
            } else if (SelectedItem.equals("RRZO")) {
                putInPreference(editor, R.string.auto_clibr_video_dump_buffer, 1);
            }

            int pos = spinnerVideoIspProfile.getSelectedItemPosition();
            putInPreferenceIsp(editor, R.string.auto_clibr_isp_profile,
                                R.array.auto_calib_video_isp_profile, pos);
            pos = spineerVideoSensorMode.getSelectedItemPosition();
            putInPreferenceSensor(editor, R.string.auto_clibr_sensor_mode,
                                R.array.auto_calib_video_sensor_mode, pos);
            pos = mVideoClipResolution.getSelectedItemPosition();
            String clip = (String) mVideoClipResolution.getItemAtPosition(pos);
            if (clip.equals(getString(R.string.auto_clibr_video_clip_2M))) {
                pos = 2;
            } else if (clip.equals(getString(R.string.auto_clibr_video_clip_2K4K))) {
                pos = 3;
            } else if (clip.equals(getString(R.string.auto_clibr_video_clip_center_2M))) {
                pos = 4;
            }
            editor.putInt(getString(R.string.auto_clibr_key_capture_resolution), pos);
            if (mVideoFrameRateSupport) {
                editor.putInt(getString(R.string.auto_clibr_video_frame_enable),
                        mVideoFrameRateSpinner.getSelectedItemPosition());

                value = Util.getIntegerFromEdit(mVideoFrameMinRate, mVideoFrameRateMin, true,
                                            mVideoFrameRateMax, true);
                if (value == -1) {
                    Toast.makeText(this, R.string.auto_clibr_video_frame_range_tip,
                                    Toast.LENGTH_LONG).show();
                    return false;
                }
                editor.putInt(getString(R.string.auto_clibr_video_frame_low), value);
                int highValue = Util.getIntegerFromEdit(mVideoFrameMaxRate, mVideoFrameRateMin, true,
                        mVideoFrameRateMax, true);
                if (highValue == -1 || highValue < value) {
                    Toast.makeText(this, R.string.auto_clibr_video_frame_range_tip,
                                    Toast.LENGTH_LONG).show();
                    return false;
                }
                editor.putInt(getString(R.string.auto_clibr_video_frame_high), highValue);
            }
        }
        editor.putInt(getString(R.string.auto_clibr_key_flicker),
                        mFlickerSpinner.getSelectedItemPosition());
        // editor.putInt(getString(R.string.auto_clibr_key_pre_flash),
        // mPreFlashSpinner.getSelectedItemPosition() + 1);
        // editor.putInt(getString(R.string.auto_clibr_key_main_flash),
        // mMainFlashSpinner.getSelectedItemPosition() + 1);
        editor.putInt(getString(R.string.auto_clibr_key_af_mode), mAfMode);
        if (mAfMode == Camera.AF_MODE_BRACKET) {
            editor.putInt(getString(R.string.auto_clibr_key_branket_interval),
                    Integer.valueOf(mAfBracketInterval.getSelectedItem().toString()));
            value = Util.getIntegerFromEdit(mAfBracketRange, 0, true, 511, true);
            if (value == -1) {
                Toast.makeText(this, R.string.auto_clibr_af_bracket_range_tip,
                                Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_branket_range), value);
        } else if (mAfMode == Camera.AF_MODE_FULL_SCAN) {
            value = Util.getIntegerFromEdit(mAfFullScanFrameInterval, mMinFullScanFrameInterval,
                                        true, mMaxFullScanFrameInterval, true);
            if (value == -1) {
                Toast.makeText(this, "Frame inverval range is " + mMinFullScanFrameInterval + " to "
                        + mMaxFullScanFrameInterval, Toast.LENGTH_SHORT).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_full_frame_interval), value);
            value = Util.getIntegerFromEdit(mAfFullScanDacStep, 1, true, mMaxFullScanFrameInterval, true);
            if (value == -1) {
                Toast.makeText(this, "Full scan dac step range is 1 " +
                        " to " + mMaxFullScanFrameInterval, Toast.LENGTH_SHORT).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_full_scan_dac_step), value);
            value = Util.getIntegerFromEdit(mAfFullScanFocusWinWidth, 1, true, 100, true);
            if (value == -1) {
                Toast.makeText(this, "Full Scan Focus win width range is 1 to 100",
                        Toast.LENGTH_SHORT).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_full_scan_focus_win_width), value);
            value = Util.getIntegerFromEdit(mAfFullScanFocusWinHeight, 1, true, 100, true);
            if (value == -1) {
                Toast.makeText(this, "Full Scan Focus win height range is 1 to 100",
                        Toast.LENGTH_SHORT).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_full_scan_focus_win_height), value);
            editor.putBoolean(getString(R.string.auto_clibr_key_advanced_full_scan),
                    mAfAdvacedFullScan.isChecked());
            if(mAfAdvacedFullScan.isChecked()) {
                value = getIntegerFromEditCross(mAfFullScanRepeat, 1, true, 20, true,
                        "Full scan repeat");
                if (value == -1) {
                    Toast.makeText(this, "Full scan repeat range is 1 to 20",
                            Toast.LENGTH_SHORT).show();
                    return false;
                }
                editor.putInt(getString(R.string.auto_clibr_key_full_scan_repeat), value);
            }
        } else if (mAfMode == Camera.AF_MODE_THROUGH_FOCUS) {
            editor.putInt(getString(R.string.auto_clibr_key_through_focus_dirct),
                    mAfThroughDirec.getSelectedItemPosition());
            if (mAfThroughDirec.getSelectedItemPosition() == 2) {
                editor.putInt(getString(R.string.auto_clibr_key_through_manual_start_pos),
                        Integer.valueOf(mThroughFocsuStartPos.getText().toString()));
                editor.putInt(getString(R.string.auto_clibr_key_through_manual_end_pos),
                        Integer.valueOf(mThroughFocsuEndPos.getText().toString()));
            }
            value = Util.getIntegerFromEdit(mAfThroughInterval, 1, true, 511, true);
            if (value == -1) {
                Toast.makeText(this, R.string.auto_clibr_af_through_interval_tip,
                                Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.auto_clibr_key_through_focus_interval), value);
    
            String strRepeat = mAfThroughRepeat.getSelectedItem().toString();
            editor.putInt(getString(R.string.auto_clibr_key_through_repeat),Integer.valueOf(strRepeat));
        }

        editor.putString(getString(R.string.camera_key_custom_parameter), collectCustomParams());
        editor.putInt(getString(R.string.camera_key_selected_sensor_id), mCameraId);
        editor.putInt(getString(R.string.camera_key_shutter_delay), getShutterDelayFromUi());
        editor.putInt(getString(R.string.camera_key_shading_table), getShadingTableFromUi());//phaseout version2
        editor.putInt(getString(R.string.camera_key_output_sdblk), getOutputSdblkFromUi());//phaseout version2
        // isChecked())
        int awbVeri = 0;
        awbVeri |= mAwbCkBoxs[0].isChecked() ? (1 << 0) : 0;
        awbVeri |= mAwbCkBoxs[1].isChecked() ? (1 << 1) : 0;
        editor.putInt(getString(R.string.auto_clibr_verification_awb), awbVeri);
        int shadingVeri = 0;
        shadingVeri |= mShadingCkBoxs[0].isChecked() ? (1 << 0) : 0;
        shadingVeri |= mShadingCkBoxs[1].isChecked() ? (1 << 1) : 0;
        shadingVeri |= mShadingCkBoxs[2].isChecked() ? (1 << 2) : 0;
        editor.putInt(getString(R.string.auto_clibr_verification_shading), shadingVeri);//phaseout version2
        editor.putInt(getString(R.string.video_hdr), mVideoHdrSpinner.getSelectedItemPosition());
        if (mVideoHdrSpinner.getSelectedItemPosition() == 1) {
            // set mode
            int pos = mVHdrModeSpinner.getSelectedItemPosition();
            String mode = (String) mVHdrModeSpinner.getItemAtPosition(pos);
            editor.putString(getString(R.string.vhdr_set_mode), mode);
            // set ratio (1000<= ration <= 9999)
            value = Util.getIntegerFromEdit(mVHdrRatio, 100, true, 9999, true);
            if (value == -1) {
                Toast.makeText(this, R.string.vhdr_ratio_range_tip, Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(getString(R.string.vhdr_set_ratio), value);
            // http://wiki.mediatek.inc/display/MM3SW6/HDR+Detection#HDRDetection-Description
            // SCENE_MODE_HDR only used for HDR on
            editor.putInt(getString(R.string.auto_clibr_scene_mode),
                    Camera.SCENE_MODE_HDR);
        } else {
            editor.putInt(getString(R.string.auto_clibr_scene_mode),
                    mSceneModeSpinner.getSelectedItemPosition());
        }
        editor.putInt(getString(R.string.multi_pass_nr), mMultiNrSpinner.getSelectedItemPosition());//phaseout version2
        editor.putInt(getString(R.string.auto_clibr_face_detection),
                mFaceDetectSpinner.getSelectedItemPosition());
        editor.putInt(getString(R.string.auto_clibr_camera_sensor_main2), mStereoCameraId);
        editor.putInt(getString(R.string.auto_clibr_flash_calibration),
                                mFlashCalibSpinner.getSelectedItemPosition());
        // put in Ae Setting
        if (setPrefAeMode(editor) == false) {
            return false;
        }

        for(ConfigBase config : mConfigList) {
            if(config.saveValues(editor) == false) {
                return false;
            }
        }
        // put in ISO value
        if (!isManualISO) {
            if (mAfAuto.isChecked() && mNormalMode.isChecked()) {
                putStrInPreference(editor, R.string.auto_clibr_key_iso_speed,
                        getIsoArrayValue(mMulISOFlags));
            } else {
                putStrInPreference(editor, R.string.auto_clibr_key_iso_speed,
                        ISO_STRS_ARRAY[mAfSpecialIso]);
            }
        } else {
            putStrInPreference(editor, R.string.auto_clibr_key_iso_speed
                    , etISOManual.getText().toString());
            Elog.i(TAG, "@putValuesToPreference, manual ISO value: "
                        + etISOManual.getText().toString());
        }

        editor.commit();
        return true;
    }

    private int getIntegerFromEditCross(EditText edit, int from, boolean includeFrom,
                                    int end,boolean includeEnd, String title) {
        String editStr = edit.getText().toString();
        int value = -1;
        try {
            value = Integer.valueOf(editStr);
        } catch (NumberFormatException e) {
            Elog.w(TAG, "getIntegerFromEditTrunc NumberFormatException:" + e.getMessage());
        }

        boolean isCrossed = false;
        if (value < from && includeFrom) {
            value = from;
            Elog.w(TAG, "getIntegerFromEditTrunc, value is smaller than min value!" + value);
            isCrossed = true;
        } else if (value > end && includeEnd) {
            value = end;
            Elog.w(TAG, "getIntegerFromEditTrunc, value is larger than max value!" + value);
            isCrossed = true;
        } else if (!includeFrom && (value <= from)) {
            value = from + 1;
            Elog.w(TAG, "getIntegerFromEditTrunc, value is smaller than min value!" + value);
            isCrossed = true;
        } else if (!includeEnd && (value >= end)) {
            value = end - 1;
            Elog.w(TAG, "getIntegerFromEditTrunc, value is larger than max value!" + value);
            isCrossed = true;
        }
        if(isCrossed) {
            edit.setText(String.valueOf(value));
            value = -1;
        }
        return value;
    }
    private String getIsoArrayValue(boolean[] array) {
        String result = "";
        int min = array.length;
        int max = 0;

        for (int i = ISO_AUTO_ID; i < array.length; i++) {
            if (array[i]) {
                result += ISO_STRS_ARRAY[i - ISO_AUTO_ID] + ",";
            }
        }
        for (int i = ISO_AUTO_ID + 1; i < array.length; i++) {
            if (array[i]) {
                if (i < min) {
                    min = i; // find min iso index except "auto".
                }
                max = i; // find max iso index
            }
        }
        if (result.length() == 0) {
            Toast.makeText(this, R.string.auto_clibr_iso_tips, Toast.LENGTH_LONG);
            return "0,";
        }
        if (mIsoInterSpinner.getSelectedItemPosition() == 0 || max <= min) {
            return result; // iso interpolation off, use iso values directly
        } else {
            String editStr = mIsoRatioEdit.getText().toString();
            int ratio = -1;
            int min_iso = 0;
            int max_iso = 0;
            try {
                ratio = Integer.valueOf(editStr);
                min_iso = Integer.valueOf(ISO_STRS_ARRAY[min - ISO_AUTO_ID]);
                max_iso = Integer.valueOf(ISO_STRS_ARRAY[max - ISO_AUTO_ID]);
            } catch (NumberFormatException e) {
                Elog.w(TAG, "getIntegerFromEdit() NumberFormatException:" + e.getMessage());
                ratio = -1;
            }
            if (ratio == -1) {
                return result;
            }
            if (array[ISO_AUTO_ID]) { // add AUTO
                result = ISO_STRS_ARRAY[0] + ",";
            } else {
                result = "";
            }
            int iso = min_iso;
            while (iso <= max_iso) {
                result += iso + ",";
                iso += min_iso * ratio / 100;
            }
            return result;
        }
    }

    private void statusChangesByAf(boolean afStatus) {
        // mPreFlashSpinner.setEnabled(afStatus);
        // mMainFlashSpinner.setEnabled(afStatus);
        mStrobeConfig.setStrobeState(StrobeConfig.OFF); // off
        mStrobeConfig.setSpinnerEnabled(afStatus);
        mVideoCliplMode.setEnabled(afStatus);
        mNormalCaptureNum.setEnabled(afStatus);
        if (mCaptureMode != 0) {
            mNormalMode.setChecked(true);
        }
        if (!isManualISO) {
            if (afStatus) {
                resetISOValue();
            } else {
                mAfSpecialIso = 0;
            }
        }
    }

    private void putInPreference(SharedPreferences.Editor editor, int keyId, int value) {
        String key = getString(keyId);
        Elog.i(TAG, "putInPreference key: " + key + ",value: " + value);
        try {
            editor.putInt(key, value);
        } catch (NullPointerException ne) {
            Elog.i(TAG, ne.getMessage());
        }
    }

    private void putStrInPreference(SharedPreferences.Editor editor, int keyId, String value) {
        String key = getString(keyId);
        Elog.i(TAG, "putInPreference key: " + key + ",value: " + value);
        try {
            editor.putString(key, value);
        } catch (NullPointerException ne) {
            Elog.i(TAG, ne.getMessage());
        }
    }

    private void setLvHeight(ListView listView) {
        ListAdapter adapter = listView.getAdapter();
        if (adapter == null) {
            Elog.v(TAG, "no data in ListView");
            return;
        }
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listView);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        ViewGroup.LayoutParams layoutParams = listView.getLayoutParams();
        layoutParams.height = totalHeight
                + (listView.getDividerHeight() * (adapter.getCount() - 1));
        listView.setLayoutParams(layoutParams);
    }

    private int getParameterValue(String para, Parameters parameters) {
        int val = 0;
        try {
            val = parameters.getInt(para);
        } catch (NumberFormatException ex) {
//            Elog.d(TAG, "getParameterValue - " + para + " is NumberFormatException");
        }
        Elog.v(TAG, "getParameterValue - " + para + " is " + val);
        return val;
    }

    private ArrayList<String> getVHdrSupportedList(Parameters parameters) {
        String str = parameters.get(KEY_ENG_VIDEO_HDR_SUPPORTED);
        if (str == null) {
            return null;
        }
        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> supportedList = new ArrayList<String>();
        for (String s : splitter) {
            if (s != null) {
                supportedList.add(s);
                Elog.d(TAG, "getVHDRSupportedList - " + s);
            }
        }
        // workaround util low layer is ready, then replace to 0.
        if (supportedList.size() < 1) {
            return null;
        }
        return supportedList;
    }

    private boolean isImageRefocusSupported(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_IMAGE_REFOCUS_SUPPORTED);
        Elog.d(TAG, "isImageRefocusSupported - " + str);
        if ("off".equals(str) || null == str) {
            return false;
        } else {
            return true;
        }
    }

    private boolean isSupportedVideoClipRaw(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_ENG_VIDEO_RAW_DUMP_SUPPORTED);
        Elog.d(TAG, "isSupportedVideoClipRaw - " + str);
        if (FALSE.equals(str)) {
            return false;
        } else {
            return true;
        }
    }

    @Override
    public void onClick(View v) {
        int buttonId = v.getId();

        switch (buttonId) {
        case R.id.setBtn:
            handleSysProp(true);
            break;
        case R.id.getBtn:
            handleSysProp(false);
            break;
        default:
            break;
        }
    }

    private void handleSysProp(Boolean isSet) {

        String name = mSysPropName.getText().toString();
        String value = mSysPropValue.getText().toString();

        if (isSet) {
            Elog.i(TAG, "name:" + name + "/" + value);

            Util.setProperty(name, value);
        }
            String result = Util.getProperty(name);
            Elog.i(TAG, "getProperty result:" + result);
            mSysPropValue.setText(result);
            mOutputScreen.setText((isSet ? "Set OK: " : "Get OK: ") + name + " = " + result);
    }

    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        Elog.i(TAG, "onRequestPermissionsResult(), requestCode = " + requestCode);
        if (grantResults == null || grantResults.length <= 0) {
            return;
        }
        if (mPermissionManager.getCameraLaunchPermissionRequestCode() == requestCode) {
            if (mPermissionManager.isCameraLaunchPermissionsResultReady(permissions, grantResults)) {
                // permission was granted open camera
                safeUseCameraDevice(mCameraId, new CameraSafeUser() {
                    @Override
                    public void useCamera(android.hardware.Camera camera) {
                        initByCameraDeviceCapability(camera.getParameters());
                    }
                });
                setStatusTodefault();
                mIsInitialized = true;
            } else {
                // more than one critical permission was denied activity finish, exit and destroy
                Toast.makeText(this, R.string.denied_required_permission,
                        Toast.LENGTH_LONG).show();
                finish();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    private boolean isNeedSwitchHal() {
        return ROPERTY_CAMERA_HAL_VAL_3.equals(Util.getProperty(ROPERTY_CAMERA_HAL_KEY));
    }

    static boolean isCameraHal1 () {
        return CameraJni.getCamHalVersion() == 1
                || ROPERTY_CAMERA_HAL_VAL_1.equals(Util.getProperty(ROPERTY_CAMERA_HAL_KEY));
    }
}
