package com.mediatek.op01.telephony;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.SubscriptionController;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.CapabilitySwitch;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import java.util.Arrays;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.*;

public class Op01SimSwitchForDSSExt implements ISimSwitchForDSSExt {
    private static String LOG_TAG = "SimSwitchOP01";

    private static boolean DBG = true;

    private static DataSubSelector mDataSubSelector = null;

    private static CapabilitySwitch mCapabilitySwitch = null;

    private static int mPhoneNum = 0;
    protected static SharedPreferences mSimSwitchPreference;
    private static final String PREF_CATEGORY_RADIO_STATUS = "RADIO_STATUS";
    private static final String PROPERTY_CAPABILITY_SWITCH_STATE = "persist.vendor.radio.simswitchstate";

    private Context mContext = null;

    public Op01SimSwitchForDSSExt(Context context) {
        mContext = context;
    }

    public void init(DataSubSelector dataSubSelector) {
        mDataSubSelector = dataSubSelector;
        mPhoneNum = mDataSubSelector.getPhoneNum();
        mSimSwitchPreference = mContext.getSharedPreferences(PREF_CATEGORY_RADIO_STATUS, 0);
        mCapabilitySwitch = CapabilitySwitch.getInstance(mContext, dataSubSelector);
    }

    public boolean checkCapSwitch(int policy) {
        return false;
    }

    public boolean checkCapSwitch() {
        return false;
    }

    public int isNeedSimSwitch() {
        int simSwitchId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH_STATE, -1);
        MtkProxyController proxyCtrl = (MtkProxyController) ProxyController.getInstance();
        int phoneId = SystemProperties.getInt(PROPERTY_3G_SIM, 0) - 1;

        if ((simSwitchId >= 0) && (simSwitchId < mDataSubSelector.getPhoneNum()) &&
            (proxyCtrl.isCapabilitySwitching() == false) && (simSwitchId != phoneId)) {
            int ret = checkOp01Standard(simSwitchId);
            if (ret == SIM_SWITCH_NO_NEED) {
                // triger sim switch
                log("sim switch is not finish at the last time, trigger again");
                mCapabilitySwitch.setCapability(simSwitchId);
                return SIM_SWITCH_NO_NEED;
            } else if (ret == SIM_SWITCH_UNKNOWN) {
                return SIM_SWITCH_UNKNOWN;
            } else {
                return checkOp01Standard(phoneId);
            }
        } else {
            return checkOp01Standard(phoneId);
        }
    }

    public int checkOp01Standard(int phoneId) {
        int[] simOpInfo = new int[mPhoneNum];
        int[] simType = new int[mPhoneNum];
        int targetPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        int insertedSimCount = 0;
        int insertedStatus = 0;
        String[] currIccId = new String[mPhoneNum];
        int[] priority = new int[mPhoneNum];
        int enabledSimCount = 0;
        int enabledPhoneId = -1;
        int op01SimCount = 0;
        int defDataSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int defDataPhoneId = SubscriptionManager.getPhoneId(defDataSubId);

        for (int i = 0; i < mPhoneNum; i++) {
            currIccId[i] = DataSubSelectorUtil.getIccidFromProp(i);
            if (currIccId[i] == null || "".equals(currIccId[i])) {
                log("isNeedSimSwitchForOp01, iccid not found, wait for next sub ready");
                return SIM_SWITCH_UNKNOWN;
            }
            if (!NO_SIM_VALUE.equals(currIccId[i])) {
                ++insertedSimCount;
                insertedStatus = insertedStatus | (1 << i);
                if (mSimSwitchPreference.contains(
                        RadioCapabilitySwitchUtil.getHashCode(currIccId[i])) == false) {
                    ++enabledSimCount;
                    enabledPhoneId = i;
                } else {
                    log("isNeedSimSwitchForOp01, phone" + i + " is in off state");
                }
            } else {
                log("isNeedSimSwitchForOp01, sim" + i + ": no SIM");
            }
        }
        log("isNeedSimSwitchForOp01, 3GPhoneId = " + phoneId + ", enabledSimCount = "
            + enabledSimCount + ", insertedSimCount = " + insertedSimCount + ", insertedStatus = "
            + insertedStatus + ", mPhoneNum = " + mPhoneNum);

        if (RadioCapabilitySwitchUtil.getSimInfo(simOpInfo, simType, insertedStatus) == false) {
            log("isNeedSimSwitchForOp01, fail to get sim info");
            return SIM_SWITCH_UNKNOWN;
        }

        // only one enabled sim
        if (enabledSimCount == 1) {
            if (enabledPhoneId != phoneId) {
                log("isNeedSimSwitchForOp01, only one enabled sim:" + enabledPhoneId);
                return SIM_SWITCH_NEEDED;
            }
        } else if (enabledSimCount == 0){
            log("isNeedSimSwitchForOp01, no enabled sim cards");
            return SIM_SWITCH_NO_NEED;
        } else {
            // dump op info
            log("isNeedSimSwitchForOp01, simOpInfo:" + Arrays.toString(simOpInfo));

            for (int i = 0; i < mPhoneNum; i++) {
                if ((simOpInfo[i] == RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01) &&
                        (mSimSwitchPreference.contains(
                        RadioCapabilitySwitchUtil.getHashCode(currIccId[i])) == false)) {
                    ++op01SimCount;
                }
            }

            // main capability is on other sim
            if ((op01SimCount != 0) && (phoneId >= 0) && (phoneId < mPhoneNum) &&
                (simOpInfo[phoneId] != RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01)) {
                log("isNeedSimSwitchForOp01, main capability is on other sim");
                return SIM_SWITCH_NEEDED;
            }

            // sync with default data
            if (RadioCapabilitySwitchUtil.getMainCapabilityPhoneId() != defDataPhoneId) {
                return SIM_SWITCH_NEEDED;
            }
        }

        return SIM_SWITCH_NO_NEED;
    }

    static private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }

    static private void loge(String txt) {
        if (DBG) {
            Rlog.e(LOG_TAG, txt);
        }
    }
}