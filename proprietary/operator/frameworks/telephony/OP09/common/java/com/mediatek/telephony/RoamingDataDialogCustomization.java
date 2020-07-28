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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.telephony;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.ContentObserver;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.view.WindowManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.R;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import mediatek.telephony.MtkServiceState;

/**
 * RoamingDataDialogCustomization is requested by China Telecom spec.
 * Popup remind dialog when user uses roaming data for first time.
 */
public class RoamingDataDialogCustomization extends Handler {
    private static final String TAG = "RoamingDataDialogCustomization";

    private static final int EVENT_DATA_SETTING_CHANGED = 1;
    private static final int EVENT_ROAMING_SETTING_CHANGED = 2;
    private static final int EVENT_SERVICE_STATE_CHANGED = 3;
    private static final int EVENT_DEFAULT_DATA_SUB_CHANGED = 4;

    private static final int MCC_LENGTH = 3;

    private static final String ROAMING_CUSTOM_PREFERENCE = "roaming_customization";
    private static final String FIRST_ROAMING_KEY = "first_roaming";
    private static final String LAST_REG_STATE_KEY = "last_reg_state";
    private static final String LAST_OPERATOR_NUMERIC_KEY = "last_operator_numeric";

    private Context mContext;
    private ContentResolver mResolver;
    private Phone mPhone;

    private String mDataUri = Settings.Global.MOBILE_DATA;
    private String mRoamingUri = Settings.Global.DATA_ROAMING;

    private String mFirstRoamingKey = FIRST_ROAMING_KEY;
    private String mLastRegStateKey = LAST_REG_STATE_KEY;
    private String mLastOperatorNumericKey = LAST_OPERATOR_NUMERIC_KEY;

