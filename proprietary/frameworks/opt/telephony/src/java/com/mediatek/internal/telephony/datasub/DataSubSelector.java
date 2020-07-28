package com.mediatek.internal.telephony.datasub;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;

import android.net.ConnectivityManager;
import android.net.NetworkInfo.DetailedState;
import android.net.NetworkInfo;

import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.*;

public class DataSubSelector {
    private static final boolean DBG = true;
    private static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");
    private static final String LOG_TAG = "DSSelector";

    private static DataSubSelector sDataSubSelector = null;

    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private static IDataSubSelectorOPExt mDataSubSelectorOPExt = null;
    private ISimSwitchForDSSExt mSimSwitchForDSSExt = null;

    private int mPhoneNum;
    private boolean mIsWaitIccid = false;
    private boolean mIsNeedPreCheck = true;
    private boolean mIsNeedWaitAirplaneModeOff = false;
    private boolean mIsNeedWaitAirplaneModeOffRoaming = false;

    private boolean mAirplaneModeOn = false;

    private static String mOperatorSpec;

    private Context mContext = null;
    private Intent mIntent = null;
    private boolean mIsInRoaming = false;
    // Add for multi policies for single operator

    private CapabilitySwitch mCapabilitySwitch = null;

    private static String ACTION_SUBSIDY_LOCK_STATE_CHANGE =
            "com.mediatek.subsidy_lock.state_change";
    private static String ACTION_BOOT_COMPLETE =
            "android.intent.action.BOOT_COMPLETED";

