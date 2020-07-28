package com.mediatek.camera.feature.setting.slowmotionquality;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.preference.PreferenceFragment;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.Preference;
import com.mediatek.camera.common.setting.ICameraSettingView;

import java.util.ArrayList;
import java.util.List;

public class SlowMotionQualitySettingView implements ICameraSettingView,
        SlowMotionQualitySelector.OnItemClickListener {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(SlowMotionQualitySettingView.class.getSimpleName());
    private List<String> mEntryValues = new ArrayList<>();

    private SlowMotionQualitySelector mQualitySelector;
    private SlowMotionQualitySettingView.OnValueChangeListener mListener;
    private SlowMotionQuality mSlowMotionQuality;
    private String mSelectedValue;
    private boolean mEnabled;
    private Activity mActivity;
    private Preference mPref;
    private String mSummary;
    private String mKey;

    public interface OnValueChangeListener {
        void onValueChanged(String value);
    }

    public SlowMotionQualitySettingView(String key, SlowMotionQuality SlowMotionQuality) {
        mKey = key;
        mSlowMotionQuality = SlowMotionQuality;
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        mActivity = fragment.getActivity();
        if (mQualitySelector == null) {
            mQualitySelector = new SlowMotionQualitySelector();
            mQualitySelector.setOnItemClickListener(this);

        }
        mQualitySelector.setActivity(mActivity);
        mQualitySelector.setCurrentID(Integer.parseInt(mSlowMotionQuality.getCameraId()));
        mQualitySelector.setValue(mSelectedValue);
        mQualitySelector.setEntryValues(mEntryValues);
        fragment.addPreferencesFromResource(R.xml.slowmotionquality_preference);
        mPref = (Preference) fragment.findPreference(mKey);
        mPref.setRootPreference(fragment.getPreferenceScreen());
        mPref.setId(R.id.slowmotion_quality_setting);
        mPref.setContentDescription(mActivity.getResources()
                .getString(R.string.slowmotion_quality_content_description));
        mPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {

            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mQualitySelector, "slowmotion_quality_selector").commit();
                return true;
            }
        });
        mPref.setEnabled(mEnabled);
        mSummary = SlowMotionQualityHelper.getQualityResolution(
                Integer.parseInt(mSlowMotionQuality.getCameraId()),
                Integer.parseInt(mSelectedValue));
    }

    @Override
    public void refreshView() {
        if (mPref != null) {
            mPref.setSummary(mSummary);
            mPref.setEnabled(mEnabled);
        }
    }

    @Override
    public void unloadView() {

    }

    @Override
    public void setEnabled(boolean enabled) {
        mEnabled = enabled;
    }

    @Override
    public boolean isEnabled() {
        return mEnabled;
    }

    public void setOnValueChangeListener(
            SlowMotionQualitySettingView.OnValueChangeListener listener) {
        mListener = listener;
    }

    public void setValue(String value) {
        mSelectedValue = value;
    }

    public void setEntryValues(List<String> entryValues) {
        mEntryValues = entryValues;
    }

    @Override
    public void onItemClick(String value) {
        LogHelper.i(TAG, "[onItemClick] value = " + value);
        mSelectedValue = value;
        mSummary = SlowMotionQualityHelper.getQualityResolution(
                Integer.parseInt(mSlowMotionQuality.getCameraId()),
                Integer.parseInt(mSelectedValue));
        mListener.onValueChanged(value);
    }
}
