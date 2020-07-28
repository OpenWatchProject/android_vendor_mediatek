package com.mediatek.emcamera.config;

import java.util.List;

import android.app.Activity;
import android.content.SharedPreferences.Editor;
import android.hardware.Camera.Parameters;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Toast;
import android.widget.AdapterView.OnItemSelectedListener;

import com.mediatek.emcamera.CameraJni;
import com.mediatek.emcamera.Elog;
import com.mediatek.emcamera.R;
import com.mediatek.emcamera.Util;

public class MfllConfig extends ConfigBase {
    private static final String TAG = MfllConfig.class.getSimpleName();
    private static final String KEY_ENG_MFLL_SUPPORTED = "eng-mfll-s";

    public static final int AUTO = 0;
    public static final int ON = 1;
    public static final int OFF = 2;

    private Spinner mMulFrameMode;
    private LinearLayout mMulFrameLayout;
    private LinearLayout mMulFrameSetLayout;
    private EditText mMulFrameCaptureNum;
    private Listener mListener;

    public MfllConfig(Activity activity) {
        super(activity);
    }

    public interface Listener {
        void updateUi(int state);
    }

    public void registerListener(Listener listener) {
        mListener = listener;
    }
    public void initComponents() {
        mMulFrameLayout = (LinearLayout) mActivity.findViewById(R.id.mult_frame_capture);
        mMulFrameSetLayout = (LinearLayout) mActivity.findViewById(R.id.mult_frame_capture_set);
        mMulFrameCaptureNum = (EditText) mActivity.findViewById(R.id.mult_capture_number);
        mMulFrameMode = (Spinner) mActivity.findViewById(R.id.auto_clibr_multi_frame);
        mMulFrameMode.setOnItemSelectedListener(new OnItemSelectedListener() {


            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 == AUTO) { // MFLL - AUTO
                    mMulFrameSetLayout.setVisibility(View.GONE);
                    mListener.updateUi(AUTO);
                } else if (arg2 == ON) { // MFLL - ON
                    mMulFrameSetLayout.setVisibility(View.VISIBLE);
                    mListener.updateUi(ON);
                } else {
                    mMulFrameSetLayout.setVisibility(View.GONE);
                    mListener.updateUi(OFF);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.d(TAG, "MulFrame - select nothing.");
            }

        });
    }

    @Override
    public void initSupportedUi(Parameters parameters) {
        if (CameraJni.getEmcameraVersion() != CameraJni.MTK_EMCAMERA_VERSION1
            || !Util.getFeatureSupported(KEY_ENG_MFLL_SUPPORTED, parameters)
                || CameraJni.getCamHalVersion() != 1) {
            mMulFrameLayout.setVisibility(View.GONE);
        } else {
            mMulFrameLayout.setVisibility(View.VISIBLE);
        }
        mMulFrameSetLayout.setVisibility(View.GONE);
    }

    @Override
    public boolean saveValues(Editor editor) {
        int mfllMode = mMulFrameMode.getSelectedItemPosition();
        editor.putInt(mActivity.getString(R.string.auto_clibr_key_mfll_enable), mfllMode);
        if(mfllMode == AUTO) {
            editor.putInt(mActivity.getString(R.string.auto_clibr_key_mfll_count), 0);
        } else if(mfllMode == ON) {
            int value = Util.getIntegerFromEdit(mMulFrameCaptureNum, 1, true, 8, true);
            if (value == -1) {
                Toast.makeText(mActivity, R.string.auto_clibr_mfll_count_range_tip,
                                Toast.LENGTH_LONG).show();
                return false;
            }
            editor.putInt(mActivity.getString(R.string.auto_clibr_key_mfll_count), value);
        }

        return true;
    }

    @Override
    public void setStatusToDefault() {
        mMulFrameCaptureNum.setText("6"); // current only support 6
        mMulFrameMode.setSelection(AUTO);
    }
}