    protected final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                return;
            }
            log("onReceive: action=" + action);
            if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimStateChanged(intent);
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                int nDefaultDataSubId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                log("nDefaultDataSubId: " + nDefaultDataSubId);
                handleDefaultDataChanged(intent);
            } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                mAirplaneModeOn = intent.getBooleanExtra("state", false) ? true : false;
                log("ACTION_AIRPLANE_MODE_CHANGED, enabled = " + mAirplaneModeOn);
                if (!mAirplaneModeOn) {
                    if (mIsNeedWaitAirplaneModeOff) {
                        mIsNeedWaitAirplaneModeOff = false;
                        handleAirPlaneModeOff(intent);
                    }
                    if (mIsNeedWaitAirplaneModeOffRoaming) {
                        mIsNeedWaitAirplaneModeOffRoaming = false;
                        // sheldon: todo
                        // getSimSwitchForDSS(OPERATOR_OP02).checkCapSwitch(ROAMING_POLICY);
                    }
                }
            } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                handleSubinfoRecordUpdated(intent);
            } else if (TelephonyIntents.ACTION_LOCATED_PLMN_CHANGED.equals(action)) {
                handlePlmnChanged(intent);
            } else if (action.equals(
                    TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION)) {
                handleSimMeLock(intent);
            } else if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION) &&
                    RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
                log("DataSubSelector receive CONNECTIVITY_ACTION");
                handleConnectivityAction();
            } else if (action.equals(ACTION_SUBSIDY_LOCK_STATE_CHANGE) &&
                    RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
                log("DataSubSelector receive ACTION_SUBSIDY_LOCK_STATE_CHANGE");
                handleSubsidyLockStateAction(intent);
            } else if (action.equals(ACTION_BOOT_COMPLETE) &&
                    RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
                log("DataSubSelector receive ACTION_BOOT_COMPLETE");
                handleBootCompleteAction();
            }
        }
    };

    public static DataSubSelector makeDataSubSelector(Context context, int phoneNum) {
        if (sDataSubSelector == null) {
            sDataSubSelector = new DataSubSelector(context, phoneNum);
        }
        return sDataSubSelector;
    }

    public static IDataSubSelectorOPExt getDataSubSelectorOpExt() {
        if (sDataSubSelector == null) {
            loge("DataSubSelector not init yet!");
            return null;
        }
        return sDataSubSelector.mDataSubSelectorOPExt;
    }

    private DataSubSelector(Context context, int phoneNum) {
        log("DataSubSelector is created");
        mPhoneNum = phoneNum;
        mOperatorSpec = SystemProperties.get(PROPERTY_OPERATOR_OPTR, OPERATOR_OM);

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_LOCATED_PLMN_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        //For sim switch add,to handle C2k card change case ,@{
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        filter.addAction(ACTION_SUBSIDY_LOCK_STATE_CHANGE);
        filter.addAction(ACTION_BOOT_COMPLETE);
        //}@

        // Sim ME lock
        if (MtkTelephonyManagerEx.getDefault().getSimLockPolicy() !=
                MtkIccCardConstants.SML_SLOT_LOCK_POLICY_NONE) {
            filter.addAction(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
        }

        context.registerReceiver(mBroadcastReceiver, filter);
        mContext = context;

        initOpDataSubSelector(context);
        if (mDataSubSelectorOPExt == null) {
            mDataSubSelectorOPExt = new DataSubSelectorOpExt(context);
        }

        initSimSwitchForDSS(context);
        if (mSimSwitchForDSSExt == null) {
            mSimSwitchForDSSExt = new SimSwitchForDSSExt(context);
        }

        mCapabilitySwitch = CapabilitySwitch.getInstance(context, this);

        mSimSwitchForDSSExt.init(this);
        mDataSubSelectorOPExt.init(this, mSimSwitchForDSSExt);

        registerPrefNetworkModeObserver();
    }

    private void initOpDataSubSelector(Context context) {
        try {
            mTelephonyCustomizationFactory =
                    OpTelephonyCustomizationUtils.getOpFactory(context);
            mDataSubSelectorOPExt =
                    mTelephonyCustomizationFactory.makeDataSubSelectorOPExt(context);
        } catch (Exception e) {
            if (DBG) log("mDataSubSelectorOPExt init fail");
            e.printStackTrace();
        }
    }

    private void initSimSwitchForDSS(Context context) {
        try {
            mTelephonyCustomizationFactory =
                    OpTelephonyCustomizationUtils.getOpFactory(context);
            mSimSwitchForDSSExt =
                    mTelephonyCustomizationFactory.makeSimSwitchForDSSOPExt(context);
        } catch (Exception e) {
            if (DBG) log("mSimSwitchForDSSExt init fail");
            e.printStackTrace();
        }
    }

    private void registerPrefNetworkModeObserver() {
        for (int i = 0; i < mPhoneNum; i++) {
            Phone curPhone = PhoneFactory.getPhone(i);
            curPhone.getContext().getContentResolver().registerContentObserver(
                    Settings.Global.getUriFor(
                            Settings.Global.PREFERRED_NETWORK_MODE + curPhone.getSubId()),
                            true,
                            mPrefNetworkModeObserver);;
        }
    }

    private ContentObserver mPrefNetworkModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            if (DBG) log("mPrefNetworkModeObserver, changed");
            handlePrefNetworkModeChanged();
        }
    };

    private void handlePrefNetworkModeChanged() {
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();

        if (defDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
            if (defDataPhoneId >= 0) {
                mCapabilitySwitch.setCapability(defDataPhoneId);
            }
        }
    }

    private void handleDefaultDataChanged(Intent intent) {
        mDataSubSelectorOPExt.handleDefaultDataChanged(intent);
    }

    private void handleSubinfoRecordUpdated(Intent intent) {
        mDataSubSelectorOPExt.handleSubinfoRecordUpdated(intent);
    }

    private void handleSimStateChanged(Intent intent) {
        mDataSubSelectorOPExt.handleSimStateChanged(intent);
    }

    private void handleAirPlaneModeOff(Intent intent) {
        mDataSubSelectorOPExt.handleAirPlaneModeOff(intent);
    }

    private void handlePlmnChanged(Intent intent) {
        mDataSubSelectorOPExt.handlePlmnChanged(intent);
    }

    private void handleConnectivityAction() {
        mDataSubSelectorOPExt.handleConnectivityAction();
    }

    private void handleSubsidyLockStateAction(Intent intent) {
        mDataSubSelectorOPExt.handleSubsidyLockStateAction(intent);
    }

    private void handleBootCompleteAction() {
        mDataSubSelectorOPExt.handleBootCompleteAction();
    }

    private void handleSimMeLock(Intent intent) {
        mDataSubSelectorOPExt.handleSimMeLock(intent);
    }

    public boolean getAirPlaneModeOn() {
        return mAirplaneModeOn;
    }

    public boolean getIsWaitIccid() {
        return mIsWaitIccid;
    }

    public void setIsWaitIccid(boolean isWaitIccid) {
        mIsWaitIccid = isWaitIccid;
    }

    public boolean getIsNeedPreCheck() {
        return mIsNeedPreCheck;
    }

    public void setIsNeedPreCheck(boolean isNeedPreCheck) {
        mIsNeedPreCheck = isNeedPreCheck;
    }

    public void setDataEnabled(int phoneId, boolean enable) {
        log("setDataEnabled: phoneId=" + phoneId + ", enable=" + enable);

        TelephonyManager telephony = TelephonyManager.getDefault();
        if (telephony != null) {
            if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                telephony.setDataEnabled(enable);
            } else {
                int phoneSubId = 0;
                if (enable == false) {
                    phoneSubId = PhoneFactory.getPhone(phoneId).getSubId();
                    log("Set Sub" + phoneSubId + " to disable");
                    telephony.setDataEnabled(phoneSubId, enable);
                } else {
                    for (int i = 0; i < mPhoneNum; i++) {
                        phoneSubId = PhoneFactory.getPhone(i).getSubId();
                        if (i != phoneId) {
                            log("Set Sub" + phoneSubId + " to disable");
                            telephony.setDataEnabled(phoneSubId, false);
                        } else {
                            log("Set Sub" + phoneSubId + " to enable");
                            telephony.setDataEnabled(phoneSubId, true);
                        }
                    }
                }
            }
        }
    }

    public void setDefaultData(int phoneId) {
        SubscriptionController subController = SubscriptionController.getInstance();
        int sub = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
        int currSub = SubscriptionManager.getDefaultDataSubscriptionId();

        log("setDefaultDataSubId: " + sub + ", current default sub:" + currSub);
        if (sub != currSub && sub >= SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            MtkSubscriptionController.getMtkInstance()
                .setDefaultDataSubIdWithoutCapabilitySwitch(sub);
        } else {
            log("setDefaultDataSubId: default data unchanged");
        }
    }

    public int getPhoneNum() {
        return mPhoneNum;
    }

    /////////////////
    private UpdateNWTypeHandler mUpdateNWTypeHandler = new UpdateNWTypeHandler();

    private Handler mProtocolHandler;

    public void updateNetworkMode(Context context, int subId) {
        mContext = context.getApplicationContext();
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();
        if (subInfoList == null) {
            log("subInfoList null");
            return;
        }

        mProtocolHandler = new Handler();
        final Runnable r = new Runnable() {
            public void run() {
                if (subInfoList.size() == 1) {
                    updateNetworkModeUtil(subId, TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA);
                } else if (subInfoList.size() > 1) {
                    for (int index = 0; index < subInfoList.size(); index ++) {
                        int tempSubId = subInfoList.get(index).getSubscriptionId();
                        if (tempSubId == subId) {
                            updateNetworkModeUtil(tempSubId,
                                    TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA);
                        } else {
                            updateNetworkModeUtil(tempSubId,
                                    TelephonyManager.NETWORK_MODE_WCDMA_PREF);
                        }
                    }
                }
            }
        };

        mProtocolHandler.postDelayed(r, 5000);
    }

    private void updateNetworkModeUtil(int subId, int mode) {
        log("Updating network mode for subId " + subId + "mode " + mode);
        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));

        phone.setPreferredNetworkType(mode,
                mUpdateNWTypeHandler.obtainMessage(
                        UpdateNWTypeHandler.MESSAGE_SET_PREFERRED_NETWORK_TYPE, subId, mode));
    }

    /**
     * UpdateNWTypeHandler class to update network type.
     */
    private class UpdateNWTypeHandler extends Handler {

        static final int MESSAGE_SET_PREFERRED_NETWORK_TYPE = 0;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_SET_PREFERRED_NETWORK_TYPE:
                    handleSetPreferredNetworkTypeResponse(msg, msg.arg1, msg.arg2);
                    break;
                default:
                    break;
            }
        }

        private void handleSetPreferredNetworkTypeResponse(Message msg, int subId, int mode) {
            AsyncResult ar = (AsyncResult) msg.obj;
            Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
            if (ar.exception == null) {
                log("handleSetPreferredNetwrokTypeResponse2: networkMode:" + mode);
                android.provider.Settings.Global.putInt(phone.getContext().getContentResolver(),
                        android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                        mode);
            } else {
                log("handleSetPreferredNetworkTypeResponse:exception in setting network.");
            }
        }
    }
    /////////////////

    private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }

    private static void loge(String txt) {
        if (DBG) {
            Rlog.e(LOG_TAG, txt);
        }
    }
}
