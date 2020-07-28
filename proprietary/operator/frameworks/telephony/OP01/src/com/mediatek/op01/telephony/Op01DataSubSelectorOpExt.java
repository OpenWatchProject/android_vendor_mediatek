package com.mediatek.op01.telephony;

import java.util.Arrays;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;

import android.os.Build;
import android.os.Handler;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.datasub.DataSubConstants;
import com.mediatek.internal.telephony.datasub.IDataSubSelectorOPExt;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.CapabilitySwitch;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;

public class Op01DataSubSelectorOpExt implements IDataSubSelectorOPExt {
    private static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");
    private static boolean DBG = true;

    private static String LOG_TAG = "Op01DSSelector";

    private static Context mContext = null;

    private static DataSubSelector mDataSubSelector = null;

    private static ISimSwitchForDSSExt mSimSwitchForDSS = null;

    private static CapabilitySwitch mCapabilitySwitch = null;

    private int mPhoneNum = 0;

    protected static SharedPreferences mSimSwitchPreference;

    private PreferenceListener mPrefListener;

    private static final String PREF_CATEGORY_RADIO_STATUS = "RADIO_STATUS";

    private long mOpenSimTime = 0l;

    public Op01DataSubSelectorOpExt(Context context) {
        mContext = context;
    }

    public void init(DataSubSelector dataSubSelector, ISimSwitchForDSSExt simSwitchForDSS) {
        mDataSubSelector = dataSubSelector;
        mSimSwitchForDSS = simSwitchForDSS;
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);

        mPhoneNum = mDataSubSelector.getPhoneNum();
        mDataSubSelector.setIsNeedPreCheck(true);
        CapabilitySwitch.setNeedWaitImsi("true");

