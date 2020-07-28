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

package com.android.camera.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.SettingUtils;
import com.android.camera.Util;
import com.android.camera.manager.ViewManager;

import com.mediatek.camera.setting.preference.ListPreference;

/* A switch setting control which turns on/off the setting. */
public class InLineSettingSublist extends InLineSettingItem implements
        SettingSublistLayout.Listener, CameraActivity.OnOrientationListener {
    private static final String TAG = "InLineSettingSublist";

    protected CameraActivity mContext;
    private TextView mEntry;
    private ImageView mImage;
    protected SettingSublistLayout mSettingLayout;
    protected View mSettingContainer;
    protected boolean mShowingChildList;

    protected OnClickListener mOnClickListener = new OnClickListener() {

        @Override
        public void onClick(View view) {
            Log.d(TAG, "onClick() mShowingChildList=" + mShowingChildList + ", mPreference="
                    + mPreference);
            if (!mShowingChildList && mPreference != null && mPreference.isClickable()) {
                expendChild();
            } else {
                collapseChild();
            }
        }
    };

    public InLineSettingSublist(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = (CameraActivity) context;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mEntry = (TextView) findViewById(R.id.current_setting);
        mImage = (ImageView) findViewById(R.id.image);
        setOnClickListener(mOnClickListener);
    }

    @Override
    protected void updateView() {
        if (mPreference == null) {
            return;
        }
        setOnClickListener(null);
        String override = mPreference.getOverrideValue();
        if (override == null) {
            setTextOrImage(mIndex, mPreference.getEntry());
        } else {
            int index = mPreference.findIndexOfValue(override);
            if (index != -1) {
                setTextOrImage(index, String.valueOf(mPreference.getEntries()[index]));
            } else {
                // Avoid the crash if camera driver has bugs.
                Log.e(TAG, "Fail to find override value=" + override);
                mPreference.print();
            }
        }
        setEnabled(mPreference.isEnabled());
        setOnClickListener(mOnClickListener);
    }

    protected void setTextOrImage(int index, String text) {
        int iconId = mPreference.getIconId(index);
        if (iconId != ListPreference.UNKNOWN) {
            mEntry.setVisibility(View.GONE);
            mImage.setVisibility(View.VISIBLE);
            mImage.setImageResource(iconId);
        } else {
            mEntry.setVisibility(View.VISIBLE);
            mEntry.setText(text);
            mImage.setVisibility(View.GONE);
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
        event.getText().add(mPreference.getTitle() + mPreference.getEntry());
    }

    public boolean expendChild() {
        boolean expend = false;
        if (!mShowingChildList) {
            mShowingChildList = true;
            if (mListener != null) {
                mListener.onShow(this);
            }
            mSettingLayout = (SettingSublistLayout) mContext.inflate(
                    R.layout.setting_sublist_layout, ViewManager.VIEW_LAYER_SETTING);
            mSettingContainer = mSettingLayout.findViewById(R.id.container);
            mSettingLayout.initialize(mPreference);
            mContext.addView(mSettingLayout, ViewManager.VIEW_LAYER_SETTING);
            mContext.addOnOrientationListener(this);
            mSettingLayout.setSettingChangedListener(this);
            setOrientation(mContext.getOrientationCompensation(), false);
            fadeIn(mSettingLayout);
            highlight();
            expend = true;
        }
        Log.d(TAG, "expendChild() return " + expend);
        return expend;
    }

    public boolean collapseChild() {
        boolean collapse = false;
        if (mShowingChildList) {
            mContext.removeOnOrientationListener(this);
            mContext.removeView(mSettingLayout, ViewManager.VIEW_LAYER_SETTING);
            fadeOut(mSettingLayout);
            normalText();
            // mSettingLayout = null; //comment this statement to avoid JE,
            // ALPS01287764
            mShowingChildList = false;
            if (mListener != null) {
                mListener.onDismiss(this);
            }
            collapse = true;
        }
        Log.d(TAG, "collapseChild() return " + collapse);
        return collapse;
    }

    protected void highlight() {
        if (mTitle != null) {
            mTitle.setTextColor(SettingUtils.getMainColor(getContext()));
        }
        if (mEntry != null) {
            mEntry.setTextColor(SettingUtils.getMainColor(getContext()));
        }
        setBackgroundDrawable(null);
    }

    protected void normalText() {
        if (mTitle != null) {
            mTitle.setTextColor(getResources().getColor(R.color.setting_item_text_color_normal));
        }
        if (mEntry != null) {
            mEntry.setTextColor(getResources()
                    .getColor(R.color.setting_item_text_color_normal));
        }
        setBackgroundResource(R.drawable.setting_picker);
    }

    @Override
    public void onSettingChanged(boolean changed) {
        Log.d(TAG, "onSettingChanged(" + changed + ") mListener=" + mListener);
        if (mListener != null && changed) {
            mListener.onSettingChanged(this, mPreference);
        }
        collapseChild();
    }

    @Override
    public void onOrientationChanged(int orientation) {
        setOrientation(orientation, true);
    }

    protected void setOrientation(int orientation, boolean animation) {
        Log.d(TAG, "setOrientation(" + orientation + ", " + animation + ")");
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
