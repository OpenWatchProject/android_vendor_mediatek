/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.settings.sim;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;

import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;

import com.android.settings.R;

import com.mediatek.settings.FeatureOption;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.ISimManagementExt;

/**
 * A preference for radio switch function.
 */
public class RadioPowerPreference extends Preference {

    private static final String TAG = "RadioPowerPreference";
    private boolean mPowerState;
    private boolean mPowerEnabled = true;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    private Switch mRadioSwith = null;
    private RadioPowerController mController;
    private static final boolean ENG_LOAD = SystemProperties.get("ro.build.type").equals("eng")
            ? true : false || Log.isLoggable(TAG, Log.DEBUG);
    private ISimManagementExt mExt;

    /**
     * Construct of RadioPowerPreference.
     * @param context Context.
     */
    public RadioPowerPreference(Context context) {
        super(context);
        mExt = UtilsExt.getSimManagementExt(context);
        mController = RadioPowerController.getInstance(context);
        setWidgetLayoutResource(R.layout.radio_power_switch);
    }

    /**
     * Set the radio switch state.
     * @param state the radio state.
     */
    public void setRadioOn(boolean state) {
        logInEng("setRadioOn, state=" + state + ", subId=" + mSubId);
        mPowerState = state;
        if (mRadioSwith != null) {
            mRadioSwith.setChecked(state);
        }
    }

    /**
     * Set the radio switch enable state.
     * @param enable the enable state.
     */
    public void setRadioEnabled(boolean enable) {
        logInEng("setRadioEnabled, enable=" + enable);
        mPowerEnabled = enable;
        if (mRadioSwith != null) {
            mRadioSwith.setEnabled(enable);
        }
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder view) {
        super.onBindViewHolder(view);
        mRadioSwith = (Switch) view.findViewById(R.id.radio_state);
        if (mRadioSwith != null) {
            mRadioSwith.setEnabled(mPowerEnabled);
            mRadioSwith.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    logInEng("onCheckedChanged, mPowerState=" + mPowerState
                            + ", isChecked=" + isChecked + ", subId=" + mSubId);
                    if (mPowerState != isChecked) {
                        if (mController.setRadionOn(mSubId, isChecked)) {
                            // Disable radio switch to prevent continuous click
                            logInEng("onCheckedChanged, mPowerState=" + isChecked);
                            mPowerState = isChecked;
                            setRadioEnabled(false);
                            // Add for SIM Settings plugin.
                            mExt.customizeMainCapabily(mPowerState, mSubId);
                            /// @}
                        } else {
                            // When setting radio fail, revert button status.
                            logInEng("onCheckedChanged, set radio power FAIL.");
                            setRadioOn(!isChecked);
                        }
                    }
                }
            });
            // Ensure setOnCheckedChangeListener before setChecked state,
            // otherwise the expired OnCheckedChangeListener will be called.
            Log.d(TAG, "onBindViewHolder, mPowerState=" + mPowerState + ", subid=" + mSubId);
            mRadioSwith.setChecked(mPowerState);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        mPowerEnabled = enabled;
        super.setEnabled(enabled);
    }

    /**
     * Bind the preference with corresponding property.
     * @param subId sub id for this preference.
     * @param normal radio is not switching and not in airplane mode.
     * @param radio the radio state.
     * @param isAirplaneMode the airplane mode.
     */
    public void bindRadioPowerState(final int subId, boolean normal, boolean radioOn,
            boolean isAirplaneMode) {
        mSubId = subId;
        if (normal) {
            setRadioOn(radioOn);
            boolean isValidSub = SubscriptionManager.isValidSubscriptionId(subId);
            logInEng("bindRadioPowerState, isValidSub=" + isValidSub);
            setRadioEnabled(isValidSub && mExt.customizeCallStateNotInCall());
        } else {
            logInEng("bindRadioPowerState, normal=false");
            setRadioEnabled(false);
            setRadioOn(!isAirplaneMode && mController.isExpectedRadioStateOn(
                    SubscriptionManager.getSlotIndex(subId)));
        }
    }

    private void logInEng(String s) {
        if (ENG_LOAD) {
            Log.d(TAG, s);
        }
    }
}
