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
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.os.SystemProperties;
import android.telecom.TelecomManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.Toast;

import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;

import com.android.settings.R;

import com.mediatek.settings.FeatureOption;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.ISimManagementExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * A preference for SIM On/Off function.
 */
public class SimPowerPreference extends Preference {

    private static final String TAG = "SimPowerPreference";
    private static final boolean ENG_LOAD = Log.isLoggable(TAG, Log.DEBUG)
            || SystemProperties.get("ro.build.type").equals("eng");
    
    private Context mContext;
    private SubscriptionInfo mSubInfo;
    private int mSlotId;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    private boolean mState = false;
    private boolean mEnabled = false;
    private Switch mSwith = null;

    private ISimManagementExt mExt;

    /**
     * Construct of SimOnOffPreference.
     * @param context Context.
     */
    public SimPowerPreference(Context context, SubscriptionInfo subInfo,
            int slotId, boolean isAirplaneMode) {
        super(context);
        mContext = context;
        mSubInfo = subInfo;
        mSlotId = slotId;
        if (subInfo != null) {
            mSubId = subInfo.getSubscriptionId();
        }
        setKey("sim" + mSlotId);
        setWidgetLayoutResource(R.layout.radio_power_switch);
        mExt = UtilsExt.getSimManagementExt(context);
        update(isAirplaneMode);
    }

    /**
     * Set the SIM on/off switch state.
     * @param state the SIM on/off state.
     */
    public void setSimState(boolean state) {
        logInEng("setSimState, state=" + state + ", slotId=" + mSlotId);
        mState = state;
        if (mSwith != null) {
            mSwith.setChecked(state);
        }
    }

