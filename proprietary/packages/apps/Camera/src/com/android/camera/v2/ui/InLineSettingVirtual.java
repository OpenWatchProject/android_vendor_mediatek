/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera.v2.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

/**
 * In Virtual item layout: ListPreference.getEntries() return child list
 * preference key ListPreference.getEntryValues() return child list default
 * value ListPreference.getDefaultValue() return default string displayed for
 * final user.
 */
public class InLineSettingVirtual extends InLineSettingItem implements
        SettingVirtualLayout.Listener {
    private static final Tag TAG = new Tag(InLineSettingVirtual.class.getSimpleName());
    private TextView mEntry;
    private SettingVirtualLayout mSettingLayout;
    private View mSettingContainer;
    private boolean mShowingChildList;
    private ListPreference[] mChildPrefs;

    private OnClickListener mOnClickListener = new OnClickListener() {

        @Override
        public void onClick(View view) {
            LogHelper.d(TAG, "onClick() mShowingChildList=" + mShowingChildList + ", mPreference="
                    + mPreference);
            if (!mShowingChildList && mChildPrefs != null && mPreference != null
                    && mPreference.isClickable()) {
                expendChild();
            } else {
                collapseChild();
            }
        }
    };

    public InLineSettingVirtual(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mEntry = (TextView) findViewById(R.id.current_setting);
        setOnClickListener(mOnClickListener);
    }

    @Override
    public boolean dispatchPopulateAccessibilityEvent(AccessibilityEvent event) {
        onPopulateAccessibilityEvent(event);
        return true;
    }

    @Override
    public void onPopulateAccessibilityEvent(AccessibilityEvent event) {
        super.onPopulateAccessibilityEvent(event);
        event.getText().add(mPreference.getTitle() + mPreference.getEntry());
    }

    @Override
    public void initialize(ListPreference preference) {
        LogHelper.i(TAG, "initialize(" + preference + ")");
        setTitle(preference);
        if (preference == null) {
            mChildPrefs = null;
            return;
        }
        mPreference = preference;
        reloadPreference();
    }

    // The value of the preference may have changed. Update the UI.
    @Override
    public void reloadPreference() {
        mChildPrefs = mPreference.getChildPreferences();
        if (mChildPrefs == null) {
            return;
        }
        for (ListPreference pref : mChildPrefs) {
            LogHelper.d(TAG, "reloadPreference() mChildPref=" + pref);
        }
        updateView();
    }

    @Override
    protected void updateView() {
        if (mPreference == null || mChildPrefs == null) {
            return;
        }
        setOnClickListener(null);
        int len = mChildPrefs.length;
        boolean allDefault = true;
        int enableCount = 0;
        for (int i = 0; i < len; i++) {
            ListPreference pref = mChildPrefs[i];
            if (pref == null) {
                continue;
            }
            String defaultValue = String.valueOf(mPreference.getEntryValues()[i]);
            String value = pref.getOverrideValue();
            if (value == null) {
                value = pref.getValue();
            }
            if (pref.isEnabled()) {
                enableCount++;
            }
            // we assume pref and default value is not null.
            if (allDefault && !defaultValue.equals(value)) {
                allDefault = false;
            }
        }
        if (allDefault) {
            mEntry.setText(mPreference.getDefaultValue());
        } else {
            mEntry.setText("");
        }
        boolean enabled = (enableCount == len);
        mPreference.setEnabled(enabled);
        setEnabled(mPreference.isEnabled());
        setOnClickListener(mOnClickListener);
        LogHelper.d(TAG, "updateView() enableCount=" + enableCount + ", len=" + len);
    }

    @Override
    public void onSettingChanged(ListPreference pref) {
        if (mListener != null) {
            mListener.onSettingChanged(this, pref);
        }

    }

    public boolean expendChild() {
        boolean expend = false;
        if (!mShowingChildList) {
            mShowingChildList = true;
            if (mListener != null) {
                mListener.onShow(this);
            }
            LayoutInflater inflater = LayoutInflater.from(getContext());
            mSettingLayout = (SettingVirtualLayout) inflater.inflate(
                    R.layout.setting_virtual_layout_v2, mRootView, false);
            mSettingContainer = mSettingLayout.findViewById(R.id.container);
            mSettingLayout.initialize(mChildPrefs);
            if (mRootView != null) {
                mRootView.addView(mSettingLayout);
            }
            mSettingLayout.setSettingChangedListener(this);
            fadeIn(mSettingLayout);
            highlight();
            expend = true;
        }
        LogHelper.d(TAG, "expendChild() return " + expend);
        return expend;
    }

    public boolean collapseChild() {
        boolean collapse = false;
        if (mShowingChildList) {
            if (mRootView != null) {
                mRootView.removeView(mSettingLayout);
            }
            fadeOut(mSettingLayout);
            normalText();
            // mSettingLayout = null; // comment this statement to avoid JE,
            // ALPS01287764
            mShowingChildList = false;
            if (mListener != null) {
                mListener.onDismiss(this);
            }
            collapse = true;
        }
        LogHelper.d(TAG, "collapseChild() return " + collapse);
        return collapse;
    }

    private void highlight() {
        if (mTitle != null) {
            mTitle.setTextColor(CameraUtil.getMainColor(getContext()));
        }
        if (mEntry != null) {
            mEntry.setTextColor(CameraUtil.getMainColor(getContext()));
        }
        setBackgroundDrawable(null);
    }

    private void normalText() {
        if (mTitle != null) {
            mTitle.setTextColor(getResources().getColor(R.color.setting_item_text_color_normal));
        }
        if (mEntry != null) {
            mEntry.setTextColor(getResources()
                    .getColor(R.color.setting_item_text_color_normal));
        }
        setBackgroundResource(R.drawable.setting_picker);
    }

    private void setOrientation(int orientation, boolean animation) {
        LogHelper.d(TAG, "setOrientation(" + orientation + ", " + animation + ")");
        if (mShowingChildList) {
            Util.setOrientation(mSettingLayout, orientation, animation);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (!enabled) {
            collapseChild();
        }
    }
}
