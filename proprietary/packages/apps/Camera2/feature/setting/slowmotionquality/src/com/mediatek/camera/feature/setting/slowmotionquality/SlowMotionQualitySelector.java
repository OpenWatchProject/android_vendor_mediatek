package com.mediatek.camera.feature.setting.slowmotionquality;

import android.app.Activity;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.ArrayList;
import java.util.List;

public class SlowMotionQualitySelector extends PreferenceFragment {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SlowMotionQualitySelector.class.getSimpleName());
    private Preference.OnPreferenceClickListener mOnPreferenceClickListener
            = new SlowMotionQualitySelector.MyOnPreferenceClickListener();

    private List<String> mEntryValues = new ArrayList<>();
    private List<String> mSummaryList = new ArrayList<>();
    private List<String> mTitleList = new ArrayList<>();

    private String mSelectedValue = null;
    private SlowMotionQualitySelector.OnItemClickListener mListener;
    private Activity mActivity;
    private int mCameraID;

    public interface OnItemClickListener {
        void onItemClick(String value);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        filterValuesOnShown();
        Toolbar toolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        toolbar.setTitle(getActivity().getResources().getString(R.string.slowmotion_quality_title));
        addPreferencesFromResource(R.xml.slowmotionquality_selector_preference);
        PreferenceScreen screen = getPreferenceScreen();
        for (int i = 0; i < mEntryValues.size(); i++) {
            RadioPreference preference = new RadioPreference(getActivity());
            if (mEntryValues.get(i).equals(mSelectedValue)) {
                preference.setChecked(true);
            }
            preference.setTitle(mTitleList.get(i));
            preference.setSummary(mSummaryList.get(i));
            preference.setOnPreferenceClickListener(mOnPreferenceClickListener);
            screen.addPreference(preference);
        }
    }

    public void setOnItemClickListener(SlowMotionQualitySelector.OnItemClickListener listener) {
        mListener = listener;
    }

    public void setValue(String value) {
        mSelectedValue = value;
    }

    public void setEntryValues(List<String> entryValues) {
        mEntryValues.clear();
        mEntryValues.addAll(entryValues);
    }

    public void setCurrentID(int currentID) {
        mCameraID = currentID;
    }

    public void setActivity(Activity activity) {
        mActivity = activity;
    }

    private void filterValuesOnShown() {
        List<String> tempValues = new ArrayList<>(mEntryValues);
        mEntryValues.clear();
        mTitleList.clear();
        mSummaryList.clear();
        for (int i = 0; i < tempValues.size(); i++) {
            String value = tempValues.get(i);
            String resolution = SlowMotionQualityHelper.getQualityResolution(
                    mCameraID, Integer.parseInt(value));
            String title = SlowMotionQualityHelper.getQualityTitle(
                    mActivity, mCameraID, Integer.parseInt(value));
            if (title != null) {
                mTitleList.add(title);
                mEntryValues.add(value);
                mSummaryList.add(resolution);
            }
        }
    }

    private class MyOnPreferenceClickListener implements Preference.OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            String title = (String) preference.getTitle();
            int index = mTitleList.indexOf(title);
            String value = mEntryValues.get(index);
            mListener.onItemClick(value);
            mSelectedValue = value;
            getActivity().getFragmentManager().popBackStack();
            return true;
        }
    }
}