    private int mLastRilRegState =
            NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    private String mLastOpNumeric = "00000";

    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    private ContentObserver mDataSettingObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            logd("mDataSettingObserver: change");
            removeMessages(EVENT_DATA_SETTING_CHANGED);
            sendEmptyMessage(EVENT_DATA_SETTING_CHANGED);
        }
    };

    private ContentObserver mRoamingSettingObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            logd("mRoamingSettingObserver: change");
            removeMessages(EVENT_ROAMING_SETTING_CHANGED);
            sendEmptyMessage(EVENT_ROAMING_SETTING_CHANGED);
        }
    };

    public RoamingDataDialogCustomization(Phone phone) {
        super();
        mContext = phone.getContext();
        mPhone = phone;
        mResolver = mContext.getContentResolver();

        mPhone.registerForServiceStateChanged(
                this, EVENT_SERVICE_STATE_CHANGED, null);

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        filter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        mContext.registerReceiver(mIntentReceiver, filter);
    }

    @Override
    public void handleMessage(Message msg) {
        int msgId = msg.what;
        logd("handleMessage: " + msgIdToString(msgId) + "(id=" + msgId + ")");
        switch (msgId) {
        case EVENT_DATA_SETTING_CHANGED:
        case EVENT_ROAMING_SETTING_CHANGED:
        case EVENT_DEFAULT_DATA_SUB_CHANGED:
            popupRoamingDataDialogIfNeeded();
            break;
        case EVENT_SERVICE_STATE_CHANGED:
            AsyncResult ar = (AsyncResult) msg.obj;
            MtkServiceState serviceState = (MtkServiceState) ar.result;
            logd("handleMessage: serviceState = " + serviceState.toString());
            final int dataRegState = serviceState.getDataRegState();

            if (dataRegState == MtkServiceState.STATE_IN_SERVICE) {
                final int rilDataRegState = serviceState.getRilDataRegState();
                final String operatorNumeric = serviceState.getOperatorNumeric();
                logd("handleMessage: rilDataRegState = " + rilDataRegState
                        + " operatorNumeric = " + operatorNumeric
                        + " mLastRilRegState = " + mLastRilRegState
                        + " mLastOpNumeric = " + mLastOpNumeric);
                if (isMccInvalid(operatorNumeric)) {
                    return;
                }
                if (rilDataRegState != mLastRilRegState ||
                        (mLastOpNumeric != null && operatorNumeric != null &&
                        !mLastOpNumeric.equals(operatorNumeric))) {
                    saveLastRegInfo(rilDataRegState, operatorNumeric);
                    if (rilDataRegState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING) {
                        popupRoamingDataDialogIfNeeded();
                    } else if (rilDataRegState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME) {
                        setFirstRoamingFlag(true);
                    }
                }
            } else {
                // reset mLastRilRegState to ensure EVENT_SERVICE_STATE_CHANGED triggered.
                saveLastRegInfo(NetworkRegistrationInfo.REGISTRATION_STATE_UNKNOWN, mLastOpNumeric);
            }
            break;
        default:
            break;
        }
    }

    private boolean isMccInvalid(String opNumeric) {
        if (TextUtils.isEmpty(opNumeric)) {
            logd("isMccInvalid, opNumeric is empty");
            return false;
        }

        String mcc = opNumeric.substring(0, MCC_LENGTH);
        return TextUtils.isEmpty(mcc) || mcc.equals("000") || mcc.equals("N/A");
    }

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                onSubInfoReady();
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                removeMessages(EVENT_DEFAULT_DATA_SUB_CHANGED);
                sendEmptyMessage(EVENT_DEFAULT_DATA_SUB_CHANGED);
            } else {
                logd("should not be here, action = " + action);
            }
        }
    };

    protected void onSubInfoReady() {
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhone.getPhoneId());

        if (SubscriptionManager.isValidSubscriptionId(subId) && mSubId != subId) {
            logd("onSubInfoReady: subId = " + subId + " mSubId = " + mSubId);
            mSubId = subId;
            initializeRoamingStatus(mSubId);
            registerForSettingEvent(mSubId);
            // Trigger self change to check whether need to popup prompt dialog,
            // in case the sub info ready is later than network registered.
            removeMessages(EVENT_DATA_SETTING_CHANGED);
            sendEmptyMessage(EVENT_DATA_SETTING_CHANGED);
        }
    }

    protected void registerForSettingEvent(int subId) {
        // For Dual SIM phones, need to change URI.
        if (TelephonyManager.getDefault().getSimCount() > 1) {
            mDataUri = Settings.Global.MOBILE_DATA + subId;
            mRoamingUri = Settings.Global.DATA_ROAMING + subId;
        }

        logd("registerForSettingEvent: mDataUri=" + mDataUri);

        // Register for mobile data enabled.
        if (mDataUri != null) {
            mResolver.unregisterContentObserver(mDataSettingObserver);
        }

        if (mRoamingUri != null) {
            mResolver.unregisterContentObserver(mRoamingSettingObserver);
        }

        mResolver.registerContentObserver(Settings.Global.getUriFor(mDataUri),
                false, mDataSettingObserver);
        mResolver.registerContentObserver(Settings.Global.getUriFor(mRoamingUri),
                false, mRoamingSettingObserver);
    }

    private void initializeRoamingStatus(int subId) {
        // Get roaming info from preference
        mFirstRoamingKey = FIRST_ROAMING_KEY + subId;
        mLastRegStateKey = LAST_REG_STATE_KEY + subId;
        mLastOperatorNumericKey = LAST_OPERATOR_NUMERIC_KEY + subId;
        SharedPreferences roamingPreferences = mContext.getSharedPreferences(
                ROAMING_CUSTOM_PREFERENCE, 0);
        mLastRilRegState = roamingPreferences.getInt(mLastRegStateKey,
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING);
        mLastOpNumeric = roamingPreferences.getString(mLastOperatorNumericKey, "00000");
    }

    protected void setFirstRoamingFlag(boolean first) {
        logd("setFirstRoamingFlag: first = " + first);
        SharedPreferences roamingPreferences =
                mContext.getSharedPreferences(ROAMING_CUSTOM_PREFERENCE, 0);
        Editor roamingEditor = roamingPreferences.edit();
        roamingEditor.putBoolean(mFirstRoamingKey, first);
        roamingEditor.apply();
    }

    private void saveLastRegInfo(int regState, String operatorNumeric) {
        logd("saveLastRegInfo: regState = " + regState + " operatorNumeric = " + operatorNumeric);
        mLastRilRegState = regState;
        mLastOpNumeric = operatorNumeric;
        SharedPreferences roamingPreferences = mContext.getSharedPreferences(ROAMING_CUSTOM_PREFERENCE, 0);
        Editor roamingEditor = roamingPreferences.edit();
        roamingEditor.putInt(mLastRegStateKey, regState);
        roamingEditor.putString(mLastOperatorNumericKey, operatorNumeric);
        roamingEditor.apply();
    }

    public void dispose() {
        mResolver.unregisterContentObserver(mDataSettingObserver);
        mResolver.unregisterContentObserver(mRoamingSettingObserver);
        mPhone.unregisterForServiceStateChanged(this);
        mContext.unregisterReceiver(mIntentReceiver);
    }

    protected void popupDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        builder.setMessage(R.string.roaming_message);
        builder.setPositiveButton(R.string.known, null);
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    }

    private boolean isCdmaCard(int phoneId) {
        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);
        return (cardType == MtkIccCardConstants.CardType.UIM_CARD
                || cardType == MtkIccCardConstants.CardType.UIM_SIM_CARD
                || cardType == MtkIccCardConstants.CardType.CT_3G_UIM_CARD
                || cardType == MtkIccCardConstants.CardType.CT_UIM_SIM_CARD
                || cardType == MtkIccCardConstants.CardType.CT_4G_UICC_CARD
                || cardType == MtkIccCardConstants.CardType.NOT_CT_UICC_CARD);
    }

    private boolean isDefaultDataSim() {
        return (SubscriptionManager.isValidSubscriptionId(mSubId)
                && SubscriptionManager.getDefaultDataSubscriptionId() == mSubId);
    }

    private boolean isInRoamingFirstTime() {
        boolean isRoaming = (mLastRilRegState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING);
        SharedPreferences roamingPreferences =
                mContext.getSharedPreferences(ROAMING_CUSTOM_PREFERENCE, 0);
        boolean firstRoaming = roamingPreferences.getBoolean(mFirstRoamingKey, true);

        logd("isInRoamingFirstTime: isRoaming = " + isRoaming + " firstRoaming = " + firstRoaming);

        return (isRoaming && firstRoaming);
    }

    private boolean isDataSwitchEnabled() {
        return mPhone.getDataEnabledSettings().isUserDataEnabled();
    }

    protected boolean isRoamingDataSwitchEnabled() {
        return mPhone.getDataRoamingEnabled();
    }

    private boolean isUseRoamingDataFirstTime() {
        if (isInRoamingFirstTime() && isDataSwitchEnabled() && isRoamingDataSwitchEnabled()) {
            setFirstRoamingFlag(false);
            return true;
        }

        return false;
    }

    private boolean isDataInService() {
       MtkServiceState ss = (MtkServiceState)(mPhone.getServiceState());
       if (ss != null && ss.getDataRegState() == MtkServiceState.STATE_IN_SERVICE) {
           return true;
       }
       return false;
    }

    public void popupRoamingDataDialogIfNeeded() {
        if (!isCdmaCard(mPhone.getPhoneId())) {
            logd("popupRoamingDataDialogIfNeeded: not cdma card");
            return;
        }

        if(!isDefaultDataSim()) {
            logd("popupRoamingDataDialogIfNeeded: curren sim is not default sim");
            return;
        }

        if (!isDataInService()) {
            logd("popupRoamingDataDialogIfNeeded: data not in service");
            return;
        }

        if (isUseRoamingDataFirstTime()) {
            logd("popupRoamingDataDialogIfNeeded: show dialog for using roaming data first time");
            popupDialog();
        }
    }

    protected String msgIdToString(int id) {
        switch (id) {
        case EVENT_DATA_SETTING_CHANGED:
            return "EVENT_DATA_SETTING_CHANGED";
        case EVENT_ROAMING_SETTING_CHANGED:
            return "EVENT_ROAMING_SETTING_CHANGED";
        case EVENT_SERVICE_STATE_CHANGED:
            return "EVENT_SERVICE_STATE_CHANGED";
        case EVENT_DEFAULT_DATA_SUB_CHANGED:
            return "EVENT_DEFAULT_DATA_SUB_CHANGED";
        default:
            return "Unknown event";
        }
    }

    protected void logd(String s) {
        Rlog.d(TAG + "[" + mPhone.getPhoneId() + "]", s);
    }
}
