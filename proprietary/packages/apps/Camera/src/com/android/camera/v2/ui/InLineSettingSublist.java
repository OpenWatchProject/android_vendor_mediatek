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
import android.widget.ImageView;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

/* A switch setting control which turns on/off the setting. */
public class InLineSettingSublist extends InLineSettingItem implements
        SettingSublistLayout.Listener {
    private static final Tag TAG = new Tag(InLineSettingSublist.class.getSimpleName());
    private Context                           mContext;
    private TextView                          mEntry;
    private ImageView                         mImage;
    private SettingSublistLayout              mSettingLayout;
    private View                              mSettingContainer;
    protected boolean                         mShowingChildList;

    protected OnClickListener mOnClickListener = new OnClickListener() {

        @Override
        public void onClick(View view) {
            LogHelper.d(TAG, "onClick() mShowingChildList=" + mShowingChildList + ", mPreference="
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
        mContext = context;
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
                LogHelper.e(TAG, "Fail to find override value=" + override);
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
            LayoutInflater inflater = LayoutInflater.from(getContext());
            mSettingLayout = (SettingSublistLayout) inflater.inflate(
                    R.layout.setting_sublist_layout_v2, mRootView, false);
            mSettingContainer = mSettingLayout.findViewById(R.id.container);
            mSettingLayout.initialize(mPreference);
            if (mRootView != null) {
                mRootView.addView(mSettingLayout);
                int orientation = (Integer) mRootView.getTag();
                CameraUtil.setOrientation(mSettingLayout, orientation, false);
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
            // mSettingLayout = null; //comment this statement to avoid JE,
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

    protected void highlight() {
        if (mTitle != null) {
            mTitle.setTextColor(CameraUtil.getMainColor(getContext()));
        }
        if (mEntry != null) {
            mEntry.setTextColor(CameraUtil.getMainColor(getContext()));
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
        LogHelper.d(TAG, "onSettingChanged(" + changed + ") mListener=" + mListener);
        if (mListener != null && changed) {
            mListener.onSettingChanged(this, mPreference);
        }
        collapseChild();
    }

    protected void setOrientation(int orientation, boolean animation) {
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
