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
import android.view.View.OnClickListener;
import android.view.accessibility.AccessibilityEvent;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;

import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;


/* A switch setting control which turns on/off the setting. */
public class InLineSettingSwitch extends InLineSettingItem implements
        SettingSwitchSublistLayout.Listener {
    private static final Tag TAG = new Tag(InLineSettingSwitch.class.getSimpleName());
    private SettingSwitchSublistLayout mSettingLayout;
    private Switch mSwitch;
    private boolean mShowingChildList;

    OnCheckedChangeListener mCheckedChangeListener = new OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean desiredState) {
            changeIndex(desiredState ? 1 : 0);
        }
    };

    public InLineSettingSwitch(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mSwitch = (Switch) findViewById(R.id.setting_switch);
        mSwitch.setOnCheckedChangeListener(mCheckedChangeListener);
        setOnClickListener(mOnClickListener);
    }

    @Override
    public void initialize(ListPreference preference) {
        super.initialize(preference);
        // Add content descriptions for the increment and decrement buttons.
        mSwitch.setContentDescription(getContext().getResources().getString(
                R.string.accessibility_switch, mPreference.getTitle()));
    }

    @Override
    protected void updateView() {
        mSwitch.setOnCheckedChangeListener(null);
        mOverrideValue = mPreference.getOverrideValue();
        if (mOverrideValue == null) {
            mSwitch.setChecked(mIndex == 1);
        } else {
            int index = mPreference.findIndexOfValue(mOverrideValue);
            mSwitch.setChecked(index == 1);
        }
        setEnabled(mPreference.isEnabled());
        mSwitch.setOnCheckedChangeListener(mCheckedChangeListener);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (mSwitch != null) {
            mSwitch.setEnabled(enabled);
        }
    }

    @Override
    public boolean dispatchPopulateAccessibilityEvent(AccessibilityEvent event) {
        onPopulateAccessibilityEvent(event);
        return true;
    }

    @Override
    public void onPopulateAccessibilityEvent(AccessibilityEvent event) {
        super.onPopulateAccessibilityEvent(event);
        event.getText().add(mPreference.getTitle());
    }

    private OnClickListener mOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View view) {
            LogHelper.d(TAG, "onClick() mPreference=" + mPreference);
            if (mPreference != null && mPreference.isClickable() && mPreference.isEnabled()) {
                if (mListener != null) {
                    mListener.onShow(InLineSettingSwitch.this);
                }
                if (mPreference.getKey().equals(SettingKeys.KEY_VOICE)) {
                    if (!mShowingChildList) {
                        extendChild();
                    } else {
                        collapseChild();
                    }
                } else {
                    if (mSwitch != null) {
                        mSwitch.performClick();
                    }
                }
            }
        }
    };

    public boolean extendChild() {
        boolean extend = false;
        if (!mShowingChildList) {
            mShowingChildList = true;
            LayoutInflater inflater = LayoutInflater.from(getContext());
            mSettingLayout = (SettingSwitchSublistLayout) inflater.inflate(
                    R.layout.setting_switch_sublist_layout_v2, mRootView, false);
            mSettingLayout.initialize(mPreference);
            if (mRootView != null) {
                mRootView.addView(mSettingLayout);
            }
            mSettingLayout.setSettingChangedListener(this);
            fadeIn(mSettingLayout);
            mSwitch.setClickable(false);
            extend = true;
        }
        LogHelper.d(TAG, "extendChild() return " + extend);
        return extend;
    }

    public boolean collapseChild() {
        boolean collapse = false;
        if (mShowingChildList) {
            if (mRootView != null) {
                mRootView.removeView(mSettingLayout);
            }
            fadeOut(mSettingLayout);
            mSettingLayout = null;
            mShowingChildList = false;
            if (mListener != null) {
                mListener.onDismiss(this);
            }
            mSwitch.setClickable(true);
            collapse = true;
        }
        LogHelper.d(TAG, "collapseChild() return " + collapse);
        return collapse;
    }

    private void setOrientation(int orientation, boolean animation) {
        LogHelper.d(TAG, "setOrientation(" + orientation + "," + animation + ")");
        if (mShowingChildList) {
            Util.setOrientation(mSettingLayout, orientation, animation);
        }
    }

    @Override
    public void onVoiceCommandChanged(int index) {
        if (mListener != null) {
            //mListener.onVoiceCommandChanged(index);
        }
    }
}