        mSimSwitchPreference = mContext.getSharedPreferences(PREF_CATEGORY_RADIO_STATUS, 0);
        mPrefListener = new PreferenceListener();
        mSimSwitchPreference.registerOnSharedPreferenceChangeListener(mPrefListener);
    }

    @Override
    public void handleSimStateChanged(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                TelephonyManager.SIM_STATE_UNKNOWN);

        log("handleSimStateChanged, sim state = " + simStatus + ", wait imsi/lock = "
            + CapabilitySwitch.isNeedWaitImsi() + CapabilitySwitch.isNeedWaitUnlock());
        if (simStatus == TelephonyManager.SIM_STATE_LOADED) {
            if (CapabilitySwitch.isNeedWaitImsi()) {
                handleNeedWaitImsi(null);
            } else if (CapabilitySwitch.isNeedWaitUnlock()) {
                handleNeedWaitUnlock(null);
            }
            mCapabilitySwitch.handleSimImsiStatus(intent);
        } else if (simStatus == TelephonyManager.SIM_STATE_NOT_READY) {
            mCapabilitySwitch.handleSimImsiStatus(intent);
        }
    }

    @Override
    public void handleSubinfoRecordUpdated(Intent intent) {
        int detectedType = intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS,
                    MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE);
        String imsEnabledKey = intent.getStringExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY);

        log("handleSubinfoRecordUpdated, detectedType = " + detectedType);
        if (detectedType != MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE) {
            subSelector(intent);
        }
        if (SubscriptionManager.ENHANCED_4G_MODE_ENABLED.equals(imsEnabledKey)) {
            // confirm main protocol when volte on/off changed
            log("handleSubinfoRecordUpdated, ENHANCED_4G_MODE_ENABLED");
            int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
            if (defDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
                if (defDataPhoneId >= 0) {
                    mCapabilitySwitch.setCapability(defDataPhoneId);
                }
            }
        }
    }

    @Override
    public void subSelector(Intent intent) {
        log("DataSubSelector for op01");
        if (intent != null &&
                TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED.equals(intent.getAction())) {
            log("subSelector, record sim status and new sim slot");
            CapabilitySwitch.setSimStatus(intent);
        }
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int insertedSimCount = 0;
        int insertedStatus = 0;
        int detectedType = CapabilitySwitch.getSimStatus();
        String[] currIccId = new String[mPhoneNum];
        int[] simOpInfo = new int[mPhoneNum];
        int[] simType = new int[mPhoneNum];

        mDataSubSelector.setIsNeedPreCheck(false);

        String defaultIccid = "";
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
        if (defDataPhoneId >= 0) {
            if (defDataPhoneId >= DataSubSelectorUtil.getMaxIccIdCount()) {
               log("phoneId out of boundary :" + defDataPhoneId);
            } else {
               defaultIccid = DataSubSelectorUtil.getIccidFromProp(defDataPhoneId);
            }
        }
        for (int i = 0; i < mPhoneNum; i++) {
            currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
            if (currIccId[i] == null || "".equals(currIccId[i])) {
                CapabilitySwitch.setNeedWaitImsi("true");
                log("error: sim not ready, wait for next sub ready");
                return;
            } else if (defaultIccid.equals(currIccId[i])) {
                phoneId = i;
            }

            if (!DataSubConstants.NO_SIM_VALUE.equals(currIccId[i])) {
                ++insertedSimCount;
                insertedStatus = insertedStatus | (1 << i);
            }
        }

        // check pin lock
        if (mCapabilitySwitch.isSimUnLocked() == false) {
            log("DataSubSelector for OP01: do not switch because of sim locking");
            CapabilitySwitch.setNeedWaitUnlock(Boolean.toString(true));
            return;
        } else {
            log("DataSubSelector for OP01: no pin lock");
            CapabilitySwitch.setNeedWaitUnlock(Boolean.toString(false));
        }

        log("Inserted SIM count: " + insertedSimCount + ", insertedStatus: " + insertedStatus);

        if (insertedSimCount == 0) {
            // No SIM inserted
            log("OP01 C0: No SIM inserted, do nothing");
        } else if (insertedSimCount == 1) {
            for (int i = 0; i < mPhoneNum; i++) {
                if ((insertedStatus & (1 << i)) != 0) {
                    phoneId = i;
                    break;
                }
            }

            if ((mCapabilitySwitch.isCanSwitch() == false)
                || (RadioCapabilitySwitchUtil.getSimInfo(
                    simOpInfo, simType, insertedStatus) == false)) {
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
                return;
            }

            log("OP01 C1: Single SIM: Set Default data to phone:" + phoneId);
            mCapabilitySwitch.setCapability(phoneId);
            if (detectedType != MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE) {
                mDataSubSelector.setDefaultData(phoneId);
            }
        } else if (insertedSimCount >= 2) {
            if (mCapabilitySwitch.isCanSwitch() == false || checkOp01CapSwitch6m() == false) {
                // need wait imsi ready
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
                return;
            }
        }
        // clean system property
        CapabilitySwitch.resetSimStatus();
    }

    private void handleNeedWaitImsi(Intent intent) {
        if (CapabilitySwitch.isNeedWaitImsi()) {
            CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));

            log("handleNeedWaitImsi, need precheck = " + mDataSubSelector.getIsNeedPreCheck());
            if (mDataSubSelector.getIsNeedPreCheck()) {
                int isNeedSimSwitch = mSimSwitchForDSS.isNeedSimSwitch();

                // need to check again if unknown
                if (isNeedSimSwitch != DataSubConstants.SIM_SWITCH_UNKNOWN) {
                    mDataSubSelector.setIsNeedPreCheck(false);
                    CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));
                } else {
                    CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
                }

                // check op01 rule
                if (isNeedSimSwitch == DataSubConstants.SIM_SWITCH_NEEDED) {
                    subSelector(intent);
                }

            } else {
                log("NEED_TO_WAIT_IMSI so check again");
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));
                subSelector(intent);
            }
        }
        if (CapabilitySwitch.isNeedWaitImsiRoaming() == true) {
            CapabilitySwitch.setNeedWaitImsiRoaming(Boolean.toString(false));
        }
    }

    private void handleNeedWaitUnlock(Intent intent) {
        if (CapabilitySwitch.isNeedWaitUnlock()) {
            CapabilitySwitch.setNeedWaitUnlock("false");
        }
        if (CapabilitySwitch.isNeedWaitUnlockRoaming()) {
            CapabilitySwitch.setNeedWaitUnlockRoaming("false");
        }
        log("get imsi so check op01 again, do not set mIntent");
        subSelector(intent);
    }

    @Override
    public void handleAirPlaneModeOff(Intent intent) {

    }

    private boolean checkOp01CapSwitch6m() {
        // check if need to switch capability
        int targetPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int insertedSimCount = 0;
        int insertedStatus = 0;

        String[] currIccId = new String[mPhoneNum];
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
        int enabledSimCount = 0;
        int enabledPhoneId = -1;

        if (defDataPhoneId >= 0 && defDataPhoneId < mPhoneNum) {
            // used to determine capability phone when two SIM cards have the same priority
            log("default data phoneId from sub = " + defDataPhoneId);
        } else {
            log("phoneId out of boundary :" + defDataPhoneId);
            defDataPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }

        log("checkOp01CapSwitch6m start");

        for (int i = 0; i < mPhoneNum; i++) {
            currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
            if (currIccId[i] == null || "".equals(currIccId[i])) {
                log("error: sim not ready, wait for next sub ready");
                return false;
            }
            if (!DataSubConstants.NO_SIM_VALUE.equals(currIccId[i])) {
                ++insertedSimCount;
                insertedStatus = insertedStatus | (1 << i);
            }

            if (mSimSwitchPreference.contains(
                    RadioCapabilitySwitchUtil.getHashCode(currIccId[i])) == false) {
                ++enabledSimCount;
                enabledPhoneId = i;
            }
        }
        // check pin lock
        log("checkOp01CapSwitch6m : Inserted SIM count: " + insertedSimCount
                + ", insertedStatus: " + insertedStatus);
        if (mCapabilitySwitch.isSimUnLocked() == false) {
            log("checkOp01CapSwitch6m: sim locked");
            CapabilitySwitch.setNeedWaitUnlock("true");
        } else {
            log("checkOp01CapSwitch6m: no sim locked");
            CapabilitySwitch.setNeedWaitUnlock("false");
        }

        // get the highest priority SIM
        if (enabledSimCount == 1) {
            targetPhoneId = enabledPhoneId;
        } else {
            targetPhoneId = defDataPhoneId;
        }

        log("op01-6m: target phone: " + targetPhoneId);
        if (targetPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
            log("op01-6m: highest priority SIM determined, change!");
            mCapabilitySwitch.setCapability(targetPhoneId);
        } else {
            log("op01-6m: can't determine highest priority SIM, no change");
        }

        // special DEFAULT DATA handle for 2+ SIMs inserted case:
        // if two active sims exist, settings will show dialog and set default data;
        // if only one active sim left(another is radio off), change it to default data.
        if (enabledSimCount == 1) {
            handleDefaultDataOp01MultiSims(targetPhoneId);
        }

        return true;
    }

    private void handleDefaultDataOp01MultiSims(int targetPhoneId) {

        log("OP01 handle Multi SIM: targetPhoneId=" + targetPhoneId);
        if (targetPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
            mDataSubSelector.setDefaultData(targetPhoneId);
        }
    }

    private class PreferenceListener implements OnSharedPreferenceChangeListener {
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            log("onSharedPreferenceChanged, key = " + key);

            // switch on sim
            if (sharedPreferences.contains(key) == false) {
                mDataSubSelector.setIsNeedPreCheck(false);
                CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));
                int delayTime = 0;
                if (System.currentTimeMillis() - mOpenSimTime < 1000) {
                    delayTime = 1000;
                }
                Handler handler = new Handler();
                handler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        subSelector(null);
                    }
                }, delayTime);
                mOpenSimTime = System.currentTimeMillis();
            } else {
                mOpenSimTime = 0;
            }
        }
    }

    public void handlePlmnChanged(Intent intent) {}

    public void handleDefaultDataChanged(Intent intent) {}

    public void handleSimMeLock(Intent intent) {}

    public void handleConnectivityAction() {}

    public void handleSubsidyLockStateAction(Intent intent) {}

    public void handleBootCompleteAction() {}

    public boolean enableAospDefaultDataUpdate() {
        return true;
    }

    public boolean enableAospDisableDataSwitch() {
        return false;
    }

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
}
