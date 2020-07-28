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

package com.mediatek.settings.cdma;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;
import android.util.Log;
import android.view.WindowManager;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.phone.R;

import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.settings.TelephonyUtils;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

/**
 * Handler class for CT Volte service update.
 */
public class CtVolteServiceChecker extends Handler {

    private final static String TAG = "CtVolteServiceChecker";

    private final static int CHECK_DURATION = 120000;
    private final static int CHECK_TIME_OUT = 100;

    private static CtVolteServiceChecker sInstance;

    private Context mContext;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    private int mPhoneId;
    private Phone mPhone;
    private ImsManager mImsManager;

    private Dialog mDialog;
    private SubscriptionManager mSubscriptionManager;
    private TelephonyManager mTelephonyManager;

    private boolean mAirplaneMode;
    private boolean mRadioOn;
    private boolean mRoaming;
    private int mDataNetworkType;
    private int mVoiceNetworkType;
    private boolean mLteEnabled;
    private boolean mVolteEnabled;
    private boolean mImsAvailable;
    private boolean mSimOnOffEnabled;
    private int mSimOnOffState;

    private boolean mChecking = false;

    private MyPhoneStateListener mPhoneStateListener = null;
    private MtkImsConnectionStateListener mImsConnectionStateListener = null;
    private DataContentObserver mDataContentObserver = null;

