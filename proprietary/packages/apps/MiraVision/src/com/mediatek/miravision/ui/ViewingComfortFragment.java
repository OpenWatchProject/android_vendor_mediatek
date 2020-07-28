package com.mediatek.miravision.ui;

import android.app.ActionBar;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.setting.MiraVisionJni.Range;
import com.mediatek.miravision.utils.Utils;
import com.mediatek.pq.PictureQuality;

public class ViewingComfortFragment extends BaseTuningFragment implements
        CompoundButton.OnCheckedChangeListener {

    private static final String TAG = "Miravision/BlueLightFragment";
    private Switch mActionBarSwitch;
    private Range mBlueLightRange;
    private PictureQuality.Range mChameleonRange;
    private final int mViewingComfortItems[] = { R.string.blue_light_item,
            R.string.chameleon_display_item };
    private final int mViewingComfortDrables[] = { R.drawable.blulight_defender,
            R.drawable.chameleon_display };

    public ViewingComfortFragment() {
    }

    public ViewingComfortFragment(int title) {
        mTuningTitle = title;
        for (int i = 0; i < mViewingComfortItems.length; i++) {
            if (mTuningTitle == mViewingComfortItems[i]) {
                index = i;
                break;
            }
        }
        Log.d(TAG, "mViewingComfortTitle = " + mTuningTitle + ", index = " + index);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Activity activity = getActivity();
        final ActionBar actionBar = activity.getActionBar();
        mActionBarSwitch = new Switch(actionBar.getThemedContext());
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        mActionBarSwitch.setPaddingRelative(0, 0, padding, 0);
        final ActionBar.LayoutParams lp = new ActionBar.LayoutParams(
                ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT,
                Gravity.CENTER_VERTICAL | Gravity.END);
        actionBar.setCustomView(mActionBarSwitch, lp);
        actionBar.setDisplayShowCustomEnabled(true);
        mActionBarSwitch.requestFocus();
        mActionBarSwitch.setOnCheckedChangeListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateUi();
    }

    @Override
    public void onClick(View v) {
        v.setEnabled(false);
        super.onClick(v);
        Log.d(TAG, "onClick index = " + index);
        updateUi();
        v.setEnabled(true);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            onSeekBarChange(seekBar.getProgress());
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    private void onSeekBarChange(int value) {
        Log.d(TAG, "onChange value = " + value);
        switch (mTuningTitle) {
        case R.string.blue_light_item:
            MiraVisionJni.setBlueLightIndex(value - mBlueLightRange.min);
            break;
        case R.string.chameleon_display_item:
            PictureQuality.setChameleonStrength(value - mChameleonRange.min);
            break;
        }
    }

    private void updateUi() {
        Log.d(TAG, "updateUi index = " + index);
        mImageBasicAfter.setVisibility(View.GONE);
        mLineView.setVisibility(View.GONE);
        mAfterText.setVisibility(View.GONE);
        mOrignalText.setVisibility(View.GONE);
        mImageAdvAfter.setVisibility(View.GONE);

        mTuningTitle = mViewingComfortItems[index];
        mSeekBarText.setText(mTuningTitle);
        mImageOrignal.setImageResource(mViewingComfortDrables[index]);

        switch (mTuningTitle) {
        case R.string.blue_light_item:
            mLastButton.setVisibility(View.INVISIBLE);
            String chameleon = Utils.getSysProperty("ro.vendor.mtk_chameleon_support", null);
            if (chameleon != null && chameleon.equals("1")) {
                mNextButton.setVisibility(View.VISIBLE);
            } else {
                mNextButton.setVisibility(View.INVISIBLE);
            }
            boolean blueLightEnabled = MiraVisionJni.isBlueLightEnabled();
            mActionBarSwitch.setChecked(blueLightEnabled);
            mSeekBar.setEnabled(blueLightEnabled);
            mBlueLightRange = MiraVisionJni.getBlueLightIndexRange();
            mSeekBar.setMax(mBlueLightRange.max - mBlueLightRange.min);
            mSeekBar.setProgress(MiraVisionJni.getBlueLightIndex() - mBlueLightRange.min);
            break;
        case R.string.chameleon_display_item:
            String blueLight = Utils.getSysProperty("ro.vendor.mtk_blulight_def_support", null);
            if (blueLight != null && blueLight.equals("1")) {
                mLastButton.setVisibility(View.VISIBLE);
            } else {
                mLastButton.setVisibility(View.INVISIBLE);
            }
            mNextButton.setVisibility(View.INVISIBLE);
            boolean chameleonEnabled = PictureQuality.isChameleonEnabled();
            mActionBarSwitch.setChecked(chameleonEnabled);
            mSeekBar.setEnabled(chameleonEnabled);
            mChameleonRange = PictureQuality.getChameleonStrengthRange();
            mSeekBar.setMax(mChameleonRange.max - mChameleonRange.min);
            mSeekBar.setProgress(PictureQuality.getChameleonStrength() - mChameleonRange.min);
            break;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.d(TAG, "onCheckedChanged " + isChecked);
        switch (mTuningTitle) {
        case R.string.blue_light_item:
            MiraVisionJni.enableBlueLight(isChecked);
            break;
        case R.string.chameleon_display_item:
            PictureQuality.enableChameleon(isChecked);
            break;
        }
        mSeekBar.setEnabled(isChecked);
    }
}