    /**
     * Set the SIM on/off switch enable state.
     * @param enable the enable state.
     */
    public void setSimEnabled(boolean enable) {
        logInEng("setSimEnabled, enable=" + enable + ", slotId=" + mSlotId);
        mEnabled = enable;
        if (mSwith != null) {
            mSwith.setEnabled(enable);
        }
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder view) {
        super.onBindViewHolder(view);
        mSwith = (Switch) view.findViewById(R.id.radio_state);
        if (mSwith != null) {
            mSwith.setEnabled(mEnabled);
            mSwith.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    logInEng("onCheckedChanged, state=" + mState
                            + ", isChecked=" + isChecked + ", slotId=" + mSlotId);
                    if (mState != isChecked) {
                        // Don't allow to turn off SIM during ECC
                        boolean isInEcc = TelecomManager.from(mContext).isInEmergencyCall();

                        if (!isChecked && isInEcc) {
                            Log.d(TAG, "Don't allow to turn off SIM during emergency call.");
                            Toast.makeText(mContext.getApplicationContext(),
                                    R.string.radio_off_during_emergency_call,
                                    Toast.LENGTH_LONG).show();
                            setSimState(!isChecked);
                            return;
                        }

                        int simState = (isChecked ? TelephonyUtils.SIM_ONOFF_STATE_ON
                                : TelephonyUtils.SIM_ONOFF_STATE_OFF);
                        int result = TelephonyUtils.setSimOnOffState(
                                mSlotId, simState);
                        if (handleResultAsSuccess(mSlotId, simState, result)) {
                            // Disable SIM on/off switch to prevent continuous click
                            logInEng("onCheckedChanged, state=" + isChecked);
                            mState = isChecked;
                            if (result != MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_OFF
                                    && result != MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_ON) {
                                setSimEnabled(false);
                            }
                            // Add for SIM Settings plugin.
                            mExt.customizeMainCapabily(mState, mSubId);
                            /// @}
                        } else {
                            // When setting SIM state fail, revert button status.
                            logInEng("onCheckedChanged, set SIM state FAIL.");
                            setSimState(!isChecked);
                        }
                    }
                }
            });
            // Ensure setOnCheckedChangeListener before setChecked state,
            // otherwise the expired OnCheckedChangeListener will be called.
            Log.d(TAG, "onBindViewHolder, state=" + mState + ", slotId=" + mSlotId);
            mSwith.setChecked(mState);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        mEnabled = enabled;
        super.setEnabled(enabled);
    }

    /**
     * Bind the preference with corresponding property.
     * @param subId sub id for this preference.
     * @param normal radio is not switching and not in airplane mode.
     * @param radio the radio state.
     * @param isAirplaneMode the airplane mode.
     */
    public void bindSimOnOffState(boolean isAirplaneMode) {
        int state = -1;
        int switchingState = -1;
        boolean isSwitching = false;
        if (mSubInfo != null) {
            switchingState = TelephonyUtils.getSimOnOffSwitchingState(mSlotId);
            switch (switchingState) {
                case TelephonyUtils.SIM_ONOFF_SWITCHING_ON:
                    isSwitching = true;
                    state = TelephonyUtils.SIM_ONOFF_STATE_ON;
                    break;

                case TelephonyUtils.SIM_ONOFF_SWITCHING_OFF:
                    isSwitching = true;
                    state = TelephonyUtils.SIM_ONOFF_STATE_OFF;
                    break;

                default:
                    state = TelephonyUtils.getSimOnOffState(mSlotId);
                    break;
            }

            setSimState(!isAirplaneMode
                    && (state == TelephonyUtils.SIM_ONOFF_STATE_ON));
            setSimEnabled(!isAirplaneMode && !isSwitching
                    && mExt.customizeCallStateNotInCall());
        } else {
            setSimState(false);
            setSimEnabled(false);
        }
        logInEng("bindSimOnOffState, slotId=" + mSlotId + ", state=" + state
                + ", switchingState=" + switchingState
                + ", isAirplaneMode=" + isAirplaneMode);
    }

    public void update(boolean isAirplaneMode) {
        final Resources res = mContext.getResources();

        setTitle(String.format(
                res.getString(R.string.sim_editor_title), mSlotId + 1));

        if (mSubInfo != null) {
            CharSequence displayName = mSubInfo.getDisplayName();
            String phoneNum = getPhoneNumber(mSubInfo);
            int state = -1;
            int switchingState = -1;
            boolean isSwitching = false;

            if (TextUtils.isEmpty(phoneNum)) {
                setSummary(displayName);
            } else {
                setSummary(displayName + " - " +
                        PhoneNumberUtils.createTtsSpannable(phoneNum));
            }
            setIcon(new BitmapDrawable(res, mSubInfo.createIconBitmap(mContext)));
            setEnabled(true);

            switchingState = TelephonyUtils.getSimOnOffSwitchingState(mSlotId);
            switch (switchingState) {
                case TelephonyUtils.SIM_ONOFF_SWITCHING_ON:
                    isSwitching = true;
                    state = TelephonyUtils.SIM_ONOFF_STATE_ON;
                    break;

                case TelephonyUtils.SIM_ONOFF_SWITCHING_OFF:
                    isSwitching = true;
                    state = TelephonyUtils.SIM_ONOFF_STATE_OFF;
                    break;

                default:
                    state = TelephonyUtils.getSimOnOffState(mSlotId);
                    break;
            }

            setSimState(!isAirplaneMode
                    && (state == TelephonyUtils.SIM_ONOFF_STATE_ON));
            setSimEnabled(!isAirplaneMode && !isSwitching
                    && mExt.customizeCallStateNotInCall());

            logInEng("update, slot=" + mSlotId + ", displayName="
                    + (displayName == null ? "null" : displayName)
                    + ", state=" + state + ", switchingState=" + switchingState);
        } else {
            setSummary(R.string.sim_slot_empty);
            setFragment(null);
            setEnabled(false);
        }
    }

    public int getSlotId() {
        return mSlotId;
    }

    private String getPhoneNumber(SubscriptionInfo info) {
        final TelephonyManager tm = (TelephonyManager)
                mContext.getSystemService(Context.TELEPHONY_SERVICE);
        return tm.getLine1Number(info.getSubscriptionId());
    }

    public boolean handleResultAsSuccess(int slotId, int state, int errorCode) {
        boolean success = false;
        switch (errorCode) {
            case MtkTelephonyManagerEx.SET_SIM_POWER_SUCCESS:
                success = true;
                break;

            case MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_EXECUTING_SIM_OFF:
            case MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_OFF:
                if (state == TelephonyUtils.SIM_ONOFF_STATE_OFF) {
                    success = true;
                }
                break;

            case MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_EXECUTING_SIM_ON:
            case MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_ON:
                if (state == TelephonyUtils.SIM_ONOFF_STATE_ON) {
                    success = true;
                }
                break;

            default:
                break;
        }
        logInEng("handleResultAsSuccess, slotId=" + slotId + ", state=" + state
                + ", success=" + success + ", errorCode=" + errorCode);
        return success;
    }

    private void logInEng(String s) {
        if (ENG_LOAD) {
            Log.d(TAG, s);
        }
    }
}
