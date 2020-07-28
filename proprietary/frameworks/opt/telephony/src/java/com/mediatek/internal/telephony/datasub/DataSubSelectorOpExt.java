package com.mediatek.internal.telephony.datasub;

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

import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import com.mediatek.internal.telephony.datasub.DataSubConstants;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class DataSubSelectorOpExt implements IDataSubSelectorOPExt {
    private static final boolean USER_BUILD = TextUtils.equals(Build.TYPE, "user");

    private static boolean DBG = true;

    private static String LOG_TAG = "DSSExt";

    private static Context mContext = null;

    private static DataSubSelectorOpExt mInstance = null;

    private static DataSubSelector mDataSubSelector = null;

    private static ISimSwitchForDSSExt mSimSwitchForDSS = null;

    private static CapabilitySwitch mCapabilitySwitch = null;

    private static SubscriptionManager mSubscriptionManager = null;

    private static final String PRIMARY_SIM = "primary_sim";

    // marker for retry cause
    private static final int DSS_RET_INVALID_PHONE_INDEX = -1;
    private static final int DSS_RET_CANNOT_GET_SIM_INFO = -2;

    // check result of SIM ME LOCK
    // already set default data by policy
    private static final int SML_CHECK_SWITCH_DONE = 1;
    // waiting for vail card info update
    private static final int SML_CHECK_WAIT_VAILD_CARD_INFO = 2;
    // follow OM flow after check(need set capability)
    private static final int SML_CHECK_FOLLOW_OM = 3;
    // follow OM flow but do notthing after check
    private static final int SML_CHECK_FOLLOW_OM_DO_NOTHING = 4;

    public DataSubSelectorOpExt(Context context) {
        mContext = context;
    }

    public void init(DataSubSelector dataSubSelector, ISimSwitchForDSSExt simSwitchForDSS) {
        mDataSubSelector = dataSubSelector;
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);
        mSimSwitchForDSS = simSwitchForDSS;
        mSubscriptionManager = (SubscriptionManager) mContext
                .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
    }

    @Override
    public void handleSimStateChanged(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
        log("subsidylock: handleSimStateChanged: " + simStatus);
        if (simStatus == TelephonyManager.SIM_STATE_LOADED) {
            log("handleSimStateChanged: INTENT_VALUE_ICC_IMSI");
            mCapabilitySwitch.handleSimImsiStatus(intent);

            handleNeedWaitImsi(intent);
            handleNeedWaitUnlock(intent);
            if (RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
                log("subsidylock: process capability switch in IMSI state");
                subSelectorForOp18Subsidy(intent);
            }
        } else if (simStatus == TelephonyManager.SIM_STATE_NOT_READY) {
            mCapabilitySwitch.handleSimImsiStatus(intent);
        } else if ((simStatus == TelephonyManager.SIM_STATE_PIN_REQUIRED
                  || simStatus == TelephonyManager.SIM_STATE_PUK_REQUIRED
                  || simStatus == TelephonyManager.SIM_STATE_NETWORK_LOCKED)
                  && RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
            log("subsidylock: process capability switch in LOCKED state");
            subSelectorForOp18Subsidy(intent);
        }
    }

    @Override
    public void handleSubinfoRecordUpdated(Intent intent) {
        int detectedType = intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS,
                    MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE);
        log("handleSubinfoRecordUpdated: detectedType = " + detectedType);
        // subsidy lock for OM -- start
        if (detectedType != MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE &&
                RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
            subSelectorForOp18Subsidy(intent);
        } else {
            subSelector(intent);
        }
        // subsidy lock for OM -- end
    }

    private void handleNeedWaitImsi(Intent intent) {
        if (CapabilitySwitch.isNeedWaitImsi()) {
            CapabilitySwitch.setNeedWaitImsi(Boolean.toString(false));
            subSelector(intent);
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

    private int getHighCapabilityPhoneIdBySimType() {
        int phoneId = DSS_RET_INVALID_PHONE_INDEX;
        int[] simOpInfo = new int[mDataSubSelector.getPhoneNum()];
        int[] simType = new int[mDataSubSelector.getPhoneNum()];
        int insertedState = 0;
        int insertedSimCount = 0;
        int tSimCount = 0;
        int wSimCount = 0;
        int cSimCount = 0;
        int op09VolteOffPhoneId = -1;
        String[] currIccId = new String[mDataSubSelector.getPhoneNum()];

        if (RadioCapabilitySwitchUtil.isPS2SupportLTE() && mDataSubSelector.getPhoneNum() == 2) {
            // check sim cards number
            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
                if (currIccId[i] == null || "".equals(currIccId[i])) {
                    log("sim not ready, can not get high capability phone id");
                    return DSS_RET_INVALID_PHONE_INDEX;
                }
                if (!DataSubConstants.NO_SIM_VALUE.equals(currIccId[i])) {
                    ++insertedSimCount;
                    insertedState = insertedState | (1 << i);
                }
            }

            // no sim card
            if (insertedSimCount == 0) {
                log("no sim card, don't switch");
                return DSS_RET_INVALID_PHONE_INDEX;
            }

            // check sim info
            if (RadioCapabilitySwitchUtil.getSimInfo(simOpInfo, simType, insertedState) == false) {
                log("cannot get sim operator info, don't switch");
                return DSS_RET_CANNOT_GET_SIM_INFO;
            }

            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                if (RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01 == simOpInfo[i]) {
                    tSimCount++;
                } else if (RadioCapabilitySwitchUtil.isCdmaCard(i, simOpInfo[i], mContext)) {
                    cSimCount++;
                    simOpInfo[i] = RadioCapabilitySwitchUtil.SIM_OP_INFO_OP09;
                    op09VolteOffPhoneId = i;
                } else if (RadioCapabilitySwitchUtil.SIM_OP_INFO_UNKNOWN!= simOpInfo[i]){
                    wSimCount++;
                    if (simOpInfo[i] != RadioCapabilitySwitchUtil.SIM_OP_INFO_OP09) {
                        simOpInfo[i] = RadioCapabilitySwitchUtil.SIM_OP_INFO_OP02;
                    }
                }
            }
            log("getHighCapabilityPhoneIdBySimType : Inserted SIM count: " + insertedSimCount
                + ", insertedStatus: " + insertedState + ", tSimCount: " + tSimCount
                + ", wSimCount: " + wSimCount + ", cSimCount: " + cSimCount
                + Arrays.toString(simOpInfo));

            // t + w --> if support real T+W, always on t card
            if (RadioCapabilitySwitchUtil.isSupportSimSwitchEnhancement(
                    RadioCapabilitySwitchUtil.ENHANCEMENT_T_PLUS_W)
                    && RadioCapabilitySwitchUtil.isTPlusWSupport()) {
                if (simOpInfo[0] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01 &&
                    simOpInfo[1] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP02) {
                    phoneId = 0;
                } else if (simOpInfo[0] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP02 &&
                    simOpInfo[1] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01) {
                    phoneId = 1;
                }
            }

            // t + c --> always on c card
            if (RadioCapabilitySwitchUtil.isSupportSimSwitchEnhancement(
                    RadioCapabilitySwitchUtil.ENHANCEMENT_T_PLUS_C)) {
                if (simOpInfo[0] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01 &&
                    RadioCapabilitySwitchUtil.isCdmaCard(1, simOpInfo[1], mContext)) {
                    phoneId = 1;
                } else if (RadioCapabilitySwitchUtil.isCdmaCard(0, simOpInfo[0], mContext) &&
                    simOpInfo[1] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01) {
                    phoneId = 0;
                }
            }

            // w + c--> always on c card
            if (RadioCapabilitySwitchUtil.isSupportSimSwitchEnhancement(
                    RadioCapabilitySwitchUtil.ENHANCEMENT_W_PLUS_C)) {
                if (RadioCapabilitySwitchUtil.isCdmaCard(0, simOpInfo[0], mContext) &&
                    simOpInfo[1] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP02) {
                    phoneId = 0;
                } else if (simOpInfo[0] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP02 &&
                    RadioCapabilitySwitchUtil.isCdmaCard(1, simOpInfo[1], mContext)) {
                    phoneId = 1;
                }
            }

            // c + c and only one card volte on --> always on volte off card
            if (simOpInfo[0] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP09 &&
                    simOpInfo[1] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP09 &&
                    (wSimCount == 1) && (cSimCount == 1) && (op09VolteOffPhoneId != -1)) {
                phoneId = op09VolteOffPhoneId;
            }
        }
        log("getHighCapabilityPhoneIdBySimType : " + phoneId);
        return phoneId;
    }

    @Override
    public void subSelector(Intent intent) {
        // only handle 3/4G switching
        int phoneId = DSS_RET_INVALID_PHONE_INDEX;
        String[] currIccId = new String[mDataSubSelector.getPhoneNum()];

        // Sim ME lock
        int checkResult = SML_CHECK_FOLLOW_OM;
        if (MtkTelephonyManagerEx.getDefault().getSimLockPolicy() !=
                MtkIccCardConstants.SML_SLOT_LOCK_POLICY_NONE) {

            checkResult = preCheckForSimMeLock(intent);
            log("preCheckForSimMeLock result=" + checkResult);

            if ((checkResult == SML_CHECK_SWITCH_DONE) ||
                    (checkResult == SML_CHECK_FOLLOW_OM_DO_NOTHING) ||
                    (checkResult == SML_CHECK_WAIT_VAILD_CARD_INFO)) {
                return;
            }
        }

        phoneId = getHighCapabilityPhoneIdBySimType();

        if (phoneId == DSS_RET_CANNOT_GET_SIM_INFO) {
            CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
        } else if (phoneId == DSS_RET_INVALID_PHONE_INDEX) {
            //Get previous default data
            //Modify the way for get defaultIccid,
            //because the SystemProperties may not update on time
            String defaultIccid = "";
            int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
            int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);
            if (defDataPhoneId >= 0) {
                if (defDataPhoneId >= DataSubSelectorUtil.getIccidNum()) {
                   log("phoneId out of boundary :" + defDataPhoneId);
                } else {
                   defaultIccid = DataSubSelectorUtil.getIccidFromProp(defDataPhoneId);
                }
            }
            if (("N/A".equals(defaultIccid)) || ("".equals(defaultIccid))) {
                return;
            }
            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
                if (currIccId[i] == null || "".equals(currIccId[i])) {
                    log("error: iccid not found, wait for next sub ready");
                    return;
                }
                if (defaultIccid.equals(currIccId[i])) {
                    phoneId = i;
                    break;
                }
            }
        }

        // check pin lock
        if (mCapabilitySwitch.isSimUnLocked() == false) {
            log("DataSubSelector for OM: do not switch because of sim locking");
            CapabilitySwitch.setNeedWaitUnlock("true");
            CapabilitySwitch.setSimStatus(intent);
            return;
        } else {
            log("DataSubSelector for OM: no pin lock");
            CapabilitySwitch.setNeedWaitUnlock("false");
        }

        log("Default data phoneid = " + phoneId);
        if (phoneId >= 0) {
            // always set capability to this phone
            mCapabilitySwitch.setCapabilityIfNeeded(phoneId);
        }

        // clean system property
        CapabilitySwitch.resetSimStatus();
    }

