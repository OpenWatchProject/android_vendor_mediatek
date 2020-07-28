package com.mediatek.op18.telephony;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo.DetailedState;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Settings;

import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;

import com.mediatek.internal.telephony.datasub.IDataSubSelectorOPExt;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.DataSubSelectorOpExt;
import com.mediatek.internal.telephony.datasub.SimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.DataSubConstants;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.CapabilitySwitch;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import java.util.List;

public class Op18DataSubSelectorOpExt implements IDataSubSelectorOPExt {
    private static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");
    private static boolean DBG = true;

    private static String LOG_TAG = "Op18DSSExt";

    private DataSubSelectorOpExt mDataSubSelectorOpExt = null;

    private SimSwitchForDSSExt mSimSwitchForDSSExt = null;

    private static Context mContext = null;

    private static DataSubSelector mDataSubSelector = null;

    private static ISimSwitchForDSSExt mSimSwitchForDSS = null;

    private static CapabilitySwitch mCapabilitySwitch = null;

    private static SubscriptionManager mSubscriptionManager = null;

    private static final String PRIMARY_SIM = "primary_sim";

    public Op18DataSubSelectorOpExt(Context context) {
        log("Op18DataSubSelectorOpExt ");
        mContext = context;
    }

    public void init(DataSubSelector dataSubSelector, ISimSwitchForDSSExt simSwitchForDSS) {
        log("init dataSubSelector");
        mDataSubSelector = dataSubSelector;
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);
        mSimSwitchForDSS = simSwitchForDSS;
        mDataSubSelectorOpExt = new DataSubSelectorOpExt(mContext);
        mSimSwitchForDSSExt = new SimSwitchForDSSExt(mContext);
        mSimSwitchForDSSExt.init(mDataSubSelector);
        mDataSubSelectorOpExt.init(mDataSubSelector, mSimSwitchForDSSExt);
        mSubscriptionManager = (SubscriptionManager) mContext
                .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
    }

    @Override
    public void handleSimStateChanged(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
        log("handleSimStateChanged for op18 " + simStatus);
        if (simStatus == TelephonyManager.SIM_STATE_LOADED) {
            mCapabilitySwitch.handleSimImsiStatus(intent);
            handleNeedWaitImsi();
            handleNeedWaitUnlock(intent);
            if (RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
                log("subsidylock: process capability switch in IMSI state");
                subSelector(intent);
            }
        } else if (simStatus == TelephonyManager.SIM_STATE_NOT_READY) {
            mCapabilitySwitch.handleSimImsiStatus(intent);
        } else if((simStatus == TelephonyManager.SIM_STATE_PIN_REQUIRED
                || simStatus == TelephonyManager.SIM_STATE_PUK_REQUIRED
                || simStatus == TelephonyManager.SIM_STATE_NETWORK_LOCKED)
                && RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
            log("subsidylock: process capability switch in LOCKED state");
            subSelector(intent);
        }
    }

    @Override
    public void handleBootCompleteAction() {
        log("handleBootCompleteAction");
        int simState1 = TelephonyManager.from(mContext).getSimState(0);
        int simState2 = TelephonyManager.from(mContext).getSimState(1);
        log("subsidylock: simState1 :" + simState1 + ", simState2 : " + simState2);
        if (simState1 == TelephonyManager.SIM_STATE_ABSENT &&
                simState2 == TelephonyManager.SIM_STATE_ABSENT) {
            log("subsidylock: both SIM ABSENT, Set capability and data to phoneId 0");
            // Set default capability to phoneId = 0
            mCapabilitySwitch.setCapability(0);
            setDefaultData(0);
        }
    }

    @Override
    public void handleSubsidyLockStateAction(Intent intent) {
        log("handleSubsidyLockStateAction");
        subSelector(intent);
    }

    @Override
    public void handleSubinfoRecordUpdated(Intent intent) {
        int detectedType = intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS,
                    MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE);
        String imsEnabledKey = intent.getStringExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY);
        if (detectedType != MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE) {
            log("handleSubinfoRecordUpdated for op18 " + detectedType);
            subSelector(intent);
        }
        if (SubscriptionManager.ENHANCED_4G_MODE_ENABLED.equals(imsEnabledKey)) {
            log("handleSubinfoRecordUpdated for op18, ENHANCED_4G_MODE_ENABLED" + detectedType);
            // confirm main protocol when volte on/off changed
            int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
            if (defDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
                if (defDataPhoneId >= 0) {
                    mCapabilitySwitch.setCapability(defDataPhoneId);
                }
            }
        }
    }

    private boolean hasConnectivity() {
        ConnectivityManager cm = (ConnectivityManager) mContext.
                getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        log("hasConnectivity, networkinfo: " + info);
        if (info != null && info.isConnected()) {
            DetailedState state = info.getDetailedState();
            log("DetailedState : " + state);
            if (state == DetailedState.CONNECTED) {
                return true;
            }
        }
        return false;
    }

    @Override
    public void handleConnectivityAction() {
        log("handleConnectivityAction");
        if(hasConnectivity()) {
            log("SET CONNECTIVITY_STATUS TO 1");
            SystemProperties.set("persist.vendor.subsidylock.connectivity_status",
                    String.valueOf(1));
        } else {
            log("SET CONNECTIVITY_STATUS TO 0");
            SystemProperties.set("persist.vendor.subsidylock.connectivity_status",
                    String.valueOf(0));
        }
    }

    private void handleNeedWaitImsi() {
        if (CapabilitySwitch.isNeedWaitImsi()) {
            CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));
            log("get imsi and need to check op18 again");
            if (mSimSwitchForDSS.checkCapSwitch(-1) == false) {
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
            }
        }
        if (CapabilitySwitch.isNeedWaitImsiRoaming() == true) {
            CapabilitySwitch.setNeedWaitImsiRoaming(Boolean.toString(false));
        }
    }

    private void handleNeedWaitUnlock(Intent intent) {
        if (CapabilitySwitch.isNeedWaitUnlock()) {
            CapabilitySwitch.setNeedWaitUnlock("false");
            subSelector(intent);
        }
        if (CapabilitySwitch.isNeedWaitUnlockRoaming()) {
            CapabilitySwitch.setNeedWaitUnlockRoaming("false");
        }
    }

    @Override
    public void subSelector(Intent intent) {
        log("subSelectorForOp18: CAPABILITY SWITCH POLICY = " +
                mCapabilitySwitch.getCapabilitySwitchPolicy());
        // op18 has multi policy, decide here
        switch (mCapabilitySwitch.getCapabilitySwitchPolicy()) {
            case DataSubConstants.POLICY_NO_AUTO:
                log("subSelectorForOp18: no auto policy, skip");
                return;
            case DataSubConstants.POLICY_DEFAULT:
                // default policy of op18 is to follow OM.
                mDataSubSelectorOpExt.subSelector(intent);
                return;
            case DataSubConstants.POLICY_POLICY1:
                // keep run op18 policy
                break;
            default:
                log("subSelectorForOp18: Unknow policy, skip");
                return;
        }
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int detectedType = (intent == null) ? mCapabilitySwitch.getSimStatus() :
                intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, 0);

        log("DataSubSelector for op18");
        for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
            int status = SubscriptionManager.getSimStateForSlotIndex(i);
            log("DataSubSelector for Op18-Subsidy: slot:" + i + ", status:" + status);
            if (status == TelephonyManager.SIM_STATE_UNKNOWN ||
                    status == TelephonyManager.SIM_STATE_NOT_READY) {
                // sim is in transient state, wait final event.
                log("DataSubSelector for Op18-Subsidy: sim state update not done, wait.");
                return;
            }
        }

        // check pin lock
        if (mCapabilitySwitch.isSimUnLocked() == false) {
            log("DataSubSelector for OP18: do not switch because of sim locking");
            CapabilitySwitch.setNeedWaitUnlock("true");
            return;
        } else {
            log("DataSubSelector for OP18: no pin lock");
            CapabilitySwitch.setNeedWaitUnlock("false");
        }

        List<SubscriptionInfo> subList = mSubscriptionManager.getActiveSubscriptionInfoList();
        int insertedSimCount = (subList == null || subList.isEmpty()) ? 0: subList.size();
        //Get previous default data
        int defaultSub = SubscriptionManager.getDefaultDataSubscriptionId();
        log("Default sub = " + defaultSub + ", insertedSimCount = " + insertedSimCount);

        if (insertedSimCount == 0) {
            // No SIM inserted
            // 1. Default Data: unset
            // 2. Data Enable: OFF
            // 3. 34G: No change
            log("C0: No SIM inserted, set data unset");
            setDefaultData(SubscriptionManager.INVALID_PHONE_INDEX);
        } else if (insertedSimCount == 1) {
            phoneId = subList.get(0).getSimSlotIndex();

            if (detectedType == MtkSubscriptionManager.EXTRA_VALUE_NEW_SIM) {
                // Case 1: Single SIM + New SIM:
                // 1. Default Data: this sub
                // 2. Data Enable: OFF
                // 3. 34G: this sub
                log("C1: Single SIM + New SIM: Set Default data to phone:" + phoneId);
                if (mCapabilitySwitch.setCapability(phoneId)) {
                    setDefaultData(phoneId);
                }
                mDataSubSelector.setDataEnabled(phoneId, true);
            } else {
                if (defaultSub == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                    //It happened from two SIMs without default SIM -> remove one SIM.
                    // Case 3: Single SIM + Non Data SIM:
                    // 1. Default Data: this sub
                    // 2. Data Enable: OFF
                    // 3. 34G: this sub
                    log("C3: Single SIM + Non Data SIM: Set Default data to phone:" + phoneId);
                    if (mCapabilitySwitch.setCapability(phoneId)) {
                        setDefaultData(phoneId);
                    }
                    mDataSubSelector.setDataEnabled(phoneId, true);
                } else {
                    if (mSubscriptionManager.isActiveSubscriptionId(defaultSub)) {
                        // Case 2: Single SIM + Defult Data SIM:
                        // 1. Default Data: this sub
                        // 2. Data Enable: No Change
                        // 3. 34G: this sub
                        log("C2: Single SIM + Data SIM: Set Default data to phone:" + phoneId);
                        if (mCapabilitySwitch.setCapability(phoneId)) {
                            setDefaultData(phoneId);
                        }
                    } else {
                        // Case 3: Single SIM + Non Data SIM:
                        // 1. Default Data: this sub
                        // 2. Data Enable: OFF
                        // 3. 34G: this sub
                        log("C3: Single SIM + Non Data SIM: Set Default data to phone:" + phoneId);
                        if (mCapabilitySwitch.setCapability(phoneId)) {
                            setDefaultData(phoneId);
                        }
                        mDataSubSelector.setDataEnabled(phoneId, true);
                    }
                }
            }
        } else if (insertedSimCount >= 2) {
            // data switching
            if (mSimSwitchForDSS.checkCapSwitch(-1) == false) {
                // need wait imsi ready
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
                return;
            }
        }
    }

    @Override
    public void handleAirPlaneModeOff(Intent intent) {
        subSelector(intent);
    }

    public void handlePlmnChanged(Intent intent) {}

    public void handleDefaultDataChanged(Intent intent) {}

    private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }

    private void loge(String txt) {
        if (DBG) {
            Rlog.e(LOG_TAG, txt);
        }
    }

    private void setDefaultData(int phoneId) {
        SubscriptionController subController = SubscriptionController.getInstance();
        int sub = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
        int currSub = SubscriptionManager.getDefaultDataSubscriptionId();

        log("setDefaultData: " + sub + ", current default sub:" + currSub);
        if (sub != currSub && sub >= SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            // M: DUAL IMS {
            updateImsSim(mContext, sub);
            // @}

        }
        mDataSubSelector.setDefaultData(phoneId);
    }

    /**
     * Update global setting for IMS SIM.
     * @param context Context
     * @param subId ims sim subid
     */
    private void updateImsSim(Context context, int subId) {
        if (!SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1") &&
                SystemProperties.getInt("persist.vendor.mims_support", 1) != 1) {
            log("updateImsSim, subId = " + subId);
            Settings.Global.putInt(context.getContentResolver(), PRIMARY_SIM,
                   subId);
            mDataSubSelector.updateNetworkMode(context, subId);
        }
    }

    public void handleSimMeLock(Intent intent) {}

    public boolean enableAospDefaultDataUpdate() {
        return true;
    }

    public boolean enableAospDisableDataSwitch() {
        return false;
    }

}