    private final SubscriptionManager.OnSubscriptionsChangedListener mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            Log.d(TAG, "onSubscriptionsChanged");
            updateListenedSubInfo();
        }
    };

    private CtVolteServiceChecker(Context context) {
        super(context.getMainLooper());
        mContext = context;
    }

    public static CtVolteServiceChecker getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new CtVolteServiceChecker(context);
        }
        return sInstance;
    }

    public void init() {
        if (!RatConfiguration.isC2kSupported()) {
            return;
        }

        mSubscriptionManager = SubscriptionManager.from(mContext);
        mTelephonyManager = TelephonyManager.from(mContext);

        mAirplaneMode = isAirplaneModeOn(mContext);
        mSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();

        IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);

        updateListenedSubInfo();

        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
    }

    private void resetSubInfo() {
        mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        mPhone = null;
        mImsManager = null;

        mRadioOn = false;
        mRoaming = false;
        mDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        mLteEnabled = false;
        mVolteEnabled = false;
        mImsAvailable = false;
        mSimOnOffState = MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON;
    }

    private void updateListenedSubInfo() {
        int subId = getListenedSubId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (subId != mSubId || phoneId != mPhoneId) {
            Log.d(TAG, "updateListenedSubInfo, prevSubId=" + mSubId
                    + ", prevPhoneId=" + mPhoneId + ", newSubId=" + subId
                    + ", newPhoneId=" + phoneId);

            if (mDialog != null && mDialog.isShowing()) {
                mDialog.dismiss();
                mDialog = null;
            }
            stopTimeOutCheck();

            if (mPhoneStateListener != null) {
                mPhoneStateListener.unregister();
                mPhoneStateListener = null;
            }
            if (mDataContentObserver != null) {
                mDataContentObserver.unregister();
                mDataContentObserver = null;
            }
            if (mImsConnectionStateListener != null) {
                try {
                    ((MtkImsManager) mImsManager).removeImsConnectionStateListener(
                            mImsConnectionStateListener);
                } catch (ImsException e) {
                }
                mImsConnectionStateListener = null;
            }

            resetSubInfo();

            if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                mSubId = subId;
                mPhoneId = phoneId;
                mPhone = PhoneFactory.getPhone(mPhoneId);
                mImsManager = ImsManager.getInstance(mContext, mPhoneId);

                ServiceState serviceState = mPhone.getServiceState();

                mRadioOn = mPhone.isRadioOn();
                mRoaming = (serviceState == null ? false : serviceState.getRoaming());
                mLteEnabled = isLteNetworkTypeEnabled(subId);
                mVolteEnabled = mImsManager.isEnhanced4gLteModeSettingEnabledByUser();
                mImsAvailable = mPhone.isImsRegistered();
                if (mSimOnOffEnabled) {
                    mSimOnOffState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(
                            mPhoneId);
                }

                mPhoneStateListener = new MyPhoneStateListener(Looper.getMainLooper());
                mPhoneStateListener.register(mSubId);

                mDataContentObserver = new DataContentObserver(
                        new Handler(Looper.getMainLooper()), mContext, mSubId);
                mDataContentObserver.register();

                if (mImsManager instanceof MtkImsManager) {
                    mImsConnectionStateListener = new MtkImsConnectionStateListener() {
                            @Override
                            public void onImsConnected(int radioTech) {
                                if (!mImsAvailable) {
                                    mImsAvailable = true;
                                    updateState();
                                }
                            }

                            @Override
                            public void onImsDisconnected(ImsReasonInfo info) {
                                if (mImsAvailable) {
                                    mImsAvailable = false;
                                    updateState();
                                }
                            }
                    };
                    try {
                        ((MtkImsManager) mImsManager).addImsConnectionStateListener(
                                mImsConnectionStateListener);
                    } catch (ImsException e) {
                        mImsConnectionStateListener = null;
                    }
                }

                updateState();
            }
        }
    }

    private void updateState() {
        boolean shouldShowAlert = shouldShowVolteAlert();

        Log.d(TAG, "updateState, checking=" + mChecking
                + ", shouldShowAlert=" + shouldShowAlert);

        if (!mChecking && shouldShowAlert) {
            startTimeOutCheck();
        }

        if (mChecking && !shouldShowAlert) {
            stopTimeOutCheck();
        }
    }

    private void handleSubInfoRecordUpdated(Intent intent) {
        String propKey = intent.getStringExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY);
        Log.d(TAG, "handleSubInfoRecordUpdated, propKey=" + (propKey == null ? "null" : propKey));
        if (mSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
                && SubscriptionManager.ENHANCED_4G_MODE_ENABLED.equals(propKey)) {
            Log.d(TAG, "handleSubInfoRecordUpdated, ENHANCED_4G_MODE_ENABLED");
            boolean volteEnabled = mImsManager.isEnhanced4gLteModeSettingEnabledByUser();
            if (volteEnabled != mVolteEnabled) {
                mVolteEnabled = volteEnabled;
                updateState();
            }
        }
    }

    private void handleSimApplicationStateChange(Intent intent) {
        if (!mSimOnOffEnabled) {
            Log.d(TAG, "handleSimApplicationStateChange, SIM On/Off is not enabled.");
            return;
        }

        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        int subId = extra.getInt(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        int state = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
        if (SubscriptionManager.isValidSubscriptionId(subId)
                && (mSubId == subId)
                && (mSimOnOffState != state)) {
            Log.d(TAG, "handleSimApplicationStateChange, subId=" + subId
                    + ", preState=" + mSimOnOffState + ", curState=" + state);
            mSimOnOffState = state;
            updateState();
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceived, action=" + action);

            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                mAirplaneMode = isAirplaneModeOn(mContext);
                updateState();
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                updateListenedSubInfo();
            } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                handleSubInfoRecordUpdated(intent);
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimApplicationStateChange(intent);
            }
        };
    };

    private int getListenedSubId() {
        List<SubscriptionInfo> subList = mSubscriptionManager.getActiveSubscriptionInfoList(true);
        if (subList == null) {
            Log.d(TAG, "getListenedSubId, subList is null.");
            return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        }

        SubscriptionInfo dataSub = mSubscriptionManager.getDefaultDataSubscriptionInfo();
        if (dataSub != null) {
            if (isCtSim(dataSub.getSimSlotIndex())) {
                if (isCt4gSim(dataSub.getSimSlotIndex())) {
                    return dataSub.getSubscriptionId();
                } else {
                    return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                }
            } else if (!MtkImsManager.isSupportMims()) {
                return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            }
        }

        for (SubscriptionInfo info : subList) {
            int phoneId = info.getSimSlotIndex();
            Phone phone = PhoneFactory.getPhone(phoneId);
            ServiceState serviceState = phone.getServiceState();
            boolean roaming = (serviceState == null ? false : serviceState.getRoaming());
            if (isCt4gSim(phoneId) && !roaming) {
                return info.getSubscriptionId();
            }
        }

        return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    private boolean shouldShowVolteAlert() {
        boolean ret = false;

        if (SubscriptionManager.isValidSubscriptionId(mSubId) && isCtVolteEnabled()) {
            boolean autoVolte = isCtAutoVolteEnabled();
            boolean imsAvailable = (mImsAvailable && isLteNetwork());
            boolean simOn = (mSimOnOffState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON);
            int state = ServiceState.STATE_OUT_OF_SERVICE;
            ServiceState ss = mTelephonyManager.getServiceStateForSubscriber(mSubId);
            if (null != ss) {
                state = ss.getVoiceRegState();
            }
            boolean isVoiceEnable = (state == ServiceState.STATE_IN_SERVICE);
            ret = !mAirplaneMode && mRadioOn && !mRoaming && mLteEnabled && mVolteEnabled
                    && simOn && !autoVolte && !imsAvailable && !isVoiceEnable;
            Log.d(TAG, "shouldShowVolteAlert, ret=" + ret + ", subId=" + mSubId
                    + ", phoneId=" + mPhoneId + ", airplaneMode=" + mAirplaneMode
                    + ", radioOn=" + mRadioOn + ", roaming=" + mRoaming
                    + ", lteEnabled=" + mLteEnabled + ", volteEnabled=" + mVolteEnabled
                    + ", simOnOffState=" + mSimOnOffState + ", autoVolte=" + autoVolte
                    + ", imsAvailable=" + imsAvailable
                    + ", isVoiceEnable = " + isVoiceEnable);
        }

        return ret;
    }

    private boolean isLteNetworkTypeEnabled(int subId) {
        boolean enabled = false;
        int networkType = Settings.Global.getInt(
                mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
        switch (networkType) {
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_ONLY:
            case TelephonyManager.NETWORK_MODE_LTE_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE:
            case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_NR_LTE_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                enabled = true;
                break;

            default:
                break;
        }
        Log.d(TAG, "isLteNetworkTypeEnabled, enabled=" + enabled
                + ", networkType=" + networkType);
        return enabled;
    }

    private void startTimeOutCheck() {
        Log.d(TAG, "startTimeOutCheck...");
        mChecking = true;
        sendMessageDelayed(obtainMessage(CHECK_TIME_OUT), CHECK_DURATION);
    }

    private void stopTimeOutCheck() {
        Log.d(TAG, "stopTimeOutCheck...");
        mChecking = false;
        removeMessages(CHECK_TIME_OUT);
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case CHECK_TIME_OUT:
                Log.d(TAG, "handleMessage, checking=" + mChecking);
                if (mChecking && shouldShowVolteAlert()) {
                    showAlertDialog();
                }
                break;

            default:
                break;
        }
    }

    private void showAlertDialog() {
        if (mDialog != null && mDialog.isShowing()) {
            Log.w(TAG, "showAlertDialog, dialog is showing, do nothing.");
            return;
        }

        Log.d(TAG, "showAlertDialog, subId=" + mSubId);

        SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfo(mSubId);
        if (subInfo == null) {
            Log.w(TAG, "showAlertDialog, sub info is null, do nothing.");
            return;
        }

        String displayName = subInfo.getDisplayName().toString();

        final Context context = mContext.getApplicationContext();
        AlertDialog.Builder b = new AlertDialog.Builder(context,
                AlertDialog.THEME_DEVICE_DEFAULT_LIGHT);
        b.setMessage(context.getString(R.string.alert_volte_no_service, displayName));
        b.setCancelable(false);
        b.setPositiveButton(android.R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Log.d(TAG, "onClick, ok, subId=" + mSubId);
                mImsManager.setEnhanced4gLteModeSetting(false);
                stopTimeOutCheck();
            }
        });
        b.setNegativeButton(android.R.string.cancel, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Log.d(TAG, "onClick, cancel, subId=" + mSubId);
                sendMessageDelayed(obtainMessage(CHECK_TIME_OUT), CHECK_DURATION);
            }
        });
        b.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                Log.d(TAG, "onCancel, subId=" + mSubId);
                sendMessageDelayed(obtainMessage(CHECK_TIME_OUT), CHECK_DURATION);
            }
        });
        b.setOnDismissListener(new OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                Log.d(TAG, "onDismiss, subId=" + mSubId);
                mDialog = null;
            }
        });
        Dialog dialog = b.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();
        mDialog = dialog;
    }

    private boolean isLteNetwork() {
        boolean isLte = false;
        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        Log.d(TAG, "isLteNetwork, dataNetworkType=" + mDataNetworkType
                + ", voiceNetworkType=" + mVoiceNetworkType);
        if (mDataNetworkType != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            networkType = mDataNetworkType;
        } else if (mVoiceNetworkType != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            networkType = mVoiceNetworkType;
        }

        if ((networkType == TelephonyManager.NETWORK_TYPE_LTE)
                || (networkType == TelephonyManager.NETWORK_TYPE_LTE_CA)) {
            isLte = true;
        }
        Log.d(TAG, "isLteNetwork, isLte=" + isLte);
        return isLte;
    }

    private boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    public static boolean isCtVolteEnabled() {
        String volteValue = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
        boolean result = (volteValue.equals("1") || volteValue.equals("2")
                || volteValue.equals("3"));
        Log.d(TAG, "isCtVolteEnabled, volteValue=" + volteValue + ", enabled=" + result);
        return result;
    }

    public static boolean isCtAutoVolteEnabled() {
        String volteValue = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
        boolean result = (volteValue.equals("2") || volteValue.equals("3"));
        Log.d(TAG, "isCtAutoVolteEnabled, volteValue=" + volteValue + ", enabled=" + result);
        return result;
    }

    public static boolean isCtSim(int phoneId) {
        boolean result = false;
        CardType cdmaCardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);
        if (cdmaCardType != null) {
            switch (cdmaCardType) {
                case CT_3G_UIM_CARD:
                case CT_UIM_SIM_CARD:
                case CT_4G_UICC_CARD:
                case CT_EXCEL_GG_CARD:
                    result = true;
                    break;

                default:
                    break;
            }
        }
        return result;
    }

    public static boolean isCt4gSim(int phoneId) {
        boolean result = false;
        CardType cdmaCardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);
        if (cdmaCardType != null) {
            switch (cdmaCardType) {
                case CT_4G_UICC_CARD:
                    result = true;
                    break;

                default:
                    break;
            }
        }
        return result;
    }

    private class MyPhoneStateListener extends PhoneStateListener {

        public MyPhoneStateListener(Looper looper) {
            super(looper);
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Log.d(TAG, "PhoneStateListener.onServiceStateChanged"
                    + ", serviceState=" + serviceState + ", phoneId=" + mPhoneId);
            boolean roaming = serviceState.getRoaming();
            int dataType = serviceState.getDataNetworkType();
            int voiceType = serviceState.getVoiceNetworkType();
            if (roaming != mRoaming || dataType != mDataNetworkType
                    || voiceType != mVoiceNetworkType) {
                mRoaming = roaming;
                mDataNetworkType = dataType;
                mVoiceNetworkType = voiceType;
                updateState();
            }
        }

        @Override
        public void onRadioPowerStateChanged(int state) {
            Log.d(TAG, "PhoneStateListener.onRadioPowerStateChanged, state=" + state
                    + ", phoneId=" + mPhoneId);
            boolean radioOn = (state == TelephonyManager.RADIO_POWER_ON);
            if (radioOn != mRadioOn) {
                mRadioOn = radioOn;
                updateState();
            }
        }

        public void register(int subId) {
            mSubId = subId;
            mTelephonyManager = mTelephonyManager.createForSubscriptionId(subId);
            mTelephonyManager.listen(this,
                    PhoneStateListener.LISTEN_SERVICE_STATE
                    | PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);
        }

        public void unregister() {
            mTelephonyManager.listen(this, PhoneStateListener.LISTEN_NONE);
        }
    }

    private class DataContentObserver extends ContentObserver {
        private Context mContext;
        private int mSubId;

        public DataContentObserver(Handler handler, Context context, int subId) {
            super(handler);
            mContext = context;
            mSubId = subId;
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            mLteEnabled = isLteNetworkTypeEnabled(mSubId);
            Log.d(TAG, "ContentObserver.onChange, selfChange=" + selfChange
                    + ", subId=" + mSubId + ", lteEnabled=" + mLteEnabled);
            updateState();
        }

        public void register() {
            final Uri uri = Settings.Global.getUriFor(
                    Settings.Global.PREFERRED_NETWORK_MODE + mSubId);
            mContext.getContentResolver().registerContentObserver(uri, false, this);
        }

        public void unregister() {
            mContext.getContentResolver().unregisterContentObserver(this);
        }
    }

}
