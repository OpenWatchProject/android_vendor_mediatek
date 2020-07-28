package com.mediatek.emcamera.config;

import java.util.List;

import android.app.Activity;
import android.content.SharedPreferences.Editor;
import android.hardware.Camera.Parameters;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.mediatek.emcamera.CameraJni;
import com.mediatek.emcamera.Elog;
import com.mediatek.emcamera.R;

public class FpsConfig extends ConfigBase {
    private static final String TAG = FpsConfig.class.getSimpleName();
    private Spinner mSpinner;
    private List<int[]> mRanges;
    private int mDefaultPosition;

    public FpsConfig(Activity activity) {
        super(activity);
    }

    public void initComponents() {
        mSpinner = (Spinner) mActivity.findViewById(R.id.auto_clibr_supported_fps_range_spnr);
    }

    @Override
    public void initSupportedUi(Parameters parameters) {
        if (CameraJni.getCamHalVersion() != 1) {
            (mActivity.findViewById(R.id.fps_range_ll)).setVisibility(View.GONE);
            return;
        }
        mRanges = parameters.getSupportedPreviewFpsRange();
        if (mRanges == null || mRanges.size() == 0) {
            (mActivity.findViewById(R.id.fps_range_ll)).setVisibility(View.GONE);
        } else {
            String[] rangeStringArray = buildEntries(mRanges);
            ArrayAdapter<String> adp = new ArrayAdapter<String>(mActivity,
                    android.R.layout.simple_list_item_1, rangeStringArray);
            adp.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mSpinner.setAdapter(adp);

            int[] range = new int[2];
            parameters.getPreviewFpsRange(range);
            mDefaultPosition = getDefaultPosition(mRanges, range);
        }
    }

    private String[] buildEntries(List<int[]> ranges) {
        int size = ranges.size();
        String[] rangeStringArray = new String[size];
        for (int i = 0; i < size; ++i) {
            int[] range = ranges.get(i);
            rangeStringArray[i] = new StringBuilder("")
                    .append(range[Parameters.PREVIEW_FPS_MIN_INDEX]).append(",")
                    .append(range[Parameters.PREVIEW_FPS_MAX_INDEX]).toString();
        }
        return rangeStringArray;
    }

    private int getDefaultPosition(List<int[]> ranges, int[] dRange) {
        int size = ranges.size();
        for (int i = 0; i < size; ++i) {
            int[] range = ranges.get(i);
            if (range[Parameters.PREVIEW_FPS_MIN_INDEX] == dRange[Parameters.PREVIEW_FPS_MIN_INDEX]
                    && range[Parameters.PREVIEW_FPS_MAX_INDEX]
                            == dRange[Parameters.PREVIEW_FPS_MAX_INDEX]) {
                return i;
            }
        }
        return 0;
    }

    @Override
    public boolean saveValues(Editor editor) {
        if(mRanges != null) {
            int[] range = mRanges.get(mSpinner.getSelectedItemPosition());
            editor.putInt(mActivity.getString(R.string.auto_clibr_min_fps),
                    range[Parameters.PREVIEW_FPS_MIN_INDEX]);
            editor.putInt(mActivity.getString(R.string.auto_clibr_max_fps),
                    range[Parameters.PREVIEW_FPS_MAX_INDEX]);
        }
        return true;
    }

    @Override
    public void setStatusToDefault() {
        if(mRanges != null && mRanges.size() > 0) {
            mSpinner.setSelection(mDefaultPosition);
        }
    }

}