// subsidy lock for OM -- start
    public void subSelectorForOp18Subsidy(Intent intent) {
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int detectedType = (intent == null) ? mCapabilitySwitch.getSimStatus() :
                intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, 0);

        log("DataSubSelector for Op18-Subsidy");
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
            log("DataSubSelector for Op18-Subsidy: do not switch because of sim locking");
            CapabilitySwitch.setNeedWaitUnlock("true");
            return;
        } else {
            log("DataSubSelector for Op18-Subsidy: no pin lock");
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

    private void setDefaultData(int phoneId) {
        if (RadioCapabilitySwitchUtil.isSubsidyLockForOmSupported()) {
            log("setDefaultData for subsidylock for phoneId: " + phoneId);
            mDataSubSelector.setDefaultData(phoneId);
        } else {
            SubscriptionController subController = SubscriptionController.getInstance();
            int sub = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
            int currSub = SubscriptionManager.getDefaultDataSubscriptionId();

            log("setDefaultData: " + sub + ", current default sub:" + currSub);
            if (sub != currSub && sub >= SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                // M: DUAL IMS {
                updateImsSim(mContext, sub);
                // @}
            }
        }
    }

    /**
     * Update global setting for IMS SIM.
     * @param context Context
     * @param subId ims sim subid
     */
    private void updateImsSim(Context context, int subId) {
        if (!SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1") &&
                SystemProperties.getInt("persist.vendor.mtk_mims_support", 1) != 1) {
            log("updateImsSim, subId = " + subId);
            Settings.Global.putInt(context.getContentResolver(), PRIMARY_SIM,
                   subId);
            mDataSubSelector.updateNetworkMode(context, subId);
        }
    }

    public void handleBootCompleteAction() {
        log("handleBootCompleteAction");
        int simState1 = TelephonyManager.from(mContext).getSimState(0);
        int simState2 = TelephonyManager.from(mContext).getSimState(1);
        log("subsidylock: simState1 :" + simState1 + ", simState2 : " + simState2);
        if (simState1 == TelephonyManager.SIM_STATE_ABSENT &&
                simState2 == TelephonyManager.SIM_STATE_ABSENT) {
            log("subsidylock: both SIM ABSENT, Set capability and data to phoneId 0");
            mCapabilitySwitch.setCapability(0);
            setDefaultData(0);
        }
    }

    public void handleSubsidyLockStateAction(Intent intent) {
        log("handleSubsidyLockStateAction");
        subSelectorForOp18Subsidy(intent);
    }

    private boolean hasConnectivity() {
        ConnectivityManager cm = (ConnectivityManager) mContext.
                getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        log("DataSubselector, networkinfo: " + info);
        if (info != null && info.isConnected()) {
            DetailedState state = info.getDetailedState();
            log("DataSubselector, DetailedState : " + state);
            if (state == DetailedState.CONNECTED) {
                return true;
            }
        }
        return false;
    }

    public void handleConnectivityAction() {
        log("handleConnectivityAction");
        if (hasConnectivity()) {
            log("SET CONNECTIVITY_STATUS TO 1");
            SystemProperties.set("persist.vendor.subsidylock.connectivity_status",
                    String.valueOf(1));
        } else {
            log("SET CONNECTIVITY_STATUS TO 0");
            SystemProperties.set("persist.vendor.subsidylock.connectivity_status",
                    String.valueOf(0));
        }
    }
// subsidy lock for OM -- end

    @Override
    public void handleAirPlaneModeOff(Intent intent) {
        subSelector(intent);
    }

    public void handlePlmnChanged(Intent intent) {}

    public void handleDefaultDataChanged(Intent intent) {}

    public void handleSimMeLock(Intent intent) {
        subSelector(intent);
    }

    private int preCheckForSimMeLock(Intent intent) {
        int simLockPolicy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        int[] simSlotvaildInfo = new int[mDataSubSelector.getPhoneNum()];

        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int slotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int vaildState = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN;

        int tempDefaultDataPhone = DSS_RET_INVALID_PHONE_INDEX;

        int simCount = 0;
        int unlockedSimCount = 0;
        int simValidCount = 0;

        if ((simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT1) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT2) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS)) {
            phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                    SubscriptionManager.INVALID_PHONE_INDEX);
            subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            vaildState = intent.getIntExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_VALID,
                    MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN);

            log("preCheckForSimMeLock() phoneId=" + phoneId + " subId= " + subId +
                    " vaildState=" + vaildState);

            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                simSlotvaildInfo[i] = MtkTelephonyManagerEx.getDefault().checkValidCard(i);
                log("preCheckForSimMeLock() simSlotvaildInfo[" + i + "]=" + simSlotvaildInfo[i]);

                if (simSlotvaildInfo[i] == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN) {
                    // wait for sim slot vaild state update
                    log("preCheckForSimMeLock() wait for sim vaild state update");
                    return SML_CHECK_WAIT_VAILD_CARD_INFO;
                } else if (simSlotvaildInfo[i] != MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_ABSENT) {
                    simCount = simCount + 1;
                }
            }

            if (simCount == 1) {
                // check if any unlocked SIM
                for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                    if (simSlotvaildInfo[i] == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_YES) {
                        log("preCheckForSimMeLock() only one unlocked sim in slot" + i);
                        return SML_CHECK_FOLLOW_OM;
                    }
                }
                log("preCheckForSimMeLock() only one locked sim in slot");
                return SML_CHECK_FOLLOW_OM_DO_NOTHING;
            } else {
                // find unlock sim
                for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                    if (simSlotvaildInfo[i] == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_YES) {
                        tempDefaultDataPhone = i;
                        unlockedSimCount = unlockedSimCount + 1;
                    }
                }

                if (unlockedSimCount == 1) {
                    // set default data to unlock SIM slot
                    // case 5: one locked SIM + one Unlocked SIM
                    log("preCheckForSimMeLock() one unlocked SIM + n Unlocked SIM");
                    // mCapabilitySwitch.setCapabilityIfNeeded(tempDefaultDataPhone);
                    CapabilitySwitch.setNeedWaitImsi(Boolean.toString(true));
                    mDataSubSelector.setDefaultData(tempDefaultDataPhone);
                    return SML_CHECK_SWITCH_DONE;
                } else if (unlockedSimCount > 1) {
                    // default data sub select by user for case 3
                    // follow OM: check card typ and set main capability
                    // case 3: two unlocked SIMs
                    log("preCheckForSimMeLock() two unlocked SIMs");
                    return SML_CHECK_FOLLOW_OM;
                } else {
                    log("preCheckForSimMeLock() two locked SIMs");
                    return SML_CHECK_FOLLOW_OM_DO_NOTHING;

                }
            }
        } else if ((simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT1) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT2) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA) ||
                (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE) ||
                (simLockPolicy ==
                    MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE)) {
            for (int i = 0; i < mDataSubSelector.getPhoneNum(); i++) {
                simSlotvaildInfo[i] = MtkTelephonyManagerEx.getDefault().checkValidCard(i);
                log("preCheckForSimMeLock() simSlotvaildInfo[" + i + "]=" + simSlotvaildInfo[i]);
                if (simSlotvaildInfo[i] ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN) {
                    // wait for sim slot vaild state update
                    log("preCheckForSimMeLock() wait for sim vaild state update");
                    return SML_CHECK_WAIT_VAILD_CARD_INFO;
                } else if (simSlotvaildInfo[i] ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_YES) {
                    simValidCount = simValidCount + 1;
                }
            }
            log("preCheckForSimMeLock() simValidCount=" + simValidCount);
            if (simValidCount >= 1) {
                return SML_CHECK_FOLLOW_OM;
            } else {
                return SML_CHECK_FOLLOW_OM_DO_NOTHING;
            }
        } else if (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LEGACY) {
            log("Follow OM for Legacy, simLockPolicy=" + simLockPolicy);
            return SML_CHECK_FOLLOW_OM;
        } else {
            log("not handled simLockPolicy=" + simLockPolicy);
            return SML_CHECK_FOLLOW_OM_DO_NOTHING;
        }
    }

    public boolean enableAospDefaultDataUpdate() {
        return true;
    }

    public boolean enableAospDisableDataSwitch() {
        return true;
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
