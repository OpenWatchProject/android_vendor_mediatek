/*
* Copyright (C) 2016 The Android Open Source Project
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

package com.android.packageinstaller.permission.ui.handheld;

import android.content.Context;
import android.view.View;
import android.widget.Switch;

import androidx.preference.PreferenceViewHolder;
import androidx.preference.SwitchPreference;

/// M: CTA requirement - permission control @{
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
///@}

class MultiTargetSwitchPreference extends SwitchPreference {
    private View.OnClickListener mSwitchOnClickLister;
    /// M: CTA requirement - permission control @{
    private OnPreferenceChangeListener mSwitchChangeListener;
    CtaManager mCtaManager;
    ///@}

    public MultiTargetSwitchPreference(Context context) {
        super(context);
        mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
    }

    public void setCheckedOverride(boolean checked) {
        super.setChecked(checked);
    }

    /// M: CTA requirement - permission control @{
    @Override
    protected void onClick() {
        //TwoStatePreference default onClick(...) will toggle the checked state.
        //For the MultiTargetPreference, it will listen the preference
        //click event explicitly through Preference.setOnPreferenceClick(...) API.
        if (!mCtaManager.isCtaSupported() || mSwitchChangeListener == null) {
            super.onClick();
        }
    }

    @Override
    public boolean callChangeListener(Object newValue) {
        // This will be invoked in two cases:
        //    1. click on the preference, this will be filtered out in the above onClick()
        //    2. When checked state change of the Switch view.
        // Overriding this API, just for updating the permission state when dragging
        // the switch view to change its checked state.
        if (mCtaManager.isCtaSupported() && mSwitchChangeListener != null) {
            mSwitchChangeListener.onPreferenceChange(this, newValue);
        }
        return super.callChangeListener(newValue);
    }
    ///@}

    @Override
    public void setChecked(boolean checked) {
        // If double target behavior is enabled do nothing
        if (mSwitchOnClickLister == null) {
            super.setChecked(checked);
        }
    }

    public void setSwitchOnClickListener(View.OnClickListener listener) {
        mSwitchOnClickLister = listener;
    }

    /// M: CTA requirement - permission control @{
    public void setSwitchChangeListener(OnPreferenceChangeListener listener) {
        mSwitchChangeListener = listener;
    }
    ///@}

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);
        Switch switchView = holder.itemView.findViewById(android.R.id.switch_widget);
        if (switchView != null) {
            switchView.setOnClickListener(mSwitchOnClickLister);

            if (mSwitchOnClickLister != null) {
                final int padding = (int) ((holder.itemView.getMeasuredHeight()
                        - switchView.getMeasuredHeight()) / 2 + 0.5f);
                /// M: CTA requirement - permission control @{
                if (mCtaManager.isCtaSupported()) {
                    // do not set left padding to avoid title change when change the
                    // switch view's checked state.
                    switchView.setPaddingRelative(0, padding, 0, padding);
                    /// @}
                } else {
                    switchView.setPaddingRelative(padding, padding, 0, padding);
                }
            }
        }
    }
}
