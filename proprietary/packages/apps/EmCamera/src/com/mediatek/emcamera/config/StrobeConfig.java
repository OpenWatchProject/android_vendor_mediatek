package com.mediatek.emcamera.config;

import java.util.List;

import android.app.Activity;
import android.content.SharedPreferences.Editor;
import android.hardware.Camera.Parameters;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;

import com.mediatek.emcamera.CameraJni;
import com.mediatek.emcamera.Elog;
import com.mediatek.emcamera.R;

public class StrobeConfig extends ConfigBase {
    private static final String TAG = StrobeConfig.class.getSimpleName();
    public static final int AUTO = 0;
    public static final int ON = 1;
    public static final int OFF = 2;
    private Spinner mStrobeModeSpinner;
    private LinearLayout mStrobeModeLayout;
    private int mAutoIndex;
    private int mOnIndex;
    private int mOffIndex;

    public StrobeConfig(Activity activity) {
        super(activity);
    }

    public void initComponents() {
        mStrobeModeLayout = (LinearLayout) mActivity.findViewById(R.id.auto_calib_strobe_mode_ll);
        mStrobeModeSpinner = (Spinner) mActivity.findViewById(R.id.led_flash);
    }

    @Override
    public void initSupportedUi(Parameters parameters) {
        List<String> strobeList = getSupportedFlashModes(parameters);
        if (CameraJni.getCamHalVersion() != 1 || strobeList == null) {
            mStrobeModeLayout.setVisibility(View.GONE);
        } else {
            mStrobeModeLayout.setVisibility(View.VISIBLE);
            // update strobe list by return string array
            getDefaultIndex(strobeList);
            ArrayAdapter<String> strobeAdapter = new ArrayAdapter<String>(mActivity,
                    android.R.layout.simple_spinner_item, strobeList);
            strobeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mStrobeModeSpinner.setAdapter(strobeAdapter);
        }

    }

    private List<String> getSupportedFlashModes(Parameters parameters) {
        List<String> list = parameters.getSupportedFlashModes();
        if (null == list) {
            return null;
        } else {
            if (list.size() == 1 && list.get(0).equals(Parameters.FLASH_MODE_OFF)) {
                return null;
            }
        }
        return list;
    }


    private int getDefaultIndex(List<String> strobeList) {
        int size = strobeList.size();
        for (int i = 0; i < size; ++i) {
            String value = strobeList.get(i);
            if ("auto".equalsIgnoreCase(value)) {
                mAutoIndex = i;
            } else if ("on".equalsIgnoreCase(value)) {
                mOnIndex = i;
            } else if("off".equalsIgnoreCase(value)) {
                mOffIndex = i;
            }
        }
        return 0;
    }

    @Override
    public boolean saveValues(Editor editor) {
        Elog.v(TAG, "get Selected Item Position = " + mStrobeModeSpinner.getSelectedItem());
        editor.putString(mActivity.getString(R.string.auto_clibr_key_led_flash),
                mStrobeModeSpinner.getSelectedItem().toString());
        return true;
    }

    @Override
    public void setStatusToDefault() {
        mStrobeModeSpinner.setSelection(mOffIndex);
    }

    public void setStrobeState(int state) {
        if(state == AUTO) {
            mStrobeModeSpinner.setSelection(mAutoIndex);
        } else if(state == ON){
            mStrobeModeSpinner.setSelection(mOnIndex);
        } else if(state == OFF){
            mStrobeModeSpinner.setSelection(mOffIndex);
        }
    }

    public void setSpinnerEnabled(boolean enabled) {
        mStrobeModeSpinner.setEnabled(enabled);
    }
}
